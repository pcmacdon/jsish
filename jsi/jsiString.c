#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#define ChkString0(_this, funcPtr) \
    if (_this->vt != JSI_VT_OBJECT || _this->d.obj->ot != JSI_OT_STRING) { \
        Jsi_LogError("apply String.%s to a non-string object\n", funcPtr->cmdSpec->name); \
        return JSI_ERROR; \
    }
    
#define ChkString(_this, funcPtr, dest, lenPtr) \
    if (_this->vt == JSI_VT_OBJECT && _this->d.obj->ot == JSI_OT_FUNCTION &&  \
       _this->d.obj->__proto__ == interp->String_prototype->d.obj->__proto__ ) { \
        skip = 1; \
        dest = Jsi_ValueArrayIndexToStr(interp, args, 0, lenPtr); \
    } else if (_this->vt != JSI_VT_OBJECT || _this->d.obj->ot != JSI_OT_STRING) { \
        Jsi_LogError("apply String.%s to a non-string object\n", funcPtr->cmdSpec->name); \
        return JSI_ERROR; \
    } else  { \
        dest = Jsi_ValueString(interp, _this, lenPtr); \
        if (!dest) dest = (char*)""; \
    }

static int StringConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        const char *nv = "";
        if (Jsi_ValueGetLength(interp, args) > 0) {
            Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
            if (v) {
                
                nv = Jsi_ValueToString(interp, v, NULL);
            }
        }
        _this->d.obj->ot = JSI_OT_STRING;
        _this->d.obj->d.s.str = Jsi_Strdup(nv);
        _this->d.obj->d.s.len = -1;
        //Jsi_ObjSetLength(interp, _this->d.obj, Jsi_Strlen(nv));
        /*
        int i;
        int len = Jsi_Strlen(nv);
        for (i = 0; i < len; ++i) {
            Jsi_Value *v = Jsi_ValueNew(interp);
            Jsi_ValueMakeString(interp,v, jsi_SubstrDup(nv, i, 1));

            jsi_ValueInsertArray(interp, _this, i, v, JSI_OM_DONTDEL|JSI_OM_DONTENUM);
        }*/
        
        return JSI_OK;
    }
    if (Jsi_ValueGetLength(interp, args) > 0) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
        if (v) {
            Jsi_ValueDup2(interp, ret, v);
            Jsi_ValueToString(interp, *ret, NULL);
            return JSI_OK;
        }
    }
    Jsi_ValueMakeStringDup(interp, ret, "");
    return JSI_OK;
}

static int StringFromCharCodeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char unibuf[BUFSIZ+1], *u = unibuf;
    //ChkString0(_this, funcPtr);
    
    int len = Jsi_ValueGetLength(interp, args);
    int i;

    if (len > BUFSIZ)
        len = BUFSIZ;
    
    unibuf[len] = 0;

    for (i = 0; i < len; ++i) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, i);
        if (!v) Jsi_LogBug("Arguments Jsi_LogError\n");

        Jsi_ValueToNumber(interp, v);
        u[i] = (char) v->d.num;
    }
    u[i] = 0;
    Jsi_ValueMakeStringDup(interp, ret, unibuf);
    return JSI_OK;
}


static int StringSplitCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char **argv; int argc;
    int skip = 0, rc = JSI_OK;
    char *v;
    
    ChkString(_this, funcPtr, v, NULL);

    Jsi_Value *spliton = Jsi_ValueArrayIndex(interp, args, skip);
    
    if (!spliton || !Jsi_ValueIsString(interp, spliton)) {
        Jsi_ValueMakeStringDup(interp, ret, v);
        return JSI_OK;
    }
    char* split = Jsi_ValueString(interp, spliton, NULL);

    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    Jsi_SplitStr(v, &argc, &argv, split, &sStr);
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    if (Jsi_ObjArraySizer(interp, obj, argc) <= 0) {
        Jsi_LogError("index too large: %d", argc);
        rc = JSI_ERROR;
        Jsi_ObjFree(interp, obj);
        goto bail;
    }
    Jsi_ValueMakeArrayObject(interp, ret, obj);
    int i;
    for (i = 0; i < argc; ++i) {
        Jsi_Value *v = Jsi_ValueNewStringDup(interp, argv[i]);
        Jsi_IncrRefCount(interp, v);
        obj->arr[i] = v;
    }
    Jsi_ObjSetLength(interp, obj, argc);
    
bail:
    Jsi_DSFree(&sStr);
    return rc;
}

static int StringSubstrCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    char *v;
    ChkString(_this, funcPtr, v, NULL);

    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, skip);
    Jsi_Value *len = Jsi_ValueArrayIndex(interp, args, skip+1);
    
    Jsi_Number nstart, nlen;
    if (!start || Jsi_GetNumberFromValue(interp,start, &nstart) != JSI_OK) {
        Jsi_ValueMakeStringDup(interp, ret, v);
        return JSI_OK;
    }
    int istart = (int)nstart;
    if (!len || Jsi_GetNumberFromValue(interp,len, &nlen) != JSI_OK) {
        Jsi_ValueMakeString(interp, ret, jsi_SubstrDup(v, istart, -1));
        return JSI_OK;
    }
    int ilen = (int)nlen;
    if (ilen <= 0) {
        Jsi_ValueMakeStringDup(interp, ret, "");
    } else {
        Jsi_ValueMakeString(interp, ret, jsi_SubstrDup(v, istart, ilen));
    }
    return JSI_OK;
}

static int StringSubstringCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0, ilen = 0;
    char *v;
    ChkString(_this, funcPtr, v, &ilen);

    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, skip);
    Jsi_Value *end = Jsi_ValueArrayIndex(interp, args, skip+1);
    
    Jsi_Number nstart, nend;
    if (!start || Jsi_GetNumberFromValue(interp,start, &nstart) != JSI_OK) {
        Jsi_ValueMakeStringDup(interp, ret, v);
        return JSI_OK;
    }
    int istart = (int)nstart;
    
    if (!end || Jsi_GetNumberFromValue(interp,end, &nend) != JSI_OK) {
        Jsi_ValueMakeString(interp, ret, jsi_SubstrDup(v, istart, -1));
        return JSI_OK;
    }
    int iend = (int)nend;
    if (iend>ilen)
        iend = ilen;
    if (iend < istart) {
        Jsi_ValueMakeStringDup(interp, ret, "");
    } else {
        Jsi_ValueMakeString(interp, ret, jsi_SubstrDup(v, istart, iend-istart+1));
    }
    return JSI_OK;
}


static int StringIndexOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    char *v;
    ChkString(_this, funcPtr, v, NULL);

    Jsi_Value *seq = Jsi_ValueArrayIndex(interp, args, skip);
    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, skip+1);

    if (!seq) {
        Jsi_ValueMakeNumber(interp,ret, -1);
        return JSI_OK;
    }

    const char *vseq = Jsi_ValueToString(interp, seq, NULL);
    int istart = 0;
    if (start) {
        if (Jsi_GetIntFromValue(interp, start, &istart)) {
            return JSI_ERROR;
        }
        if (istart < 0) istart = 0;
    }

    int r = Jsi_Strpos(v, istart, vseq, 0);
    Jsi_ValueMakeNumber(interp, ret, r);

    return JSI_OK;
}

static int StringMatchCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    char *source_str;
    ChkString(_this, funcPtr, source_str, NULL);
    //int source_len = Jsi_Strlen(source_str);
    char *v = source_str;
    Jsi_Value *seq = Jsi_ValueArrayIndex(interp, args, skip);

    if (Jsi_ValueIsString(interp, seq)) {
        char *cp = Jsi_ValueString(interp, seq, NULL);

        if (jsi_RegExpValueNew(interp, cp, seq) != JSI_OK)
            return JSI_ERROR;
    }
    /* TODO: differentiate from re.exec() */
    return Jsi_RegExpMatches(interp, seq, v, *ret);
}

static int StringCharCodeAtCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *ttPtr = Jsi_ValueNew1(interp);
    Jsi_ValueCopy(interp, ttPtr, _this);
    const char *ttStr = Jsi_ValueToString(interp, ttPtr, NULL);
    
    int slen = Jsi_Strlen(ttStr);
    int pos = 0;
    Jsi_Value *vpos;
    if ((vpos = Jsi_ValueArrayIndex(interp, args, 0))) {
        jsi_ValueToOInt32(interp, vpos);
        pos = (int)vpos->d.num;
    }

    if (pos < 0 || pos >= slen) {
        Jsi_ValueMakeNumber(interp, ret, jsi_ieee_makenan());
    } else {
        Jsi_ValueMakeNumber(interp, ret, ttStr[pos]);
    }
    Jsi_DecrRefCount(interp, ttPtr);
    return JSI_OK;
}


static int _StringTrimCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int ends)
{
    int skip = 0;
    const char *tstr = " \t\n\r", *vstr;
    int vend,  n, slen = Jsi_Strlen(tstr), vlen;
    ChkString(_this, funcPtr, vstr, &vlen);
    
    Jsi_Value *tchars = Jsi_ValueArrayIndex(interp, args, skip);
    
    if (tchars) {
        tstr = Jsi_ValueToString(interp, tchars, NULL);
        slen = Jsi_Strlen(tstr);
    }
    
    if (ends&1) {
        while (*vstr) {
            for (n=0; n<slen; n++)
                if (tstr[n] == *vstr) break;
            if (n>=slen) break;
            vstr++;
            vlen--;
        }
    }
    vend = vlen-1;
    if (ends&2) {
        for (; vend>=0; vend--) {
            for (n=0; n<slen; n++)
                if (tstr[n] == vstr[vend]) break;
            if (n>=slen) break;
        }
    }
    Jsi_ValueMakeString(interp, ret, jsi_SubstrDup(vstr, 0, vend+1));
    return JSI_OK;
}

static int StringTrimCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _StringTrimCmd(interp, args, _this, ret, funcPtr, 3);
}
static int StringTrimLeftCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _StringTrimCmd(interp, args, _this, ret, funcPtr, 1);
}

static int StringTrimRightCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _StringTrimCmd(interp, args, _this, ret, funcPtr, 2);
}


static char *unistrdup_tolower(const char *str)
{
    char *cp, *s;
    cp = s = (char*)Jsi_Strdup(str);
    while (*cp) {
        *cp = tolower(*cp);
        cp++;
    }
    return s;
}

static char *unistrdup_toupper(const char *str)
{
    char *cp, *s;
    cp = s = (char*)Jsi_Strdup(str);
    while (*cp) {
        *cp = toupper(*cp);
        cp++;
    }
    return s;
}

static int StringToLowerCaseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    char *str, *vstr;
    ChkString(_this, funcPtr, vstr, NULL);
    skip = skip;
    str = unistrdup_tolower(vstr);
    Jsi_ValueMakeString(interp, ret, str);
    return JSI_OK;
}

static int StringToUpperCaseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    char *str, *vstr;
    ChkString(_this, funcPtr, vstr, NULL);
    skip = skip;

    str = unistrdup_toupper(vstr);
    Jsi_ValueMakeString(interp, ret, str);
    return JSI_OK;
}

static int StringToTitleCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    char *str, *vstr;
    ChkString(_this, funcPtr, vstr, NULL);
    skip = skip;
    str = Jsi_Strdup(vstr);
    str[0] = toupper(str[0]);
    Jsi_ValueMakeString(interp, ret, str);
    return JSI_OK;
}

static int StringCharAtCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int pos = 0, len, skip = 0;
    char *vstr;
    ChkString(_this, funcPtr, vstr, &len);
    skip = skip;
   
    Jsi_Value *vpos = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_GetIntFromValue(interp, vpos, &pos)) {
        return JSI_ERROR;        
    }
    if (pos<0 || pos >=len)
        Jsi_ValueMakeStringDup(interp, ret, "");
    else
        Jsi_ValueMakeString(interp, ret, jsi_SubstrDup(vstr, pos, 1));
    return JSI_OK;
}

static int StringLastIndexOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    char *v;
    ChkString(_this, funcPtr, v, NULL);

    Jsi_Value *seq = Jsi_ValueArrayIndex(interp, args, skip);
    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, 1+skip);

    if (!seq) {
        Jsi_ValueMakeNumber(interp, ret, -1);
        return JSI_OK;
    }

    const char *vseq = Jsi_ValueToString(interp, seq, NULL);
    int istart = 0;
    if (start) {
        if (Jsi_GetIntFromValue(interp, start, &istart)) {
            return JSI_ERROR;
        }
        if (istart < 0) istart = 0;
    }

    int r = Jsi_Strrpos(v, istart, vseq, 0);
    Jsi_ValueMakeNumber(interp, ret, r);

    return JSI_OK;;
}

static int StringConcatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    const char *vstr;
    ChkString(_this, funcPtr, vstr, NULL);
    
    Jsi_DString dStr = {};
    Jsi_DSAppend(&dStr, vstr, NULL);
    int i, argc = Jsi_ValueGetLength(interp, args);
    for (i=skip; i<argc; i++)
    {
        Jsi_Value *s = Jsi_ValueArrayIndex(interp, args, i);
        if (Jsi_GetStringFromValue(interp, s, &vstr)) {
            Jsi_LogError("String get failure\n");
            Jsi_DSFree(&dStr);
            return JSI_ERROR;
        }
        Jsi_DSAppend(&dStr, vstr, NULL);
    }

    Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return JSI_OK;
    
}

static int StringSliceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    const char *vstr;
    ChkString(_this, funcPtr, vstr, NULL);

    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, skip);
    Jsi_Value *end = Jsi_ValueArrayIndex(interp, args, 1+skip);
    int istart, iend, len = strlen(vstr);

    if (Jsi_GetIntFromValue(interp, start, &istart)) {
        return JSI_ERROR;
    }
    if (istart < 0)
        istart = len-istart;
    if (istart < 0)
        istart = 0;
    if (istart>=len)
        istart = len-1;
    iend = len-1;
    if (end) {
        if (Jsi_GetIntFromValue(interp, end, &iend)) {
            return JSI_ERROR;
        }
        if (iend < 0)
            iend = len+iend;
        if (iend>=len)
            iend = len-1;
        if (iend<istart)
            iend = istart;
    }

    char *r = jsi_SubstrDup(vstr, istart, iend);
    Jsi_ValueMakeString(interp, ret, r);
    return JSI_OK;
}

/*
static int StringFormatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int rc;
    Jsi_DString dStr = {};
    rc = Jsi_FormatString(interp, args, &dStr);
    if (rc != JSI_OK)
        return rc;
    Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return JSI_OK;
}*/
#define MAX_SUB_MATCHES 50

static int StringReplaceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    /* Now handles perl regex flag extensions.*/
    const char *source_str;
    int source_len;
    const char *replace_str;
    int replace_len;
    int regexec_flags = 0;
    Jsi_Value *seq;
    Jsi_DString dStr = {};
    regex_t *regex;
    Jsi_Regex *re;
    const char *p;
    int skip = 0;

    int offset = 0, n, j, opt_all = 0, num_matches = 0;
    /* Is a generic  String.replace if _this->d.obj is a function */
    ChkString(_this, funcPtr, source_str, &source_len);
    
    seq = Jsi_ValueArrayIndex(interp, args, skip);

    replace_str = Jsi_ValueArrayIndexToStr(interp, args, 1+skip, &replace_len);
    Jsi_DSInit(&dStr);

    if (Jsi_ValueIsString(interp, seq)) {
        const char *ce, *cp = Jsi_ValueString(interp, seq, NULL);
        if (!(ce = strstr(source_str, cp)))
            Jsi_ValueMakeStringDup(interp, ret, source_str);
        else {
            int slen;
            slen = (ce-source_str);
            if (slen)
                Jsi_DSAppendLen(&dStr, source_str, slen);
            Jsi_DSAppendLen(&dStr, replace_str, replace_len);
            Jsi_DSAppend(&dStr, ce+strlen(cp), NULL);
            Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&dStr));
            Jsi_DSFree(&dStr);
        }
        return JSI_OK;
    }
    if (seq == NULL || seq->vt != JSI_VT_OBJECT || seq->d.obj->ot != JSI_OT_REGEXP) {
        Jsi_ValueMakeNull(interp, ret);
        return JSI_OK;
    }

    re = seq->d.obj->d.robj;
    regex = &re->reg;
    opt_all = (re->eflags & JSI_REG_GLOB);
    
    regmatch_t pmatch[MAX_SUBREGEX];
    memset(&pmatch, 0, MAX_SUBREGEX * sizeof(regmatch_t));
    /* If an offset has been specified, adjust for that now.
     * If it points past the end of the string, point to the terminating null
     */
    if (offset) {
        if (offset < 0) {
            offset += source_len + 1;
        }
        if (offset > source_len) {
            offset = source_len;
        }
        else if (offset < 0) {
            offset = 0;
        }
    }

    Jsi_DSAppendLen(&dStr, source_str, offset);
    n = source_len - offset;
    p = source_str + offset;
    int rc = JSI_OK;
    do {
        if (num_matches > 10000000) {
            Jsi_LogBug("regexp infinite loop");
            rc = JSI_ERROR;
            break;
        }
        int match = regexec(regex, p, MAX_SUBREGEX, pmatch, regexec_flags);

        if (match >= REG_BADPAT) {
            char buf[100];

            regerror(match, regex, buf, sizeof(buf));
            Jsi_LogError("error while matching pattern: %s", buf);
            Jsi_DSFree(&dStr);
            return JSI_ERROR;
        }
        if (match == REG_NOMATCH) {
            break;
        }
        num_matches++;
        Jsi_DSAppendLen(&dStr, p, pmatch[0].rm_so);
        
        for (j = 0; j < replace_len; j++) {
            int idx;
            int c = replace_str[j];

            if (c == '&') {
                idx = 0;
            }
            else if (c == '\\' && j < replace_len) {
                c = replace_str[++j];
                if ((c >= '0') && (c <= '9')) {
                    idx = c - '0';
                }
                else if ((c == '\\') || (c == '&')) {
                    Jsi_DSAppendLen(&dStr, replace_str + j, 1);
                    continue;
                }
                else {
                    Jsi_DSAppendLen(&dStr, replace_str + j - 1, 2);
                    continue;
                }
            }
            else {
                Jsi_DSAppendLen(&dStr, replace_str + j, 1);
                continue;
            }
            if ((idx < MAX_SUBREGEX) && pmatch[idx].rm_so != -1 && pmatch[idx].rm_eo != -1) {
                Jsi_DSAppendLen(&dStr, p + pmatch[idx].rm_so,
                    pmatch[idx].rm_eo - pmatch[idx].rm_so);
            }
        }

        p += pmatch[0].rm_eo;
        n -= pmatch[0].rm_eo;
        /* If -all is not specified, or there is no source left, we are done */
        if (!opt_all || n == 0 || pmatch[0].rm_eo == 0) {
            break;
        }
        /* An anchored pattern without -line must be done */
        if ((re->eflags & JSI_REG_NEWLINE) == 0 && re->pattern[0] == '^') {
            break;
        }
        
        /* If the pattern is empty, need to step forwards */
        if (re->pattern[0] == '\0' && n) {
            /* Need to copy the char we are moving over */
            Jsi_DSAppendLen(&dStr, p, 1);
            p++;
            n--;
        }

        regexec_flags |= REG_NOTBOL;
    } while (n);

    /*
     * Copy the portion of the string after the last match to the
     * result variable.
     */
    Jsi_DSAppend(&dStr, p, NULL);

    Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    
    return rc;

}

static int StringSearchCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{

    int skip = 0;
    char *source_str;
    ChkString(_this, funcPtr, source_str, NULL);
    //int source_len = Jsi_Strlen(source_str);
    
    char *v = _this->d.obj->d.s.str;
    Jsi_Value *seq = Jsi_ValueArrayIndex(interp, args, skip);

    if (Jsi_ValueIsString(interp, seq)) {
        char *ce, *cp = Jsi_ValueString(interp, seq, NULL);
        int n = -1;
        if ((ce = strstr(source_str, cp))) {
            n = (ce-source_str);
        }
        Jsi_ValueMakeNumber(interp, ret, n);
        return JSI_OK;
    }
    if (!seq || seq->vt != JSI_VT_OBJECT || seq->d.obj->ot != JSI_OT_REGEXP) {
        Jsi_ValueMakeNumber(interp, ret, -1);
        return JSI_OK;
    }

    regex_t *reg = &seq->d.obj->d.robj->reg;
    
    regmatch_t pos[MAX_SUBREGEX];
    memset(&pos, 0, MAX_SUBREGEX * sizeof(regmatch_t));
    int r;
    if ((r = regexec(reg, v, MAX_SUBREGEX, pos, 0)) != 0) {
        if (r == REG_NOMATCH) {
            Jsi_ValueMakeNumber(interp, ret, -1.0);
            return JSI_OK;
        }
        if (r >= REG_BADPAT) {
            char buf[100];

            regerror(r, reg, buf, sizeof(buf));
            Jsi_LogError("error while matching pattern: %s", buf);
            return JSI_ERROR;
        }

    }
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)pos[0].rm_so);
    return JSI_OK;
}

/* UNIMPL: 'toLocaleLowerCase', 'toLocaleUpperCase', 'localeCompare', */

static Jsi_CmdSpec stringCmds[] = {
    { "String",     StringConstructor,      0,  1,  "str", JSI_CMD_IS_CONSTRUCTOR, .help="String constructor", .retType=(uint)JSI_TT_STRING },
/*    { "localeCompare",StringLocaleCompareCmd,1, 1, "pos" },*/
    { "charAt",     StringCharAtCmd,        1, 1, "index:number", .help="Return char at index", .retType=(uint)JSI_TT_STRING},
    { "charCodeAt", StringCharCodeAtCmd,    1, 1, "index:number", .help="Return char code at index", .retType=(uint)JSI_TT_NUMBER },
    { "fromCharCode",StringFromCharCodeCmd, 0,-1, "code:string, ...", JSI_CMDSPEC_NONTHIS, .help="Return char its code number", .retType=(uint)JSI_TT_STRING},
    { "concat",     StringConcatCmd,        0,-1, "str:string, ...", .help="Append one or more strings", .retType=(uint)JSI_TT_STRING },
    { "indexOf",    StringIndexOfCmd,       1, 2, "str:string, start:number", .help="Return index of char", .retType=(uint)JSI_TT_NUMBER },
    { "lastIndexOf",StringLastIndexOfCmd,   1, 2, "str:string, start:number", .help="Return index of last char", .retType=(uint)JSI_TT_NUMBER },
    { "match",      StringMatchCmd,         1, 1, "pattern:regexp|string", .help="Return array of matches", .retType=(uint)JSI_TT_ARRAY|JSI_TT_NULL },
    { "replace",    StringReplaceCmd,       2, 2, "pattern:regexp|string, replace:string", .help="Return a string after replacement", .retType=(uint)JSI_TT_STRING },
    { "search",     StringSearchCmd,        1, 1, "pattern:regexp|string", .help="Return index of first char matching pattern", .retType=(uint)JSI_TT_NUMBER },
    { "slice",      StringSliceCmd,         1, 2, "start:number, end:number", .help="Return section of string", .retType=(uint)JSI_TT_STRING },
    { "split",      StringSplitCmd,         1, 1, "char:string", .help="Split on char and return Array", .retType=(uint)JSI_TT_ARRAY },
    { "substr",     StringSubstrCmd,        0, 2, "start:number, length:number", .help="Return substring", .retType=(uint)JSI_TT_STRING },
    { "substring",  StringSubstringCmd,     0, 2, "start:number, end:number", .help="Return substring", .retType=(uint)JSI_TT_STRING },
    { "toLocaleLowerCase",StringToLowerCaseCmd,0, 0, "",.help="Lower case", .retType=(uint)JSI_TT_STRING },
    { "toLocaleUpperCase",StringToUpperCaseCmd,0, 0, "",.help="Upper case", .retType=(uint)JSI_TT_STRING },
    { "toLowerCase",StringToLowerCaseCmd,   0, 0, "",.help="Return lower cased string", .retType=(uint)JSI_TT_STRING },
    { "toUpperCase",StringToUpperCaseCmd,   0, 0, "",.help="Return upper cased string", .retType=(uint)JSI_TT_STRING },
    { "toTitle",    StringToTitleCmd,       0, 1, "chars:string",.help="Make first char upper case", .retType=(uint)JSI_TT_STRING },
    { "trim",       StringTrimCmd,          0, 1, "chars:string",.help="Trim chars", .retType=(uint)JSI_TT_STRING },
    { "trimLeft",   StringTrimLeftCmd,      0, 1, "chars:string",.help="Trim chars from left", .retType=(uint)JSI_TT_STRING },
    { "trimRight",  StringTrimRightCmd,     0, 1, "chars:string",.help="Trim chars from right", .retType=(uint)JSI_TT_STRING },
    { NULL, .help="Commands for accessing string objects." }
};

int jsi_StringInit(Jsi_Interp *interp)
{
    interp->String_prototype = Jsi_CommandCreateSpecs(interp, "String", stringCmds, NULL, 0);
    return JSI_OK;
}

#endif
