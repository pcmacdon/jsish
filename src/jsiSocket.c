#ifndef JSI_LITE_ONLY

#if (defined(JSI__SOCKET) && JSI__SOCKET==1)
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#ifndef __WIN32
#include <time.h>
#include <sys/time.h>
#endif /* !__WIN32 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <fcntl.h>

#ifdef WIN32
#define _GET_TIME_OF_DAY_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stddef.h>

//#include "sock-w32.h"

#else /* WIN32 */
#include <syslog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#endif /* WIN32 */

#include <signal.h>
#include <errno.h>

typedef enum {  PSS_DEAD, PSS_HTTP, PSS_CONNECTED, PSS_RECV, PSS_SENT, PSS_SENDERR } pss_state;

typedef enum {
    SOCK_CALLBACK_RECEIVE,
    SOCK_CALLBACK_WRITEABLE,
    SOCK_CALLBACK_CLOSED,
    SOCK_CALLBACK_OPEN,
} callback_reasons;

typedef struct { /* Interp wide data. */
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_Interp *interp;
    Jsi_Hash *wsTable;
    int wIdx;
} SocketObjInterpData;

//#define JSI__IPV6
typedef union {
    struct sockaddr_in sin;
#ifdef JSI__IPV6
    struct sockaddr_in6 sin6;
#endif
} SockAddrAll;

struct SocketObj;

typedef struct SocketPss { /* Per session connection to server */
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    struct SocketObj *cmdPtr;
    pss_state state;
    int sentCnt, recvCnt, sentErrCnt, eventCnt;
    time_t sentLast, recvLast, sentErrLast, eventLast;
    Jsi_HashEntry *hPtr;
    Jsi_Stack *stack;
    int id;
    int fd;
    int offset;
    bool echo;
    SockAddrAll sa;
    SockAddrAll recvAddr;
    Jsi_Value *udata;
    uint siLen;
} SocketPss;

typedef struct SockSendOpts {
    bool noAsync;
} SockSendOpts;

typedef struct SocketObj { /* Per server (or client) data. */
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    SocketObjInterpData *interpData;
    Jsi_Interp *interp;
    Jsi_Hash *pssTable;
    Jsi_Value *onRecv;
    Jsi_Value *onCloseLast;
    Jsi_Value *onClose;
    Jsi_Value *onOpen;
    Jsi_Value *defaultUrl;
    Jsi_Value *interface;
    Jsi_Value *address;
    char client_name[128];
    char client_ip[128];
    int idx;
    int port;
    int srcPort;
    Jsi_Number timeout;
    Jsi_Value *srcAddress;
    Jsi_Value *mcastAddMember;
    Jsi_Value *mcastInterface, *udata;
    int family;
    int saLen;
    char *iface;
    bool quiet;
    unsigned int oldus;
    bool udp, client, noUpdate, noAsync, reuse, broadcast, keepalive, mcastNoLoop, echo;
    int hasOpts;
    int debug;
    int maxConnects;
    int connectCnt;
    int createCnt;
    int ttl;
    int mcastTtl;
    uint64_t recvTimeout;
    uint64_t sendTimeout;
    int8_t tos;
    time_t createLast;
    time_t startTime;
    struct timeval tv;
    char *cmdName;
    bool noConfig;
    bool deleted;
    Jsi_Event *event;
    Jsi_Obj *fobj;
    int objId;
    char *ssl_cert_filepath;
    char *ssl_private_key_filepath;
    char *cl_host;
    char *cl_origin;
    int maxfd;
    fd_set exceptSet, readSet, writeSet;
    int sendFlags;
    int recvFlags;
    SocketPss stats; // Server/non-async client pss.
} SocketObj;

static Jsi_RC sock_handler(SocketObj *cmdPtr, callback_reasons reason, SocketPss *pss, char *inPtr, size_t len);
static Jsi_RC jsk_Service(SocketObj *cmdPtr);
static Jsi_RC socketObjFree(Jsi_Interp *interp, void *data);
static bool socketObjIsTrue(void *data);
static bool socketObjEqual(void *data1, void *data2);
static Jsi_RC ValueToSockAddrOpt(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags);
static Jsi_RC SockAddrOptToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record, Jsi_Wide flags);

static Jsi_OptionCustom socketAddOption = {
    .name="sockAddr", .parseProc=ValueToSockAddrOpt, .formatProc=SockAddrOptToValue
};

static Jsi_OptionSpec SPSOptions[] =
{
    JSI_OPT(BOOL,       SocketPss, echo,         .help="LogInfo outputs all socket Send/Recv messages"),
    JSI_OPT(INT,        SocketPss, eventCnt,     .help="Number of events of any type"),
    JSI_OPT(TIME_T,     SocketPss, eventLast,    .help="Time of last event of any type"),
    JSI_OPT(CUSTOM,     SocketPss, recvAddr,     .help="Incoming port and address", .flags=0, .custom=&socketAddOption),
    JSI_OPT(INT,        SocketPss, recvCnt,      .help="Number of recieves"),
    JSI_OPT(TIME_T,     SocketPss, recvLast,     .help="Time of last recv"),
    JSI_OPT(INT,        SocketPss, sentCnt,      .help="Number of sends"),
    JSI_OPT(TIME_T,     SocketPss, sentLast,     .help="Time of last send"),
    JSI_OPT(INT,        SocketPss, sentErrCnt,   .help="Number of sends"),
    JSI_OPT(TIME_T,     SocketPss, sentErrLast,  .help="Time of last sendErr"),
    JSI_OPT(OBJ,        SocketPss, udata,        .help="User data"),
    JSI_OPT_END(SocketPss, .help="Socket options for per-connection")
};

static Jsi_OptionSpec SockSendOptions[] =
{
    JSI_OPT(BOOL,   SockSendOpts, noAsync,  .help="Send is not async"),
    JSI_OPT_END(SockSendOpts, .help="Socket send options")
};

static Jsi_OptionSpec SockOptions[] =
{
    JSI_OPT(STRING, SocketObj, address,    .help="Client destination address (127.0.0.0)", jsi_IIOF ),
    JSI_OPT(BOOL,   SocketObj, broadcast,  .help="Enable broadcast", jsi_IIOF),
    JSI_OPT(BOOL,   SocketObj, client,     .help="Enable client mode", jsi_IIOF),
    JSI_OPT(INT,    SocketObj, connectCnt, .help="Counter for number of active connections", jsi_IIRO),
    JSI_OPT(TIME_T, SocketObj, createLast, .help="Time of last create", jsi_IIRO),
    JSI_OPT(INT,    SocketObj, debug,      .help="Debugging level"),
    JSI_OPT(BOOL,   SocketObj, echo,       .help="LogInfo outputs all socket Send/Recv messages"),
    JSI_OPT(STRING, SocketObj, interface,  .help="Interface for server to listen on, eg. 'eth0' or 'lo'", jsi_IIOF),
    JSI_OPT(BOOL,   SocketObj, keepalive,  .help="Enable keepalive", jsi_IIOF),
    JSI_OPT(INT,    SocketObj, maxConnects,.help="In server mode, max number of client connections accepted"),
    JSI_OPT(STRING, SocketObj, mcastAddMember, .help="Multicast add membership: address/interface ('127.0.0.1/0.0.0.0')", jsi_IIOF ),
    JSI_OPT(STRING, SocketObj, mcastInterface, .help="Multicast interface address", jsi_IIOF ),
    JSI_OPT(BOOL,   SocketObj, mcastNoLoop,  .help="Multicast loopback disable", jsi_IIOF ),
    JSI_OPT(INT,    SocketObj, mcastTtl,   .help="Multicast TTL", jsi_IIOF ),
    JSI_OPT(BOOL,   SocketObj, noAsync,    .help="Send is not async", jsi_IIOF),
    JSI_OPT(BOOL,   SocketObj, noUpdate,   .help="Stop processing update events (eg. to exit)"),
    JSI_OPT(FUNC,   SocketObj, onClose,    .help="Function to call when connection closes", .flags=0, .custom=0, .data=(void*)"s:userobj|null, id:number"),
    JSI_OPT(FUNC,   SocketObj, onCloseLast,.help="Function to call when last connection closes. On object delete arg is null", .flags=0, .custom=0, .data=(void*)"s:userobj|null"),
    JSI_OPT(BOOL,   SocketObj, noConfig,   .help="Disable use of Socket.conf to change options after create", jsi_IIOF),
    JSI_OPT(FUNC,   SocketObj, onOpen,     .help="Function to call when connection opens", .flags=0, .custom=0, .data=(void*)"s:userobj, info:object"),
    JSI_OPT(FUNC,   SocketObj, onRecv,     .help="Function to call with recieved data", .flags=0, .custom=0, .data=(void*)"s:userobj, id:number, data:string"),
    JSI_OPT(INT,    SocketObj, port,       .help="Port for client dest or server listen", jsi_IIOF ),
    JSI_OPT(BOOL,   SocketObj, quiet,      .help="Suppress info messages", jsi_IIOF),
    JSI_OPT(UINT64, SocketObj, recvTimeout,.help="Timeout for receive, in microseconds", jsi_IIOF),
    JSI_OPT(UINT64, SocketObj, sendTimeout,.help="Timeout for send, in microseconds", jsi_IIOF),
    JSI_OPT(STRING, SocketObj, srcAddress, .help="Client source address", jsi_IIOF ),
    JSI_OPT(INT,    SocketObj, srcPort,    .help="Client source port", jsi_IIOF ),
    JSI_OPT(TIME_T, SocketObj, startTime,  .help="Time of start", jsi_IIRO),
    JSI_OPT(CUSTOM, SocketObj, stats,      .help="Statistical data", jsi_IIRO, .custom=Jsi_Opt_SwitchSuboption, .data=SPSOptions),
    JSI_OPT(NUMBER, SocketObj, timeout,    .help="Timeout value in seconds (0.5)", jsi_IIOF),
    JSI_OPT(INT8,   SocketObj, tos,        .help="Type-Of-Service value", jsi_IIOF ),
    JSI_OPT(INT,    SocketObj, ttl,        .help="Time-To-Live value", jsi_IIOF ),
    JSI_OPT(OBJ,    SocketObj, udata,      .help="User data"),
    JSI_OPT(BOOL,   SocketObj, udp,        .help="Protocol is udp", jsi_IIOF),
    JSI_OPT_END(SocketObj, .help="Socket options")
};

/* Scanning function */
static Jsi_RC ValueToSockAddrOpt(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags)
{
    Jsi_LogBug("UNIMPLEMENTED"); //TODO: finish
#if 0
    int n, *s = (SockAddrAll*)(((char*)record) + spec->offset)
    //int flags = (spec->flags&JSI_OPT_CUST_NOCASE?JSI_CMP_NOCASE:0);
    if (!si) 
        return Jsi_LogError("custom enum spec did not set data: %s", spec->name);
    if (inStr) {
        if (JSI_OK != Jsi_GetIndex(interp, (char*)inStr, list, "enum", flags, &n))
            return JSI_ERROR;
        *s = n;
        return JSI_OK;
    }
    if (JSI_OK != Jsi_ValueGetIndex(interp, inValue, list, "enum", flags, &n))
        return JSI_ERROR;
    *s = n;
#endif
    return JSI_OK;

}

/* Printing function. */
static Jsi_RC SockAddrOptToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record, Jsi_Wide flags)
{
    SockAddrAll* sin = (SockAddrAll*)(((char*)record) + spec->offset);
    Jsi_RC rc = JSI_OK;
   /* const char **list = spec->data;
    if (!list) 
        return Jsi_LogError("custom enum spec did not set data: %s", spec->name);*/
    if (outStr) {
        /*n = *s;
        Jsi_DSAppendLen(outStr, list[i], -1);*/
        return JSI_OK;
    }
#ifndef JSI_LITE_ONLY
    Jsi_DString dStr = {};
    Jsi_DSPrintf(&dStr, "{port: %d, address:\"%s\"}",
        ntohs(sin->sin.sin_port), inet_ntoa(sin->sin.sin_addr));
    rc = Jsi_JSONParse(interp, Jsi_DSValue(&dStr), outValue, 0);
    Jsi_DSFree(&dStr);
    return rc;
#endif
    return JSI_ERROR;

}


static int sockAddrSize(SockAddrAll* sa) {
#ifdef JSI__IPV6
    if (sa->sin.sin_family == AF_INET6)
        return sizeof(sa->sin6);
#endif
    return sizeof(sa->sin);
}

static void sockUpdateEvent(SocketObj* cmdPtr, SocketPss* pss) {
    pss->eventCnt++;
    cmdPtr->stats.eventCnt++;
    cmdPtr->stats.eventLast = pss->eventLast = time(NULL);
}

static SocketPss*
sockGetPss(SocketObj *cmdPtr, int fd, int create)
{
    if (fd == cmdPtr->stats.fd)
        return &cmdPtr->stats;
    Jsi_HashEntry *hPtr;
    SocketPss* pss;
    bool isNew;
    if (create)
        hPtr = Jsi_HashEntryNew(cmdPtr->pssTable, (void*)(long)fd, &isNew);
    else
        hPtr = Jsi_HashEntryFind(cmdPtr->pssTable, (void*)(long)fd);
    if (!hPtr)
        return NULL;
    if (create == 0 || isNew == 0) {
        pss = (SocketPss*)Jsi_HashValueGet(hPtr);
        SIGASSERT(pss, SOCKETPSS);
    } else {
        pss = (SocketPss*)Jsi_Calloc(1, sizeof(*pss));
        SIGINIT(pss, SOCKETPSS);
        pss->hPtr = hPtr;
        cmdPtr->connectCnt++;
        cmdPtr->createCnt++;
        cmdPtr->createLast = time(NULL);
        Jsi_HashValueSet(hPtr, pss);
        pss->cmdPtr = cmdPtr;
        pss->fd = fd;
        pss->state = PSS_CONNECTED;
        pss->id = ++cmdPtr->idx;
        pss->stack = Jsi_StackNew();
        pss->udata = Jsi_ValueNewObj(cmdPtr->interp, NULL);
        Jsi_IncrRefCount(cmdPtr->interp, pss->udata);
    }
    return pss;
}


static SocketPss *sockFindPss(SocketObj *cmdPtr, int id) {
    if (id == 0) 
        return &cmdPtr->stats;
    SocketPss *tpss = NULL;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    for (hPtr = Jsi_HashSearchFirst(cmdPtr->pssTable, &cursor);
        hPtr != NULL; hPtr = Jsi_HashSearchNext(&cursor)) {
        tpss = (SocketPss*)Jsi_HashValueGet(hPtr);
        SIGASSERT(tpss, SOCKETPSS);
        if (tpss->id == id)
            return tpss;
    }
    return NULL;
}

static Jsi_RC sockFreeStackPss(Jsi_Interp *interp, void *data) {
    if (!data) return JSI_OK;
    Jsi_Value *v = (Jsi_Value*)data;
    SIGASSERT(v, VALUE);
    Jsi_DecrRefCount(interp, v);
    return JSI_OK;
}

static void
sockDeletePss(SocketPss *pss)
{
    if (pss == &pss->cmdPtr->stats)
        return;
    if (pss->hPtr) {
        Jsi_HashValueSet(pss->hPtr, NULL);
        Jsi_HashEntryDelete(pss->hPtr);
        pss->hPtr = NULL;
    }
    Jsi_StackFreeElements(pss->cmdPtr->interp, pss->stack, sockFreeStackPss);
    Jsi_StackFree(pss->stack);
    Jsi_OptionsFree(pss->cmdPtr->interp, SPSOptions, pss, 0);
    pss->cmdPtr->connectCnt--;
    pss->state = PSS_DEAD;
    Jsi_Free(pss);
}

static Jsi_RC SocketConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
  
    if (!cmdPtr) 
        return Jsi_LogError("Apply in a non-sock object");
    Jsi_Value *opts = Jsi_ValueArrayIndex(interp, args, 0);
    if (cmdPtr->noConfig && opts && !Jsi_ValueIsString(interp, opts))
        return Jsi_LogError("Socket conf() is disabled for set");
    return Jsi_OptionsConf(interp, SockOptions, cmdPtr, opts, ret, 0);

}

static Jsi_RC sockGetPssId(SocketObj* cmdPtr, Jsi_Value* darg, int *idPtr) {
    Jsi_Interp *interp = cmdPtr->interp;
    Jsi_Number dnum = 0;
    int id;
    if (Jsi_ValueGetNumber(interp, darg, &dnum) != JSI_OK) 
        return Jsi_LogError("invalid id");
    id = (int)dnum;
    if (id < 0 && cmdPtr->client)
        id = 0;
    if (id > 0 && cmdPtr->client) 
        return Jsi_LogError("invalid id");
    *idPtr = id;
    return JSI_OK;
}

#define FN_socksend JSI_INFO("\
Send a message to a (or all if -1) connection.")

static Jsi_RC SocketSendCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    Jsi_RC rc = JSI_OK;
    if (!cmdPtr) 
        return Jsi_LogError("Apply in a non-sock object");
    SIGASSERT(cmdPtr, SOCKET);
    SocketPss *pss;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    int aLen = 0, id = -1;
    char *str = Jsi_ValueString(interp, arg, &aLen);
    SockSendOpts opts = {};
    int argc = Jsi_ValueGetLength(interp, args);
    if (argc<2 || str == NULL || !Jsi_ValueIsString(interp, arg)) 
        return Jsi_LogError("expected string");
    
    if (str == NULL || aLen <= 0) return JSI_OK;
    //Jsi_DString eStr = {};
    Jsi_Value *arg1 = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_ValueIsNumber(interp, arg1)) {
        if (sockGetPssId(cmdPtr, arg1, &id) != JSI_OK)
            return JSI_ERROR;
    } else if (!Jsi_ValueIsObjType(interp, arg1, JSI_OT_OBJECT)) 
        return Jsi_LogError("expected int id or options object");
    Jsi_Value *arg2 = Jsi_ValueArrayIndex(interp, args, 2);
    if (argc>2 && arg2 &&
        Jsi_OptionsProcess(interp, SockSendOptions, &opts, arg, 0) < 0)
        return JSI_ERROR;
   /* if (cmdPtr->udp) {
        Jsi_IncrRefCount(interp, arg);
        str = Jsi_ValueString(interp, arg, &aLen);
        int rc = sendto(cmdPtr->stats.fd, str, aLen, 0, &cmdPtr->stats.sa.sa, 0);
    }*/
    if (cmdPtr->echo)
        Jsi_LogInfo("SOCK-SEND: %s\n", str); 
    if (id == 0) {
        if (!cmdPtr->stats.stack)
            cmdPtr->stats.stack = Jsi_StackNew();
        pss = &cmdPtr->stats;
        goto process;
    }
    for (hPtr = Jsi_HashSearchFirst(cmdPtr->pssTable, &cursor);
        hPtr != NULL; hPtr = Jsi_HashSearchNext(&cursor)) {
        pss = (SocketPss *)Jsi_HashValueGet(hPtr);
        SIGASSERT(pss, SOCKETPSS);
        if ((id==-1 || pss->id == id) && pss->state != PSS_DEAD) {
process:
            sockUpdateEvent(cmdPtr, pss);
            if (cmdPtr->noAsync || opts.noAsync) {
                rc = sock_handler(cmdPtr, SOCK_CALLBACK_WRITEABLE, pss, NULL, 0);
                int n, siLen = sockAddrSize(&pss->sa);
                if (cmdPtr->udp)
                    n = sendto(pss->fd, str, aLen, cmdPtr->sendFlags, (struct sockaddr*)&cmdPtr->stats.sa.sin, siLen);
                else
                    n = send(pss->fd, str, aLen, cmdPtr->sendFlags);
                if (n != aLen)
                    rc = Jsi_LogError("wrote only %d of %d bytes", n, aLen);
            } else {
                if (!cmdPtr->echo && pss->echo)
                    Jsi_LogInfo("WS-SEND: %s\n", str); 
                Jsi_StackPush(pss->stack, arg);
                Jsi_IncrRefCount(interp, arg);
                if (pss->fd>=0)
                    FD_SET (pss->fd, &cmdPtr->writeSet);
            }
            if (id != -1 ||  rc != JSI_OK)
                break;
        }
    }
    //Jsi_DSFree(&eStr);
    return rc;
}

static Jsi_RC SocketRecvCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) 
        return Jsi_LogError("Apply in a non-sock object");
    SIGASSERT(cmdPtr, SOCKET);

    SocketPss *pss = &cmdPtr->stats;
    int id = 0, argc = Jsi_ValueGetLength(interp, args);
    if (argc>1) {
        if (sockGetPssId(cmdPtr, Jsi_ValueArrayIndex(interp, args, 0), &id) != JSI_OK)
            return JSI_ERROR;
    }
    char buf[JSI_BUFSIZ];
    int n;
    pss->siLen = sockAddrSize(&pss->recvAddr);
    if (cmdPtr->udp)
        n = recvfrom(pss->fd, buf, sizeof(buf)-1, cmdPtr->sendFlags, (struct sockaddr*)&pss->recvAddr.sin, &pss->siLen);
    else
        n = recv(pss->fd, buf, sizeof(buf)-1, cmdPtr->recvFlags);
    if (n>0)
        buf[n] = 0;
    else 
        return Jsi_LogError("read failed");
    sockUpdateEvent(cmdPtr, pss);
    unsigned char *uptr = (unsigned char*)Jsi_Malloc(n+1);
    memcpy(uptr, buf, n+1);
    uptr[n] = 0;
    Jsi_ValueMakeBlob(interp, ret, (unsigned char*)buf, n);
    return JSI_OK;
}

static Jsi_RC SocketCloseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) 
        return Jsi_LogError("Apply in a non-sock object");
    SIGASSERT(cmdPtr, SOCKET);
    SocketPss *pss;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    int id = 0, argc = Jsi_ValueGetLength(interp, args);
    if (argc>1) {
        if (sockGetPssId(cmdPtr, Jsi_ValueArrayIndex(interp, args, 0), &id) != JSI_OK)
            return JSI_ERROR;
    }
    if (id == 0) {
        if (cmdPtr->stats.fd>=0) {
            close(cmdPtr->stats.fd);
            FD_CLR(cmdPtr->stats.fd, &cmdPtr->writeSet);
            FD_CLR(cmdPtr->stats.fd, &cmdPtr->readSet);
            cmdPtr->stats.fd = -1;
        }
    } else {
        for (hPtr = Jsi_HashSearchFirst(cmdPtr->pssTable, &cursor);
            hPtr != NULL; hPtr = Jsi_HashSearchNext(&cursor)) {
            pss = (SocketPss *)Jsi_HashValueGet(hPtr);
            SIGASSERT(pss, SOCKETPSS);
            if ((id==-1 || pss->id == id) && pss->state != PSS_DEAD) {
                if (pss->fd>=0) {
                    close(pss->fd);
                    FD_CLR(pss->fd, &cmdPtr->writeSet);
                    FD_CLR(pss->fd, &cmdPtr->readSet);
                    pss->fd = -1;
                }
            }
        }
    }
    return JSI_OK;
}

static Jsi_RC SocketUpdateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) 
        return Jsi_LogError("Apply to non-socket object");
    if (!cmdPtr->noUpdate)
        jsk_Service(cmdPtr);
    return JSI_OK;
}

static Jsi_RC sockUpdate(Jsi_Interp *interp, void *data)
{
    SocketObj *cmdPtr = (SocketObj *)data;
    SIGASSERT(cmdPtr,SOCKET);
    jsk_Service(cmdPtr);
    return JSI_OK;
}

static void socketObjErase(SocketObj *cmdPtr)
{
    cmdPtr->deleted = 1;
    if (cmdPtr->stats.fd>=0)
        close(cmdPtr->stats.fd);
    cmdPtr->stats.fd = -1;
    if (cmdPtr->interp) {
        if (cmdPtr->event)
            Jsi_EventFree(cmdPtr->interp, cmdPtr->event);
        cmdPtr->event = NULL;
        if (cmdPtr->hasOpts)
            Jsi_OptionsFree(cmdPtr->interp, SockOptions, cmdPtr, 0);
        cmdPtr->hasOpts = 0;
        if (cmdPtr->pssTable)
            Jsi_HashDelete(cmdPtr->pssTable);
        cmdPtr->pssTable = NULL;
    }
    if (cmdPtr->stats.stack) {
        Jsi_StackFreeElements(cmdPtr->interp, cmdPtr->stats.stack, sockFreeStackPss);
        Jsi_StackFree(cmdPtr->stats.stack);
    }

    cmdPtr->interp = NULL;
}

static Jsi_RC socketObjFree(Jsi_Interp *interp, void *data)
{
    SocketObj *cmdPtr = (SocketObj *)data;
    SIGASSERT(cmdPtr,SOCKET);
    socketObjErase(cmdPtr);
    Jsi_Free(cmdPtr);
    return JSI_OK;
}

static bool socketObjIsTrue(void *data)
{
    //SocketObj *cmdPtr = data;
    return 1;
   /* if (!fo->sockname) return 0;
    else return 1;*/
}

static bool socketObjEqual(void *data1, void *data2)
{
    return (data1 == data2);
}


static Jsi_RC sockFreePss(Jsi_Interp *interp, Jsi_HashEntry* hPtr, void *ptr) {
    SocketPss *pss = (SocketPss *)ptr;
    if (pss) {
        pss->hPtr = NULL;
        sockDeletePss(pss);
    }
    return JSI_OK;
}

static Jsi_RC sockParseHostname(const char *hostname, SockAddrAll *sa, int *saLen, int port) {
#ifndef WITHOUT_GETADDRINFO
    struct addrinfo req = {}, *ai;
#ifdef JSI__IPV6
    if (Jsi_Strchr(hostname, ':')) {
        if (inet_pton (AF_INET6, hostname, &sa->sin6.sin6_addr) != 1) {
            return JSI_ERROR;
        }
        sa->sin6.sin6_family = AF_INET6;
        sa->sin.sin_port = htons(port);
        *saLen = sizeof(sa->sin6);
        return JSI_OK;
    }
#endif

    req.ai_family = sa->sin.sin_family;

    if (getaddrinfo(hostname, NULL, &req, &ai))
        return JSI_ERROR;
    memcpy(&sa->sin, ai->ai_addr, ai->ai_addrlen);
    sa->sin.sin_port = htons(port);
    *saLen = ai->ai_addrlen;
    freeaddrinfo(ai);
#else
    struct hostent *he;

    if (!(he = gethostbyname(hostname)))
        return JSI_ERROR;
    if (he->h_length == sizeof(sa->sin.sin_addr)) {
        *saLen = sizeof(sa->sin);
        sa->sin.sin_family= he->h_addrtype;
        memcpy(&sa->sin.sin_addr, he->h_addr, he->h_length);
    }
#endif
    return JSI_OK;
}

static Jsi_Value*
dump_socket_info(SocketObj *cmdPtr, SocketPss *pss)
{
    Jsi_Interp *interp = cmdPtr->interp;
    Jsi_Value *ret = Jsi_ValueNew1(interp);
#ifdef JSI_MEM_DEBUG
    jsi_ValueDebugLabel(ret, "socket", "dump_socket");
#endif
    Jsi_JSONParseFmt(interp, &ret, "{address:\"%s\"}", inet_ntoa(pss->sa.sin.sin_addr));
    return ret;
}


static Jsi_RC sock_handler(SocketObj *cmdPtr, callback_reasons reason, SocketPss *pss, char *inPtr, size_t len)
{
    Jsi_Interp *interp = cmdPtr->interp;
    int n;
    Jsi_RC rc = JSI_OK;
    SocketPss *spss = ((!cmdPtr->client && &cmdPtr->stats!=pss)?&cmdPtr->stats:NULL);
    int deleted = (cmdPtr->deleted || Jsi_InterpGone(interp));

   // char buf[JSI_BUFSIZ];//, *bufPtr = buf;
    //static char *statBuf = NULL;
    //static int statSize = 0;
    
    SIGASSERT(cmdPtr, SOCKET);
    SIGASSERT(pss, SOCKETPSS);
    switch (reason) {
        
    case SOCK_CALLBACK_OPEN:
        if (deleted) return JSI_ERROR;
        sockUpdateEvent(cmdPtr, pss);
        if (cmdPtr->debug)
            fprintf(stderr, "SOCK:CALLBACK_OPEN: %p\n", pss);
        if (cmdPtr->onOpen) {
            int killcon = 0, n = 0;
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[10], *ret = Jsi_ValueNew1(interp);
            /* Pass 2 args: ws, id. */
            vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
            vargs[n++] = dump_socket_info(cmdPtr, pss);
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_DecrRefCount(interp, vargs[1]);
            Jsi_IncrRefCount(interp, vpargs);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onOpen, vpargs, &ret, NULL);
            if (rc == JSI_OK && Jsi_ValueIsFalse(interp, ret)) {
                if (cmdPtr->debug)
                    fprintf(stderr, "SOCK:KILLING CONNECTION: %p\n", pss);
                killcon = 1;
            }

            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (rc != JSI_OK) 
                return Jsi_LogError("socket bad rcv eval");
            if (killcon)
                return JSI_ERROR;
        }
        break;

    case SOCK_CALLBACK_CLOSED:
        if (cmdPtr->debug)
            fprintf(stderr, "SOCK:CALLBACK_CLOSE: %p\n", pss);
        if (!pss) break;
            sockUpdateEvent(cmdPtr, pss);
        if (cmdPtr->onClose && !Jsi_InterpGone(interp)) {
            /* Pass 2 args: ws, id. */
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[10];
            int n = 0;
            vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
            vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->id));
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            
            Jsi_Value *ret = Jsi_ValueNew1(interp);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onClose, vpargs, &ret, NULL);

            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
        }        
        sockDeletePss(pss);
        if (rc == JSI_OK && cmdPtr->connectCnt<=0 && cmdPtr->onCloseLast && !Jsi_InterpGone(interp)) {
            if (Jsi_InterpGone(interp))
                return JSI_ERROR;
            Jsi_Value *vpargs, *vargs[10], *ret = Jsi_ValueNew1(interp);
            int n = 0;
            vargs[n++] = (cmdPtr->deleted?Jsi_ValueNewNull(interp):Jsi_ValueNewObj(interp, cmdPtr->fobj));
            vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onCloseLast, vpargs, &ret, NULL);
            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
        }
        if (rc != JSI_OK) 
            return Jsi_LogError("sock bad rcv eval");
        break;
    case SOCK_CALLBACK_WRITEABLE:
        if (deleted) return JSI_ERROR;
        n=0;
        while (1) {
            Jsi_Value *dv = (Jsi_Value*)Jsi_StackPeek(pss->stack);
            char *data;
            int sLen;
            if (dv == NULL || (data=Jsi_ValueString(interp, dv, &sLen)) == NULL)
                break;
            pss->state = PSS_SENT;
        
            int osiz = sLen-pss->offset;
            int siLen = sockAddrSize(&pss->sa);
            sockUpdateEvent(cmdPtr, pss);
            if (cmdPtr->udp)
                n = sendto(pss->fd, data + pss->offset, osiz, cmdPtr->sendFlags, (struct sockaddr*)&cmdPtr->stats.sa.sin, siLen);
            else
                n = send(pss->fd, data + pss->offset, osiz, cmdPtr->sendFlags);
            if (n<=0) {
                perror("failed");
                if (cmdPtr->debug)
                    fprintf(stderr, "error on write\n");
                return JSI_ERROR;
            }
            if (n >= osiz) {
                Jsi_StackPop(pss->stack);
                pss->offset = 0;
            } else {
                if (cmdPtr->debug)
                    fprintf(stderr, "more to write: %d\n", (osiz-n));
                pss->offset += n;
            }
            if (cmdPtr->debug)
                fprintf(stderr, "SOCK:CLIENT WRITE(%d): %d=>%d: %s\n", pss->id, sLen, n, data);
            Jsi_DecrRefCount(interp, dv);                                   
            if (n >= 0) {
                pss->sentCnt++;
                pss->sentLast = time(NULL);
                if (spss) {
                    spss->sentCnt++;
                    spss->sentLast = time(NULL);
                }
            } else {
                if (cmdPtr->debug)
                    fprintf(stderr, "ERROR %d writing to socket\n", n);
                pss->state = PSS_SENDERR;
                pss->sentErrCnt++;
                pss->sentErrLast = time(NULL);
                if (spss) {
                    spss->sentErrCnt++;
                    spss->sentErrLast = time(NULL);
                }
                return JSI_ERROR;
            }
        }
        break;
        
    case SOCK_CALLBACK_RECEIVE:
    {
        if (deleted) return JSI_ERROR;
        int src;
        if (cmdPtr->debug)
            fprintf(stderr, "SOCK:RECV: %p\n", pss);

        pss->recvCnt++;
        pss->recvLast = time(NULL);
        if (spss) {
            spss->recvCnt++;
            spss->recvLast = time(NULL);
            spss->recvAddr = pss->sa;
        }
        sockUpdateEvent(cmdPtr, pss);

        if (cmdPtr->echo || pss->echo)
            Jsi_LogInfo("WS-RECV: %s\n", inPtr); 
        if (cmdPtr->onRecv) {
            /* Pass 3 args: ws, id, data. */
            Jsi_Value *vpargs, *vargs[10], *ret = Jsi_ValueNew1(interp);;
            int n = 0;
            vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
            vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->id));
            vargs[n++]  = Jsi_ValueNewBlob(interp, (uchar*)inPtr, len);
            vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            Jsi_ValueMakeUndef(interp, &ret);
            src = Jsi_FunctionInvoke(interp, cmdPtr->onRecv, vpargs, &ret, NULL);
            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (src != JSI_OK)
                rc = Jsi_LogError("sock2 bad rcv eval");
        }

        break;
 
    }
    default:
        break;
    }
    return rc;
}


static Jsi_RC jsk_Service(SocketObj *cmdPtr)
{
    char buf[JSI_BUFSIZ];
    struct sockaddr_in sin;
    fd_set writeSet, readSet;
    int i, n, cnt = 0;
    Jsi_RC rc = JSI_ERROR;

    readSet = cmdPtr->readSet;
    writeSet = cmdPtr->writeSet;
    int maxfd = cmdPtr->maxfd+1;
    maxfd = FD_SETSIZE;
    if (cmdPtr->debug)
        fprintf(stderr, "selecting\n");
    struct timeval tv;
    tv = cmdPtr->tv;
    if ((n=select(maxfd, &readSet, &writeSet, NULL, &tv)) < 0 && errno!=EINTR) {
        perror("Select");
        goto done;
    }
    if (cmdPtr->debug)
        fprintf(stderr, "select done: %d\n", n);

    for (i = 0; i < maxfd && cnt < n; ++i) {
        if (cmdPtr->deleted || Jsi_InterpGone(cmdPtr->interp)) {
            rc = JSI_ERROR;
            goto done;
        }
        if (FD_ISSET (i, &readSet)) {
            cnt++;
            if (!cmdPtr->client && i == cmdPtr->stats.fd && cmdPtr->udp==0) {
                uint c = sizeof(sin);
                int csock = accept(cmdPtr->stats.fd, (struct sockaddr*)&sin, &c);
                if (csock < 0) {
                    perror("accept");
                    goto done;
                }
                SocketPss* pss = sockGetPss(cmdPtr, csock, 1);
                pss->sa.sin = sin;
                if (sock_handler(cmdPtr, SOCK_CALLBACK_OPEN, pss, NULL, 0) != JSI_OK) {
                    close(csock);
                    sockDeletePss(pss);
                    continue;
                }
                if (cmdPtr->debug)
                    fprintf (stderr, "Server: connect %d from host %s, port %d.\n",
                        csock, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
                FD_SET(csock, &cmdPtr->readSet);
            } else {
                SocketPss* pss = sockGetPss(cmdPtr, i, 0);
                if (!pss) {
                    if (cmdPtr->debug)
                        fprintf(stderr, "no pss for %d\n", i);
                    continue;
                }
                uint siLen = sockAddrSize(&pss->recvAddr);
                if (cmdPtr->udp)
                    n = recvfrom(pss->fd, buf, sizeof(buf)-1, cmdPtr->sendFlags, (struct sockaddr*)&pss->recvAddr.sin, &siLen);
                else
                    n = recv(i, buf, sizeof(buf)-1, cmdPtr->recvFlags);
                if (n<=0) {
                    if (cmdPtr->debug)
                        fprintf(stderr, "read failed %d\n", i);
                    close(i);
                    FD_CLR(i, &cmdPtr->readSet);
                    FD_CLR(i, &cmdPtr->writeSet);
                    sock_handler(cmdPtr, SOCK_CALLBACK_CLOSED, pss, NULL, 0);
                }
                buf[sizeof(buf)-1] = 0;
                if (cmdPtr->debug)
                    fprintf(stderr, "Read data(%d): %s\n", n, buf);
                if (n>0 && sock_handler(cmdPtr, SOCK_CALLBACK_RECEIVE, pss, buf, n) != JSI_OK) {
                    sockDeletePss(pss);
                    continue;
                }
            }
        }
        if (FD_ISSET(i, &writeSet)) {
            cnt++;
            SocketPss* pss = sockGetPss(cmdPtr, i, 0);
            if (!pss) {
                if (cmdPtr->debug)
                    fprintf(stderr, "no pss for %d\n", i);
                continue;
            }
            if (sock_handler(cmdPtr, SOCK_CALLBACK_WRITEABLE, pss, NULL, 0) != JSI_OK) {
                close(pss->fd);
                FD_CLR(pss->fd, &cmdPtr->writeSet);
                FD_CLR(pss->fd, &cmdPtr->readSet);
                pss->fd = -1;
                sockDeletePss(pss);
                if (cmdPtr->udp)
                    goto done;
                continue;
            }
            if (Jsi_StackSize(pss->stack)==0)
                FD_CLR(i, &cmdPtr->writeSet);
        }

    }
    rc = JSI_OK;
done:
    return rc;
}

#define FN_Socket JSI_INFO("\
Create a socket server or client object.")
static Jsi_RC SocketConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);

static Jsi_RC SocketIdConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) 
        return Jsi_LogError("Apply in a non-socket object");
    SocketPss *pss = &cmdPtr->stats;
    int id = 0;
    int argc = Jsi_ValueGetLength(interp, args);
    if (argc) {
        Jsi_Value *valPtr = Jsi_ValueArrayIndex(interp, args, 0);
        Jsi_Number vid;
        if (Jsi_ValueGetNumber(interp, valPtr, &vid) != JSI_OK || vid < 0) 
            return Jsi_LogError("Expected number id");
        id = (int)vid;
        pss = sockFindPss(cmdPtr, id);
    }
    if (!pss) 
        return Jsi_LogError("No such id: %d", id);
    return Jsi_OptionsConf(interp, SPSOptions, pss, Jsi_ValueArrayIndex(interp, args, 1), ret, 0);
}

static Jsi_RC SocketIdsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) 
        return Jsi_LogError("Apply in a non-socket object");
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_DSAppend(&dStr, "{", NULL);
    SocketPss *pss = NULL;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    int cnt = 0;
    for (hPtr = Jsi_HashSearchFirst(cmdPtr->pssTable, &cursor);
        hPtr != NULL; hPtr = Jsi_HashSearchNext(&cursor)) {
        pss = (SocketPss *)Jsi_HashValueGet(hPtr);
        SIGASSERT(pss, SOCKETPSS);
        if (pss->state != PSS_DEAD) {
            Jsi_DSPrintf(&dStr, "%s%d", cnt++?",":"", pss->id);
        }
    }
    Jsi_DSAppend(&dStr, "}", NULL);
    Jsi_RC rc = Jsi_JSONParse(interp, Jsi_DSValue(&dStr), ret, 0);
    Jsi_DSFree(&dStr);
    return rc;
}


static Jsi_CmdSpec sockCmds[] = {
    { "Socket",     SocketConstructor, 0,  1, "options:object=void", .help="Create socket server/client object",
            .retType=(uint)JSI_TT_USEROBJ, .flags=JSI_CMD_IS_CONSTRUCTOR, .info=FN_Socket, .opts=SockOptions },
    { "close",      SocketCloseCmd,    0,  1, "", .help="Close socket(s)", .retType=(uint)JSI_TT_VOID },
    { "conf",       SocketConfCmd,     0,  1, "options:string|object=void",.help="Configure options", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=SockOptions },
    { "names",      SocketIdsCmd,      0,  0, "", .help="Return list of active ids on server", .retType=(uint)JSI_TT_ARRAY },
    { "idconf",     SocketIdConfCmd,   0,  2, "id:number=void, options:string|object=void",.help="Configure options for a connection id, or return list of ids", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=SPSOptions },
    { "recv",       SocketRecvCmd,     0,  1, "id:number=void", .help="Recieve data", .retType=(uint)JSI_TT_STRING },
    { "send",       SocketSendCmd,     2,  3, "id:number, data:string, options:object=void", .help="Send a socket message to id", .retType=(uint)JSI_TT_VOID, .flags=0, .info=FN_socksend, .opts=SockSendOptions },
    { "update",     SocketUpdateCmd,   0,  0, "", .help="Service events for just this socket", .retType=(uint)JSI_TT_VOID },
    { NULL, 0,0,0,0, .help="Commands for managing Socket server/client connections"  }
};


static Jsi_UserObjReg sockobject = {
    "Socket",
    sockCmds,
    socketObjFree,
    socketObjIsTrue,
    socketObjEqual
};

    
static Jsi_RC SocketConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_InterpAccess(interp, NULL, JSI_INTACCESS_NETWORK ) != JSI_OK)
        return Jsi_LogError("Socket disallowed by Interp.noNetwork option");
    int lfd, fd = -1;
    SocketObj *cmdPtr;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_DString aaStr = {};
    Jsi_Obj *fobj;
    const char *address;
    SockAddrAll *sap;
    char loopch;
    int on;
    Jsi_Value *toacc = NULL;
        
    cmdPtr = (SocketObj *)Jsi_Calloc(1, sizeof(*cmdPtr));
    SIGINIT(cmdPtr, SOCKET);
    SIGINIT(&cmdPtr->stats, SOCKETPSS);
    cmdPtr->stats.cmdPtr = cmdPtr;
    cmdPtr->stats.fd = -1;
    cmdPtr->interp = interp;
    cmdPtr->startTime = time(NULL);
    cmdPtr->timeout = 0.5;
    cmdPtr->hasOpts = 1;
    if ((arg != NULL && !Jsi_ValueIsNull(interp,arg))
        && Jsi_OptionsProcess(interp, SockOptions, cmdPtr, arg, 0) < 0) {
        goto bail;
    }
    if (!cmdPtr->udata) {
        cmdPtr->udata = Jsi_ValueNewObj(interp, NULL);
        Jsi_IncrRefCount(interp, cmdPtr->udata);
    }
    cmdPtr->tv.tv_sec = (uint)cmdPtr->timeout;
    cmdPtr->tv.tv_usec = (uint)((cmdPtr->timeout - cmdPtr->tv.tv_sec)*1000000);
#if 0
    cmdPtr->info.port = (cmdPtr->client ? CONTEXT_PORT_NO_LISTEN : cmdPtr->port);
#endif
    cmdPtr->family = AF_INET;
    sap = &cmdPtr->stats.sa;
    memset(sap, 0, sizeof(*sap));
    address = (cmdPtr->address ? Jsi_ValueString(interp, cmdPtr->address, NULL) : "127.0.0.1");
    sap->sin.sin_family = cmdPtr->family;
    sap->sin.sin_addr.s_addr = INADDR_ANY;
    sap->sin.sin_port = htons(cmdPtr->port); 

    if (sockParseHostname(address, sap, &cmdPtr->saLen, cmdPtr->port) != JSI_OK) {
        Jsi_LogError("hostname parse");
        goto bail;
    }
    sap->sin.sin_port = htons(cmdPtr->port); 
    FD_ZERO(&cmdPtr->readSet);
    FD_ZERO(&cmdPtr->writeSet);
    FD_ZERO(&cmdPtr->exceptSet);
    if ((fd = socket(cmdPtr->family, cmdPtr->udp ? SOCK_DGRAM : SOCK_STREAM, 0)) < 0) {
        Jsi_LogError("sock create failed");
        goto bail;
    }
//#ifndef WIN32
    fcntl(fd, F_SETFD, FD_CLOEXEC);
//#endif
    on = 1;
    if (cmdPtr->reuse && setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) )) {
        Jsi_LogError("sock reuse failed");
        goto bail;
    }
    if (cmdPtr->broadcast && setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on) )) {
        Jsi_LogError("sock broadcast failed");
        goto bail;
    }
    if (cmdPtr->keepalive && setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on) )) {
        Jsi_LogError("sock keepalive failed");
        goto bail;
    }
    if (cmdPtr->ttl && setsockopt(fd, IPPROTO_IP, IP_TTL, &cmdPtr->ttl, sizeof(cmdPtr->ttl)) < 0) {
        Jsi_LogError("sock ttl failed");
        goto bail;
    }

    if (cmdPtr->tos && setsockopt(fd, IPPROTO_IP, IP_TOS, &cmdPtr->tos, sizeof(cmdPtr->tos))) {
        Jsi_LogWarn("sock tos failed");
    }

    if (cmdPtr->mcastTtl && setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &cmdPtr->mcastTtl, sizeof(cmdPtr->mcastTtl)) < 0) {
        Jsi_LogError("sock mcast ttl failed");
        goto bail;
    }
    loopch = 0;
    if (cmdPtr->mcastNoLoop && setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loopch, sizeof(loopch))) {
        Jsi_LogError("sock mcast loop failed");
        goto bail;
    }

    if (cmdPtr->mcastInterface) {
        
        const char *saddress = (cmdPtr->mcastInterface ? Jsi_ValueString(interp, cmdPtr->mcastInterface, NULL) : NULL);
        SockAddrAll srcaddr = {};
        srcaddr.sin.sin_family = AF_INET;
        srcaddr.sin.sin_addr.s_addr = INADDR_ANY;
        srcaddr.sin.sin_port = 0;

        int srcLen;
        if (saddress && sockParseHostname(saddress, &srcaddr, &srcLen, cmdPtr->srcPort) != JSI_OK) {
            Jsi_LogError("source hostname parse");
            goto bail;
        }

        if (saddress && setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &srcaddr.sin.sin_addr.s_addr, sizeof(srcaddr.sin.sin_addr.s_addr)) < 0) {
            Jsi_LogError("sock mcast interface failed");
            goto bail;
        }
    }

    if (cmdPtr->recvTimeout) {
        struct timeval tv = {};
        tv.tv_sec = cmdPtr->recvTimeout/1000000LL;
        tv.tv_usec = cmdPtr->recvTimeout - tv.tv_sec*1000000LL;

        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv,  sizeof(struct timeval))) {
            Jsi_LogError("sock reuse failed");
            goto bail;
        }
    }
    if (cmdPtr->sendTimeout) {
        struct timeval tv = {};
        tv.tv_sec = cmdPtr->sendTimeout/1000000LL;
        tv.tv_usec = cmdPtr->sendTimeout - tv.tv_sec*1000000LL;

        if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv,  sizeof(struct timeval))) {
            Jsi_LogError("sock reuse failed");
            goto bail;
        }
    }
    
    if (cmdPtr->mcastAddMember) {
        const char *iaddress = NULL, *maddress = (cmdPtr->mcastAddMember ? Jsi_ValueString(interp, cmdPtr->mcastAddMember, NULL) : NULL);
        SockAddrAll maddr = {}, iaddr;
        maddr.sin.sin_family = cmdPtr->family;
        maddr.sin.sin_addr.s_addr = INADDR_ANY;
        maddr.sin.sin_port = 0; 
        iaddr = maddr;
        
        if (maddress && Jsi_Strchr(maddress, '/')) {
            char *icp;
            maddress = Jsi_DSAppend(&aaStr, maddress, NULL);
            icp = (char*)Jsi_Strchr(maddress, '/');
            iaddress = icp+1;
            *icp = 0;
        }
        int msaLen;
        if (maddress && sockParseHostname(maddress, &maddr, &msaLen, 0) != JSI_OK) {
            Jsi_LogError("mcast address parse");
            goto bail;
        }

        struct ip_mreq receiveGroup = {};
        receiveGroup.imr_multiaddr.s_addr = maddr.sin.sin_addr.s_addr; 
        if (iaddress) {
            if (sockParseHostname(iaddress, &iaddr, &msaLen, 0) != JSI_OK) {
                Jsi_LogError("mcast interface parse");
                goto bail;
            }
            receiveGroup.imr_interface.s_addr = iaddr.sin.sin_addr.s_addr;
        }
    
        //Set receive address
        if(setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&receiveGroup, sizeof(receiveGroup)) < 0)
        {
            Jsi_LogError("mcast address set failed");
            goto bail;
        }
    }
    
    if (cmdPtr->srcAddress || cmdPtr->srcPort) {
        if (!cmdPtr->client) {
            Jsi_LogError("only client mode accepts a source port/address");
            goto bail;
        }
        
        const char *saddress = (cmdPtr->srcAddress ? Jsi_ValueString(interp, cmdPtr->srcAddress, NULL) : "0.0.0.0");
        SockAddrAll srcaddr = {};
        srcaddr.sin.sin_family = AF_INET;
        srcaddr.sin.sin_addr.s_addr = INADDR_ANY;
        srcaddr.sin.sin_port = htons(cmdPtr->srcPort);

        int srcLen;
        if (sockParseHostname(saddress, &srcaddr, &srcLen, cmdPtr->srcPort) != JSI_OK) {
            Jsi_LogError("source hostname parse");
            goto bail;
        }

        if (bind(fd, (struct sockaddr *) &srcaddr.sin, sizeof(srcaddr)) < 0) {
            Jsi_LogError("sock bind failed");
            goto bail;
        }
    }

    if (fd>cmdPtr->maxfd)
        cmdPtr->maxfd = fd;
    
    if (cmdPtr->client) { // Client
        lfd = cmdPtr->stats.fd = fd;
        fd = -1;
        if (!cmdPtr->udp) {
            if (connect(lfd , (struct sockaddr*)sap , sizeof(sap->sin)) < 0) {
                Jsi_LogError("sock connect failed");
                goto bail;
            }
            FD_SET(lfd, &cmdPtr->readSet);
        }
    } else { // Server
        lfd = cmdPtr->stats.fd = fd;
        fd = -1;
        if( bind(lfd, (struct sockaddr*)sap , sizeof(sap->sin)) < 0) {
            Jsi_LogError("sock bind failed");
            goto bail;
        }
        if (cmdPtr->udp == 0) {
            if (listen(lfd , 3)) {
                Jsi_LogError("sock listen failed");
                goto bail;
            }
        }
        if (cmdPtr->port==0) {
            struct sockaddr_in sin;
            socklen_t len = sizeof(sin);
            if (getsockname(lfd, (struct sockaddr *)&sin, &len) == -1)
                perror("getsockname");
            else {
                cmdPtr->port = ntohs(sin.sin_port);
            }
        }
        if (cmdPtr->debug)
            printf("listening on port: %d\n", cmdPtr->port);
        cmdPtr->pssTable = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, sockFreePss);
        FD_SET(lfd, &cmdPtr->readSet);
    }

    cmdPtr->event = Jsi_EventNew(interp, sockUpdate, cmdPtr);
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        toacc = _this;
    } else {
        Jsi_Obj *o = Jsi_ObjNew(interp);
        Jsi_PrototypeObjSet(interp, "Socket", o);
        Jsi_ValueMakeObject(interp, ret, o);
        toacc = *ret;
    }

    fobj = Jsi_ValueGetObj(interp, toacc);
    if ((cmdPtr->objId = Jsi_UserObjNew(interp, &sockobject, fobj, cmdPtr))<0)
        goto bail;
    cmdPtr->fobj = fobj;
    return JSI_OK;

bail:
    Jsi_DSFree(&aaStr);
    socketObjFree(interp, cmdPtr);
    if (fd>=0)
        close(fd);
    Jsi_ValueMakeUndef(interp, ret);
    return JSI_ERROR;
}

static Jsi_RC Jsi_DoneSocket(Jsi_Interp *interp)
{
    if (Jsi_UserObjUnregister(interp, &sockobject) != JSI_OK)
        return JSI_ERROR;
    Jsi_PkgProvide(interp, "Socket", -1, NULL);
    return JSI_OK;
}

Jsi_RC Jsi_InitSocket(Jsi_Interp *interp, int release)
{
    if (release) return Jsi_DoneSocket(interp);
    Jsi_Hash *wsys;
    if (!(wsys = Jsi_UserObjRegister(interp, &sockobject))) {
        Jsi_LogBug("Can not init socket");
        return JSI_ERROR;
    }
    if (Jsi_PkgProvide(interp, "Socket", 2, Jsi_InitSocket) != JSI_OK)
        return JSI_ERROR;

    if (!Jsi_CommandCreateSpecs(interp, sockobject.name, sockCmds, wsys, JSI_CMDSPEC_ISOBJ))
        return JSI_ERROR;
    return JSI_OK;
}

#endif
#endif //JSI_LITE_ONLY
