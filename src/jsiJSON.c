#ifndef JSI_LITE_ONLY

#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

static void jsonNewDString(Jsi_Interp *interp, Jsi_DString *dStr, const char* str, int len)
{
    char buf[JSI_MAX_NUMBER_STRING], *dp = buf;
    const char *cp = str;
    int ulen;
    
    while ((cp-str)<len) {
        if (*cp == '\\') {
            switch (cp[1]) {
                case 'b': *dp++ = '\b'; break;
                case 'n': *dp++ = '\n'; break;
                case 'r': *dp++ = '\r'; break;
                case 'f': *dp++ = '\f'; break;
                case 't': *dp++ = '\t'; break;
                case '\"': *dp++ = '\"'; break;
                case '\\': *dp++ = '\\'; break;
                case 'u': 
                    if ((ulen=Jsi_UtfDecode(cp+2, dp))) {
                        dp += ulen;
                        cp += 4;
                    } else {                    
                        *dp++ = '\\';
                        *dp++ = 'u';
                    }
                    break;
            }
            cp+=2;
        } else {
            *dp++ = *cp++;
        }
        if ((dp-buf)>90) {
            *dp = 0;
            Jsi_DSAppendLen(dStr, buf, dp-buf);
            dp = buf;
        }
    }
    *dp = 0;
    Jsi_DSAppendLen(dStr, buf, dp-buf);
}

static Jsi_Value *jsonNewStringObj(Jsi_Interp *interp, const char* str, int len)
{
    Jsi_Value *v = NULL;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    jsonNewDString(interp, &dStr, str, len);
    v = Jsi_ValueNew(interp);
    Jsi_ValueFromDS(interp, &dStr, &v);
    //Jsi_DSFree(&dStr);
    return v;
}

static Jsi_Value* jsonGenObject(Jsi_Interp *interp, Jsi_JsonParser *p, const char *js, uint pos, uint *endPos);
static Jsi_Value* jsonGenArray(Jsi_Interp *interp, Jsi_JsonParser *p, const char *js, uint pos, uint *endPos);

static Jsi_Value*
jsonGen1Value(Jsi_Interp *interp, Jsi_JsonParser *p, const char *js, uint i, uint *endPos, int incr)
{
    uint len;
    const char *t;
    Jsi_Value *v = NULL;
    
    switch (p->tokens[i].type) {
        case JSI_JTYPE_PRIMITIVE:
            t = Jsi_JsonGetTokstr(p, js, i, &len);
            if (len == 4 && Jsi_Strncmp(t, "true", len)==0)
                v = Jsi_ValueMakeBool(interp, NULL, 1);
            else if (len == 5 && Jsi_Strncmp(t, "false", len)==0)
                v = Jsi_ValueMakeBool(interp, NULL, 0);
            else if (len == 4 && Jsi_Strncmp(t, "null", len)==0)
                v = Jsi_ValueMakeNull(interp, NULL);
            else if (len == 9 && Jsi_Strncmp(t, "undefined", len)==0)
                v = Jsi_ValueMakeNull(interp, NULL);
            else {
                char *ep;
                Jsi_Number d;
                d = strtod(t,&ep);
                if (ep>(t+len))
                    Jsi_LogWarn("bad number %*s", len, t);
                v = Jsi_ValueMakeNumber(interp, NULL, d);
            }
            break;
        case JSI_JTYPE_STRING:
            t = Jsi_JsonGetTokstr(p, js, i, &len);
            v = jsonNewStringObj(interp, t, len);
            break;
        case JSI_JTYPE_ARRAY:
            v = jsonGenArray(interp, p, js, i, &i);
            i--;
            break;
        case JSI_JTYPE_OBJECT:
            v = jsonGenObject(interp, p, js, i, &i);
            i--;
            break;
        default:
            break;
    }
    if (endPos)
        *endPos = i;
    if (v == NULL)
        v = Jsi_ValueMakeUndef(interp, NULL);
    if (incr)
        Jsi_IncrRefCount(interp, v);
    return v;
}

static Jsi_Value*
jsonGenObject(Jsi_Interp *interp, Jsi_JsonParser *p, const char *js, uint pos, uint *endPos)
{
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
    Jsi_Value *nv, *v = Jsi_ValueMakeObject(interp, NULL, obj);
    uint i, n, len;
    Jsi_JsonTok *tok;
    const char *t;
    tok = p->tokens+pos;
    for (i=pos+1, n=0; i<p->toknext && n<tok->size; i++, n++) {

        Jsi_DString dStr;
        t = Jsi_JsonGetTokstr(p, js, i, &len);
        i++; n++;
        if (n>=tok->size)
            nv = Jsi_ValueMakeUndef(interp, NULL);
        else
            nv = jsonGen1Value(interp, p, js, i, &i, 0);
        Jsi_DSInit(&dStr);
        Jsi_DSAppendLen(&dStr, t, len);
        Jsi_ObjInsert(interp, obj, Jsi_DSValue(&dStr), nv, 0);
        Jsi_DSFree(&dStr);
    }
    if (endPos)
        *endPos = i;
    return v;
}

static Jsi_Value*
jsonGenArray(Jsi_Interp *interp, Jsi_JsonParser *p, const char *js, uint pos, uint *endPos)
{
    if (js==NULL || !js[0])
        return NULL;
    Jsi_Value *v = Jsi_ValueNewArray(interp, 0, 0);
    Jsi_Obj *nobj = v->d.obj;
    uint i, n;
    Jsi_JsonTok *tok;
    
    tok = p->tokens+pos;
    Jsi_ValueMakeArrayObject(interp, &v, nobj);
    for (i=pos+1, n=0; i<p->toknext && n<tok->size; i++, n++) {
        if (n >= nobj->arrMaxSize) {
            if (Jsi_ObjArraySizer(interp, nobj, n+1)<=0) {
                break;
            }
        }
        nobj->arr[n] = jsonGen1Value(interp, p, js, i, &i, 1);
    }
    Jsi_ObjSetLength(interp, nobj, n);
    if (endPos)
        *endPos = i;
    return v;
}

Jsi_RC Jsi_JSONParse(Jsi_Interp *interp, const char *js, Jsi_Value **ret, int flags)
{
    uint i = 0, r;
    Jsi_RC result = JSI_OK;
    int strict = (flags & JSI_JSON_STRICT);
    Jsi_JsonTok *tok;
    Jsi_Value *v;
    const char *err;
    Jsi_Number d;
    if (js == NULL)
        return JSI_OK;
    while (isspace(*js))
        js++;
    if (js[0] == 0)
        return JSI_OK;
    switch (js[0]) {
        case 't': if (Jsi_Strcmp(js,"true")==0) { if (ret) Jsi_ValueMakeBool(interp, ret, 1); return JSI_OK; } break;
        case 'f': if (Jsi_Strcmp(js,"false")==0) { if (ret) Jsi_ValueMakeBool(interp, ret, 0); return JSI_OK; } break;
        case 'n': if (Jsi_Strcmp(js,"null")==0) { if (ret) Jsi_ValueMakeNull(interp, ret); return JSI_OK; } break;
        case '0': case '1': case '2': case '3': case '4': case '5': 
        case '6': case '7': case '8': case '9': case '-':
            if (Jsi_GetDouble(interp, js, &d) == JSI_OK) { if (ret) Jsi_ValueMakeNumber(interp, ret, d); return JSI_OK; } break;
    }
    JSI_JSON_DECLARE(pp, tokens, 0);
    Jsi_JsonParser *p = &pp;
    pp.strict = strict;
    pp.flags = flags;

    r = Jsi_JsonParse(p, js);
    if (r != JSI_JSON_ERR_NONE) {
        int ofs = pp.pos, len = Jsi_Strlen(js);
        if (ofs<0 || ofs>len)
            ofs = 0;
        err = p->errStr;
        if (!err)
            err = Jsi_JsonGetErrname(r);
        if (interp)
            Jsi_LogError("JSON parse error (%s) at offset %d \"%.30s\"", err, ofs, js+ofs);
        result = JSI_ERROR;
        goto done;
    }
    if (!ret)
        goto done;
    tok = p->tokens;
    if (tok->size<=0) {
        if (!*ret)
            *ret = Jsi_ValueNew1(interp);
        if (tok->type == JSI_JTYPE_OBJECT)
            Jsi_ValueMakeObject(interp, ret, Jsi_ObjNewObj(interp, NULL, 0));
        else if (tok->type == JSI_JTYPE_ARRAY)
            Jsi_ValueMakeArrayObject(interp, ret, Jsi_ObjNew(interp));
        else
            Jsi_ValueMakeUndef(interp, ret);
        goto done;
    }
    v = jsonGen1Value(interp, p, js, i, &i, 1);
    Jsi_ValueReplace(interp, ret, v);
    Jsi_DecrRefCount(interp, v);
done:
    Jsi_JsonFree(&pp);
    return result;
}

// Perform an sprintf format, then return the JSON parsed results.
Jsi_RC Jsi_JSONParseFmt(Jsi_Interp *interp, Jsi_Value **ret, const char *fmt, ...) 
{
    va_list argList;
    uint n;
    char buf[JSI_BUFSIZ], *cp = buf;
    Jsi_DString dStr = {};
    va_start(argList, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, argList);
    if (n>JSI_MAX_ALLOC_BUF) {
        Jsi_LogError("Jsi_JSONParseFmt error: rc = %d", n);
        va_end(argList);
        return JSI_ERROR;
    }
    if (n >= sizeof(buf)) {
        uint m;
        Jsi_DSSetLength(&dStr, n+1);
        m = vsnprintf(Jsi_DSValue(&dStr), n+1, fmt, argList);
        assert(m == n);
        JSI_NOWARN(m);
        cp = Jsi_DSValue(&dStr);
    }
    va_end(argList);
    return Jsi_JSONParse(interp, cp, ret, 0);
}

// Process options from json string. Handles only primatives and string. Returns number of fields parsed.
int Jsi_OptionsProcessJSON(Jsi_Interp *interp, Jsi_OptionSpec *opts, void *data, const char *json, Jsi_Wide flags)
{
    uint i = 0, r, len;
    Jsi_RC rc = JSI_OK;
    int result = 0;
    int strict = (flags & JSI_JSON_STRICT);
    Jsi_JsonTok *tok;
    const char *err;
    Jsi_Value *v = NULL;
    Jsi_DString nStr;
    const char *t, *name = NULL;
    if (json == NULL)
        return 0;
    while (isspace(*json))
        json++;
    if (json[0] == 0)
        return 0;
        
    JSI_JSON_DECLARE(pp, tokens, 0);
    Jsi_JsonParser *p = &pp;
    pp.strict = strict;
    pp.flags = flags;

    r = Jsi_JsonParse(p, json);
    if (r != JSI_JSON_ERR_NONE) {
        int ofs = pp.pos, len = Jsi_Strlen(json);
        if (ofs<0 || ofs>len)
            ofs = 0;
        err = p->errStr;
        if (!err)
            err = Jsi_JsonGetErrname(r);
        Jsi_LogError("JSON parse error (%s) at offset %d \"%.30s\"", err, ofs, json+ofs);
        result = -1;
        goto done;
    }
    tok = p->tokens;
    Jsi_DSInit(&nStr);
    if (tok->size%2 || tok->type != JSI_JTYPE_OBJECT) {
        result = -1;
        goto done;
    }
    v = Jsi_ValueNew1(interp);
    for (i=1; i<p->toknext && i<tok->size; i++) {
        if (p->tokens[i].type != JSI_JTYPE_STRING) {
            result = Jsi_LogError("expected string at %d", i-1);
            goto bail;
        }
        name = Jsi_JsonGetTokstr(p, json, i, &len);
        Jsi_DSSetLength(&nStr, 0);
        Jsi_DSAppendLen(&nStr, name, len);
        name = Jsi_DSValue(&nStr);
        i++;
        switch (p->tokens[i].type) {
            case JSI_JTYPE_PRIMITIVE:
                t = Jsi_JsonGetTokstr(p, json, i, &len);
                if ((len == 4 && Jsi_Strncmp(t, "true", len)==0) || (len == 5 && Jsi_Strncmp(t, "false", len)==0)) {
                    Jsi_ValueMakeBool(interp, &v, (bool)(len==4?1:0));
                } else if (len == 4 && Jsi_Strncmp(t, "null", len)==0) {
                    Jsi_ValueMakeNull(interp, &v);
                } else {
                    char *ep;
                    Jsi_Number d = strtod(t,&ep);
                    if (ep>(t+len)) {
                        result = Jsi_LogError("bad number %*s", len, t);
                        goto bail;
                    }
                    Jsi_ValueMakeNumber(interp, &v, d);
                }
                break;
            case JSI_JTYPE_STRING:
                t = Jsi_JsonGetTokstr(p, json, i, &len);
                Jsi_DString dStr;
                Jsi_DSInit(&dStr);
                jsonNewDString(interp, &dStr, t, len);
                Jsi_ValueMakeStringKey(interp, &v, Jsi_DSValue(&dStr));
                Jsi_DSFree(&dStr);
                break;
            case JSI_JTYPE_ARRAY:
            case JSI_JTYPE_OBJECT:
            default:
                result = -1;
                goto bail;
                break;
        }
        result++;
        rc = Jsi_OptionsSet(interp, opts, data, name, v, 0);
        if (rc == JSI_OK)
            continue;

bail:
        Jsi_LogError("bad at field: %s", name);
        result = -1;
        break;
    }

done:
    Jsi_DSFree(&nStr);
    if (v)
        Jsi_DecrRefCount(interp, v);
    Jsi_JsonFree(&pp);
    return result;
}

static Jsi_RC JSONParseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    bool strict = 1;
    Jsi_Value *jsv = Jsi_ValueArrayIndex(interp, args, 1);
    if (jsv && Jsi_ValueGetBoolean(interp, jsv, &strict) != JSI_OK) 
        return Jsi_LogError("Expected boolean");
    jsv = Jsi_ValueArrayIndex(interp, args, 0); 
    const char *js = Jsi_ValueToString(interp, jsv, NULL);
    return Jsi_JSONParse(interp, js, ret, strict);
}



static Jsi_RC JSONCheckCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{

    int r;
    Jsi_RC result = JSI_OK;
    bool strict = 1;
    const char *cp;
    JSI_JSON_DECLARE(p, tokens, 0);
    Jsi_Value *jsv = Jsi_ValueArrayIndex(interp, args, 1);
    if (jsv && Jsi_ValueGetBoolean(interp, jsv, &strict) != JSI_OK) 
        return Jsi_LogError("Expected boolean");
    p.strict = strict;
    jsv = Jsi_ValueArrayIndex(interp, args, 0);
    cp = Jsi_ValueToString(interp, jsv, NULL);
    
    r = Jsi_JsonParse(&p, cp);
    Jsi_ValueMakeBool(interp, ret, r == JSI_JSON_ERR_NONE);
    Jsi_JsonFree(&p);
    return result;
}

// Apply JSON quoting to str and append result to dsPtr.
char *
Jsi_JSONQuote(Jsi_Interp *interp, const char *str, int len, Jsi_DString *dsPtr)
{
    const char *cp = str;
    int i = 0;
    if (len<0)
        len = Jsi_Strlen(str);
    char cbuf[10];
    Jsi_DSAppend(dsPtr,"\"",NULL);
    while (*cp && i++<len) {
        if (*cp == '\\' /* || *cp == '/' */ || *cp == '\"') {
            cbuf[0] = '\\';
            cbuf[1] = *cp;
            cbuf[2] = 0;
            Jsi_DSAppend(dsPtr,cbuf,NULL);
        } else if (!isprint(*cp)) {
            int ilen;
            switch (*cp) {
                case '\b': Jsi_DSAppend(dsPtr,"\\b",NULL); break;
                case '\n': Jsi_DSAppend(dsPtr,"\\n",NULL); break;
                case '\r': Jsi_DSAppend(dsPtr,"\\r",NULL); break;
                case '\f': Jsi_DSAppend(dsPtr,"\\f",NULL); break;
                case '\t': Jsi_DSAppend(dsPtr,"\\t",NULL); break;
                default:
                    if ((ilen = Jsi_UtfEncode(cp, cbuf))) {
                        Jsi_DSAppend(dsPtr,cbuf,NULL);
                        cp += (ilen-1);
                    }
            }
        } else {
            cbuf[0] = *cp;
            cbuf[1] = 0;
            Jsi_DSAppend(dsPtr,cbuf,NULL);
        }
        cp++;
    }
    
    Jsi_DSAppend(dsPtr,"\"", NULL);
    return Jsi_DSValue(dsPtr);;
}

static Jsi_RC JSONStringifyCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    bool strict = 1;
    Jsi_Value *jsv = Jsi_ValueArrayIndex(interp, args, 1);
    if (jsv && Jsi_ValueGetBoolean(interp, jsv, &strict) != JSI_OK) 
        return Jsi_LogError("Expected boolean");
    int quote = JSI_OUTPUT_JSON;
    if (strict) quote|=JSI_JSON_STRICT;
    Jsi_DString dStr = {};
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_ValueGetDString(interp, arg, &dStr, quote);
    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
}

Jsi_RC Jsi_CommandInvoke(Jsi_Interp *interp, const char *cmdstr, Jsi_Value *args, Jsi_Value **ret)
{
    Jsi_Value *func = Jsi_NameLookup(interp, cmdstr);
    if (func)
        return Jsi_FunctionInvoke(interp, func, args, ret, NULL);
    return Jsi_LogError("can not find cmd: %s", cmdstr);
}

static Jsi_CmdSpec jsonCmds[] = {
    { "check",      JSONCheckCmd,       1, 2, "str:string, strict:boolean=true", .help="Return true if str is JSON", .retType=(uint)JSI_TT_BOOLEAN },
    { "parse",      JSONParseCmd,       1, 2, "str:string, strict:boolean=true", .help="Parse JSON and return js", .retType=(uint)JSI_TT_ANY },
    { "stringify",  JSONStringifyCmd,   1, 2, "value:any,  strict:boolean=true", .help="Return JSON from a js object", .retType=(uint)JSI_TT_STRING },
    { NULL, 0,0,0,0, .help="Commands for handling JSON data" }
};

Jsi_RC jsi_InitJSON(Jsi_Interp *interp, int release) {
    if (release) return JSI_OK;
    Jsi_CommandCreateSpecs(interp, "JSON", jsonCmds, NULL, 0);
#ifdef TEST_JSON_INVOKE
    Jsi_Value *ret = Jsi_ValueNew1(interp);
    Jsi_CommandInvokeJSON(interp, "Info.cmds", "[\"*\", true]", ret);
    Jsi_DString dStr = {};
    Jsi_Puts(NULL, Jsi_ValueGetDString(interp, ret, &dStr, 1), -1);
    Jsi_DSFree(&dStr);
    Jsi_DecrRefCount(interp, ret);
#endif
    return JSI_OK;
}
#endif
