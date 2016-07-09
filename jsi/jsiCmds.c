#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#else
char *strptime(const char *buf, const char *fmt, struct tm *tm);
#endif
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <limits.h>

static int consoleInputCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[1024];
    char *p = buf;
    
    if (fgets(buf, 1024, stdin) == NULL) {
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_OK;
    }
    if ((p = Jsi_Strchr(buf, '\r'))) *p = 0;
    if ((p = Jsi_Strchr(buf, '\n'))) *p = 0;
    
    Jsi_ValueMakeString(interp, ret, Jsi_Strdup(buf));
    return JSI_OK;
}

typedef struct {
    int debug;
    Jsi_Bool index;
    Jsi_Bool isMain;
} SourceData;

static Jsi_OptionSpec SourceOptions[] = {
    JSI_OPT(INT,    SourceData, debug,  .help="Debug level" ),
    JSI_OPT(BOOL,   SourceData, index,  .help="Setup for load of jsiIndex.jsi files" ),
    JSI_OPT(BOOL,   SourceData, isMain, .help="Make Info.isMain() to true for the sourced file" ),
    JSI_OPT_END(SourceData)
};


static int SysSourceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    jsi_Pstate *ps = interp->ps;
    int rc = JSI_OK, flags = 0;
    int i, argc, oisi;
    SourceData data = {};
    Jsi_Value *v, *va = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vo = Jsi_ValueArrayIndex(interp, args, 1);
    if (vo) {
        if (!Jsi_ValueIsObjType(interp, vo, JSI_OT_OBJECT)) { /* Future options. */
            Jsi_LogError("expected options object");
            return JSI_ERROR;
        }
        if (Jsi_OptionsProcess(interp, SourceOptions, vo, &data, 0) < 0) {
            return JSI_ERROR;
        }
        if (data.index)
            flags |= JSI_EVAL_INDEX;
    }
    if ((interp->includeDepth+1) > interp->maxIncDepth) {
        Jsi_LogError("max source depth exceeded");
        return JSI_ERROR;
    }
    interp->includeDepth++;
    oisi = interp->isMain;
    interp->isMain = data.isMain;
    if (!Jsi_ValueIsArray(interp, va)) {
        v = va;
        if (v && Jsi_ValueIsString(interp,v)) {
            if (data.debug)
                fprintf(stderr, "sourcing: %s\n", Jsi_ValueString(interp, v, 0));
            rc = Jsi_EvalFile(ps->interp, v, flags);
        } else {
            Jsi_LogError("expected string");
            rc = JSI_ERROR;
        }
        goto done;
    }
    argc = Jsi_ValueGetLength(interp, va);
    for (i=0; i<argc && rc == JSI_OK; i++) {
        v = Jsi_ValueArrayIndex(interp, va, i);
        if (v && Jsi_ValueIsString(interp,v)) {
            if (data.debug)
                fprintf(stderr, "sourcing: %s\n", Jsi_ValueString(interp, v, 0));
            rc = Jsi_EvalFile(ps->interp, v, flags);
        } else {
            Jsi_LogError("expected string");
            rc = JSI_ERROR;
            break;
        }
    }
done:
    interp->isMain = oisi;
    interp->includeDepth--;
    return rc;
}

static void jsiGetTime(long *seconds, long *milliseconds)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *milliseconds = tv.tv_usec / 1000;
}

void Jsi_EventFree(Jsi_Interp *interp, Jsi_Event* event) {
    SIGASSERT(event,EVENT);
    if (event->funcVal)
        Jsi_DecrRefCount(interp, event->funcVal);
    if (event->hPtr)
        Jsi_HashEntryDelete(event->hPtr);
    MEMCLEAR(event);
    Jsi_Free(event);
}

/* Create an event and add to interp event table. */
Jsi_Event* Jsi_EventNew(Jsi_Interp *interp, Jsi_EventHandlerProc *callback, void* data)
{
    Jsi_Event *ev;
    while (1) {
        int isNew, id = interp->eventIdx++;
        Jsi_HashEntry *hPtr = Jsi_HashEntryNew(interp->eventTbl, (void*)id, &isNew);
        if (!isNew)
            continue;
        ev = (Jsi_Event*)Jsi_Calloc(1, sizeof(*ev));
        SIGINIT(ev,EVENT);
        ev->id = id;
        ev->handler = callback;
        ev->data = data;
        ev->hPtr = hPtr;
        Jsi_HashValueSet(hPtr, ev);
        break;
    }
    return ev;
}

/* Process events and return count. */
int Jsi_EventProcess(Jsi_Interp *interp, int maxEvents)
{
    Jsi_Event *ev;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch search;
    Jsi_Value* nret = NULL;
    int rc, cnt = 0, newIdx = interp->eventIdx;
    long cur_sec, cur_ms;
    jsiGetTime(&cur_sec, &cur_ms);
    Jsi_Value *vpargs = NULL;

    /*if (Jsi_MutexLock(interp, interp->Mutex) != JSI_OK)
        return JSI_ERROR;*/
    
    for (hPtr = Jsi_HashEntryFirst(interp->eventTbl, &search);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
        
        if (!(ev = (Jsi_Event*)Jsi_HashValueGet(hPtr)))
            continue;
        SIGASSERT(ev,EVENT);
        if ((int)ev->id >= newIdx) /* Avoid infinite loop of event creating events. */
            continue;
        switch (ev->evType) {
            case JSI_EVENT_SIGNAL:
#ifndef JSI_OMIT_SIGNAL   /* TODO: win signals? */
                if (!jsi_SignalIsSet(interp, ev->sigNum))
                    continue;
                jsi_SignalClear(interp, ev->sigNum);
#endif
                break;
            case JSI_EVENT_TIMER:
                if (cur_sec <= ev->when_sec && (cur_sec != ev->when_sec || cur_ms < ev-> when_ms)) {
                    if (ev->when_sec && ev->when_ms)
                        continue;
                }
                break;
            case JSI_EVENT_ALWAYS:
                break;
            default: assert(0);
        }

        cnt++;
        ev->count++;
        if (ev->handler) {
            rc = ev->handler(interp, ev->data);
        } else {
            if (vpargs == NULL) {
                vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, NULL, 0, 0));
                Jsi_IncrRefCount(interp, vpargs);
            }
            nret = Jsi_ValueNew1(interp);
            rc = Jsi_FunctionInvoke(interp, ev->funcVal, vpargs, &nret, NULL);
        }
        if (interp->deleting) {
            cnt = -1;
            goto bail;
        }
        if (rc != JSI_OK) {
            if (interp->exited) {
                cnt = rc;
                goto bail;
            }
            Jsi_LogError("event function call failure");
        }
        if (ev->once) {
            Jsi_EventFree(interp, ev);
        } else {
            ev->when_sec = cur_sec + ev->initialms / 1000;
            ev->when_ms = cur_ms + ev->initialms % 1000;
            if (ev->when_ms >= 1000) {
                ev->when_sec++;
                ev->when_ms -= 1000;
            }
        }
        if (maxEvents>0 && cnt>=maxEvents)
            break;
    }
bail:
    if (vpargs)
        Jsi_DecrRefCount(interp, vpargs);
    if (nret)
        Jsi_DecrRefCount(interp, nret);
    /*Jsi_MutexUnlock(interp, interp->Mutex);*/
    return cnt;
}

/*
 * \brief: sleep for so many milliseconds with mutex unlocked.
 */
int Jsi_Sleep(Jsi_Interp *interp, Jsi_Number dtim) {
    uint utim = 0;
    if (dtim <= 0)
        return JSI_OK;
    Jsi_MutexUnlock(interp, interp->Mutex);
    dtim = (dtim/1e3);
    if (dtim>1) {
        utim = 1e6*(dtim - (Jsi_Number)((int)dtim));
        dtim = (int)dtim;
    } else if (dtim<1) {
        utim = 1e6 * dtim;
        dtim = 0;
    }
    if (utim>0)
        usleep(utim);
    if (dtim>0)
        sleep(dtim);
    if (Jsi_MutexLock(interp, interp->Mutex) != JSI_OK) {
        Jsi_LogBug("could not reget mutex");
        return JSI_ERROR;
    }
    return JSI_OK;
}

typedef struct {
    int minTime;
    int maxEvents;
    int maxPasses;
    int sleep;
} UpdateData;

static Jsi_OptionSpec UpdateOptions[] = {
    JSI_OPT(INT,    UpdateData, maxEvents,  .help="Maximum number of events to process (or -1)" ),
    JSI_OPT(INT,    UpdateData, maxPasses,  .help="Maximum passes through event queue" ),
    JSI_OPT(INT,    UpdateData, minTime,    .help="Minimum milliseconds before returning, or -1 to loop forever" ),
    JSI_OPT(INT,    UpdateData, sleep,      .help="Time to sleep time (in milliseconds) between event checks. Default is 1" ),
    JSI_OPT_END(UpdateData)
};

#define FN_update JSI_INFO("\
Returns the number of events processed. \
Events are processed until minTime (in milliseconds) is exceeded, or forever if -1. \
The default minTime is 0, meaning return as soon as no events can be processed. \
A positive mintime will result in sleeps between event checks.")
static int SysUpdateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int maxEvents = -1, hasopts = 0;
    int cnt = 0, lcnt = 0, rc = JSI_OK;
    Jsi_Value *opts = Jsi_ValueArrayIndex(interp, args, 0);
    long cur_sec, cur_ms;
    long start_sec, start_ms;
    jsiGetTime(&start_sec, &start_ms);
    UpdateData udata;
    memset(&udata, 0, sizeof(udata));
    udata.sleep = 1;
    jsi_AddEventHandler(interp); 
       
    if (opts != NULL) {
        Jsi_Number dms;
        if (opts->vt == JSI_VT_OBJECT) {
            hasopts = 1;
            if (Jsi_OptionsProcess(interp, UpdateOptions, opts, &udata, 0) < 0) {
                return JSI_ERROR;
            }
        } else if (opts->vt != JSI_VT_NULL && Jsi_GetNumberFromValue(interp, opts, &dms) != JSI_OK)
            return JSI_ERROR;
        else
            udata.minTime = (unsigned long)dms;
    }
  
    while (1) {
        long long diftime;
        int ne = Jsi_EventProcess(interp, maxEvents);
        if (ne<0)
            break;
        cnt += ne;
        if (Jsi_InterpGone(interp))
            return JSI_ERROR;
        if (udata.minTime==0)
            break;
        jsiGetTime(&cur_sec, &cur_ms);
        if (cur_sec == start_sec)
            diftime = (long long)(cur_ms-start_ms);
        else
            diftime = (cur_sec-start_sec)*1000LL + cur_ms + (1000-start_ms);
        if (udata.minTime>0 && diftime >= (long long)udata.minTime)
            break;
        if (udata.maxPasses && ++lcnt >= udata.maxPasses)
            break;
        if ((rc = Jsi_Sleep(interp, udata.sleep)) != JSI_OK)
            break;
    }
    if (hasopts)
        Jsi_OptionsFree(interp, UpdateOptions, &udata, 0);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)cnt);
    return rc;
}

static int intervalTimer(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int once)
{
    int isNew;
    Jsi_Event *ev;
    uint id;
    Jsi_Number milli;
    long milliseconds, cur_sec, cur_ms;
    Jsi_Value *fv = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *tv = Jsi_ValueArrayIndex(interp, args, 1);
    
    if (!Jsi_ValueIsFunction(interp, fv)) {
        Jsi_LogError("expected function");
        return JSI_ERROR;
    }
    if (Jsi_GetNumberFromValue(interp, tv, &milli) != JSI_OK) {
        Jsi_LogError("expected number");
        return JSI_ERROR;
    }
    milliseconds = (long)milli;
    if (milliseconds < 0)
        milliseconds = 0;
    while (1) {
        id = interp->eventIdx++;
        Jsi_HashEntry *hPtr = Jsi_HashEntryNew(interp->eventTbl, (void*)id, &isNew);
        if (!isNew)
            continue;
        ev = (Jsi_Event*)Jsi_Calloc(1, sizeof(*ev));
        SIGINIT(ev,EVENT);
        ev->id = id;
        ev->funcVal = fv;
        Jsi_IncrRefCount(interp, fv);
        ev->hPtr = hPtr;
        jsiGetTime(&cur_sec, &cur_ms);
        ev->initialms = milliseconds;
        ev->when_sec = cur_sec + milliseconds / 1000;
        ev->when_ms = cur_ms + milliseconds % 1000;
        if (ev->when_ms >= 1000) {
            ev->when_sec++;
            ev->when_ms -= 1000;
        }
        ev->once = once;
        Jsi_HashValueSet(hPtr, ev);
        break;
    }
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)id);
    return JSI_OK;
}

static int setIntervalCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return intervalTimer(interp, args, _this, ret, funcPtr, 0);
}

static int clearIntervalCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Event *ev;
    Jsi_Number nid;
    int id;
    Jsi_Value *tv = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_HashEntry *hPtr;
    if (Jsi_GetNumberFromValue(interp, tv, &nid) != JSI_OK) {
        Jsi_LogError("expected number");
        return JSI_ERROR;
    }
    id = (uint)nid;
    hPtr = Jsi_HashEntryFind(interp->eventTbl, (void*)id);
    if (hPtr == NULL) {
        Jsi_LogError("id not found: %d", id);
        return JSI_ERROR;
    }
    ev = (Jsi_Event*)Jsi_HashValueGet(hPtr);
    Jsi_EventFree(interp, ev);
    return JSI_OK;
}

static int setTimeoutCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return intervalTimer(interp, args, _this, ret, funcPtr, 1);
}

static int SysExitCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int err = 0;
    Jsi_Value *v = NULL;
    Jsi_Number n;
    if (Jsi_ValueGetLength(interp, args) > 0) {
        v = Jsi_ValueArrayIndex(interp, args, 0);
        if (v && Jsi_GetNumberFromValue(interp,v, &n) == JSI_OK)
            err = (int)n;
        else {
            Jsi_LogError("expected number");
            return JSI_ERROR;
        }
    }
    if (interp->onExit && interp->parent && Jsi_FunctionInvokeBool(interp->parent, interp->onExit, v))
        return JSI_OK;
    if (interp->parent == NULL && 0) {
        if (interp == interp->mainInterp)
            exit(err);
        Jsi_InterpDelete(interp);
        return JSI_ERROR;
    } else {
        interp->exited = 1;
        interp->exitCode = err;
        return JSI_ERROR;
        /* TODO: cleanup events, etc. */
    }
    return JSI_OK;
}

int jsi_AssertCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Bool b;
    int rc = 0;
    Jsi_Number d;
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    char *msg = Jsi_ValueArrayIndexToStr(interp, args, 1, NULL);
    if (interp->nDebug)
        return JSI_OK;
    if (Jsi_ValueGetNumber(interp,v, &d) == JSI_OK)
        rc = (int)d;
    else if (Jsi_ValueGetBoolean(interp,v, &b) == JSI_OK)
        rc = b;
    else if (Jsi_ValueIsFunction(interp, v)) {
        Jsi_Value *vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, NULL, 0, 0));
        Jsi_IncrRefCount(interp, vpargs);
        rc = Jsi_FunctionInvoke(interp, v, vpargs, ret, NULL);
        Jsi_DecrRefCount(interp, vpargs);
        Jsi_Bool b;
        if (Jsi_ValueGetNumber(interp, *ret, &d) == JSI_OK)
            rc = (int)d;
        else if (Jsi_ValueGetBoolean(interp, *ret, &b) == JSI_OK)
            rc = b;
        else {
            Jsi_LogWarn("invalid function assert");
            return JSI_ERROR;
        }
    } else {
        Jsi_LogWarn("invalid assert");
        return JSI_ERROR;
    }
    if (rc == 0) {
        Jsi_ValueDup2(interp, ret, v);
        Jsi_LogError("assert failed: %s", msg?msg:"");
        return JSI_ERROR;
    }
    return JSI_OK;
}


static int parseIntCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Wide w = 0;
    Jsi_Number d = 0;

    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *bv = Jsi_ValueArrayIndex(interp, args, 1);
    if (!v)
        return JSI_ERROR;
    if (!bv)
        d = Jsi_ValueToNumberInt(interp, v, 1);
    else {
        int base = 0;
        char *eptr, *str = Jsi_ValueString(interp, v, NULL);
        if (str == NULL || JSI_OK != Jsi_GetIntFromValue(interp, bv, &base) || base<2 || base>36)
            return JSI_ERROR;
        d = (Jsi_Number)strtoll(str, &eptr, base);
    }
    if (jsi_num_isNaN(d) || (jsi_num_isFinite(d)==0 && Jsi_GetDoubleFromValue(interp, v, &d) != JSI_OK))
        Jsi_ValueDup2(interp, ret, interp->NaNValue);
    else {
        w = (Jsi_Wide)d;
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)w);
    }
    return JSI_OK;
}

static int parseFloatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Number n;
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_GetDoubleFromValue(interp, v, &n) != JSI_OK)
        Jsi_ValueDup2(interp, ret, interp->NaNValue);
    else {
        Jsi_ValueMakeNumber(interp, ret, n);
    }
    return JSI_OK;
}


static int SysNoOpCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return JSI_OK;
}

static int isNaNCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Number n;
    int rc = 0;
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_GetDoubleFromValue(interp, v, &n) != JSI_OK || jsi_num_isNaN(n))
        rc = 1;
    Jsi_ValueMakeBool(interp, ret, rc);
    return JSI_OK;
}
static int isFiniteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Number n;
    int rc = 1;
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_GetDoubleFromValue(interp, v, &n) != JSI_OK || !jsi_num_isFinite(n))
        rc = 0;
    Jsi_ValueMakeBool(interp, ret, rc);
    return JSI_OK;
}
/* Converts a hex character to its integer value */
static char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
static char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
static char *url_encode(char *str) {
  char *pstr = str, *buf = (char*)Jsi_Malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
static char *url_decode(char *str) {
  char *pstr = str, *buf = (char*)Jsi_Malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') { 
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}
/*
static int EscapeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
}
static int UnescapeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
}*/

static int EncodeURICmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char *cp, *str = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    cp = url_encode(str);
    Jsi_ValueMakeString(interp, ret, cp);
    return JSI_OK;
}

static int DecodeURICmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char *cp, *str = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    cp = url_decode(str);
    Jsi_ValueMakeString(interp, ret, cp);
    return JSI_OK;
}

static int SysSleepCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Number dtim = 1;
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    if (v) {
        if (Jsi_GetDoubleFromValue(interp, v, &dtim) != JSI_OK) {
            return JSI_ERROR;
        }
    }
    return Jsi_Sleep(interp, dtim);
}


static int SysGetEnvCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    extern char **environ;
    char *cp;
    int i;
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    if (v != NULL) {
        const char *fnam = Jsi_ValueString(interp, v, NULL);
        if (!fnam) {
            Jsi_LogError("expected string");
            return JSI_ERROR;
        }
        cp = getenv(fnam);
        if (cp != NULL) {
            Jsi_ValueMakeString(interp, ret, Jsi_Strdup(cp));
        }
        return JSI_OK;
    }
   /* Single object containing result members. */
    Jsi_Value *vres;
    Jsi_Obj  *ores = Jsi_ObjNew(interp);
    Jsi_Value *nnv;
    char *val, nam[200];
    Jsi_ObjIncrRefCount(interp, ores);
    vres = Jsi_ValueMakeObject(interp, NULL, ores);
    Jsi_IncrRefCount(interp, vres);
    
    for (i=0; ; i++) {
        int n;
        cp = environ[i];
        if (cp == 0 || ((val = Jsi_Strchr(cp, '='))==NULL))
            break;
        n = val-cp;
        if (n>=(int)sizeof(nam))
            n = sizeof(nam)-1;
        Jsi_Strncpy(nam, cp, n);
        val = val+1;
        nnv = Jsi_ValueMakeString(interp, NULL, Jsi_Strdup(val));
        Jsi_ObjInsert(interp, ores, nam, nnv, 0);
    }
    Jsi_ValueReplace(interp, ret, vres);
    return JSI_OK;
}

static int SysSetEnvCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    const char *fnam = Jsi_ValueString(interp, v, NULL);
    Jsi_Value *vv = Jsi_ValueArrayIndex(interp, args, 1);
    const char *fval = Jsi_ValueString(interp, vv, NULL);
    int rc = -1;
    if (fnam == 0 || fval == 0) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }

    if (fnam[0] != 0) {
#ifndef __WIN32
        rc = setenv(fnam, fval, 1);
#else  /* TODO: win setenv */
        char ebuf[BUFSIZ];
        snprintf(ebuf, sizeof(ebuf), "%s=%s", fnam, fval);
        rc = _putenv(ebuf);
#endif
    }
    if (rc != 0) {
        Jsi_LogError("setenv failure: %s = %s", fnam, fval);
        return JSI_ERROR;
    }
    return JSI_OK;
}

static int SysGetPidCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)getpid());
    return JSI_OK;
}

static int SysGetPpidCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
#ifndef __WIN32
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)getppid());
#endif
    return JSI_OK;
}

typedef struct {
    Jsi_Value* inputStr;
    Jsi_Bool bg;
    Jsi_Bool noError;
    Jsi_Bool noTrim;
    Jsi_Bool retCode;
    Jsi_Bool retAll;
    Jsi_Bool retval;
} ExecOpts;

static Jsi_OptionSpec ExecOptions[] = {
    JSI_OPT(BOOL,   ExecOpts, bg,       .help="Run command in background using system() and return OS code" ),
    JSI_OPT(STRING, ExecOpts, inputStr, .help="Use string as input and return OS code" ),
    JSI_OPT(BOOL,   ExecOpts, noError,  .help="Suppress all OS errors" ),
    JSI_OPT(BOOL,   ExecOpts, noTrim,   .help="Do not trim trailing whitespace from output" ),
    JSI_OPT(BOOL,   ExecOpts, retAll,   .help="Return the OS return code and data as an object" ),
    JSI_OPT(BOOL,   ExecOpts, retCode,  .help="Return only the OS return code" ),
    {JSI_OPTION_END}
};

#define FN_exec JSI_INFO("\
If the command ends with '&', set the 'bg' option to true. \
If the second argument is null, set the 'noError' option to true. \
If the second argument is a string, the 'inputStr' option is set. \
By default, returns the string output, unless the 'bg', 'inputStr', 'retCode' or 'retAll' options are used")

static int SysExecCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (interp->isSafe) {
        Jsi_LogError("no exec in safe mode");
        return JSI_ERROR;
    }
    int n, rc = JSI_OK, exitCode = 0, hasopts = 0, sLen, sLen2;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *opt = Jsi_ValueArrayIndex(interp, args, 1);
    const char *cp2=NULL, *cp = Jsi_ValueString(interp, arg, &sLen);
    Jsi_DString dStr = {};
    ExecOpts edata = {};
    edata.retval = 1;
    
    if (opt != NULL) {
        if (opt->vt == JSI_VT_OBJECT && opt->d.obj->ot == JSI_OT_OBJECT) {
            hasopts = 1;
            if (Jsi_OptionsProcess(interp, ExecOptions, opt, &edata, 0) < 0) {
                return JSI_ERROR;
            }
            if (edata.retCode)
                edata.retval = 0;
        } else if ((cp2=Jsi_ValueString(interp, opt, &sLen2))) {
            edata.inputStr = opt;
        } else if (Jsi_ValueIsNull(interp, opt)) {
                edata.noError = 1;
        } else {
            Jsi_LogError("expected: string?,null|instr?");
            return JSI_ERROR;
        }
    }

    int isbg = 0, ec = 0;
    if (edata.bg || (isbg=((sLen>1 && cp[sLen-1] == '&')))) {
        if (edata.inputStr) {
            Jsi_LogError("inputStr can not used with bg");
            goto done;
            rc = JSI_ERROR;
        }
        if (!isbg) {
            Jsi_DSAppend(&dStr, cp, " &", NULL);
            cp = Jsi_DSValue(&dStr);
        }
        edata.bg = 1;
        edata.retCode = 1;
        edata.retval = 0;
        exitCode = ((ec=system(cp))>>8);
        Jsi_DSSetLength(&dStr, 0);
    } else if (edata.inputStr) {
        edata.retCode = 1;
        edata.retval = 0;
        cp2 = Jsi_ValueString(interp, edata.inputStr, &sLen2);
        FILE *fp = popen(cp, "w");
        if (!fp) 
            exitCode = errno;
        else {
            while ((n=fwrite(cp2, 1, sLen, fp))>0) {
                sLen -= n;
            }
            exitCode = ((ec=pclose(fp))>>8);
        }
    } else {
        FILE *fp = popen(cp, "r");
        if (!fp) 
            exitCode = errno;
        else {
            char buf[BUFSIZ];;
            while ((n=fread(buf, 1, sizeof(buf), fp))>0)
                Jsi_DSAppendLen(&dStr, buf, n);
            exitCode = ((ec=pclose(fp))>>8);
        }
    }
    if (exitCode && edata.noError==0 && edata.retCode==0 && edata.retAll==0) {
        if (exitCode==ENOENT)
            Jsi_LogError("command not found: %s", cp);
        else
            Jsi_LogError("program exit code (%x)", exitCode);
        rc = JSI_ERROR;
    }
    if (!edata.noTrim) {
        char *cp = Jsi_DSValue(&dStr);
        int iLen = Jsi_DSLength(&dStr);
        while (iLen>0 && isspace(cp[iLen-1]))
            iLen--;
        cp[iLen] = 0;
    }
    if (edata.retAll) {
        Jsi_Obj *nobj = Jsi_ObjNew(interp);
        Jsi_ValueMakeObject(interp, ret, nobj);
        Jsi_Value *cval = Jsi_ValueNewNumber(interp, (Jsi_Number)exitCode);
        Jsi_ObjInsert(interp, nobj, "code", cval, 0);
        cval = Jsi_ValueNewNumber(interp, (Jsi_Number)(ec&0xff));
        Jsi_ObjInsert(interp, nobj, "status", cval, 0);
        cval = Jsi_ValueNew(interp);
        Jsi_ValueFromDS(interp, &dStr, &cval);
        Jsi_ObjInsert(interp, nobj, "data", cval, 0);
        
    } else if (edata.retval)
        Jsi_ValueFromDS(interp, &dStr, ret);
    else {
        Jsi_DSFree(&dStr);
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)exitCode);
    }
done:
    if (hasopts)
        Jsi_OptionsFree(interp, ExecOptions, &edata, 0);
    return rc;
}

#define FN_conslog JSI_INFO("\
Each argument is quoted (unlike the builtin string concatenation).\
If called with 0 or 1 argument, a newline is output, otherwise stderr is flushed")
static int consoleLogCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, Jsi_Value **ret,
    Jsi_Func *funcPtr)
{
    int argc = Jsi_ValueGetLength(interp, args);
    int i, cnt = 0;
    for (i = 0; i < argc; ++i) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, i);
        if (!v) continue;
        if (cnt++)
            fprintf(stderr, " ");
        Jsi_Puts(interp, v, JSI_OUTPUT_QUOTE|JSI_OUTPUT_STDERR);
    }
    if (argc<=1)
        printf("\n");
    else
        fflush(stderr);
    return JSI_OK;
}

#define FN_puts JSI_INFO("\
Each argument is quoted (unlike the builtin string concatenation).\
If called with 0 or 1 argument, a newline is output, otherwise stdout is flushed")
static int SysPutsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, Jsi_Value **ret,
    Jsi_Func *funcPtr)
{
    int argc = Jsi_ValueGetLength(interp, args);;
    Jsi_Value *v;
    int i, cnt = 0;
    for (i=0; i<argc; i++) {
        v = Jsi_ValueArrayIndex(interp, args, i);
        if (!v) continue;
        if (cnt++)
            printf(" ");
        Jsi_Puts(interp, v, JSI_OUTPUT_QUOTE);
    }
    if (argc<=1)
        printf("\n");
    else
        fflush(stdout);
    return JSI_OK;
}

/*static int DateNowCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    Jsi_Number n = ((double)(tv.tv_usec/1000)) + ((double)tv.tv_sec)*1000;
    Jsi_ValueMakeNumber(interp, ret, n);
    return JSI_OK;
}*/


typedef struct {
    Jsi_Bool utc;
    const char *fmt;
} DateOpts;

static Jsi_OptionSpec DateOptions[] = {
    JSI_OPT(BOOL,   DateOpts, utc, .help="time is in utc" ),
    JSI_OPT(STRKEY, DateOpts, fmt, .help="format string for time" ),
    {JSI_OPTION_END}
};

static const char *timeFmts[] = {
    "%c",
    "%Y-%m-%d %H:%M:%S.",
    "%Y-%m-%dT%H:%M:%S.",
    "%Y-%m-%d %H:%M:%S %Z",
    "%Y-%m-%d %H:%M:%S",
    "%Y-%m-%d %H:%M",
    "%Y-%m-%d",
    "%Y-%m-%dT%H:%M:%S %Z",
    "%Y-%m-%dT%H:%M:%S",
    "%Y-%m-%dT%H:%M",
    "%Y-%m-%d",
    "%H:%M:%S.",
    "%H:%M:%S",
    "%H:%M",
    "%z",
    NULL
};

int Jsi_DatetimeParse(Jsi_Interp *interp, const char *str, const char *fmt, int isUtc, Jsi_Number *datePtr)
{
    const char *rv = NULL;
    Jsi_Wide n = -1;
    int j = 0, rc = JSI_OK;
    Jsi_Number ms = 0;
    struct tm tm = {};
    time_t t;
    if (!Jsi_Strcmp("now", str)) {
        struct timeval tv;
        struct timezone tz;
        gettimeofday(&tv, &tz);
        n = tv.tv_sec*1000LL + ((Jsi_Wide)tv.tv_usec)/1000LL;
        if (isUtc) {
            n += 60 * tz.tz_minuteswest;
        }
        if (datePtr)
            *datePtr = (Jsi_Number)n;
        return rc;
    }
    if (fmt && fmt[0]) {
        rv = strptime(str, fmt, &tm);
    } else {
        if (fmt) j++;
        while (timeFmts[j]) {
            rv = strptime(str, timeFmts[j], &tm);
            if (rv != NULL)
                break;
            j++;
        }
    }
    if (!rv)
        rc = JSI_ERROR;
    else {
        if (j <= 1 && rv != str && *(rv-1) == '.') {
            ms = atof(rv-1);
        }
        if (isUtc) {
#ifdef __WIN32
            t = _mkgmtime(&tm);
#else
            t = timegm(&tm);
#endif
        } else {
            t = mktime(&tm);
            //TODO: dst
            /*if (tm.tm_isdst)
                t -= 3600;*/
        }
        n = (Jsi_Number)t*1000 + ms;
    }
    if (rc == JSI_OK && datePtr)
        *datePtr = (Jsi_Number)n;
    return rc;
}

int Jsi_DatetimeFormat(Jsi_Interp *interp, Jsi_Number num, const char *fmt, int isUtc, Jsi_DString *dStr)
{
    char buf[1000];
    time_t tt;
    int rc = JSI_OK;
    Jsi_DString eStr;
    Jsi_DSInit(&eStr);
    
    tt = (time_t)(num/1000);
    if (fmt==NULL)
        fmt = timeFmts[0];
    else if (*fmt == 0) {
        if (!isUtc)
            fmt = timeFmts[1];
        else
            fmt = timeFmts[2];
    }
    int flen = Jsi_Strlen(fmt);
    if (flen>4 && Jsi_Strcmp(fmt+flen-2, "%f")==0) {
        Jsi_DSAppendLen(&eStr, fmt, flen-2);
        Jsi_DSAppendLen(&eStr, "%S.", -1);
        fmt = Jsi_DSValue(&eStr);
    }
#ifdef __WIN32
    struct tm *tm;
    if (isUtc)
        tm = gmtime(&tt);
    else
        tm = localtime(&tt);
    int rr = strftime(buf, sizeof(buf), fmt, tm);
#else
    struct tm tm;
    if (isUtc)
        gmtime_r(&tt, &tm);
    else
        localtime_r(&tt, &tm);
    int rr = strftime(buf, sizeof(buf), fmt, &tm);
#endif
       
    if (rr<=0) {
        Jsi_LogError("time format error");
        rc = JSI_ERROR;
    } else {
        Jsi_DSAppendLen(dStr, buf, -1);
        if (buf[rr-1] == '.') {
            snprintf(buf, sizeof(buf), "%3.3d", (int)(((Jsi_Wide)num)%1000));
            Jsi_DSAppendLen(dStr, buf[1]=='.'?buf+2:buf, -1);
        }
    }
    Jsi_DSFree(&eStr);
    return rc;
}

static int DateStrptimeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char *str = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    Jsi_Value *opt = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Number w = 0;
    int isOpt = 0, rc = JSI_OK;
    DateOpts opts = {};
    const char *fmt = NULL;
    if (opt != NULL && opt->vt != JSI_VT_NULL && !(fmt = Jsi_ValueString(interp, opt, NULL))) {
        if (Jsi_OptionsProcess(interp, DateOptions, opt, &opts, 0) < 0) {
            return JSI_ERROR;
        }
        isOpt = 1;
        fmt = opts.fmt;
    }
    rc = Jsi_DatetimeParse(interp, str, fmt, opts.utc, &w);
    if (rc != JSI_OK) {
        Jsi_ValueDup2(interp, ret, interp->NaNValue);
        rc = JSI_ERROR;
    } else
        Jsi_ValueMakeNumber(interp, ret, w);
    if (isOpt)
        Jsi_OptionsFree(interp, DateOptions, &opts, 0);
    return rc;
}

#define FN_strftime JSI_INFO("\
Giving null as the value will use current time.")
static int DateStrftimeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value* val = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value* opt = Jsi_ValueArrayIndex(interp, args, 1);
    const char *fmt = NULL, *cp = NULL;
    DateOpts opts = {};
    Jsi_Number num;
    int isOpt = 0, rc = JSI_OK;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    
    if (val==NULL || Jsi_ValueIsNull(interp, val))
        num = 1000.0 * (Jsi_Number)time(NULL);
    else if ((cp=Jsi_ValueString(interp, val, NULL))) {
        /* Handle below */
    } else if (Jsi_GetDoubleFromValue(interp, val, &num) != JSI_OK)
        return JSI_ERROR;
        
    if (opt != NULL && opt->vt != JSI_VT_NULL && !(fmt = Jsi_ValueString(interp, opt, NULL))) {
        if (Jsi_OptionsProcess(interp, DateOptions, opt, &opts, 0) < 0) {
            return JSI_ERROR;
        }
        isOpt = 1;
        fmt = opts.fmt;
    }
    const char *errMsg = "time format error";
    if (cp) {
        if (isdigit(*cp))
            rc = Jsi_GetDouble(interp, cp, &num);
        else {
            if (Jsi_Strcmp(cp,"now")==0) {
                num = Jsi_DateTime();
            } else {
                rc = JSI_ERROR;
                errMsg = "allowable strings are 'now' or digits";
            }
        }
    }
    if (rc == JSI_OK)
        rc = Jsi_DatetimeFormat(interp, num, fmt, opts.utc, &dStr);
    if (rc != JSI_OK) {
        Jsi_LogError("%s", errMsg);
    } else {
        Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&dStr));
    }
    if (isOpt)
        Jsi_OptionsFree(interp, DateOptions, &opts, 0);
    Jsi_DSFree(&dStr);
    return rc;
}

#define FN_infovars JSI_INFO("\
Returns all values, data or function.")
static int InfoVarsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char *key;
    int n, curlen = 0, isreg = 0, isobj = 0;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch search;
    Jsi_Value *v;
    Jsi_Obj *nobj = Jsi_ObjNew(interp);
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    const char *name = NULL;

    if (arg) {
        if (arg->vt == JSI_VT_STRING)
            name = arg->d.s.str;
        else if (arg->vt == JSI_VT_OBJECT) {
            switch (arg->d.obj->ot) {
                case JSI_OT_STRING: name = arg->d.obj->d.s.str; break;
                case JSI_OT_REGEXP: isreg = 1; break;
                case JSI_OT_FUNCTION:
                case JSI_OT_OBJECT: isobj = 1; break;
                default: return JSI_OK;
            }
        } else
            return JSI_OK;
    }
    if (!name)
        Jsi_ValueMakeArrayObject(interp, ret, nobj);
    if (isobj) {
        Jsi_TreeEntry* tPtr;
        Jsi_TreeSearch search;
        for (tPtr = Jsi_TreeEntryFirst(arg->d.obj->tree, &search, 0);
            tPtr; tPtr = Jsi_TreeEntryNext(&search)) {
            v = (Jsi_Value*)Jsi_TreeValueGet(tPtr);
            if (v==NULL || Jsi_ValueIsFunction(interp, v)) continue;
    
            n = curlen++;
            key = (char*)Jsi_TreeKeyGet(tPtr);
            Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringKey(interp, key), n);
        }
        Jsi_TreeSearchDone(&search);
        return JSI_OK;
    }
    for (hPtr = Jsi_HashEntryFirst(interp->varTbl, &search);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
        if (Jsi_HashValueGet(hPtr))
            continue;
        key = (char*)Jsi_HashKeyGet(hPtr);
        if (name) {
            if (Jsi_Strcmp(name,key))
                continue;
            Jsi_ValueMakeObject(interp, ret, nobj);
            v = Jsi_VarLookup(interp, key);
            Jsi_ObjInsert(interp, nobj, "type", Jsi_ValueNewStringKey(interp,Jsi_ValueTypeStr(interp, v)),0);
            Jsi_ValueMakeObject(interp, ret, nobj);
            return JSI_OK;
        }
        if (isreg) {
            int ismat;
            Jsi_RegExpMatch(interp, arg, key, &ismat, NULL);
            if (!ismat)
                continue;
        }
        n = curlen++;
        Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringKey(interp, key), n);
    }
    return JSI_OK;
}

static Jsi_Value *jsiNewConcatValue(Jsi_Interp *interp, const char *c1, const char *c2)
{
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_DSAppend(&dStr, c1, c2, NULL);
    Jsi_Value *val = Jsi_ValueNewStringKey(interp, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return val;
}

static int InfoFuncDataSub(Jsi_Interp *interp, Jsi_Value *arg, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int flags, Jsi_Obj *nobj)
{
    const char *key, *name = NULL, *ename;
    int n, curlen = 0, isreg = 0, isobj = 0, isglob = 0, nLen = 0;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch search;
    Jsi_FuncObj *fo;
    Jsi_Func *func;
    Jsi_Obj *fobj;
    Jsi_Value *val, *nval;
    int isfunc = (flags&1);
    int isdata = (flags&2);
    int addPre = (flags&4);
    Jsi_DString dPre = {};
    
    if (!nobj) {
        nobj = Jsi_ObjNew(interp);
        Jsi_ValueMakeArrayObject(interp, ret, nobj);
    }
    
    if (!arg)
        name = "*";
    else {
        if (arg->vt == JSI_VT_STRING)
            name = Jsi_ValueString(interp, arg, &nLen);
        else if (arg->vt == JSI_VT_OBJECT) {
            switch (arg->d.obj->ot) {
                case JSI_OT_STRING: name = Jsi_ValueString(interp, arg, &nLen); break;
                case JSI_OT_REGEXP: isreg = 1; break;
                case JSI_OT_OBJECT: isobj = 1; break;
                case JSI_OT_FUNCTION:
                    fo = arg->d.obj->d.fobj;
                    if (fo->func->type&FC_BUILDIN)
                        return JSI_OK;
                    goto dumpfunc;
                break;
                default: return JSI_OK;
            }
        } else
            return JSI_OK;
    }
    ename = name;
    if (isobj) {
        Jsi_TreeEntry* tPtr;
        Jsi_TreeSearch search;

dumpobj:
        for (tPtr = Jsi_TreeEntryFirst(arg->d.obj->tree, &search, 0);
            tPtr; tPtr = Jsi_TreeEntryNext(&search)) {
            Jsi_Value *v = (Jsi_Value*)Jsi_TreeValueGet(tPtr);
            key = (char*)Jsi_TreeKeyGet(tPtr);
            if (v==NULL) continue;
            if (Jsi_ValueIsFunction(interp, v)) {
                if (!isfunc) continue;
            } else {
                if (!isdata) continue;
            }
            if (isreg) {
                int ismat;
                Jsi_RegExpMatch(interp, arg, key, &ismat, NULL);
                if (!ismat)
                    continue;
            } else if (ename) {
                if (isglob) {
                    if (!Jsi_GlobMatch(ename, key, 0))
                        continue;
                } else {
                    if (Jsi_Strcmp(ename, key))
                        continue;
                }
            }
    
            n = curlen++;
            nval = jsiNewConcatValue(interp, (addPre?Jsi_DSValue(&dPre):""), key);
            Jsi_ObjArraySet(interp, nobj, nval, n);
        }
        Jsi_TreeSearchDone(&search);
        Jsi_DSFree(&dPre);
        return JSI_OK;
    }
    
    Jsi_ScopeStrs *sstrs;            
    const char *gs, *gb, *dotStr;
    
    if (name) {
        val = Jsi_NameLookup(interp, name);
        if (val)
            goto dumpvar;
        gs=strchr(name,'*');
        gb=strchr(name,'[');
        dotStr = strrchr(name, '.');
        if (dotStr && ((gs && gs < dotStr) || (gb && gb < dotStr))) {
            Jsi_LogError("glob must be after last dot");
            return JSI_ERROR;
        }
        isglob = (gs || gb);
        if (!isglob)
            return JSI_OK;
        if (dotStr) {
            if (addPre)
                Jsi_DSAppendLen(&dPre, name, dotStr-ename+1);
            ename = dotStr+1;
            Jsi_DString pStr = {};
            Jsi_DSAppendLen(&pStr, name, dotStr-name);
            val = Jsi_NameLookup(interp, Jsi_DSValue(&pStr));
            Jsi_DSFree(&pStr);
            if (!val)
                return JSI_OK;
            if (Jsi_ValueIsObjType(interp, val, JSI_OT_OBJECT)) {
                arg = val;
                goto dumpobj;
            }
            return JSI_OK;
        } 
    }
        
    for (hPtr = Jsi_HashEntryFirst(interp->varTbl, &search);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
        key = (char*)Jsi_HashKeyGet(hPtr);
        if (!isfunc)
            goto doreg;
        if (!(fobj = (Jsi_Obj*)Jsi_HashValueGet(hPtr)))
            continue;
        if (isfunc && name && isreg==0 && isglob==0) {

            if (Jsi_Strcmp(key, name))
                continue;
            /* Fill object with args and locals of func. */
            fo = fobj->d.fobj; 
            goto dumpfunc;
        }
doreg:
        if (isreg) {
            int ismat;
            Jsi_RegExpMatch(interp, arg, key, &ismat, NULL);
            if (!ismat)
                continue;
        } else if (name) {
            if (isglob) {
                if (!Jsi_GlobMatch(name, key, 0))
                    continue;
            } else {
                if (Jsi_Strcmp(name, key))
                    continue;
            }
        }
        n = curlen++;
        nval = jsiNewConcatValue(interp, (addPre?Jsi_DSValue(&dPre):""), key);
        Jsi_ObjArraySet(interp, nobj, nval, n);
    }
    Jsi_DSFree(&dPre);
    return JSI_OK;

dumpfunc:
{
    nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeObject(interp, ret, nobj);
    func = fo->func;
    sstrs = func->argnames;
    char *strs[sstrs->count+1];
    int i;
    for (i=0; i<sstrs->count; i++)
        strs[i] = sstrs->args[i].name;
    Jsi_Value *aval = Jsi_ValueNewArray(interp, strs, sstrs->count);
    Jsi_ObjInsert(interp, nobj, "args", aval, 0);
    sstrs = func->localnames;
    Jsi_Value *lval = Jsi_ValueNewArray(interp, strs, sstrs->count);
    Jsi_ObjInsert(interp, nobj, "locals", lval, 0);
    if (func->script) {
        lval = Jsi_ValueNewStringKey(interp, func->script);
        Jsi_ObjInsert(interp, nobj, "script", lval, 0);
        lval = Jsi_ValueNewNumber(interp, (Jsi_Number)func->opcodes->codes->line);
        Jsi_ObjInsert(interp, nobj, "lineStart", lval, 0);
        lval = Jsi_ValueNewNumber(interp, (Jsi_Number)func->bodyline.last_line);
        Jsi_ObjInsert(interp, nobj, "lineEnd", lval, 0);
    }
    return JSI_OK;
}
dumpvar:
    if (isfunc && Jsi_ValueIsFunction(interp, val)) {
        fo = val->d.obj->d.fobj;
        goto dumpfunc;
    }
    nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeObject(interp, ret, nobj);
    Jsi_Value *aval = Jsi_ValueNewStringDup(interp, ename);
    Jsi_ObjInsert(interp, nobj, "name", aval, 0);
    aval = Jsi_ValueNewStringDup(interp, jsi_ValueTypeName(interp, val));
    Jsi_ObjInsert(interp, nobj, "type", aval, 0);
    return JSI_OK;
}

static int InfoLookupCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char *str = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    if (!str)
        return JSI_OK;
    Jsi_Value *val = Jsi_NameLookup(interp, str);
    if (!val)
        return JSI_OK;
    Jsi_ValueDup2(interp, ret, val);
    return JSI_OK;
}

static int InfoFuncsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    return InfoFuncDataSub(interp, arg, _this, ret, funcPtr, 1, NULL);
}

#define FN_infodata JSI_INFO("\
Like info.vars(), but does not return function values.")
static int InfoDataCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    return InfoFuncDataSub(interp, arg, _this, ret, funcPtr, 2, NULL);
}

static int InfoKeywordsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (interp->lexkeyTbl) {
        Jsi_HashKeysDump(interp, interp->lexkeyTbl, ret, 0);
        Jsi_ValueArraySort(interp, *ret, 0);
    }
    return JSI_OK;
}


static int InfoOptionsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    uint i, argc =  Jsi_ValueGetLength(interp, args);
    Jsi_DString dStr = {};
    const char *str;
    Jsi_DSAppend(&dStr, "[", NULL);
    for (i=1; (str = Jsi_OptionTypeStr((Jsi_OptionTypes)i, argc));i++)
        Jsi_DSAppend(&dStr, (i>1?", ":""), "\"", str, "\"", NULL);
    Jsi_DSAppend(&dStr, "]", NULL);
    Jsi_JSONParse(interp, Jsi_DSValue(&dStr), ret, 0);
    return JSI_OK;
}
static int InfoVersionCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    if (!arg)
        Jsi_ValueMakeNumber(interp, ret, Jsi_Version());
    else if (!Jsi_ValueIsTrue(interp, arg))
        return JSI_ERROR;
    else {
        char buf[BUFSIZ];
        snprintf(buf, sizeof(buf),
            "{major:%d, minor:%d, release:%d}",
            JSI_VERSION_MAJOR, JSI_VERSION_MINOR, JSI_VERSION_RELEASE);
        
        return Jsi_JSONParse(interp, buf, ret, 0);
    }
    return JSI_OK;
}


static int InfoIsMainCmd (Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int isi = (interp->isMain);
    if (isi == 0) {
        const char *c2 = interp->curFile;
        Jsi_Value *v1 = interp->argv0;
        if (c2 && v1 && Jsi_ValueIsString(interp, v1)) {
            char *c1 = Jsi_ValueString(interp, v1, NULL);
            isi = (c1 && !strcmp(c1,c2));
        }
    }
    Jsi_ValueMakeBool(interp, ret, isi);
    return JSI_OK;
}

static int InfoArgv0Cmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (interp->argv0)
        Jsi_ValueDup2(interp, ret, interp->argv0);
     return JSI_OK;
}

static int InfoExecZipCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (interp->execZip)
        Jsi_ValueDup2(interp, ret, interp->execZip);
     return JSI_OK;
}

static int InfoScriptCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    int isreg = 0;
    const char *name = NULL;

    if (!arg) {
        name = interp->curFile;
    } else {
        if (arg->vt == JSI_VT_OBJECT) {
            switch (arg->d.obj->ot) {
                case JSI_OT_FUNCTION: name = arg->d.obj->d.fobj->func->script; break;
                case JSI_OT_REGEXP: isreg = 1; break;
            }
        } else
            return JSI_OK;
    }
    if (isreg) {
        Jsi_HashEntry *hPtr;
        Jsi_HashSearch search;
        int curlen = 0, n;
        const char *key;
        Jsi_Obj *nobj = Jsi_ObjNew(interp);
        Jsi_ValueMakeArrayObject(interp, ret, nobj);
        for (hPtr = Jsi_HashEntryFirst(interp->fileTbl, &search);
            hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
            key = (const char*)Jsi_HashKeyGet(hPtr);
            if (isreg) {
                int ismat;
                Jsi_RegExpMatch(interp, arg, key, &ismat, NULL);
                if (!ismat)
                    continue;
            }
            n = curlen++;
            Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringKey(interp, key), n);
        }
        return JSI_OK;
    }
    if (name)
        Jsi_ValueMakeStringDup(interp, ret, name);
    return JSI_OK;
}

static int InfoScriptDirCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path = interp->curDir;
    if (path)
        Jsi_ValueMakeString(interp, ret, Jsi_Strdup(path));
    return JSI_OK;
}

static int isBigEndian()
{
    union { unsigned short s; unsigned char c[2]; } uval = {0x0102};
    return uval.c[0] == 1;
}

static int InfoPlatformCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[BUFSIZ];
#ifdef __WIN32
    const char *os="win", *platform = "win";
#else
    const char *os="linux", *platform = "unix";
#endif
#ifndef JSI_OMIT_THREADS
    int thrd = 1;
#else
    int thrd = 0;
#endif
    if (Jsi_FunctionReturnIgnored(interp, funcPtr))
        return JSI_OK;
    snprintf(buf, sizeof(buf),
        "{os:\"%s\", platform:\"%s\", hasThreads:%s, pointerSize:%d, "
        "intSize:%d, wideSize:%d, byteOrder:\"%sEndian\"}",
        os, platform, thrd?"true":"false", sizeof(void*), sizeof(int),
        sizeof(Jsi_Wide), isBigEndian()? "big" : "little");
        
    return Jsi_JSONParse(interp, buf, ret, 0);
}

static int InfoNamedCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Obj *nobj;
    char *argStr = NULL;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    if (arg && (argStr = (char*)Jsi_ValueString(interp, arg, NULL)) == NULL) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }
    nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    return jsi_UserObjDump(interp, argStr, nobj);
}

/* 
    { "size",    cDataSize,     1,  1, "name", .help="" },
    { "names",   cDataNames,    0,  0, "Return name of all defined Cdata", .help="" },
    { "info",    cDataInfo,     1,  1, "name", .help="" },
    { "get",     cDataGet,      2,  3, "name,index?,field?", .help="" },
    { "set",     cDataSet,      3,  3, "name,index,fieldvals", .help="" },
*/

static int cDataNamesCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return Jsi_HashKeysDump(interp, interp->optionDataHash, ret, 0);
}

enum {
    _JSI_CDATA_INFO=0,
    _JSI_CDATA_GET=1,
    _JSI_CDATA_SET=2,
    _JSI_CDATA_SIZE=3,
    _JSI_CDATA_SCHEMA=4,
    _JSI_CDATA_SCHEMAMD5=5,
    _JSI_CDATA_STRUCT=6
};

static int cdatasubCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int sub)
{
    char *argStr = NULL;
    int i, aidx = 0, slen;
    Jsi_Number dnum;
    int argc = Jsi_ValueGetLength(interp, args);
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    if (arg == NULL || (argStr = (char*)Jsi_ValueString(interp, arg, &slen)) == NULL) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }
    Jsi_DbMultipleBind* opts = (Jsi_DbMultipleBind*)Jsi_HashGet(interp->optionDataHash, argStr);
    if (!opts) {
        Jsi_LogError("unknown option-data: %s", argStr);
        return JSI_ERROR;
    }
    if (sub == _JSI_CDATA_INFO) { // Dump
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        Jsi_Value *svalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        jsi_DumpOptionSpecs(interp, sobj, opts->opts);
        Jsi_ValueDup2(interp, ret, svalue);
        return JSI_OK;
    }
    if (sub == _JSI_CDATA_SCHEMA || sub == _JSI_CDATA_SCHEMAMD5) {  // Generate a DB schema.
        char tbuf[100];
        Jsi_DString dStr, mStr;
        Jsi_DSInit(&dStr);
        Jsi_DSInit(&mStr);
        i = 0;
        const Jsi_OptionSpec *specs, *send = NULL;
        for (specs=opts->opts; specs->type>JSI_OPTION_NONE && specs->type<=JSI_OPTION_END; specs++) {
            const char *tstr = NULL;
            if (specs->flags&JSI_OPT_DB_DIRTY)
                continue;
            if (!Jsi_Strcmp(specs->name, "rowid"))
                continue;
            switch(specs->type) {
            case JSI_OPTION_BOOL: tstr = "BOOLEAN"; break;
            case JSI_OPTION_TIMESTAMP: tstr = "TIMESTAMP"; break;
            case JSI_OPTION_DATE: tstr = "DATE"; break;
            case JSI_OPTION_TIME: tstr = "TIME"; break;
            case JSI_OPTION_DATETIME: tstr = "DATETIME"; break;
            case JSI_OPTION_BYTE:
            case JSI_OPTION_WORD:
            case JSI_OPTION_DWORD:
            case JSI_OPTION_INT:
            case JSI_OPTION_QWORD:
            case JSI_OPTION_WIDE: tstr = "INT"; break;
            case JSI_OPTION_DOUBLE: tstr = "FLOAT"; break;
            
            case JSI_OPTION_STRBUF:
                tstr = tbuf;
                snprintf(tbuf, sizeof(tbuf), "VARCHAR(%d)", specs->size);;
                break;
            case JSI_OPTION_DSTRING: 
            case JSI_OPTION_STRKEY:
            case JSI_OPTION_STRING:
                tstr = "TEXT";
                break;
            case JSI_OPTION_CUSTOM:
                if (specs->custom == Jsi_Opt_SwitchEnum || specs->custom == Jsi_Opt_SwitchBitset) {
                    if (specs->flags&JSI_OPT_FORCE_INT)
                        tstr = "INT";
                    else
                        tstr = "TEXT";
                } else
                    tstr = "";
                break;
            case JSI_OPTION_VALUE: /* Unsupported. */
            case JSI_OPTION_VAR:
            case JSI_OPTION_OBJ:
            case JSI_OPTION_ARRAY:
            case JSI_OPTION_FUNC:
            case JSI_OPTION_NONE:
                break;
            case JSI_OPTION_END:
                send = specs;
                break;
            }
            
            if (!tstr) continue;
            Jsi_DSAppend(&dStr, (i?"\n ,":"  "), (specs->extName?specs->extName:specs->name), NULL);
            Jsi_DSAppend(&mStr, (i?"\n ,":"  "), (specs->extName?specs->extName:specs->name), NULL);
            if (tstr[0]) {
                Jsi_DSAppend(&dStr, " ", tstr, NULL);
                Jsi_DSAppend(&mStr, " ", tstr, NULL);
            }
            if (specs->userData) {
                const char *udrest = NULL;
                if (specs->type == JSI_OPTION_TIMESTAMP && !Jsi_Strcmp(specs->userData, "DEFAULT"))
                    udrest = "(round((julianday('now') - 2440587.5)*86400.0))";
                else if (specs->type == JSI_OPTION_DATETIME && !Jsi_Strcmp(specs->userData, "DEFAULT"))
                    udrest = "(round((julianday('now','start of day') - 2440587.5)*86400000))";
                else if (specs->type == JSI_OPTION_DATE && !Jsi_Strcmp(specs->userData, "DEFAULT"))
                    udrest = "(round((julianday('now') - 2440587.5)*86400000))";
                else if (specs->type == JSI_OPTION_TIME && !Jsi_Strcmp(specs->userData, "DEFAULT"))
                    udrest = "(round((julianday('now')-julianday('now','start of day') - 2440587.5)*86400000))";
                Jsi_DSAppend(&dStr, " ", specs->userData, udrest, NULL);
                Jsi_DSAppend(&mStr, " ", specs->userData, udrest, NULL);
            }
            if (specs->help && sub != _JSI_CDATA_SCHEMAMD5)
                Jsi_DSAppend(&dStr, " -- ", specs->help, NULL);
            if (specs->type==JSI_OPTION_END)
                break;
            i++;
        }
        if (send->userData) {
            Jsi_DSAppend(&dStr,  send->userData, NULL);
            Jsi_DSAppend(&mStr,  send->userData, NULL);
        }
        char buf[33];
        Jsi_Md5Str(interp, buf, Jsi_DSValue(&mStr), -1);
        if (sub == _JSI_CDATA_SCHEMAMD5)
            Jsi_ValueMakeStringDup(interp, ret, buf);
        else {
            Jsi_DString eStr;
            Jsi_DSInit(&eStr);
            Jsi_DSAppend(&eStr, "\n-- '", send->name, "\': ", NULL);
            if (send->help)
                Jsi_DSAppend(&eStr,  send->help, NULL);
            Jsi_DSAppend(&eStr, "\n", Jsi_DSValue(&dStr), "\n-- MD5: ", buf, NULL);
            Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&eStr));
            Jsi_DSFree(&eStr);
        }
        Jsi_DSFree(&dStr);
        Jsi_DSFree(&mStr);
        return JSI_OK;
    }
    if (sub == _JSI_CDATA_STRUCT) {  // Generate the C struct
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        i = 0;
        const Jsi_OptionSpec *specs, *send = NULL;
        for (specs=opts->opts; specs->type>JSI_OPTION_NONE && specs->type<=JSI_OPTION_END; specs++) {
            int nsz = 0;
            const char *tstr = NULL;
            switch(specs->type) {
            case JSI_OPTION_BOOL: tstr = "int"; break;
            case JSI_OPTION_TIMESTAMP: tstr = "time_t"; break;
            case JSI_OPTION_DATE:
            case JSI_OPTION_TIME:
            case JSI_OPTION_DATETIME: tstr = "Jsi_Number"; break;
            case JSI_OPTION_INT:
            case JSI_OPTION_BYTE:
            case JSI_OPTION_WORD:
            case JSI_OPTION_DWORD:
            case JSI_OPTION_QWORD:
            case JSI_OPTION_WIDE: tstr = "int"; break;
            case JSI_OPTION_DOUBLE: tstr = "Jsi_Number"; break;
            case JSI_OPTION_DSTRING: tstr = "Jsi_DString"; break;
            case JSI_OPTION_STRBUF:tstr = "char"; nsz=1; break;
            case JSI_OPTION_STRKEY:tstr = "char*"; break;
            case JSI_OPTION_STRING:tstr = "int"; break;
            case JSI_OPTION_CUSTOM:
                if (specs->custom == Jsi_Opt_SwitchEnum || specs->custom == Jsi_Opt_SwitchBitset) {
                    tstr = "int";
                } else {
                    tstr = "char";
                    nsz = 1;
                }
                break;
            case JSI_OPTION_VALUE: /* Unsupported. */
            case JSI_OPTION_VAR:
            case JSI_OPTION_OBJ:
            case JSI_OPTION_ARRAY:
            case JSI_OPTION_FUNC:
            case JSI_OPTION_NONE:
                break;
            case JSI_OPTION_END:
                send = specs;
                break;
            }
            
            if (!tstr) continue;
            Jsi_DSAppend(&dStr, "\n    ", tstr, " ", specs->name, NULL);
            if (nsz) {
                Jsi_DSPrintf(&dStr, "[%d]", specs->size);
            }
            Jsi_DSAppend(&dStr, ";", NULL);
            if (specs->help)
                Jsi_DSAppend(&dStr, " /* ", specs->help, " */", NULL);
            if (specs->type==JSI_OPTION_END)
                break;
            i++;
        }
        Jsi_DString eStr;
        Jsi_DSInit(&eStr);
        Jsi_DSAppend(&eStr, "\n/* ", send->name, ": ", NULL);
        if (send->help)
            Jsi_DSAppend(&eStr,  send->help, NULL);
        Jsi_DSAppend(&eStr, " */\ntypedef struct ",  send->name, " {", Jsi_DSValue(&dStr), "\n};\n", NULL);
        Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&eStr));
        Jsi_DSFree(&eStr);
        
        Jsi_DSFree(&dStr);
        return JSI_OK;
    }
    
    int flags = opts->flags;
    void *rec = opts->data, *prec = rec;
    void **recPtrPtr = NULL;
    if (flags&JSI_DB_PTR_PTRS) {
        recPtrPtr = (void**)rec; /* This is really a void***, but gets recast below. */
        rec = *recPtrPtr;
    }
    int cnt = opts->numData;
    if (cnt<=0 && rec && flags&JSI_DB_PTR_PTRS) {
        for (cnt=0; ((void**)rec)[cnt]!=NULL; cnt++);
    } else if (cnt==0)
        cnt = 1;
    
    if (sub == _JSI_CDATA_SIZE) {
        Jsi_ValueMakeNumber(interp, ret, cnt);
        return JSI_OK;
    }

    Jsi_Value *argIdx = Jsi_ValueArrayIndex(interp, args, 1);
    if (Jsi_ValueGetNumber(interp, argIdx, &dnum) != JSI_OK)
        return JSI_ERROR;
    aidx = (int)dnum;
        
    if (aidx<0 || aidx>=cnt) {
        Jsi_LogError("index %d out of range: 0-%d", aidx, cnt);
        return JSI_ERROR;
    }
    Jsi_Value *arg2 = (argc>2 ? Jsi_ValueArrayIndex(interp, args, 2) : NULL);

    if (sub == _JSI_CDATA_GET && arg2 && (argStr = (char*)Jsi_ValueString(interp, arg2, &slen)) == NULL) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }
    if (flags & (JSI_DB_PTRS|JSI_DB_PTR_PTRS))
        prec = ((void**)rec)[aidx];
    else {
        for (i=0; opts->opts[i].type < JSI_OPTION_END; i++);
        int structSize = opts->opts[i].size;
        prec = (char*)rec + (aidx * structSize);
    }
    if (!prec)
        return JSI_OK;
    return Jsi_OptionsConf(interp, opts->opts, arg2, prec, ret, 0);
}

static int cDataInfoCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return cdatasubCmd(interp, args, _this, ret, funcPtr, _JSI_CDATA_INFO);
}

static int cDataGetCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return cdatasubCmd(interp, args, _this, ret, funcPtr, _JSI_CDATA_GET);
}

static int cDataSetCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return cdatasubCmd(interp, args, _this, ret, funcPtr, _JSI_CDATA_SET);
}

static int cDataSizeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return cdatasubCmd(interp, args, _this, ret, funcPtr, _JSI_CDATA_SIZE);
}

static int cDataSchemaCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return cdatasubCmd(interp, args, _this, ret, funcPtr, _JSI_CDATA_SCHEMA);
}

static int cDataSchemaMd5Cmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return cdatasubCmd(interp, args, _this, ret, funcPtr, _JSI_CDATA_SCHEMAMD5);
}

static int cDataStructCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return cdatasubCmd(interp, args, _this, ret, funcPtr, _JSI_CDATA_STRUCT);
}

static int InfoExecutableCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (jsi_execName == NULL)
        Jsi_ValueMakeNull(interp, ret);
    else
        Jsi_ValueMakeStringKey(interp, ret, jsi_execName);
    return JSI_OK;
}

#define FN_infoevent JSI_INFO("\
With no args, returns list of all outstanding events.  With one arg, returns info\
for the given event id.")

static int InfoEventCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int n = 0, nid;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch search;
    Jsi_Obj *nobj;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    
    if (arg && Jsi_GetIntFromValue(interp, arg, &nid) != JSI_OK)
        return JSI_ERROR;
    if (!arg) {
        nobj = Jsi_ObjNew(interp);
        Jsi_ValueMakeArrayObject(interp, ret, nobj);
    }

    for (hPtr = Jsi_HashEntryFirst(interp->eventTbl, &search);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
        int id;
        id = (int)Jsi_HashKeyGet(hPtr);
        if (!arg) {
            Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewNumber(interp, (Jsi_Number)id), n);
            n++;
        } else if (id == nid) {
            Jsi_Event *ev = (Jsi_Event *)Jsi_HashValueGet(hPtr);
            Jsi_DString dStr;
            int rc;
            
            Jsi_DSInit(&dStr);
            switch (ev->evType) {
                case JSI_EVENT_SIGNAL:
                    Jsi_DSPrintf(&dStr, "{ type:\"signal\", sigNum:%d, count:%u, builtin:%s }", 
                        ev->sigNum, ev->count, (ev->handler?"true":"false") );
                    break;
                case JSI_EVENT_TIMER: {
             
                    long cur_sec, cur_ms;
                    long long ms;
                    jsiGetTime(&cur_sec, &cur_ms);
                    ms = (ev->when_sec*1000LL + ev->when_ms) - (cur_sec * 1000LL + cur_ms);
                    Jsi_DSPrintf(&dStr, "{ type:\"timer\", once:%s, when:%lld, count:%u, initial:%ld, builtin:%s }",
                        ev->once?"true":"false", ms, ev->count, ev->initialms, (ev->handler?"true":"false") );
                    break;
                }
                case JSI_EVENT_ALWAYS:
                    Jsi_DSPrintf(&dStr, "{ type:\"always\", count:%u, builtin:%s }", 
                        ev->count, (ev->handler?"true":"false") );
                    break;
            }
            rc = Jsi_JSONParse(interp, Jsi_DSValue(&dStr), ret, 0);
            Jsi_DSFree(&dStr);
            return rc;
        }
    }
    return JSI_OK;
}

static int eventInfoCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return InfoEventCmd(interp, args, _this, ret, funcPtr);
}

static int InfoErrorCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Obj *nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeObject(interp, ret, nobj);
    Jsi_ValueInsert( interp, *ret, "file", Jsi_ValueNewStringKey(interp, interp->errFile?interp->errFile:""), 0);
    Jsi_ValueInsert(interp, *ret, "line", Jsi_ValueNewNumber(interp, interp->errLine), 0);
    return JSI_OK;
}

void jsi_DumpCmdSpec(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_CmdSpec* spec, const char *name)
{
    Jsi_ObjInsert(interp, nobj, "minArgs", Jsi_ValueNewNumber(interp, spec->minArgs),0);
    Jsi_ObjInsert(interp, nobj, "maxArgs", Jsi_ValueNewNumber(interp, spec->maxArgs),0);
    if (spec->help)
        Jsi_ObjInsert(interp, nobj, "help", Jsi_ValueNewStringKey(interp, spec->help),0);
    if (spec->info)
        Jsi_ObjInsert(interp, nobj, "info", Jsi_ValueNewStringKey(interp, spec->info),0);
    Jsi_ObjInsert(interp, nobj, "args", Jsi_ValueNewStringKey(interp, spec->argStr?spec->argStr:""),0);
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_ObjInsert(interp, nobj, "retType", Jsi_ValueNewStringKey(interp, jsi_typeName(interp, spec->retType, &dStr)), 0);
    Jsi_DSFree(&dStr);
    Jsi_ObjInsert(interp, nobj, "name", Jsi_ValueNewStringKey(interp, name?name:spec->name),0);
    Jsi_ObjInsert(interp, nobj, "type", Jsi_ValueNewStringKey(interp, "command"),0);
    if (spec->opts) {
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        Jsi_Value *svalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        jsi_DumpOptionSpecs(interp, sobj, spec->opts);
        Jsi_ObjInsert(interp, nobj, "options", svalue, 0);
    }
}
void jsi_DumpCmdItem(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_CmdSpecItem* csi, const char *name)
{
    Jsi_ObjInsert(interp, nobj, "name", Jsi_ValueNewStringKey(interp, name), 0);
    Jsi_ObjInsert(interp, nobj, "type", Jsi_ValueNewStringKey(interp, "object"),0);
    Jsi_ObjInsert(interp, nobj, "constructor", Jsi_ValueNewBoolean(interp, csi && csi->isCons), 0);
    if (csi && csi->help)
        Jsi_ObjInsert(interp, nobj, "help", Jsi_ValueNewStringDup(interp, csi->help),0);
    if (csi && csi->info)
        Jsi_ObjInsert(interp, nobj, "info", Jsi_ValueNewStringDup(interp, csi->info),0);
    if (csi && csi->isCons && csi->spec->argStr)
        Jsi_ObjInsert(interp, nobj, "args", Jsi_ValueNewStringDup(interp, csi->spec->argStr),0);
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_ObjInsert(interp, nobj, "retType", Jsi_ValueNewStringKey(interp, jsi_typeName(interp, csi->spec->retType, &dStr)), 0);
    Jsi_DSFree(&dStr);
    if (csi && csi->isCons && csi->spec->opts) {
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        Jsi_Value *svalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        jsi_DumpOptionSpecs(interp, sobj, csi->spec->opts);
        Jsi_ObjInsert(interp, nobj, "options", svalue, 0);
    }
}

static int InfoCmdsCmdSub(Jsi_Interp *interp, Jsi_Value *arg, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int all, int sort, Jsi_Obj *nobj)
{
    const char *key, *name = NULL, *cp;
    int curlen = 0, icnt = 0, isglob = 0, dots = 0, gotFirst = 0, ninc;
    Jsi_HashEntry *hPtr = NULL;
    Jsi_HashSearch search;
    Jsi_CmdSpecItem *csi;
    Jsi_DString dStr;
    const char *skey;
    Jsi_CmdSpec *spec;
    
    Jsi_DSInit(&dStr);
    int isreg = 0;

    if (!arg)
        name = "*";
    else {
        if (arg->vt == JSI_VT_STRING)
            name = arg->d.s.str;
        else if (arg->vt == JSI_VT_OBJECT) {
            switch (arg->d.obj->ot) {
                case JSI_OT_STRING: name = arg->d.obj->d.s.str; break;
                case JSI_OT_REGEXP: isreg = 1; break;
                case JSI_OT_FUNCTION:
                    spec = arg->d.obj->d.fobj->func->cmdSpec;
                    if (!spec)
                        return JSI_OK;
                    skey = spec->name;
                    goto dumpfunc;
                    break;
                default: return JSI_OK;
            }
        } else
            return JSI_OK;
    }
    ninc = 0;
    isglob = (isreg == 0 && name && (Jsi_Strchr(name,'*') || Jsi_Strchr(name,'[')));
    if (isglob) {
        cp = name;
        while (*cp) { if (*cp == '.') dots++; cp++; }
    }
    while (1) {
        if (++icnt == 1) {
           /* if (0 && name)
                hPtr = Jsi_HashEntryFind(interp->cmdSpecTbl, "");
            else */ {
                hPtr = Jsi_HashEntryFirst(interp->cmdSpecTbl, &search);
                gotFirst = 1;
            }
        } else if (icnt == 2 && name && !gotFirst)
            hPtr = Jsi_HashEntryFirst(interp->cmdSpecTbl, &search);
        else
            hPtr = Jsi_HashEntryNext(&search);
        if (!hPtr)
            break;
        csi = (Jsi_CmdSpecItem*)Jsi_HashValueGet(hPtr);
        key = (const char *)Jsi_HashKeyGet(hPtr);
        if (isglob && dots == 0 && *key && Jsi_GlobMatch(name, key, 0)) {
                Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringKey(interp, key), curlen++);
                ninc++;
        }
        if (isglob == 0 && name && key[0] && Jsi_Strcmp(name, key)==0) {
            skey = (char*)name;
            spec = NULL;
            goto dumpfunc;
        }
        assert(csi);
        do {
            int i;
            for (i=0; csi->spec[i].name; i++) {
                Jsi_DSSetLength(&dStr, 0);
                spec = csi->spec+i;
                if (i==0 && spec->flags&JSI_CMD_IS_CONSTRUCTOR && !all) /* ignore constructor name. */
                    continue;
                if (key[0])
                    Jsi_DSAppend(&dStr, key, ".", NULL);
                Jsi_DSAppend(&dStr, spec->name, NULL);
                skey = Jsi_DSValue(&dStr);
                if (isglob) {
                    if (!(((key[0]==0 && dots == 0) || (key[0] && dots == 1)) &&
                        Jsi_GlobMatch(name, skey, 0)))
                    continue;
                }
                if (name && isglob == 0 && Jsi_Strcmp(name,skey) == 0) {
dumpfunc:
                    nobj = Jsi_ObjNew(interp);
                    Jsi_ValueMakeObject(interp, ret, nobj);
                    if (spec == NULL)
                        jsi_DumpCmdItem(interp, nobj, csi, skey);
                    else
                        jsi_DumpCmdSpec(interp, nobj, spec, skey);
                    Jsi_DSFree(&dStr);
                    return JSI_OK;
                    
                } else if (isglob == 0 && isreg == 0)
                    continue;
                if (isreg) {
                    int ismat;
                    Jsi_RegExpMatch(interp, arg, skey, &ismat, NULL);
                    if (!ismat)
                        continue;
                }
                Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringKey(interp, skey), curlen++);
                ninc++;
            }
            csi = csi->next;
        } while (csi);
    }
    Jsi_DSFree(&dStr);
    if (sort && (*ret)->vt != JSI_VT_UNDEF)
        Jsi_ValueArraySort(interp, *ret, JSI_SORT_ASCII);
    return JSI_OK;    
}

static int InfoCmdsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Bool b = 0;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vbool = Jsi_ValueArrayIndex(interp, args, 1);
    if (vbool && Jsi_ValueGetBoolean(interp, vbool, &b) != JSI_OK) {
        Jsi_LogError("expected boolean");
        return JSI_ERROR;
    }
    Jsi_Obj *nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);

    return InfoCmdsCmdSub(interp, arg, _this, ret, funcPtr, b, 1, nobj);
}

static int InfoCompletionsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int slen, argc = Jsi_ValueGetLength(interp, args);
    Jsi_Value *arg1 = Jsi_ValueArrayIndex(interp, args, 0);
    char *key, *substr, *str = Jsi_ValueString(interp, arg1, &slen);
    int start = 0, end = slen-1;

    if (argc>1) {
        Jsi_Number n1;
        if (Jsi_GetNumberFromValue(interp, Jsi_ValueArrayIndex(interp, args, 1), &n1) != JSI_OK)
            return JSI_ERROR;
        start = (int)n1;
        if (start<0) start = 0;
        if (start>=slen) start=slen-1;
    }
    if (argc>2) {
        Jsi_Number n2;
        if (Jsi_GetNumberFromValue(interp, Jsi_ValueArrayIndex(interp, args, 2), &n2) != JSI_OK)
            return JSI_ERROR;
        end = (int)n2;
        if (end<0) end = slen-1;
        if (end>=slen) end=slen-1;
    }

    Jsi_DString dStr = {};
    Jsi_DSAppendLen(&dStr, str?str+start:"", end-start+1);
    Jsi_DSAppend(&dStr, "*", NULL);
    substr = Jsi_DSValue(&dStr);
    Jsi_Value *arg = Jsi_ValueNewStringDup(interp, substr);
    Jsi_IncrRefCount(interp, arg);
    
    Jsi_Obj *nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);

    int rc = InfoCmdsCmdSub(interp, arg, _this, ret, funcPtr, 0, 0, nobj);
    if (rc != JSI_OK || (Jsi_ValueIsArray(interp, *ret) && (*ret)->d.obj->arrCnt>0))
        goto done;
    rc = InfoFuncDataSub(interp, arg, _this, ret, funcPtr, 0x7, nobj);
    if (rc != JSI_OK || (Jsi_ValueIsArray(interp, *ret) && (*ret)->d.obj->arrCnt>0))
        goto done;
    if (str) {
        substr[end-start+1] = 0;
        slen = Jsi_Strlen(substr);
    }
    if (str == NULL || !Jsi_Strchr(substr, '.')) {
        Jsi_HashEntry *hPtr;
        Jsi_HashSearch search;
        int n = Jsi_ValueGetLength(interp, *ret);
        for (hPtr = Jsi_HashEntryFirst(interp->lexkeyTbl, &search);
            hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
            key = (char*)Jsi_HashKeyGet(hPtr);
            if (str == NULL || !Jsi_Strncmp(substr, key, slen))
                Jsi_ValueArraySet(interp, *ret, Jsi_ValueNewStringKey(interp, key), n++);
        }
    }
    Jsi_ValueArraySort(interp, *ret, 0);
done:
    Jsi_DSFree(&dStr);
    Jsi_DecrRefCount(interp, arg);
    return rc;
}

Jsi_Value * Jsi_LookupCS(Jsi_Interp *interp, const char *name, int *ofs)
{
    const char *csdot = strrchr(name, '.');
    int len;
    *ofs = 0;
    if (csdot == name)
        return NULL;
    if (!csdot)
        return interp->csc;
    if (!csdot[1])
        return NULL;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_DSAppendLen(&dStr, name, len=csdot-name);
    Jsi_Value *cs = Jsi_NameLookup(interp, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    *ofs = len+1;
    return cs;
}

static void ValueObjDeleteStr(Jsi_Interp *interp, Jsi_Value *target, const char *key, int force)
{
    const char *kstr = key;
    if (target->vt != JSI_VT_OBJECT) return;

    Jsi_TreeEntry *hPtr;
    Jsi_HashEntry *hePtr = Jsi_HashEntryFind(target->d.obj->tree->interp->strKeyTbl, key);
    if (hePtr)
        kstr = (const char*)Jsi_HashKeyGet(hePtr);
    hPtr = Jsi_TreeEntryFind(target->d.obj->tree, kstr);
    if (hPtr == NULL || ( hPtr->f.bits.dontdel && !force))
        return;
    Jsi_TreeEntryDelete(hPtr);
}

int Jsi_CommandDelete(Jsi_Interp *interp, const char *name) {
    int ofs;
    Jsi_Value *fv = Jsi_NameLookup(interp, name);
    Jsi_Value *cs = Jsi_LookupCS(interp, name, &ofs);
    if (cs) {
        const char *key = name + ofs;
        ValueObjDeleteStr(interp, cs, key, 1);
    }
    if (fv)
        Jsi_DecrRefCount(interp, fv);
    return JSI_OK;
}


Jsi_Value * Jsi_CommandCreate(Jsi_Interp *interp, const char *name, Jsi_CmdProc *cmdProc, void *privData)
{
    Jsi_Value *n = NULL;
    const char *csdot = strrchr(name, '.');
    if (0 && csdot) {
        Jsi_LogBug("commands with dot unsupported: %s", name);
        return NULL;
    }
    if (csdot == name) {
        name = csdot+1;
        csdot = NULL;
    }
    if (!csdot) {
        n = jsi_MakeFuncValue(interp, cmdProc, name);
        Jsi_IncrRefCount(interp, n);
        Jsi_ObjDecrRefCount(interp, n->d.obj);
        Jsi_Func *f = n->d.obj->d.fobj->func;
        f->privData = privData;
        f->name = Jsi_KeyAdd(interp, name);
        Jsi_ValueInsertFixed(interp, interp->csc, f->name, n);
        return n;
    }
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_DSAppendLen(&dStr, name, csdot-name);
    Jsi_Value *cs = Jsi_NameLookup(interp, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    if (cs) {
    
        n = jsi_MakeFuncValue(interp, cmdProc, csdot+1);
        Jsi_IncrRefCount(interp, n);
        Jsi_ObjDecrRefCount(interp, n->d.obj);
        Jsi_Func *f = n->d.obj->d.fobj->func;
        
        f->name = Jsi_KeyAdd(interp, csdot+1);
        f->privData = privData;
        Jsi_ValueInsertFixed(interp, cs, f->name, n);
        //Jsi_ObjDecrRefCount(interp, n->d.obj);
    }
    return n;
}

static Jsi_Value *CommandCreateWithSpec(Jsi_Interp *interp, Jsi_CmdSpec *cmdSpec, Jsi_Value *proto, void *privData, const char *parent)
{
    int iscons = (cmdSpec->flags&JSI_CMD_IS_CONSTRUCTOR);
    Jsi_Value *func;
    Jsi_Func *f;
    if (cmdSpec->name)
        Jsi_KeyAdd(interp, cmdSpec->name);
    if (cmdSpec->proc == NULL)
    {
        Jsi_Value *func = jsi_ProtoObjValueNew1(interp, cmdSpec->name);
        Jsi_ValueInsertFixed(interp, NULL, cmdSpec->name, func);
        f = func->d.obj->d.fobj->func;
    } else {
    
        func = jsi_MakeFuncValueSpec(interp, cmdSpec, privData);
    #ifdef JSI_MEM_DEBUG
        func->VD.label = "CMDspec";
        func->VD.label2 = cmdSpec->name;
    #endif
        //Jsi_IncrRefCount(interp, func);
        //Jsi_HashSet(interp->genValueTbl, func, func);
        Jsi_ValueInsertFixed(interp, (iscons?NULL:proto), cmdSpec->name, func);
        f = func->d.obj->d.fobj->func;
    
        if (cmdSpec->name)
            f->name = cmdSpec->name;
        f->f.flags = (cmdSpec->flags & JSI_CMD_MASK);
        f->f.bits.hasattr = 1;
        if (iscons) {
            f->f.bits.iscons = 1;
            Jsi_ValueInsertFixed(interp, func, "prototype", proto);
            Jsi_PrototypeObjSet(interp, "Function", Jsi_ValueGetObj(interp, func));
        }
    }
    if (cmdSpec->argStr && interp->typeCheck >= jsi_TypeChk_Static) { // Sanity check args signature.
        Jsi_DString dStr = {};
        Jsi_DSAppend(&dStr, cmdSpec->argStr, NULL);
        const char *elips = strstr(cmdSpec->argStr, "...");
        if (cmdSpec->maxArgs<0 && !elips)
            Jsi_DSAppend(&dStr, ", ...", NULL);
        int aCnt = 0, i = -1, mis = 0;
        char *cp = Jsi_DSValue(&dStr);
        while (cp[++i]) if (cp[i]==',') aCnt++;
        
        if (cmdSpec->maxArgs>=0) {
            aCnt++;
            if (cmdSpec->minArgs<0 || cmdSpec->minArgs>cmdSpec->maxArgs || 
                cmdSpec->minArgs>aCnt || cmdSpec->maxArgs>aCnt || elips) {
                mis = 1;
            }                    
        } else {
            if (cmdSpec->minArgs<0 || cmdSpec->minArgs>aCnt) {
                mis = 1;
            }                    
        }
        Jsi_DSFree(&dStr);
        if (mis)
            Jsi_LogWarn("inconsistent arg string for \"%s.%s(%s)\" [%d,%d]",
                parent, cmdSpec->name, cmdSpec->argStr, cmdSpec->minArgs, cmdSpec->maxArgs);
        if (f->argnames==NULL && cmdSpec->argStr) {
            // At least give a clue where the problem is.
            const char *ocfile = interp->curFile;
            jsi_Pline *opl = interp->parseLine, pline;
            interp->parseLine = &pline;
            pline.first_line = 1;
            interp->curFile = cmdSpec->name;
            f->argnames = jsi_ParseArgStr(interp, cmdSpec->argStr);
            interp->curFile = ocfile;
            interp->parseLine = opl;
        }
    }

    f->retType = cmdSpec->retType;
    return func;
}


Jsi_Value *Jsi_CommandCreateSpecs(Jsi_Interp *interp, const char *name, Jsi_CmdSpec *cmdSpecs,
    void *privData, int flags)
{
    int i = 0;
    Jsi_Value *proto;
    if (!cmdSpecs[0].name)
        return NULL;
    if (!name)
        name = cmdSpecs[0].name;
    name = Jsi_KeyAdd(interp, name);
    if (flags & JSI_CMDSPEC_PROTO) {
        proto = (Jsi_Value*)privData;
        privData = NULL;
        i++;
    } else if (name[0] == 0)
        proto = NULL;
    else {
        if ((flags & JSI_CMDSPEC_NOTOBJ)) {
            proto = Jsi_CommandCreate(interp, name, NULL, privData);
        } else {
            proto = jsi_ProtoValueNew(interp, name, NULL);
        }
    }
    for (; cmdSpecs[i].name; i++) {
        CommandCreateWithSpec(interp,  cmdSpecs+i, proto, privData, name);
        /*Jsi_Value *fc = CommandCreateWithSpec(interp,  cmdSpecs+i, proto, privData);
        if (fc && i>0 && flags & JSI_CMDSPEC_GENVALUE) {
            Jsi_HashSet(interp->genValueTbl, fc, fc);
        }*/
    }

    int isNew;
    Jsi_HashEntry *hPtr;
    hPtr = Jsi_HashEntryNew(interp->cmdSpecTbl, name, &isNew);
    if (!hPtr) {
        Jsi_LogBug("failed cmdspec register: %s", name);
        return NULL;
    }
    Jsi_CmdSpecItem *op, *p = (Jsi_CmdSpecItem*)Jsi_Calloc(1,sizeof(*p));
    SIGINIT(p,CMDSPECITEM);
    p->spec = cmdSpecs;
    p->flags = flags;
    p->proto = proto;
    p->privData = privData;
    p->name = (const char*)Jsi_HashKeyGet(hPtr);
    p->hPtr = hPtr;
    Jsi_CmdSpec *csi = cmdSpecs;
    p->isCons = (csi && csi->flags&JSI_CMD_IS_CONSTRUCTOR);
    while (csi->name)
        csi++;
    p->help = csi->help;
    p->info = csi->info;
    if (!isNew) {
        op = (Jsi_CmdSpecItem*)Jsi_HashValueGet(hPtr);
        p->next = op;
    }
    Jsi_HashValueSet(hPtr, p);
    return proto;
}

void jsi_CmdSpecDelete(Jsi_Interp *interp, void *ptr)
{
    Jsi_CmdSpecItem *cs = (Jsi_CmdSpecItem*)ptr;
    Jsi_CmdSpec *p;
    SIGASSERT(cs,CMDSPECITEM);
    //return;
    while (cs) {
        p = cs->spec;
        Jsi_Value *proto = cs->proto;
        if (proto)
            Jsi_DecrRefCount(interp, proto);
        while (0 && p && p->name) {
            /*Jsi_Value *proto = p->proto;
            if (proto)
                Jsi_DecrRefCount(interp, proto);*/
            p++;
        }
        ptr = cs;
        cs = cs->next;
        Jsi_Free(ptr);
    }
}

static int SysTimesCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int rc=JSI_OK, i, n=1, argc = Jsi_ValueGetLength(interp, args);
    Jsi_Value *func = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Wide diff, start, end;
    if(!Jsi_ValueIsFunction(interp, func)){
        Jsi_LogError("expected function");
        return JSI_ERROR;
    }
    if (argc > 1 && Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, 1), &n) != JSI_OK)
        return JSI_ERROR;
    if (n<=0) {
        Jsi_LogError("count not > 0: %d", n);
        return JSI_ERROR;
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    start = (Jsi_Wide) tv.tv_sec * 1000000 + tv.tv_usec;
    for (i=0; i<n; i++) {
        rc = Jsi_FunctionInvoke(interp, func, NULL, ret, NULL);
    }
    gettimeofday(&tv, NULL);
    end = (Jsi_Wide) tv.tv_sec * 1000000 + tv.tv_usec;
    diff = (end - start);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)diff);
    return rc;
}

static int SysFormatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int rc;
    Jsi_DString dStr;
    rc = Jsi_FormatString(interp, args, &dStr);
    if (rc != JSI_OK)
        return rc;
    Jsi_ValueMakeString(interp, ret, Jsi_Strdup(Jsi_DSValue(&dStr)));
    Jsi_DSFree(&dStr);
    return JSI_OK;
}

static int quoteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_DString dStr = {};
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    const char *str = Jsi_ValueGetDString(interp, arg, &dStr, 1);
    Jsi_ValueMakeString(interp, ret, Jsi_Strdup(str));
    Jsi_DSFree(&dStr);
    return JSI_OK;
}

static const char ev[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int atend;

static int
getidx(char *buffer, int len, int *posn) {
    char c;
    char *idx;
    if (atend) return -1;
    do {
    if ((*posn)>=len) {
        atend = 1;
        return -1;
    }
    c = buffer[(*posn)++];
    if (c<0 || c=='=') {
        atend = 1;
        return -1;
    }
    idx = Jsi_Strchr(ev, c);
    } while (!idx);
    return idx - ev; 
} 

static int B64Decode(Jsi_Interp *interp, char *inbuffer, int ilen, Jsi_Value **ret)
{
    int olen, pos, tlen=1024, tpos=0;
    char outbuffer[3], *tbuf;
    int c[4];
    
    tbuf=(char*)Jsi_Malloc(tlen);
    pos = 0; 
    atend = 0;
    while (!atend) {
        if (inbuffer[pos]=='\n' ||inbuffer[pos]=='\r') { pos++; continue; }
        c[0] = getidx(inbuffer, ilen, &pos);
        c[1] = getidx(inbuffer, ilen, &pos);
        c[2] = getidx(inbuffer, ilen, &pos);
        c[3] = getidx(inbuffer, ilen, &pos);

        olen = 0;
        if (c[0]>=0 && c[1]>=0) {
            outbuffer[0] = ((c[0]<<2)&0xfc)|((c[1]>>4)&0x03);
            olen++;
            if (c[2]>=0) {
                outbuffer[1] = ((c[1]<<4)&0xf0)|((c[2]>>2)&0x0f);
                olen++;
                if (c[3]>=0) {
                    outbuffer[2] = ((c[2]<<6)&0xc0)|((c[3])&0x3f);
                    olen++;
                }
            }
        }

        if (olen>0) {
            if ((tpos+olen+1)>=tlen) {
                tbuf=(char*)Jsi_Realloc(tbuf,tlen+1024);
                tlen+=1024;
            }
            memcpy(tbuf+tpos,outbuffer,olen);
            tpos+=olen;
        }
    }
    tbuf[tpos] = 0;
    Jsi_ValueMakeStringDup(interp, ret, tbuf);
    free(tbuf);
    return JSI_OK;
}

static int
B64Encode(Jsi_Interp *interp, char *ib, int ilen, Jsi_Value **ret)
{
    int i=0, pos=0;
    char c[74];
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);

    while (pos<ilen) {
#define P(n,s) ((pos+n)>ilen?'=':ev[s])
        c[i++]=ev[(ib[pos]>>2)&0x3f];
        c[i++]=P(1,((ib[pos]<<4)&0x30)|((ib[pos+1]>>4)&0x0f));
        c[i++]=P(2,((ib[pos+1]<<2)&0x3c)|((ib[pos+2]>>6)&0x03));
        c[i++]=P(3,ib[pos+2]&0x3f);
        if (i>=72) {
            c[i++]='\n';
            c[i]=0;
            Jsi_DSAppendLen(&dStr, c, i);
            i=0;
        }
        pos+=3;
    }
    if (i) {
        /*    c[i++]='\n';*/
        c[i]=0;
        Jsi_DSAppendLen(&dStr, c, i);
        i=0;
    }
    Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&dStr));
    return JSI_OK;
}

static int B64DecodeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int ilen;
    char *inbuffer = Jsi_ValueArrayIndexToStr(interp, args, 0, &ilen);
    return B64Decode(interp, inbuffer, ilen, ret);
}

static int B64EncodeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int ilen;
    char *inbuffer = Jsi_ValueArrayIndexToStr(interp, args, 0, &ilen);
    return B64Encode(interp, inbuffer, ilen, ret);
}

static int B64DecodeFileCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char *b = NULL, buf[BUFSIZ];
    Jsi_Value *fn = Jsi_ValueArrayIndex(interp, args, 0);
    int rc, n, siz = 0;
    Jsi_Channel chan = Jsi_Open(interp, fn, "rb");
    if (!chan) {
        Jsi_LogError("open failed");
        return JSI_ERROR;
    }
    while ((n=Jsi_Read(chan, buf, BUFSIZ)) > 0) {
        b = (char*)Jsi_Realloc(b, siz+n);
        memcpy(b+siz, buf, n);
        siz += n;
    }
    Jsi_Close(chan);
    rc = B64Decode(interp, b, siz, ret);
    Jsi_Free(b);
    return rc;
}

static int B64EncodeFileCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char *b = NULL, buf[BUFSIZ];
    Jsi_Value *fn = Jsi_ValueArrayIndex(interp, args, 0);
    int rc, n, siz = 0;
    Jsi_Channel chan = Jsi_Open(interp, fn, "rb");
    if (!chan) {
        Jsi_LogError("open failed");
        return JSI_ERROR;
    }
    while ((n=Jsi_Read(chan, buf, BUFSIZ)) > 0) {
        b = (char*)Jsi_Realloc(b, siz+n);
        memcpy(b+siz, buf, n);
        siz += n;
    }
    Jsi_Close(chan);
    rc = B64Encode(interp, b, siz, ret);
    Jsi_Free(b);
    return rc;
}


static int SysB64EncodeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    int n;
    if (arg && Jsi_GetBoolFromValue(interp, arg, &n) == JSI_OK && n)
        return B64EncodeFileCmd(interp, args, _this, ret, funcPtr);
    return B64EncodeCmd(interp, args, _this, ret, funcPtr);
}


static int SysB64DecodeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    int n;
    if (arg && Jsi_GetBoolFromValue(interp, arg, &n) == JSI_OK && n)
        return B64DecodeFileCmd(interp, args, _this, ret, funcPtr);
    return B64DecodeCmd(interp, args, _this, ret, funcPtr);
}

static int SysMd5Cmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    const char *str = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    int n;
    if (arg && Jsi_GetBoolFromValue(interp, arg, &n) == JSI_OK && n)
        return jsi_Md5File(interp, args, _this, ret, funcPtr);
    char buf[33];
    Jsi_Md5Str(interp, buf, str, -1);
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static int SysSha1Cmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    int ilen;
    const char *str = Jsi_ValueArrayIndexToStr(interp, args, 0, &ilen);
    int n;
    if (arg && Jsi_GetBoolFromValue(interp, arg, &n) == JSI_OK && n)
        return jsi_Sha1File(interp, args, _this, ret, funcPtr);
    char buf[41];
    Jsi_Sha1Str(interp, buf, str, ilen);
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static int SysSha256Cmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *str = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    char buf[65];
    Jsi_Sha256Str(interp, buf, str, -1);
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static int _SysEnDecryptCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int decrypt)
{
    Jsi_Value *keyVal = Jsi_ValueArrayIndex(interp, args, 0);
    int ilen;
    const char *str = Jsi_ValueArrayIndexToStr(interp, args, 1, &ilen);
    char *key = Jsi_ValueString(interp, keyVal, NULL);
    if (!key) {
        Jsi_LogError("key must be a string");
        return JSI_ERROR;
    }
    Jsi_DString dStr = {};
    Jsi_DSAppendLen(&dStr, str, ilen);
    while (ilen<=4 || (ilen%4)) {
        Jsi_DSAppendLen(&dStr, "\0", 1);
        ilen++;
    }
    if (JSI_OK != Jsi_EncryptBuf(interp, key, (int*)Jsi_DSValue(&dStr), ilen/4, decrypt))
        return JSI_ERROR;
    Jsi_ValueMakeDStringObject(interp, ret, &dStr);
    return JSI_OK;
}

#define FN_decrypt JSI_INFO("\
Xxtea decrypt data using key string.")
static int SysDecryptCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _SysEnDecryptCmd(interp, args, _this, ret, funcPtr, 1);
}

#define FN_encrypt JSI_INFO("\
Xxtea encryption data using key string.  \
Input data is first padded with NULLs making the length a multiple of 4. \
If this is a problem, data can be b64encoded.")
static int SysEncryptCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return _SysEnDecryptCmd(interp, args, _this, ret, funcPtr, 0);
}

/* Commands only visible at the toplevel. */

static Jsi_CmdSpec cDataCmds[] = {
    { "get",      cDataGetCmd,       2,  3, "name:string, index:number, field:string=void", .help="Return struct data for one or all fields", .retType=(uint)JSI_TT_ANY },
    { "info",     cDataInfoCmd,      1,  1, "name:string", .help="Return struct details", .retType=(uint)JSI_TT_OBJECT },
    { "names",    cDataNamesCmd,     0,  0, "",     .help="Return name of all defined Cdata items", .retType=(uint)JSI_TT_ARRAY },
    { "schema",   cDataSchemaCmd,    1,  1, "name:string", .help="Return a DB schema compatible with struct", .retType=(uint)JSI_TT_STRING },
    { "schemaMd5",cDataSchemaMd5Cmd, 1,  1, "name:string", .help="Calculate md5 of schema (ignores comments from .help)", .retType=(uint)JSI_TT_STRING },
    { "struct",   cDataStructCmd,    1,  1, "name:string", .help="Return the C struct for mydata", .retType=(uint)JSI_TT_STRING },
    { "set",      cDataSetCmd,       3,  3, "name:string, index:number, dataobj:string|object|null,", .help="Update/get struct data", .retType=(uint)JSI_TT_ANY },
    { "size",     cDataSizeCmd,      1,  1, "name:string", .help="Return allocated size of data array", .retType=(uint)JSI_TT_NUMBER },
    { NULL,   .help="Access commands for Cdata" }
};

static Jsi_CmdSpec consoleCmds[] = {
    { "input",  consoleInputCmd,    0,  0, "", .help="Read input from the console", .retType=(uint)JSI_TT_STRING },
    { "log",    consoleLogCmd,      1, -1, "val, ...", .help="Output one or more values to stderr", .info=FN_conslog, .retType=(uint)JSI_TT_VOID },
    { "puts",   SysPutsCmd,         1, -1, "val, ...", .help="Output one or more values to stdout", .info=FN_puts, .retType=(uint)JSI_TT_VOID },
    { NULL,   .help="Console input and output" }
};

static Jsi_CmdSpec eventCmds[] = {
    { "clearInterval",clearIntervalCmd, 1,  1, "id:number", .help="Delete an event (created with setInterval/setTimeout)", .retType=(uint)JSI_TT_VOID },
    { "info",       eventInfoCmd,       1,  1, "id:number", .help="Return info for the given event id", .retType=(uint)JSI_TT_OBJECT },
    { "names",      eventInfoCmd,       0,  0, "", .help="Return list event ids (created with setTimeout/setInterval)", .retType=(uint)JSI_TT_ARRAY },
    { "setInterval",setIntervalCmd,     2,  2, "callback:function, millisecs:number", .help="Setup recurring function to run every given millisecs", .retType=(uint)JSI_TT_NUMBER },
    { "setTimeout", setTimeoutCmd,      2,  2, "callback:function, millisecs:number", .help="Setup function to run after given millisecs", .retType=(uint)JSI_TT_NUMBER },
    { "update",     SysUpdateCmd,       0,  1, "options:number|object=void", .help="Execute interval/timer tasks",.info=FN_update, .opts=UpdateOptions, .retType=(uint)JSI_TT_NUMBER },
    { NULL,   .help="Event management" }
};

static Jsi_CmdSpec infoCmds[] = {
    { "argv0",      InfoArgv0Cmd,       0,  0, "", .help="Return initial start script file name", .retType=(uint)JSI_TT_STRING },
    { "cmds",       InfoCmdsCmd,        0,  2, "val:string|regexp='*', all:boolean=false", .help="Return details or list of matching commands", .retType=(uint)JSI_TT_ARRAY|JSI_TT_OBJECT },
    { "completions",InfoCompletionsCmd,  1,  3, "str:string, start:number=0, end:number=void", .help="Return command completions for string", .retType=(uint)JSI_TT_ARRAY },
    { "data",       InfoDataCmd,        0,  1, "val:string|regexp|object=void", .help="Return list of matching data (non-functions)", .info=FN_infodata, .retType=(uint)JSI_TT_ARRAY },
    { "error",      InfoErrorCmd,       0,  0, "", .help="Return file and line number of error (used inside catch", .retType=(uint)JSI_TT_OBJECT },
    { "event",      InfoEventCmd,       0,  1, "id:number=void", .help="List events or info for 1 event (setTimeout/setInterval)", .info=FN_infoevent, .retType=(uint)JSI_TT_ARRAY|JSI_TT_OBJECT },
    { "executable", InfoExecutableCmd,  0,  0, "", .help="Return name of executable", .retType=(uint)JSI_TT_STRING },
    { "execZip",    InfoExecZipCmd,     0,  0, "", .help="If executing a .zip file, return file name", .retType=(uint)JSI_TT_STRING },
    { "funcs",      InfoFuncsCmd,       0,  1, "string|regexp|object=void", .help="Return details or list of matching functions", .retType=(uint)JSI_TT_ARRAY|JSI_TT_OBJECT },
    { "interp",     jsi_InterpInfo,     0,  1, "interp:userobj=void", .help="Return info on given or current interp", .retType=(uint)JSI_TT_OBJECT },
    { "isMain",     InfoIsMainCmd,      0,  0, "", .help="Return true if current script was the main script invoked from command-line", .retType=(uint)JSI_TT_BOOL },
    { "keywords",   InfoKeywordsCmd,    0,  0, "", .help="Return list of reserved jsi keywords", .retType=(uint)JSI_TT_ARRAY },
    { "lookup",     InfoLookupCmd,      1,  1, "name:string", .help="Given string name, lookup and return value (eg. function).", .retType=(uint)JSI_TT_ANY },
    { "named",      InfoNamedCmd,       0,  1, "name:string", .help="Returns command names for builtin Objects (eg. 'File', 'Sqlite') or their new'ed names", .retType=(uint)JSI_TT_ARRAY },
//    { "now",        DateNowCmd,         0,  0, "",  .help="Return current time (in ms) since unix epoch (ie. 1970)", .retType=(uint)JSI_TT_NUMBER },
    { "options",    InfoOptionsCmd,     0,  1, "ctype:boolean=false", .help="Return Option type name, or with true the C type)", .retType=(uint)JSI_TT_STRING },
    { "platform",   InfoPlatformCmd,    0,  0, "", .help="N/A. Returns general platform information for JSI", .retType=(uint)JSI_TT_OBJECT  },
    { "script",     InfoScriptCmd,      0,  1, "func:function|regexp=void", .help="Get current script file name, or file containing function", .retType=(uint)JSI_TT_STRING|JSI_TT_OBJECT },
    { "scriptDir",  InfoScriptDirCmd,   0,  0, "", .help="Get directory of current script", .retType=(uint)JSI_TT_STRING },
    { "vars",       InfoVarsCmd,        0,  1, "val:string|regexp|object=void", .help="Return details or list of matching variables", .info=FN_infovars, .retType=(uint)JSI_TT_ARRAY|JSI_TT_OBJECT },
    { "version",    InfoVersionCmd,     0,  1, "getObj:boolean=false", .help="Return JSI version double (or object with true arg)", .retType=(uint)JSI_TT_NUMBER|JSI_TT_OBJECT  },
    { NULL,  .help="Commands for inspecting internal state information in JSI"  }
};

static Jsi_CmdSpec sysCmds[] = {
    { "assert",     jsi_AssertCmd,   2,  2, "expr:boolean|number|function, msg:string",  .help="Generate an with msg error if expr evaluates to false/zero, and return arg 1", .retType=(uint)JSI_TT_ANY },
    { "b64decode",  SysB64DecodeCmd, 1,  2, "val:string, isfile:boolean=false",.help="Decode string/file", .retType=(uint)JSI_TT_STRING },
    { "b64encode",  SysB64EncodeCmd, 1,  2, "val:string, isfile:boolean=false",.help="Encode string/file", .retType=(uint)JSI_TT_STRING },
    { "clearInterval",clearIntervalCmd, 1,  1, "id:number", .help="Delete event id returned from setInterval/setTimeout/info.events()", .retType=(uint)JSI_TT_VOID },
    { "decodeURI",  DecodeURICmd,       1,  1, "val:string", .help="Decode an HTTP URL", .retType=(uint)JSI_TT_STRING },
    { "decrypt",    SysDecryptCmd,   2,  2, "key:string, val:string", .help="Return decryption of string", .info=FN_decrypt, .retType=(uint)JSI_TT_STRING },
    { "encodeURI",  EncodeURICmd,       1,  1, "val:string", .help="Encode an HTTP URL", .retType=(uint)JSI_TT_STRING },
    { "encrypt",    SysEncryptCmd,   2,  2, "key,string, val:string", .help="Return encryption of string", .info=FN_encrypt, .retType=(uint)JSI_TT_STRING },
    { "exec",       SysExecCmd,      1,  2, "val:string, options:null|string|object=void", .help="Execute an OS command", .info=FN_exec, .retType=(uint)JSI_TT_ANY, .opts=ExecOptions},
    { "exit",       SysExitCmd,      0,  1, "code:number=0", .help="Exit the current interpreter", .retType=(uint)JSI_TT_VOID },
    { "format",     SysFormatCmd,    1, -1, "format:string, ...", .help="Implement printf style formatting", .retType=(uint)JSI_TT_STRING },
    { "getenv",     SysGetEnvCmd,    0,  1, "name:string=void", .help="Get one or all environment", .retType=(uint)JSI_TT_STRING|JSI_TT_OBJECT  },
    { "getpid",     SysGetPidCmd,    0,  1, "", .help="Get process id", .retType=(uint)JSI_TT_NUMBER },
    { "getppid",    SysGetPpidCmd,   0,  1, "", .help="Get parent process id", .retType=(uint)JSI_TT_NUMBER },
    { "isFinite",   isFiniteCmd,        1,  1, "val:string", .help="Return true if is a finite number", .retType=(uint)JSI_TT_BOOL },
    { "isNaN",      isNaNCmd,           1,  1, "val:string", .help="Return true if not a number", .retType=(uint)JSI_TT_BOOL },
    { "md5",        SysMd5Cmd,       1,  2, "val:string, isfile:boolean=false",.help="Compute md5 hash of string/file", .retType=(uint)JSI_TT_STRING },
    { "noOp",       SysNoOpCmd,      0,  0, "", .help="Do nothing; useful for measuring function call overhead", .retType=(uint)JSI_TT_VOID },
    { "parseInt",   parseIntCmd,        1,  2, "val:any, base:number=10", .help="Convert string to an integer", .retType=(uint)JSI_TT_NUMBER },
    { "parseFloat", parseFloatCmd,      1,  1, "val", .help="Convert string to a double", .retType=(uint)JSI_TT_NUMBER },
    { "puts",       SysPutsCmd,      1, -1, "val, ...", .help="Output one or more values to stdout", .info=FN_puts, .retType=(uint)JSI_TT_VOID },
    { "quote",      quoteCmd,           1,  1, "val:string", .help="Return quoted string", .retType=(uint)JSI_TT_STRING },
    { "setenv",     SysSetEnvCmd,    2,  2, "name:string, value:string", .help="Set an environment var"  },
    { "sha1",       SysSha1Cmd,      1,  2, "val:string, isfile:boolean=false", .help="Return sha1 of string/file", .retType=(uint)JSI_TT_STRING },
    { "sha256",     SysSha256Cmd,    1,  2, "val:string, isfile:boolean=false", .help="Return sha256 of string/file", .retType=(uint)JSI_TT_STRING },
    { "sleep",      SysSleepCmd,     0,  1, "secs:number=1.0",  .help="sleep for N milliseconds, minimum .001", .retType=(uint)JSI_TT_VOID },
    { "setInterval",setIntervalCmd,     2,  2, "callback:function, ms:number", .help="Setup recurring function to run every given millisecs", .retType=(uint)JSI_TT_NUMBER },
    { "setTimeout", setTimeoutCmd,      2,  2, "callback:function, ms:number", .help="Setup function to run after given millisecs", .retType=(uint)JSI_TT_NUMBER },
    { "source",     SysSourceCmd,    1, 2, "val:string|array, options:object=void",  .help="Load and evaluate source files", .opts=SourceOptions, .retType=(uint)JSI_TT_VOID},
    { "strftime",   DateStrftimeCmd, 1,  2, "num:number, options:string|object=void",  .help="Format numeric time (in ms) to a string", .opts=DateOptions,.info=FN_strftime, .retType=(uint)JSI_TT_STRING },
    { "strptime",   DateStrptimeCmd, 1,  2, "val:string, options:string|object=void",  .help="Parse time from string and return time (in ms) since 1970", .opts=DateOptions, .retType=(uint)JSI_TT_NUMBER },
    { "times",      SysTimesCmd,     1,  2, "callback:function, count:number=1", .help="Call function count times and return execution time in microseconds", .retType=(uint)JSI_TT_NUMBER },
    { NULL, .help="Builtin system commands. All are callable from the either the top level or as System.XXX()" }
};

int jsi_CmdsInit(Jsi_Interp *interp)
{
    interp->console = Jsi_CommandCreateSpecs(interp, "console", consoleCmds, NULL, JSI_CMDSPEC_NOTOBJ);
    Jsi_IncrRefCount(interp, interp->console);
    Jsi_ValueInsertFixed(interp, interp->console, "args", interp->args);
        
    Jsi_CommandCreateSpecs(interp, "",       sysCmds,    NULL, JSI_CMDSPEC_NOTOBJ);
    Jsi_CommandCreateSpecs(interp, "System", sysCmds,    NULL, JSI_CMDSPEC_NOTOBJ);
    Jsi_CommandCreateSpecs(interp, "Info",   infoCmds,   NULL, JSI_CMDSPEC_NOTOBJ);
    Jsi_CommandCreateSpecs(interp, "Cdata",  cDataCmds,  NULL, JSI_CMDSPEC_NOTOBJ);
    Jsi_CommandCreateSpecs(interp, "Event",  eventCmds,  NULL, JSI_CMDSPEC_NOTOBJ);
    return JSI_OK;
}
#endif
