#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

/* Jsi_Obj constructor */
static Jsi_RC ObjectConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        /* new operator will do the rest */
        return JSI_OK;
    }
    
    if (Jsi_ValueGetLength(interp, args) <= 0) {
        Jsi_Obj *o = Jsi_ObjNew(interp);
        o->__proto__ = interp->Object_prototype;
        Jsi_ValueMakeObject(interp, ret, o);
        return JSI_OK;
    }
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    if (!v || v->vt == JSI_VT_UNDEF || v->vt == JSI_VT_NULL) {
        Jsi_Obj *o = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
        Jsi_ValueMakeObject(interp, ret, o);
        return JSI_OK;
    }
    Jsi_ValueDup2(interp, ret, v);
    Jsi_ValueToObject(interp, *ret);
    return JSI_OK;
}

/* Function.prototype pointed to a empty function */
static Jsi_RC jsi_FunctionPrototypeConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return JSI_OK;
}

static Jsi_RC jsi_Function_constructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return Jsi_LogError("Calling Function is unsupported");
  /*  if (Jsi_FunctionIsConstructor(funcPtr)) {
        _this->d.obj->ot = JSI_OT_FUNCTION;
        return JSI_OK;
    }
    Jsi_Obj *o = Jsi_ObjNewType(interp, JSI_OT_FUNCTION);
    Jsi_ValueMakeObject(interp, ret, o);
    return JSI_OK;*/
}

// Guesstimate type based on default value.
static int jsi_GetDefaultType(const char *cp) {
    if (isdigit(*cp) || *cp == '-' || *cp == '.') return JSI_TT_NUMBER;
    if (*cp == 'f' || *cp == 't') return JSI_TT_BOOLEAN;
    if (*cp == 'n') return JSI_TT_NULL;
    if (*cp == 'v') return JSI_TT_VOID;
    if (*cp == '\'' || *cp == '\"') return JSI_TT_STRING;
    return 0;
}

// Extract typechecking info from argStr for builtin Jsi_CmdSpec on first call
Jsi_ScopeStrs* jsi_ParseArgStr(Jsi_Interp *interp, const char *argStr)
{
    int i;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    int argc;
    char **argv, *sname, *stype, *cp, *ap = NULL;
    Jsi_SplitStr(argStr, &argc, &argv, ",", &dStr);
    if (argc<=0)
        return NULL;
    Jsi_ScopeStrs *ss = jsi_ScopeStrsNew();
    for (i=0; i<argc; i++) {
        sname = argv[i];
        stype = NULL;
        ap = NULL;
        while (sname && *sname && isspace(*sname)) { sname++; }
        if ((cp=Jsi_Strchr(sname, '='))) {
            ap = cp+1;
            *cp = 0;
        }
        if ((cp=Jsi_Strchr(sname, ':'))) {
            stype = cp+1;
            *cp = 0;
            while (*stype && isspace(*stype)) { stype++; }
            if (*stype) {
                cp = stype+Jsi_Strlen(stype)-1;
                while (cp>=stype && isspace(*cp)) { *cp = 0; cp--; }
            }
        }
        int atyp = jsi_typeGet(interp, stype);
        if (ap) {
            int datyp = jsi_GetDefaultType(ap);
            if (datyp != JSI_TT_VOID)
                atyp |= datyp;
        }
        if (sname && *sname) {
            cp = sname+Jsi_Strlen(sname)-1;
            while (cp>=sname && isspace(*cp)) { *cp = 0; cp--; }
        }
        jsi_ScopeStrsPush(interp, ss, sname, atyp);
    }
    Jsi_DSFree(&dStr);
    return ss;
}

static Jsi_RC jsi_SharedArgs(Jsi_Interp *interp, Jsi_Value *args, Jsi_Func *func, int alloc)
{
    int i;
    Jsi_RC rc = JSI_OK, nrc = JSI_OK;
    Jsi_ScopeStrs *argnames;
    // Extract typechecking info from argStr for builtin commands on first call
    const char *argStr = (func->cmdSpec ? func->cmdSpec->argStr: NULL);
    if (alloc && func->type == FC_BUILDIN && func->callCnt == 0 && func->argnames==NULL
        && argStr && Jsi_Strchr(argStr, ':'))
        func->argnames = jsi_ParseArgStr(interp, argStr);
    argnames = func->argnames;
    int argc = Jsi_ValueGetLength(interp, args);
    if (alloc && (interp->typeCheck.all|interp->typeCheck.run) && jsi_RunFuncCallCheck(interp, func, argc, func->name, NULL, NULL, 0) != JSI_OK
        && (interp->typeCheck.strict || interp->typeCheck.error))
        nrc = JSI_ERROR;
    if (!argnames)
        return nrc;
    
    int addargs = func->callflags.bits.addargs;
    for (i = 0; i < argnames->count; ++i) {
        int n = i-addargs;
        const char *argkey = jsi_ScopeStrsGet(argnames, i);
        if (!argkey) break;
        
        Jsi_Value *dv = NULL, *v = Jsi_ValueArrayIndex(interp, args, i);
        if (!alloc) {
            if (func->type == FC_BUILDIN)
                continue;
            if (v==NULL  && i >= addargs) {
                v = argnames->args[n].defValue;
                if (v) {
                    dv = Jsi_ValueObjLookup(interp, args, argkey, 1);
                    if (dv)
                        v = dv;
                }
            }
            if (v)
                Jsi_DecrRefCount(interp, v);
        } else {
            if (v==NULL  && i >= addargs)
                dv = v = argnames->args[n].defValue;
            if (v && rc == JSI_OK && i >= addargs) {
                int typ = argnames->args[n].type;
                if ((typ && interp->typeCheck.run) || interp->typeCheck.all)
                    rc = jsi_ArgTypeCheck(interp, typ, v, "for argument", argkey, i+1, func, (dv!=NULL));
            }
            if (func->type == FC_BUILDIN)
                continue;
            if (v && dv)
                v = Jsi_ValueDup(interp, v);
            else if (v)
                Jsi_IncrRefCount(interp, v);
            else {
                v = Jsi_ValueNew(interp);
            }
            jsi_ValueObjSet(interp, args, argkey, v, JSI_OM_DONTENUM | JSI_OM_INNERSHARED, 1);
        }
    }
    return (nrc == JSI_ERROR?nrc:rc);
}

// Handle all function/command calls.
Jsi_RC jsi_FuncCallSub(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *callee,
    Jsi_Value **ret, Jsi_Func *funcPtr, Jsi_Value *fthis, int calltrc)
{
    Jsi_Func *pprevActive = interp->prevActiveFunc;
    Jsi_Func *prevActive = interp->activeFunc;
    Jsi_IncrRefCount(interp, args);
    Jsi_RC rc = jsi_SharedArgs(interp, args, funcPtr, 1);
    const char *oldCurFunc = interp->curFunction;
    if (funcPtr->name && funcPtr->name[0] && funcPtr->type == FC_NORMAL)
        interp->curFunction = funcPtr->name;
    if (rc == JSI_OK) {
    
        interp->prevActiveFunc = interp->activeFunc;
        interp->activeFunc = funcPtr;
        int profile = interp->profile, coverage = interp->coverage;
        int tc = interp->traceCall;
        double timStart = 0;
        jsi_PkgInfo *pkg = funcPtr->pkg;
        if (pkg) {
            tc |= pkg->popts.conf.traceCall;
            profile |= pkg->popts.conf.profile;
            coverage |= pkg->popts.conf.coverage;
        }
    
        interp->callDepth++;
        int adds = funcPtr->callflags.bits.addargs;
        Jsi_CmdSpec *cs  = funcPtr->cmdSpec;
        if (adds && cs && (cs->flags&JSI_CMDSPEC_NONTHIS))
            adds = 0;
        funcPtr->callflags.bits.addargs = 0;
        jsi_InitLocalVar(interp, args, funcPtr);
    
        if (!calltrc) {
            if (funcPtr->type == FC_NORMAL)
                calltrc = (tc&jsi_callTraceFuncs);
            else
                calltrc = (tc&jsi_callTraceCmds);
        }
        if (calltrc && funcPtr->name)
            jsi_TraceFuncCall(interp, funcPtr, interp->curIp, callee, args, 0, tc);
    
        Jsi_Value *oc = interp->callee;
        interp->callee = callee;
        if (profile || coverage) {
            interp->profileCnt++;
            timStart = jsi_GetTimestamp();
        }
        int as_cons = funcPtr->callflags.bits.iscons;
        if (funcPtr->type == FC_NORMAL) {
            rc = jsi_evalcode(interp->ps, funcPtr, funcPtr->opcodes, callee->d.obj->d.fobj->scope, 
                       args, fthis, ret, funcPtr->filePtr);
            interp->funcCallCnt++;
        } else if (!funcPtr->callback) {
            rc = Jsi_LogError("can not call:\"%s()\"", funcPtr->name);
        } else {
            if (funcPtr->f.bits.hasattr)
            {
                if ((funcPtr->f.bits.isobj) && callee->vt != JSI_VT_OBJECT) {
                    rc = Jsi_LogError("'this' is not object: \"%s()\"", funcPtr->name);
                } else if ((!(funcPtr->f.bits.iscons)) && as_cons) {
                    rc = Jsi_LogError("can not call as constructor: \"%s()\"", funcPtr->name);
                } else {
                    int aCnt = Jsi_ValueGetLength(interp, args);
                    const char *cstr = cs->argStr;
                    if (!cstr) cstr = "";
                    if (aCnt<(cs->minArgs+adds)) {
                        rc = Jsi_LogError("missing args, expected \"%s(%s)\" ", cs->name, cstr);
                    } else if (cs->maxArgs>=0 && (aCnt>cs->maxArgs+adds)) {
                        rc = Jsi_LogError("extra args, expected \"%s(%s)\" ", cs->name, cstr);
                    }
                }
            }
            if (rc == JSI_OK) {
                rc = funcPtr->callback(interp, args, fthis, ret, funcPtr);
                interp->cmdCallCnt++;
            }
        }
        interp->callee = oc;
        funcPtr->callCnt++;
        if (profile || coverage) {
            double timEnd = jsi_GetTimestamp(), timUsed = (timEnd - timStart);;
            assert(timUsed>=0);
            funcPtr->allTime += timUsed;
            if (interp->framePtr->evalFuncPtr)
                interp->framePtr->evalFuncPtr->subTime += timUsed;
            else
                interp->subTime += timUsed;
        }
    
        if (rc == JSI_OK && calltrc && (tc&jsi_callTraceReturn))
            jsi_TraceFuncCall(interp, funcPtr, NULL, fthis, NULL, *ret, tc);
        if (rc == JSI_OK && !as_cons && funcPtr->retType && (interp->typeCheck.all || interp->typeCheck.run))
            rc = jsi_ArgTypeCheck(interp, funcPtr->retType, *ret, "returned from", funcPtr->name, 0, funcPtr, 0);
        interp->callDepth--;
    }

    jsi_SharedArgs(interp, args, funcPtr, 0);
    interp->curFunction = oldCurFunc;
    interp->activeFunc = prevActive;
    interp->prevActiveFunc = pprevActive;
    Jsi_DecrRefCount(interp, args);
    return rc;
}

static Jsi_RC jsi_FuncBindCall(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_FuncObj *fo = funcPtr->fobj;
    if (!fo)
        return Jsi_LogError("bind failure");

    Jsi_Value *nargs = args, *fargs = fo->bindArgs;
    int i, argc = Jsi_ValueGetLength(interp, args);
    int fargc = (fargs? Jsi_ValueGetLength(interp, fargs) : 0);

    if (fargc>0) {
        Jsi_Value *nthis = Jsi_ValueArrayIndex(interp, fargs, 0);
        if (nthis && !Jsi_ValueIsNull(interp, nthis))
            _this = nthis;
        if (fargc>1) {
            nargs = Jsi_ValueNewArray(interp, NULL, 0);
            for (i=1; i<fargc; i++)
                Jsi_ValueArrayPush(interp, nargs, Jsi_ValueArrayIndex(interp, fargs, i));
            for (i=0; i<argc; i++)
                Jsi_ValueArrayPush(interp, nargs, Jsi_ValueArrayIndex(interp, args, i));
            Jsi_IncrRefCount(interp, nargs);
        }
    }
    Jsi_RC rc = Jsi_FunctionInvoke(interp, fo->bindFunc, nargs, ret, _this);
    if (Jsi_InterpGone(interp))
        return JSI_ERROR;
    if (nargs != args)
        Jsi_DecrRefCount(interp, nargs);
    return rc;
}

static Jsi_RC jsi_FunctionBindCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *tocall = _this;
    if (!Jsi_ValueIsFunction(interp, tocall) || !tocall->d.obj->d.fobj) 
        return Jsi_LogError("can not execute expression, expression is not a function");
    
    Jsi_Value *oval = jsi_MakeFuncValue(interp, jsi_FuncBindCall, NULL, ret, NULL);
    Jsi_Obj *obj = oval->d.obj;
    Jsi_FuncObj *fo = obj->d.fobj;
    Jsi_Func *fstatic = tocall->d.obj->d.fobj->func;
    Jsi_ObjDecrRefCount(interp, obj);
    fo->bindArgs = Jsi_ValueDup(interp, args);
    fo->bindFunc = tocall;
    Jsi_IncrRefCount(interp, tocall);
    if (fstatic->callback == jsi_NoOpCmd)
        obj->isNoOp = 1;
    return JSI_OK;
}

static Jsi_RC Jsi_FunctionCall(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret)
{
    if (!Jsi_ValueIsFunction(interp, _this)) 
        return Jsi_LogError("can not execute expression, expression is not a function");

    Jsi_FuncObj *fo = _this->d.obj->d.fobj;
    if (!fo)   /* empty function */
        return JSI_OK;
    Jsi_Func *funcPtr = fo->func;
    if (funcPtr->type == FC_BUILDIN)
        funcPtr->fobj = fo; // Backlink for bind.

    /* new this */
    Jsi_Value *fthis;
    Jsi_Value *arg1 = NULL;
    if ((arg1 = Jsi_ValueArrayIndex(interp, args, 0)) && !Jsi_ValueIsUndef(interp, arg1)
        && !Jsi_ValueIsNull(interp, arg1))
        fthis = Jsi_ValueDup(interp, arg1);
    else
        fthis = Jsi_ValueDup(interp, interp->Top_object);
    Jsi_ValueToObject(interp, fthis);

    /* prepare args */
    Jsi_ValueArrayShift(interp, args);
    Jsi_RC res = jsi_FuncCallSub(interp, args, _this, ret, funcPtr, fthis, 0);
    Jsi_DecrRefCount(interp, fthis);
    return res;
}

static Jsi_RC jsi_FunctionCallCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_FunctionIsConstructor(funcPtr)) 
        return Jsi_LogError("Execute call as constructor");
    return Jsi_FunctionCall(interp, args, _this, ret);
}

static Jsi_RC ObjectFreezeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *val = Jsi_ValueArrayIndex(interp, args, 0),
        *fval = Jsi_ValueArrayIndex(interp, args, 1),
        *bval = Jsi_ValueArrayIndex(interp, args, 2),
        *rval = Jsi_ValueArrayIndex(interp, args, 3);
    if (!val || !Jsi_ValueIsObjType(interp, val, JSI_OT_OBJECT))
        return Jsi_LogError("arg 1: expected object");
    if (fval && !Jsi_ValueIsBoolean(interp, fval))
        return Jsi_LogError("arg 2: expected bool");
    if (bval && !Jsi_ValueIsBoolean(interp, bval))
        return Jsi_LogError("arg 3: expected bool");
    if (rval && !Jsi_ValueIsBoolean(interp, rval))
        return Jsi_LogError("arg 4: expected bool");
    bool bnum = 1, rnum = 1, fnum = 1;
    if (bval)
        Jsi_GetBoolFromValue(interp, bval, &bnum);
    if (rval)
        Jsi_GetBoolFromValue(interp, rval, &rnum);
    if (fval)
        Jsi_GetBoolFromValue(interp, fval, &fnum);
    Jsi_Obj *obj = val->d.obj;
    obj->freeze = fnum;
    obj->freezeModifyOk = bnum;
    obj->freezeReadCheck = rnum;
    return JSI_OK;

}

static Jsi_RC ObjectKeysCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
   int argc = Jsi_ValueGetLength(interp, args);
   Jsi_Value *val = _this;
   
   if (argc>0)
        val = Jsi_ValueArrayIndex(interp, args, 0);

    Jsi_RC rc = Jsi_ValueGetKeys(interp, val, *ret);
    if (rc != JSI_OK)
        Jsi_LogError("can not call Keys() with non-object");
    return rc;
}

static Jsi_RC ObjectValuesCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
   int argc = Jsi_ValueGetLength(interp, args);
   Jsi_Value *val = _this;
   
   if (argc>0)
        val = Jsi_ValueArrayIndex(interp, args, 0);

    if (!Jsi_ValueIsObjType(interp, val, JSI_OT_OBJECT))
        return Jsi_LogError("can not call values() with non-object");
    Jsi_ValueMakeArrayObject(interp, ret, NULL);
    return Jsi_ObjGetValues(interp, Jsi_ValueGetObj(interp, val), *ret);
}

Jsi_RC jsi_ObjectToStringCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_ValueIsString(interp, _this)) {
        Jsi_ValueCopy(interp, *ret, _this);
        return JSI_OK;
    }    
    int quote = JSI_OUTPUT_QUOTE;
    Jsi_DString dStr = {};
    Jsi_ValueGetDString(interp, _this, &dStr, quote);
    Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return JSI_OK;
}


Jsi_RC Jsi_FunctionApply(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret)
{
    int isalloc = 0;
    if (!Jsi_ValueIsFunction(interp, _this)) 
        return Jsi_LogError("can not execute expression, expression is not a function");

    if (!_this->d.obj->d.fobj)
        return JSI_OK;
    
    Jsi_Func *funcPtr = _this->d.obj->d.fobj->func;
    Jsi_Value *fthis;
    Jsi_Value *arg1 = NULL;
    if ((arg1 = Jsi_ValueArrayIndex(interp, args, 0)) && !Jsi_ValueIsUndef(interp, arg1)
        && !Jsi_ValueIsNull(interp, arg1))
        fthis = Jsi_ValueDup(interp, arg1);
    else
        fthis = Jsi_ValueDup(interp, interp->Top_object);
    Jsi_ValueToObject(interp, fthis);
    
    /* prepare args */
    Jsi_RC rc = JSI_ERROR;
    Jsi_Value *fargs = Jsi_ValueArrayIndex(interp, args, 1);
    if (fargs) {
        if (fargs->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, fargs->d.obj)) {
            Jsi_LogError("second argument to Function.prototype.apply must be an array");
            goto done;
        }
    } else {
        isalloc = 1;
        fargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewType(interp, JSI_OT_ARRAY));
        Jsi_IncrRefCount(interp, fargs);
    }
    rc = jsi_FuncCallSub(interp, fargs, _this, ret, funcPtr, fthis, 0);
    
done:
    if (isalloc)
        Jsi_DecrRefCount(interp, fargs);
    Jsi_DecrRefCount(interp, fthis);
    return rc;
}


static Jsi_RC ObjectValueOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_ValueDup2(interp, ret, _this);
    return JSI_OK;
}

static Jsi_RC ObjectMergeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args,0);
    if (!v || v->vt != JSI_VT_OBJECT || v->d.obj->ot != JSI_OT_OBJECT ||
        _this->vt != JSI_VT_OBJECT || _this->d.obj->ot != JSI_OT_OBJECT)
        return Jsi_LogError("expected object");
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
    Jsi_ValueMakeObject(interp, ret, obj);
    Jsi_TreeEntry *tPtr;
    Jsi_TreeSearch search;
    for (tPtr = Jsi_TreeSearchFirst(_this->d.obj->tree, &search, 0, NULL);
        tPtr; tPtr = Jsi_TreeSearchNext(&search)) {
        Jsi_Value *v = (Jsi_Value *)Jsi_TreeValueGet(tPtr);
        if (v && v->f.bits.dontenum == 0)
            Jsi_ObjInsert(interp, obj, (const char *)Jsi_TreeKeyGet(tPtr), v, 0);
    }
    Jsi_TreeSearchDone(&search);
    for (tPtr = Jsi_TreeSearchFirst(v->d.obj->tree, &search, 0, NULL);
        tPtr; tPtr = Jsi_TreeSearchNext(&search)) {
        Jsi_Value *v = (Jsi_Value *)Jsi_TreeValueGet(tPtr);
        if (v && v->f.bits.dontenum == 0)
            Jsi_ObjInsert(interp, obj, (const char *)Jsi_TreeKeyGet(tPtr), v, 0);
    }
    Jsi_TreeSearchDone(&search);

    return JSI_OK;
}

#if (JSI_HAS___PROTO__==1)
static Jsi_RC jsi_GetPrototypeOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args,0);
    if (v->vt != JSI_VT_OBJECT)
        return JSI_ERROR;
    Jsi_ValueDup2(interp, ret, v->d.obj->__proto__);
    return JSI_OK;
}

static Jsi_RC jsi_SetPrototypeOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args,0);
    Jsi_Value *a = Jsi_ValueArrayIndex(interp, args,1);
    if (!v || !a || v->vt != JSI_VT_OBJECT || a->vt != JSI_VT_OBJECT)
        return JSI_ERROR;
    if (a->d.obj->tree->numEntries && a->d.obj != v->d.obj) {
        v->d.obj->__proto__ = Jsi_ValueDup(interp, a);
        v->d.obj->clearProto = 1;
    } else
        v->d.obj->__proto__ = a->d.obj->__proto__;
    return JSI_OK;
}
#endif

static Jsi_RC jsi_HasOwnPropertyCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *v;
    char *key = Jsi_ValueArrayIndexToStr(interp, args,0, NULL);
    v = Jsi_ValueObjLookup(interp, _this, key, 0);
    Jsi_ValueMakeBool(interp, ret, (v != NULL));
    return JSI_OK;
}

static Jsi_RC ObjectIsPrototypeOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *proto, *sproto, *v = Jsi_ValueArrayIndex(interp, args,0);
    int bval = 0;
    if (v->vt != JSI_VT_OBJECT || _this->vt != JSI_VT_OBJECT) {
        goto retval;
    }
    proto = _this->d.obj->__proto__;
    sproto = v->d.obj->__proto__;
    if (!proto)
        goto retval;
    while (sproto) {
        if ((bval=(sproto == proto)))
            break;
        if (sproto->vt != JSI_VT_OBJECT)
            break;
        sproto = sproto->d.obj->__proto__;
    }
retval:
    Jsi_ValueMakeBool(interp, ret, bval);
    return JSI_OK;
    
}

static Jsi_RC ObjectIsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *v1 = Jsi_ValueArrayIndex(interp, args,0);
    Jsi_Value *v2 = Jsi_ValueArrayIndex(interp, args,1);
    Jsi_ValueMakeBool(interp, ret, Jsi_ValueIsEqual(interp, v1, v2));
    return JSI_OK;
}

static Jsi_RC ObjectCreateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (interp->subOpts.noproto) 
        return Jsi_LogError("inheritance is disabled in interp");
    Jsi_Obj *obj;
    Jsi_Value *proto = Jsi_ValueArrayIndex(interp, args,0);
    Jsi_Value *props = Jsi_ValueArrayIndex(interp, args,1);

    if (proto->vt != JSI_VT_NULL && proto->vt != JSI_VT_OBJECT) 
        return Jsi_LogError("arg 1 is not a proto object or null");
    if (props && (props->vt != JSI_VT_OBJECT || props->d.obj->ot != JSI_OT_OBJECT)) 
        return Jsi_LogError("arg 2 is not a properties object");
        
    Jsi_ValueMakeObject(interp, ret, obj=Jsi_ObjNew(interp));
    if (proto->vt == JSI_VT_OBJECT) {
        obj->__proto__ = proto;
        obj->clearProto = 1;
        Jsi_IncrRefCount(interp, proto);
    }
    if (props) {
        Jsi_Obj *pobj = props->d.obj;
        Jsi_TreeEntry *tPtr;
        Jsi_TreeSearch search;
        for (tPtr = Jsi_TreeSearchFirst(pobj->tree, &search, 0, NULL);
            tPtr; tPtr = Jsi_TreeSearchNext(&search)) {
            Jsi_Value *v = (Jsi_Value *)Jsi_TreeValueGet(tPtr);
            if (v && v->f.bits.dontenum == 0)
                Jsi_ObjInsert(interp, obj, (const char *)Jsi_TreeKeyGet(tPtr), v, 0);
        }
        Jsi_TreeSearchDone(&search);
    }
    return JSI_OK;
    
}

static Jsi_RC ObjectPropertyIsEnumerableCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *v;
    int b = 0;

    char *key = Jsi_ValueArrayIndexToStr(interp, args,0, NULL);
    v = Jsi_ValueObjLookup(interp, _this, key, 0);
    b = (v && (v->f.bits.dontenum==0));

    Jsi_ValueMakeBool(interp, ret, b);
    return JSI_OK;
}

static Jsi_RC ObjectToLocaleStringCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return jsi_ObjectToStringCmd(interp, args, _this, ret, funcPtr);
}

static Jsi_RC jsi_FunctionApplyCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_FunctionIsConstructor(funcPtr)) 
        return Jsi_LogError("Execute apply as constructor");
    
    return Jsi_FunctionApply(interp, args, _this, ret);
}

Jsi_Value *jsi_ProtoObjValueNew1(Jsi_Interp *interp, const char *name) {
    Jsi_Value *v = jsi_ObjValueNew(interp);
    Jsi_IncrRefCount(interp, v);
    //v->f.bits.isglob = 1;
#ifdef JSI_MEM_DEBUG
    v->VD.label = "jsi_ProtoValueNew1";
    v->VD.label2 = name;
#endif
    if (name != NULL)
        Jsi_PrototypeDefine(interp, name, v);
    return v;
}

Jsi_Value *jsi_ProtoValueNew(Jsi_Interp *interp, const char *name, const char *parent)
{
    Jsi_Value *fproto;
    if (parent == NULL)
        parent = "Object";
    fproto = jsi_ProtoObjValueNew1(interp, name);
    Jsi_PrototypeObjSet(interp, parent, Jsi_ValueGetObj(interp, fproto));
    return fproto;
}

static Jsi_CmdSpec functionCmds[] = {
    { "Function",  jsi_Function_constructor,   0, 0,  "", .help="Function constructor (unimplemented)", .retType=(uint)JSI_TT_FUNCTION, .flags=JSI_CMD_IS_CONSTRUCTOR },
    { "apply",     jsi_FunctionApplyCmd,       1, 2,  "thisArg:null|object|function, args:array=void", .help="Call function passing args array", .retType=(uint)JSI_TT_ANY },
    { "bind",      jsi_FunctionBindCmd,        0, -1, "thisArg:object|function=null,...", .help="Return function that calls bound function prepended with thisArg+arguments", .retType=(uint)JSI_TT_ANY },
    { "call",      jsi_FunctionCallCmd,        1, -1, "thisArg:null|object|function, arg1, ...", .help="Call function with args", .retType=(uint)JSI_TT_ANY },
    { NULL, 0,0,0,0, .help="Commands for accessing functions" }
};

int jsi_InitFunction(Jsi_Interp *interp, int release)
{
    if (!release)
        Jsi_CommandCreateSpecs(interp, NULL, functionCmds, interp->Function_prototype, JSI_CMDSPEC_PROTO|JSI_CMDSPEC_ISOBJ);
    return JSI_OK;
}

/* TODO: defineProperty, defineProperties, */
static Jsi_CmdSpec objectCmds[] = {
#ifndef __JSI_OMITDECL
    { "Object",         ObjectConstructor,      0, 1,  "val:object|function|null=void", .help="Object constructor", .retType=(uint)JSI_TT_OBJECT, .flags=JSI_CMD_IS_CONSTRUCTOR },
    { "create",         ObjectCreateCmd,        1, 2, "proto:null|object, properties:object=void", .help="Create a new object with prototype object and properties", .retType=(uint)JSI_TT_OBJECT },
#if (JSI_HAS___PROTO__>0)
    { "getPrototypeOf", jsi_GetPrototypeOfCmd,  1, 1, "name:object|function", .help="Return prototype of an object", .retType=(uint)JSI_TT_OBJECT|JSI_TT_FUNCTION },
#endif
    { "hasOwnProperty", jsi_HasOwnPropertyCmd,  1, 1, "name:string", .help="Returns a true if object has the specified property", .retType=(uint)JSI_TT_BOOLEAN },
    { "is",             ObjectIsCmd, 2, 2, "value1, value2", .help="Tests if two values are equal", .retType=(uint)JSI_TT_BOOLEAN },
    { "isPrototypeOf",  ObjectIsPrototypeOfCmd, 1, 1, "name", .help="Tests for an object in another object's prototype chain", .retType=(uint)JSI_TT_BOOLEAN },
    { "freeze",         ObjectFreezeCmd,        1, 4, "obj:object, freeze:boolean=true, modifyok:boolean=true, readcheck:boolean=true", .help="Freeze/unfreeze an object with optionally", .retType=(uint)JSI_TT_VOID },
    { "keys",           ObjectKeysCmd,          0, 1, "obj:object|function=void", .help="Return the keys of an object or array", .retType=(uint)JSI_TT_ARRAY },
    { "merge",          ObjectMergeCmd,         1, 1, "obj:object|function", .help="Return new object containing merged values", .retType=(uint)JSI_TT_OBJECT },
    { "propertyIsEnumerable", ObjectPropertyIsEnumerableCmd,1, 1, "name", .help="Determine if a property is enumerable", .retType=(uint)JSI_TT_BOOLEAN },
#if (JSI_HAS___PROTO__>0)
    { "setPrototypeOf", jsi_SetPrototypeOfCmd,  2, 2, "name:object, value:object", .help="Set prototype of an object" },
#endif
    { "toLocaleString", ObjectToLocaleStringCmd,0, 1, "quote:boolean=false", .help="Convert to string", .retType=(uint)JSI_TT_STRING },
    { "toString",       jsi_ObjectToStringCmd,  0, 1, "quote:boolean=false", .help="Convert to string", .retType=(uint)JSI_TT_STRING }, 
    { "values",         ObjectValuesCmd,        0, 1, "obj:object=void", .help="Return the  values of an object", .retType=(uint)JSI_TT_ARRAY },
    { "valueOf",        ObjectValueOfCmd,       0, 0, "", .help="Returns primitive value", .retType=(uint)JSI_TT_ANY },
    { NULL, 0,0,0,0, .help="Commands for accessing Objects" }
#endif
};

int jsi_InitObject(Jsi_Interp *interp, int release)
{
    if (!release)
        Jsi_CommandCreateSpecs(interp, NULL, objectCmds, interp->Object_prototype, JSI_CMDSPEC_PROTO|JSI_CMDSPEC_ISOBJ);
    return JSI_OK;
}

Jsi_RC jsi_InitProto(Jsi_Interp *interp, int release)
{
    if (release) return JSI_OK;
     /* Function and Object are created together. */
    Jsi_Value *global = interp->csc;
    /* Top, the default "this" value, pointed to global, is an object */
    interp->Top_object = global;

    /* object_prototype the start of protochain */
    interp->Object_prototype = jsi_ProtoObjValueNew1(interp, "Object");
    interp->Top_object->d.obj->__proto__ = interp->Object_prototype;
        
    /* Function.prototype.prototype is a common object */
    interp->Function_prototype_prototype = jsi_ProtoObjValueNew1(interp, "Function.prototype");
    interp->Function_prototype_prototype->d.obj->__proto__ = interp->Object_prototype;
    
    /* Function.prototype.__proto__ pointed to Jsi_Obj.prototype */
    interp->Function_prototype = jsi_MakeFuncValue(interp, jsi_FunctionPrototypeConstructor, "prototype", NULL, NULL);
    //Jsi_IncrRefCount(interp, interp->Function_prototype);
    Jsi_ValueInsertFixed(interp, interp->Function_prototype, "prototype", 
                              interp->Function_prototype_prototype);
    interp->Function_prototype->d.obj->__proto__ = interp->Object_prototype;
    
    /* Jsi_Obj.__proto__ pointed to Function.prototype */
    Jsi_Value *_Object = jsi_MakeFuncValue(interp, ObjectConstructor, "prototype", NULL, NULL);
    //Jsi_IncrRefCount(interp, _Object);
    Jsi_ValueInsertFixed(interp, _Object, "prototype", interp->Object_prototype);
    _Object->d.obj->__proto__ = interp->Function_prototype;

    /* both Function.prototype,__proto__ pointed to Function.prototype */
    Jsi_Value *_Function = jsi_MakeFuncValue(interp, jsi_Function_constructor, "prototype", NULL, NULL);
    //Jsi_IncrRefCount(interp, _Function);
    Jsi_ValueInsertFixed(interp, _Function, "prototype", interp->Function_prototype);
    _Function->d.obj->__proto__ = interp->Function_prototype;

    Jsi_ValueInsert(interp, global, "Object", _Object, JSI_OM_DONTENUM);
    Jsi_ValueInsert(interp, global, "Function", _Function, JSI_OM_DONTENUM);

    //Jsi_HashSet(interp->genValueTbl, _Object, _Object);
    //Jsi_HashSet(interp->genValueTbl, _Function, _Function);
    //Jsi_HashSet(interp->genValueTbl, interp->Object_prototype , interp->Object_prototype);
    //Jsi_HashSet(interp->genValueTbl, interp->Function_prototype, interp->Function_prototype);
    Jsi_HashSet(interp->genObjTbl, interp->Function_prototype->d.obj, interp->Function_prototype->d.obj);

    interp->cleanObjs[0] = _Function->d.obj;
    interp->cleanObjs[1] = _Object->d.obj;
    interp->cleanObjs[2] = NULL;

    jsi_InitObject(interp, 0);
    jsi_InitFunction(interp, 0);
    
    jsi_InitString(interp, 0);
    jsi_InitBoolean(interp, 0);
    jsi_InitNumber(interp, 0);
    jsi_InitArray(interp, 0);
    jsi_InitRegexp(interp, 0);
#ifndef JSI_OMIT_MATH
    jsi_InitMath(interp, 0);
#endif
    jsi_InitTree(interp, 0);
    interp->protoInit = 1;
    return JSI_OK;
}

#endif
