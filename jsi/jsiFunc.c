#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

/* Return value from call to function will is not used. */
int Jsi_FunctionReturnIgnored(Jsi_Interp *interp, Jsi_Func *funcPtr) {
    return funcPtr->callflags.bits.isdiscard;
}

int Jsi_FunctionIsConstructor(Jsi_Func *funcPtr)
{
    return (funcPtr->f.bits.iscons);
}

Jsi_CmdSpec *Jsi_FunctionGetSpecs(Jsi_Func *funcPtr)
{
    return funcPtr->cmdSpec;
}

void *Jsi_FunctionPrivData(Jsi_Func *funcPtr)
{
    return funcPtr->privData;
}

const char *jsi_ObjectTypeName(Jsi_Interp *interp, Jsi_otype otyp)
{
    switch (otyp) {
        case JSI_OT_NUMBER:     return "number"; 
        case JSI_OT_STRING:     return "string"; 
        case JSI_OT_BOOL:       return "boolean"; 
        case JSI_OT_ARRAY:      return "array"; 
        case JSI_OT_FUNCTION:   return "function"; 
        case JSI_OT_OBJECT:     return "object"; 
        case JSI_OT_REGEXP:     return "regexp"; 
        case JSI_OT_ITER:       return "iter"; 
        case JSI_OT_USEROBJ:    return "userobj"; 
        case JSI_OT_UNDEF:      return "any"; break;
    }
    return "undefined";
}

const char *jsi_ValueTypeName(Jsi_Interp *interp, Jsi_Value *val)
{
    switch (val->vt) {
        case JSI_VT_NUMBER:     return "number"; 
        case JSI_VT_STRING:     return "string"; 
        case JSI_VT_BOOL:       return "boolean"; 
        case JSI_VT_OBJECT:     return jsi_ObjectTypeName(interp, val->d.obj->ot); 
        case JSI_VT_VARIABLE:   return "variable"; 
        case JSI_VT_NULL:       return "null"; 
        case JSI_VT_UNDEF:      break;
    }
    return "undefined";
}

int jsi_typeGet(Jsi_Interp *interp, const char *tname) {
    if (!tname)
        return 0;
    if (strchr(tname, '|')) {
        int argc, i, rc, val = 0;
        char **argv;
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        Jsi_SplitStr(tname, &argc, &argv, "|", &dStr);
        for (i=0; i<argc; i++) {
            rc = jsi_typeGet(interp, argv[i]);
            if (rc < 0)
                break;
            val |= rc;
        }
        Jsi_DSFree(&dStr);
        if (i<argc)
            return -1;
        return val;
    }
    switch (tname[0]) {
        case 'b': if (Jsi_Strcmp(tname, "boolean")==0) return JSI_TT_BOOL; break;
        case 's': if (Jsi_Strcmp(tname, "string")==0) return JSI_TT_STRING; break;
        case 'n': if (Jsi_Strcmp(tname, "null")==0) return JSI_TT_NULL;
                  if (Jsi_Strcmp(tname, "number")==0) return JSI_TT_NUMBER; break;
        case 'o': if (Jsi_Strcmp(tname, "object")==0) return JSI_TT_OBJECT; break;
        case 'r': if (Jsi_Strcmp(tname, "regexp")==0) return JSI_TT_REGEXP; break;
        case 'f': if (Jsi_Strcmp(tname, "function")==0) return JSI_TT_FUNCTION; break;
        case 'u': if (Jsi_Strcmp(tname, "userobj")==0) return JSI_TT_USEROBJ; break;
        case 'a': if (Jsi_Strcmp(tname, "array")==0) return JSI_TT_ARRAY;
                  if (Jsi_Strcmp(tname, "any")==0) return 0; break;
    }
    Jsi_LogWarn("Type \"%s\" is not one of boolean, string, number, function, array, object, regexp, userobj, null, or any", tname);
    return 0;
}

const char *jsi_typeName(Jsi_Interp *interp, int typ, Jsi_DString *dStr) {
    if (typ<=0 || (typ&JSI_TT_ANY)) {
        Jsi_DSAppend(dStr, "any", NULL);
        return Jsi_DSValue(dStr);
    }
    int i = 0;
    if (typ&JSI_TT_NUMBER) Jsi_DSAppend(dStr, (i++?"|":""), "number", NULL);
    if (typ&JSI_TT_STRING) Jsi_DSAppend(dStr, (i++?"|":""), "string", NULL);
    if (typ&JSI_TT_BOOL)  Jsi_DSAppend(dStr, (i++?"|":""), "boolean", NULL);
    if (typ&JSI_TT_ARRAY)   Jsi_DSAppend(dStr, (i++?"|":""), "array", NULL);
    if (typ&JSI_TT_FUNCTION) Jsi_DSAppend(dStr, (i++?"|":""), "function", NULL);
    if (typ&JSI_TT_OBJECT) Jsi_DSAppend(dStr, (i++?"|":""), "object", NULL);
    if (typ&JSI_TT_REGEXP) Jsi_DSAppend(dStr, (i++?"|":""), "regexp", NULL);
    if (typ&JSI_TT_USEROBJ) Jsi_DSAppend(dStr, (i++?"|":""), "userobj", NULL);
    if (typ&JSI_TT_NULL) Jsi_DSAppend(dStr, (i++?"|":""), "null", NULL);
    if (typ&JSI_TT_VOID) Jsi_DSAppend(dStr, (i++?"|":""), "void", NULL);
    return Jsi_DSValue(dStr);
}

int jsi_ArgTypeCheck(Jsi_Interp *interp, int typ,  Jsi_Value *arg, const char *p1, const char *p2, int index, Jsi_Func *func) {
    int rc;
    if (typ <= 0 || (interp->typeCheck <= jsi_TypeChk_Disable))
        return JSI_OK;
    if (index == 0 && func && func->type == FC_BUILDIN && interp->typeCheck < jsi_TypeChk_Static)
        return JSI_OK; // Normally do not check return types for builtins.
    if ((typ&JSI_TT_ANY)) return JSI_OK;
    if (index == 0 && (typ&JSI_TT_VOID)) {
        if (arg->vt != JSI_VT_UNDEF)
            goto done;
        return JSI_OK;
    }
    if (arg->vt == JSI_VT_UNDEF)
        return JSI_OK;
    rc = JSI_OK;
    if (typ&JSI_TT_NUMBER && Jsi_ValueIsNumber(interp, arg)) return rc;
    if (typ&JSI_TT_STRING && Jsi_ValueIsString(interp, arg)) return rc;
    if (typ&JSI_TT_BOOL && Jsi_ValueIsBoolean(interp, arg))  return rc;
    if (typ&JSI_TT_ARRAY && Jsi_ValueIsArray(interp, arg))   return rc;
    if (typ&JSI_TT_FUNCTION && Jsi_ValueIsFunction(interp, arg)) return rc;
    if (typ&JSI_TT_OBJECT && Jsi_ValueIsObjType(interp, arg, JSI_OT_OBJECT)) return rc;
    if (typ&JSI_TT_REGEXP && Jsi_ValueIsObjType(interp, arg, JSI_OT_REGEXP)) return rc;
    if (typ&JSI_TT_USEROBJ && Jsi_ValueIsObjType(interp, arg, JSI_OT_USEROBJ)) return rc;
    if (typ&JSI_TT_NULL && Jsi_ValueIsNull(interp, arg)) return rc;
done:
    {
        Jsi_DString dStr = {};
        const char *exp = jsi_typeName(interp, typ, &dStr);
        const char *vtyp = jsi_ValueTypeName(interp, arg);
        char idxBuf[200];
        idxBuf[0] = 0;
        if (index>0)
            sprintf(idxBuf, " %d", index);
        int ltyp = JSI_LOG_WARN;
        if (interp->typeCheck >= jsi_TypeChk_Error) {
            ltyp = JSI_LOG_ERROR;
            rc = JSI_ERROR;
        }
        Jsi_LogMsg(interp, ltyp, "type mismatch %s%s: \"%s\" is \"%s\" not a \"%s\"", p1, idxBuf, p2, vtyp, exp);
        Jsi_DSFree(&dStr);
    }
    return rc;
}

int jsi_RunFuncCallCheck(Jsi_Interp *interp, Jsi_Func *func, int argc, const char *name, jsi_Pline *line)
{
    int rc = JSI_OK;
    if (interp->typeCheck <= jsi_TypeChk_Disable)
        return JSI_OK;
    Jsi_CmdSpec *spec = func->cmdSpec;
    Jsi_ScopeStrs *ss = func->argnames;
    if (ss==NULL && spec == NULL)
        return JSI_OK;
    int minArgs, maxArgs, mis = 0, varargs = 0;
    char nbuf[100];
    if (func->type == FC_BUILDIN) {
        varargs =  (spec->maxArgs<0);
        maxArgs = spec->maxArgs + func->callflags.bits.addargs;
        minArgs = spec->minArgs + func->callflags.bits.addargs;
    } else {
        varargs = ss->varargs;
        minArgs = (ss->firstDef>0 ? ss->firstDef-1 : ss->count);
        maxArgs = ss->count;
        mis = (argc != ss->count);
        if (func->retType == 0 && ss && ss->typeCnt == 0)
            return JSI_OK;
    }
    if (varargs) {
        if (argc >= minArgs)
            return JSI_OK;
        mis = (argc<minArgs);
    } else 
        mis = (argc<minArgs || argc>maxArgs);
    if (mis) {
        if (varargs)
            snprintf(nbuf, sizeof(nbuf), "%d or more", minArgs);
        else if (maxArgs > minArgs)
            snprintf(nbuf, sizeof(nbuf), "%d-%d", minArgs, maxArgs);
        else
            snprintf(nbuf, sizeof(nbuf), "%d", maxArgs);
        if (line)
            interp->parseLine = line;
        int ltyp = JSI_LOG_WARN;
        if (interp->typeCheck >= jsi_TypeChk_Error) {
            ltyp = JSI_LOG_ERROR;
            rc = JSI_ERROR;
        }
        Jsi_LogMsg(interp, ltyp, "incorrect arg count in function call \"%s(%s)\": got %d args, but expected %s",
            name, (spec&&spec->argStr)?spec->argStr:"", argc, nbuf);
        if (line)
            interp->parseLine = NULL;
        return rc;
    }
    return rc;
}

// Parse time function call checker.
void jsi_FuncCallCheck(jsi_Pstate *p, jsi_Pline *line, int argc, int isNew, const char *name, const char *namePre)
{
    Jsi_Interp *interp = p->interp;
    if (interp->typeCheck<jsi_TypeChk_Static || name == NULL)
        return;
    Jsi_Value *val;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    if (namePre) {
        Jsi_DSAppend(&dStr, namePre, ".", name, NULL);
        name = Jsi_DSValue(&dStr);
    }
    val = Jsi_NameLookup(interp, name);
    if (val != NULL && Jsi_ValueIsFunction(interp, val))
        jsi_RunFuncCallCheck(interp, val->d.obj->d.fobj->func, argc, name, line);
    Jsi_DSFree(&dStr);
}

/* TODO: if not in a file (an eval) save copy of body string from pstate->lexer??? */
Jsi_Func *jsi_FuncMake(jsi_Pstate *pstate, Jsi_ScopeStrs *args, OpCodes *ops, jsi_Pline* line, char *name)
{
    Jsi_Interp *interp = pstate->interp;
    Jsi_ScopeStrs *localvar = jsi_ScopeGetVarlist(pstate);
    Jsi_Func *f = jsi_FuncNew(interp);
    f->type = FC_NORMAL;
    f->opcodes = ops;
    f->argnames = args;
    f->localnames = localvar;
    f->script = interp->curFile;
    f->bodyline = *line;
    if (name)
        f->name = Jsi_KeyAdd(interp, name);
    f->retType = (Jsi_otype)pstate->argType;
    pstate->argType = 0;
    return f;
}

int Jsi_FunctionArguments(Jsi_Interp *interp, Jsi_Value *func, int *argcPtr)
{
    Jsi_FuncObj *funcPtr;
    Jsi_Func *f;
    if (!Jsi_ValueIsFunction(interp, func))
        return JSI_ERROR;
    funcPtr = func->d.obj->d.fobj;
    f = funcPtr->func;
    SIGASSERT(f, FUNC);
    *argcPtr = f->argnames->count;
    return JSI_OK;
}

void jsi_InitLocalVar(Jsi_Interp *interp, Jsi_Value *arguments, Jsi_Func *who)
{
    SIGASSERT(who, FUNC);
    if (who->localnames) {
        int i;
        for (i = 0; i < who->localnames->count; ++i) {
            const char *argkey = jsi_ScopeStrsGet(who->localnames, i);
            if (argkey) {
                DECL_VALINIT(key);// = VALINIT;
                Jsi_Value *v, *kPtr = &key; // Note: a string key so no reset needed.
                Jsi_ValueMakeStringKey(interp, &kPtr, argkey);
                v = jsi_ValueObjKeyAssign(interp, arguments, kPtr, NULL, JSI_OM_DONTENUM);
                jsi_ValueDebugLabel(v, "locals", who->name);
                v = v; // Compiler warning.
            }
        }
    }
}

int Jsi_FuncObjToString(Jsi_Interp *interp, Jsi_Obj *o, Jsi_DString *dStr)
{
    if (o->ot != JSI_OT_FUNCTION)
        return JSI_ERROR;
    Jsi_Func *f = o->d.fobj->func;
    if (f->type == FC_NORMAL) {
        Jsi_DSAppend(dStr, "function ", f->name?f->name:"", "(", NULL);
        int i;
        for (i = 0; i < f->argnames->count; ++i) {
            if (i) Jsi_DSAppend(dStr, ",", NULL);
            Jsi_DSAppend(dStr,  jsi_ScopeStrsGet(f->argnames, i), NULL);
        }
        Jsi_DSAppend(dStr, ") {...}", NULL);
    } else {
        Jsi_DSAppend(dStr, "function ", f->name?f->name:"", "() { [native code] }", NULL);
    }
    return JSI_OK;
}

Jsi_Value *jsi_MakeFuncValue(Jsi_Interp *interp, Jsi_CmdProc *callback, const char *name)
{
    Jsi_Obj *o = Jsi_ObjNew(interp);
    Jsi_Func *f = jsi_FuncNew(interp);
    Jsi_ObjIncrRefCount(interp, o);
    o->ot = JSI_OT_FUNCTION;
    f->type = FC_BUILDIN;
    f->callback = callback;
    f->privData = NULL;
    o->d.fobj = jsi_FuncObjNew(interp, f);
    f->cmdSpec = (Jsi_CmdSpec*)Jsi_Calloc(1,sizeof(Jsi_CmdSpec));
    Jsi_HashSet(interp->genDataTbl, f->cmdSpec, f->cmdSpec);
    f->cmdSpec->maxArgs = -1;
    if (name)
        f->cmdSpec->name = (char*)Jsi_KeyAdd(interp, name);
    f->script = interp->curFile;
    f->callback = callback;
    return Jsi_ValueMakeObject(interp,NULL, o);
}

Jsi_Value *jsi_MakeFuncValueSpec(Jsi_Interp *interp, Jsi_CmdSpec *cmdSpec, void *privData)
{
    Jsi_Obj *o = Jsi_ObjNew(interp);
    Jsi_Func *f = jsi_FuncNew(interp);
    o->ot = JSI_OT_FUNCTION;
    f->type = FC_BUILDIN;
    f->cmdSpec = cmdSpec;
    f->callback = cmdSpec->proc;
    f->privData = privData;
    f->f.flags = (cmdSpec->flags & JSI_CMD_MASK);
    f->script = interp->curFile;
    o->d.fobj = jsi_FuncObjNew(interp, f);
    return Jsi_ValueMakeObject(interp, NULL, o);
}


/* Call a function with args: args and/or ret can be NULL. */
int Jsi_FunctionInvoke(Jsi_Interp *interp, Jsi_Value *func, Jsi_Value *args, Jsi_Value **ret, Jsi_Value *_this)
{
    if (interp->deleting)
        return JSI_ERROR;
    if (!Jsi_ValueIsFunction(interp, func)) {
        Jsi_LogError("can not execute expression, expression is not a function\n");
        return JSI_ERROR;
    }
    if (!func->d.obj->d.fobj) {   /* empty function */
        return JSI_OK;
    }
    if (!ret) {
        if (!interp->nullFuncRet) {
            interp->nullFuncRet = Jsi_ValueNew(interp);
            Jsi_IncrRefCount(interp, interp->nullFuncRet);
        }
        *ret = interp->nullFuncRet;
        Jsi_ValueMakeUndef(interp, ret);
    }
    if (!args) {
        if (!interp->nullFuncArg) {
            interp->nullFuncArg = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, NULL, 0, 0));
            Jsi_IncrRefCount(interp, interp->nullFuncArg);
        }
        args = interp->nullFuncArg;
    }
    /* func to call */
    Jsi_Func *fstatic = func->d.obj->d.fobj->func;
    SIGASSERT(fstatic, FUNC);
    
    /* prepare args */
    if (args->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, args->d.obj)) {
        Jsi_LogError("argument must be an array\n");
        return JSI_ERROR;
    }
    /* new this */
    Jsi_Value *ntPtr = Jsi_ValueDup(interp, _this ? _this : func);
    Jsi_Func *prevActive = interp->activeFunc;
    int res = jsi_SharedArgs(interp, args, fstatic, 1);
    if (res == JSI_OK) {
        jsi_InitLocalVar(interp, args, fstatic);
        jsi_SetCallee(interp, args, func);
    
        Jsi_IncrRefCount(interp, args);
        
        if (interp->callTrace && fstatic->name)
            jsi_TraceFuncCall(interp, fstatic, NULL, args, ntPtr);
    
        interp->activeFunc = fstatic;
        if (fstatic->type == FC_NORMAL) {
            res = jsi_evalcode(interp->ps, fstatic->opcodes, func->d.obj->d.fobj->scope, 
                args, ntPtr, ret);
        } else {
            res = fstatic->callback(interp, args, ntPtr, ret, fstatic);
        }
        fstatic->callCnt++;
    }
    if (res == JSI_OK && fstatic->retType)
        res = jsi_ArgTypeCheck(interp, fstatic->retType, *ret, "returned from", fstatic->name, 0, fstatic);
    interp->activeFunc = prevActive;
    jsi_SharedArgs(interp, args, fstatic, 0);
    Jsi_DecrRefCount(interp, args);
    Jsi_DecrRefCount(interp, ntPtr);
    return res;
}

/* Special case: Call function with a single argument.  Return 1 if returns true. */
int Jsi_FunctionInvokeBool(Jsi_Interp *interp, Jsi_Value *func, Jsi_Value *arg)
{
    if (interp->deleting)
        return JSI_ERROR;
    Jsi_Value *vpargs, *frPtr = Jsi_ValueNew1(interp);
    int rc, bres = 0;
    if (!arg) {
        if (!interp->nullFuncArg) {
            interp->nullFuncArg = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, NULL, 0, 0));
            Jsi_IncrRefCount(interp, interp->nullFuncArg);
        }
        vpargs = interp->nullFuncArg;
    } else {
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, &arg, 1, 1));
    }
    Jsi_IncrRefCount(interp, vpargs);
    rc = Jsi_FunctionInvoke(interp, func, vpargs, &frPtr, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    if (rc == JSI_OK)
        bres = Jsi_ValueIsTrue(interp, frPtr);
    else
        Jsi_LogError("function call failed");
    Jsi_DecrRefCount(interp, frPtr);
    return bres;
}
       

Jsi_FuncObj *jsi_FuncObjNew(Jsi_Interp *interp, Jsi_Func *func)
{
    Jsi_FuncObj *f = (Jsi_FuncObj*)Jsi_Calloc(1,sizeof(Jsi_FuncObj));
    f->interp = interp;
    SIGINIT(f,FUNCOBJ);
    f->func = func;
    func->refCnt++;
    return f;
}

void jsi_FuncFree(Jsi_Interp *interp, Jsi_Func *func)
{
    if (--func->refCnt > 0)
        return;
    if (func->opcodes)
        jsi_FreeOpcodes(func->opcodes);
    if (func->hPtr)
        Jsi_HashEntryDelete(func->hPtr);
    if (func->localnames)
        jsi_ScopeStrsFree(interp, func->localnames);
    if (func->argnames)
        jsi_ScopeStrsFree(interp, func->argnames);
    MEMCLEAR(func);
    Jsi_Free(func);
    interp->funcCnt--;
}

Jsi_Func *jsi_FuncNew(Jsi_Interp *interp)
{
     Jsi_Func *func = (Jsi_Func*)Jsi_Calloc(1, sizeof(Jsi_Func));
     SIGINIT(func, FUNC);
     func->hPtr = Jsi_HashSet(interp->funcsTbl, func, func);
     func->refCnt = 1;
     interp->funcCnt++;
     return func;
}

void jsi_FuncObjFree(Jsi_FuncObj *fobj)
{
    if (fobj->scope)
        jsi_ScopeChainFree(fobj->interp, fobj->scope);
    if (fobj->func->bindArgs)
        Jsi_DecrRefCount(fobj->interp, fobj->func->bindArgs);
    if (fobj->func)
        jsi_FuncFree(fobj->interp, fobj->func);
    MEMCLEAR(fobj);
    Jsi_Free(fobj);
}

#endif
