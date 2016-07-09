#ifndef JSI_LITE_ONLY
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#ifndef JSI_AMALGAMATION
/* Adapted to update so "signal.handle(func) gets called back like setInterval/setTimeout */
#include "jsiInt.h"
#endif
#ifndef JSI_OMIT_SIGNAL

enum { SIGNAL_ACTION_IGNORE=-1,  SIGNAL_ACTION_DEFAULT=0, SIGNAL_ACTION_HANDLE=1, MAX_SIGNALS = (sizeof(Jsi_Wide) * 8)};

static Jsi_Wide *sigloc;
static Jsi_Wide sigsblocked;
static struct sigaction *saOldPtr[MAX_SIGNALS] = {NULL};
static struct sigaction saOld[MAX_SIGNALS];
static int signal_handling[MAX_SIGNALS] = {0};

/* Make sure to do this as a wide, not int */
#define sig_to_bit(SIG) ((Jsi_Wide)1 << (SIG))

static void signal_handler(int sig)
{
    /* We just remember which signals occurred. Jsi_Eval() will
     * notice this as soon as it can and throw an error
     */
    *sigloc |= sig_to_bit(sig);
}

static void signal_ignorer(int sig)
{
    /* We just remember which signals occurred */
    sigsblocked |= sig_to_bit(sig);
}

#define CASESIGNAL(name) case SIG##name: return "SIG" #name;

const char *Jsi_SignalId(int sig)
{
    switch (sig) {
    CASESIGNAL(ABRT); CASESIGNAL(ALRM); CASESIGNAL(BUS);  CASESIGNAL(CONT);   CASESIGNAL(FPE);
    CASESIGNAL(HUP);  CASESIGNAL(ILL);  CASESIGNAL(INT);  CASESIGNAL(KILL);   CASESIGNAL(PIPE);
    CASESIGNAL(PROF); CASESIGNAL(QUIT); CASESIGNAL(SEGV); CASESIGNAL(STOP);   CASESIGNAL(SYS);
    CASESIGNAL(TERM); CASESIGNAL(TRAP); CASESIGNAL(TSTP); CASESIGNAL(TTIN);   CASESIGNAL(TTOU);
    CASESIGNAL(URG);  CASESIGNAL(USR1); CASESIGNAL(USR2); CASESIGNAL(VTALRM); CASESIGNAL(WINCH);
    CASESIGNAL(XCPU);
    CASESIGNAL(XFSZ);
#ifdef SIGPWR
    CASESIGNAL(PWR);
#endif
#ifdef SIGCLD
    CASESIGNAL(CLD);
#endif
#ifdef SIGEMT
    CASESIGNAL(EMT);
#endif
#ifdef SIGLOST
    CASESIGNAL(LOST);
#endif
#ifdef SIGIO
    CASESIGNAL(IO);
#endif
#if defined(SIGPOLL) && (SIGPOLL != SIGIO)
    CASESIGNAL(POLL);
#endif
#ifdef SIGINFO
    CASESIGNAL(INFO);
#endif
    }
    return NULL;
}

const char *Jsi_SignalName(int sig)
{
    const char *cp;
#ifdef HAVE_SYS_SIGLIST
    if (sig >= 0 && sig < NSIG) {
        return sys_siglist[sig];
    }
#endif
    cp = Jsi_SignalId(sig);
    if (cp == NULL)
        cp = "unknown signal";
    return cp;
}

/**
 * Given the name of a signal, returns the signal value if found,
 * or returns -1 (and sets an error) if not found.
 * We accept -SIGINT, SIGINT, INT or any lowercase version or a number,
 * either positive or negative.
 */
static int find_signal_by_name(Jsi_Interp *interp, const char *name)
{
    int i;
    const char *pt = name;

    /* Remove optional - and SIG from the front of the name */
    if (*pt == '-') {
        pt++;
    }
    if (strncasecmp(name, "sig", 3) == 0) {
        pt += 3;
    }
    if (isdigit(UCHAR(pt[0]))) {
        i = atoi(pt);
        if (i > 0 && i < MAX_SIGNALS) {
            return i;
        }
    }
    else {
        for (i = 1; i < MAX_SIGNALS; i++) {
            /* Jsi_SignalId() returns names such as SIGINT, and
             * returns "unknown signal id" if unknown, so this will work
             */
            if (Jsi_Strncasecmp(Jsi_SignalName(i) + 3, pt, -1) == 0) {
                return i;
            }
        }
    }
    return -1;
}

void jsi_SignalClear(Jsi_Interp *interp, int sigNum) {
    Jsi_Wide nsig = sig_to_bit(sigNum);
    (*sigloc) &= ~nsig;
}

int jsi_SignalIsSet(Jsi_Interp *interp, int sigNum) {
    return (((*sigloc) & sig_to_bit(sigNum)) != 0);
}

static void jsi_SignalSet(Jsi_Interp *interp, int sig) {
    struct sigaction sa;

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = signal_handler;

    if (SIGNAL_ACTION_HANDLE != signal_handling[sig]) {

        if (signal_handling[sig] == SIGNAL_ACTION_DEFAULT) {
            saOldPtr[sig] = &saOld[sig];
            sigaction(sig, &sa, saOldPtr[sig]);
        }
        else {
            sigaction(sig, &sa, 0);
        }
        signal_handling[sig] = SIGNAL_ACTION_HANDLE;
    }
}

static int SignalSub(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int action)
{

    struct sigaction sa;
    int i, argc = Jsi_ValueGetLength(interp, args);
    
    if (&interp->sigmask != sigloc) {
        Jsi_LogWarn("not primary interp");
        return JSI_OK;
    }
    
    if (argc == 0) {
        Jsi_Obj *nobj = Jsi_ObjNew(interp);
        int m;
        Jsi_ValueMakeArrayObject(interp, ret, nobj);
        for (i = 0, m = 0; i < MAX_SIGNALS; i++) {
            if (signal_handling[i] == action) {
                /* Add signal name to the list  */
                Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringKey(interp, Jsi_SignalName(i)), m++);
            }
        }
        return JSI_OK;
    }

    /* Catch all the signals we care about */
    if (action != SIGNAL_ACTION_DEFAULT) {
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        if (action == SIGNAL_ACTION_HANDLE) {
            sa.sa_handler = signal_handler;
        }
        else {
            sa.sa_handler = signal_ignorer;
        }
    }

    /* Iterate through the provided signals */
    for (i = 0; i < argc; i++) {
        char *sstr;
        int sig = find_signal_by_name(interp, sstr=Jsi_ValueArrayIndexToStr(interp,args,i, NULL));

        if (sig < 0) {
            Jsi_LogError("unknown signal: %s", sstr);
            return JSI_ERROR;
        }
        if (action != signal_handling[sig]) {
            /* Need to change the action for this signal */
            switch (action) {
                case SIGNAL_ACTION_HANDLE:
                case SIGNAL_ACTION_IGNORE:
                    if (signal_handling[sig] == SIGNAL_ACTION_DEFAULT) {
                        saOldPtr[sig] = &saOld[sig];
                        sigaction(sig, &sa, saOldPtr[sig]);
                    }
                    else {
                        sigaction(sig, &sa, 0);
                    }
                    break;

                case SIGNAL_ACTION_DEFAULT:
                    /* Restore old handler */
                    sigaction(sig, saOldPtr[sig], NULL);
            }
            signal_handling[sig] = action;
        }
    }

    return JSI_OK;
}

static int SignalHandleCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return SignalSub(interp, args, _this, ret, funcPtr, SIGNAL_ACTION_HANDLE);
}

static int SignalIgnoreCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return SignalSub(interp, args, _this, ret, funcPtr, SIGNAL_ACTION_IGNORE);
}

static int SignalDefaultCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return SignalSub(interp, args, _this, ret, funcPtr, SIGNAL_ACTION_DEFAULT);
}

static int SignalNamesCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Obj *nobj = Jsi_ObjNew(interp);
    int i, m;
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    for (i=0, m=0; i<MAX_SIGNALS; i++) {
        const char *nam;
        nam = Jsi_SignalId(i);
        if (nam)
            Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringKey(interp, nam), m++);
    }
    return JSI_OK;
}

static int SignalAlarmCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *sv = Jsi_ValueArrayIndex(interp, args, 0);
    uint rc;
    Jsi_Number dtim;
    
    if (Jsi_GetNumberFromValue(interp, sv, &dtim) != JSI_OK) {
        Jsi_LogError("bad time");
        return JSI_ERROR;
    }
    if (dtim<1)
        rc = ualarm(dtim * 1e6, 0);
    else
        rc = alarm((uint)dtim);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)rc);
    return JSI_OK;
}

static int SignalKillCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *vpid = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *sv = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Number npid;
    int rc, sigNum = SIGTERM;
    if (Jsi_GetNumberFromValue(interp, vpid, &npid) != JSI_OK) {
        Jsi_LogError("bad pid");
        return JSI_ERROR;
    }
    if (sv) {
        if (sv->vt == JSI_VT_NUMBER)
            sigNum = (int)sv->d.num;
        else {
            char *ts = Jsi_ValueArrayIndexToStr(interp, args, 1, NULL);
            if ((sigNum = find_signal_by_name(interp, ts)) < 0) {
                Jsi_LogError("bad signal: %s", ts);
                return JSI_ERROR;
            }
        }
    }
    if (sigNum < 0 || sigNum >= MAX_SIGNALS) {
        Jsi_LogError("bad signal: %d", sigNum);
        return JSI_ERROR;
    }
    rc = kill((int)npid, sigNum);
    if (rc != 0) {
        Jsi_LogError("kill failure");
        return JSI_ERROR;
    }
    return JSI_OK;
}

static int SignalCallbackCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int isNew;
    Jsi_Event *ev;
    uint id;
    int sigNum;
    Jsi_Value *fv = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *sv = Jsi_ValueArrayIndex(interp, args, 1);
    
    if (!Jsi_ValueIsFunction(interp, fv)) {
        Jsi_LogError("expected function");
        return JSI_ERROR;
    }
    if (sv && sv->vt == JSI_VT_NUMBER)
        sigNum = (int)sv->d.num;
    else {
        char *ts = Jsi_ValueArrayIndexToStr(interp, args, 1, NULL);
        if ((sigNum = find_signal_by_name(interp, ts)) < 0) {
            Jsi_LogError("expected signal");
            return JSI_ERROR;
        }
    }
    if (sigNum < 0 || sigNum >= MAX_SIGNALS) {
        Jsi_LogError("unknown signal: %d", sigNum);
        return JSI_ERROR;
    }
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
        ev->sigNum = sigNum;
        ev->evType = JSI_EVENT_SIGNAL;
        Jsi_HashValueSet(hPtr, ev);
        break;
    }
    jsi_SignalSet(interp, sigNum);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)id);
    return JSI_OK;
}


static Jsi_CmdSpec signalCmds[] = {
    { "alarm",      SignalAlarmCmd,       1, 1, "secs", .help="Setup alarm in seconds", .retType=(uint)JSI_TT_NUMBER },
    { "callback",   SignalCallbackCmd,    2, 2, "func:function, sig:number|string",  .help="Setup callback handler for signal", .retType=(uint)JSI_TT_NUMBER },
    { "default",    SignalDefaultCmd,     0,-1, "sig:number|string=void, ...", .help="Set named signals to default action", .retType=(uint)JSI_TT_ARRAY },
    { "handle",     SignalHandleCmd,      0,-1, "sig:number|string=void, ...", .help="Set named signals to handle action", .retType=(uint)JSI_TT_ANY },
    { "ignore",     SignalIgnoreCmd,      0,-1, "sig:number|string=void, ...", .help="Set named signals to ignore action", .retType=(uint)JSI_TT_ANY },
    { "kill",       SignalKillCmd,        1, 2, "pid:number, sig:number|string='SIGTERM'", .help="Send signal to process id", .retType=(uint)JSI_TT_VOID },
    { "names",      SignalNamesCmd,       0, 0, "", .help="Return names of all signals", .retType=(uint)JSI_TT_ARRAY },
    { NULL, .help="Commands for handling unix signals" }
};

int jsi_SignalInit(Jsi_Interp *interp)
{
    static int isinit = 0;
    if (!isinit) {
        sigloc = &interp->sigmask;
        isinit = 1;
    }
    Jsi_CommandCreateSpecs(interp, "Signal",    signalCmds,    NULL, JSI_CMDSPEC_NOTOBJ);
    return JSI_OK;
}

#endif
#endif
