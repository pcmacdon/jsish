#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
    
#define ChkString(_this, funcPtr, dest, lenPtr, bytePtr) \
    int skip __attribute__((unused)); skip = 0; \
    if (_this->vt != JSI_VT_OBJECT || _this->d.obj->ot != JSI_OT_STRING) { \
        Jsi_LogError("apply String.%s to a non-string object", funcPtr->cmdSpec->name); \
        return JSI_ERROR; \
    } else  { \
        dest = Jsi_ValueString(interp, _this, bytePtr); \
        *lenPtr = Jsi_NumUtfChars(dest, -1); \
        if (!dest) dest = (char*)""; \
    }

static Jsi_RC StringConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        const char *nv = "";
        int len = -1;
        if (Jsi_ValueGetLength(interp, args) > 0) {
            Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
            if (v) {
                
                nv = Jsi_ValueToString(interp, v, &len);
            }
        }
        if (_this->vt == JSI_VT_OBJECT && _this->d.obj->ot == JSI_OT_STRING) {
            if (_this->d.obj->d.s.str)
                if (!_this->d.obj->isstrkey)
                    Jsi_Free(_this->d.obj->d.s.str);
            _this->d.obj->isstrkey = 0;
            _this->d.obj->d.s.str = Jsi_StrdupLen(nv, len);
            _this->d.obj->d.s.len = len;
        } else
            jsi_ValueMakeBlobDup(interp, &_this, (uchar*)nv, len);
        Jsi_ValueDup2(interp, ret, _this);
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


static Jsi_RC StringSplitCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, //TODO: UTF
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char **argv; int argc;
    int sLen, bLen, spLen = 1, noEmpty=0;
    Jsi_RC rc = JSI_OK;
    const char *v, *split = "";
    
    ChkString(_this, funcPtr, v, &sLen, &bLen);

    Jsi_Value *spliton = Jsi_ValueArrayIndex(interp, args, skip);
    
    if (spliton) {
        if (Jsi_ValueIsNull(interp, spliton))
            noEmpty = 1;
        else if (Jsi_ValueIsString(interp, spliton))
            split = Jsi_ValueString(interp, spliton, &spLen);
    }

    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    if (spLen)
        Jsi_SplitStr(v, &argc, &argv, split, &sStr);
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    int i, n = 0, siz = argc;
    if (noEmpty) {
        for (i=0, siz=0; i<argc; i++) if (argv[i][0]) siz++;
    } else if (!spLen)
        argc = siz = bLen;
    if (Jsi_ObjArraySizer(interp, obj, siz) <= 0) {
        rc = Jsi_LogError("index too large: %d", siz);
        Jsi_ObjFree(interp, obj);
        goto bail;
    }
    Jsi_ValueMakeArrayObject(interp, ret, obj);
    for (i = 0; i < argc; ++i) {
        if (noEmpty && !argv[i][0]) continue;
        char ctmp[2] = " ",  *cst = ctmp;
        if (!spLen)
            ctmp[0] = v[i];
        else
            cst = argv[i];
        Jsi_Value *v = Jsi_ValueNewStringDup(interp, cst);
        Jsi_IncrRefCount(interp, v);
        obj->arr[n++] = v;
    }
    Jsi_ObjSetLength(interp, obj, siz);
    
bail:
    Jsi_DSFree(&sStr);
    return rc;
}

static Jsi_RC StringSubstrCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *v;
    ChkString(_this, funcPtr, v, &sLen, &bLen);

    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, skip);
    Jsi_Value *len = Jsi_ValueArrayIndex(interp, args, skip+1);
    
    Jsi_Number nstart, nlen;
    if (!start || Jsi_GetNumberFromValue(interp,start, &nstart) != JSI_OK) {
        jsi_ValueMakeBlobDup(interp, ret, (uchar*)v, bLen);
        return JSI_OK;
    }
    int istart = (int)nstart, olen = -1;
    Jsi_DString dStr;
    char *ostr;
    Jsi_DSInit(&dStr);
    if (!len || Jsi_GetNumberFromValue(interp,len, &nlen) != JSI_OK) {
        if (sLen == bLen) {
            ostr = jsi_SubstrDup(v, bLen, istart, -1, &olen);
            Jsi_ValueMakeBlob(interp, ret, (uchar*)ostr, olen);
        } else {
            Jsi_UtfSubstr(v, istart, -1, &dStr);
            Jsi_ValueFromDS(interp, &dStr, ret);
        }
        return JSI_OK;
    }
    int ilen = (int)nlen;
    if (ilen <= 0) {
        Jsi_ValueMakeStringDup(interp, ret, "");
    } else {
        if (sLen == bLen) {
            ostr = jsi_SubstrDup(v, bLen, istart, ilen, &olen);
            Jsi_ValueMakeBlob(interp, ret, (uchar*)ostr, olen);
        } else {
            Jsi_UtfSubstr(v, istart, ilen, &dStr);
            Jsi_ValueFromDS(interp, &dStr, ret);
        }
    }
    return JSI_OK;
}

static Jsi_RC StringSubstringCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen = 0, bLen;
    const char *v;
    ChkString(_this, funcPtr, v, &sLen, &bLen);

    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, skip);
    Jsi_Value *end = Jsi_ValueArrayIndex(interp, args, skip+1);
    
    Jsi_Number nstart, nend;
    if (!start || Jsi_GetNumberFromValue(interp,start, &nstart) != JSI_OK) {
        Jsi_ValueMakeStringDup(interp, ret, v);
        return JSI_OK;
    }
    int istart = (int)nstart, olen = -1;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    char *ostr;
    if (!end || Jsi_GetNumberFromValue(interp,end, &nend) != JSI_OK) {
        if (sLen == bLen) {
            ostr = jsi_SubstrDup(v, bLen, istart, -1, &olen);
            Jsi_ValueMakeBlob(interp, ret, (uchar*)ostr, olen);
        } else {
            Jsi_UtfSubstr(v, istart, -1, &dStr);
            Jsi_ValueFromDS(interp, &dStr, ret);
        }
        return JSI_OK;
    }
    int iend = (int)nend;
    if (iend>sLen)
        iend = sLen;
    if (iend < istart) {
        Jsi_ValueMakeStringDup(interp, ret, "");
    } else {
        if (sLen == bLen) {
            ostr = jsi_SubstrDup(v, bLen, istart, iend-istart+1, &olen);
            Jsi_ValueMakeBlob(interp, ret, (uchar*)ostr, olen);
        } else {
            Jsi_UtfSubstr(v, istart, iend-istart+1, &dStr);
            Jsi_ValueFromDS(interp, &dStr, ret);
        }
    }
    return JSI_OK;
}


static Jsi_RC StringRepeatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *v;
    ChkString(_this, funcPtr, v, &sLen, &bLen);
    
    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, skip);
    
    Jsi_Number ncnt;
    if (!start || Jsi_GetNumberFromValue(interp,start, &ncnt) != JSI_OK || ncnt>MAX_LOOP_COUNT || ncnt<0 || bLen<=0) {
        return JSI_ERROR;
    }
    int cnt = (int)ncnt;
    Jsi_DString dStr = {};
    while (cnt-- > 0) {
        Jsi_DSAppendLen(&dStr, v, bLen);
        if (Jsi_DSLength(&dStr)>JSI_MAX_ALLOC_BUF) {
            Jsi_DSFree(&dStr);
            return Jsi_LogError("too long");
        }
    }
    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
}

static Jsi_RC StringIndexOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *v;
    ChkString(_this, funcPtr, v, &sLen, &bLen);

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

static Jsi_RC StringMatchCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *source_str;
    ChkString(_this, funcPtr, source_str, &sLen, &bLen);
    const char *v = source_str;
    Jsi_Value *seq = Jsi_ValueArrayIndex(interp, args, skip);

    if (Jsi_ValueIsString(interp, seq)) {
        char *cp = Jsi_ValueString(interp, seq, NULL);

        if (jsi_RegExpValueNew(interp, cp, seq) != JSI_OK)
            return JSI_ERROR;
    }
    /* TODO: differentiate from re.exec() */
    return jsi_RegExpMatches(interp, seq, v, bLen, *ret, NULL, 1);
}

static Jsi_RC StringFromCharCodeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (_this->vt != JSI_VT_OBJECT || _this->d.obj->ot == JSI_OT_STRING)
        return Jsi_LogError("should be called via String.fromCharCode");
    
    Jsi_DString dStr = {};
    int n, i, len, argc = Jsi_ValueGetLength(interp, args);
    for (i=0; i<argc; i++) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, i);
        if (!Jsi_ValueIsNumber(interp, v) || Jsi_GetIntFromValue(interp, v, &n) != JSI_OK) {
            Jsi_DSFree(&dStr);
            return Jsi_LogError("expected int value at arg %d", i+1);
        }
        char dest[5];
        len = Jsi_UniCharToUtf((Jsi_UniChar)n, dest);
        Jsi_DSAppendLen(&dStr, dest, len);
    }
    
    Jsi_ValueMakeDStringObject(interp, ret, &dStr);
    return JSI_OK;
}

static Jsi_RC StringCharCodeAtCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *source_str;
    ChkString(_this, funcPtr, source_str, &sLen, &bLen);
    const char *v = source_str;

    Jsi_Value *ttPtr = Jsi_ValueNew1(interp);
    Jsi_ValueCopy(interp, ttPtr, _this);
    
    int pos = 0;
    Jsi_Value *vpos;
    if ((vpos = Jsi_ValueArrayIndex(interp, args, skip))) {
        jsi_ValueToOInt32(interp, vpos);
        pos = (int)vpos->d.num;
    }

#if JSI__UTF8
    int m;
    if (pos >= 0 && pos < sLen && ((m=Jsi_UtfIndexToOffset(v, pos))>=0)) {
        int32_t n;
        Jsi_UtfToUniChar(v+m, &n);
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)n);
    }
#else
    int slen = Jsi_Strlen(v);
    if (pos >= 0 && pos < slen) {
        Jsi_ValueMakeNumber(interp, ret, v[pos]);
    }
#endif
    else
        Jsi_ValueMakeNumber(interp, ret, Jsi_NumberNaN());
    Jsi_DecrRefCount(interp, ttPtr);
    return JSI_OK;
}


static Jsi_RC _StringTrimCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, // TODO: UTF
    Jsi_Value **ret, Jsi_Func *funcPtr, int ends)
{
    const char *tstr = " \t\n\r", *vstr;
    int vend,  n, tlen = Jsi_Strlen(tstr), clen, bLen;
    ChkString(_this, funcPtr, vstr, &clen, &bLen);
    
    Jsi_Value *tchars = Jsi_ValueArrayIndex(interp, args, skip);
    
    if (tchars) {
        tstr = Jsi_ValueToString(interp, tchars, NULL);
        tlen = Jsi_Strlen(tstr);
    }
    
    if (ends&1) {
        while (*vstr) {
            for (n=0; n<tlen; n++)
                if (tstr[n] == *vstr) break;
            if (n>=tlen) break;
            vstr++;
            clen--;
        }
    }
    vend = clen-1;
    if (ends&2) {
        for (; vend>=0; vend--) {
            for (n=0; n<tlen; n++)
                if (tstr[n] == vstr[vend]) break;
            if (n>=tlen) break;
        }
    }
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_UtfSubstr(vstr, 0, vend+1, &dStr);
    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
}

static Jsi_RC StringTrimCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _StringTrimCmd(interp, args, _this, ret, funcPtr, 3);
}
static Jsi_RC StringTrimLeftCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _StringTrimCmd(interp, args, _this, ret, funcPtr, 1);
}

static Jsi_RC StringTrimRightCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _StringTrimCmd(interp, args, _this, ret, funcPtr, 2);
}


char *jsi_utf_tocase(const char *cp, int upper, Jsi_DString *dsPtr)
{
    char unibuf[10];
    while (*cp) {
#if JSI__UTF8
        if (*cp&0x80) {
            int32_t c;
            Jsi_UtfToUniChar(cp, &c);
            int n = Jsi_UniCharToUtf(c, unibuf);
            unibuf[n] = 0;
            cp += n;
        } else
#endif
        {
            unibuf[0] = (upper?toupper(*cp):tolower(*cp));
            unibuf[1] = 0;
            cp++;
        }
        if (upper==2) //totile
            upper = 0;
        Jsi_DSAppend(dsPtr, unibuf, NULL);
    }
    return Jsi_DSValue(dsPtr);
}



static Jsi_RC StringToLowerCaseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *vstr;
    ChkString(_this, funcPtr, vstr, &sLen, &bLen);
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    jsi_utf_tocase(vstr, 0, &dStr);
    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
}

static Jsi_RC StringToUpperCaseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *vstr;
    ChkString(_this, funcPtr, vstr, &sLen, &bLen);

    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    jsi_utf_tocase(vstr, 1, &dStr);
    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
}

static Jsi_RC StringToTitleCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *vstr;
    ChkString(_this, funcPtr, vstr, &sLen, &bLen);
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    jsi_utf_tocase(vstr, 2, &dStr);
    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
}

static Jsi_RC StringCharAtCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int pos = 0, len, bLen;
    const char *vstr;
    ChkString(_this, funcPtr, vstr, &len, &bLen);
   
    Jsi_Value *vpos = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_GetIntFromValue(interp, vpos, &pos)) {
        return JSI_ERROR;        
    }
    if (pos<0 || pos >=len)
        Jsi_ValueMakeStringDup(interp, ret, "");
    else {
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        Jsi_UtfSubstr(vstr, pos, 1, &dStr);
        Jsi_ValueFromDS(interp, &dStr, ret);
    }
    return JSI_OK;
}

static Jsi_RC StringLastIndexOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *v;
    ChkString(_this, funcPtr, v, &sLen, &bLen);

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

static Jsi_RC StringConcatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *vstr;
    ChkString(_this, funcPtr, vstr, &sLen, &bLen);
    
    Jsi_DString dStr = {};
    Jsi_DSAppend(&dStr, vstr, NULL);
    int i, argc = Jsi_ValueGetLength(interp, args);
    for (i=skip; i<argc; i++)
    {
        Jsi_Value *s = Jsi_ValueArrayIndex(interp, args, i);
        if (Jsi_GetStringFromValue(interp, s, &vstr)) {
            Jsi_LogError("String get failure");
            Jsi_DSFree(&dStr);
            return JSI_ERROR;
        }
        Jsi_DSAppend(&dStr, vstr, NULL);
    }

    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
    
}

static Jsi_RC StringSliceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int sLen, bLen;
    const char *vstr;
    ChkString(_this, funcPtr, vstr, &sLen, &bLen);

    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, skip);
    Jsi_Value *end = Jsi_ValueArrayIndex(interp, args, 1+skip);
    int istart, iend, len = Jsi_Strlen(vstr);

    if (Jsi_GetIntFromValue(interp, start, &istart)) {
        return JSI_ERROR;
    }
    if (istart < 0)
        istart = len+istart;
    if (istart < 0)
        istart = 0;
    if (istart>=len)
        istart = len-1;
    iend = len;
    if (end) {
        if (Jsi_GetIntFromValue(interp, end, &iend)) {
            return JSI_ERROR;
        }
        if (iend < 0)
            iend = len+iend;
        if (iend>=len)
            iend = len;
        if (iend<istart)
            iend = istart;
    }
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_UtfSubstr(vstr, istart, iend, &dStr);
    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
}

static Jsi_RC StringMapCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    /* Now handles perl regex flag extensions.*/
    const char *source_str;
    const char *replace_str = NULL;
    uint i, j;
    int slen, source_len, replace_len, bLen;

    const char *p;
    bool nocase = 0;
    /* Is a generic  String.replace if _this->d.obj is a function */
    ChkString(_this, funcPtr, source_str, &source_len, &bLen);
    int argc = Jsi_ValueGetLength(interp, args);
    Jsi_Value *repVal = Jsi_ValueArrayIndex(interp, args, skip);
    if (Jsi_ValueIsArray(interp, repVal)==0 || repVal->d.obj->arrCnt&1) 
        return Jsi_LogError("expected even length array");
    if (argc>(skip+1) && Jsi_ValueGetBoolean(interp, Jsi_ValueArrayIndex(interp, args, skip+1), &nocase) != JSI_OK) 
        return Jsi_LogError("expected boolean");
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_Obj *obj = repVal->d.obj;
    p = source_str;
    for (j=0; j<(uint)bLen; j++, p++) {
        for (i=0; i<obj->arrCnt; i+=2) {
            if (!obj->arr[i]) continue;
            if (!obj->arr[i+1]) continue;
            const char *cp = Jsi_ValueToString(interp, obj->arr[i], &slen);
            if (!cp || !slen) {
                Jsi_DSFree(&dStr);
                return Jsi_LogError("map src can not be empty");
            }
            int res = (nocase ? Jsi_Strncasecmp(cp, p, slen) : Jsi_Strncmp(cp, p, slen));
            if (!res) {
                replace_str = Jsi_ValueToString(interp, obj->arr[i+1], &replace_len);
                Jsi_DSAppendLen(&dStr, replace_str, replace_len);
                p += slen-1;
                j += slen-1;
                break;
            }
        }
        if (i>=obj->arrCnt)
            Jsi_DSAppendLen(&dStr, p, 1);
    }

    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
}
    
#define FN_strreplace JSI_INFO("\
If the replace argument is a function, it is called with match,p1,p2,...,offset,string.  \
If called function is known to have 1 argument, it is called with just the match.")
static Jsi_RC StringReplaceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    /* Now handles perl regex flag extensions.*/
    const char *source_str;
    int source_len, bLen;
    const char *replace_str = NULL;
    int replace_len;
    int regexec_flags = 0;
    Jsi_Value *seq, *strVal;
    Jsi_DString dStr = {};
    regex_t *regex;
    Jsi_Regex *re;
    const char *p;
    int maxArgs = 1;

    int offset = 0, n, j, isglob = 0, num_matches = 0;
    /* Is a generic  String.replace if _this->d.obj is a function */
    ChkString(_this, funcPtr, source_str, &source_len, &bLen);
    source_len = bLen;
    if (!skip)
        strVal = _this;
    else
        strVal = Jsi_ValueArrayIndex(interp, args, 0);
    seq = Jsi_ValueArrayIndex(interp, args, skip);
    Jsi_Value *repVal = Jsi_ValueArrayIndex(interp, args, 1+skip);
    if (!Jsi_ValueIsFunction(interp, repVal))
        replace_str = Jsi_ValueToString(interp, repVal, &replace_len);
    else
        maxArgs = repVal->d.obj->d.fobj->func->argnames->argCnt;
    Jsi_DSInit(&dStr);

    if (Jsi_ValueIsString(interp, seq)) {
        const char *ce, *cp = Jsi_ValueString(interp, seq, NULL);
        if (!(ce = Jsi_Strstr(source_str, cp)))
            Jsi_ValueMakeStringDup(interp, ret, source_str);
        else {
            int slen;
            slen = (ce-source_str);
            if (slen)
                Jsi_DSAppendLen(&dStr, source_str, slen);
            if (replace_str)
                Jsi_DSAppendLen(&dStr, replace_str, replace_len);
            else {
                Jsi_Value *inStr = Jsi_ValueNewStringDup(interp, source_str);
                Jsi_IncrRefCount(interp, inStr);
                Jsi_RC rc = Jsi_FunctionInvokeString(interp, repVal, inStr, &dStr);
                if (Jsi_InterpGone(interp))
                    return JSI_ERROR;
                if (rc != JSI_OK) {
                    Jsi_DSFree(&dStr);
                    Jsi_DecrRefCount(interp, inStr);
                    return JSI_ERROR;
                }
                Jsi_DecrRefCount(interp, inStr);
            }
            Jsi_DSAppend(&dStr, ce+Jsi_Strlen(cp), NULL);
            Jsi_ValueFromDS(interp, &dStr, ret);
        }
        return JSI_OK;
    }
    if (seq == NULL || seq->vt != JSI_VT_OBJECT || seq->d.obj->ot != JSI_OT_REGEXP) {
        Jsi_ValueMakeNull(interp, ret);
        return JSI_OK;
    }

    re = seq->d.obj->d.robj;
    regex = &re->reg;
    isglob = (re->eflags & JSI_REG_GLOB);
    
    regmatch_t pmatch[MAX_SUBREGEX] = {};
    /* If an offset has been specified, adjust for that now.
     * If it points past the end of the string, point to the terminating null
     */
    int eoffset=0;
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
    Jsi_RC rc = JSI_OK;
    do {
        if (num_matches > 10000000) {
            Jsi_LogBug("regexp infinite loop");
            rc = JSI_ERROR;
            break;
        }
        int match = regexec(regex, p, MAX_SUBREGEX, pmatch, regexec_flags);

        if (match >= REG_BADPAT) {
            char buf[JSI_MAX_NUMBER_STRING];

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

        if (replace_str &&  !Jsi_Strchr(replace_str, '$'))
            Jsi_DSAppend(&dStr, replace_str, NULL);
        else if (replace_str) {
            for (j = 0; j < replace_len; j++) {
                int idx;
                int c = replace_str[j];
     
                if (c == '$' && j < replace_len) {
                    c = replace_str[++j];
                    if ((c >= '0') && (c <= '9')) {
                        idx = c - '0';
                    } else if (c == '&') {
                        idx = 0;
                    } else if (c == '$') {
                        Jsi_DSAppendLen(&dStr, replace_str + j, 1);
                        continue;
                    }
                    else if (c == '\'') {
                        Jsi_DSAppendLen(&dStr, p + pmatch[0].rm_eo, pmatch[0].rm_eo-Jsi_Strlen(p));
                        continue;
                    }
                    else if (c == '`') {
                        Jsi_DSAppendLen(&dStr, p, pmatch[0].rm_so);
                        continue;
                    }
                    else {
                        Jsi_DSAppendLen(&dStr, replace_str + j - 1, 2);
                        continue;
                    }
                } else {
                     Jsi_DSAppendLen(&dStr, replace_str + j, 1);
                     continue;
                }
                if ((idx < MAX_SUBREGEX) && pmatch[idx].rm_so != -1 && pmatch[idx].rm_eo != -1) {
                    Jsi_DSAppendLen(&dStr, p + pmatch[idx].rm_so,
                        pmatch[idx].rm_eo - pmatch[idx].rm_so);
                }
            }
    
        } else {
            Jsi_DString sStr;
            Jsi_DSInit(&sStr); 
            if (pmatch[0].rm_so <= 0 && pmatch[0].rm_eo <= 0)
                break;
            int olen = -1;
            char *ostr = jsi_SubstrDup(p, -1, pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so, &olen);
            Jsi_Value *inStr = Jsi_ValueMakeBlob(interp, NULL, (uchar*)ostr, olen);
            Jsi_DSFree(&sStr);
            Jsi_IncrRefCount(interp, inStr);
            if (maxArgs==1) {
                Jsi_RC rc = Jsi_FunctionInvokeString(interp, repVal, inStr, &dStr);
                if (Jsi_InterpGone(interp))
                    return JSI_ERROR;
                if (rc != JSI_OK) {
                    Jsi_DSFree(&dStr);
                    Jsi_DecrRefCount(interp, inStr);
                    return JSI_ERROR;
                }
            } else {
                Jsi_Value *vpargs, *items[MAX_SUBREGEX] = {}, *ret;
                int i;
                items[0] = inStr;
                for (i=1; i<=(int)re->reg.re_nsub && i<(MAX_SUBREGEX-3); i++) {
                    if (pmatch[i].rm_so<0)
                        items[i] = interp->NullValue;
                    else {
                        ostr = jsi_SubstrDup(p, -1, pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so, &olen);
                        items[i] = Jsi_ValueMakeBlob(interp, NULL, (uchar*)ostr, olen);
                    }
                }
                items[i++] = Jsi_ValueMakeNumber(interp, NULL, eoffset+pmatch[0].rm_so);
                items[i++] = strVal;
                vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, i, 0));
                Jsi_IncrRefCount(interp, vpargs);
                ret = Jsi_ValueNew1(interp);
                rc = Jsi_FunctionInvoke(interp, repVal, vpargs, &ret, NULL);
                if (Jsi_InterpGone(interp))
                    return JSI_ERROR;
                Jsi_DecrRefCount(interp, vpargs);
                if (rc == JSI_OK)
                    Jsi_DSAppend(&dStr, Jsi_ValueToString(interp, ret, NULL), NULL);
                Jsi_DecrRefCount(interp, ret);
                if (rc != JSI_OK) {
                    Jsi_DSFree(&dStr);
                    Jsi_DecrRefCount(interp, inStr);
                    return JSI_ERROR;
                }
            }
            Jsi_DecrRefCount(interp, inStr);
        }
        eoffset += pmatch[0].rm_eo;
        p += pmatch[0].rm_eo;
        n -= pmatch[0].rm_eo;
        /* If -all is not specified, or there is no source left, we are done */
        if (!isglob || n == 0 || pmatch[0].rm_eo == 0) {
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
    Jsi_ValueFromDS(interp, &dStr, ret);
    return rc;

}

static Jsi_RC StringSearchCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{

    int sLen, bLen;
    const char *source_str;
    ChkString(_this, funcPtr, source_str, &sLen, &bLen);
    
    char *v = _this->d.obj->d.s.str;
    Jsi_Value *seq = Jsi_ValueArrayIndex(interp, args, skip);

    if (Jsi_ValueIsString(interp, seq)) {
        char *ce, *cp = Jsi_ValueString(interp, seq, NULL);
        int n = -1;
        if ((ce = Jsi_Strstr(source_str, cp))) {
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
    
    regmatch_t pos[MAX_SUBREGEX] = {};
    int r;
    if ((r = regexec(reg, v, MAX_SUBREGEX, pos, 0)) != 0) {
        if (r == REG_NOMATCH) {
            Jsi_ValueMakeNumber(interp, ret, -1.0);
            return JSI_OK;
        }
        if (r >= REG_BADPAT) {
            char buf[JSI_MAX_NUMBER_STRING];

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
    { "String",     StringConstructor,      0,  1,  "str", .help="String constructor", .retType=(uint)JSI_TT_STRING, .flags=JSI_CMD_IS_CONSTRUCTOR },
/*    { "localeCompare",StringLocaleCompareCmd,1, 1, "pos" },*/
    { "charAt",     StringCharAtCmd,        1, 1, "index:number", .help="Return char at index", .retType=(uint)JSI_TT_STRING},
    { "charCodeAt", StringCharCodeAtCmd,    1, 1, "index:number", .help="Return char code at index", .retType=(uint)JSI_TT_NUMBER },
    { "concat",     StringConcatCmd,        0,-1, "str:string, ...", .help="Append one or more strings", .retType=(uint)JSI_TT_STRING },
    { "fromCharCode",StringFromCharCodeCmd, 0, -1, "...", .help="Return string for char codes", .retType=(uint)JSI_TT_STRING },
    { "indexOf",    StringIndexOfCmd,       1, 2, "str:string, start:number", .help="Return index of char", .retType=(uint)JSI_TT_NUMBER },
    { "lastIndexOf",StringLastIndexOfCmd,   1, 2, "str:string, start:number", .help="Return index of last char", .retType=(uint)JSI_TT_NUMBER },
    { "match",      StringMatchCmd,         1, 1, "pattern:regexp|string", .help="Return array of matches", .retType=(uint)JSI_TT_ARRAY|JSI_TT_NULL },
    { "map",        StringMapCmd,           1, 2, "strMap:array, nocase:boolean=false", .help="Replaces characters in string based on the key-value pairs in strMap", .retType=(uint)JSI_TT_STRING },
    { "repeat",     StringRepeatCmd,        1, 1, "count:number", .help="Return count copies of string", .retType=(uint)JSI_TT_STRING, .flags=0, .info=0 },
    { "replace",    StringReplaceCmd,       2, 2, "pattern:regexp|string, replace:string|function", .help="Regex/string replacement", .retType=(uint)JSI_TT_STRING, .flags=0, .info=FN_strreplace },
    { "search",     StringSearchCmd,        1, 1, "pattern:regexp|string", .help="Return index of first char matching pattern", .retType=(uint)JSI_TT_NUMBER },
    { "slice",      StringSliceCmd,         1, 2, "start:number, end:number", .help="Return section of string", .retType=(uint)JSI_TT_STRING },
    { "split",      StringSplitCmd,         0, 1, "char:string|null=void", .help="Split on char and return Array. When char is omitted splits on bytes.  When char==null splits on whitespace and removes empty elements", .retType=(uint)JSI_TT_ARRAY },
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
    { NULL, 0,0,0,0, .help="Commands for accessing string objects." }
};

Jsi_RC jsi_InitString(Jsi_Interp *interp, int release)
{
    if (!release)
        interp->String_prototype = Jsi_CommandCreateSpecs(interp, "String", stringCmds, NULL, JSI_CMDSPEC_ISOBJ);
    return JSI_OK;
}

#endif
