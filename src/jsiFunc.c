#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

/* Return value from call to function will is not used. */
bool Jsi_FunctionReturnIgnored(Jsi_Interp *interp, Jsi_Func *funcPtr) {
    return funcPtr->callflags.bits.isdiscard;
}

bool Jsi_FunctionIsConstructor(Jsi_Func *funcPtr)
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

const char *jsi_TypeName(Jsi_Interp *interp, Jsi_ttype otyp)
{
    switch (otyp) {
        case JSI_TT_NUMBER:     return "number"; 
        case JSI_TT_STRING:     return "string"; 
        case JSI_TT_BOOLEAN:    return "boolean"; 
        case JSI_TT_ARRAY:      return "array"; 
        case JSI_TT_FUNCTION:   return "function"; 
        case JSI_TT_OBJECT:     return "object"; 
        case JSI_TT_REGEXP:     return "regexp"; 
        case JSI_TT_ANY:        return "any"; 
        case JSI_TT_USEROBJ:    return "userobj"; 
        case JSI_TT_ITEROBJ:    return "iterobj"; 
        case JSI_TT_UNDEFINED:      return "undefined";
        case JSI_TT_VOID:       return "void";
        case JSI_TT_NULL:       return "null"; 
    }
    return "undefined";
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
        case JSI_OT_UNDEF:      return "any";
    }
    return "undefined";
}

const char *jsi_ValueTypeName(Jsi_Interp *interp, Jsi_Value *val)
{
    switch (val->vt) {
        case JSI_VT_NUMBER:     return "number"; 
        case JSI_VT_STRING:     return "string"; 
        case JSI_VT_BOOL:       return "boolean"; 
        case JSI_VT_OBJECT:     if (val->d.obj->ot == JSI_OT_OBJECT && val->d.obj->isarrlist) return "array"; return jsi_ObjectTypeName(interp, val->d.obj->ot); 
        case JSI_VT_VARIABLE:   return "variable"; 
        case JSI_VT_NULL:       return "null"; 
        case JSI_VT_UNDEF:      break;
    }
    return "undefined";
}

int jsi_typeGet(Jsi_Interp *interp, const char *tname) {
    if (!tname)
        return 0;
    if (Jsi_Strchr(tname, '|')) {
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
        case 'b': if (Jsi_Strcmp(tname, "boolean")==0) return JSI_TT_BOOLEAN; break;
        case 's': if (Jsi_Strcmp(tname, "string")==0) return JSI_TT_STRING; break;
        case 'n': if (Jsi_Strcmp(tname, "null")==0) return JSI_TT_NULL;
                  if (Jsi_Strcmp(tname, "number")==0) return JSI_TT_NUMBER; break;
        case 'o': if (Jsi_Strcmp(tname, "object")==0) return JSI_TT_OBJECT; break;
        case 'r': if (Jsi_Strcmp(tname, "regexp")==0) return JSI_TT_REGEXP; break;
        case 'f': if (Jsi_Strcmp(tname, "function")==0) return JSI_TT_FUNCTION; break;
        case 'i': if (Jsi_Strcmp(tname, "iterobj")==0) return JSI_TT_ITEROBJ;
        case 'u': if (Jsi_Strcmp(tname, "userobj")==0) return JSI_TT_USEROBJ;
                  if (Jsi_Strcmp(tname, "undefined")==0) return JSI_TT_UNDEFINED; break;
        case 'a': if (Jsi_Strcmp(tname, "array")==0) return JSI_TT_ARRAY;
                  if (Jsi_Strcmp(tname, "any")==0) return JSI_TT_ANY; break;
        case 'v': if (Jsi_Strcmp(tname, "void")==0) return JSI_TT_VOID; break;
    }
    Jsi_LogWarn("Type \"%s\" is not one of boolean, string, number, function, array, object, regexp, userobj, null, undefined, void or any", tname);
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
    if (typ&JSI_TT_BOOLEAN)  Jsi_DSAppend(dStr, (i++?"|":""), "boolean", NULL);
    if (typ&JSI_TT_ARRAY)   Jsi_DSAppend(dStr, (i++?"|":""), "array", NULL);
    if (typ&JSI_TT_FUNCTION) Jsi_DSAppend(dStr, (i++?"|":""), "function", NULL);
    if (typ&JSI_TT_OBJECT) Jsi_DSAppend(dStr, (i++?"|":""), "object", NULL);
    if (typ&JSI_TT_REGEXP) Jsi_DSAppend(dStr, (i++?"|":""), "regexp", NULL);
    if (typ&JSI_TT_USEROBJ) Jsi_DSAppend(dStr, (i++?"|":""), "userobj", NULL);
    if (typ&JSI_TT_ITEROBJ) Jsi_DSAppend(dStr, (i++?"|":""), "iterobj", NULL);
    if (typ&JSI_TT_NULL) Jsi_DSAppend(dStr, (i++?"|":""), "null", NULL);
    if (typ&JSI_TT_UNDEFINED) Jsi_DSAppend(dStr, (i++?"|":""), "undefined", NULL);
    if (typ&JSI_TT_VOID) Jsi_DSAppend(dStr, (i++?"|":""), "void", NULL);
    return Jsi_DSValue(dStr);
}

const char* jsi_FuncGetCode(Jsi_Interp *interp, Jsi_Func *func, int *lenPtr) {
    if (interp->subOpts.noFuncString || !func->bodyStr)
        return NULL;
    const char *cp, *cp2;
    if (func->startPos == -1) {
        cp = func->bodyStr;
        int cplin = func->bodyline.last_line-1;
        while (*cp && cplin>0) {
            if (*cp=='\n' && --cplin<=0)
                break;
            cp++;
        }
        while (*cp && isspace(*cp))
            cp++;
        func->startPos = (*cp?(cp - func->bodyStr):-2);
    }
    if (func->startPos >= 0) {
        int len = func->endPos - func->startPos;
        cp = func->bodyStr + func->startPos;
        while (len>0 && (isspace(cp[len-1]) || cp[len-1]==';')) len--;
        if (*cp != 'f' && Jsi_Strncmp(cp, "function", 8) && (cp2=Jsi_Strstr(cp, "function"))) {
            len -= (cp2-cp);
            cp = cp2;
        }
        *lenPtr = len;
        return cp;
    }
    return NULL;
}

const char *jsiFuncInfo(Jsi_Interp *interp, Jsi_DString *dStr, Jsi_Func* func, Jsi_Value *arg) {
    if (!func) return "";
    if (func->name)
        Jsi_DSPrintf(dStr, ", in call to '%s'", func->name);
    else
        Jsi_DSPrintf(dStr, ", in call to function");
    if (func->script) {
        const char *cp = Jsi_Strrchr(func->script, '/');
        if (cp)
            cp++;
        else
            cp = func->script;
        Jsi_DSPrintf(dStr, " declared at %s:%d.%d", cp, func->bodyline.first_line, func->bodyline.first_column);
    }
        if (arg) {
        Jsi_DSAppend(dStr, " <", NULL);
        Jsi_ValueGetDString(interp, arg, dStr, 0);
        Jsi_DSAppend(dStr, ">.", NULL);
    }
    return Jsi_DSValue(dStr);
}

// Check argument matches type.  If func is null, this is a parse. An index of 0 is the return value.
Jsi_RC jsi_ArgTypeCheck(Jsi_Interp *interp, int typ,  Jsi_Value *arg, const char *p1,
    const char *p2, int index, Jsi_Func *func, bool isdefault) {
    Jsi_RC rc = JSI_OK;
    char idxBuf[200];
    idxBuf[0] = 0;
    if (func && arg->vt == JSI_VT_UNDEF && !interp->typeCheck.noundef && index>0 && !isdefault && !(typ&JSI_TT_UNDEFINED)) {
        snprintf(idxBuf, sizeof(idxBuf), " arg %d", index);
        jsi_TypeMismatch(interp);
       
        Jsi_DString fStr = {};
        rc = Jsi_LogType("call with undefined var %s%s '%s'%s", p1, idxBuf, p2, jsiFuncInfo(interp, &fStr, func, arg));
        Jsi_DSFree(&fStr);
        return rc;
    }
    if (typ <= 0)
        return JSI_OK;
    //if (typ&JSI_TT_VOID)
    //    return JSI_OK;
    if (interp->typeCheck.all==0) {
        if (func ? (interp->typeCheck.run==0) : (interp->typeCheck.parse==0))
            return JSI_OK;
    }
    if (index == 0 && func && func->type == FC_BUILDIN && 
        interp->typeCheck.all == 0) // Normally do not check return types for builtins.
        return JSI_OK; 
    if ((typ&JSI_TT_ANY)) return JSI_OK;
    if (index == 0 && arg->vt == JSI_VT_UNDEF) {
        if (!(typ&JSI_TT_VOID)) 
            goto done;
        return JSI_OK;
    }
    if (isdefault && index && typ&JSI_TT_VOID && arg->vt == JSI_VT_UNDEF)
        return JSI_OK;
    if (typ&JSI_TT_UNDEFINED && Jsi_ValueIsUndef(interp, arg)) return rc;
    if (typ&JSI_TT_NUMBER && Jsi_ValueIsNumber(interp, arg)) return rc;
    if (typ&JSI_TT_STRING && Jsi_ValueIsString(interp, arg)) return rc;
    if (typ&JSI_TT_BOOLEAN && Jsi_ValueIsBoolean(interp, arg))  return rc;
    if (typ&JSI_TT_ARRAY && Jsi_ValueIsArray(interp, arg))   return rc;
    if (typ&JSI_TT_FUNCTION && Jsi_ValueIsFunction(interp, arg)) return rc;
    if (typ&JSI_TT_REGEXP && Jsi_ValueIsObjType(interp, arg, JSI_OT_REGEXP)) return rc;
    if (typ&JSI_TT_USEROBJ && Jsi_ValueIsObjType(interp, arg, JSI_OT_USEROBJ)) return rc;
    if (typ&JSI_TT_ITEROBJ && Jsi_ValueIsObjType(interp, arg, JSI_OT_ITER)) return rc;
    if (typ&JSI_TT_OBJECT && (
        Jsi_ValueIsObjType(interp, arg, JSI_OT_OBJECT) && Jsi_ValueIsArray(interp, arg)==0))
        return rc;
    if (typ&JSI_TT_NULL && Jsi_ValueIsNull(interp, arg)) return rc;
done:
    {
        Jsi_DString dStr = {};
        const char *exp = jsi_typeName(interp, typ, &dStr);
        const char *vtyp = jsi_ValueTypeName(interp, arg);
        if (index>0)
            snprintf(idxBuf, sizeof(idxBuf), " arg %d", index);
        if (interp->typeCheck.error)
            rc = JSI_ERROR;
        jsi_TypeMismatch(interp);
        Jsi_DString fStr = {};
        rc = Jsi_LogType("type mismatch %s%s '%s': expected \"%s\" but got \"%s\"%s",
            p1, idxBuf, p2, exp, vtyp, jsiFuncInfo(interp, &fStr, func, arg));
        Jsi_DSFree(&fStr);
        Jsi_DSFree(&dStr);
    }
    return rc;
}

Jsi_RC jsi_StaticArgTypeCheck(Jsi_Interp *interp, int atyp, const char *p1, const char *p2, int index, Jsi_Func *func, jsi_Pline *line) {
    Assert(index>0);
    Jsi_RC rc;
    if (interp->typeCheck.parse==0 && interp->typeCheck.all==0)
        return JSI_OK;
    int ai = index-1+func->callflags.bits.addargs;
    if (func->argnames == NULL || ai>=func->argnames->count || ai<0)
        return JSI_OK;
    int typ = func->argnames->args[ai].type;
    if (typ <= 0)
        return JSI_OK;
    if (index == 0 && func && func->type == FC_BUILDIN && 
        interp->typeCheck.all==0) // Normally do not check return types for builtins.
        return JSI_OK; 
    if ((typ&JSI_TT_ANY)) return JSI_OK;
    if (index == 0 && atyp == JSI_VT_UNDEF) {
        if (!(typ&JSI_TT_VOID)) 
            goto done;
        return JSI_OK;
    }
/*    if (index == 0 && (typ&JSI_TT_VOID)) {
        if (atyp != JSI_VT_UNDEF && !(typ&JSI_TT_UNDEFINED))
            goto done;
        return JSI_OK;
    }*/
    if (atyp == JSI_VT_UNDEF)
        return JSI_OK;
    rc = JSI_OK;
    if (typ&JSI_TT_UNDEFINED && atyp == JSI_TT_UNDEFINED) return rc;
    if (typ&JSI_TT_NUMBER && atyp==JSI_TT_NUMBER) return rc;
    if (typ&JSI_TT_STRING && atyp==JSI_TT_STRING) return rc;
    if (typ&JSI_TT_BOOLEAN && atyp==JSI_TT_BOOLEAN)  return rc;
    if (typ&JSI_TT_ARRAY && atyp==JSI_TT_ARRAY)   return rc;
    if (typ&JSI_TT_FUNCTION && atyp==JSI_TT_FUNCTION) return rc;
    if (typ&JSI_TT_REGEXP && atyp==JSI_TT_REGEXP) return rc;
    if (typ&JSI_TT_USEROBJ && atyp==JSI_TT_USEROBJ) return rc;
    if (typ&JSI_TT_ITEROBJ && atyp==JSI_TT_ITEROBJ) return rc;
    if (typ&JSI_TT_OBJECT && atyp==JSI_TT_OBJECT) return rc;
    if (typ&JSI_TT_NULL && atyp==JSI_TT_NULL) return rc;
done:
    {
        Jsi_DString dStr = {};
        const char *exp = jsi_typeName(interp, typ, &dStr);
        const char *vtyp = jsi_TypeName(interp, (Jsi_ttype)atyp);
        char idxBuf[200];
        idxBuf[0] = 0;
        if (index>0)
            snprintf(idxBuf, sizeof(idxBuf), " arg %d", index);
        if (line)
            interp->parseLine = line;
        if (interp->typeCheck.error)
            rc = JSI_ERROR;
        jsi_TypeMismatch(interp);
        Jsi_DString fStr = {};
        rc = Jsi_LogType("type mismatch %s%s '%s': expected \"%s\" but got \"%s\"%s",
            p1, idxBuf, p2, exp, vtyp, jsiFuncInfo(interp, &fStr, func, NULL));
        Jsi_DSFree(&fStr);
        Jsi_DSFree(&dStr);
    }
    return rc;
}

int jsiPopArgs(Jsi_OpCodes *argCodes, int i)
{
    int m=i-1, n = (uintptr_t)argCodes->codes[i].data, cnt = 0;
    if (argCodes->codes[i].op == OP_OBJECT)
        n *= 2;
    for (; m>=0 && cnt<n; m--, cnt++) {
        int op = argCodes->codes[m].op;
        if (op == OP_ARRAY || op == OP_OBJECT)
            m = jsiPopArgs(argCodes, m);
    }
    return m+1;
}

Jsi_RC jsi_RunFuncCallCheck(Jsi_Interp *interp, Jsi_Func *func, int argc, const char *name, jsi_Pline *line, Jsi_OpCodes *argCodes, bool isParse)
{
    Jsi_RC rc = JSI_OK;
    if (interp->typeCheck.all==0) {
        if (!argCodes ? (interp->typeCheck.run==0) : (interp->typeCheck.parse==0))
            return JSI_OK;
    }

    Jsi_CmdSpec *spec = func->cmdSpec;
    Jsi_ScopeStrs *ss = func->argnames;
    if (ss==NULL && spec == NULL)
        return JSI_OK;
    int i, minArgs, maxArgs, mis = 0, varargs = 0;
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
        if (func->retType == 0 && ss && ss->typeCnt == 0 && interp->typeCheck.all==0)
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
        if (interp->typeCheck.error)
            rc = JSI_ERROR;
        Jsi_DString dStr = {};
        Jsi_FuncObjToString(interp, func, &dStr, 2);
        if (isParse)
            Jsi_LogWarn("got %d args, expected %s, calling %s", argc, nbuf, Jsi_DSValue(&dStr));
        else
            rc = Jsi_LogType("got %d args, expected %s, calling %s", argc, nbuf, Jsi_DSValue(&dStr));
        jsi_TypeMismatch(interp);
        Jsi_DSFree(&dStr);
        if (line)
            interp->parseLine = NULL;
        return rc;
    }
    if (argCodes && argCodes->code_len>=argc) {
        int cl = argCodes->code_len;
        int aind=argc-1;
        for (i=cl-1; rc == JSI_OK && i>=0 && aind>=0; i--,aind--) {
            Jsi_ttype atyp = JSI_TT_ANY;
            switch (argCodes->codes[i].op) {
                case OP_PUSHSTR: atyp=JSI_TT_STRING; break;
                case OP_PUSHNUM: atyp=JSI_TT_NUMBER; break;
                case OP_PUSHBOO: atyp=JSI_TT_BOOLEAN; break;
                case OP_PUSHFUN: atyp=JSI_TT_FUNCTION; break;
                case OP_PUSHTHS: atyp=JSI_TT_OBJECT; break;
                case OP_PUSHREG: atyp=JSI_TT_REGEXP; break;
                case OP_PUSHUND: atyp=JSI_TT_VOID; break;
                case OP_PUSHNULL: atyp=JSI_TT_NULL; break;
                case OP_PUSHARG: atyp=JSI_TT_ARRAY; break;
                case OP_SUBSCRIPT: i++; break;
                case OP_ARRAY: atyp=JSI_TT_ARRAY; i=jsiPopArgs(argCodes, i); break;
                case OP_OBJECT: atyp=JSI_TT_OBJECT; i=jsiPopArgs(argCodes, i); break;
                default: break;
            }
            if (atyp == JSI_TT_ANY) continue;
            rc = jsi_StaticArgTypeCheck(interp, atyp, "for argument", name, aind+1, func, line);  
        }
    }
    return rc;
}

int jsi_BuiltinCmd(Jsi_Interp *interp, const char *name)
{
    Jsi_Value *val = Jsi_NameLookup(interp, name);
    if (!name)
        return 0;
    if (!Jsi_ValueIsFunction(interp, val))
        return 0;
    Jsi_Func *f = val->d.obj->d.fobj->func;
    return (f->type == FC_BUILDIN);
}

// Parse time function call checker.
void jsi_FuncCallCheck(jsi_Pstate *p, jsi_Pline *line, int argc, bool isNew, const char *name, const char *namePre, Jsi_OpCodes *argCodes)
{
    Jsi_Interp *interp = p->interp;
    if (name == NULL || !(interp->typeCheck.funcsig|interp->typeCheck.all|interp->typeCheck.parse))
        return;
    if (name && isdigit(name[0]))
        return;
    Jsi_Value *val;
    val = Jsi_NameLookup2(interp, name, namePre);
    Jsi_Func *f = NULL;
    if (val != NULL) {
        if (Jsi_ValueIsFunction(interp, val))
            f = val->d.obj->d.fobj->func;
    } else if (interp->staticFuncsTbl) {
        f = (Jsi_Func*)Jsi_HashGet(interp->staticFuncsTbl, (void*)name, 0);
    }
    if (f)
        jsi_RunFuncCallCheck(interp, f, argc, name, line, argCodes, 1);
    else if (interp->typeCheck.funcsig && (namePre==NULL || jsi_BuiltinCmd(interp, namePre))) {
        if (line)
            interp->parseLine = line;
        Jsi_LogWarn("called function '%s' with no previous definition", name);
        jsi_TypeMismatch(interp);
        if (line)
            interp->parseLine = NULL;
    }
}

int jsi_FuncSigsMatch(jsi_Pstate *pstate, Jsi_Func *f1, Jsi_Func *f2)
{
    // Skip where both functions have no types.
    if (f1->retType==0 && f1->argnames->typeCnt==0 && f1->argnames->varargs==0 &&
        f2->retType==0 && f2->argnames->typeCnt==0 && f2->argnames->varargs==0 &&
        pstate->interp->typeCheck.all==0)
        return 1;
    if (f1->retType != f2->retType)
        return 0;
    if (f1->argnames->count != f2->argnames->count)
        return 0;
    if (f1->argnames->typeCnt != f2->argnames->typeCnt)
        return 0;
    if (f1->argnames->varargs != f2->argnames->varargs)
        return 0;
    int i;
    for (i=0; i<f1->argnames->count; i++) {
        Jsi_ScopeStrs *a1 = f1->argnames, *a2 = f2->argnames;
        if (a1->args[i].type != a2->args[i].type)
            return 0;
        Jsi_Value *v1, *v2;
        v1 = a1->args[i].defValue;
        v2 = a2->args[i].defValue;
        if (v1==NULL && v2 == NULL)
            continue;
        if (v1==NULL || v2 == NULL)
            return 0;
        if (v1->vt != v2->vt)
            return 0;
        if (Jsi_ValueCmp(pstate->interp, v1, v2, 0))
            return 0;
    }
    return 1;
}

// Return directive from first instruction.
const char* jsi_GetDirective(Jsi_Interp *interp, Jsi_OpCodes *ops, const char *str) {
    if (!ops) return NULL;
    if (!ops->code_len) return NULL;
    if (ops->codes[0].op != OP_PUSHSTR || !ops->codes[0].data) return NULL;
    if (Jsi_Strncmp((char*)ops->codes[0].data, str, Jsi_Strlen(str))) return NULL;
    return (char*)ops->codes[0].data;
}

/* TODO: if not in a file (an eval) save copy of body string from pstate->lexer??? */
Jsi_Func *jsi_FuncMake(jsi_Pstate *pstate, Jsi_ScopeStrs *args, Jsi_OpCodes *ops, jsi_Pline* line, const char *name, int isArrow)
{
    Jsi_Interp *interp = pstate->interp;
    Jsi_ScopeStrs *localvar = jsi_ScopeGetVarlist(pstate);
    Jsi_Func *f = jsi_FuncNew(interp);
    jsi_Lexer *l = pstate->lexer;
    if (isArrow)
        f->isArrow = isArrow;
    f->type = FC_NORMAL;
    f->opcodes = ops;
    f->argnames = args;
    f->localnames = localvar;
    f->script = interp->curFile;
    f->bodyline = *line;
    f->retType = (Jsi_otype)args->retType;
    if (!pstate->eval_flag) {
        f->scriptFile = f->script;
    }
    if (l->ltype == LT_STRING)
        f->bodyStr = l->d.str;
    f->endPos = l->cur;
    f->startPos = -1; // Have to get these from newline count.
    if (f->retType & JSI_TT_UNDEFINED)
        Jsi_LogWarn("illegal use of 'undefined' in a return type: %s", name?name:"");
    
    //f->strict = (jsi_GetDirective(interp, ops, "use strict") != NULL);
    pstate->argType = 0;
    if (localvar && args && (interp->strict)) {
        int i, j;
        for (i=0; i<args->count; i++) {
            for (j=0; j<args->count; j++) {
                if (i != j && !Jsi_Strcmp(args->args[i].name, args->args[j].name)) {
                        if (line)
                            interp->parseLine = line;
                        Jsi_LogWarn("function %s():  duplicate parameter name '%s'", name?name:"", args->args[i].name);
                        if (line)
                            interp->parseLine = NULL;
                        jsi_TypeMismatch(interp);
                        if (interp->typeCheck.error)
                            pstate->err_count++;
                }
            }
            for (j=0; j<localvar->count; j++) {
                if (!Jsi_Strcmp(localvar->args[j].name, args->args[i].name)) {
                        if (line)
                            interp->parseLine = line;
                        Jsi_LogWarn("function %s():  parameter name conflicts with 'var %s'", name?name:"", localvar->args[j].name);
                        if (line)
                            interp->parseLine = NULL;
                        jsi_TypeMismatch(interp);
                        if (interp->typeCheck.error)
                            pstate->err_count++;
                }
            }
        }
    }
    if (name) {
        f->name = Jsi_KeyAdd(interp, name);
        if ((interp->typeCheck.run|interp->typeCheck.parse|interp->typeCheck.all|interp->typeCheck.funcsig)) {
            
            if (f->retType && !(f->retType&JSI_TT_VOID) && ops && ops->code_len && ops->codes[ops->code_len-1].op != OP_RET) {
                if (line)
                    interp->parseLine = line;
                Jsi_LogWarn("missing return at end of function '%s'", name);
                if (line)
                    interp->parseLine = NULL;
                //if (interp->typeCheck.error)
                 //   pstate->err_count++;
            }
             
            if (interp->staticFuncsTbl) {
                Jsi_Func *fo = (Jsi_Func*)Jsi_HashGet(interp->staticFuncsTbl, (void*)name, 0);
                
                // Forward declaration signature compare (indicated by an empty body).
                if (interp->typeCheck.funcsig && fo && fo->opcodes && fo->opcodes->code_len == 1 && fo->opcodes->codes->op == OP_NOP) {
                    if (!jsi_FuncSigsMatch(pstate, f, fo)) {
                        if (line)
                            interp->parseLine = line;
                        Jsi_LogWarn("possible signature mismatch for function '%s' at %.120s:%d", name, fo->script, fo->bodyline.first_line);
                        if (line)
                            interp->parseLine = NULL;
                        jsi_TypeMismatch(interp);
                    }
                    //printf("OLD: %s\n", name);
                }
                Jsi_HashSet(interp->staticFuncsTbl, name, f);
            }
        }
    }
    return f;
}

Jsi_RC Jsi_FunctionArguments(Jsi_Interp *interp, Jsi_Value *func, int *argcPtr)
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

bool jsi_FuncIsNoop(Jsi_Interp* interp, Jsi_Value *func) {
    Jsi_FuncObj *funcPtr;
    Jsi_Func *f;
    if (func->vt != JSI_VT_OBJECT || func->d.obj->ot != JSI_OT_FUNCTION)
        return 0;
    funcPtr = func->d.obj->d.fobj;
    f = funcPtr->func;
    return (f->callback == jsi_NoOpCmd);
}

void jsi_InitLocalVar(Jsi_Interp *interp, Jsi_Value *arguments, Jsi_Func *who)
{
    SIGASSERTV(who, FUNC);
    if (who->localnames) {
        int i;
        for (i = 0; i < who->localnames->count; ++i) {
            const char *argkey = jsi_ScopeStrsGet(who->localnames, i);
            if (argkey) {
                DECL_VALINIT(key);// = VALINIT;
                Jsi_Value *v __attribute__((unused));
                Jsi_Value *kPtr = &key; // Note: a string key so no reset needed.
                Jsi_ValueMakeStringKey(interp, &kPtr, argkey);
                v = jsi_ValueObjKeyAssign(interp, arguments, kPtr, NULL, JSI_OM_DONTENUM);
                jsi_ValueDebugLabel(v, "locals", who->name);
            }
        }
    }
}

Jsi_RC jsi_FuncArgsToString(Jsi_Interp *interp, Jsi_Func *f, Jsi_DString *dStr, int withTypes)
{
    if (f->type == FC_NORMAL) {
        int i;
        for (i = 0; i < f->argnames->count; ++i) {
            jsi_ArgValue *av = f->argnames->args+i;
            if (i) Jsi_DSAppend(dStr, ", ", NULL);
            Jsi_DSAppend(dStr,  jsi_ScopeStrsGet(f->argnames, i), NULL);
            if (withTypes && av) {
                Jsi_DString tStr = {};
                int atyp = av->type;
                if (av->defValue)
                    atyp &= ~(av->defValue->vt==JSI_VT_NULL?JSI_TT_NULL:(1<<av->defValue->vt));
                if (atyp) {
                    Jsi_DSAppend(dStr, ":", jsi_typeName(interp, atyp, &tStr), NULL);
                }
                Jsi_DSSetLength(&tStr, 0);
                if (av->defValue)
                    Jsi_DSAppend(dStr, "=", Jsi_ValueGetDString(interp, av->defValue, &tStr, 1), NULL);
                Jsi_DSFree(&tStr);
            }
        }
    } else if (f->cmdSpec && f->cmdSpec->argStr)
        Jsi_DSAppend(dStr, f->cmdSpec->argStr, NULL);
    return JSI_OK;
}

Jsi_RC Jsi_FuncObjToString(Jsi_Interp *interp, Jsi_Func *f, Jsi_DString *dStr, int flags)
{
    int withBody = flags&1;
    int withTypes = flags&2;
    int withJSON = flags&4;
    int withFull = (flags&8 && !withJSON);
    if (withFull && f->type == FC_NORMAL && f->opcodes) {
        int len;
        const char *cp = jsi_FuncGetCode(interp, f, &len);
        if (cp) {
            Jsi_DSAppendLen(dStr,cp, len);
            return JSI_OK;
        }
    }
    Jsi_CmdSpec *spec = f->cmdSpec;
    if (withJSON)
        Jsi_DSAppend(dStr, "\"", NULL);
    if (f->type == FC_NORMAL) {
        Jsi_DSAppend(dStr, "function ", f->name?f->name:"", "(", NULL);
        jsi_FuncArgsToString(interp, f, dStr, withTypes);
        Jsi_DSAppend(dStr, ")", NULL);
        if (withTypes && f->retType) {
            Jsi_DString tStr;
            Jsi_DSInit(&tStr);
            Jsi_DSAppend(dStr, ":", jsi_typeName(interp, f->retType, &tStr), NULL);
            Jsi_DSFree(&tStr);
        }
        if (withBody)
            Jsi_DSAppend(dStr, " {...}", NULL);
    } else {
        Jsi_DSAppend(dStr, "function ", f->name?f->name:"", "(",
            (spec&&spec->argStr)?spec->argStr:"", ")", NULL);
        if (withBody)
            Jsi_DSAppend(dStr, " { [native code] }", NULL);
    }
    if (withJSON)
        Jsi_DSAppend(dStr, "\"", NULL);
    return JSI_OK;
}

Jsi_Value *jsi_MakeFuncValue(Jsi_Interp *interp, Jsi_CmdProc *callback, const char *name, Jsi_Value** toVal, Jsi_CmdSpec *cspec)
{
    Jsi_Obj *o = Jsi_ObjNew(interp);
    Jsi_Func *f = jsi_FuncNew(interp);
    Jsi_ObjIncrRefCount(interp, o);
    o->ot = JSI_OT_FUNCTION;
    f->type = FC_BUILDIN;
    f->callback = callback;
    f->privData = NULL;
    o->d.fobj = jsi_FuncObjNew(interp, f);
    f->cmdSpec = cspec;
    if (!cspec) {
        f->cmdSpec = (Jsi_CmdSpec*)Jsi_Calloc(2,sizeof(Jsi_CmdSpec));
        f->cmdSpec->reserved[3] = (void*)0x1;
        f->cmdSpec->maxArgs = -1;
        if (name)
            f->cmdSpec->name = (char*)Jsi_KeyAdd(interp, name);
    }
    f->script = interp->curFile;
    f->callback = callback;
    return Jsi_ValueMakeObject(interp, toVal, o);
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
static Jsi_RC jsi_FunctionInvoke(Jsi_Interp *interp, Jsi_Value *tocall, Jsi_Value *args, Jsi_Value **ret, Jsi_Value *_this)
{
    if (interp->maxDepth>0 && interp->maxDepth && interp->callDepth>=interp->maxDepth)
        return Jsi_LogError("max call depth exceeded");
    if (interp->deleting)
        return JSI_ERROR;
    if (!Jsi_ValueIsFunction(interp, tocall)) 
        return Jsi_LogError("can not execute expression, expression is not a function");
    if (!tocall->d.obj->d.fobj) {   /* empty function */
        return JSI_OK;
    }
    if (!ret) {
        if (!interp->nullFuncRet) {
            interp->nullFuncRet = Jsi_ValueNew(interp);
            Jsi_IncrRefCount(interp, interp->nullFuncRet);
        }
        ret = &interp->nullFuncRet;
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
    Jsi_Func *funcPtr = tocall->d.obj->d.fobj->func;
    SIGASSERT(funcPtr, FUNC);
    
    /* prepare args */
    if (args->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, args->d.obj)) 
        return Jsi_LogError("argument must be an array");
    /* new this */
    Jsi_Value *fthis = Jsi_ValueDup(interp, _this ? _this : tocall);
    Jsi_Func *prevActive = interp->activeFunc;
    Jsi_RC res = jsi_SharedArgs(interp, args, funcPtr, 1);
    bool isalloc = 0;
    int calltrc = 0;
    int tc = interp->traceCall | (funcPtr->pkg?funcPtr->pkg->popts.modConf.traceCall:0);
    interp->callDepth++;
    if (res == JSI_OK) {
        jsi_InitLocalVar(interp, args, funcPtr);
        jsi_SetCallee(interp, args, tocall);
        isalloc = 1;
        Jsi_IncrRefCount(interp, args);
        if (funcPtr->type == FC_NORMAL) {
            if ((tc&jsi_callTraceFuncs) && funcPtr->name)
                calltrc = 1;
        } else {
            if ((tc&jsi_callTraceCmds) && funcPtr->name)
                calltrc = 1;
        }
        interp->activeFunc = funcPtr;
        if (funcPtr->type == FC_NORMAL) {
            if (calltrc)
                jsi_TraceFuncCall(interp, funcPtr, NULL, fthis, args, NULL, tc);
            res = jsi_evalcode(interp->ps, funcPtr, funcPtr->opcodes, tocall->d.obj->d.fobj->scope, 
                args, fthis, ret);
        } else {
            if (calltrc)
                jsi_TraceFuncCall(interp, funcPtr, NULL, fthis, args, NULL, tc);
            res = funcPtr->callback(interp, args, fthis, ret, funcPtr);
        }
        funcPtr->callCnt++;
    }
    interp->callDepth--;
    if (res == JSI_OK && funcPtr->retType)
        res = jsi_ArgTypeCheck(interp, funcPtr->retType, *ret, "returned from", funcPtr->name, 0, funcPtr, 0);
    if (calltrc && (tc&jsi_callTraceReturn))
        jsi_TraceFuncCall(interp, funcPtr, NULL, fthis, NULL, *ret, tc);
    interp->activeFunc = prevActive;
    jsi_SharedArgs(interp, args, funcPtr, 0);
    if (isalloc) 
        Jsi_DecrRefCount(interp, args);
    Jsi_DecrRefCount(interp, fthis);
    return res;
}

Jsi_RC Jsi_FunctionInvoke(Jsi_Interp *interp, Jsi_Value *func, Jsi_Value *args, Jsi_Value **ret, Jsi_Value *_this)
{
    // Arrange for error reporting to point to called function.
    Jsi_Func *fstatic = func->d.obj->d.fobj->func;
    jsi_OpCode *oldops = interp->curIp;
    if (fstatic->opcodes)
        interp->curIp = fstatic->opcodes->codes;
    Jsi_RC rc = jsi_FunctionInvoke(interp, func, args, ret, _this);
    interp->curIp = oldops;
    if (Jsi_InterpGone(interp))
        return JSI_ERROR;
    return rc;
}

// Do typechecking for callback using argStr from .data in builtin Jsi_Options: may not use = or ...
bool jsi_FuncArgCheck(Jsi_Interp *interp, Jsi_Func *f, const char *argStr)
{
    int i, atyp, ftyp, rc = 0, acnt;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    int argc = 0;
    char **argv, *sname, *stype, *cp;
    if (!argStr)
        goto done;
    if (f->type == FC_BUILDIN) {
        // Check builtin cmd
        jsi_CommandArgCheck(interp, f->cmdSpec, f, f->parentName);
        goto done;
    }
    if ((cp=Jsi_Strchr(argStr, '='))) {
        Jsi_LogWarn("may not have default value in option, expected: %s", argStr);
        goto done;
    }
    if (Jsi_Strstr(argStr, "...")) {
        Jsi_LogWarn("may not have ... in args, expected: %s", argStr);
        goto done;
    }
    if (argStr[0]) {
        Jsi_SplitStr(argStr, &argc, &argv, ",", &dStr);
        if (argc<=0)
            goto done;
    }
    if (!f->argnames) {
        if (argStr[0])
            Jsi_LogWarn("function has no args, expected: %s", argStr);
        else
            rc = 1;
        goto done;
    } else {
        if (f->argnames->varargs) { // TODO: could allow varargs...
            if (argc < f->argnames->argCnt) {
                Jsi_LogWarn("vararg argument mismatch, expected: %s", argStr);
                goto done;
            }
        }
        else if (f->argnames->argCnt != argc) {
            if (argc)
                Jsi_LogWarn("argument mismatch, expected: %s", argStr);
            else
                Jsi_LogWarn("function should have no arguments");
            goto done;
        }

    }
    acnt = f->argnames->argCnt;
    for (i=0; i<argc && i<acnt; i++) {
        sname = argv[i];
        stype = NULL;
        while (sname && *sname && isspace(*sname)) { sname++; }
        if ((cp=Jsi_Strchr(sname, ':')))
        {
            stype = cp+1;
            *cp = 0;
            while (*stype && isspace(*stype)) { stype++; }
            if (*stype) {
                cp = stype+Jsi_Strlen(stype)-1;
                while (cp>=stype && isspace(*cp)) { *cp = 0; cp--; }
            }
        }
        if (sname && *sname) {
            cp = sname+Jsi_Strlen(sname)-1;
            while (cp>=sname && isspace(*cp)) { *cp = 0; cp--; }
        }
        ftyp = f->argnames->args[i].type;
        if (ftyp<=0 || (ftyp&JSI_TT_ANY))
            continue;
        atyp = jsi_typeGet(interp, stype);
        if (ftyp != atyp && atyp) {
            Jsi_LogWarn("argument %d of function \"%s\" does not match \"func(%s)\"" ,
                i+1, f->name, argStr);
            goto done;
        }
    }
    rc = 1;
done:
    Jsi_DSFree(&dStr);
    if (!rc)
        jsi_TypeMismatch(interp);
    return rc;
}

/* Call function that returns a bool with a single argument. Returns -1, else 0/1 for false/true,  */
int Jsi_FunctionInvokeBool(Jsi_Interp *interp, Jsi_Value *func, Jsi_Value *arg)
{
    if (interp->deleting)
        return JSI_ERROR;
    Jsi_Value *vpargs, *frPtr = Jsi_ValueNew1(interp);
    Jsi_RC rc;
    int bres = 0;
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
    else {
        bres = 2;
        Jsi_LogError("function call failed");
    }
    Jsi_DecrRefCount(interp, frPtr);
    if (Jsi_InterpGone(interp))
        return JSI_ERROR;
    return bres;
}

// Invoke function with one string argument.
Jsi_RC Jsi_FunctionInvokeString(Jsi_Interp *interp, Jsi_Value *func, Jsi_Value *arg, Jsi_DString *dStr)
{
    if (interp->deleting)
        return JSI_ERROR;
    Jsi_Value *vpargs, *frPtr = Jsi_ValueNew1(interp);
    Jsi_RC rc;
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
    if (rc != JSI_OK)
        Jsi_LogError("function call failed");
    else
        Jsi_ValueGetDString(interp, frPtr, dStr, 0);
    Jsi_DecrRefCount(interp, frPtr);
    return rc;
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
    jsi_PkgInfo *pkg = func->pkg;
    bool profile = (interp->profile || (pkg?pkg->popts.modConf.profile:0)), 
        cover = (interp->coverage || (pkg?pkg->popts.modConf.coverage:0));
    if (profile || cover) {
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        const char *file = func->script;
        if (!file)
            file = "";
        int line = func->bodyline.last_line;
        if (!func->callCnt) {
            if (cover && func->type == FC_NORMAL && func->name)
                Jsi_DSPrintf(&dStr, "COVERAGE: func=%s  file=%s:%d  cover=0%%\n", func->name, file, line );
        } else {
            char buf[JSI_BUFSIZ];
            if (func->type == FC_BUILDIN) {
                const char *fnam = func->parentName;
                snprintf(buf, sizeof(buf), "cmd=%s%s%s", fnam, fnam[0]?".":"", func->name);
                interp->cmdSelfTime += (func->allTime-func->subTime);
            } else {
                
                double coverage = 0; // Calculate hits/all.
                if (func->opcodes && func->opcodes->code_len>0) {
                    int i, cchit=0, ccall=0, ccline=0, cchitline=0;
                    Jsi_OpCodes *oc = func->opcodes;
                    for (i=0; i<oc->code_len; i++) {
                        if (oc->codes[i].Line<=0) continue;
                        if (ccline != oc->codes[i].Line) {
                            ccline = oc->codes[i].Line;
                            ccall++;
                            interp->coverAll++;
                        }
                        if (cchitline != oc->codes[i].Line && oc->codes[i].hit) {
                            cchitline = oc->codes[i].Line;
                            cchit++;
                            interp->coverHit++;
                        }
                    }
                    if (ccall)
                        coverage = (int)(100.0*cchit/ccall);
                        
                    if (cover && cchit<ccall) { // Generate the misses list.
                        char cbuf[JSI_BUFSIZ];
                        int lastout = 0, lastpos=0, dupcnt=0, cccnt=0;
                        cbuf[0] = 0;
                        ccline=cchitline=0;
                        for (i=0; i<oc->code_len; i++) {
                            int ismiss = 0;
                            if (i==oc->code_len) {
                                ismiss = (ccline>0 && cchitline != ccline);
                            } else {
                                if (oc->codes[i].Line<=0) continue;
                                ismiss = (ccline>0 && ccline != oc->codes[i].Line && cchitline != ccline);
                            }
                            if (ismiss) {
                                cccnt++;
                                const char *sep = (cccnt>1?",":"");
                                if (lastout && (lastout+1)==ccline) {
                                    sep = "-";
                                    dupcnt++;
                                    if (dupcnt>1)
                                        cbuf[lastpos]=0; // Inefficient, but reliable.
                                    else
                                        lastpos = Jsi_Strlen(cbuf);
                                } else 
                                    dupcnt = 0;
                                int cbl = Jsi_Strlen(cbuf);
                                snprintf(cbuf+cbl, sizeof(cbuf)-cbl, "%s%d", sep, ccline);
                                lastout = ccline;
                            }
                            ccline = oc->codes[i].Line;
                            if (oc->codes[i].hit)
                                cchitline = ccline;
                        }
                        Jsi_DSPrintf(&dStr, "COVERAGE: func=%s  file=%s:%d  cover=%2.1f%%  hits=%d,  all=%d,  misses=%s\n", 
                            func->name, file, line, coverage, cchit, ccall, cbuf);
                    }
                }
                snprintf(buf, sizeof(buf), "cover=%#5.1f%%  func=%s file=%s:%d", coverage, func->name, file, line);
                interp->funcSelfTime += (func->allTime-func->subTime);
            }
            if (profile)
                Jsi_DSPrintf(&dStr, "PROFILE:  self=%6.6f  all=%6.6f  #calls=%-8d  self/call=%6.6f  all/call=%6.6f  %s %s%s\n",
                    (func->allTime-func->subTime), (double)(func->allTime), func->callCnt, 
                    (double)(func->allTime-func->subTime)/func->callCnt,  (double)(func->allTime)/func->callCnt, 
                    buf, interp->parent?" ::":"", (interp->parent&&interp->name?interp->name:""));
        }
        if (Jsi_DSLength(&dStr))
            Jsi_Puts(interp, jsi_Stderr, Jsi_DSValue(&dStr), -1);
        Jsi_DSFree(&dStr);
    }

    if (func->opcodes)
        jsi_FreeOpcodes(func->opcodes);
    if (func->hPtr)
        Jsi_HashEntryDelete(func->hPtr);
    if (func->localnames)
        jsi_ScopeStrsFree(interp, func->localnames);
    if (func->argnames)
        jsi_ScopeStrsFree(interp, func->argnames);
    if (func->cmdSpec && (intptr_t)func->cmdSpec->reserved[3]& 0x1)
        Jsi_Free(func->cmdSpec);
    _JSI_MEMCLEAR(func);
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
    if (fobj->bindArgs)
        Jsi_DecrRefCount(fobj->interp, fobj->bindArgs);
    if (fobj->bindFunc)
        Jsi_DecrRefCount(fobj->interp, fobj->bindFunc);
    if (fobj->func)
        jsi_FuncFree(fobj->interp, fobj->func);
    _JSI_MEMCLEAR(fobj);
    Jsi_Free(fobj);
}

#endif
