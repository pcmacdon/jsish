#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
/* TODO: handle delete */

#define ChkRegexp(_this, funcPtr, dest) \
    if (_this->vt == JSI_VT_OBJECT && _this->d.obj->ot == JSI_OT_FUNCTION &&  \
       _this->d.obj->__proto__ == interp->RegExp_prototype->d.obj->__proto__ ) { \
        skip = 1; \
        dest = Jsi_ValueArrayIndex(interp, args, 0); \
    } else if (_this->vt != JSI_VT_OBJECT || _this->d.obj->ot != JSI_OT_REGEXP) { \
        Jsi_LogError("apply Regexp.%s to a non-regex object\n", funcPtr->cmdSpec->name); \
        return JSI_ERROR; \
    } else  { \
        dest = _this; \
    }

void Jsi_RegExpFree(Jsi_Regex* re) {
    regfree(&re->reg);
    Jsi_Free(re);
}

Jsi_Regex* Jsi_RegExpNew(Jsi_Interp *interp, const char *regtxt, int eflag)
{
    int isNew;
    Jsi_HashEntry *hPtr;
    int flag = REG_EXTENDED;
    char c, *cm, *ce;
    const char *cp;
    Jsi_Regex *re;
    eflag |= JSI_REG_STATIC;
    if (!regtxt[0])
        return NULL;
    hPtr = Jsi_HashEntryFind(interp->regexpTbl, regtxt);
    if (hPtr) {
        re = (Jsi_Regex*)Jsi_HashValueGet(hPtr);
        if (JSI_REG_STATIC & eflag)
            re->eflags |= JSI_REG_STATIC;
        return re;
    }
    cp = regtxt+1;
    if (regtxt[0] != '/')
        return NULL;
    ce = strrchr(cp, '/');
    if (ce == cp || !ce)
        return NULL;
    cm = ce + 1;    
    while (*cm) {
        c = *cm++;
        if (c == 'i') flag |= REG_ICASE;
        else if (c == 'g') eflag |= JSI_REG_GLOB;
        else if (c == 'm') { /* PERL NON-STANDARD */
            eflag |= JSI_REG_NEWLINE;
            flag |= REG_NEWLINE;
        }
#ifdef RE_DOT_NEWLINE
        else if (c == 's') { /* PERL NON-STANDARD */
            eflag |= JSI_REG_DOT_NEWLINE;
            flag |= RE_DOT_NEWLINE;
        }
#endif
    }
    *ce = 0;
    regex_t reg;
    if (regcomp(&reg, cp, flag)) {
        *ce++ = '/';
        Jsi_LogError("Invalid regex string '%s'", cp);
        return NULL;
    }
    *ce++ = '/';
    re = (Jsi_Regex*)Jsi_Calloc(1, sizeof(Jsi_Regex));
    assert (re);
    re->reg = reg;
    re->eflags = eflag;
    re->flags = flag;
    hPtr = Jsi_HashEntryNew(interp->regexpTbl, regtxt, &isNew);
    assert(hPtr);
    Jsi_HashValueSet(hPtr, re);
    re->pattern = (char*)Jsi_HashKeyGet(hPtr);
    return re;

}

int jsi_RegExpValueNew(Jsi_Interp *interp, const char *regtxt, Jsi_Value *ret)
{
    
    Jsi_DString dStr = {};
    Jsi_DSAppend(&dStr, "/", regtxt, "/", NULL);
    Jsi_Regex *re = Jsi_RegExpNew(interp, Jsi_DSValue(&dStr), 0);
    Jsi_DSFree(&dStr);
    if (re == NULL)
        return JSI_ERROR;
    Jsi_Obj *o = Jsi_ObjNewType(interp, JSI_OT_REGEXP);
    Jsi_ValueMakeObject(interp, &ret, o);
    ret->d.obj->d.robj = re;
    ret->d.obj->ot = JSI_OT_REGEXP;
    return JSI_OK;
}


static int RegExp_constructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *target;
    
    if (Jsi_FunctionIsConstructor(funcPtr))
        target = _this;
    else {
        Jsi_Obj *o = Jsi_ObjNewType(interp, JSI_OT_REGEXP);
        Jsi_ValueMakeObject(interp, ret, o);
        target = *ret;
    }
    
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    const char *regtxt = "";
    const char *mods = NULL;
    if (v) {
        if (v->vt == JSI_VT_OBJECT && v->d.obj->ot == JSI_OT_REGEXP) {
            Jsi_ValueCopy(interp,target, v);
            return JSI_OK;
        } else if (!(regtxt = Jsi_ValueString(interp, v, NULL))) {
            return JSI_ERROR;
        }
    }
    Jsi_Value *f = Jsi_ValueArrayIndex(interp, args, 1);
    if (f)
        mods = Jsi_ValueString(interp, f, NULL);
    Jsi_DString dStr = {};
    Jsi_DSAppend(&dStr, "/", regtxt, "/", mods, NULL);
    Jsi_Regex *re = Jsi_RegExpNew(interp, Jsi_DSValue(&dStr), 0);
    Jsi_DSFree(&dStr);
    if (re == NULL)
        return JSI_ERROR;
    target->d.obj->d.robj = re;
    target->d.obj->ot = JSI_OT_REGEXP;
    return JSI_OK;
}


int Jsi_RegExpMatch(Jsi_Interp *interp, Jsi_Value *pattern, const char *v, int *rc, Jsi_DString *dStr)
{
    Jsi_Regex *re;
    int regexec_flags = 0;
    if (rc)
        *rc = 0;
    if (pattern == NULL || pattern->vt != JSI_VT_OBJECT || pattern->d.obj->ot != JSI_OT_REGEXP) {
        Jsi_LogError("expected pattern");
        return JSI_ERROR;
    }
    re = pattern->d.obj->d.robj;
    regex_t *reg = &re->reg;
    
    regmatch_t pos;
    memset(&pos, 0,  sizeof(regmatch_t));
    if (dStr)
        Jsi_DSInit(dStr);
        
    int r  = regexec(reg, v, 1, &pos, regexec_flags);

    if (r >= REG_BADPAT) {
        char buf[100];

        regerror(r, reg, buf, sizeof(buf));
        Jsi_LogError("error while matching pattern: %s", buf);
        return JSI_ERROR;
    }
    if (r != REG_NOMATCH) {
        if (rc) *rc = 1;
        if (dStr && pos.rm_so >= 0 && pos.rm_eo >= 0 &&  pos.rm_eo >= pos.rm_so)
            Jsi_DSAppendLen(dStr, v + pos.rm_so, pos.rm_eo - pos.rm_so);
    }
    
    return JSI_OK;
}



int Jsi_RegExpMatches(Jsi_Interp *interp, Jsi_Value *pattern, const char *v, Jsi_Value *ret)
{
    Jsi_Regex *re;
    int regexec_flags = 0;
    Jsi_Value *seq = pattern;

    if (seq == NULL || seq->vt != JSI_VT_OBJECT || seq->d.obj->ot != JSI_OT_REGEXP) {
        Jsi_ValueMakeNull(interp, &ret);
        return JSI_OK;
    }
    re = seq->d.obj->d.robj;
    regex_t *reg = &re->reg;
    
    regmatch_t pos[MAX_SUBREGEX];
    memset(&pos, 0, MAX_SUBREGEX * sizeof(regmatch_t));
    int num_matches = 0, r, n = strlen(v);
    Jsi_Obj *obj;
    
    do {
        r = regexec(reg, v, MAX_SUBREGEX, pos, regexec_flags);

        if (r >= REG_BADPAT) {
            char buf[BUFSIZ];

            regerror(r, reg, buf, sizeof(buf));
            Jsi_LogError("error while matching pattern: %s", buf);
            return JSI_ERROR;
        }
        if (r == REG_NOMATCH) {
            if (num_matches == 0) {
                Jsi_ValueMakeNull(interp, &ret);
                return JSI_OK;
            }
            break;
        }

        if (num_matches == 0) {
            obj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
            obj->__proto__ = interp->Array_prototype;
            Jsi_ValueMakeObject(interp, &ret, obj);
            Jsi_ObjSetLength(interp, ret->d.obj, 0);
        }
    
        int i;
        for (i = 0; i < MAX_SUBREGEX; ++i) {
            if (pos[i].rm_so <= 0 && pos[i].rm_eo <= 0)
                break;
            if (i && pos[i].rm_so == pos[i-1].rm_so && pos[i].rm_eo == pos[i-1].rm_eo)
                continue;
    
            Jsi_Value *val = Jsi_ValueMakeString(interp, NULL, 
                jsi_SubstrDup(v, pos[i].rm_so, pos[i].rm_eo - pos[i].rm_so));
            jsi_ValueInsertArray(interp, ret, num_matches, val, 0);
            num_matches++;
        }
        if (num_matches == 1 && (!(re->eflags&JSI_REG_GLOB)))
            break;
        
        v += pos[0].rm_eo;
        n -= pos[0].rm_eo;

        regexec_flags |= REG_NOTBOL;
    } while (n && pos[0].rm_eo>0);
    
    return JSI_OK;
}

#define FN_regexec JSI_INFO("\
Perform regexp match checking.  Returns the array of matches.\
Incompatibility note: does not set properties such as global, lastIndex, etc.")
static int RegexpExecCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0;
    Jsi_Value *v;
    ChkRegexp(_this, funcPtr, v);
    /* TODO: add lastIndex support. */
    char *str = Jsi_ValueString(interp,Jsi_ValueArrayIndex(interp, args, skip), NULL);
    if (!str) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }
    return Jsi_RegExpMatches(interp, v, str, *ret);
}

static int RegexpTestCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int skip = 0, rc = 0;
    Jsi_Value *v;
    ChkRegexp(_this, funcPtr, v);
    char *str = Jsi_ValueString(interp,Jsi_ValueArrayIndex(interp, args, skip), NULL);
    if (!str) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }
    if (Jsi_RegExpMatch(interp, v, str, &rc, NULL) != JSI_OK)
        return JSI_ERROR;
    Jsi_ValueMakeBool(interp, ret, rc != 0);
    return JSI_OK;
}

static Jsi_CmdSpec regexpCmds[] = {
    { "RegExp",  RegExp_constructor,    1, 1, "val:regexp|string", JSI_CMD_IS_CONSTRUCTOR, .help="Create a regexp object", .retType=(uint)JSI_TT_REGEXP },
    { "exec",    RegexpExecCmd,         1, 1, "val:string", .help="return matching string", .info=FN_regexec, .retType=(uint)JSI_TT_ARRAY|JSI_TT_OBJECT|JSI_TT_NULL },
    { "test",    RegexpTestCmd,         1, 1, "val:string", .help="test if a string matches", .retType=(uint)JSI_TT_BOOL },
    { NULL,.help="Commands for managing reqular expression objects" }
};

int jsi_RegexpInit(Jsi_Interp *interp)
{
    interp->RegExp_prototype = Jsi_CommandCreateSpecs(interp, "RegExp", regexpCmds, NULL, 0);
    return JSI_OK;
}

#endif
