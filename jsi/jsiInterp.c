#ifndef JSI_LITE_ONLY
#define __JSIINT_C__
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#ifdef __WIN32
#include <windows.h>
#include <shlwapi.h>
#ifndef JSI_OMIT_THREADS
#include <process.h>
#endif
#else
#ifndef JSI_OMIT_THREADS
#include <pthread.h>
#endif
#endif

static int jsi_is_init = 0;
Jsi_Interp *jsiMainInterp = NULL;
Jsi_Interp *jsiDelInterp = NULL;
static Jsi_Hash *interpsTbl;

const char *jsi_TypeChkStrs[] = { "disable", "warn", "error", "static", NULL };
#define IIOF .flags=JSI_OPT_INIT_ONLY

static Jsi_OptionSpec InterpOptions[] = {
    JSI_OPT(ARRAY, Jsi_Interp, args,        .help="The console.arguments for interp", IIOF),
    JSI_OPT(BOOL,  Jsi_Interp, doUnlock,    .help="Unlock our mutex when evaling in other interps", IIOF, .init="true"),
    JSI_OPT(BOOL,  Jsi_Interp, noUndef,     .help="Suppress printing undefined value result when in interactive mode"),
    JSI_OPT(INT,   Jsi_Interp, callTrace,  .help="Echo method call/return value"),
    JSI_OPT(STRKEY,Jsi_Interp, evalCallback,.help="String name of callback function in parent to handle eval stepping", IIOF ),
    JSI_OPT(VALUE, Jsi_Interp, indexFiles,  .help="File(s) to source for loading index for unknown commands"),
    JSI_OPT(BOOL,  Jsi_Interp, noInherit,   .help="Disallow access to __proto__, prototype, constructor, etc"),
    JSI_OPT(BOOL,  Jsi_Interp, isSafe,      .help="Interp is safe (ie. no file access)", IIOF),
    JSI_OPT(INT,   Jsi_Interp, lockTimeout, .help="Timeout for mutex lock-acquire (milliseconds)" ),
    JSI_OPT(STRKEY,Jsi_Interp, logCallback, .help="String name of callback function in parent to handle logging", IIOF ),
    JSI_OPT(BOOL,  Jsi_Interp, logAllowDups,.help="Disable log duplicate filtering"),
    JSI_OPT(INT,   Jsi_Interp, maxDepth,    .help="Recursion call depth limit", .init="1000"),
    JSI_OPT(INT,   Jsi_Interp, maxIncDepth, .help="Max file include nesting limit", .init="50" ),
    JSI_OPT(INT,   Jsi_Interp, maxInterpDepth,.help="Max nested subinterp create limit", .init="10" ),
    JSI_OPT(INT,   Jsi_Interp, maxUserObjs, .help="Cap on number of 'new' object calls (eg. File, Regexp, etc)" ),
    JSI_OPT(INT,   Jsi_Interp, maxOpCnt,    .help="Execution cap on opcodes evaluated" ),
    JSI_OPT(INT,   Jsi_Interp, memDebug,    .help="Set memory debugging level 1=summary, 2=detail", .flags=JSI_OPT_NO_CLEAR),
    JSI_OPT(BOOL,  Jsi_Interp, nDebug,      .help="Make assert statements have no effect"),
    JSI_OPT(STRKEY,Jsi_Interp, name, .help="Name of interp", IIOF),
    JSI_OPT(BOOL,  Jsi_Interp, noreadline,  .help="Do not use readline in interactive mode", IIOF),
    JSI_OPT(FUNC,  Jsi_Interp, onComplete,  .help="Command return command completions" ),
    JSI_OPT(FUNC,  Jsi_Interp, onEval,      .help="Interactive eval" ),
    JSI_OPT(FUNC,  Jsi_Interp, onExit,      .help="Command to call in parent on exit (which returns true to continue)", IIOF ),
    JSI_OPT(INT,   Jsi_Interp, opTrace,     .help="Set debugging level"),
    JSI_OPT(BOOL,  Jsi_Interp, noSubInterps,.help="Disallow sub-interp creation", IIOF),
    JSI_OPT(BOOL,  Jsi_Interp, privKeys,    .help="Disable string key sharing with other interps", IIOF, .init="true"),
    JSI_OPT(STRKEY,Jsi_Interp, recvCmd,     .help="Name of function to recv 'send' msgs"),
    JSI_OPT(ARRAY, Jsi_Interp, safeReadDirs,.help="In safe mode, directories to allow reads from", IIOF),
    JSI_OPT(ARRAY, Jsi_Interp, safeWriteDirs,.help="In safe mode, directories to allow writes to", IIOF),
    JSI_OPT(STRKEY,Jsi_Interp, scriptStr,   .help="Startup script string", IIOF),
    JSI_OPT(VALUE, Jsi_Interp, scriptFile,  .help="Startup script file name", IIOF),
    JSI_OPT(BOOL,  Jsi_Interp, compat,      .help="If set to true, option parses ignore unknown options" ),
    JSI_OPT(BOOL,  Jsi_Interp, subthread,   .help="Create thread for interp", IIOF),
    JSI_OPT(CUSTOM,Jsi_Interp, typeCheck,   .help="Type-checking control", .custom=Jsi_Opt_SwitchEnum, .data=jsi_TypeChkStrs),
    JSI_OPT_END(Jsi_Interp)
};

/* Object for each interp created. */
typedef struct InterpObj {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_Interp *subinterp;
    Jsi_Interp *parent;
    Jsi_Hash *aliases;
    //char *interpname;
    char *mode;
    Jsi_Obj *fobj;
    int objId;
    int deleting;
} InterpObj;

/* Global state of interps. */

typedef struct {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    int refCount;
    const char *cmdName;
    Jsi_Value *args;
    Jsi_Value *func;
    Jsi_Value *cmdVal;
    InterpObj *intobj;
    Jsi_Interp *interp;
} AliasCmd;


static void interpObjErase(InterpObj *fo);
static int interpObjFree(Jsi_Interp *interp, void *data);
static int interpObjIsTrue(void *data);
static int interpObjEqual(void *data1, void *data2);

static void ConvertReturn(Jsi_Interp *interp, Jsi_Interp *inInterp, Jsi_Value **ret)
{
    Jsi_DString dStr = {};

    switch ((*ret)->vt) {
        case JSI_VT_UNDEF:
        case JSI_VT_BOOL:
        case JSI_VT_NUMBER:
        case JSI_VT_NULL:
            break;
        default:
            Jsi_DSInit(&dStr);
            char *cp = (char*)Jsi_ValueGetDString(inInterp, *ret, &dStr, JSI_OUTPUT_JSON);
            Jsi_JSONParse(interp, cp, ret, 0);
            Jsi_DSFree(&dStr);
    }
}

/* Call a command with JSON args.  Returned value is converted to JSON. */
int Jsi_EvalCmdJSON(Jsi_Interp *interp, const char *cmd, const char *jsonArgs, Jsi_DString *dStr)
{
    if (Jsi_MutexLock(interp, interp->Mutex) != JSI_OK)
        return JSI_ERROR;
    Jsi_Value *nrPtr = Jsi_ValueNew1(interp);
    int rc = Jsi_CommandInvokeJSON(interp, cmd, jsonArgs, &nrPtr);
    Jsi_DSInit(dStr);
    Jsi_ValueGetDString(interp, nrPtr, dStr, JSI_OUTPUT_JSON);
    Jsi_DecrRefCount(interp, nrPtr);
    Jsi_MutexUnlock(interp, interp->Mutex);
    return rc;
}

/* Call a function with JSON args.  Return a primative. */
int Jsi_FunctionInvokeJSON(Jsi_Interp *interp, Jsi_Value *func, const char *json, Jsi_Value **ret)
{
    int rc;
    Jsi_Value *aPtr = Jsi_ValueNew1(interp);
    rc = Jsi_JSONParse(interp, json, &aPtr, 0);
    if (rc == JSI_OK) {
        rc = Jsi_FunctionInvoke(interp, func, aPtr, ret, NULL);
    }
    Jsi_DecrRefCount(interp, aPtr);
    return rc;
}
/* Lookup cmd from cmdstr and invoke with JSON args. */
/*
 *   Jsi_CommandInvokeJSON(interp, "info.cmds", "[\"*\",true]", ret);
 */
int Jsi_CommandInvokeJSON(Jsi_Interp *interp, const char *cmdstr, const char *json, Jsi_Value **ret)
{
    Jsi_Value *func = Jsi_NameLookup(interp, cmdstr);
    if (func)
        return Jsi_FunctionInvokeJSON(interp, func, json, ret);
    Jsi_LogError("can not find cmd: %s", cmdstr);
    return JSI_ERROR;
}

static int NeedClean(Jsi_Interp *interp, Jsi_Value *arg)
{
    switch (arg->vt) {
        case JSI_VT_BOOL: return 0;
        case JSI_VT_NULL: return 0;
        case JSI_VT_NUMBER: return 0;
        case JSI_VT_STRING: return (interp->privKeys || arg->f.bits.isstrkey);
        case JSI_VT_UNDEF: return 0;
        //case JSI_VT_VARIABLE: return 1;
        case JSI_VT_OBJECT: {
            Jsi_Obj *o = arg->d.obj;
            switch (o->ot) {
                case JSI_OT_NUMBER: return 0;
                case JSI_OT_BOOL: return 0;
                case JSI_OT_STRING: return (interp->privKeys || arg->d.obj->isstrkey);
                case JSI_OT_FUNCTION: return 1;
                case JSI_OT_REGEXP: return 1;
                case JSI_OT_USEROBJ: return 1;
                case JSI_OT_ITER: return 1;     
                case JSI_OT_OBJECT:
                case JSI_OT_ARRAY:
                    if (o->isarrlist && o->arr)
                    {
                        int i;
                        for (i = 0; i < o->arrCnt; ++i) {
                            if (o->arr[i] && NeedClean(interp, o->arr[i]))
                                return 1;
                        }
                    } else if (o->tree) {
                        int trc = 0;
                        Jsi_TreeEntry *tPtr;
                        Jsi_TreeSearch search;
                        for (tPtr = Jsi_TreeEntryFirst(o->tree, &search, 0);
                            tPtr; tPtr = Jsi_TreeEntryNext(&search)) {
                            Jsi_Value *v = (Jsi_Value *)Jsi_TreeValueGet(tPtr);
                            if (v && (trc = NeedClean(interp, v)))
                                break;
                        }
                        Jsi_TreeSearchDone(&search);
                        return trc;
                    } else {
                        return 1;
                    }
                    return 0;
                default:
                    return 1;
            }
        }
    }
    return 1;
}

static  int Jsi_CleanValue(Jsi_Interp *interp, Jsi_Interp *tointerp, Jsi_Value *args, Jsi_Value **ret)
{
    if (tointerp->threadId == interp->threadId && !NeedClean(interp, args)) {
        Jsi_ValueCopy(tointerp, *ret, args);
        return JSI_OK;
    }
    /* Cleanse input args by convert to JSON and back. */
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    const char *cp = Jsi_ValueGetDString(interp, args, &dStr, JSI_OUTPUT_JSON);
    if (Jsi_JSONParse(tointerp, cp, ret, 0) != JSI_OK) {
        Jsi_DSFree(&dStr);
        Jsi_LogError("bad subinterp parse");
        return JSI_ERROR;
    }
    Jsi_DSFree(&dStr);
    return JSI_OK;
}

static int AliasInvoke(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Interp *pinterp = interp->parent;
    if (!pinterp)
        return JSI_ERROR;
    AliasCmd *ac = (AliasCmd *)funcPtr->cmdSpec->udata3;
    Jsi_Value *nargs = NULL;
    int argc = Jsi_ValueGetLength(interp, args);
    if (!ac) {
        Jsi_LogBug("BAD ALIAS INVOKE OF DELETED");
        return JSI_ERROR;
    }
    SIGASSERT(ac,ALIASCMD);
    Jsi_Value *nrPtr = Jsi_ValueNew1(interp);
    if (argc == 0 && ac->args)
        nargs = ac->args;
    else if (argc) {
        if (Jsi_CleanValue(interp, pinterp, args, &nrPtr) != JSI_OK)
            return JSI_ERROR;
        if (ac->args) {
            nargs = Jsi_ValueArrayConcat(pinterp, ac->args, nrPtr);
        } else {
            nargs = nrPtr;
        }
    }
    if (interp->doUnlock) Jsi_MutexUnlock(interp, interp->Mutex);
    if (Jsi_MutexLock(interp, pinterp->Mutex) != JSI_OK) {
        if (interp->doUnlock) Jsi_MutexLock(interp, interp->Mutex);
        return JSI_ERROR;
    }
    ac->refCount++;
    if (nargs && nargs != nrPtr)
        Jsi_IncrRefCount(interp, nargs);
    int oref = nargs->d.obj->refcnt;
    int rc = Jsi_FunctionInvoke(pinterp, ac->func, nargs, ret, NULL);
    if (oref != nargs->d.obj->refcnt && strcmp(ac->func->d.obj->d.fobj->func->name,"include") == 0) {
        // TODO: a hack to reduce a mem-leak, probably due to use of "arguments[n]" in JS.
        //fprintf(stderr, "REF CHG: %d\n", oref - nargs->d.obj->refcnt);
        Jsi_ObjDecrRefCount(interp, nargs->d.obj); 
    }
    ac->refCount--;
    Jsi_MutexUnlock(interp, pinterp->Mutex);
    if (interp->doUnlock && Jsi_MutexLock(interp, interp->Mutex) != JSI_OK) {
        return JSI_ERROR;
    }
    Jsi_DecrRefCount(interp, nrPtr);
    if (nargs && nargs != nrPtr)
        Jsi_DecrRefCount(interp, nargs);
    ConvertReturn(pinterp, interp, ret);
    return rc;
}


static int AliasFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *data) {
    /* TODO: deal with other copies of func may be floating around (refCount). */
    AliasCmd *ac = (AliasCmd *)data;
    SIGASSERT(ac,ALIASCMD);
    if (ac->func)
        Jsi_DecrRefCount(ac->interp, ac->func);
    if (ac->args)
        Jsi_DecrRefCount(ac->interp, ac->args);
    if (!ac->cmdVal)
        return JSI_OK;
    Jsi_Func *fobj = ac->cmdVal->d.obj->d.fobj->func;
    fobj->cmdSpec->udata3 = NULL;
    fobj->cmdSpec->proc = NULL;
    if (ac->intobj->subinterp) {
        Jsi_CommandDelete(ac->intobj->subinterp, ac->cmdName);
        if (strchr(ac->cmdName, '.'))
            Jsi_LogBug("alias free with X.Y dot name leaks memory: %s", ac->cmdName);
    } else
        Jsi_DecrRefCount(ac->interp, ac->cmdVal);
    MEMCLEAR(ac);
    Jsi_Free(ac);
    return JSI_OK;
}

static int InterpAliasCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!udf) {
        Jsi_LogError("Apply Interp.eval in a non-interp object");
        return JSI_ERROR;
    }
    if (!udf->aliases) {
        Jsi_LogError("Sub-interp gone");
        return JSI_ERROR;
    }
    int argc = Jsi_ValueGetLength(interp, args);
    if (argc == 0) {
        return Jsi_HashKeysDump(interp, udf->aliases, ret, 0);
    }
    Jsi_HashEntry *hPtr;
    char *key = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    if (!key) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }
    AliasCmd* ac;
    if (argc == 1) {
        hPtr = Jsi_HashEntryFind(udf->aliases, (void*)key);
        if (!hPtr)
            return JSI_OK;
        ac = (AliasCmd*)Jsi_HashValueGet(hPtr);
        SIGASSERT(ac,ALIASCMD);
        Jsi_ValueDup2(interp, ret, ac->func);
        return JSI_OK;
    }
    Jsi_Value *afunc = Jsi_ValueArrayIndex(interp, args, 1);
    if (argc == 2) {
        hPtr = Jsi_HashEntryFind(udf->aliases, (void*)key);
        if (!hPtr)
            return JSI_OK;
        ac = (AliasCmd*)Jsi_HashValueGet(hPtr);
        if (!Jsi_ValueIsFunction(interp, afunc)) {
            Jsi_LogError("arg 2: expected function");
            return JSI_ERROR;
        }
        Jsi_ValueDup2(interp, ret, ac->args);
        return JSI_OK;
    }
    if (argc == 3) {
        int isNew;
        Jsi_Value *aargs = Jsi_ValueArrayIndex(interp, args, 2);
        if (Jsi_ValueIsNull(interp, afunc) && Jsi_ValueIsNull(interp, aargs)) {
            hPtr = Jsi_HashEntryFind(udf->aliases, (void*)key);
            if (hPtr == NULL)
                return JSI_OK;
            AliasCmd *ac = (AliasCmd*)Jsi_HashValueGet(hPtr);
            if (0 && ac->cmdVal)
                Jsi_DecrRefCount(interp, ac->cmdVal);
            AliasFree(interp, NULL, ac);
            Jsi_HashEntryDelete(hPtr);
            return JSI_OK;
        }
        hPtr = Jsi_HashEntryNew(udf->aliases, (void*)key, &isNew);
        if (!hPtr) {
            Jsi_LogError("create failed: %s", key);
            return JSI_ERROR;
        }
        if (!Jsi_ValueIsFunction(interp, afunc)) {
            Jsi_LogError("arg 2: expected function");
            return JSI_ERROR;
        }
        if (Jsi_ValueIsNull(interp, aargs) == 0 && Jsi_ValueIsArray(interp, aargs) == 0) {
            Jsi_LogError("arg 3: expected array or null");
            return JSI_ERROR;
        }
        AliasCmd *ac;
        if (!isNew) {
            AliasFree(interp, NULL, Jsi_HashValueGet(hPtr));
        }
        ac = (AliasCmd*)Jsi_Calloc(1, sizeof(AliasCmd));
        SIGINIT(ac, ALIASCMD);
        ac->cmdName = (const char*)Jsi_HashKeyGet(hPtr);
        ac->func = afunc;
        Jsi_IncrRefCount(interp, afunc);
        if (!Jsi_ValueIsNull(interp, aargs)) {
            ac->args = aargs;
            Jsi_IncrRefCount(interp, aargs);
        }
        ac->intobj = udf;
        ac->interp = interp;
        Jsi_HashValueSet(hPtr, ac);
        Jsi_Value *cmd = Jsi_CommandCreate(udf->subinterp, key, AliasInvoke, NULL);
        if (!cmd) {
            Jsi_LogError("command create failure");
            return JSI_ERROR;
        }
        ac->cmdVal = cmd;
        Jsi_Func *fobj = cmd->d.obj->d.fobj->func;
        fobj->cmdSpec->udata3 = ac;
    }
    return JSI_OK;
}

static int freeCodeTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    jsi_Pstate *ps = (jsi_Pstate *)ptr;
    if (!ps) return JSI_OK;
    ps->hPtr = NULL;
    jsi_PstateFree(ps);
    return JSI_OK;
}

static int freeOnDeleteTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    Jsi_DeleteProc *proc = (Jsi_DeleteProc *)ptr;
    proc(interp, NULL);
    return JSI_OK;
}

static int freeAssocTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    jsi_DelAssocData(interp, ptr);
    return JSI_OK;
}

static int freeEventTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Event *event = (Jsi_Event *)ptr;
    SIGASSERT(event,EVENT);
    if (!ptr) return JSI_OK;
    event->hPtr = NULL;
    Jsi_EventFree(interp, event);
    return JSI_OK;
}
static int jsiFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Free(ptr);
    return JSI_OK;
}

static int regExpFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_RegExpFree((Jsi_Regex*)ptr);
    return JSI_OK;
}

static int freeCmdSpecTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    jsi_CmdSpecDelete(interp, ptr);
    return JSI_OK;
}

static int freeGenObjTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Obj *obj = (Jsi_Obj *)ptr;
    SIGASSERT(obj,OBJ);
    if (!obj) return JSI_OK;
    Jsi_ObjDecrRefCount(interp, obj);
    return JSI_OK;
}


static int freeFuncsTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Func *func = (Jsi_Func *)ptr;
    if (!func) return JSI_OK;
    SIGASSERT(func,FUNC);
    func->hPtr = NULL;
    jsi_FuncFree(interp, func);
    return JSI_OK;
}

static int freeFuncObjTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Obj *v = (Jsi_Obj *)ptr;
    if (!v) return JSI_OK;
    SIGASSERT(v,OBJ);
    if (v->d.fobj) {
        if (v->d.fobj->scope) {
            jsi_ScopeChain *scope = v->d.fobj->scope;
            v->d.fobj->scope = NULL;
            jsi_ScopeChainFree(interp, scope);
        }
    }
    Jsi_ObjDecrRefCount(interp, v);
    return JSI_OK;
}

static int freeBindObjTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Obj *v = (Jsi_Obj *)ptr;
    if (!v) return JSI_OK;
    SIGASSERT(v,OBJ);
    if (v->d.fobj && v->d.fobj->scope) {
        v->d.fobj->scope = NULL;
    }
    Jsi_ObjDecrRefCount(interp, v);
    return JSI_OK;
}

/* TODO: incr ref before add then just decr till done. */
static int freeValueTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Value *val = (Jsi_Value *)ptr;
    SIGASSERT(val,VALUE);
    if (!val) return JSI_OK;
    //printf("GEN: %p\n", val);
   /* if (val->refCnt>1)
        Jsi_DecrRefCount(interp, val);*/
    Jsi_DecrRefCount(interp, val);
    return JSI_OK;
}

static int freeUserdataTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (ptr) 
        jsi_UserObjDelete(interp, ptr);
    return JSI_OK;
}

void Jsi_ShiftArgs(Jsi_Interp *interp) {
    Jsi_Value *v = interp->args; //Jsi_NameLookup(interp, "console.args");
    if (v==NULL || v->vt != JSI_VT_OBJECT || v->d.obj->arr == NULL || v->d.obj->arrCnt <= 0)
        return;
    Jsi_Obj *obj = v->d.obj;
    int n = v->d.obj->arrCnt;
    n--;
    v = obj->arr[0];
    if (n>0)
        memmove(obj->arr, obj->arr+1, n*sizeof(Jsi_Value*));
    obj->arr[n] = NULL;
    Jsi_ObjSetLength(interp, obj, n);    
}

char *jsi_execName = NULL;
static Jsi_Value *jsi_execValue = NULL;

Jsi_Value *Jsi_Executable(Jsi_Interp *interp)
{
    return jsi_execValue;
}

static int KeyLocker(Jsi_Hash* tbl, int lock)
{
    if (!lock)
        Jsi_MutexUnlock(jsiMainInterp, jsiMainInterp->Mutex);
    else
        return Jsi_MutexLock(jsiMainInterp, jsiMainInterp->Mutex);
    return JSI_OK;
}

Jsi_Interp* Jsi_InterpNew(Jsi_Interp *parent, int argc, char **argv, Jsi_Value *opts)
{
    Jsi_Interp* interp = (Jsi_Interp *)Jsi_Calloc(1,sizeof(*interp));
    char buf[BUFSIZ];
    if (jsiMainInterp == NULL && parent == NULL)
        jsiMainInterp = interp;
    interp->parent = parent;
    interp->mainInterp = jsiMainInterp;
    if (parent) {
        interp->dbPtr = parent->dbPtr;
    } else {
        interp->dbPtr = &interp->dbStatic;
    }
#ifdef JSI_MEM_DEBUG
    if (!interp->dbPtr->valueDebugTbl) {
        interp->dbPtr->valueDebugTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, NULL);
        interp->dbPtr->objDebugTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, NULL);
    }
#endif    
    if (!parent)
        interp->maxInterpDepth = JSI_MAX_SUBINTERP_DEPTH;
    else {
        if (parent->noSubInterps) {
            Jsi_Free(interp);
            interp = parent;
            Jsi_LogError("subinterps disallowed");
            return NULL;
        }
        interp->maxInterpDepth = parent->maxInterpDepth;
        interp->interpDepth = parent->interpDepth+1;
        if (interp->interpDepth > interp->maxInterpDepth) {
            Jsi_Free(interp);
            interp = parent;
            Jsi_LogError("exceeded max subinterp depth");
            return NULL;
        }
    }
#ifdef JSI_USE_COMPAT
    interp->compat = JSI_USE_COMPAT;
#endif
    const char *ocp = NULL;
    const char *ocp2;
    if (ocp && ((ocp2=strstr(ocp,"memDebug:"))))
        interp->memDebug=strtol(ocp+sizeof("memDebug:"), NULL, 0);
    if (ocp && ((ocp2=strstr(ocp,"compat:"))))
        interp->compat=(ocp[sizeof("compat:")]=='t');
    interp->maxDepth = JSI_MAX_EVAL_DEPTH;
    interp->maxIncDepth = JSI_MAX_INCLUDE_DEPTH;
    SIGINIT(interp,INTERP);
    interp->NullValue = Jsi_ValueNewNull(interp);
    Jsi_IncrRefCount(interp, interp->NullValue);
    interp->curDir = Jsi_Strdup(getcwd(buf, sizeof(buf)));   
    interp->onDeleteTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeOnDeleteTbl);
    interp->assocTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, freeAssocTbl);
    interp->cmdSpecTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, freeCmdSpecTbl);
    interp->eventTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeEventTbl);
    interp->genDataTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, jsiFree);
    interp->fileTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, jsiFree);
    interp->funcObjTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeFuncObjTbl);
    interp->funcsTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeFuncsTbl);
    interp->bindTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeBindObjTbl);
    interp->protoTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL/*freeValueTbl*/);
    interp->regexpTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, regExpFree);
    interp->preserveTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, jsiFree);
    interp->loadTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, jsi_FreeOneLoadHandle);
    interp->optionDataHash = Jsi_HashNew(interp, JSI_KEYS_STRING, jsiFree);
    interp->lockTimeout = -1;
#ifdef JSI_LOCK_TIMEOUT
    interp->lockTimeout JSI_LOCK_TIMEOUT;
#endif
#ifndef JSI_DO_UNLOCK
#define JSI_DO_UNLOCK 1
#endif
    interp->doUnlock = JSI_DO_UNLOCK;

    if (interp == jsiMainInterp || interp->threadId != jsiMainInterp->threadId) {
        interp->strKeyTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
        interp->privKeys = 1;
    }
    if (!interp->strKeyTbl)
        interp->strKeyTbl = jsiMainInterp->strKeyTbl;
    if (opts && opts->vt != JSI_VT_NULL && Jsi_OptionsProcess(interp, InterpOptions, opts, interp, 0) < 0) {
        Jsi_InterpDelete(interp);
        return NULL;
    }
    if (interp == jsiMainInterp) {
        interp->subthread = 0;
    } else {
        if (interp->privKeys && interp->strKeyTbl == jsiMainInterp->strKeyTbl) {
            //Jsi_HashDelete(interp->strKeyTbl);
            Jsi_OptionsFree(interp, InterpOptions, interp, 0); /* Reparse options to populate new key table. */
            interp->strKeyTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
            if (opts->vt != JSI_VT_NULL) Jsi_OptionsProcess(interp, InterpOptions, opts, interp, 0);
        } else if (interp->privKeys == 0 && interp->strKeyTbl != jsiMainInterp->strKeyTbl) {
            Jsi_OptionsFree(interp, InterpOptions, interp, 0); /* Reparse options to populate new key table. */
            Jsi_HashDelete(interp->strKeyTbl);
            interp->strKeyTbl = jsiMainInterp->strKeyTbl;
            if (opts->vt != JSI_VT_NULL) Jsi_OptionsProcess(interp, InterpOptions, opts, interp, 0);
        }
        if (interp->subthread)
            jsiMainInterp->threadCnt++;
        if (interp->subthread && interp->strKeyTbl == jsiMainInterp->strKeyTbl)
            jsiMainInterp->threadShrCnt++;
        if (jsiMainInterp->threadShrCnt)
            jsiMainInterp->strKeyTbl->lockProc = KeyLocker;
    }
    if (parent && parent->isSafe)
        interp->isSafe = 1;
    Jsi_DString oStr = {};
#ifdef JSI_INTERP_OPTS  /* eg. "nonStrict: true, maxOpCnt:1000000" */
    if (ocp && *ocp)
        Jsi_DSAppend(&oStr, "{", JSI_INTERP_OPTS, ", ", ocp+1, NULL);
    else
        Jsi_DSAppend(&oStr, "{", JSI_INTERP_OPTS, "}", NULL);
#else
    Jsi_DSAppend(&oStr, ocp, NULL);
#endif
    ocp = Jsi_DSValue(&oStr);
    if (interp == jsiMainInterp  && *ocp) {
        Jsi_Value *popts = Jsi_ValueNew1(interp);
        if (Jsi_JSONParse(interp, ocp, &popts, 0) != JSI_OK ||
            Jsi_OptionsProcess(interp, InterpOptions, popts, interp, JSI_OPTS_IS_UPDATE) < 0) {
            Jsi_InterpDelete(interp);
            Jsi_DSFree(&oStr);
            return NULL;
        }
        Jsi_DecrRefCount(interp, popts);
    }
    Jsi_DSFree(&oStr);
#ifndef JSI_MEM_DEBUG
    static int warnNoDebug = 0;
    if (interp->memDebug && warnNoDebug == 0) {
        Jsi_LogWarn("ignoring memDebug as jsi was compiled without memory debugging");
        warnNoDebug = 1;
    }
#endif
    interp->threadId = Jsi_CurrentThread();
    if (interp == jsiMainInterp)
        interp->lexkeyTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    else
        interp->lexkeyTbl = jsiMainInterp->lexkeyTbl;
    interp->thisTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeValueTbl);
    interp->userdataTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, freeUserdataTbl);
    interp->varTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    interp->codeTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, freeCodeTbl);
    interp->genValueTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD,freeValueTbl);
    interp->genObjTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeGenObjTbl);
#ifdef JSI_MEM_DEBUG
    interp->codesTbl = (interp == jsiMainInterp ? Jsi_HashNew(interp, JSI_KEYS_ONEWORD, NULL) : jsiMainInterp->codesTbl);
#endif
    interp->maxArrayList = MAX_ARRAY_LIST;
    if (!jsi_is_init) {
        jsi_is_init = 1;
        jsi_ValueInit(interp);
        interpsTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, 0);
    }
    

    /* current scope, also global */
    interp->csc = Jsi_ValueNew1(interp);
    Jsi_ValueMakeObject(interp, &interp->csc, Jsi_ObjNew(interp));
    interp->incsc = interp->csc;
    
    /* initial scope chain, nothing */
    interp->ingsc = interp->gsc = jsi_ScopeChainNew(interp, 0);
    
    interp->ps = jsi_PstateNew(interp); /* Default parser. */

    if (interp->args && argc) {
        Jsi_LogFatal("args may not be specified both as options and parameter");
        Jsi_InterpDelete(interp);
        return NULL;
    }
    if (interp->maxDepth>JSI_MAX_EVAL_DEPTH)
        interp->maxDepth = JSI_MAX_EVAL_DEPTH;

#define JSIDOINIT(nam) if (jsi_##nam##Init(interp) != JSI_OK) { Jsi_LogFatal("Init failure in %s", #nam); }
#define JSIDOINIT2(nam) if (Jsi_##Init##nam(interp) != JSI_OK) { Jsi_LogFatal("Init failure in %s", #nam); }

    JSIDOINIT(Proto);

    if (argc >= 0) {
        Jsi_Value *iargs = Jsi_ValueNew1(interp);
        iargs->f.bits.dontdel = 1;
        iargs->f.bits.readonly = 1;
        Jsi_Obj *iobj = Jsi_ObjNew(interp);
        Jsi_ValueMakeArrayObject(interp, &iargs, iobj);
        int msiz = (argc?argc-1:0);
        Jsi_ObjArraySizer(interp, iobj, msiz);
        iobj->arrMaxSize = msiz;
        iobj->arrCnt = argc;
        int i;
        for (i = 1; i < argc; ++i) {
            iobj->arr[i-1] = Jsi_ValueNewStringDup(interp, argv[i]);
            Jsi_IncrRefCount(interp, iobj->arr[i-1]);
            if (i==1)
                Jsi_HashSet(interp->genValueTbl, iobj->arr[i-1], iobj->arr[i-1]);
            jsi_ValueDebugLabel(iobj->arr[i-1], "InterpCreate", "args");
        }
        Jsi_ObjSetLength(interp, iobj, msiz);
        interp->args = iargs;
    }

    JSIDOINIT(Cmds);
    JSIDOINIT(Interp);
    JSIDOINIT(JSON);

    interp->retPtr = Jsi_ValueNew1(interp);
    interp->Mutex = Jsi_MutexNew(interp, -1, JSI_MUTEX_RECURSIVE);
    if (1 || interp->subthread) {
        interp->QMutex = Jsi_MutexNew(interp, -1, JSI_MUTEX_RECURSIVE);
        Jsi_DSInit(&interp->interpEvalQ);
        Jsi_DSInit(&interp->interpMsgQ);
    }
    JSIDOINIT(Lexer);
    if (interp != jsiMainInterp && !parent)
        Jsi_HashSet(interpsTbl, interp, NULL);
        
    if (!interp->isSafe) {
        JSIDOINIT(Load);
#ifndef JSI_OMIT_SIGNAL
        JSIDOINIT(Signal);
#endif
    }
    if (interp->isSafe == 0 || interp->safeWriteDirs!=NULL || interp->safeReadDirs!=NULL) {
#ifndef JSI_OMIT_FILESYS
        JSIDOINIT(FileCmds);
        JSIDOINIT(Filesys);
#endif
#ifndef JSI_OMIT_SOCKET
    JSIDOINIT2(Socket);
#endif
#ifdef HAVE_SQLITE
        JSIDOINIT2(Sqlite);
#endif
#ifdef HAVE_MYSQL
        JSIDOINIT2(MySql);
#endif
    }
#ifdef HAVE_WEBSOCKET
    JSIDOINIT2(WebSocket);
#endif

    if (argc > 0) {
        char *ss = argv[0];
        char epath[PATH_MAX] = "";
#ifdef __WIN32
  
        if (GetModuleFileName(NULL, epath, sizeof(epath))>0)
            ss = epath;
#else
#ifndef PROC_SELF_DIR
#define PROC_SELF_DIR "/proc/self/exe"
#endif
        if (ss && *ss != '/' && readlink(PROC_SELF_DIR, epath, sizeof(epath))) {
            ss = epath;
        }
#endif
        Jsi_Value *src = Jsi_ValueNewStringDup(interp, ss);
        Jsi_IncrRefCount(interp, src);
        jsi_execName = Jsi_Realpath(interp, src, NULL);
        Jsi_DecrRefCount(interp, src);
        jsi_execValue = Jsi_ValueNewString(interp, jsi_execName, -1);
        Jsi_IncrRefCount(interp, jsi_execValue);
        Jsi_HashSet(interp->genValueTbl, jsi_execValue, jsi_execValue);
    }
    
    //interp->nocacheOpCodes = 1;
    return interp;
}

int Jsi_InterpGone( Jsi_Interp* interp)
{
    return (interp == NULL || interp->deleting || interp->destroying || interp->exited);
}

static void DeleteAllInterps() { /* Delete toplevel interps. */
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch search;
    if (!interpsTbl)
        return;
    for (hPtr = Jsi_HashEntryFirst(interpsTbl, &search); hPtr; hPtr = Jsi_HashEntryNext(&search)) {
        Jsi_Interp *interp = (Jsi_Interp *)Jsi_HashKeyGet(hPtr);
        Jsi_HashEntryDelete(hPtr);
        interp->destroying = 1;
        Jsi_InterpDelete(interp);
    }
    Jsi_HashDelete(interpsTbl);
    interpsTbl = NULL;
    jsi_is_init = 0;
}

#ifdef JSI_MEM_DEBUG

typedef enum { MDB_INOBJ=1, MDB_VISITED=2 } jsi_MDB;

void jsiFlagDebugValues(Jsi_Interp *interp, Jsi_Obj *obj)
{
    Jsi_Value *v;
    int oflags;
    if (obj->ot != JSI_OT_OBJECT && obj->ot != JSI_OT_ARRAY)
        return;
    if (obj->tree) {
        Jsi_TreeEntry *hPtr;
        Jsi_TreeSearch srch;
        for (hPtr=Jsi_TreeEntryFirst(obj->tree, &srch,  JSI_TREE_INORDER); hPtr;
            hPtr=Jsi_TreeEntryNext(&srch)) {
            v = (Jsi_Value*)Jsi_TreeValueGet(hPtr);
            if (v == NULL || v->sig != JSI_SIG_VALUE) continue;
            oflags = v->VD.flags;
            v->VD.flags |= (MDB_VISITED|MDB_INOBJ);
            if (oflags&MDB_VISITED || v->vt != JSI_VT_OBJECT)
                continue;
            jsiFlagDebugValues(interp, v->d.obj);
        }
    }
    if (obj->arr) {
        int i;
        for (i=0; i<obj->arrCnt; i++) {
            v = obj->arr[i];
            if (v == NULL || v->sig != JSI_SIG_VALUE) continue;
            oflags = v->VD.flags;
            v->VD.flags |= (MDB_VISITED|MDB_INOBJ);
            if (oflags&MDB_VISITED || v->vt != JSI_VT_OBJECT)
                continue;
            jsiFlagDebugValues(interp, v->d.obj);
        }
    }
}

void jsi_DebugDumpValues(Jsi_Interp *interp)
{
    if (jsiMainInterp != interp) return;
    int vdLev = interp->memDebug;
    int have = (interp->dbPtr->valueDebugTbl->numEntries || interp->dbPtr->objDebugTbl->numEntries);
    if ((have && vdLev>0) || vdLev>=3) {
        // First traverse all Object trees/arrays and mark all values contained therein.
        Jsi_HashSearch search;
        Jsi_HashEntry *hPtr;
        for (hPtr = Jsi_HashEntryFirst(interp->dbPtr->objDebugTbl, &search);
            hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
            Jsi_Obj *vp = (Jsi_Obj *)Jsi_HashKeyGet(hPtr);
            if (vp!=NULL && vp->sig == JSI_SIG_OBJ) {
                jsiFlagDebugValues(interp, vp);
            }
        }
        if (interp->dbPtr->valueDebugTbl->numEntries != interp->dbPtr->valueCnt)
            fprintf(stderr, "\n\nValues table/alloc mismatch: table=%d, alloc=%d\n",
                interp->dbPtr->valueDebugTbl->numEntries, interp->dbPtr->valueCnt);
        // Dump unfreed values and objs.
        int refSum=0, refsum=0;
        int bcnt[4] = {};
        if (vdLev>1 && interp->dbPtr->valueDebugTbl->numEntries)
            fprintf(stderr, "\n\nUNFREED VALUES \"[*ptr,#refCnt,type,idx:label,label2]: @file:line in func() ...\"\n");
        for (hPtr = Jsi_HashEntryFirst(interp->dbPtr->valueDebugTbl, &search);
            hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
            Jsi_Value *vp = (Jsi_Value *)Jsi_HashKeyGet(hPtr);
            if (vp==NULL || vp->sig != JSI_SIG_VALUE) {
                bcnt[0]++;
                if (vdLev>1)
                    fprintf(stderr, "BAD VALUE: %p\n", vp);
            } else {
                bcnt[1]++;
                refSum += vp->refCnt;
                if (vdLev>1) {
                    char ebuf[BUFSIZ], ebuf2[JSI_MAX_NUMBER_STRING];
                    ebuf[0] = 0;
                    if (vp->vt==JSI_VT_OBJECT)
                        sprintf(ebuf, " {obj=%p, otype=%s}", vp->d.obj, Jsi_ObjTypeStr(interp, vp->d.obj));
                    else if (vp->vt==JSI_VT_NUMBER)
                        sprintf(ebuf, " {num=%s}", Jsi_NumberToString(vp->d.num, ebuf2));
                    else if (vp->vt==JSI_VT_BOOL)
                        sprintf(ebuf, " {bool=%s}", vp->d.val?"true":"false");
                    else if (vp->vt==JSI_VT_STRING) {
                        const char *sbuf = ((vp->d.s.str && strlen(vp->d.s.str)>40)?"...":"");
                        sprintf(ebuf, " {string=\"%.40s%s\"}", (vp->d.s.str?vp->d.s.str:""), sbuf);
                    }
                    const char *pfx = "";
                    if (!(vp->VD.flags&MDB_INOBJ))
                        pfx = "!"; // Value is not contained in an object.
                    fprintf(stderr, "[%s*%p,#%d,%s,%d:%s%s%s]:%s @%s:%d in %s()%s\n", pfx,
                        vp, vp->refCnt, Jsi_ValueTypeStr(interp, vp), vp->VD.Idx,
                        (vp->VD.label?vp->VD.label:""), (vp->VD.label2?":":""),
                        (vp->VD.label2?vp->VD.label2:""), vp->VD.interp==jsiMainInterp?"":"!",
                        vp->VD.fname, vp->VD.line, vp->VD.func, ebuf);
                }
            }
        }
        if (interp->dbPtr->objDebugTbl->numEntries != interp->dbPtr->objCnt)
            fprintf(stderr, "\n\nObject table/alloc mismatch: table=%d, alloc=%d\n",
                interp->dbPtr->objDebugTbl->numEntries, interp->dbPtr->objCnt);
        if (vdLev>1 && interp->dbPtr->objDebugTbl->numEntries)
            fprintf(stderr, "\n\nUNFREED OBJECTS \"[*ptr,#refCnt,type,idx:label,label2]: @file:line in func() ...\"\n");
        for (hPtr = Jsi_HashEntryFirst(interp->dbPtr->objDebugTbl, &search);
            hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
            Jsi_Obj *vp = (Jsi_Obj *)Jsi_HashKeyGet(hPtr);
            if (vp==NULL || vp->sig != JSI_SIG_OBJ) {
                bcnt[2]++;
                fprintf(stderr, "BAD OBJ: %p\n", vp);
            } else {
                bcnt[3]++;
                refsum += vp->refcnt;
                if (vdLev>1) {
                    char ebuf[BUFSIZ], ebuf2[JSI_MAX_NUMBER_STRING];
                    ebuf[0] = 0;
                    if (vp->ot==JSI_OT_OBJECT) {
                        if (vp->isarrlist)
                            sprintf(ebuf, "tree#%d, array#%d", (vp->tree?vp->tree->numEntries:0), vp->arrCnt);
                        else
                            sprintf(ebuf, "tree#%d", (vp->tree?vp->tree->numEntries:0));
                    } else if (vp->ot==JSI_OT_NUMBER)
                        sprintf(ebuf, "num=%s", Jsi_NumberToString(vp->d.num, ebuf2));
                    else if (vp->ot==JSI_OT_BOOL)
                        sprintf(ebuf, "bool=%s", vp->d.val?"true":"false");
                    else if (vp->ot==JSI_OT_STRING) {
                        const char *sbuf = ((vp->d.s.str && strlen(vp->d.s.str)>40)?"...":"");
                        sprintf(ebuf, "string=\"%.40s%s\"", (vp->d.s.str?vp->d.s.str:""), sbuf);
                    }
                    fprintf(stderr, "[*%p,#%d,%s,%d:%s%s%s]:%s @%s:%d in %s() {%s}\n",
                        vp, vp->refcnt, Jsi_ObjTypeStr(interp, vp), vp->VD.Idx, vp->VD.label?vp->VD.label:"",
                        vp->VD.label2?":":"",vp->VD.label2?vp->VD.label2:"", vp->VD.interp==jsiMainInterp?"":"!",
                        vp->VD.fname, vp->VD.line,
                        vp->VD.func, ebuf);
                }
            }
        }
        fprintf(stderr, "\nVALUES: bad=%d,unfreed=%d,allocs=%d,refsum=%d  | OBJECTS: bad=%d,unfreed=%d,allocs=%d,refsum=%d\n",
            bcnt[0], bcnt[1], interp->dbPtr->valueAllocCnt, refSum, bcnt[2], bcnt[3], interp->dbPtr->objAllocCnt, refsum);

        if (interp->codesTbl)
            for (hPtr = Jsi_HashEntryFirst(interp->codesTbl, &search);
                hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
                OpCodes *vp = (OpCodes *)Jsi_HashKeyGet(hPtr);
                fprintf(stderr, "unfreed opcodes: %d\n", vp->id);
            }
    }
    Jsi_HashDelete(interp->dbPtr->valueDebugTbl);
    Jsi_HashDelete(interp->dbPtr->objDebugTbl);
    Jsi_HashDelete(interp->codesTbl);
}
#endif

static int jsiInterpDelete(Jsi_Interp* interp, void *unused)
{
    SIGASSERT(interp,INTERP);
    if (interp == jsiMainInterp)
        DeleteAllInterps();

    jsiDelInterp = interp;
    if (interp->gsc) jsi_ScopeChainFree(interp, interp->gsc);
    //if (interp->csc->d.obj->refcnt>1) /* TODO: This is a hack to release global. */
       // Jsi_ObjDecrRefCount(interp, interp->csc->d.obj);
    if (interp->csc) Jsi_DecrRefCount(interp, interp->csc);
    if (interp->ps) jsi_PstateFree(interp->ps);
    int i;
    for (i=0; i<interp->maxStack; i++) {
        if (interp->Stack[i]) Jsi_DecrRefCount(interp, interp->Stack[i]);
        if (interp->Obj_this[i]) Jsi_DecrRefCount(interp, interp->Obj_this[i]);
    }
    Jsi_Free(interp->Stack);
    Jsi_Free(interp->Obj_this);

    Jsi_HashDelete(interp->assocTbl);
    Jsi_HashDelete(interp->codeTbl);
    Jsi_HashDelete(interp->cmdSpecTbl);
    Jsi_HashDelete(interp->fileTbl);
    Jsi_HashDelete(interp->funcObjTbl);
    Jsi_HashDelete(interp->funcsTbl);
    if (interp == jsiMainInterp)
        Jsi_HashDelete(interp->lexkeyTbl);
    Jsi_HashDelete(interp->protoTbl);
    Jsi_HashDelete(interp->regexpTbl);
    if (interp->subthread)
        jsiMainInterp->threadCnt--;
    if (interp->subthread && interp->strKeyTbl == jsiMainInterp->strKeyTbl)
        jsiMainInterp->threadShrCnt--;
    if (!jsiMainInterp->threadShrCnt)
        jsiMainInterp->strKeyTbl->lockProc = NULL;
    //Jsi_ValueMakeUndef(interp, &interp->ret);
    Jsi_HashDelete(interp->thisTbl);
    Jsi_HashDelete(interp->userdataTbl);
    Jsi_HashDelete(interp->eventTbl);
    Jsi_HashDelete(interp->varTbl);
    Jsi_HashDelete(interp->genValueTbl);
    Jsi_HashDelete(interp->genObjTbl);
    Jsi_HashDelete(interp->genDataTbl);
    Jsi_HashDelete(interp->loadTbl);
    Jsi_HashDelete(interp->optionDataHash);
    if (interp->preserveTbl->numEntries!=0)
        Jsi_LogBug("Preserves unbalanced");
    Jsi_HashDelete(interp->preserveTbl);
    if (interp->argv0)
        Jsi_DecrRefCount(interp, interp->argv0);
    if (interp->console)
        Jsi_DecrRefCount(interp, interp->console);
    if (interp->lastSubscriptFail)
        Jsi_DecrRefCount(interp, interp->lastSubscriptFail);
    if (interp->curDir)
        Jsi_Free(interp->curDir);
    if (interp == jsiMainInterp) {
        jsi_FilesysDone(interp);
    }
    if (interp->Mutex)
        Jsi_MutexDelete(interp, interp->Mutex);
    if (interp->QMutex) {
        Jsi_MutexDelete(interp, interp->QMutex);
        Jsi_DSFree(&interp->interpEvalQ);
        Jsi_DSFree(&interp->interpMsgQ);
    }
    if (interp->nullFuncArg)
        Jsi_DecrRefCount(interp, interp->nullFuncArg);
    if (interp->NullValue)
        Jsi_DecrRefCount(interp, interp->NullValue);
    if (interp->Function_prototype_prototype) {
        if (interp->Function_prototype_prototype->refCnt>1)
            Jsi_DecrRefCount(interp, interp->Function_prototype_prototype);
        Jsi_DecrRefCount(interp, interp->Function_prototype_prototype);
    }
    if (interp->Object_prototype) {
        Jsi_DecrRefCount(interp, interp->Object_prototype);
    }
    if (interp->retPtr)
        Jsi_DecrRefCount(interp, interp->retPtr);
    if (interp->args) {
        //Jsi_DecrRefCount(interp, interp->args);
    }
    Jsi_OptionsFree(interp, InterpOptions, interp, 0);

    for (i=0; interp->cleanObjs[i]; i++) {
        //Jsi_Free(interp->cleanObjs[i]->tree);
        //interp->cleanObjs[i]->tree = NULL;
        interp->cleanObjs[i]->tree->freeProc = 0;
        Jsi_ObjFree(interp, interp->cleanObjs[i]);
    }
    Jsi_HashDelete(interp->bindTbl);
    for (i = 0; i <= interp->cur_scope; i++)
        jsi_ScopeStrsFree(interp, interp->scopes[i]);

#ifdef JSI_MEM_DEBUG
    jsi_DebugDumpValues(interp);
#endif
    if (interp == jsiMainInterp || interp->strKeyTbl != jsiMainInterp->strKeyTbl)
        Jsi_HashDelete(interp->strKeyTbl);
    if (interp == jsiMainInterp)
        jsiMainInterp = NULL;
    SIGASSERT(interp,INTERP);
    MEMCLEAR(interp);
    jsiDelInterp = NULL;
    Jsi_Free(interp);
    return JSI_OK;
}

void Jsi_InterpDelete(Jsi_Interp* interp)
{
    if (interp->deleting || interp->level > 0)
        return;
    Jsi_HashDelete(interp->onDeleteTbl);
    interp->deleting = 1;
    Jsi_EventuallyFree(interp, interp, jsiInterpDelete);
}

typedef struct {
    void *data;
    Jsi_Interp *interp;
    int refCnt;
    Jsi_DeleteProc* proc;
} PreserveData;

void Jsi_Preserve(Jsi_Interp* interp, void *data) {
    int isNew;
    PreserveData *ptr;
    Jsi_HashEntry *hPtr = Jsi_HashEntryNew(interp->preserveTbl, data, &isNew);
    assert(hPtr);
    if (!isNew) {
        ptr = (PreserveData*)Jsi_HashValueGet(hPtr);
        assert(interp == ptr->interp);
        ptr->refCnt++;
    } else {
        ptr = (PreserveData*)Jsi_Calloc(1,sizeof(*ptr));
        Jsi_HashValueSet(hPtr, ptr);
        ptr->interp = interp;
        ptr->data = data;
        ptr->refCnt = 1;
    }
}

void Jsi_Release(Jsi_Interp* interp, void *data) {
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->preserveTbl, data);
    if (!hPtr) return;
    PreserveData *ptr = (PreserveData*)Jsi_HashValueGet(hPtr);
    assert(ptr->interp == interp);
    if (--ptr->refCnt > 0) return;
    if (ptr->proc)
        (*ptr->proc)(interp, data);
    Jsi_Free(ptr);
    Jsi_HashEntryDelete(hPtr);
}

void Jsi_EventuallyFree(Jsi_Interp* interp, void *data, Jsi_DeleteProc* proc) {
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->preserveTbl, data);
    if (!hPtr) {
        (*proc)(interp, data);
        return;
    }
    PreserveData *ptr = (PreserveData*)Jsi_HashValueGet(hPtr);
    assert(ptr && ptr->interp == interp);
    Jsi_HashEntryDelete(hPtr);
}

void Jsi_InterpOnDelete(Jsi_Interp *interp, Jsi_DeleteProc *freeProc, void *ptr)
{
    Jsi_HashSet(interp->onDeleteTbl, ptr?ptr:(void*)freeProc, (void*)freeProc);
}

static void interpObjErase(InterpObj *fo)
{
    SIGASSERT(fo,INTERPOBJ);
    if (fo->subinterp) {
        Jsi_Interp *interp = fo->subinterp;        
        fo->subinterp = NULL;
        Jsi_HashDelete(fo->aliases);
        Jsi_InterpDelete(interp);
        /*fclose(fo->fp);
        Jsi_Free(fo->interpname);
        Jsi_Free(fo->mode);*/
    }
    fo->subinterp = NULL;
}

static int interpObjFree(Jsi_Interp *interp, void *data)
{
    InterpObj *fo = (InterpObj *)data;
    SIGASSERT(fo,INTERPOBJ);
    if (fo->deleting) return JSI_OK;
    fo->deleting = 1;
    interpObjErase(fo);
    Jsi_Free(fo);
    return JSI_OK;
}

static int interpObjIsTrue(void *data)
{
    InterpObj *fo = (InterpObj *)data;
    SIGASSERT(fo,INTERPOBJ);
    if (!fo->subinterp) return 0;
    else return 1;
}

static int interpObjEqual(void *data1, void *data2)
{
    return (data1 == data2);
}


#define FN_eval JSI_INFO("\
Unless an 'async' parameter of true is given, we wait until the sub-interp is idle, \
make the call, and return the result.  Otherwise the call is acyncronous (threaded only)")

/* TODO: support async func-callback.  Same for call/send. */
static int InterpEvalCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int async = 0, rc = JSI_OK, isthrd;
    Jsi_ValueMakeUndef(interp, ret);
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!udf) {
        Jsi_LogError("Apply Interp.eval in a non-interp object");
        return JSI_ERROR;
    }
    Jsi_Interp *sinterp = udf->subinterp;
    if (Jsi_InterpGone(interp)) {
        Jsi_LogError("Sub-interp gone");
        return JSI_ERROR;
    }
    isthrd = (interp->threadId != sinterp->threadId);
    Jsi_Value *nw = Jsi_ValueArrayIndex(interp, args, 1);
    if (nw && Jsi_GetBoolFromValue(interp, nw, &async))
        return JSI_ERROR;
    char *cp = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    if (cp==NULL || *cp == 0)
        return JSI_OK;
    if (async && isthrd) {
        /* Post to thread event in sub-interps queue. TODO: could just use event like below... */
        if (Jsi_MutexLock(interp, sinterp->QMutex) != JSI_OK)
            return JSI_ERROR;
        Jsi_DSAppend(&sinterp->interpEvalQ, Jsi_Strlen(Jsi_DSValue(&sinterp->interpEvalQ))?";":"", cp, NULL);
        Jsi_MutexUnlock(interp, sinterp->QMutex);
        return JSI_OK;
    }
    if (interp->doUnlock) Jsi_MutexUnlock(interp, interp->Mutex);
    if (!isthrd) {
        sinterp->level++;
        if (interp->tryDepth)
            sinterp->tryDepth++;
        rc = Jsi_EvalString(sinterp, cp, 0);
        if (interp->tryDepth) {
            sinterp->tryDepth--;
            if (rc != JSI_OK) {
                strcpy(interp->errMsgBuf, sinterp->errMsgBuf);
                interp->errLine = sinterp->errLine;
                interp->errFile = sinterp->errFile;
            }
        }
        sinterp->level--;
    } else {
        if (Jsi_MutexLock(interp, sinterp->QMutex) != JSI_OK)
            return JSI_ERROR;
        InterpStrEvent *se, *s = (InterpStrEvent *)Jsi_Calloc(1, sizeof(*s));
        SIGINIT(s,INTERPSTREVENT);
        s->isExec = 1;
        s->tryDepth = interp->tryDepth;
        Jsi_DSInit(&s->data);
        Jsi_DSAppend(&s->data, cp, NULL);
        Jsi_DSInit(&s->func);
        //s->mutex = Jsi_MutexNew(interp, -1, JSI_MUTEX_RECURSIVE);
        //Jsi_MutexLock(s->mutex);
        se = sinterp->interpStrEvents;
        if (!se)
            sinterp->interpStrEvents = s;
        else {
            while (se->next)
                se = se->next;
            se->next = s;
        }
    
        Jsi_MutexUnlock(interp, sinterp->QMutex);
        while (s->isExec)      /* Wait until done. TODO: timeout??? */
            Jsi_Sleep(interp, 1);
        rc = s->rc;
        if (rc != JSI_OK)
            Jsi_LogError("eval failed: %s", Jsi_DSValue(&s->data));
        Jsi_DSFree(&s->func);
        Jsi_DSFree(&s->data);
        Jsi_Free(s);
    }

    if (interp->doUnlock && Jsi_MutexLock(interp, interp->Mutex) != JSI_OK) {
        return JSI_ERROR;
    }

    if (Jsi_InterpGone(sinterp))
    {
        /* TODO: should exit() be able to delete??? */
        //Jsi_InterpDelete(sinterp);
        return JSI_OK;
    }
    if (rc != JSI_OK && !async)
        return rc;
    if (sinterp->retPtr->vt != JSI_VT_UNDEF) {
        Jsi_CleanValue(sinterp, interp, sinterp->retPtr, ret);
    }
    return JSI_OK;
}

static int InterpInfoCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    Jsi_Interp *subinterp = interp;
    if (udf) {
        if (!udf->subinterp) {
            Jsi_LogError("Sub-interp gone");
            return JSI_ERROR;
        }
        subinterp = udf->subinterp;
    }
    return jsi_InterpInfo(subinterp, args, _this, ret, funcPtr);
}

int jsi_InterpInfo(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Interp *sinterp = interp;
    Jsi_DString dStr = {}, cStr = {};
    char tbuf[1024];
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    tbuf[0] = 0;
    if (v) {
        InterpObj *udf = NULL;
        if (v && v->vt == JSI_VT_OBJECT && v->d.obj->ot == JSI_OT_USEROBJ)
            udf = (InterpObj *)v->d.obj->d.uobj->data;
        if (udf && udf->subinterp) {
            SIGASSERT(udf, INTERPOBJ);
            sinterp = udf->subinterp;
        } else {
            Jsi_LogError("unknown interp");
            return JSI_ERROR;
        }
    }
    if (sinterp->subthread)
        sprintf(tbuf, ", thread:{errorCnt:%u, evalCnt:%u, msgCnt:%u }",
            sinterp->threadErrCnt, sinterp->threadEvalCnt, sinterp->threadMsgCnt );
    Jsi_DSPrintf(&dStr, "{curLevel:%d, hasExited:%d, opCnt:%d, isSafe:%s, codeCacheHits: %d, "
        "funcCallCnt:%d, cmdCallCnt:%d, cwd:\"%s\", lockTimeout: %d, name, \"%s\", parent: %p %s%s};",
        sinterp->level, sinterp->exited, sinterp->opCnt, sinterp->isSafe?"true":"false", sinterp->codeCacheHit,
        sinterp->funcCallCnt, sinterp->cmdCallCnt,
        (sinterp->curDir?sinterp->curDir:Jsi_GetCwd(sinterp,&cStr)),
        sinterp->lockTimeout, sinterp->name?sinterp->name:"", sinterp->parent, tbuf[0]?",":"", tbuf);
    int rc = Jsi_JSONParse(interp, Jsi_DSValue(&dStr), ret, 0);
    Jsi_DSFree(&dStr);
    Jsi_DSFree(&cStr);
    return rc;
}

static int SubInterpEvalCallback(Jsi_Interp *interp, void* data)
{
    int rc = JSI_OK;
    Jsi_DString dStr = {};
    if (Jsi_MutexLock(interp, interp->QMutex) != JSI_OK)
        return JSI_ERROR;
    char *cp = Jsi_DSValue(&interp->interpEvalQ);
    if (*cp) {
        Jsi_DSAppend(&dStr, cp, NULL);
        Jsi_DSSetLength(&interp->interpEvalQ, 0);
        interp->threadEvalCnt++;
        Jsi_MutexUnlock(interp, interp->QMutex);
        if (Jsi_EvalString(interp, Jsi_DSValue(&dStr), 0) != JSI_OK)
            rc = JSI_ERROR;
        Jsi_DSSetLength(&dStr, 0);
        if (Jsi_MutexLock(interp, interp->QMutex) != JSI_OK)
            return JSI_ERROR;
    }
    cp = Jsi_DSValue(&interp->interpMsgQ);
    if (*cp) {
        //if (!interp->parent) printf("RECIEVING: %s\n", cp);
        Jsi_DSAppend(&dStr, cp, NULL);
        Jsi_DSSetLength(&interp->interpEvalQ, 0);
        interp->threadMsgCnt++;
        Jsi_MutexUnlock(interp, interp->QMutex);
        if (Jsi_CommandInvokeJSON(interp, interp->recvCmd, Jsi_DSValue(&dStr), NULL) != JSI_OK)
            rc = JSI_ERROR;
        Jsi_DSSetLength(&interp->interpMsgQ, 0);
    } else
        Jsi_MutexUnlock(interp, interp->QMutex);
    Jsi_DSFree(&dStr);
    if (Jsi_MutexLock(interp, interp->QMutex) != JSI_OK)
        return JSI_ERROR;
        
    /* Process subevents. */
    InterpStrEvent *oldse, *se = interp->interpStrEvents;
    Jsi_MutexUnlock(interp, interp->QMutex);
    while (se) {
        oldse = se;
        int isExec = se->isExec;
        if (isExec) {
            if (se->tryDepth)
                interp->tryDepth++;
            se->rc = Jsi_EvalString(interp, Jsi_DSValue(&se->data), 0);
            Jsi_DSSetLength(&se->data, 0);
            if (se->rc != JSI_OK && se->tryDepth) {
                Jsi_DSAppend(&se->data, interp->errMsgBuf, NULL);
                se->errLine = interp->errLine;
                se->errFile = interp->errFile;
            } else {
                Jsi_ValueGetDString(interp, interp->retPtr, &se->data, JSI_OUTPUT_JSON);
            }
            if (se->tryDepth)
                interp->tryDepth--;
                
        /* Otherwise, async calls. */
        } else if (se->objData) {
            if (JSI_OK != Jsi_CommandInvoke(interp, Jsi_DSValue(&se->func), se->objData, NULL))
                rc = JSI_ERROR;
        } else {
            if (JSI_OK != Jsi_CommandInvokeJSON(interp, Jsi_DSValue(&se->func), Jsi_DSValue(&se->data), NULL))
                rc = JSI_ERROR;
        }
        if (!isExec) {
            Jsi_DSFree(&se->func);
            Jsi_DSFree(&se->data);
        }
        if (Jsi_MutexLock(interp, interp->QMutex) != JSI_OK)
            return JSI_ERROR;
        interp->interpStrEvents = se->next;
        if (!isExec) Jsi_Free(se);
        se = interp->interpStrEvents;
        Jsi_MutexUnlock(interp, interp->QMutex);
        if (isExec)
            oldse->isExec = 0;
    }

    return rc;
}


static int ThreadEvalCallback(Jsi_Interp *interp, void* data) {
    int rc;

    if ((rc=SubInterpEvalCallback(interp, data)) != JSI_OK)
        interp->threadErrCnt++;
    return rc;
}

/* Create an event handler in interp to handle call/eval/send asyncronously via 'Sys.update()'. */
void jsi_AddEventHandler(Jsi_Interp *interp)
{
    Jsi_Event *ev;
    while (!interp->hasEventHdl) { /* Find an empty event slot. */
        int isNew, id;
        id = interp->eventIdx++;
        Jsi_HashEntry *hPtr = Jsi_HashEntryNew(interp->eventTbl, (void*)id, &isNew);
        if (!isNew)
            continue;
        ev = (Jsi_Event*)Jsi_Calloc(1, sizeof(*ev));
        SIGINIT(ev,EVENT);
        ev->id = id;
        ev->handler = ThreadEvalCallback;
        ev->hPtr = hPtr;
        ev->evType = JSI_EVENT_ALWAYS;
        Jsi_HashValueSet(hPtr, ev);
        interp->hasEventHdl = 1;
    }
}

#define FN_call JSI_INFO("\
Invoke function in sub-interp with arguments.  Since interps are not allowed to share objects, \
data is automatically cleansed by encoding/decoding \
to/from JSON if required.  Unless an 'async' parameter of true is given, we wait until the sub-interp is idle, \
make the call, and return the result.  Otherwise the call is acyncronous.")

static int InterpCallCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    int isthrd;
    Jsi_Interp *sinterp;
    if (udf)
        sinterp = udf->subinterp;
    else {
        Jsi_LogError("Apply Interp.call in a non-subinterp");
        return JSI_ERROR;
    }
    if (Jsi_InterpGone(sinterp)) {
        Jsi_LogError("Sub-interp gone");
        return JSI_ERROR;
    }
    isthrd = (interp->threadId != sinterp->threadId);
    
    Jsi_Value *func = NULL;
    char *fname = NULL; 
    func = Jsi_ValueArrayIndex(interp, args, 0);   
    fname = Jsi_ValueString(interp, func, NULL);
    if (!fname) {
        Jsi_LogError("function name must be a string");
        return JSI_ERROR;
    }
    if (Jsi_MutexLock(interp, sinterp->Mutex) != JSI_OK)
        return JSI_ERROR;
    Jsi_Value *namLU = Jsi_NameLookup(sinterp, fname);
    Jsi_MutexUnlock(interp, sinterp->Mutex);
    if (namLU == NULL) {
        Jsi_LogError("unknown function: %s", fname);
        return JSI_ERROR;
    }
    
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    char *cp = Jsi_ValueString(interp, arg, NULL);

    if (cp == NULL && !Jsi_ValueIsArray(interp, arg)) {
        Jsi_LogError("expected string or array");
        return JSI_ERROR;
    }

    Jsi_Value *vasync = Jsi_ValueArrayIndex(interp, args, 2);
    int rc = JSI_OK, async = 0;
    if (vasync && Jsi_GetBoolFromValue(interp, vasync, &async))
        return JSI_ERROR;
    
    if (!async) {
        Jsi_DString dStr = {};
        if (cp == NULL)
            cp = (char*)Jsi_ValueGetDString(interp, arg, &dStr, JSI_OUTPUT_JSON);
        if (interp->doUnlock) Jsi_MutexUnlock(interp, interp->Mutex);
        if (Jsi_MutexLock(interp, sinterp->Mutex) != JSI_OK) {
            if (interp->doUnlock) Jsi_MutexLock(interp, interp->Mutex);
            return JSI_ERROR;
        }
        Jsi_Value *srPtr = Jsi_ValueNew1(interp);
        /* TODO: call from this interp may not be safe if threaded. 
         * Could instead use async code below then wait for unlock on se->mutex. */
        rc = Jsi_CommandInvokeJSON(sinterp, fname, cp, &srPtr);
        Jsi_DSSetLength(&dStr, 0);
        ConvertReturn(interp, sinterp, &srPtr);
        Jsi_ValueCopy(interp, *ret, srPtr);
        Jsi_DecrRefCount(interp, srPtr);
        Jsi_DSFree(&dStr);
        Jsi_MutexUnlock(interp, sinterp->Mutex);
        if (interp->doUnlock && Jsi_MutexLock(interp, interp->Mutex) != JSI_OK) {
            Jsi_LogBug("mutex re-get failed");
            return JSI_ERROR;
        }
        return rc;
    }
    
    /* Post to thread event in sub-interps queue. */
    if (Jsi_MutexLock(interp, sinterp->QMutex) != JSI_OK)
        return JSI_ERROR;
        
    /* Is an async call. */
    InterpStrEvent *se, *s = (InterpStrEvent *)Jsi_Calloc(1, sizeof(*s));
    // TODO: is s->data inited?
    if (!cp) {
        Jsi_ValueGetDString(interp, arg, &s->data, JSI_OUTPUT_JSON);
    }
    Jsi_DSInit(&s->data);
    Jsi_DSAppend(&s->data, cp, NULL);
    Jsi_DSInit(&s->func);
    Jsi_DSAppend(&s->func, fname, NULL);
    se = sinterp->interpStrEvents;
    if (!se)
        sinterp->interpStrEvents = s;
    else {
        while (se->next)
            se = se->next;
        se->next = s;
    }

    Jsi_MutexUnlock(interp, sinterp->QMutex);
    if (!isthrd) {
        if (SubInterpEvalCallback(sinterp, NULL) != JSI_OK)
            sinterp->threadErrCnt++;
    }
    return JSI_OK;
}

int Jsi_Mount( Jsi_Interp *interp, Jsi_Value *archive, Jsi_Value *mount, Jsi_Value **ret)
{
#ifdef JSI_OMIT_ZVFS
    return JSI_ERROR;
#else
    return Zvfs_Mount(interp, archive, mount, ret);
#endif
}

int Jsi_AddIndexFiles(Jsi_Interp *interp, const char *dir) {
    /* Look for jsiIndex.js to setup. */
    Jsi_DString dStr = {};
    Jsi_StatBuf stat;
    int i, cnt = 0;
    for (i=0; i<2; i++) {
        Jsi_DSAppend(&dStr, dir, (i==0?"/lib":""),"/jsiIndex.jsi", NULL);
        Jsi_Value *v = Jsi_ValueNewStringKey(interp, Jsi_DSValue(&dStr));
        if (Jsi_Stat(interp, v, &stat) != 0)
            Jsi_ValueFree(interp, v);
        else {
            if (!interp->indexFiles) {
                interp->indexFiles = Jsi_ValueNewArray(interp, 0, 0);
                Jsi_IncrRefCount(interp, interp->indexFiles);
            }
            Jsi_ObjArrayAdd(interp, interp->indexFiles->d.obj, v);
            cnt++;
            interp->indexLoaded = 0;
        }
        Jsi_DSSetLength(&dStr, 0);
    }
    Jsi_DSFree(&dStr);
    return cnt;
}

int Jsi_ExecZip(Jsi_Interp *interp, const char *exeFile, const char *mntDir, int *jsFound)
{
#ifndef JSI_OMIT_ZVFS
    Jsi_StatBuf stat;
    Jsi_Value *vinit, *vmnt = NULL;
    Jsi_Value *vexe = Jsi_ValueNewStringKey(interp, exeFile);
    Jsi_Value *ret = NULL;
    int rc;
    const char *omntDir = mntDir;
    Jsi_IncrRefCount(interp, vexe);
    Jsi_HashSet(interp->genValueTbl, vexe, vexe);
    if (jsFound)
        *jsFound = 0;
    if (!mntDir)
        ret = Jsi_ValueNew(interp);
    else {
        vmnt = Jsi_ValueNewStringKey(interp, mntDir);
        Jsi_IncrRefCount(interp, vmnt);
        Jsi_HashSet(interp->genValueTbl, vmnt, vmnt);
    }
    rc =Jsi_Mount(interp, vexe, vmnt, &ret);
    if (rc != JSI_OK)
        return -1;
    Jsi_DString dStr = {};
    if (!mntDir)
        mntDir = Jsi_ValueString(interp, ret, NULL);
    Jsi_DSAppend(&dStr, mntDir, "/main.jsi", NULL);
    vinit = Jsi_ValueNewStringKey(interp,  Jsi_DSValue(&dStr));
    Jsi_IncrRefCount(interp, vinit);
    Jsi_HashSet(interp->genValueTbl, vinit, vinit);
    Jsi_DSFree(&dStr);
    if (Jsi_Stat(interp, vinit, &stat) == 0) {
        if (jsFound)
            *jsFound |= JSI_ZIP_MAIN;
        interp->execZip = vexe;
        return Jsi_EvalFile(interp, vinit, JSI_EVAL_ARGV0|JSI_EVAL_INDEX);
    } else {
        if (Jsi_AddIndexFiles(interp, mntDir) && omntDir)
            *jsFound = JSI_ZIP_INDEX;
    }
#endif
    return JSI_OK;
}

#define FN_send JSI_INFO("\
Add messages to queue to be processed by the 'recvCmd' interp option.")

static int InterpSendCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    //return SendCmd(interp, args, _this, ret, funcPtr, 1);
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    Jsi_Interp *sinterp = NULL;
    int isthrd;
    if (udf) {
        sinterp = udf->subinterp;
    } else {
        sinterp = interp->parent;
        if (!sinterp) {
            Jsi_LogError("Apply Interp.send in a non-subinterp");
            return JSI_ERROR;
        }
    }

    if (Jsi_InterpGone(sinterp)) {
        Jsi_LogError("Sub-interp gone");
        return JSI_ERROR;
    }
    isthrd = (interp->threadId != sinterp->threadId);
    if (!sinterp->recvCmd) {
        Jsi_LogError("interp was not created with 'recvCmd' option");
        return JSI_ERROR;
    }

    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    char *cp = Jsi_ValueString(interp, arg, NULL);

    /* Post to thread event in sub-interps queue. */
    if (Jsi_MutexLock(interp, sinterp->QMutex) != JSI_OK)
        return JSI_ERROR;
        
    int slen = Jsi_Strlen(Jsi_DSValue(&sinterp->interpMsgQ));
    Jsi_DString eStr = {};
    if (!cp) {
        cp = (char*)Jsi_ValueGetDString(interp, arg, &eStr, JSI_OUTPUT_JSON);
    }
    if (!slen)
        Jsi_DSAppend(&sinterp->interpMsgQ, "[", cp, "]", NULL);
    else {
        Jsi_DSSetLength(&sinterp->interpMsgQ, slen-1);
        Jsi_DSAppend(&sinterp->interpMsgQ, ", ", cp, "]", NULL);
    }
    // if (interp->parent) printf("SENDING: %s\n", Jsi_DSValue(&sinterp->interpMsgQ));
    Jsi_DSFree(&eStr);

    Jsi_MutexUnlock(interp, sinterp->QMutex);
    if (!isthrd) {
        if (SubInterpEvalCallback(sinterp, NULL) != JSI_OK)
            sinterp->threadErrCnt++;
    }
    return JSI_OK;

}

#ifndef JSI_OMIT_THREADS

#ifdef __WIN32
#define JSITHREADRET void
#else
#define JSITHREADRET void*
#endif

static JSITHREADRET NewInterpThread(void* iPtr)
{
    int rc = JSI_OK;
    InterpObj *udf = (InterpObj *)iPtr;
    Jsi_Interp *interp = udf->subinterp;
    interp->threadId = Jsi_CurrentThread();
   if (interp->scriptStr)
        rc = Jsi_EvalString(interp, interp->scriptStr, 0);
    else if (interp->scriptStr)
        rc = Jsi_EvalFile(interp, interp->scriptFile, 0);
    else {
        jsi_AddEventHandler(interp); 
        int mrc = Jsi_MutexLock(interp, interp->Mutex);      
        while (mrc == JSI_OK) {
            if (Jsi_EventProcess(interp, -1)<0)
                break;
            Jsi_Sleep(interp, 1);
        }
    }
    if (rc != JSI_OK) {
        Jsi_LogError("eval failure");
        interp->threadErrCnt++;
    }
    interpObjErase(udf);
#ifndef __WIN32
    /* TODO: should we wait/notify parent??? */
    pthread_detach(pthread_self());
    return NULL;
#endif
}
#endif


#define FN_interp JSI_INFO("\
The new interp may optionally be threaded.")
static int InterpConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
static int InterpConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
    
static Jsi_CmdSpec interpCmds[] = {
    { "Interp", InterpConstructor,0,  1,  "options:object=void", JSI_CMD_IS_CONSTRUCTOR, .help="Create a new interp", .info=FN_interp, .opts=InterpOptions, .retType=(uint)JSI_TT_USEROBJ },
    { "alias",  InterpAliasCmd,   0,  3, "name:string=void, func:function|null=void, args:array|function|null=void",.help="Set/get alias command in the interp", .retType=(uint)JSI_TT_ANY },
    { "conf",   InterpConfCmd,    0,  1, "val:string|object=void",.help="Configure options" , .opts=InterpOptions, .retType=(uint)JSI_TT_ANY },
    { "send",   InterpSendCmd,    1,  1, "msg", .help="Send message to enqueue on subinterps recvCmd handler", .info=FN_send, .retType=(uint)JSI_TT_ANY },
    { "eval",   InterpEvalCmd,    1,  2, "js:string, async:boolean=false", .help="Interpet javascript code within subinterp", .info=FN_eval, .retType=(uint)JSI_TT_ANY },
    { "info",   InterpInfoCmd,    0,  0,  "", .help="Return detailed info about interp", .retType=(uint)JSI_TT_OBJECT },
    { "call",   InterpCallCmd,    2,  3, "funcName:string, args:string|array, async:boolean=false", .help="Call named function in subinterp", .info=FN_call, .retType=(uint)JSI_TT_ANY },
    { NULL,     NULL, .help="Commands for accessing interps" }
};
/*
    { "alias",      InterpAliasCmd,     NULL,   3,  3, "interp,funcName,function",  },
    { "aliases",    InterpAliasesCmd,   NULL,   1,  2, "interp?,funcName?",  },
    { "children",   InterpChildrenCmd,  NULL,   0,  0, "",  },
    { "create",     InterpCreateCmd,    NULL,   0,  1, "?isSafe?",  },
    { "delete",     InterpDeleteCmd,    NULL,   1,  1, "interp",  },
    { "exists",     InterpExistsCmd,    NULL,   1,  1, "interp",  },
    { "eval",       InterpEvalCmd,      NULL,   2,  2, "interp,string",  },
    { "limit",      InterpLimitCmd,     NULL,   1,  2, "interp?,options?",  },
*/

static Jsi_UserObjReg interpobject = {
    "Interp",
    interpCmds,
    interpObjFree,
    interpObjIsTrue,
    interpObjEqual
};


static int InterpConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Obj *fobj;
    Jsi_Value *toacc;
    InterpObj *cmdPtr = (InterpObj *)Jsi_Calloc(1,sizeof(*cmdPtr));
    SIGINIT(cmdPtr,INTERPOBJ);
    cmdPtr->parent = interp;

    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);

    if (!(cmdPtr->subinterp = Jsi_InterpNew(interp, 0,0, arg))) {
        Jsi_Free(cmdPtr);
        return JSI_ERROR;
    }
    Jsi_Interp *sinterp = cmdPtr->subinterp;
    if (sinterp->scriptStr != 0 && sinterp->scriptFile != 0) {
        Jsi_LogError("can not use both scriptStr and scriptFile options");
        goto bail;
    }

    toacc = NULL;
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        toacc = _this;
    } else {
        Jsi_Obj *o = Jsi_ObjNew(interp);
        Jsi_PrototypeObjSet(interp, "Interp", o);
        Jsi_ValueMakeObject(interp, ret, o);
        toacc = *ret;
    }

    fobj = Jsi_ValueGetObj(interp, toacc);
    if ((cmdPtr->objId = Jsi_UserObjNew(interp, &interpobject, fobj, cmdPtr))<0)
        goto bail;
    cmdPtr->fobj = fobj;
    cmdPtr->aliases = Jsi_HashNew(interp, JSI_KEYS_STRING, AliasFree);
#ifndef JSI_OMIT_THREADS
    if (sinterp->subthread) {
       /* if (sinterp->scriptStr == 0 && sinterp->scriptFile == 0) {
            Jsi_LogError("must give scriptStr or scriptFile option with thread");
            goto bail;
        }*/
#ifdef __WIN32
        if (!_beginthread( NewInterpThread, 0, cmdPtr )) {
            Jsi_LogError("thread create failed");
            goto bail;
        }
#else
        pthread_t nthread;
        if (pthread_create(&nthread, NULL, NewInterpThread, cmdPtr) != 0) {
            Jsi_LogError("thread create failed");
            goto bail;
        }
#endif
#else
    if (0) {
#endif
    } else {
        int rc = JSI_OK;
        if (sinterp->scriptStr != 0) {
            rc = Jsi_EvalString(sinterp, sinterp->scriptStr, 0);
        } else if (sinterp->scriptFile != 0) {
            rc = Jsi_EvalFile(sinterp, sinterp->scriptFile, 0);        
        }
        if (rc != JSI_OK)
            goto bail;
    }
    return JSI_OK;
    
bail:
    interpObjErase(cmdPtr);
    Jsi_ValueMakeUndef(interp, ret);
    return JSI_ERROR;
}

int Jsi_DoneInterp(Jsi_Interp *interp)
{
    Jsi_UserObjUnregister(interp, &interpobject);
    return JSI_OK;
}

static int InterpConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
  
    return Jsi_OptionsConf(interp, InterpOptions, Jsi_ValueArrayIndex(interp, args, 0), udf?udf->subinterp:interp, ret, 0);

}

Jsi_Value *Jsi_ReturnValue(Jsi_Interp *interp) {
    return interp->retPtr;
}

int jsi_InterpInit(Jsi_Interp *interp)
{
    Jsi_Hash *isys;
    if (!(isys = Jsi_UserObjRegister(interp, &interpobject)))
        Jsi_LogFatal("Can not init interp\n");

    Jsi_CommandCreateSpecs(interp, interpobject.name, interpCmds, isys, 0);
    return JSI_OK;
}

int Jsi_InterpSafe(Jsi_Interp *interp)
{
    return interp->isSafe;
}

int Jsi_InterpAccess(Jsi_Interp *interp, Jsi_Value* file, int toWrite)
{
    Jsi_Value *v, *dirs = (toWrite ? interp->safeWriteDirs : interp->safeReadDirs);
    if (!interp->isSafe)
        return JSI_OK;
    if (!dirs)
        return JSI_ERROR;
    int i, n, m, argc = Jsi_ValueGetLength(interp, dirs);
    char *str, *dstr = Jsi_ValueString(interp, file, &n); /* TODO: normalize? */
    if (!dstr)
        return JSI_ERROR;
    char *scp = strrchr(dstr, '/');
    if (scp)
        n -= strlen(scp);
    for (i=0; i<argc; i++) {
        v = Jsi_ValueArrayIndex(interp, dirs, i);
        str = Jsi_ValueString(interp, v, &m);
        if (v && str && strncmp(str, dstr, m) == 0 && (n==m || dstr[m] == '/'))
            return JSI_OK;
    }
    return JSI_ERROR;
}

#endif
