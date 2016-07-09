#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION

#include "jsiInt.h"
#include "jsmn.h"
#endif

static int utf8tounicode(const char *str, int *uc)
{
    unsigned const char *s = (unsigned const char *)str;

    if (s[0] < 0xc0) {
        *uc = s[0];
        return 1;
    }
    if (s[0] < 0xe0) {
        if ((s[1] & 0xc0) == 0x80) {
            *uc = ((s[0] & ~0xc0) << 6) | (s[1] & ~0x80);
            return 2;
        }
    }
    else if (s[0] < 0xf0) {
        if (((str[1] & 0xc0) == 0x80) && ((str[2] & 0xc0) == 0x80)) {
            *uc = ((s[0] & ~0xe0) << 12) | ((s[1] & ~0x80) << 6) | (s[2] & ~0x80);
            return 3;
        }
    }
    else if (s[0] < 0xf8) {
        if (((str[1] & 0xc0) == 0x80) && ((str[2] & 0xc0) == 0x80) && ((str[3] & 0xc0) == 0x80)) {
            *uc = ((s[0] & ~0xf0) << 18) | ((s[1] & ~0x80) << 12) | ((s[2] & ~0x80) << 6) | (s[3] & ~0x80);
            return 4;
        }
    }

    /* Invalid sequence, so just return the byte */
    *uc = *s;
    return 1;
}

#define hexdigit(c) (isdigit(c)?(c-'0'):(10+(toupper(c)-'A')))
static const char hexchars[] = "0123456789ABCDEF";

#if 0
static unsigned int
EncodeUtf(unsigned char *ci, unsigned char *uo) {
    int ival, ilen;
    uo[0] = '\\';
    uo[1] = 'u';
    uo[6] = 0;
    ilen = utf8tounicode((const char*)ci,&ival);
    uo[2] = hexchars[(ival&0xF000)>>12];
    uo[3] = hexchars[(ival&0xF00)>>8];
    uo[4] = hexchars[(ival&0xF0)>>4];
    uo[5] = hexchars[(ival&0x0F)];
    return ilen;
}
#endif

static unsigned int
jsi_DecodeUtf(const char *str, char*uo) {
    unsigned char c;
    unsigned int uc = 0, len = 0;
    int pos = 0;

    while(pos<4) {
        c = str[pos];
        if(!isxdigit(c)) {
            return 0;
        }
        uc += ((unsigned int)hexdigit(c) << ((3-pos++)*4));
    }
    if (uc < 0x80) {
        uo[0] = uc;
        len = 1;
    } else if (uc < 0x800) {
        uo[0] = 0xc0 | (uc >> 6);
        uo[1] = 0x80 | (uc & 0x3f);
        len = 2;
    } else {
        uo[0] = 0xe0 | (uc >> 12);
        uo[1] = 0x80 | ((uc >> 6) & 0x3f);
        uo[2] = 0x80 | (uc & 0x3f);
        len = 3;
    }
    return len;
}

 unsigned int
jsi_EncodeUtf(unsigned char *ci, unsigned char *uo) {
    int ival, ilen;
    uo[0] = '\\';
    uo[1] = 'u';
    uo[6] = 0;
    ilen = utf8tounicode((const char*)ci,&ival);
    uo[2] = hexchars[(ival&0xF000)>>12];
    uo[3] = hexchars[(ival&0xF00)>>8];
    uo[4] = hexchars[(ival&0xF0)>>4];
    uo[5] = hexchars[(ival&0x0F)];
    return ilen;
}

static Jsi_Value *jsonNewStringObj(Jsi_Interp *interp, const char* str, int len)
{

    Jsi_Value *v = NULL;
    char buf[100], *dp = buf;
    const char *cp = str;
    Jsi_DString dStr = {};
    int ulen;
    
    while ((cp-str)<len) {
        if (*cp == '\\') {
            switch (cp[1]) {
                case 'b': *dp++ = '\b'; break;
                case 'n': *dp++ = '\n'; break;
                case 'r': *dp++ = '\r'; break;
                case 'f': *dp++ = '\f'; break;
                case 't': *dp++ = '\t'; break;
                case '\\': *dp++ = '\\'; break;
                case 'u': 
                    if ((ulen=jsi_DecodeUtf(cp+2, dp))) {
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
            dp = buf;
            Jsi_DSAppend(&dStr, buf, NULL);
        }
    }
    *dp = 0;
    Jsi_DSAppend(&dStr, buf, NULL);
    v = Jsi_ValueMakeString(interp, NULL, Jsi_Strdup(Jsi_DSValue(&dStr)));
    Jsi_DSFree(&dStr);
    return v;
}

static Jsi_Value* jsonGenObject(Jsi_Interp *interp, jsmn_parser *p, const char *js, uint pos, uint *endPos);
static Jsi_Value* jsonGenArray(Jsi_Interp *interp, jsmn_parser *p, const char *js, uint pos, uint *endPos);

static Jsi_Value*
jsonGen1Value(Jsi_Interp *interp, jsmn_parser *p, const char *js, uint i, uint *endPos, int incr)
{
    uint len;
    const char *t;
    Jsi_Value *v = NULL;
    
    switch (p->tokens[i].type) {
        case JSMN_PRIMITIVE:
            t = jsmn_tokstr(p, js, i, &len);
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
                    Jsi_LogWarn("bad number %*s\n", len, t);
                v = Jsi_ValueMakeNumber(interp, NULL, d);
            }
            break;
        case JSMN_STRING:
            t = jsmn_tokstr(p, js, i, &len);
            v = jsonNewStringObj(interp, t, len);
            break;
        case JSMN_ARRAY:
            v = jsonGenArray(interp, p, js, i, &i);
            i--;
            break;
        case JSMN_OBJECT:
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
jsonGenObject(Jsi_Interp *interp, jsmn_parser *p, const char *js, uint pos, uint *endPos)
{
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
    Jsi_Value *nv, *v = Jsi_ValueMakeObject(interp, NULL, obj);
    uint i, n, len;
    jsmntok_t *tok;
    const char *t;
    tok = p->tokens+pos;
    for (i=pos+1, n=0; i<p->toknext && n<tok->size; i++, n++) {

        Jsi_DString dStr;
        t = jsmn_tokstr(p, js, i, &len);
        i++; n++;
        if (n>=tok->size)
            nv = Jsi_ValueMakeUndef(interp, NULL);
        else
            nv = jsonGen1Value(interp, p, js, i, &i, 0);
        Jsi_DSInit(&dStr);
        Jsi_DSAppendLen(&dStr, t, len);
        Jsi_ObjInsert(interp, obj, Jsi_DSValue(&dStr), nv, p->flags);
        Jsi_DSFree(&dStr);
    }
    if (endPos)
        *endPos = i;
    return v;
}

static Jsi_Value*
jsonGenArray(Jsi_Interp *interp, jsmn_parser *p, const char *js, uint pos, uint *endPos)
{
    if (js==NULL || !js[0])
        return NULL;
    Jsi_Value *v = Jsi_ValueNewArray(interp, 0, 0);
    Jsi_Obj *nobj = v->d.obj;
    uint i, n;
    jsmntok_t *tok;
    
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

int Jsi_JSONParse(Jsi_Interp *interp, const char *js, Jsi_Value **ret, int flags)
{
    uint i = 0, r;
    int result = JSI_OK, strict = (flags & JSI_JSON_STRICT);
    jsmntok_t *tok;
    Jsi_Value *v;
    const char *err;
    if (js == NULL || js[0] == 0)
        return JSI_OK;

    JSMN_DECLARE(pp, tokens);
    jsmn_parser *p = &pp;
    pp.strict = strict;
    pp.flags = flags;

    r = jsmn_parse(p, js);
    if (r != JSMN_SUCCESS) {
        int ofs = pp.pos, len = Jsi_Strlen(js);
        if (ofs<0 || ofs>len)
            ofs = 0;
        err = p->errStr;
        if (!err)
            err = jsmn_errname(r);
        Jsi_LogError("parse error (%s) at offset %d \"%.30s\"", err, ofs, js+ofs);
        result = JSI_ERROR;
        goto done;
    }
    tok = p->tokens;
    if (tok->size<=0) {
        Jsi_ValueMakeUndef(interp, ret);
        goto done;
    }
    v = jsonGen1Value(interp, p, js, i, &i, 1);
    Jsi_ValueReplace(interp, ret, v);
    Jsi_DecrRefCount(interp, v);
done:
    jsmn_free(&pp);
    return result;
}

int Jsi_JSONParseFmt(Jsi_Interp *interp, Jsi_Value **ret, const char *fmt, ...) 
{
    va_list argList;
    uint n;
    char buf[BUFSIZ], *cp = buf;
    Jsi_DString dStr = {};
    va_start(argList, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, argList);
    if (n<0 || n>JSI_MAX_ALLOC_BUF) {
        Jsi_LogError("Jsi_JSONParseFmt error: rc = %d", n);
        va_end(argList);
        return JSI_ERROR;
    }
    if (n >= sizeof(buf)) {
        uint m;
        Jsi_DSSetLength(&dStr, n+1);
        m = vsnprintf(dStr.str, n+1, fmt, argList);
        assert(m == n);
        cp = Jsi_DSValue(&dStr);
    }
    va_end(argList);
    return Jsi_JSONParse(interp, cp, ret, 0);
}

static int _JSONParseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int strict)
{
    const char *js;
    Jsi_Value *jsv = Jsi_ValueArrayIndex(interp, args, 0); 
    js = Jsi_ValueToString(interp, jsv, NULL);
    return Jsi_JSONParse(interp, js, ret, strict);
  
}


static int JSONParseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _JSONParseCmd(interp, args, _this, ret, funcPtr, 1);
}

static int JSONScanCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _JSONParseCmd(interp, args, _this, ret, funcPtr, 0);
}


static int JSONCheckCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{

    int r, result = JSI_OK;
    Jsi_Bool strict = 0;
    const char *cp;
    JSMN_DECLARE(p, tokens);
    Jsi_Value *jsv = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_ValueGetBoolean(interp, Jsi_ValueArrayIndex(interp, args, 1), &strict) == JSI_OK)
        p.strict = strict;
    cp = Jsi_ValueToString(interp, jsv, NULL);
    
    r = jsmn_parse(&p, cp);
    Jsi_ValueMakeBool(interp, ret, r == JSMN_SUCCESS);
    jsmn_free(&p);
    return result;
}

/* TODO: Actually use len for blob. */
char *
Jsi_JSONQuote(Jsi_Interp *interp, const char *cp, int len, Jsi_DString *dsPtr)
{
    char cbuf[10];
    Jsi_DSAppend(dsPtr,"\"",NULL);
    while (*cp) {
        if (*cp == '\\' || *cp == '/' || *cp == '\"') {
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
                    if ((ilen = jsi_EncodeUtf((unsigned char *)cp, (unsigned char*)cbuf))) {
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

static int JSONStringifyCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int quote = JSI_OUTPUT_JSON|JSI_JSON_STRICT;
    Jsi_DString dStr = {};
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_ValueGetDString(interp, arg, &dStr, quote);
    Jsi_ValueMakeString(interp, ret, Jsi_Strdup(Jsi_DSValue(&dStr)));
    Jsi_DSFree(&dStr);
    return JSI_OK;
}

static int JSONStringifyNSCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int quote = JSI_OUTPUT_JSON;
    Jsi_DString dStr = {};
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_ValueGetDString(interp, arg, &dStr, quote);
    Jsi_ValueMakeString(interp, ret, Jsi_Strdup(Jsi_DSValue(&dStr)));
    Jsi_DSFree(&dStr);
    return JSI_OK;
}

int Jsi_CommandInvoke(Jsi_Interp *interp, const char *cmdstr, Jsi_Value *args, Jsi_Value **ret)
{
    Jsi_Value *func = Jsi_NameLookup(interp, cmdstr);
    if (func)
        return Jsi_FunctionInvoke(interp, func, args, ret, NULL);
    Jsi_LogError("can not find cmd: %s", cmdstr);
    return JSI_ERROR;
}

static Jsi_CmdSpec jsonCmds[] = {
    { "parse",      JSONParseCmd,       1, 1, "str:string", .help="Parse JSON and return js", .retType=(uint)JSI_TT_ANY },
    { "parseNS",    JSONScanCmd,        1, 1, "str:string", .help="Non-strict parse (member names not quoted)", .retType=(uint)JSI_TT_ANY},
    { "check",      JSONCheckCmd,       1, 2, "str:string, strict:boolean=false", .help="Return true if str is JSON", .retType=(uint)JSI_TT_BOOL },
    { "stringify",  JSONStringifyCmd,   1, 1, "obj:object", .help="Return JSON from a js object", .retType=(uint)JSI_TT_STRING },
    { "stringifyNS",JSONStringifyNSCmd, 1, 1, "obj:object", .help="Return Non-strict JSON (member names not quoted)", .retType=(uint)JSI_TT_STRING },
    { NULL, .help="Commands for handling JSON data" }
};

int jsi_JSONInit(Jsi_Interp *interp)
{
    Jsi_CommandCreateSpecs(interp, "JSON", jsonCmds, NULL, JSI_CMDSPEC_NOTOBJ);
#ifdef TEST_JSON_INVOKE
    Jsi_Value *ret = Jsi_ValueNew1(interp);
    Jsi_CommandInvokeJSON(interp, "Info.cmds", "[\"*\", true]", ret);
    Jsi_Puts(interp, &ret, 0);
    Jsi_DecrRefCount(interp, ret);
#endif
    return JSI_OK;
}
#endif
