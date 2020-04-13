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

#if (JSI_VERSION_MINOR<0 || JSI_VERSION_MINOR>99 || JSI_VERSION_RELEASE<0 || JSI_VERSION_RELEASE>99)
#error "VERSION MINOR/RELEASE not between 0-99 inclusive"
#endif

static Jsi_OptionSpec InterpDebugOptions[] = {
    JSI_OPT(CUSTOM,Jsi_DebugInterp, debugCallback,  .help="Command in parent interp for handling debugging", .flags=0, .custom=Jsi_Opt_SwitchParentFunc, .data=(void*)"file:string, line:number, level:number, func:string, opstr:string, bpId:number, emsg:string" ),
    JSI_OPT(BOOL,  Jsi_DebugInterp, doContinue,     .help="Continue execution until breakpoint" ),
    JSI_OPT(BOOL,  Jsi_DebugInterp, forceBreak,     .help="Force debugger to break" ),
    JSI_OPT(BOOL,  Jsi_DebugInterp, includeOnce,    .help="Source the file only if not already sourced" ),
    JSI_OPT(BOOL,  Jsi_DebugInterp, includeTrace,   .help="Trace includes" ),
    JSI_OPT(INT,   Jsi_DebugInterp, minLevel,       .help="Disable eval callback for level higher than this" ),
    JSI_OPT(CUSTOM,Jsi_DebugInterp, msgCallback,    .help="Comand in parent interp to handle Jsi_LogError/Jsi_LogWarn,..", .flags=0, .custom=Jsi_Opt_SwitchParentFunc, .data=(void*)"msg:string, type:string, file:string, line:number, col:number" ),
    JSI_OPT(BOOL,  Jsi_DebugInterp, pkgTrace,       .help="Trace package loads" ),
    JSI_OPT(CUSTOM,Jsi_DebugInterp, putsCallback,   .help="Comand in parent interp to handle puts output", .flags=0, .custom=Jsi_Opt_SwitchParentFunc, .data=(void*)"msg:string, isStderr:number" ),
    JSI_OPT(CUSTOM,Jsi_DebugInterp, traceCallback,  .help="Comand in parent interp to handle traceCall", .flags=0, .custom=Jsi_Opt_SwitchParentFunc, .data=(void*)"cmd:string, args:string, ret:string, file:string, line:number, col:number" ),
    JSI_OPT(CUSTOM,Jsi_DebugInterp, testFmtCallback,.help="Comand in parent interp to format unittest string", .flags=0, .custom=Jsi_Opt_SwitchParentFunc, .data=(void*)"cmd:string, line:number" ),
    JSI_OPT_END(Jsi_DebugInterp, .help="Interp options for debugging")
};

Jsi_OptionSpec jsi_InterpLogOptions[] = {
    JSI_OPT(BOOL,   jsi_LogOptions, Test,    .help="Enable LogTest messages" ),
    JSI_OPT(BOOL,   jsi_LogOptions, Debug,   .help="Enable LogDebug messages" ),
    JSI_OPT(BOOL,   jsi_LogOptions, Trace,   .help="Enable LogTrace messages" ),
    JSI_OPT(BOOL,   jsi_LogOptions, Info,    .help="Enable LogInfo messages" ),
    JSI_OPT(BOOL,   jsi_LogOptions, Warn,    .help="Enable LogWarn messages" ),
    JSI_OPT(BOOL,   jsi_LogOptions, Error,   .help="Enable LogError messages" ),
    JSI_OPT(BOOL,   jsi_LogOptions, time,    .help="Prefix with time" ),
    JSI_OPT(BOOL,   jsi_LogOptions, date,    .help="Prefix with date" ),
    JSI_OPT(BOOL,   jsi_LogOptions, file,    .help="Ouptut contains file:line" ),
    JSI_OPT(BOOL,   jsi_LogOptions, func,    .help="Output function" ),
    JSI_OPT(BOOL,   jsi_LogOptions, full,    .help="Show full file path" ),
    JSI_OPT(BOOL,   jsi_LogOptions, ftail,   .help="Show tail of file only, even in LogWarn, etc" ),
    JSI_OPT(BOOL,   jsi_LogOptions, before,  .help="Output file:line before message string" ),
    JSI_OPT(BOOL,   jsi_LogOptions, isUTC,   .help="Time is to be UTC" ),
    JSI_OPT(STRKEY, jsi_LogOptions, timeFmt, .help="A format string to use with strftime" ),
    JSI_OPT(USEROBJ,jsi_LogOptions, chan,    .help="Channel to send output to", .flags=0, .custom=0, .data=(void*)"Channel" ),
    JSI_OPT_END(jsi_LogOptions, .help="Interp options for logging")
};
static Jsi_OptionSpec InterpSubOptions[] = {
    JSI_OPT(STRKEY,jsi_SubOptions, blacklist,   .help="Comma separated modules to disable loading for", jsi_IIOF ),
    JSI_OPT(BOOL,  jsi_SubOptions, compat,      .help="Ignore unknown options via JSI_OPTS_IGNORE_EXTRA in option parser" ),
    JSI_OPT(INT,   jsi_SubOptions, dblPrec,     .help="Format precision of double where 0=max, -1=max-1, ... (max-1)" ),
    JSI_OPT(BOOL,  jsi_SubOptions, istty,       .help="Indicates interp is in interactive mode", jsi_IIRO),
    JSI_OPT(BOOL,  jsi_SubOptions, logColNums,  .help="Display column numbers in error messages"),
    JSI_OPT(BOOL,  jsi_SubOptions, logAllowDups,.help="Log should not filter out duplicate messages"),
    JSI_OPT(BOOL,  jsi_SubOptions, mutexUnlock, .help="Unlock own mutex when evaling in other interps (true)", jsi_IIOF),
    JSI_OPT(BOOL,  jsi_SubOptions, noproto,     .help="Disable support of the OOP symbols:  __proto__, prototype, constructor, etc"),
    JSI_OPT(BOOL,  jsi_SubOptions, noFuncString,.help="Disable viewing code body for functions", jsi_IIOF),
    JSI_OPT(BOOL,  jsi_SubOptions, noRegex,     .help="Disable viewing code for functions", jsi_IIOF),
    JSI_OPT(BOOL,  jsi_SubOptions, noReadline,  .help="In interactive mode disable use of readline" ),
    JSI_OPT(BOOL,  jsi_SubOptions, outUndef,    .help="In interactive mode output result values that are undefined"),
    JSI_OPT(STRKEY,jsi_SubOptions, prompt,      .help="Prompt for interactive mode ('$ ')" ),
    JSI_OPT(STRKEY,jsi_SubOptions, prompt2,     .help="Prompt for interactive mode line continue ('> ')" ),
    JSI_OPT_END(jsi_SubOptions, .help="Lesser sub-feature options")
};

static const char *jsi_SafeModeStrs[] = { "none", "read", "write", "writeRead", "lockdown", NULL };
static const char *jsi_TypeChkStrs[] = { "parse", "run", "all", "error", "strict", "noundef", "nowith", "funcsig", NULL };
const char *jsi_callTraceStrs[] = { "funcs", "cmds", "new", "return", "args", "notrunc", "noparent", "full", "before", NULL};
const char *jsi_AssertModeStrs[] = { "throw", "log", "puts", NULL};

static Jsi_OptionSpec InterpOptions[] = {
    JSI_OPT(ARRAY, Jsi_Interp, args,        .help="The console.arguments for interp", jsi_IIOF),
    JSI_OPT(BOOL,  Jsi_Interp, asserts,     .help="Enable assert" ),
    JSI_OPT(CUSTOM,Jsi_Interp, assertMode,  .help="Action upon assert failure", .flags=0, .custom=Jsi_Opt_SwitchEnum, .data=jsi_AssertModeStrs ),
    JSI_OPT(ARRAY, Jsi_Interp, autoFiles,   .help="File(s) to source for loading Jsi_Auto to handle unknown commands"),
    JSI_OPT(CUSTOM,Jsi_Interp, busyCallback,.help="Command in parent interp (or noOp) to periodically call", .flags=0, .custom=Jsi_Opt_SwitchParentFunc, .data=(void*)"interpName:string, opCnt:number"),
    JSI_OPT(INT   ,Jsi_Interp, busyInterval,.help="Call busyCallback command after this many op-code evals (100000)"),
    JSI_OPT(STRKEY,Jsi_Interp, confFile,    .help="Config file of options in non-strict JSON form", jsi_IIOF|JSI_OPT_LOCKSAFE),
    JSI_OPT(BOOL,  Jsi_Interp, coverage,    .help="On exit generate detailed code coverage for function calls (with profile)"),
    JSI_OPT(CUSTOM,Jsi_Interp, debugOpts,   .help="Options for debugging", .flags=0, .custom=Jsi_Opt_SwitchSuboption, .data=InterpDebugOptions),
    JSI_OPT(BOOL,  Jsi_Interp, interactive, .help="Force interactive mode. ie. ignore no_interactive flag", jsi_IIOF),
    JSI_OPT(BOOL,  Jsi_Interp, hasOpenSSL,  .help="Is SSL available in WebSocket", jsi_IIOF),
    JSI_OPT(STRKEY,Jsi_Interp, historyFile, .help="In interactive mode, file to use for history (~/.jsish_history)", jsi_IIOF),
    JSI_OPT(BOOL,  Jsi_Interp, isSafe,      .help="Is this a safe interp (ie. with limited or no file access)", jsi_IIOF),
    JSI_OPT(STRKEY,Jsi_Interp, jsppChars,   .help="Line preprocessor when sourcing files. Line starts with first char, and either ends with it, or matches string"),
    JSI_OPT(FUNC,  Jsi_Interp, jsppCallback,.help="Command to preprocess lines that match jsppChars. Call func(interpName:string, opCnt:number)"),
    JSI_OPT(INT,   Jsi_Interp, lockTimeout, .help="Thread time-out for mutex lock acquires (milliseconds)" ),
    JSI_OPT(CUSTOM,Jsi_Interp, logOpts,     .help="Options for log output to add file/line/time", .flags=0, .custom=Jsi_Opt_SwitchSuboption, .data=jsi_InterpLogOptions),
    JSI_OPT(INT,   Jsi_Interp, maxDepth,    .help="Depth limit of recursive function calls (1000)", .flags=JSI_OPT_LOCKSAFE),
    JSI_OPT(UINT,  Jsi_Interp, maxArrayList,.help="Maximum array convertable to list (100000)", .flags=JSI_OPT_LOCKSAFE),
    JSI_OPT(INT,   Jsi_Interp, maxIncDepth, .help="Maximum allowed source/require nesting depth (50)", .flags=JSI_OPT_LOCKSAFE),
    JSI_OPT(INT,   Jsi_Interp, maxInterpDepth,.help="Maximum nested subinterp create depth (10)", .flags=JSI_OPT_LOCKSAFE),
    JSI_OPT(INT,   Jsi_Interp, maxUserObjs, .help="Maximum number of 'new' object calls, eg. File, RegExp, etc", .flags=JSI_OPT_LOCKSAFE ),
    JSI_OPT(INT,   Jsi_Interp, maxOpCnt,    .help="Execution limit for op-code evaluation", jsi_IIOF|JSI_OPT_LOCKSAFE ),
    JSI_OPT(INT,   Jsi_Interp, memDebug,    .help="Memory debugging level: 1=summary, 2=detail", .flags=JSI_OPT_NO_CLEAR),
    JSI_OPT(STRKEY,Jsi_Interp, name,        .help="Optional text name for this interp"),
    JSI_OPT(BOOL,  Jsi_Interp, noAutoLoad,  .help="Disable autoload", .flags=JSI_OPT_LOCKSAFE ),
    JSI_OPT(BOOL,  Jsi_Interp, noConfig,    .help="Disable use of Interp.conf to change options after create", jsi_IIOF),
    JSI_OPT(BOOL,  Jsi_Interp, noInput,     .help="Disable use of console.input()" ),
    JSI_OPT(BOOL,  Jsi_Interp, noLoad,      .help="Disable load of shared libs", .flags=JSI_OPT_LOCKSAFE),
    JSI_OPT(BOOL,  Jsi_Interp, noNetwork,   .help="Disable new Socket/WebSocket, or load of builtin MySql" ),
    JSI_OPT(BOOL,  Jsi_Interp, noStderr,    .help="Make puts, log, assert, etc use stdout" ),
    JSI_OPT(BOOL,  Jsi_Interp, noSubInterps,.help="Disallow sub-interp creation"),
    JSI_OPT(FUNC,  Jsi_Interp, onComplete,  .help="Function to return commands completions for interactive mode.  Default uses Info.completions ", .flags=0, .custom=0, .data=(void*)"prefix:string, start:number, end:number" ),
    JSI_OPT(FUNC,  Jsi_Interp, onEval,      .help="Function to get control for interactive evals", .flags=0, .custom=0, .data=(void*)"cmd:string" ),
    JSI_OPT(FUNC,  Jsi_Interp, onExit,      .help="Command to call in parent on exit, returns true to continue", jsi_IIOF , .custom=0, .data=(void*)""),
    JSI_OPT(ARRAY, Jsi_Interp, pkgDirs,     .help="list of library directories for require() to search" ),
    JSI_OPT(BOOL,  Jsi_Interp, profile,     .help="On exit generate profile of function calls"),
    JSI_OPT(VALUE, Jsi_Interp, retValue,    .help="Return value from last eval", jsi_IIRO),
    JSI_OPT(CUSTOM,Jsi_Interp, safeMode,    .help="In safe mode source() support for pwd and script-dir ", jsi_IIOF, .custom=Jsi_Opt_SwitchEnum, .data=jsi_SafeModeStrs ),
    JSI_OPT(ARRAY, Jsi_Interp, safeReadDirs,.help="In safe mode, files/dirs to allow reads to", jsi_IIOF),
    JSI_OPT(ARRAY, Jsi_Interp, safeWriteDirs,.help="In safe mode, files/dirs to allow writes to", jsi_IIOF),
    JSI_OPT(STRKEY, Jsi_Interp,safeExecPattern,.help="In safe mode, regexp pattern allow exec of commands", jsi_IIOF),
    JSI_OPT(STRKEY,Jsi_Interp, scriptStr,   .help="Interp init script string", jsi_IIOF),
    JSI_OPT(STRING,Jsi_Interp, scriptFile,  .help="Interp init script file"),
    JSI_OPT(STRING,Jsi_Interp, stdinStr,    .help="String to use as stdin for console.input()"),
    JSI_OPT(STRING,Jsi_Interp, stdoutStr,   .help="String to collect stdout for puts()"),
    JSI_OPT(BOOL,  Jsi_Interp, strict,      .help="Globally enable strict: same as 'use strict' in main program"),
    JSI_OPT(CUSTOM,Jsi_Interp, subOpts,     .help="Infrequently used sub-options", .flags=0, .custom=Jsi_Opt_SwitchSuboption, .data=InterpSubOptions),
    JSI_OPT(BOOL,  Jsi_Interp, subthread,   .help="Create a threaded Interp", jsi_IIOF|JSI_OPT_LOCKSAFE),
    JSI_OPT(CUSTOM,Jsi_Interp, traceCall,   .help="Trace commands", .flags=0,  .custom=Jsi_Opt_SwitchBitset,  .data=jsi_callTraceStrs),
    JSI_OPT(INT,   Jsi_Interp, traceOp,     .help="Set debugging level for OPCODE execution"),
    JSI_OPT(BOOL,  Jsi_Interp, tracePuts,   .help="Trace puts by making it use logOpts" ),
    JSI_OPT(CUSTOM,Jsi_Interp, typeCheck,   .help="Type-check control options", .flags=0, .custom=Jsi_Opt_SwitchBitset, .data=jsi_TypeChkStrs),
    JSI_OPT(INT,   Jsi_Interp, typeWarnMax, .help="Type checking is silently disabled after this many warnings (50)" ),
    JSI_OPT(OBJ,   Jsi_Interp, udata,       .help="User data"),
    JSI_OPT(UINT,  Jsi_Interp, unitTest,    .help="Unit test control bits: 1=subst, 2=Puts with file:line prefix" ),
    JSI_OPT_END(Jsi_Interp, .help="Options for the Jsi interpreter")
};

/* Object for each interp created. */
typedef struct InterpObj {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_Interp *subinterp;
    Jsi_Interp *parent;
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
    Jsi_Interp *dinterp; // Dest interp.
    Jsi_Interp *subinterp;
} AliasCmd;


static void interpObjErase(InterpObj *fo);
static Jsi_RC interpObjFree(Jsi_Interp *interp, void *data);
static bool interpObjIsTrue(void *data);
static bool interpObjEqual(void *data1, void *data2);

static Jsi_RC jsi_InterpConfFiles(Jsi_Interp *interp);

/* Call a command with JSON args.  Returned string by using Jsi_ValueGetDString(..., flags). */
Jsi_RC Jsi_EvalCmdJSON(Jsi_Interp *interp, const char *cmd, const char *jsonArgs, Jsi_DString *dStr, int flags)
{
    if (Jsi_MutexLock(interp, interp->Mutex) != JSI_OK)
        return JSI_ERROR;
    Jsi_Value *nrPtr = Jsi_ValueNew1(interp);
    Jsi_RC rc = Jsi_CommandInvokeJSON(interp, cmd, jsonArgs, &nrPtr);
    Jsi_DSInit(dStr);
    Jsi_ValueGetDString(interp, nrPtr, dStr, flags /*JSI_OUTPUT_JSON*/);
    Jsi_DecrRefCount(interp, nrPtr);
    Jsi_MutexUnlock(interp, interp->Mutex);
    return rc;
}

/* Call a function with JSON args.  Return a primative. */
Jsi_RC Jsi_FunctionInvokeJSON(Jsi_Interp *interp, Jsi_Value *func, const char *json, Jsi_Value **ret)
{
    if (!Jsi_ValueIsFunction(interp, func))
        return JSI_ERROR;
    Jsi_Value *aPtr = Jsi_ValueNew1(interp);
    Jsi_RC rc = Jsi_JSONParse(interp, json, &aPtr, 0);
    if (rc == JSI_OK)
        rc = Jsi_FunctionInvoke(interp, func, aPtr, ret, NULL);
    Jsi_DecrRefCount(interp, aPtr);
    return rc;
}
/* Lookup cmd from cmdstr and invoke with JSON args. */
/*
 *   Jsi_CommandInvokeJSON(interp, "info.cmds", "[\"*\",true]", ret);
 */
Jsi_RC Jsi_CommandInvokeJSON(Jsi_Interp *interp, const char *cmdstr, const char *json, Jsi_Value **ret)
{
    Jsi_Value *func = Jsi_NameLookup(interp, cmdstr);
    if (func)
        return Jsi_FunctionInvokeJSON(interp, func, json, ret);
    return Jsi_LogError("can not find cmd: %s", cmdstr);
}

/* Clean-copying of value between interps: uses JSON parse if needed. */
Jsi_RC Jsi_CleanValue(Jsi_Interp *interp, Jsi_Interp *tointerp, Jsi_Value *val, Jsi_Value **ret)
{
    Jsi_RC rc = JSI_OK;
    const char *cp;
    int len, iskey;
    Jsi_Obj *obj;
    switch (val->vt) {
        case JSI_VT_UNDEF: Jsi_ValueMakeUndef(interp, ret); return rc;
        case JSI_VT_NULL: Jsi_ValueMakeNull(tointerp, ret); return rc;
        case JSI_VT_BOOL: Jsi_ValueMakeBool(tointerp, ret, val->d.val); return rc;
        case JSI_VT_NUMBER: Jsi_ValueMakeNumber(tointerp, ret, val->d.num); return rc;
        case JSI_VT_STRING:
            iskey = val->f.bits.isstrkey;
            cp = val->d.s.str;
            len = val->d.s.len;
makestr:
            if (iskey) {
                Jsi_ValueMakeStringKey(interp, ret, cp);
                return rc;
            }
            jsi_ValueMakeBlobDup(tointerp, ret, (uchar*)cp, len);
            return rc;
        case JSI_VT_OBJECT:
            obj = val->d.obj;
            switch (obj->ot) {
                case JSI_OT_BOOL: Jsi_ValueMakeBool(tointerp, ret, obj->d.val); return rc;
                case JSI_OT_NUMBER: Jsi_ValueMakeNumber(tointerp, ret, obj->d.num); return rc;
                case JSI_OT_STRING:
                    cp = obj->d.s.str;
                    len = obj->d.s.len;
                    iskey = obj->isstrkey;
                    goto makestr;
                default: break;
            }
            break;
        default:
            break;
    }
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    cp = Jsi_ValueGetDString(interp, val, &dStr, JSI_OUTPUT_JSON);
    if (Jsi_JSONParse(tointerp, cp, ret, 0) != JSI_OK) {
        Jsi_DSFree(&dStr);
        return Jsi_LogWarn("bad JSON parse in subinterp");
    }
    Jsi_DSFree(&dStr);
    return rc;
}

/* Invoke command in target interp. */
Jsi_RC jsi_AliasInvoke(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    AliasCmd *ac = (AliasCmd *)funcPtr->cmdSpec->reserved[2];
    Jsi_Interp *dinterp = ac->dinterp;
    Jsi_Value *nargs = NULL;
    int inc=0, argc = Jsi_ValueGetLength(interp, args);
    if (!ac) {
        Jsi_LogBug("BAD ALIAS INVOKE OF DELETED");
        return JSI_ERROR;
    }
    SIGASSERT(ac,ALIASCMD);
    bool isthrd = (interp != dinterp && interp->threadId != dinterp->threadId);
    Jsi_Value *nrPtr = Jsi_ValueNew1(dinterp);

     if (argc == 0 && ac->args)
        nargs = ac->args;
     else if (argc) {
        if (dinterp == interp)
            Jsi_ValueCopy(interp, nrPtr, args);
        else if (Jsi_CleanValue(interp, dinterp, args, &nrPtr) != JSI_OK)
            return JSI_ERROR;
        if (ac->args && Jsi_ValueGetLength(dinterp, ac->args)) {
            nargs = Jsi_ValueArrayConcat(dinterp, ac->args, nrPtr);
            Jsi_IncrRefCount(dinterp, nargs);
            inc=1;
        } else {
            nargs = nrPtr;
        }
    }

    if (isthrd) {
        /* Post to thread event in sub-interps queue. */
        if (Jsi_MutexLock(interp, dinterp->QMutex) != JSI_OK)
            return JSI_ERROR;

       /* Is an async call. */
        InterpStrEvent *se, *s = (InterpStrEvent *)Jsi_Calloc(1, sizeof(*s));
        // TODO: is s->data inited?
        Jsi_DSInit(&s->data);
        Jsi_ValueGetDString(interp, nargs, &s->data, JSI_OUTPUT_JSON);
        if (inc)
            Jsi_DecrRefCount(dinterp, nargs);
        Jsi_DecrRefCount(dinterp, nrPtr);
        s->acfunc = ac->func;
        Jsi_IncrRefCount(dinterp, ac->func);
        se = dinterp->interpStrEvents;
        if (!se)
            dinterp->interpStrEvents = s;
        else {
            while (se->next)
                se = se->next;
            se->next = s;
        }

        Jsi_MutexUnlock(interp, dinterp->QMutex);
        return JSI_OK;
    }

    if (dinterp != interp) {
        if (interp->subOpts.mutexUnlock) Jsi_MutexUnlock(interp, interp->Mutex);
        if (Jsi_MutexLock(interp, dinterp->Mutex) != JSI_OK) {
            if (interp->subOpts.mutexUnlock) Jsi_MutexLock(interp, interp->Mutex);
            return JSI_ERROR;
        }
    }
    ac->refCount++;
    Jsi_Value *srPtr, **srpPtr = ret;
    if (dinterp != interp) {
        srPtr = Jsi_ValueNew1(dinterp);
        srpPtr = &srPtr;
    }
    Jsi_RC rc = Jsi_FunctionInvoke(dinterp, ac->func, nargs, srpPtr, NULL);
    ac->refCount--;
    if (inc)
        Jsi_DecrRefCount(dinterp, nargs);
    Jsi_DecrRefCount(dinterp, nrPtr);
    if (dinterp != interp) {
        Jsi_MutexUnlock(interp, dinterp->Mutex);
        if (interp->subOpts.mutexUnlock && Jsi_MutexLock(interp, interp->Mutex) != JSI_OK) {
            return JSI_ERROR;
        }
    }
    if (dinterp != interp) {
        Jsi_CleanValue(dinterp, interp, *srpPtr, ret);
        Jsi_DecrRefCount(dinterp, srPtr);
        if (rc != JSI_OK && dinterp->errMsgBuf[0] && interp != dinterp) {
            Jsi_Strcpy(interp->errMsgBuf, dinterp->errMsgBuf);
            interp->errLine = dinterp->errLine;
            interp->errFile = dinterp->errFile;
            dinterp->errMsgBuf[0] = 0;
        }
    }
    return rc;
}


static Jsi_RC jsi_AliasFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *data) {
    /* TODO: deal with other copies of func may be floating around (refCount). */
    AliasCmd *ac = (AliasCmd *)data;
    if (!ac) return JSI_ERROR;
    SIGASSERT(ac,ALIASCMD);
    if (ac->func)
        Jsi_DecrRefCount(ac->dinterp, ac->func);
    if (ac->args)
        Jsi_DecrRefCount(ac->dinterp, ac->args);
    if (!ac->cmdVal)
        return JSI_OK;
    Jsi_Func *fobj = ac->cmdVal->d.obj->d.fobj->func;
    fobj->cmdSpec->reserved[2] = NULL;
    fobj->cmdSpec->proc = NULL;
    if (ac->intobj && ac->intobj->subinterp) {
        Jsi_CommandDelete(ac->intobj->subinterp, ac->cmdName);
        //if (Jsi_Strchr(ac->cmdName, '.'))
        //    Jsi_LogBug("alias free with X.Y dot name leaks memory: %s", ac->cmdName);
    } else
        Jsi_DecrRefCount(ac->subinterp, ac->cmdVal);
    _JSI_MEMCLEAR(ac);
    Jsi_Free(ac);
    return JSI_OK;
}

static Jsi_RC jsi_AliasCreateCmd(Jsi_Interp* interp, const char* key, AliasCmd* ac) {
    if (Jsi_InterpGone(interp))
        return JSI_ERROR;
    key = Jsi_KeyAdd(interp, key);
    Jsi_Value *cmd = jsi_CommandCreate(interp, key, jsi_AliasInvoke, NULL, 0, 0);
    if (!cmd)
        return Jsi_LogBug("command create failure");
    ac->cmdVal = cmd;
    Jsi_Func *fobj = cmd->d.obj->d.fobj->func;
    fobj->cmdSpec->reserved[2] = ac;
    cmd->d.obj->isNoOp = (ac->func->d.obj->d.fobj->func->callback == jsi_NoOpCmd);
    return JSI_OK;
}

#define FN_intalias JSI_INFO("With 0 args, returns list of all aliases in interp.\n\
With 1 arg returns func for given alias name.\n\
With 2 args where arg2 == null, returns args for given alias name .\n\
With 3 args, create/update an alias for func and args. \n\
Delete an alias by creating it with null for both func and args.")
static Jsi_RC InterpAliasCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    Jsi_Interp *sinterp = (udf ? udf->subinterp : interp );
    Jsi_Hash *aliases = sinterp->aliasHash;
    if (!aliases)
        return Jsi_LogError("Sub-interp gone");
    int argc = Jsi_ValueGetLength(interp, args);
    if (argc == 0)
        return Jsi_HashKeysDump(interp, aliases, ret, 0);
    Jsi_HashEntry *hPtr;
    char *key = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    if (!key)
        return Jsi_LogError("expected string");
    AliasCmd* ac;
    if (argc == 1) {
        hPtr = Jsi_HashEntryFind(aliases, (void*)key);
        if (!hPtr)
            return JSI_OK;
        ac = (AliasCmd*)Jsi_HashValueGet(hPtr);
        if (!ac) return JSI_ERROR;
        SIGASSERT(ac,ALIASCMD);
        Jsi_ValueDup2(interp, ret, ac->func);
        return JSI_OK;
    }
    Jsi_Value *afunc = Jsi_ValueArrayIndex(interp, args, 1);
    if (argc == 2) {
        hPtr = Jsi_HashEntryFind(aliases, (void*)key);
        if (!hPtr)
            return JSI_OK;
        ac = (AliasCmd*)Jsi_HashValueGet(hPtr);
        if (!Jsi_ValueIsNull(interp, afunc))
            return Jsi_LogError("arg 2: expected null to query args");
        if (!ac) return JSI_ERROR;
        SIGASSERT(ac,ALIASCMD);
        Jsi_ValueDup2(interp, ret, ac->args); //TODO: JSON??
        return JSI_OK;
    }
    
    if (argc < 3)
        return JSI_ERROR;
    bool isthrd = (interp->threadId != sinterp->threadId);
    //if (isthrd)
        //return Jsi_LogError("alias not supported with threads");
    bool isNew;
    Jsi_Value *aargs = Jsi_ValueArrayIndex(interp, args, 2);
    if (Jsi_ValueIsNull(interp, afunc) && Jsi_ValueIsNull(interp, aargs)) {
        hPtr = Jsi_HashEntryFind(aliases, (void*)key);
        if (hPtr == NULL)
            return JSI_OK;
        ac = (AliasCmd*)Jsi_HashValueGet(hPtr);
        if (!ac) return JSI_ERROR;
        if (0 && ac->cmdVal)
            Jsi_DecrRefCount(interp, ac->cmdVal);
        jsi_AliasFree(interp, NULL, ac);
        Jsi_HashValueSet(hPtr, NULL);
        Jsi_HashEntryDelete(hPtr);
        return JSI_OK;
    }
    hPtr = Jsi_HashEntryNew(aliases, (void*)key, &isNew);
    if (!hPtr)
        return Jsi_LogError("create failed: %s", key);
    if (!Jsi_ValueIsFunction(interp, afunc))
        return Jsi_LogError("arg 2: expected function");
    if (Jsi_ValueIsNull(interp, aargs) == 0 && Jsi_ValueIsArray(interp, aargs) == 0)
        return Jsi_LogError("arg 3: expected array or null");
    if (!isNew) {
        jsi_AliasFree(interp, NULL, Jsi_HashValueGet(hPtr));
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
    ac->dinterp = interp;
    ac->subinterp = sinterp;
    Jsi_HashValueSet(hPtr, ac);
    if (!isthrd)
        return jsi_AliasCreateCmd(sinterp, key, ac);

    Jsi_Value *vasync = Jsi_ValueArrayIndex(interp, args, 3);
    bool async = 0;
    if (vasync && Jsi_GetBoolFromValue(interp, vasync, &async))
        return JSI_ERROR;
        
    if (!async) {
        if (Jsi_MutexLock(interp, sinterp->Mutex) != JSI_OK)
            return JSI_ERROR;
        Jsi_RC rc = jsi_AliasCreateCmd(sinterp, key, ac);
        Jsi_MutexUnlock(interp, sinterp->Mutex);
        return rc;
    }

    /* Post to thread event in sub-interps queue. */
    if (Jsi_MutexLock(interp, sinterp->QMutex) != JSI_OK)
        return JSI_ERROR;

    /* Is an async call. */
    InterpStrEvent *se, *s = (InterpStrEvent *)Jsi_Calloc(1, sizeof(*s));
    // TODO: is s->data inited?
    s->acdata = ac;
    Jsi_DSInit(&s->func);
    Jsi_DSAppend(&s->func, ac->cmdName, NULL);
    se = sinterp->interpStrEvents;
    if (!se)
        sinterp->interpStrEvents = s;
    else {
        while (se->next)
            se = se->next;
        se->next = s;
    }

    Jsi_MutexUnlock(interp, sinterp->QMutex);
    return JSI_OK;
}

static Jsi_RC freeCodeTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    jsi_Pstate *ps = (jsi_Pstate *)ptr;
    if (!ps) return JSI_OK;
    ps->hPtr = NULL;
    jsi_PstateFree(ps);
    return JSI_OK;
}

static Jsi_RC freeOnDeleteTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    Jsi_DeleteProc *proc = (Jsi_DeleteProc *)ptr;
    proc(interp, NULL);
    return JSI_OK;
}

static Jsi_RC freeAssocTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    jsi_DelAssocData(interp, ptr);
    return JSI_OK;
}

static Jsi_RC freeEventTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Event *event = (Jsi_Event *)ptr;
    SIGASSERT(event,EVENT);
    if (!ptr) return JSI_OK;
    Jsi_HashValueSet(event->hPtr, NULL);
    event->hPtr = NULL;
    Jsi_EventFree(interp, event);
    return JSI_OK;
}
Jsi_RC jsi_HashFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Free(ptr);
    return JSI_OK;
}


static Jsi_RC packageHashFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    jsi_PkgInfo *p = (jsi_PkgInfo*)ptr;
    if (p->popts.info) Jsi_DecrRefCount(interp, p->popts.info);
    Jsi_Free(p);
    return JSI_OK;
}

static Jsi_RC regExpFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_RegExpFree((Jsi_Regex*)ptr);
    return JSI_OK;
}

static Jsi_RC freeCmdSpecTbl(Jsi_Interp *interp, Jsi_MapEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    jsi_CmdSpecDelete(interp, ptr);
    return JSI_OK;
}

static Jsi_RC freeGenObjTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Obj *obj = (Jsi_Obj *)ptr;
    SIGASSERT(obj,OBJ);
    if (!obj) return JSI_OK;
    Jsi_ObjDecrRefCount(interp, obj);
    return JSI_OK;
}


static Jsi_RC freeFuncsTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Func *func = (Jsi_Func *)ptr;
    if (!func) return JSI_OK;
    SIGASSERT(func,FUNC);
    func->hPtr = NULL;
    jsi_FuncFree(interp, func);
    return JSI_OK;
}

static Jsi_RC freeFuncObjTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Obj *v = (Jsi_Obj *)ptr;
    if (!v) return JSI_OK;
    SIGASSERT(v,OBJ);
    if (v->ot != JSI_OT_FUNCTION)
        fprintf(stderr, "invalid func obj\n");
    else if (v->d.fobj) {
        if (v->d.fobj->scope) {
            jsi_ScopeChain *scope = v->d.fobj->scope;
            v->d.fobj->scope = NULL;
            jsi_ScopeChainFree(interp, scope);
        }
    }
    Jsi_ObjDecrRefCount(interp, v);
    return JSI_OK;
}

static Jsi_RC freeBindObjTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Obj *v = (Jsi_Obj *)ptr;
    if (!v) return JSI_OK;
    SIGASSERT(v,OBJ);
    if (v->ot != JSI_OT_FUNCTION)
        fprintf(stderr, "invalid func obj\n");
    else if (v->d.fobj && v->d.fobj->scope) {
        v->d.fobj->scope = NULL;
    }
    Jsi_ObjDecrRefCount(interp, v);
    return JSI_OK;
}

/* TODO: incr ref before add then just decr till done. */
static Jsi_RC freeValueTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    Jsi_Value *val = (Jsi_Value *)ptr;
    if (!val) return JSI_OK;
    SIGASSERT(val,VALUE);
    //printf("GEN: %p\n", val);
   /* if (val->refCnt>1)
        Jsi_DecrRefCount(interp, val);*/
    Jsi_DecrRefCount(interp, val);
    return JSI_OK;
}

static Jsi_RC freeUserdataTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (ptr)
        jsi_UserObjDelete(interp, ptr);
    return JSI_OK;
}

void Jsi_ShiftArgs(Jsi_Interp *interp, Jsi_Value *v) {
    if (!v)
        v = interp->args;
    if (v==NULL || v->vt != JSI_VT_OBJECT || v->d.obj->arr == NULL || v->d.obj->arrCnt <= 0)
        return;
    Jsi_Obj *obj = v->d.obj;
    int n = v->d.obj->arrCnt;
    n--;
    v = obj->arr[0];
    if (v)
        Jsi_DecrRefCount(interp, v);
    if (n>0)
        memmove(obj->arr, obj->arr+1, n*sizeof(Jsi_Value*));
    obj->arr[n] = NULL;
    Jsi_ObjSetLength(interp, obj, n);
}

Jsi_Value *Jsi_Executable(Jsi_Interp *interp)
{
    return jsiIntData.execValue;
}

static Jsi_RC KeyLocker(Jsi_Hash* tbl, int lock)
{
    if (!lock)
        Jsi_MutexUnlock(jsiIntData.mainInterp, jsiIntData.mainInterp->Mutex);
    else
        return Jsi_MutexLock(jsiIntData.mainInterp, jsiIntData.mainInterp->Mutex);
    return JSI_OK;
}

#ifdef JSI_USE_MANY_STRKEY
static Jsi_RC KeyLockerTree(Jsi_Tree* tree, int lock) { return KeyLocker((Jsi_Hash*)tree, lock); }
#endif

static int jsi_deleted = 0, jsi_exitCode = 0; // TODO: move to jsiIntData
static Jsi_Value *jsi_vf = NULL;

static Jsi_RC jsi_InterpDelete(Jsi_Interp *interp, void *ptr) {
    if (jsi_vf)
        Jsi_DecrRefCount(interp, jsi_vf);
    jsi_vf = NULL;
    jsi_exitCode = interp->exitCode;
    jsi_deleted = 1;
    return JSI_OK;
}

Jsi_Interp* Jsi_Main(Jsi_InterpOpts *opts)
{
    Jsi_RC rc = JSI_OK;
    Jsi_Interp* interp = NULL;
    int argc = 0, first = 1;
    char **argv = NULL;
    if (opts) {
        interp = opts->interp;
        argc = opts->argc;
        argv = opts->argv;
    }
    if (!interp)
        interp = Jsi_InterpNew(opts);
    if (!interp)
        return NULL;
    Jsi_InterpOnDelete(interp, &jsi_InterpDelete, (void*)&jsi_InterpDelete);
    argc -= interp->iskips;
    argv += interp->iskips;

#ifndef NO_JAZ
    /* Mount zip at end of executable */
    Jsi_Value *v = Jsi_Executable(interp);
    const char *exeFile = (v?Jsi_ValueString(interp, v, NULL):NULL);
    int jsFound = 0;
    if (v && (argc != 2 || Jsi_Strcmp(argv[1], "--nozvfs"))) {
        rc = Jsi_EvalZip(interp, exeFile, JSI_ZVFS_DIR, &jsFound);
        if (rc == JSI_OK) {
            interp->selfZvfs = 1;
            if (!jsFound) {
#if (JSI__FILESYS && JSI__ZVFS)
                fprintf(stderr, "warning: no main.jsi or autoload.jsi\n");
#endif
            }
            if (jsi_deleted)
                return jsi_DoExit(interp, jsi_exitCode);
            else if (rc != 0) {
                fprintf(stderr, "Error\n");
                return jsi_DoExit(interp, 1);
            }
        }
    }
#endif
    const char *ext = NULL, *ai1, *iext = (argc<=1?NULL:Jsi_Strrchr(argv[1], '.'));
    if (interp->selfZvfs && iext && Jsi_Strcmp(iext,".fossil")==0) {
        rc = Jsi_EvalString(interp, "runModule('Archive');", JSI_EVAL_ISMAIN);
        goto done;
    }
    Jsi_ShiftArgs(interp, NULL);
    if (argc <= 1) {
        if (interp->opts.no_interactive && !interp->interactive)
            return interp;
        rc = Jsi_Interactive(interp, JSI_OUTPUT_QUOTE|JSI_OUTPUT_NEWLINES);
        goto done;
    }
    ai1 = argv[1];
    if ((!Jsi_Strcmp(ai1, "-help") || !Jsi_Strcmp(ai1, "-h")) && argc<=3) {
        if (argc>2) {
            if (Jsi_PkgRequire(interp, "Help", 0)>=0) {
                char tbuf[BUFSIZ];
                snprintf(tbuf, sizeof(tbuf), "return runModule('Help', '%s'.trim().split(null));", argv[2]);
                Jsi_RC rc = Jsi_EvalString(interp, tbuf, 0);
                const char *hstr = Jsi_ValueToString(interp, interp->retValue, NULL);
                if (rc == JSI_OK)
                    puts(hstr);
                return jsi_DoExit(interp, 1);
            }
        }
        dohelp:
        puts("USAGE:\n  jsish [PREFIX-OPTS] [COMMAND-OPTS|FILE] ...\n"
          "\nPREFIX-OPTS:\n"
          "  --C FILE\tOption file of config options.\n"
          "  --F\t\tTrace all function calls and returns.\n"
          "  --I OPT:VAL\tInterp option: equivalent to Interp.conf({OPT:VAL}).\n"
          "  --L PATH\tSet safeMode to \"lockdown\" using PATH for safe(Read/Write)Dirs.\n"
          "  --T OPT\tTypecheck option: equivalent to \"use OPT\".\n"
          "  --U\t\tDisplay unittest output, minus pass/fail compare.\n"
          "  --V\t\tSame as --U, but adds file and line number to output.\n"
          "\nCOMMAND-OPTS:\n"
          "  -a\t\tArchive: mount an archive (zip, sqlar or fossil repo) and run module.\n"
          "  -c\t\tCData: generate .c or JSON output from a .jsc description.\n"
          "  -d\t\tDebug: console script debugger.\n"
          "  -e CODE ...\tEvaluate javascript CODE.\n"
          "  -g\t\tGendeep: generate html output from markdeep source.\n"
          "  -h ?CMD?\tHelp: show help for jsish or its commands.\n"
          "  -m\t\tModule: utility create/manage/invoke a Module.\n"
          "  -s\t\tSafe: runs script in safe sub-interp.\n"
          "  -u\t\tUnitTest: test script file(s) or directories .js/.jsi files.\n"
          "  -w\t\tWget: web client to download file from url.\n"
          "  -v\t\tVersion: show version detail: add an arg to show only X.Y.Z\n"
          "  -z\t\tZip: append/manage zip files at end of executable.\n"
          "  -D\t\tDebugUI: web-gui script debugger.\n"
          "  -J\t\tJSpp: preprocess javascript for web.\n"
          "  -S\t\tSqliteUI: web-gui for sqlite database file.\n"
          "  -W\t\tWebsrv: web server to serve out content.\n"
          "\nInterp options may also be set via the confFile.'\n"
           );
        return jsi_DoExit(interp, 1);
    }
    if (!Jsi_Strcmp(ai1, "-version"))
        ai1 = "-v";
    if (ai1[0] == '-') {
        switch (ai1[1]) {
            case 'a':
                rc = Jsi_EvalString(interp, "runModule('Archive');", JSI_EVAL_ISMAIN);
                break;
            case 'c':
                rc = Jsi_EvalString(interp, "runModule('Cdata');", JSI_EVAL_ISMAIN);
                break;
            case 'd':
                interp->debugOpts.isDebugger = 1;
                rc = Jsi_EvalString(interp, "runModule('Debug');", JSI_EVAL_ISMAIN);
                break;
            case 'D':
                interp->debugOpts.isDebugger = 1;
                rc = Jsi_EvalString(interp, "runModule('DebugUI');", JSI_EVAL_ISMAIN);
                break;
            case 'e':
                if (argc < 3)
                    rc = Jsi_LogError("missing argument");
                else {
                    rc = Jsi_EvalString(interp, argv[2], JSI_EVAL_ISMAIN|JSI_EVAL_NOSKIPBANG);
                    if (rc == JSI_OK && argc>3) {
                        first += 2;
                        Jsi_ShiftArgs(interp, NULL);
                        Jsi_ShiftArgs(interp, NULL);
                        goto dofile;
                    }
                }
                break;
            case 'g':
                rc = Jsi_EvalString(interp, "runModule('GenDeep');", JSI_EVAL_ISMAIN);
                break;
            case 'h':
                goto dohelp;
            case 'J':
                rc = Jsi_EvalString(interp, "runModule('Jspp');", JSI_EVAL_ISMAIN);
                break;
            case 'm':
                if (argc <= 2 || argv[2][0] == '-')
                    rc = Jsi_EvalString(interp, "runModule('Module');", JSI_EVAL_ISMAIN);
                else {
                    Jsi_DString dStr = {}, eStr = {};
                    const char *cps, *cpe;
                    cps = Jsi_Strrchr(argv[2], '/');
                    if (cps) cps++; else cps = argv[2];
                    cpe = Jsi_Strrchr(cps, '.');
                    int len = (cpe?cpe-cps:(int)Jsi_Strlen(cps));
                    if (cpe)
                        Jsi_DSPrintf(&dStr, "source(\"%s\");", argv[2]);
                    else
                        Jsi_DSPrintf(&dStr, "require(\"%s\");", argv[2]);
                    Jsi_DSPrintf(&dStr, "puts(runModule(\"%.*s\",console.args.slice(1)));", len, cps);
                    rc = Jsi_EvalString(interp, Jsi_DSValue(&dStr), JSI_EVAL_NOSKIPBANG);
                    Jsi_DSFree(&dStr);
                    Jsi_DSFree(&eStr);
                }
                break;
            case 's':
                rc = Jsi_EvalString(interp, "runModule('Safe');", JSI_EVAL_ISMAIN);
                break;
            case 'S':
                rc = Jsi_EvalString(interp, "runModule('SqliteUI');", JSI_EVAL_ISMAIN);
                break;
            case 'u':
                rc = Jsi_EvalString(interp, "exit(runModule('UnitTest'));", JSI_EVAL_ISMAIN);
                break;
            case 'v': {
                char str[200] = "\n";
                    
                Jsi_Value* fval = Jsi_ValueNewStringKey(interp, "/zvfs/lib/sourceid.txt");
                if (!Jsi_Access(interp, fval, R_OK)) {
                    Jsi_Channel chan = Jsi_Open(interp, fval, "r");
                    if (chan)
                        Jsi_Read(interp, chan, str, sizeof(str));
                }
                if (argc>2)
                    printf("%u.%u.%u\n", JSI_VERSION_MAJOR, JSI_VERSION_MINOR, JSI_VERSION_RELEASE);
                else 
                    printf("%u.%u.%u %." JSI_VERFMT_LEN JSI_NUMGFMT " %s", JSI_VERSION_MAJOR, JSI_VERSION_MINOR, JSI_VERSION_RELEASE, Jsi_Version(), str);
                return jsi_DoExit(interp, 1);
            }
            case 'w':
                rc = Jsi_EvalString(interp, "runModule('Wget');", JSI_EVAL_ISMAIN);
                break;
            case 'W':
                rc = Jsi_EvalString(interp, "runModule('Websrv');", JSI_EVAL_ISMAIN);
                break;
            case 'z':
                rc = Jsi_EvalString(interp, "runModule('Zip');", JSI_EVAL_ISMAIN);
                break;
            default:
                puts("usage: jsish [  --C FILE | --I OPT:VAL | --L PATH | --T OPT | --U | --V | --F ] | -e STRING |\n\t"
                "| -a | -c | -d | -D | -h | -m | -s | -S | -u | -v | -w | -W | -z | FILE ...\nUse -help for long help.");
                return jsi_DoExit(interp, 1);
        }
    } else {
dofile:
        ext = Jsi_Strrchr(argv[first], '.');

        /* Support running "main.jsi" from a zip file. */
        if (ext && (Jsi_Strcmp(ext,".zip")==0 ||Jsi_Strcmp(ext,".jsz")==0 ) ) {
            rc = Jsi_EvalZip(interp, argv[first], NULL, &jsFound);
            if (rc<0) {
                fprintf(stderr, "zip mount failed\n");
                return jsi_DoExit(interp, 1);
            }
            if (!(jsFound&JSI_ZIP_MAIN)) {
                fprintf(stderr, "main.jsi not found\n");
                return jsi_DoExit(interp, 1);
            }
        } else if (ext && !Jsi_Strcmp(ext,".jsc")) {
            Jsi_DString dStr = {};
            Jsi_DSPrintf(&dStr, "console.args.unshift('%s'); runModule('CData');", argv[first]);
            rc = Jsi_EvalString(interp, Jsi_DSValue(&dStr), JSI_EVAL_ISMAIN|JSI_EVAL_NOSKIPBANG);
            Jsi_DSFree(&dStr);

        } else {
            if (argc>1) {
                jsi_vf = Jsi_ValueNewStringKey(interp, argv[first]);
                Jsi_IncrRefCount(interp, jsi_vf);
            }
            rc = Jsi_EvalFile(interp, jsi_vf, JSI_EVAL_ARGV0|JSI_EVAL_AUTOINDEX|JSI_EVAL_ISMAIN);
            if (jsi_vf) {
                Jsi_DecrRefCount(interp, jsi_vf);
                jsi_vf = NULL;
            }

        }
    }
    if (jsi_deleted) //TODO: rationalize jsi_deleted, jsi_exitCode, etc
        return jsi_DoExit(rc==JSI_EXIT?NULL:interp, jsi_exitCode);
    if (rc == JSI_OK) {
        /* Skip output from an ending semicolon which evaluates to undefined */
        Jsi_Value *ret = Jsi_ReturnValue(interp);
        if (!Jsi_ValueIsType(interp, ret, JSI_VT_UNDEF)) {
            Jsi_DString dStr = {};
            fputs(Jsi_ValueGetDString(interp, ret, &dStr, 0), stdout);
            Jsi_DSFree(&dStr);
            fputs("\n", stdout);
        }
    } else {
        if (!interp->parent && !interp->isHelp)
            fprintf(stderr, "ERROR: %s\n", interp->errMsgBuf);
        return jsi_DoExit(interp, 1);
    }

done:
    if (rc == JSI_EXIT) {
        if (opts)
            opts->exitCode = jsi_exitCode;
        return NULL;
    }
    if (jsi_deleted == 0 && interp->opts.auto_delete) {
        Jsi_InterpDelete(interp);
        return NULL;
    }
    return interp;
}

bool jsi_ModBlacklisted(Jsi_Interp *interp, const char *mod) {
    if (!interp->subOpts.blacklist) return false;
    const char *blstr =Jsi_Strstr(interp->subOpts.blacklist, mod);
    if (!blstr) return false;
    if ((blstr==interp->subOpts.blacklist || !isalnum(blstr[-1])) && !isalnum(blstr[Jsi_Strlen(mod)]))
        return false;
    return true;
}

// Get control during script evaluation to support debugging.
static Jsi_RC jsi_InterpDebugHook(struct Jsi_Interp* interp, const char *curFile,
    int curLine, int curLevel, const char *curFunc, const char *opCode, jsi_OpCode *op, const char *emsg)
{
    // TODO: when code is run in debugger, parser.y should attribute op for case stmt to skip str compares, etc.
    int isfun=0;
    if (interp->isInCallback || curLine<=0)
        return JSI_OK;
    if (op && op->nodebug)
        return JSI_OK;
    int isbp = 0, bpId = 0, cont = interp->debugOpts.doContinue,
        stop = (interp->debugOpts.noFilter || interp->debugOpts.forceBreak);
    if (!curFunc)
        curFunc = "";

    if (interp->parent && interp->parent->sigmask) {
        interp->parent->sigmask = 0;
        opCode = "SIGINT";

    } else if (Jsi_Strcmp(opCode, "DEBUG") || !interp->parent) {

        // Avoid overhead of multiple ops on same line of code.
        int sameLine = (interp->debugOpts.lastLine == curLine && interp->debugOpts.lastLevel == curLevel
            && interp->debugOpts.lastFile == curFile);

        if (sameLine && stop==0 && (interp->debugOpts.bpLast==0
            || (interp->debugOpts.bpOpCnt+10) >= interp->opCnt)) //TODO: need better way to detect bp dups.
            goto done;

        if (!interp->debugOpts.debugCallback || !interp->parent) {
            fprintf(stderr, "FILE %s:%d (%d) %s %s\n", curFile, curLine, curLevel, curFunc, opCode);
            return JSI_OK;
        }

        // Check for breakpoints.
        if (interp->breakpointHash) {
            Jsi_HashEntry *hPtr;
            Jsi_HashSearch search;
            for (hPtr = Jsi_HashSearchFirst(interp->breakpointHash, &search);
                hPtr != NULL && stop == 0; hPtr = Jsi_HashSearchNext(&search)) {
                jsi_BreakPoint* bptr = (jsi_BreakPoint*)Jsi_HashValueGet(hPtr);
                if (bptr == NULL || bptr->enabled == 0) continue;
                if (bptr->func)
                    stop = (!Jsi_Strcmp(bptr->func, curFunc));
                else
                    stop = (bptr->line == curLine && !Jsi_Strcmp(bptr->file, curFile));
                if (stop) {
                    isbp = 1;
                    bpId = bptr->id;
                    bptr->hits++;
                    if (bptr->temp)
                        bptr->enabled = 0;
                }
            }
        }

        if (stop == 0) { // No breakpoint.
            if (cont  // Cmd is "continue"
                // Handle "next" by skipping calls into functions.
                || (interp->debugOpts.minLevel>0 && curLevel>interp->debugOpts.minLevel)
                || (isfun=(Jsi_Strcmp(opCode, "PUSHVAR")==0 && op[1].op == OP_PUSHFUN)))
            {
                if (isfun) {
                    interp->debugOpts.lastLine = curLine;
                    interp->debugOpts.lastLevel = curLevel;
                    interp->debugOpts.lastFile = curFile;
                }
done:
                return JSI_OK;
            }
        }
    }
    interp->debugOpts.bpLast = isbp;
    interp->debugOpts.bpOpCnt = interp->opCnt;
    interp->debugOpts.lastLine = curLine;
    interp->debugOpts.lastLevel = curLevel;
    interp->debugOpts.lastFile = curFile;
    interp->debugOpts.forceBreak = 0;

    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    if (emsg && Jsi_Strchr(emsg,'\"'))
        emsg = 0;
    Jsi_DSPrintf(&dStr, "[\"%s\", %d, %d, \"%s\", \"%s\", %d, \"%s\"]", curFile?curFile:"", curLine, curLevel, curFunc, opCode, bpId, emsg?emsg:"");
    interp->isInCallback = 1;
    Jsi_RC rc = JSI_ERROR;
    if (interp->debugOpts.debugCallback)
        rc = Jsi_FunctionInvokeJSON(interp->parent, interp->debugOpts.debugCallback, Jsi_DSValue(&dStr), &interp->retValue);
    interp->isInCallback = 0;
    if (interp->parent->exited == 0 && rc != JSI_OK)
        Jsi_LogError("debugger failure");
    return rc;
}

Jsi_RC jsi_ParseTypeCheckStr(Jsi_Interp *interp, const char *str) {
    uint *iptr = (uint*)&interp->typeCheck;
    const char *wcp = str, *wcn = wcp;
    while (wcn && wcp) {
        int isnot = 0;
        if (*wcp == '!') { isnot = 1; wcp++; }
        wcn = Jsi_Strchr(wcp, ',');
        int ti, wlen = (wcn?(wcn-wcp):(int)Jsi_Strlen(wcp));
#define _JSIPARSETYPES(nam, field) \
        if (wlen == (sizeof(#nam)-1) && !Jsi_Strncmp(#nam, wcp, (sizeof(#nam)-1))) { \
            interp->field = (1-isnot); \
            wcp = (wcn?wcn+1:NULL); \
            continue; \
        }
        _JSIPARSETYPES(Debug, logOpts.Debug)
        _JSIPARSETYPES(Trace, logOpts.Trace)
        _JSIPARSETYPES(Test,  logOpts.Test)
        _JSIPARSETYPES(Info, logOpts.Info)
        _JSIPARSETYPES(Warn, logOpts.Warn)
        _JSIPARSETYPES(Error,  logOpts.Error)
        _JSIPARSETYPES(full,  logOpts.full)
        _JSIPARSETYPES(before,  logOpts.before)
        _JSIPARSETYPES(time,  logOpts.time)
        _JSIPARSETYPES(date,  logOpts.date)
        _JSIPARSETYPES(asserts, asserts)
        _JSIPARSETYPES(assert, asserts)
        _JSIPARSETYPES(noproto, subOpts.noproto)

        const char **tstrs = jsi_TypeChkStrs;
        for (ti=0; tstrs[ti]; ti++) {
            wlen = Jsi_Strlen(tstrs[ti]);
            if (!Jsi_Strncmp(tstrs[ti], wcp, wlen) && (!tstrs[ti][wlen] || tstrs[ti][wlen] == ',')) break;
        }
        if (tstrs[ti]) {
            if (isnot)
                *iptr &= ~(1<<ti);
            else {
                *iptr |= (1<<ti);
                if (!Jsi_Strcmp(tstrs[ti], "all"))
                    interp->typeCheck.parse = interp->typeCheck.run = 1;
                if (!Jsi_Strcmp(tstrs[ti], "strict")) {
                    interp->typeCheck.parse = interp->typeCheck.run = interp->typeCheck.all = 1;
                    if (interp->framePtr->level<=0 || interp->isMain)
                        interp->strict = 1;
                }
            }
        } else {
            Jsi_DString wStr = {};
            int i;
            tstrs = jsi_TypeChkStrs;
            for (i=0; tstrs[i]; i++) Jsi_DSAppend(&wStr, i?", ":"", tstrs[i], NULL);
            Jsi_LogWarn("unknown typeCheck warn option(s) \"%s\" not in: Debug, Trace, Test, Info, Warn, Error, assert, %s, noproto, full, before, time, date", str, Jsi_DSValue(&wStr));
            Jsi_DSFree(&wStr);
        }
        wcp = (wcn?wcn+1:NULL);
    }
    return JSI_OK;
}

static Jsi_Interp* jsi_InterpNew(Jsi_Interp *parent, Jsi_Value *opts, Jsi_InterpOpts *iopts)
{
    Jsi_Interp* interp;
    if (parent && parent->noSubInterps) {
        interp = parent;
        Jsi_LogError("subinterps disallowed");
        return NULL;
    }
    if (opts && parent && (Jsi_ValueIsObjType(parent, opts, JSI_OT_OBJECT)==0 ||
        Jsi_TreeSize(opts->d.obj->tree)<=0))
        opts = NULL;
    interp = (Jsi_Interp *)Jsi_Calloc(1,sizeof(*interp) + sizeof(jsi_Frame));
    interp->framePtr = (jsi_Frame*)(((uchar*)interp)+sizeof(*interp));
    if (!parent)
        interp->maxInterpDepth = JSI_MAX_SUBINTERP_DEPTH;
    else {
        interp->maxInterpDepth = parent->maxInterpDepth;
        interp->interpDepth = parent->interpDepth+1;
        if (interp->interpDepth > interp->maxInterpDepth) {
            Jsi_Free(interp);
            interp = parent;
            Jsi_LogError("exceeded max subinterp depth");
            return NULL;
        }
    }
    interp->maxDepth = JSI_MAX_EVAL_DEPTH;
    interp->maxIncDepth = JSI_MAX_INCLUDE_DEPTH;
    interp->maxArrayList = MAX_ARRAY_LIST;
    interp->typeWarnMax = 50;
    interp->subOpts.dblPrec = __DBL_DECIMAL_DIG__-1;
    interp->subOpts.prompt = "$ ";
    interp->subOpts.prompt2 = "> ";

    int iocnt;
    if (iopts) {
        iopts->interp = interp;
        interp->opts = *iopts;
    }
    interp->logOpts.file = 1;
    interp->logOpts.func = 1;
    interp->logOpts.Info = 1;
    interp->logOpts.Warn = 1;
    interp->logOpts.Error = 1;
    int argc = interp->opts.argc;
    char **argv = interp->opts.argv;
    char *argv0 = (argv?argv[0]:NULL);
    interp->parent = parent;
    interp->topInterp = (parent == NULL ? interp: parent->topInterp);
    if (jsiIntData.mainInterp == NULL)
        jsiIntData.mainInterp = interp->topInterp;
    interp->mainInterp = jsiIntData.mainInterp; // The first interps handles exit.
    interp->memDebug = interp->opts.mem_debug;
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
    if (parent) {
        if (parent->pkgDirs)
            interp->pkgDirs = Jsi_ValueDupJSON(interp, parent->pkgDirs);
    } else {
#ifdef JSI_PKG_DIRS
        interp->pkgDirs = Jsi_StringSplit(interp, JSI_PKG_DIRS, ",");
        Jsi_IncrRefCount(interp, interp->pkgDirs);
#endif
    }
#ifdef JSI_USE_COMPAT
    interp->compat = JSI_USE_COMPAT;
#endif
#ifndef JSI_CONF_ARGS
#define JSI_CONF_ARGS ""
#endif
    interp->confArgs = JSI_CONF_ARGS;
    for (iocnt = 1; (iocnt+1)<argc; iocnt+=2)
    {
        const char *aio = argv[iocnt];
        if (Jsi_Strcmp(aio, "--T") == 0 || Jsi_Strcmp(aio, "--C") == 0 || Jsi_Strcmp(aio, "--L") == 0) {
            continue;
        }
        if (Jsi_Strcmp(aio, "--F") == 0 || Jsi_Strcmp(aio, "--U") == 0 || Jsi_Strcmp(aio, "--V") == 0) {
            iocnt--;
            continue;
        }
        if (!Jsi_Strcmp(aio, "--I")) {
            const char *aio2 = argv[iocnt+1];
            if (!Jsi_Strncmp("memDebug:", aio2, sizeof("memDebug")))
                interp->memDebug=strtol(aio2+sizeof("memDebug"), NULL, 0);
            else if (!Jsi_Strncmp("compat", aio2, sizeof("compat")))
                interp->subOpts.compat=strtol(aio2+sizeof("compat"), NULL, 0);
            continue;
        }
        break;
    }
    SIGINIT(interp,INTERP);
    interp->NullValue = Jsi_ValueNewNull(interp);
    Jsi_IncrRefCount(interp, interp->NullValue);
#ifdef __WIN32
    Jsi_DString cwdStr;
    Jsi_DSInit(&cwdStr);
    interp->curDir = Jsi_Strdup(Jsi_GetCwd(interp, &cwdStr));
    Jsi_DSFree(&cwdStr);
#else
    char buf[JSI_BUFSIZ];
    interp->curDir = getcwd(buf, sizeof(buf));
    interp->curDir = Jsi_Strdup(interp->curDir?interp->curDir:".");
#endif
    interp->onDeleteTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeOnDeleteTbl);
    interp->assocTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, freeAssocTbl);
    interp->cmdSpecTbl = Jsi_MapNew(interp, JSI_MAP_TREE, JSI_KEYS_STRING, freeCmdSpecTbl);
    interp->eventTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeEventTbl);
    interp->fileTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, jsi_HashFree);
    interp->funcObjTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeFuncObjTbl);
    interp->funcsTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeFuncsTbl);
    interp->bindTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeBindObjTbl);
    interp->protoTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL/*freeValueTbl*/);
    interp->regexpTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, regExpFree);
    interp->preserveTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, jsi_HashFree);
    interp->loadTbl = (parent?parent->loadTbl:Jsi_HashNew(interp, JSI_KEYS_STRING, jsi_FreeOneLoadHandle));
    interp->packageHash = Jsi_HashNew(interp, JSI_KEYS_STRING, packageHashFree);
    interp->aliasHash = Jsi_HashNew(interp, JSI_KEYS_STRING, jsi_AliasFree);

    interp->lockTimeout = -1;
#ifdef JSI_LOCK_TIMEOUT
    interp->lockTimeout JSI_LOCK_TIMEOUT;
#endif
#ifndef JSI_DO_UNLOCK
#define JSI_DO_UNLOCK 1
#endif
    interp->subOpts.mutexUnlock = JSI_DO_UNLOCK;
    Jsi_Map_Type mapType = JSI_MAP_HASH;
#ifdef JSI_USE_MANY_STRKEY
    mapType = JSI_MAP_TREE;
#endif

    if (interp == jsiIntData.mainInterp || interp->threadId != jsiIntData.mainInterp->threadId) {
        interp->strKeyTbl = Jsi_MapNew(interp,  mapType, JSI_KEYS_STRING, NULL);
        interp->subOpts.privKeys = 1;
    }
    // Handle interp options: -T value and -Ixxx value
    for (iocnt = 1; (iocnt+1)<argc && !interp->parent; iocnt+=2)
    {
        const char *aio = argv[iocnt];
        if (Jsi_Strcmp(aio, "--F") == 0) {
            interp->traceCall |= (jsi_callTraceFuncs |jsi_callTraceArgs |jsi_callTraceReturn | jsi_callTraceBefore | jsi_callTraceFullPath);
            iocnt--;
            interp->iskips++;
            continue;
        }
        if (Jsi_Strcmp(aio, "--U") == 0) {
            interp->asserts = 1;
            interp->unitTest = 1;
            iocnt--;
            interp->iskips++;
            continue;
        }
        if (Jsi_Strcmp(aio, "--V") == 0) {
            interp->asserts = 1;
            interp->unitTest = 5;
            interp->tracePuts = 1;
            iocnt--;
            interp->iskips++;
            continue;
        }
        if (Jsi_Strcmp(aio, "--C") == 0) {
            if (interp->confFile)
               Jsi_LogWarn("overriding confFile: %s", interp->confFile);
            interp->confFile = argv[iocnt+1];
            interp->iskips+=2;
            continue;
        }
        if (Jsi_Strcmp(aio, "--L") == 0) {
            struct stat sb;
            const char* path = argv[iocnt+1]; //TODO: convert to Jsi_Value first?
            if (!path || stat(path, &sb)
                || !((S_ISREG(sb.st_mode) && !access(path, W_OK)) || (S_ISDIR(sb.st_mode) && !access(path, X_OK)))) {
                Jsi_LogError("Lockdown path must exist and be a writable file or executable dir: %s", path);
                Jsi_InterpDelete(interp);
                return NULL;
            }
            interp->isSafe = true;
            interp->safeMode = jsi_safe_Lockdown;
            if (interp->safeWriteDirs) {
                Jsi_LogWarn("Overriding safeWriteDirs");
                Jsi_DecrRefCount(interp, interp->safeWriteDirs);
            }
            const char *vda[2] = {};
            char npath[PATH_MAX];
            vda[0] = Jsi_FileRealpathStr(interp, path, npath);
            interp->safeWriteDirs = Jsi_ValueNewArray(interp, vda, 1);
            Jsi_IncrRefCount(interp, interp->safeWriteDirs);
            if (!interp->safeReadDirs) {
                interp->safeReadDirs = interp->safeWriteDirs;
                Jsi_IncrRefCount(interp, interp->safeReadDirs);
            }
            interp->iskips+=2;
            continue;
        }
        if (Jsi_Strcmp(aio, "--T") == 0) {
            if (jsi_ParseTypeCheckStr(interp, argv[iocnt+1]) != JSI_OK) {
                Jsi_InterpDelete(interp);
                return NULL;
            }
            interp->iskips+=2;
            continue;
        }
        if (!Jsi_Strcmp(aio, "--I"))  {
            bool bv = 1;
            char *aio2 = argv[iocnt+1], *aioc = Jsi_Strchr(aio2, ':'),
                argNamS[50], *argNam = aio2;
            const char *argVal;
            if (!Jsi_Strcmp("traceCall", aio2))
                interp->traceCall |= (jsi_callTraceFuncs |jsi_callTraceArgs |jsi_callTraceReturn | jsi_callTraceBefore | jsi_callTraceFullPath);
            else {
                if (aioc) {
                    argNam = argNamS;
                    argVal = aioc+1;
                    snprintf(argNamS, sizeof(argNamS), "%.*s", (int)(aioc-aio2), aio2);
                }
                
                DECL_VALINIT(argV);
                Jsi_Value *argValue = &argV;
                Jsi_Number dv;
                if (!aioc || Jsi_GetBool(interp, argVal, &bv) == JSI_OK) {
                    Jsi_ValueMakeBool(interp, &argValue, bv);
                } else if (!Jsi_Strcmp("null", argVal)) {
                    Jsi_ValueMakeNull(interp, &argValue);
                } else if (Jsi_GetDouble(interp, argVal, &dv) == JSI_OK) {
                    Jsi_ValueMakeNumber(interp, &argValue, dv);
                } else {
                    Jsi_ValueMakeStringKey(interp, &argValue, argVal);
                }
                if (JSI_OK != Jsi_OptionsSet(interp, InterpOptions, interp, argNam, argValue, 0)) {
                    Jsi_InterpDelete(interp);
                    return NULL;
                }
            }
            interp->iskips+=2;
            continue;
        }
        break;
    }
    if (!interp->strKeyTbl)
        interp->strKeyTbl = jsiIntData.mainInterp->strKeyTbl;
    if (opts) {
        interp->inopts = opts = Jsi_ValueDupJSON(interp, opts);
        if (Jsi_OptionsProcess(interp, InterpOptions, interp, opts, 0) < 0) {
            Jsi_DecrRefCount(interp, opts);
            interp->inopts = NULL;
            Jsi_InterpDelete(interp);
            return NULL;
        }
    }
    if (interp == jsiIntData.mainInterp) {
        interp->subthread = 0;
    } else {
        if (opts) {
            if (interp->subOpts.privKeys && interp->strKeyTbl == jsiIntData.mainInterp->strKeyTbl) {
                //Jsi_HashDelete(interp->strKeyTbl);
                Jsi_OptionsFree(interp, InterpOptions, interp, 0); /* Reparse options to populate new key table. */
                interp->strKeyTbl = Jsi_MapNew(interp, mapType, JSI_KEYS_STRING, NULL);
                if (opts->vt != JSI_VT_NULL) Jsi_OptionsProcess(interp, InterpOptions, interp, opts, 0);
            } else if (interp->subOpts.privKeys == 0 && interp->strKeyTbl != jsiIntData.mainInterp->strKeyTbl) {
                Jsi_OptionsFree(interp, InterpOptions, interp, 0); /* Reparse options to populate new key table. */
                Jsi_MapDelete(interp->strKeyTbl);
                interp->strKeyTbl = jsiIntData.mainInterp->strKeyTbl;
                if (opts->vt != JSI_VT_NULL) Jsi_OptionsProcess(interp, InterpOptions, interp, opts, 0);
            }
        }
        if (parent && parent->isSafe) {
            interp->isSafe = 1;
            interp->safeMode = parent->safeMode;
        }
        if (interp->subthread && interp->isSafe) {
            interp->subthread = 0;
            Jsi_LogError("threading disallowed in safe mode");
            Jsi_InterpDelete(interp);
            return NULL;
        }
        if (interp->subthread)
            jsiIntData.mainInterp->threadCnt++;
        if (interp->subthread && interp->strKeyTbl == jsiIntData.mainInterp->strKeyTbl)
            jsiIntData.mainInterp->threadShrCnt++;
        if (jsiIntData.mainInterp->threadShrCnt)
#ifdef JSI_USE_MANY_STRKEY
            jsiIntData.mainInterp->strKeyTbl->v.tree->opts.lockTreeProc = KeyLockerTree;
#else
            jsiIntData.mainInterp->strKeyTbl->v.hash->opts.lockHashProc = KeyLocker;
#endif
    }
    if (parent && parent->isSafe) {
        interp->isSafe = 1;
        interp->safeMode = parent->safeMode;
        interp->maxOpCnt = parent->maxOpCnt;
        if (interp->safeWriteDirs || interp->safeReadDirs || interp->safeExecPattern) {
            Jsi_LogWarn("ignoring safe* options in safe sub-sub-interp");
            if (interp->safeWriteDirs) Jsi_DecrRefCount(interp, interp->safeWriteDirs);
            if (interp->safeReadDirs) Jsi_DecrRefCount(interp, interp->safeReadDirs);
            interp->safeWriteDirs = interp->safeReadDirs = NULL;
            interp->safeExecPattern = NULL;
        }
    }

    jsi_InterpConfFiles(interp);
    if (!interp->udata) {
        interp->udata = Jsi_ValueNewObj(interp, NULL);
        Jsi_IncrRefCount(interp, interp->udata);
    }
    if (interp->subthread && !interp->scriptStr && !interp->scriptFile) {
        Jsi_LogError("subthread interp must be specify either scriptFile or scriptStr");
        Jsi_InterpDelete(interp);
        return NULL;
    }
#ifndef JSI_MEM_DEBUG
    static int warnNoDebug = 0;
    if (interp->memDebug && warnNoDebug == 0) {
        Jsi_LogWarn("ignoring memDebug as jsi was compiled without memory debugging");
        warnNoDebug = 1;
    }
#endif
    interp->threadId = Jsi_CurrentThread();
    if (interp->parent && interp->subthread==0 && interp->threadId != interp->parent->threadId) {
        interp->threadId = interp->parent->threadId;
#ifndef JSI_MEM_DEBUG
        Jsi_LogWarn("non-threaded sub-interp created by different thread than parent");
#endif
    }
    if (interp->safeMode != jsi_safe_None)
        interp->isSafe = interp->startSafe = 1;
    if (!interp->parent) {
        if (interp->isSafe)
            interp->startSafe = 1;
        if (interp->debugOpts.msgCallback)
            Jsi_LogWarn("ignoring msgCallback");
        if (interp->debugOpts.putsCallback)
            Jsi_LogWarn("ignoring putsCallback");
        if (interp->busyCallback)
            Jsi_LogWarn("ignoring busyCallback");
        if (interp->debugOpts.traceCallback)
            Jsi_LogWarn("ignoring traceCallback");
    } else if (interp->busyCallback && interp->threadId != interp->parent->threadId) {
        Jsi_LogWarn("disabling busyCallback due to threads");
        interp->busyCallback = NULL;
    }
    if (interp == jsiIntData.mainInterp)
        interp->lexkeyTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    else
        interp->lexkeyTbl = jsiIntData.mainInterp->lexkeyTbl;
    interp->thisTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeValueTbl);
    interp->userdataTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, freeUserdataTbl);
    interp->varTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    interp->codeTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, freeCodeTbl);
    interp->genValueTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD,freeValueTbl);
    interp->genObjTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, freeGenObjTbl);
#ifdef JSI_MEM_DEBUG
    interp->codesTbl = (interp == jsiIntData.mainInterp ? Jsi_HashNew(interp, JSI_KEYS_ONEWORD, NULL) : jsiIntData.mainInterp->codesTbl);
#endif
    if (interp->typeCheck.all|interp->typeCheck.parse|interp->typeCheck.funcsig)
        interp->staticFuncsTbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    if (!jsiIntData.isInit) {
        jsiIntData.isInit = 1;
        jsi_InitValue(interp, 0);
        jsiIntData.interpsTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, 0);
    }

    /* current scope, also global */
    interp->csc = Jsi_ValueNew1(interp);
    Jsi_ValueMakeObject(interp, &interp->csc, Jsi_ObjNew(interp));
    interp->framePtr->incsc = interp->csc;

#define JSIDOINIT(nam) if (!jsi_ModBlacklisted(interp,#nam)) { if (jsi_Init##nam(interp, 0) != JSI_OK) { Jsi_LogBug("Init failure in %s", #nam); } }
#define JSIDOINIT2(nam) if (!jsi_ModBlacklisted(interp,#nam)) { if (Jsi_Init##nam(interp, 0) != JSI_OK) { Jsi_LogBug("Init failure in %s", #nam); } }

    JSIDOINIT(Proto);

    if (interp->pkgDirs) // Fix-up because above, array was not yet initialized.
        interp->pkgDirs->d.obj->__proto__ = interp->Array_prototype;

    Jsi_Value *modObj = Jsi_ValueNewObj(interp, Jsi_ObjNewType(interp, JSI_OT_OBJECT));
    Jsi_ValueInsert(interp, interp->csc, "Jsi_Auto", modObj, JSI_OM_DONTDEL);

    /* initial scope chain, nothing */
    interp->framePtr->ingsc = interp->gsc = jsi_ScopeChainNew(interp, 0);

    interp->ps = jsi_PstateNew(interp); /* Default parser. */
    if (interp->unitTest&2) {
        interp->logOpts.before = 1;
        interp->logOpts.full = 1;
        interp->tracePuts = 1;
        interp->noStderr = 1;
    }
    if (interp->args && argc) {
        Jsi_LogBug("args may not be specified both as options and parameter");
        Jsi_InterpDelete(interp);
        return NULL;
    }
    if (interp->maxDepth>JSI_MAX_EVAL_DEPTH)
        interp->maxDepth = JSI_MAX_EVAL_DEPTH;

    // Create the args array.
    if (argc >= 0 && !interp->args) {
        Jsi_Value *iargs = Jsi_ValueNew1(interp);
        iargs->f.bits.dontdel = 1;
        iargs->f.bits.readonly = 1;
        Jsi_Obj *iobj = Jsi_ObjNew(interp);
        Jsi_ValueMakeArrayObject(interp, &iargs, iobj);
        int i = 1, ii = (iocnt>1 ? iocnt : 1);
        int msiz = (argc?argc-iocnt:0);
        Jsi_ObjArraySizer(interp, iobj, msiz);
        iobj->arrMaxSize = msiz;
        iocnt--;
        iobj->arrCnt = argc-iocnt;
        for (i = 1; ii < argc; ++ii, i++) {
            iobj->arr[i-1] = Jsi_ValueNewStringKey(interp, argv[ii]);
            Jsi_IncrRefCount(interp, iobj->arr[i-1]);
            jsi_ValueDebugLabel(iobj->arr[i-1], "InterpCreate", "args");
        }
        Jsi_ObjSetLength(interp, iobj, msiz);
        interp->args = iargs;
    } else if (interp->parent && interp->args) {
        // Avoid strings from sneeking in with options from parent...
        Jsi_Value *nar = Jsi_ValueDupJSON(interp, interp->args);
        Jsi_DecrRefCount(interp, interp->args);
        interp->args = nar;
    }
    JSIDOINIT(Options);
    JSIDOINIT(Cmds);
    JSIDOINIT(Interp);
    JSIDOINIT(JSON);

    interp->retValue = Jsi_ValueNew1(interp);
    interp->Mutex = Jsi_MutexNew(interp, -1, JSI_MUTEX_RECURSIVE);
    if (1 || interp->subthread) {
        interp->QMutex = Jsi_MutexNew(interp, -1, JSI_MUTEX_RECURSIVE);
        //Jsi_DSInit(&interp->interpEvalQ);
    }
    JSIDOINIT(Lexer);
    if (interp != jsiIntData.mainInterp && !parent)
        Jsi_HashSet(jsiIntData.interpsTbl, interp, NULL);

    if (!interp->isSafe) {
        JSIDOINIT(Load);
#if JSI__SIGNAL==1
        JSIDOINIT(Signal);
#endif
    }
    if (interp->isSafe == 0 || interp->startSafe || interp->safeWriteDirs!=NULL || interp->safeReadDirs!=NULL) {
#if JSI__FILESYS==1
        JSIDOINIT(FileCmds);
        JSIDOINIT(Filesys);
#endif
    }
#if JSI__SQLITE==1
    JSIDOINIT2(Sqlite);
#else
    Jsi_initSqlite(interp, 0);
#endif
#if JSI__MYSQL==1
    if (!interp->noNetwork) {
        JSIDOINIT2(MySql);
    }
#endif
#if JSI__SOCKET==1
    JSIDOINIT2(Socket);
#endif
#if JSI__WEBSOCKET==1
    JSIDOINIT2(WebSocket);
#endif

#if JSI__CDATA==1
    JSIDOINIT(CData);
#endif

#ifdef JSI_USER_EXTENSION
    extern int JSI_USER_EXTENSION(Jsi_Interp *interp, int release);
    if (JSI_USER_EXTENSION (interp, 0) != JSI_OK) {
        fprintf(stderr, "extension load failed");
        return jsi_DoExit(interp, 1);
    }
#endif
    Jsi_PkgProvide(interp, "Jsi", JSI_VERSION, NULL);
    if (argc > 0) {
        char *ss = argv0;
        char epath[PATH_MAX] = ""; // Path of executable
#ifdef __WIN32

        if (GetModuleFileName(NULL, epath, sizeof(epath))>0)
            ss = epath;
#else
#ifndef PROC_SELF_DIR
#define PROC_SELF_DIR "/proc/self/exe"
#endif
        if (ss && *ss != '/' && readlink(PROC_SELF_DIR, epath, sizeof(epath)) && epath[0])
            ss = epath;
#endif
        Jsi_Value *src = Jsi_ValueNewStringDup(interp, ss);
        Jsi_IncrRefCount(interp, src);
        jsiIntData.execName = Jsi_Realpath(interp, src, NULL);
        Jsi_DecrRefCount(interp, src);
        if (!jsiIntData.execName) jsiIntData.execName = Jsi_Strdup("");
        jsiIntData.execValue = Jsi_ValueNewString(interp, jsiIntData.execName, -1);
        Jsi_IncrRefCount(interp, jsiIntData.execValue);
        Jsi_HashSet(interp->genValueTbl, jsiIntData.execValue, jsiIntData.execValue);
    }

    //interp->nocacheOpCodes = 1;
    if (interp->debugOpts.debugCallback && !interp->debugOpts.hook) {
        interp->debugOpts.hook = jsi_InterpDebugHook;
    }
    interp->startTime = jsi_GetTimestamp();
#ifdef JSI_INTERP_EXTENSION_CODE // For extending interp from jsi.c
    JSI_INTERP_EXTENSION_CODE
#endif
    if (interp->opts.initProc && (*interp->opts.initProc)(interp, 0) != JSI_OK)
        Jsi_LogBug("Init failure in initProc");

    return interp;
}

Jsi_Interp* Jsi_InterpNew(Jsi_InterpOpts *opts)
{
    return jsi_InterpNew(NULL, NULL, opts);
}

bool Jsi_InterpGone( Jsi_Interp* interp)
{
    return (interp == NULL || interp->deleting || interp->destroying || interp->exited);
}

static void DeleteAllInterps() { /* Delete toplevel interps. */
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch search;
    if (!jsiIntData.interpsTbl)
        return;
    for (hPtr = Jsi_HashSearchFirst(jsiIntData.interpsTbl, &search); hPtr; hPtr = Jsi_HashSearchNext(&search)) {
        Jsi_Interp *interp = (Jsi_Interp *)Jsi_HashKeyGet(hPtr);
        Jsi_HashEntryDelete(hPtr);
        interp->destroying = 1;
        Jsi_InterpDelete(interp);
    }
    Jsi_HashDelete(jsiIntData.interpsTbl);
    jsiIntData.interpsTbl = NULL;
    jsiIntData.isInit = 0;
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
        for (hPtr=Jsi_TreeSearchFirst(obj->tree, &srch,  JSI_TREE_ORDER_IN, NULL); hPtr;
            hPtr=Jsi_TreeSearchNext(&srch)) {
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
        uint i;
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
    if (jsiIntData.mainInterp != interp) return;
    int vdLev = interp->memDebug;
    int have = (interp->dbPtr->valueDebugTbl->numEntries || interp->dbPtr->objDebugTbl->numEntries);
    if ((have && vdLev>0) || vdLev>=3) {
        // First traverse all Object trees/arrays and mark all values contained therein.
        Jsi_HashSearch search;
        Jsi_HashEntry *hPtr;
        for (hPtr = Jsi_HashSearchFirst(interp->dbPtr->objDebugTbl, &search);
            hPtr != NULL; hPtr = Jsi_HashSearchNext(&search)) {
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
        for (hPtr = Jsi_HashSearchFirst(interp->dbPtr->valueDebugTbl, &search);
            hPtr != NULL; hPtr = Jsi_HashSearchNext(&search)) {
            Jsi_Value *vp = (Jsi_Value *)Jsi_HashKeyGet(hPtr);
            if (vp==NULL || vp->sig != JSI_SIG_VALUE) {
                bcnt[0]++;
                if (vdLev>1)
                    fprintf(stderr, "BAD VALUE: %p\n", vp);
            } else {
                bcnt[1]++;
                refSum += vp->refCnt;
                if (vdLev>1) {
                    char ebuf[JSI_BUFSIZ], ebuf2[JSI_MAX_NUMBER_STRING];
                    ebuf[0] = 0;
                    if (vp->vt==JSI_VT_OBJECT)
                        snprintf(ebuf, sizeof(ebuf), " {obj=%p, otype=%s}", vp->d.obj, Jsi_ObjTypeStr(interp, vp->d.obj));
                    else if (vp->vt==JSI_VT_NUMBER)
                        snprintf(ebuf, sizeof(ebuf), " {num=%s}", Jsi_NumberToString(interp, vp->d.num, ebuf2, sizeof(ebuf2)));
                    else if (vp->vt==JSI_VT_BOOL)
                        snprintf(ebuf, sizeof(ebuf), " {bool=%s}", vp->d.val?"true":"false");
                    else if (vp->vt==JSI_VT_STRING) {
                        const char *sbuf = ((vp->d.s.str && Jsi_Strlen(vp->d.s.str)>40)?"...":"");
                        snprintf(ebuf, sizeof(ebuf), " {string=\"%.40s%s\"}", (vp->d.s.str?vp->d.s.str:""), sbuf);
                    }
                    const char *pfx = "";
                    if (!(vp->VD.flags&MDB_INOBJ))
                        pfx = "!"; // Value is not contained in an object.
                    fprintf(stderr, "[%s*%p,#%d,%s,%d:%s%s%s]:%s @%s:%d in %s()%s\n", pfx,
                        vp, vp->refCnt, Jsi_ValueTypeStr(interp, vp), vp->VD.Idx,
                        (vp->VD.label?vp->VD.label:""), (vp->VD.label2?":":""),
                        (vp->VD.label2?vp->VD.label2:""), vp->VD.interp==jsiIntData.mainInterp?"":"!",
                        vp->VD.fname, vp->VD.line, vp->VD.func, ebuf);
                }
            }
        }
        if (interp->dbPtr->objDebugTbl->numEntries != interp->dbPtr->objCnt)
            fprintf(stderr, "\n\nObject table/alloc mismatch: table=%d, alloc=%d\n",
                interp->dbPtr->objDebugTbl->numEntries, interp->dbPtr->objCnt);
        if (vdLev>1 && interp->dbPtr->objDebugTbl->numEntries)
            fprintf(stderr, "\n\nUNFREED OBJECTS \"[*ptr,#refCnt,type,idx:label,label2]: @file:line in func() ...\"\n");
        for (hPtr = Jsi_HashSearchFirst(interp->dbPtr->objDebugTbl, &search);
            hPtr != NULL; hPtr = Jsi_HashSearchNext(&search)) {
            Jsi_Obj *vp = (Jsi_Obj *)Jsi_HashKeyGet(hPtr);
            if (vp==NULL || vp->sig != JSI_SIG_OBJ) {
                bcnt[2]++;
                fprintf(stderr, "BAD OBJ: %p\n", vp);
            } else {
                bcnt[3]++;
                refsum += vp->refcnt;
                if (vdLev>1) {
                    char ebuf[JSI_BUFSIZ], ebuf2[JSI_MAX_NUMBER_STRING];
                    ebuf[0] = 0;
                    if (vp->ot==JSI_OT_OBJECT) {
                        if (vp->isarrlist)
                            snprintf(ebuf, sizeof(ebuf), "tree#%d, array#%d", (vp->tree?vp->tree->numEntries:0), vp->arrCnt);
                        else
                            snprintf(ebuf, sizeof(ebuf), "tree#%d", (vp->tree?vp->tree->numEntries:0));
                    } else if (vp->ot==JSI_OT_NUMBER)
                        snprintf(ebuf, sizeof(ebuf), "num=%s", Jsi_NumberToString(interp, vp->d.num, ebuf2, sizeof(ebuf2)));
                    else if (vp->ot==JSI_OT_BOOL)
                        snprintf(ebuf, sizeof(ebuf), "bool=%s", vp->d.val?"true":"false");
                    else if (vp->ot==JSI_OT_STRING) {
                        const char *sbuf = ((vp->d.s.str && Jsi_Strlen(vp->d.s.str)>40)?"...":"");
                        snprintf(ebuf, sizeof(ebuf), "string=\"%.40s%s\"", (vp->d.s.str?vp->d.s.str:""), sbuf);
                    }
                    fprintf(stderr, "[*%p,#%d,%s,%d:%s%s%s]:%s @%s:%d in %s() {%s}\n",
                        vp, vp->refcnt, Jsi_ObjTypeStr(interp, vp), vp->VD.Idx, vp->VD.label?vp->VD.label:"",
                        vp->VD.label2?":":"",vp->VD.label2?vp->VD.label2:"", vp->VD.interp==jsiIntData.mainInterp?"":"!",
                        vp->VD.fname, vp->VD.line,
                        vp->VD.func, ebuf);
                }
            }
        }
        fprintf(stderr, "\nVALUES: bad=%d,unfreed=%d,allocs=%d,refsum=%d  | OBJECTS: bad=%d,unfreed=%d,allocs=%d,refsum=%d  interp=%p\n",
            bcnt[0], bcnt[1], interp->dbPtr->valueAllocCnt, refSum, bcnt[2], bcnt[3], interp->dbPtr->objAllocCnt, refsum, interp);

        if (interp->codesTbl)
            for (hPtr = Jsi_HashSearchFirst(interp->codesTbl, &search);
                hPtr != NULL; hPtr = Jsi_HashSearchNext(&search)) {
                Jsi_OpCodes *vp = (Jsi_OpCodes *)Jsi_HashKeyGet(hPtr);
                fprintf(stderr, "unfreed opcodes: %d\n", vp->id);
            }
    }
    Jsi_HashDelete(interp->dbPtr->valueDebugTbl);
    Jsi_HashDelete(interp->dbPtr->objDebugTbl);
    Jsi_HashDelete(interp->codesTbl);
    bool isMainInt = (interp == jsiIntData.mainInterp);
    if (isMainInt && vdLev>3)
        _exit(1); // Avoid sanitize output.
}
#endif

static Jsi_RC jsiInterpDelete(Jsi_Interp* interp, void *unused)
{
    SIGASSERT(interp,INTERP);
    bool isMainInt = (interp == jsiIntData.mainInterp);
    int mainFlag = (isMainInt ? 2 : 1);
    if (isMainInt)
        DeleteAllInterps();
    if (interp->opts.initProc)
        (*interp->opts.initProc)(interp, mainFlag);
    jsiIntData.delInterp = interp;
    if (interp->gsc) jsi_ScopeChainFree(interp, interp->gsc);
    if (interp->csc) Jsi_DecrRefCount(interp, interp->csc);
    if (interp->ps) jsi_PstateFree(interp->ps);
    int i;
    for (i=0; i<interp->maxStack; i++) {
        if (interp->Stack[i]) Jsi_DecrRefCount(interp, interp->Stack[i]);
        if (interp->Obj_this[i]) Jsi_DecrRefCount(interp, interp->Obj_this[i]);
    }
    if (interp->Stack) {
        Jsi_Free(interp->Stack);
        Jsi_Free(interp->Obj_this);
    }

    if (interp->argv0)
        Jsi_DecrRefCount(interp, interp->argv0);
    if (interp->console)
        Jsi_DecrRefCount(interp, interp->console);
    if (interp->lastSubscriptFail)
        Jsi_DecrRefCount(interp, interp->lastSubscriptFail);
    if (interp->nullFuncRet)
        Jsi_DecrRefCount(interp, interp->nullFuncRet);
    Jsi_HashDelete(interp->codeTbl);
    Jsi_MapDelete(interp->cmdSpecTbl);
    Jsi_HashDelete(interp->fileTbl);
    Jsi_HashDelete(interp->funcObjTbl);
    Jsi_HashDelete(interp->funcsTbl);
    if (interp->profileCnt) { // TODO: resolve some values from dbPtr, others not.
        double endTime = jsi_GetTimestamp();
        double coverage = (int)(100.0*interp->coverHit/interp->coverAll);
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        Jsi_DSPrintf(&dStr, "PROFILE: TOTAL: time=%.6f, func=%.6f, cmd=%.6f, #funcs=%d, #cmds=%d, cover=%2.1f%%, #values=%d, #objs=%d %s%s\n",
            endTime-interp->startTime, interp->funcSelfTime, interp->cmdSelfTime, interp->funcCallCnt, interp->cmdCallCnt,
            coverage, interp->dbPtr->valueAllocCnt,  interp->dbPtr->objAllocCnt,
            interp->parent?" ::":"", (interp->parent&&interp->name?interp->name:""));
        Jsi_Puts(interp, jsi_Stderr, Jsi_DSValue(&dStr), -1);
        Jsi_DSFree(&dStr);
    }
    if (isMainInt)
        Jsi_HashDelete(interp->lexkeyTbl);
    Jsi_HashDelete(interp->protoTbl);
    if (interp->subthread)
        jsiIntData.mainInterp->threadCnt--;
    if (interp->subthread && interp->strKeyTbl == jsiIntData.mainInterp->strKeyTbl)
        jsiIntData.mainInterp->threadShrCnt--;
    if (!jsiIntData.mainInterp->threadShrCnt)
#ifdef JSI_USE_MANY_STRKEY
        jsiIntData.mainInterp->strKeyTbl->v.tree->opts.lockTreeProc = NULL;
#else
        jsiIntData.mainInterp->strKeyTbl->v.hash->opts.lockHashProc = NULL;
#endif
    //Jsi_ValueMakeUndef(interp, &interp->ret);
    Jsi_HashDelete(interp->thisTbl);
    Jsi_HashDelete(interp->varTbl);
    Jsi_HashDelete(interp->genValueTbl);
    Jsi_HashDelete(interp->genObjTbl);
    Jsi_HashDelete(interp->aliasHash);
    if (interp->staticFuncsTbl)
        Jsi_HashDelete(interp->staticFuncsTbl);
    if (interp->breakpointHash)
        Jsi_HashDelete(interp->breakpointHash);
    if (interp->preserveTbl->numEntries!=0)
        Jsi_LogBug("Preserves unbalanced");
    Jsi_HashDelete(interp->preserveTbl);
    if (interp->curDir)
        Jsi_Free(interp->curDir);
    if (isMainInt) {
        jsi_InitFilesys(interp, mainFlag);
    }
#ifndef JSI_OMIT_VFS
    jsi_InitVfs(interp, 1);
#endif
#ifndef JSI_OMIT_CDATA
        jsi_InitCData(interp, mainFlag);
#endif
#if JSI__MYSQL==1
        Jsi_InitMySql(interp, mainFlag);
#endif
    while (interp->interpStrEvents) {
        InterpStrEvent *se = interp->interpStrEvents;
        interp->interpStrEvents = se->next;
        if (se->acfunc)
            Jsi_DecrRefCount(interp, se->acfunc);
        if (se->acdata)
            Jsi_Free(se->acdata);
        Jsi_Free(se);
    }

    if (interp->Mutex)
        Jsi_MutexDelete(interp, interp->Mutex);
    if (interp->QMutex) {
        Jsi_MutexDelete(interp, interp->QMutex);
        Jsi_DSFree(&interp->interpEvalQ);
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
    Jsi_HashDelete(interp->regexpTbl);
    Jsi_OptionsFree(interp, InterpOptions, interp, 0);
    Jsi_HashDelete(interp->userdataTbl);
    Jsi_HashDelete(interp->eventTbl);
    if (interp->inopts)
        Jsi_DecrRefCount(interp, interp->inopts);
    if (interp->safeWriteDirs)
        Jsi_DecrRefCount(interp, interp->safeWriteDirs);
    if (interp->safeReadDirs)
        Jsi_DecrRefCount(interp, interp->safeReadDirs);
    if (interp->pkgDirs)
        Jsi_DecrRefCount(interp, interp->pkgDirs);
    for (i=0; interp->cleanObjs[i]; i++) {
        interp->cleanObjs[i]->tree->opts.freeHashProc = 0;
        Jsi_ObjFree(interp, interp->cleanObjs[i]);
    }
    Jsi_HashDelete(interp->bindTbl);
    for (i = 0; i <= interp->cur_scope; i++)
        jsi_ScopeStrsFree(interp, interp->scopes[i]);
#if JSI__ZVFS==1
    Jsi_InitZvfs(interp, mainFlag);
#endif
    if (!interp->parent)
        Jsi_HashDelete(interp->loadTbl);
    if (interp->packageHash)
        Jsi_HashDelete(interp->packageHash);
    Jsi_HashDelete(interp->assocTbl);
    interp->cleanup = 1;
    jsi_AllObjOp(interp, NULL, -1);
#ifdef JSI_MEM_DEBUG
    jsi_DebugDumpValues(interp);
#endif
    if (isMainInt || interp->strKeyTbl != jsiIntData.mainInterp->strKeyTbl)
        Jsi_MapDelete(interp->strKeyTbl);

    if (isMainInt)
        jsiIntData.mainInterp = NULL;
    _JSI_MEMCLEAR(interp);
    jsiIntData.delInterp = NULL;
    Jsi_Free(interp);
    return JSI_OK;
}

void Jsi_InterpDelete(Jsi_Interp* interp)
{
    if (interp->deleting || interp->level > 0 || !interp->onDeleteTbl)
        return;
    interp->deleting = 1;
    Jsi_HashDelete(interp->onDeleteTbl);
    interp->onDeleteTbl = NULL;
    Jsi_EventuallyFree(interp, interp, jsiInterpDelete);
}

typedef struct {
    void *data;
    Jsi_Interp *interp;
    int refCnt;
    Jsi_DeleteProc* proc;
} PreserveData;

void Jsi_Preserve(Jsi_Interp* interp, void *data) {
    bool isNew;
    PreserveData *ptr;
    Jsi_HashEntry *hPtr = Jsi_HashEntryNew(interp->preserveTbl, data, &isNew);
    assert(hPtr);
    if (!isNew) {
        ptr = (PreserveData*)Jsi_HashValueGet(hPtr);
        if (ptr) {
            assert(interp == ptr->interp);
            ptr->refCnt++;
        }
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
    if (!ptr) return;
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
    JSI_NOWARN(ptr);
    Jsi_HashEntryDelete(hPtr);
}

void Jsi_InterpOnDelete(Jsi_Interp *interp, Jsi_DeleteProc *freeProc, void *ptr)
{
    if (freeProc)
        Jsi_HashSet(interp->onDeleteTbl, ptr, (void*)freeProc);
    else {
        Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->onDeleteTbl, ptr);
        if (hPtr)
            Jsi_HashEntryDelete(hPtr);
    }
}

static void interpObjErase(InterpObj *fo)
{
    SIGASSERTV(fo,INTERPOBJ);
    if (fo->subinterp) {
        Jsi_Interp *interp = fo->subinterp;
        fo->subinterp = NULL;
        Jsi_InterpDelete(interp);
        /*fclose(fo->fp);
        Jsi_Free(fo->interpname);
        Jsi_Free(fo->mode);*/
    }
    fo->subinterp = NULL;
}

static Jsi_RC interpObjFree(Jsi_Interp *interp, void *data)
{
    InterpObj *fo = (InterpObj *)data;
    SIGASSERT(fo,INTERPOBJ);
    if (fo->deleting) return JSI_OK;
    fo->deleting = 1;
    interpObjErase(fo);
    Jsi_Free(fo);
    return JSI_OK;
}

static bool interpObjIsTrue(void *data)
{
    InterpObj *fo = (InterpObj *)data;
    SIGASSERT(fo,INTERPOBJ);
    if (!fo->subinterp) return 0;
    else return 1;
}

static bool interpObjEqual(void *data1, void *data2)
{
    return (data1 == data2);
}

/* TODO: possibly support async func-callback.  Also for call/send. */
static Jsi_RC InterpEvalCmd_(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int flags)
{
    int isFile = flags&2;
    int isUplevel = flags&1;
    int lev = 0;
    bool async = 0;
    Jsi_RC rc = JSI_OK;
    int isthrd;
    Jsi_Interp *sinterp = interp;
    Jsi_ValueMakeUndef(interp, ret);
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (udf)
        sinterp = udf->subinterp;
    if (Jsi_InterpGone(interp) || Jsi_InterpGone(sinterp))
        return Jsi_LogError("Sub-interp gone");
    isthrd = (interp->threadId != sinterp->threadId);
    jsi_Frame *f = sinterp->framePtr;
    Jsi_Value *nw = Jsi_ValueArrayIndex(interp, args, 1);
    if (!isUplevel) {
        if (nw && Jsi_GetBoolFromValue(interp, nw, &async))
            return JSI_ERROR;
    } else {
        if (isthrd)
            return Jsi_LogError("can not use uplevel() with threaded interp");
        Jsi_Number nlev = sinterp->framePtr->level;
        if (nw && Jsi_GetNumberFromValue(interp, nw, &nlev)!=JSI_OK)
            return Jsi_LogError("expected number");
        lev = (int)nlev;
        if (lev <= 0)
            lev = f->level+lev;
        if (lev <= 0 || lev > f->level)
            return Jsi_LogError("level %d not between 1 and %d", (int)nlev, f->level);
    }

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
    if (interp->subOpts.mutexUnlock) Jsi_MutexUnlock(interp, interp->Mutex);
    if (!isthrd) {
        int ostrict = sinterp->strict;
        sinterp->strict = 0;
        sinterp->level++;
        if (interp->framePtr->tryDepth)
            sinterp->framePtr->tryDepth++;
        if (isFile) {
            int sflags = 0;
            if (!sinterp->includeCnt) {
                sflags = JSI_EVAL_ARGV0|JSI_EVAL_AUTOINDEX;
                sinterp->isMain = 1;
            }
            if (sinterp->debugOpts.debugCallback && !sinterp->includeCnt)  // TODO: safe debugging can't use "source"
                // TODO: we do this in debugger, even though it is illegal for interps to share objects.
                sinterp->autoFiles = Jsi_ValueDup(sinterp, interp->autoFiles);
            sinterp->includeCnt++;
            rc = Jsi_EvalFile(sinterp, Jsi_ValueArrayIndex(interp, args, 0), sflags);
        } else if (isUplevel == 0 || lev <= 1)
            rc = (Jsi_EvalString(sinterp, cp, 0) == 0 ? JSI_OK : JSI_ERROR);
        else {
            rc = (jsi_evalStrFile(sinterp, NULL, cp, 0, lev) == 0 ? JSI_OK : JSI_ERROR);
        }
        sinterp->strict = ostrict;
        if (interp->framePtr->tryDepth) {
            sinterp->framePtr->tryDepth--;
            if (rc != JSI_OK && interp != sinterp) {
                Jsi_Strcpy(interp->errMsgBuf, sinterp->errMsgBuf);
                interp->errLine = sinterp->errLine;
                interp->errFile = sinterp->errFile;
                sinterp->errMsgBuf[0] = 0;
            }
        }
        sinterp->level--;
    } else {
        if (Jsi_MutexLock(interp, sinterp->QMutex) != JSI_OK)
            return JSI_ERROR;
        InterpStrEvent *se, *s = (InterpStrEvent *)Jsi_Calloc(1, sizeof(*s));
        SIGINIT(s,INTERPSTREVENT);
        s->isExec = 1;
        s->tryDepth = interp->framePtr->tryDepth;
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
        rc = (s->rc == 0 ? JSI_OK : JSI_ERROR);
        if (rc != JSI_OK)
            Jsi_LogError("eval failed: %s", Jsi_DSValue(&s->data));
        Jsi_DSFree(&s->func);
        Jsi_DSFree(&s->data);
        Jsi_Free(s);
    }

    if (interp->subOpts.mutexUnlock && Jsi_MutexLock(interp, interp->Mutex) != JSI_OK) {
        return JSI_ERROR;
    }

    if (Jsi_InterpGone(sinterp))
    {
        /* TODO: perhaps exit() be able to delete. */
        //Jsi_InterpDelete(sinterp);
        return JSI_OK;
    }
    /*if (rc != JSI_OK && !async)
        return rc;*/
    if (sinterp->retValue->vt != JSI_VT_UNDEF) {
        if (sinterp == interp)
            Jsi_ValueCopy(interp, *ret, sinterp->retValue);
        else
            Jsi_CleanValue(sinterp, interp, sinterp->retValue, ret);
    }
    return rc;
}

Jsi_Interp *jsi_DoExit(Jsi_Interp *interp, int rc)
{
    if (rc<0 || rc>127) rc = 127;
    if (!interp || !interp->opts.no_exit) {
        if (rc) {
            Jsi_Flush(interp, jsi_Stdout);
            Jsi_Flush(interp, jsi_Stderr);
        }
        exit(rc);
    }
    fprintf(stderr, "ignoring attempted exit: may cause a crash\n");
    if (interp) interp->deleting = 1;
    return NULL;
}

#define FN_interpeval JSI_INFO("\
When the 'async' option is used on a threaded interp, the script is queued as an Event.")

static Jsi_RC InterpEvalCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return InterpEvalCmd_(interp, args, _this, ret, funcPtr, 0);
}


#define FN_interpuplevel JSI_INFO("\
The level argument is as returned by Info.level().  Not supported with threads.")
static Jsi_RC InterpUplevelCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return InterpEvalCmd_(interp, args, _this, ret, funcPtr, 1);
}

static Jsi_RC InterpSourceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return InterpEvalCmd_(interp, args, _this, ret, funcPtr, 2);
}

static Jsi_RC InterpValueCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    Jsi_Interp *sinterp = interp;
    if (udf) {
        if (!udf->subinterp)
        return Jsi_LogError("Sub-interp gone");
        sinterp = udf->subinterp;
        if (interp->threadId != udf->subinterp->threadId)
            return Jsi_LogError("value not supported with threads");
    }
    Jsi_Value *nw = Jsi_ValueArrayIndex(interp, args, 1);
    jsi_Frame *f = sinterp->framePtr;
    Jsi_Number nlev = sinterp->framePtr->level;
    if (nw && Jsi_GetNumberFromValue(interp, nw, &nlev))
        return JSI_ERROR;
    int lev = (int)nlev;
    if (lev <= 0)
        lev = f->level+lev;
    if (lev <= 0 || lev > f->level)
        return Jsi_LogError("level %d not between 1 and %d", (int)nlev, f->level);
    while (f->level != lev  && f->parent)
        f = f->parent;

    const char* arg = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    Jsi_Value *val = NULL;
    if (arg) {
        if (f == sinterp->framePtr)
            val = Jsi_NameLookup(sinterp, arg);
        else {
            jsi_Frame *of = sinterp->framePtr;
            sinterp->framePtr = f;
            val = Jsi_NameLookup(sinterp, arg);
            sinterp->framePtr = of;
        }
    }
    if (!val)
        return Jsi_LogError("unknown var: %s", arg);
    if (sinterp == interp) {
        Jsi_ValueCopy(interp, *ret, val);
        return JSI_OK;
    }
    Jsi_CleanValue(sinterp, interp, val, ret);
    return JSI_OK;
}

static Jsi_RC InterpInfoCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    Jsi_Interp *subinterp = interp;
    if (udf) {
        if (!udf->subinterp)
            return Jsi_LogError("Sub-interp gone");
        subinterp = udf->subinterp;
    }
    return jsi_InterpInfo(subinterp, args, _this, ret, funcPtr);
}

Jsi_RC jsi_InterpInfo(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
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
        } else
            return Jsi_LogError("unknown interp");
    }
    if (sinterp->subthread)
        snprintf(tbuf, sizeof(tbuf), ", thread:{errorCnt:%u, evalCnt:%u, msgCnt:%u }",
            sinterp->threadErrCnt, sinterp->threadEvalCnt, sinterp->threadMsgCnt );
    const char *funcstr = sinterp->framePtr->funcName;
    if (!funcstr)
        funcstr = "";
    int curLine = (sinterp->curIp?sinterp->curIp->Line:0);
    Jsi_DSPrintf(&dStr, "{curLevel:%d, curLine:%d, curFile:\"%s\", curFunc:\"%s\", hasExited:%d, "
        "opCnt:%d, isSafe:%s, depth:%d, codeCacheHits: %d, typeMismatchCnt:%d, "
        "funcCallCnt:%d, cmdCallCnt:%d, includeCnt:%d, includeDepth:%d, pkgReqDepth:%d, "
        "cwd:\"%s\", lockTimeout: %d, name, \"%s\" %s%s};",
        sinterp->level, curLine, jsi_GetCurFile(sinterp), funcstr?funcstr:"",
        sinterp->exited, sinterp->opCnt, sinterp->isSafe?"true":"false",
        sinterp->interpDepth, sinterp->codeCacheHit, sinterp->typeMismatchCnt,
        sinterp->funcCallCnt, sinterp->cmdCallCnt, sinterp->includeCnt, sinterp->includeDepth, sinterp->pkgReqDepth,
        (sinterp->curDir?sinterp->curDir:Jsi_GetCwd(sinterp,&cStr)),
        sinterp->lockTimeout, sinterp->name?sinterp->name:"", tbuf[0]?",":"", tbuf);
    Jsi_RC rc = Jsi_JSONParse(interp, Jsi_DSValue(&dStr), ret, 0);
    if (rc != JSI_OK)
        puts(Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    Jsi_DSFree(&cStr);
    return rc;
}

static Jsi_RC SubInterpEvalCallback(Jsi_Interp *interp, void* data)
{
    Jsi_RC rc = JSI_OK;
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
        if (se->acdata) {
            jsi_AliasCreateCmd(interp, Jsi_DSValue(&se->func), se->acdata);
        }
        else if (se->acfunc) {
            if (JSI_OK != Jsi_FunctionInvokeJSON(interp, se->acfunc, Jsi_DSValue(&se->data), NULL))
                rc = JSI_ERROR;
            Jsi_DecrRefCount(interp, se->acfunc);
        }
        else if (isExec) {
            if (se->tryDepth)
                interp->framePtr->tryDepth++;
            se->rc = Jsi_EvalString(interp, Jsi_DSValue(&se->data), 0);
            Jsi_DSSetLength(&se->data, 0);
            if (se->rc != JSI_OK && se->tryDepth) {
                Jsi_DSAppend(&se->data, interp->errMsgBuf, NULL);
                se->errLine = interp->errLine;
                se->errFile = interp->errFile;
            } else {
                Jsi_ValueGetDString(interp, interp->retValue, &se->data, JSI_OUTPUT_JSON);
            }
            if (se->tryDepth)
                interp->framePtr->tryDepth--;

        /* Otherwise, async calls. */
        /*} else if (se->objData) {
            if (JSI_OK != Jsi_CommandInvoke(interp, Jsi_DSValue(&se->func), se->objData, NULL))
                rc = JSI_ERROR;*/
        } else {
            const char *scp = Jsi_DSValue(&se->data);
            if (JSI_OK != Jsi_CommandInvokeJSON(interp, Jsi_DSValue(&se->func), scp, NULL))
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


static Jsi_RC ThreadEvalCallback(Jsi_Interp *interp, void* data) {
    Jsi_RC rc;

    if ((rc=SubInterpEvalCallback(interp, data)) != JSI_OK)
        interp->threadErrCnt++;
    return rc;
}

/* Create an event handler in interp to handle call/eval/send asyncronously via 'Sys.update()'. */
void jsi_AddEventHandler(Jsi_Interp *interp)
{
    Jsi_Event *ev;
    while (!interp->EventHdlId) { /* Find an empty event slot. */
        bool isNew;
        uintptr_t id = ++interp->eventIdx;
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
        interp->EventHdlId = id;
    }
}

#define FN_call JSI_INFO("\
Invoke function in sub-interp with arguments.\n\
Since interps are not allowed to share objects, \
data is automatically cleansed by encoding/decoding to/from JSON if required.\n\
Unless an 'async' parameter is true call is acyncronous.\n\
Otherwise waits until the sub-interp is idle, \
to make call and return result.")

static Jsi_RC InterpCallCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    InterpObj *udf = (InterpObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    int isthrd;
    Jsi_Interp *sinterp;
    if (udf)
        sinterp = udf->subinterp;
    else
        return Jsi_LogError("Apply Interp.call in a non-subinterp");
    if (Jsi_InterpGone(sinterp))
        return Jsi_LogError("Sub-interp gone");
    isthrd = (interp->threadId != sinterp->threadId);

    Jsi_Value *func = NULL;
    char *fname = NULL;
    func = Jsi_ValueArrayIndex(interp, args, 0);
    fname = Jsi_ValueString(interp, func, NULL);
    if (!fname)
        return Jsi_LogError("function name must be a string");
    if (Jsi_MutexLock(interp, sinterp->Mutex) != JSI_OK)
        return JSI_ERROR;
    Jsi_Value *namLU = Jsi_NameLookup(sinterp, fname);
    Jsi_MutexUnlock(interp, sinterp->Mutex);
    if (namLU == NULL)
        return Jsi_LogError("unknown function: %s", fname);

    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    char *cp = Jsi_ValueString(interp, arg, NULL);

    if (cp == NULL && !Jsi_ValueIsArray(interp, arg))
        return Jsi_LogError("expected string or array");

    Jsi_Value *vwait = Jsi_ValueArrayIndex(interp, args, 2);
    Jsi_RC rc = JSI_OK;
    bool wait = 0;
    if (vwait && Jsi_GetBoolFromValue(interp, vwait, &wait))
        return JSI_ERROR;

    if (wait) {
        Jsi_DString dStr = {};
        if (cp == NULL)
            cp = (char*)Jsi_ValueGetDString(interp, arg, &dStr, JSI_OUTPUT_JSON);
        if (interp->subOpts.mutexUnlock) Jsi_MutexUnlock(interp, interp->Mutex);
        if (Jsi_MutexLock(interp, sinterp->Mutex) != JSI_OK) {
            if (interp->subOpts.mutexUnlock) Jsi_MutexLock(interp, interp->Mutex);
            return JSI_ERROR;
        }
        Jsi_Value *srPtr = Jsi_ValueNew1(sinterp);
        /* TODO: call from this interp may not be safe if threaded.
         * Could instead use async code below then wait for unlock on se->mutex. */
        rc = Jsi_CommandInvokeJSON(sinterp, fname, cp, &srPtr);
        Jsi_DSSetLength(&dStr, 0);
        Jsi_CleanValue(sinterp, interp, srPtr, ret);
        //Jsi_ValueCopy(interp, *ret, srPtr);
        Jsi_DecrRefCount(sinterp, srPtr);
        Jsi_DSFree(&dStr);
        Jsi_MutexUnlock(interp, sinterp->Mutex);
        if (interp->subOpts.mutexUnlock && Jsi_MutexLock(interp, interp->Mutex) != JSI_OK) {
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
    Jsi_DSInit(&s->data);
    if (!cp) {
        Jsi_ValueGetDString(interp, arg, &s->data, JSI_OUTPUT_JSON);
    } else {
        Jsi_DSSetLength(&s->data, 0);
        Jsi_DSAppend(&s->data, cp, NULL);
    }
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

Jsi_RC Jsi_Mount( Jsi_Interp *interp, Jsi_Value *archive, Jsi_Value *mount, Jsi_Value **ret)
{
#if JSI__ZVFS==1
    return Zvfs_Mount(interp, archive, mount, ret);
#else
    return JSI_ERROR;
#endif
}

/* Looks in dir for autoload.jsi or lib/autoload.jsi to add to autoFiles list. */
int Jsi_AddAutoFiles(Jsi_Interp *interp, const char *dir) {
    Jsi_DString dStr = {};
    Jsi_StatBuf stat;
    int i, cnt = 0;
    for (i=0; i<2; i++) {
        Jsi_DSAppend(&dStr, dir, (i==0?"/lib":""),"/autoload.jsi", NULL);
        Jsi_Value *v = Jsi_ValueNewStringKey(interp, Jsi_DSValue(&dStr));
        if (Jsi_Stat(interp, v, &stat) != 0)
            Jsi_ValueFree(interp, v);
        else {
            if (!interp->autoFiles) {
                interp->autoFiles = Jsi_ValueNewArray(interp, 0, 0);
                Jsi_IncrRefCount(interp, interp->autoFiles);
            }
            Jsi_ObjArrayAdd(interp, interp->autoFiles->d.obj, v);
            cnt++;
            interp->autoLoaded = 0;
        }
        Jsi_DSSetLength(&dStr, 0);
    }
    Jsi_DSFree(&dStr);
    return cnt;
}

Jsi_RC Jsi_EvalZip(Jsi_Interp *interp, const char *exeFile, const char *mntDir, int *jsFound)
{
#if JSI__ZVFS==1
    Jsi_StatBuf stat;
    Jsi_Value *vinit, *linit, *vmnt = NULL;
    Jsi_Value *vexe = Jsi_ValueNewStringKey(interp, exeFile);
    Jsi_Value *ret = NULL;
    Jsi_RC rc;
    const char *omntDir = mntDir;
    int pass = 0;
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
    bool osafe = interp->isSafe;
    if (interp->startSafe)
        interp->isSafe = 0;
    rc =Jsi_Mount(interp, vexe, vmnt, &ret);
    interp->isSafe = osafe;
    if (rc != JSI_OK)
        return rc;
    Jsi_DString dStr, bStr;
    Jsi_DSInit(&dStr);
    Jsi_DSInit(&bStr);
    if (!mntDir) {
        mntDir = Jsi_KeyAdd(interp, Jsi_ValueString(interp, ret, NULL));
        Jsi_DecrRefCount(interp, ret);
    }
dochk:
    Jsi_DSAppend(&dStr, mntDir, "/main.jsi", NULL);
    if (interp->pkgDirs)
        Jsi_ValueArrayPush(interp, interp->pkgDirs, Jsi_ValueNewStringKey(interp, mntDir));
    else {
        interp->pkgDirs = Jsi_ValueNewArray(interp, &mntDir, 1);
        Jsi_IncrRefCount(interp, interp->pkgDirs);
    }
    vinit = Jsi_ValueNewStringKey(interp,  Jsi_DSValue(&dStr));
    Jsi_IncrRefCount(interp, vinit);
    Jsi_HashSet(interp->genValueTbl, vinit, vinit);
    Jsi_DSFree(&dStr);
    Jsi_DSAppend(&dStr, mntDir, "/lib", NULL);
    const char *str = Jsi_DSValue(&dStr);
    linit = Jsi_ValueNewStringKey(interp, str);
    Jsi_IncrRefCount(interp, linit);
    if (Jsi_Stat(interp, linit, &stat) == 0)
        Jsi_ValueArrayPush(interp, interp->pkgDirs, linit);
    Jsi_DecrRefCount(interp, linit);
    Jsi_DSFree(&dStr);
    Jsi_DSFree(&bStr);
    if (Jsi_Stat(interp, vinit, &stat) == 0) {
        if (jsFound)
            *jsFound |= JSI_ZIP_MAIN;
        interp->execZip = vexe;
        return Jsi_EvalFile(interp, vinit, JSI_EVAL_ARGV0|JSI_EVAL_AUTOINDEX);
    }
    if (Jsi_AddAutoFiles(interp, mntDir) && omntDir)
        *jsFound = JSI_ZIP_INDEX;
    if (!pass++) {
        str = Jsi_Strrchr(exeFile, '/');
        if (str) str++;
        else str = exeFile;
        char *bn = Jsi_DSAppend(&bStr, mntDir, "/", str, NULL);
        bn = Jsi_Strrchr(bn, '.');
        if (bn) *bn = 0;
        mntDir = Jsi_DSValue(&bStr);
        linit = Jsi_ValueNewStringKey(interp, mntDir);
        Jsi_IncrRefCount(interp, linit);
        int bsi = Jsi_Stat(interp, linit, &stat);
        Jsi_DecrRefCount(interp, linit);
        if (bsi == 0)
            goto dochk;
        Jsi_DSFree(&bStr);
    }
#endif
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
    else if (interp->scriptFile) {
        if (!interp->debugOpts.debugCallback) // Debug will use Interp.source() instead.
            Jsi_EvalFile(interp, interp->scriptFile, 0);
    } else {
        Jsi_LogBug("no eval");
    }
    if (rc != JSI_OK) {
        Jsi_LogError("eval failure");
        interp->threadErrCnt++;
        if (Jsi_MutexLock(interp, interp->Mutex) != JSI_OK)
            return NULL;
        Jsi_MutexUnlock(interp, interp->Mutex);
    }
    interpObjErase(udf);
#ifndef __WIN32
    /* TODO: should we wait/notify parent??? */
    pthread_detach(pthread_self());
    return NULL;
#endif
}
#endif

static Jsi_RC InterpConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
static Jsi_RC InterpConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);

static Jsi_CmdSpec interpCmds[] = {
    { "Interp", InterpConstructor,0,  1, "options:object=void", .help="Create a new interp", .retType=(uint)JSI_TT_USEROBJ, .flags=JSI_CMD_IS_CONSTRUCTOR, .info=0, .opts=InterpOptions},
    { "alias",  InterpAliasCmd,   0,  4, "name:string=void, func:function|null=void, args:array|null=void, async=false",.help="Set/get global alias bindings for command in an interp", .retType=(uint)JSI_TT_ANY, .flags=0, .info=FN_intalias },
    { "call",   InterpCallCmd,    2,  3, "funcName:string, args:array, wait:boolean=false", .help="Call named function in subinterp", .retType=(uint)JSI_TT_ANY, .flags=0, .info=FN_call },
    { "conf",   InterpConfCmd,    0,  1, "options:string|object=void",.help="Configure option(s)" , .retType=(uint)JSI_TT_ANY,.flags=0,.info=0,.opts=InterpOptions},
    { "eval",   InterpEvalCmd,    1,  2, "js:string, async:boolean=false", .help="Interpret script within sub-interp", .retType=(uint)JSI_TT_ANY, .flags=0, .info=FN_interpeval },
    { "info",   InterpInfoCmd,    0,  0, "", .help="Returns internal statistics about interp", .retType=(uint)JSI_TT_OBJECT },
    { "source", InterpSourceCmd,  1,  2, "file:string, async:boolean=false", .help="Interpret file within sub-interp", .retType=(uint)JSI_TT_ANY, .flags=0, .info=FN_interpeval },
    { "uplevel",InterpUplevelCmd, 2,  2, "js:string, level:number=0", .help="Interpret code at the given stack level", .retType=(uint)JSI_TT_ANY, .flags=0, .info=FN_interpuplevel },
    { "value",  InterpValueCmd,   1,  2, "var:string, level:number=0", .help="Lookup value of variable at stack level", .retType=(uint)JSI_TT_ANY },
    { NULL,     0,0,0,0, .help="Commands for accessing interps" }
};

static Jsi_UserObjReg interpobject = {
    "Interp",
    interpCmds,
    interpObjFree,
    interpObjIsTrue,
    interpObjEqual
};


static Jsi_RC InterpConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Obj *fobj;
    Jsi_Value *toacc;
    InterpObj *cmdPtr = (InterpObj *)Jsi_Calloc(1,sizeof(*cmdPtr));
    int rc = JSI_OK;
    SIGINIT(cmdPtr,INTERPOBJ);
    cmdPtr->parent = interp;

    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);

    if (!(cmdPtr->subinterp = jsi_InterpNew(interp, arg, NULL))) {
        Jsi_Free(cmdPtr);
        return JSI_ERROR;
    }
    Jsi_Interp *sinterp = cmdPtr->subinterp;
    sinterp->opts.no_exit = interp->opts.no_exit;
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
    sinterp->objId = cmdPtr->objId;
    cmdPtr->fobj = fobj;
#ifndef JSI_OMIT_THREADS
    if (sinterp->subthread) {
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
#endif //__WIN32
#else
    if (0) {
#endif //JSI_OMIT_THREADS
    } else {
        //sinterp->framePtr->tryDepth++;
        if (sinterp->scriptStr != 0) {
            if (sinterp->scriptFile && !interp->curFile)
                sinterp->curFile = Jsi_ValueString(sinterp, sinterp->scriptFile, NULL);
            rc = Jsi_EvalString(sinterp, sinterp->scriptStr, JSI_EVAL_ISMAIN);
        } else if (sinterp->scriptFile && !sinterp->debugOpts.debugCallback) {
            int len;
            if (Jsi_ValueString(interp, sinterp->scriptFile, &len) && len==0)
                Jsi_Interactive(sinterp, JSI_OUTPUT_QUOTE|JSI_OUTPUT_NEWLINES);
            else
                rc = Jsi_EvalFile(sinterp, sinterp->scriptFile, JSI_EVAL_ISMAIN);
        }
        //sinterp->framePtr->tryDepth--;
        if (rc == JSI_EXIT)
            return JSI_OK;
        if (rc != JSI_OK) {
            /*Jsi_Strcpy(interp->errMsgBuf, sinterp->errMsgBuf);
            interp->errLine = sinterp->errLine;
            interp->errFile = sinterp->errFile;
            sinterp->errMsgBuf[0] = 0;*/
            goto bail;
        }
    }
    return JSI_OK;

bail:
    interpObjErase(cmdPtr);
    Jsi_ValueMakeUndef(interp, ret);
    return JSI_ERROR;
}

static Jsi_RC Jsi_DoneInterp(Jsi_Interp *interp)
{
    Jsi_UserObjUnregister(interp, &interpobject);
    return JSI_OK;
}

static Jsi_RC jsi_InterpConfFile(Jsi_Interp *interp, const char *fname, bool etc)
{
    Jsi_RC rc;
    Jsi_DString dStr = {};
    Jsi_Value *opts = NULL, *fn = Jsi_ValueNewStringConst(interp, fname, -1);
    Jsi_IncrRefCount(interp, fn);
    bool isSafe = interp->isSafe;
    if (etc)
        interp->isSafe = 0;
    rc = Jsi_FileRead(interp, fn, &dStr);
    if (rc != JSI_OK)
        goto done;
    opts = Jsi_ValueNew1(interp);
    if (rc == JSI_OK)
        rc = Jsi_JSONParse(interp, Jsi_DSValue(&dStr), &opts, 0);
    if (rc == JSI_OK
        && Jsi_OptionsProcess(interp, InterpOptions, interp, opts, 0) < 0)
        rc = JSI_ERROR;
done:
    if (etc && isSafe)
        interp->isSafe = 1;
    if (opts)
        Jsi_DecrRefCount(interp, opts);
    Jsi_DecrRefCount(interp, fn);
    Jsi_DSFree(&dStr);
    return rc;
}

static Jsi_RC jsi_InterpConfFiles(Jsi_Interp *interp)
{
    Jsi_RC rc = JSI_OK;
#ifndef JSI__CONFFILE
#define JSI__CONFFILE "/etc/jsish.conf"
#endif
    const char *fn = JSI__CONFFILE;
    if (!fn || !fn[0])
        return JSI_OK;
    if (interp->confFile)
        rc = jsi_InterpConfFile(interp, interp->confFile, 0);
    if (rc == JSI_OK && access(fn, R_OK)==0)
        rc = jsi_InterpConfFile(interp, fn, 1);
    if (rc != JSI_OK)
        Jsi_LogWarn("parse failure: %s", fn);
    if (interp->jsppChars && Jsi_Strlen(interp->jsppChars)!=2) {
        Jsi_LogWarn("jsppChars ignored: length not 2: %s", interp->jsppChars);
        interp->jsppChars = NULL;
    }
    return rc;
}

static Jsi_RC InterpConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    InterpObj *udf = (typeof(udf))Jsi_UserObjGetData(interp, _this, funcPtr);
    Jsi_RC rc;
    Jsi_Value *opts = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Interp *sinterp = interp;
    if (!udf || udf->subinterp == interp) {
        if (interp->noConfig && opts && !Jsi_ValueIsString(interp, opts))
            return Jsi_LogError("Interp conf() is disabled for set");
        rc = Jsi_OptionsConf(interp, InterpOptions, interp, opts, ret, 0);
    } else {
        sinterp = udf->subinterp;
        Jsi_Value *popts = opts;
        if (opts && opts->vt != JSI_VT_NULL && !Jsi_ValueString(interp, opts, NULL) && opts->vt == JSI_VT_OBJECT) {
            popts = Jsi_ValueNew1(sinterp);
            Jsi_CleanValue(interp, sinterp, opts, &popts);
        }
        rc = Jsi_OptionsConf(sinterp, InterpOptions, sinterp, popts, ret, 0);
        if (popts && popts != opts)
            Jsi_DecrRefCount(sinterp, popts);
        Jsi_CleanValue(sinterp, interp, *ret, ret);
    }
    return rc;
}

Jsi_Value *Jsi_ReturnValue(Jsi_Interp *interp) {
    return interp->retValue;
}

Jsi_RC jsi_InitInterp(Jsi_Interp *interp, int release)
{
    if (release) return Jsi_DoneInterp(interp);
    Jsi_Hash *isys;
    if (!(isys = Jsi_UserObjRegister(interp, &interpobject)))
        Jsi_LogBug("Can not init interp");

    Jsi_CommandCreateSpecs(interp, interpobject.name, interpCmds, isys, JSI_CMDSPEC_ISOBJ);
    return JSI_OK;
}

bool Jsi_InterpSafe(Jsi_Interp *interp)
{
    return interp->isSafe;
}

Jsi_RC Jsi_InterpAccess(Jsi_Interp *interp, Jsi_Value* resource, int aflag)
{
    switch (aflag) {
        case JSI_INTACCESS_NETWORK:
            return (interp->noNetwork?JSI_ERROR:JSI_OK);
        case JSI_INTACCESS_MAININTERP:
            return (interp->parent?JSI_ERROR:JSI_OK);
        case JSI_INTACCESS_SETSSL:
            interp->hasOpenSSL = 1;
            return JSI_OK;
        case JSI_INTACCESS_CREATE:
            return (interp->isSafe && interp->safeMode==jsi_safe_Lockdown?JSI_ERROR: JSI_OK);
        case JSI_INTACCESS_WRITE:
        case JSI_INTACCESS_READ:
            break;
        default:
            return JSI_ERROR;
    }
    if (!resource)
        return JSI_ERROR;
    Jsi_Value *v, *dirs = ((aflag==JSI_INTACCESS_WRITE) ? interp->safeWriteDirs : interp->safeReadDirs);
    if (!interp->isSafe)
        return JSI_OK;
    if (!dirs)
        return JSI_ERROR;
    char pbuf[PATH_MAX];
    int i, m, argc = Jsi_ValueGetLength(interp, dirs);
    char *str, *dstr = Jsi_Realpath(interp, resource, pbuf);
    if (!dstr)
        return JSI_ERROR;
    for (i=0; i<argc; i++) {
        v = Jsi_ValueArrayIndex(interp, dirs, i);
        if (!v) continue;
        str = Jsi_ValueString(interp, v, &m);
        if (!str || m<=0) continue;
        if (!Jsi_Strcmp(str, dstr)) // Exact match.
            return JSI_OK;
        if (Jsi_Strncmp(str, dstr, m))
            continue;
        if (m>1 && str[m-1]=='/')
            return JSI_OK;
        if (dstr[m] == '/')
            return JSI_OK;
    }
    return JSI_ERROR;
}

Jsi_Value *Jsi_InterpResult(Jsi_Interp *interp)
{
    return interp->retValue;
}

const char *Jsi_InterpLastError(Jsi_Interp *interp, const char **errFilePtr, int *errLinePtr)
{
    if (errFilePtr)
        *errFilePtr = interp->errFile;
    if (errLinePtr)
        *errLinePtr = interp->errLine;
    return interp->errMsgBuf;
}

#ifdef __WIN32
void bzero(void *s, size_t n) {
    memset(s, 0, n);
}
#endif
#endif
