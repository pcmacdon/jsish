#ifndef JSI_LITE_ONLY
#ifndef JSI_OMIT_SOCKET

#include "jsiInt.h"

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

//#define HAVE_IPV6
typedef union {
    struct sockaddr_in sin;
#ifdef HAVE_IPV6
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
    int sentCnt, recvCnt, sentErrCnt;
    time_t sentLast, recvLast, sentErrLast;
    Jsi_HashEntry *hPtr;
    Jsi_Stack *stack;
    int id;
    int fd;
    int offset;
    SockAddrAll sa;
    SockAddrAll recvAddr;
    uint siLen;
} SocketPss;

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
    Jsi_Value *onConnect;
    Jsi_Value *onOpen;
    Jsi_Value *defaultUrl;
    Jsi_Value *interface;
    Jsi_Value *address;
    char client_name[128];
    char client_ip[128];
    int idx;
    int port;
    int family;
    int saLen;
    char *iface;
    unsigned int oldus;
    Jsi_Bool udp;
    Jsi_Bool server;
    Jsi_Bool noUpdate;
    int hasOpts;
    int debug;
    int maxConnects;
    int deleted;
    int close_test;
    int connectCnt;
    int createCnt;
    int redirCnt;
    time_t createLast;
    time_t startTime;
    struct timeval tv;
    char *cmdName;

    Jsi_Event *event;
    Jsi_Obj *fobj;
    int objId;
    int rx_buffer_size;
    char *ssl_cert_filepath;
    char *ssl_private_key_filepath;
    int ws_uid;
    int ws_gid;
    char *cl_host;
    char *cl_origin;
    int maxfd;
    fd_set exceptSet, readSet, writeSet;
    int sendFlags;
    int recvFlags;
    SocketPss pss; // Server/non-async client pss.
} SocketObj;


/* Scanning function */
static int ValueToSockAddrOpt(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record )
{
#if 0
    int n, *s = (SockAddrAll*)(((char*)record) + spec->offset)
    //int flags = (spec->flags&JSI_OPT_CUST_NOCASE?JSI_CMP_NOCASE:0);
    if (!si) {
        Jsi_LogError("custom enum spec did not set data: %s", spec->name);
        return JSI_ERROR;
    }
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
static int SockAddrOptToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record)
{
    SockAddrAll* sin = (SockAddrAll*)(((char*)record) + spec->offset);
    int rc = JSI_OK;
   /* const char **list = spec->data;
    if (!list) {
        Jsi_LogError("custom enum spec did not set data: %s", spec->name);
        return JSI_ERROR;
    }*/
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

static Jsi_OptionCustom socketAddOption = {
    .name="sockAddr", .parseProc=ValueToSockAddrOpt, .formatProc=SockAddrOptToValue
};

#define IIOF .flags=JSI_OPT_INIT_ONLY

static Jsi_OptionSpec PSSOptions[] =
{
    JSI_OPT(INT,        SocketPss, recvCnt,      .help="Number of recieves"),
    JSI_OPT(TIMESTAMP,  SocketPss, recvLast,     .help="Time of last recv"),
    JSI_OPT(INT,        SocketPss, sentCnt,      .help="Number of sends"),
    JSI_OPT(TIMESTAMP,  SocketPss, sentLast,     .help="Time of last send"),
    JSI_OPT(INT,        SocketPss, sentErrCnt,   .help="Number of sends"),
    JSI_OPT(TIMESTAMP,  SocketPss, sentErrLast,  .help="Time of last sendErr"),
    JSI_OPT(CUSTOM,     SocketPss, recvAddr,     .help="Incoming port and address", .custom=&socketAddOption),
    JSI_OPT_END(SocketPss)
};

static Jsi_OptionSpec SockOptions[] =
{
    JSI_OPT(VALUE,  SocketObj, address,    .help="Client destination address", .init="127.0.0.0", IIOF ),
    JSI_OPT(INT,    SocketObj, connectCnt, .help="Counter for number of active connections", .flags=JSI_OPT_READ_ONLY),
    JSI_OPT(INT,    SocketObj, debug,      .help="Debugging level"),
    JSI_OPT(VALUE,  SocketObj, interface,  .help="Interface for server to listen on, eg. 'eth0' or 'lo'", IIOF),
    JSI_OPT(INT,    SocketObj, maxConnects,.help="In server mode, max number of client connections accepted"),
    JSI_OPT(BOOL,   SocketObj, noUpdate,   .help="Stop processing update events (eg. to exit)"),
    JSI_OPT(FUNC,   SocketObj, onClose,    .help="Function to call when connection closes"),
    JSI_OPT(FUNC,   SocketObj, onCloseLast,.help="Function to call when last connection closes"),
    JSI_OPT(FUNC,   SocketObj, onConnect,  .help="Function to call when connection starts"),
    JSI_OPT(FUNC,   SocketObj, onOpen,     .help="Function to call when connection opens"),
    JSI_OPT(FUNC,   SocketObj, onRecv,     .help="Function to call with recieved data"),
    JSI_OPT(INT,    SocketObj, port,       .help="Port for client dest or server listen", IIOF, .init="9000" ),
    JSI_OPT(BOOL,   SocketObj, server,     .help="Enable server mode", IIOF),
    JSI_OPT(TIMESTAMP, SocketObj, startTime,     .help="Time of start", .flags=JSI_OPT_READ_ONLY),
    JSI_OPT(TIMESTAMP, SocketObj, createLast,     .help="Time of last create", .flags=JSI_OPT_READ_ONLY),
    JSI_OPT(BOOL,   SocketObj, udp,        .help="Protocol is udp", IIOF),
    JSI_OPT_END(SocketObj)
};

static int sockService(SocketObj *cmdPtr);

static int socketObjFree(Jsi_Interp *interp, void *data);
static int socketObjIsTrue(void *data);
static int socketObjEqual(void *data1, void *data2);

static int sockAddrSize(SockAddrAll* sa) {
#ifdef HAVE_IPV6
    if (sa->sin.sin_family == AF_INET6)
        return sizeof(sa->sin6);
#endif
    return sizeof(sa->sin);
}

static SocketPss*
sockGetPss(SocketObj *cmdPtr, int fd, int create)
{
    if (fd == cmdPtr->pss.fd)
        return &cmdPtr->pss;
    Jsi_HashEntry *hPtr;
    SocketPss* pss;
    int isNew;
    if (create)
        hPtr = Jsi_HashEntryNew(cmdPtr->pssTable, (void*)fd, &isNew);
    else
        hPtr = Jsi_HashEntryFind(cmdPtr->pssTable, (void*)fd);
    if (!hPtr)
        return NULL;
    if (create == 0 || isNew == 0) {
        pss = (SocketPss*)Jsi_HashValueGet(hPtr);
        SIGASSERT(pss, SOCKETPSS);
        return pss;
    }
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
    return pss;
}


static SocketPss *sockFindPss(SocketObj *cmdPtr, int id) {
    if (id == 0) 
        return &cmdPtr->pss;
    SocketPss *tpss = NULL;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    for (hPtr = Jsi_HashEntryFirst(cmdPtr->pssTable, &cursor);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&cursor)) {
        tpss = (SocketPss*)Jsi_HashValueGet(hPtr);
        SIGASSERT(tpss, SOCKETPSS);
        if (tpss->id == id)
            return tpss;
    }
    return NULL;
}

static int sockFreeStackPss(Jsi_Interp *interp, void *data) {
    if (!data) return JSI_OK;
    Jsi_Value *v = (Jsi_Value*)data;
    SIGASSERT(v, VALUE);
    Jsi_DecrRefCount(interp, v);
    return JSI_OK;
}

static void
sockDeletePss(SocketPss *pss)
{
    if (pss == &pss->cmdPtr->pss)
        return;
    if (pss->hPtr) {
        Jsi_HashEntryDelete(pss->hPtr);
        pss->hPtr = NULL;
    }
    Jsi_StackFreeElements(pss->cmdPtr->interp, pss->stack, sockFreeStackPss);
    Jsi_StackFree(pss->stack);
    pss->cmdPtr->connectCnt--;
    /*Jsi_ObjDecrRefCount(pss->msgs);*/
    pss->state = PSS_DEAD;
}

static int SocketConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
  
    if (!cmdPtr) {
        Jsi_LogError("Apply in a non-sock object");
        return JSI_ERROR;
    }
    return Jsi_OptionsConf(interp, SockOptions, Jsi_ValueArrayIndex(interp, args, 0), cmdPtr, ret, 0);

}

static int sockGetPssId(SocketObj* cmdPtr, Jsi_Value* darg, int *idPtr) {
    Jsi_Interp *interp = cmdPtr->interp;
    Jsi_Number dnum = 0;
    int id;
    if (Jsi_ValueGetNumber(interp, darg, &dnum) != JSI_OK) {
        Jsi_LogError("invalid id");
        return JSI_ERROR;
    }
    id = (int)dnum;
    if (id < 0 && !cmdPtr->server)
        id = 0;
    if (id > 0 && !cmdPtr->server) {
        Jsi_LogError("invalid id");
        return JSI_ERROR;
    }
    *idPtr = id;
    return JSI_OK;
}

#define FN_wssend JSI_INFO("\
Send a message to 1 (or all connections if -1). If not already a string, msg is format as JSON prior to the send.")

static int SocketSendCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) {
        Jsi_LogError("Apply in a non-sock object");
        return JSI_ERROR;
    }
    SIGASSERT(cmdPtr, SOCKET);
    SocketPss *pss;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    int aLen = 0;
    char *str = Jsi_ValueString(interp, arg, &aLen);
    
    if (str == NULL || aLen <= 0) return JSI_OK;
    int id = 0, argc = Jsi_ValueGetLength(interp, args);
    Jsi_DString eStr = {};
    if (argc>1) {
        if (sockGetPssId(cmdPtr, Jsi_ValueArrayIndex(interp, args, 1), &id) != JSI_OK)
            return JSI_ERROR;
    }
    if (!str) {
        Jsi_Value *narg = Jsi_ValueDup(interp, arg);
        Jsi_ValueToString(interp, narg, NULL);
        arg = narg;
    }
   /* if (cmdPtr->udp) {
        Jsi_IncrRefCount(interp, arg);
        str = Jsi_ValueString(interp, arg, &aLen);
        int rc = sendto(cmdPtr->pss.fd, str, aLen, 0, &cmdPtr->pss.sa.sa, 0);
    }*/
    if (id == 0) {
        if (!cmdPtr->pss.stack)
            cmdPtr->pss.stack = Jsi_StackNew();
        Jsi_StackPush(cmdPtr->pss.stack, arg);
        Jsi_IncrRefCount(interp, arg);
        if (cmdPtr->pss.fd>=0)
            FD_SET(cmdPtr->pss.fd, &cmdPtr->writeSet);
    } else {
        for (hPtr = Jsi_HashEntryFirst(cmdPtr->pssTable, &cursor);
            hPtr != NULL; hPtr = Jsi_HashEntryNext(&cursor)) {
            pss = (SocketPss *)Jsi_HashValueGet(hPtr);
            SIGASSERT(pss, SOCKETPSS);
            if ((id==-1 || pss->id == id) && pss->state != PSS_DEAD) {
                Jsi_StackPush(pss->stack, arg);
                Jsi_IncrRefCount(interp, arg);
                FD_SET (pss->fd, &cmdPtr->writeSet);
                if (id != -1)
                    break;
            }
        }
    }
    Jsi_DSFree(&eStr);
    return JSI_OK;
}

static int SocketRecvCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) {
        Jsi_LogError("Apply in a non-sock object");
        return JSI_ERROR;
    }
    SIGASSERT(cmdPtr, SOCKET);

    SocketPss *pss = &cmdPtr->pss;
    int id = 0, argc = Jsi_ValueGetLength(interp, args);
    if (argc>1) {
        if (sockGetPssId(cmdPtr, Jsi_ValueArrayIndex(interp, args, 0), &id) != JSI_OK)
            return JSI_ERROR;
    }
    char buf[BUFSIZ];
    int n;
    pss->siLen = sockAddrSize(&pss->recvAddr);
    if (cmdPtr->udp)
        n = recvfrom(pss->fd, buf, sizeof(buf)-1, cmdPtr->sendFlags, (struct sockaddr*)&pss->recvAddr.sin, &pss->siLen);
    else
        n = recv(pss->fd, buf, sizeof(buf)-1, cmdPtr->recvFlags);
    if (n>0)
        buf[n] = 0;
    else {
        Jsi_LogError("read failed");
        return JSI_ERROR;
    }
    unsigned char *uptr = (unsigned char*)Jsi_Malloc(n+1);
    memcpy(uptr, buf, n+1);
    Jsi_ValueMakeBlob(interp, ret, (unsigned char*)buf, n);
    return JSI_OK;
}

static int SocketCloseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) {
        Jsi_LogError("Apply in a non-sock object");
        return JSI_ERROR;
    }
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
        if (cmdPtr->pss.fd>=0) {
            close(cmdPtr->pss.fd);
            FD_CLR(cmdPtr->pss.fd, &cmdPtr->writeSet);
            FD_CLR(cmdPtr->pss.fd, &cmdPtr->readSet);
            cmdPtr->pss.fd = -1;
        }
    } else {
        for (hPtr = Jsi_HashEntryFirst(cmdPtr->pssTable, &cursor);
            hPtr != NULL; hPtr = Jsi_HashEntryNext(&cursor)) {
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


#define FN_wsupdate JSI_INFO("\
Update socket queue.  This is used only in server mode to broadcast to clients.")

static int SocketUpdateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    Jsi_Value *oldcb = cmdPtr->onRecv;
    if (!cmdPtr) {
        Jsi_LogError("Apply to non-socket object");
        return JSI_ERROR;
    }
    if (cmdPtr->noUpdate)
        return JSI_OK;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    if (arg) {
        if (Jsi_ValueIsType(interp, arg, JSI_VT_OBJECT)==0 || Jsi_ValueIsObjType(interp, arg, JSI_OT_FUNCTION)==0) {
            Jsi_LogError("arg is not a function");
            return JSI_ERROR;
        }
        cmdPtr->onRecv = arg;
    }
    sockService(cmdPtr);
    if (arg)
        cmdPtr->onRecv = oldcb;
    return JSI_OK;
}

static int sockUpdate(Jsi_Interp *interp, void *data)
{
    SocketObj *cmdPtr = (SocketObj *)data;
    SIGASSERT(cmdPtr,SOCKET);
    sockService(cmdPtr);
    return JSI_OK;
}

static void socketObjErase(SocketObj *cmdPtr)
{
    if (cmdPtr->pss.fd>=0)
        close(cmdPtr->pss.fd);
    cmdPtr->pss.fd = -1;
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
    if (cmdPtr->pss.stack) {
        Jsi_StackFreeElements(cmdPtr->interp, cmdPtr->pss.stack, sockFreeStackPss);
        Jsi_StackFree(cmdPtr->pss.stack);
    }

    cmdPtr->interp = NULL;
}

static int socketObjFree(Jsi_Interp *interp, void *data)
{
    SocketObj *cmdPtr = (SocketObj *)data;
    SIGASSERT(cmdPtr,SOCKET);
    socketObjErase(cmdPtr);
    Jsi_Free(cmdPtr);
    return 0;
}

static int socketObjIsTrue(void *data)
{
    //SocketObj *cmdPtr = data;
    return 1;
   /* if (!fo->sockname) return 0;
    else return 1;*/
}

static int socketObjEqual(void *data1, void *data2)
{
    return (data1 == data2);
}


static int sockFreePss(Jsi_Interp *interp, Jsi_HashEntry* hPtr, void *ptr) {
    SocketPss *pss = (SocketPss *)ptr;
    if (pss) {
        pss->hPtr = NULL;
        sockDeletePss(pss);
    }
    return JSI_OK;
}

static int sockParseHostname(const char *hostname, SockAddrAll *sa, int *saLen, int port) {
#ifndef WITHOUT_GETADDRINFO
    struct addrinfo req, *ai;
#ifdef HAVE_IPV6
    if (strchr(hostname, ':')) {
        if (inet_pton (AF_INET6, hostname, &sa->sin6.sin6_addr) != 1) {
            return JSI_ERROR;
        }
        sa->sin6.sin6_family = AF_INET6;
        sa->sin.sin_port = htons(port);
        *saLen = sizeof(sa->sin6);
        return JSI_OK;
    }
#endif

    memset(&req, 0, sizeof(req));
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
    Jsi_Value *ret = Jsi_ValueNew(interp);
#ifdef JSI_MEM_DEBUG
    jsi_ValueDebugLabel(ret, "socket", "dump_socket");
#endif
    Jsi_JSONParseFmt(interp, &ret, "{address:\"%s\"}", inet_ntoa(pss->sa.sin.sin_addr));
    return ret;
}


static int
sock_handler(SocketObj *cmdPtr, callback_reasons reason,
      SocketPss *pss, char *inPtr, size_t len)
{
    Jsi_Interp *interp = cmdPtr->interp;
    int n, rc = JSI_OK;
   // char buf[BUFSIZ];//, *bufPtr = buf;
    //static char *statBuf = NULL;
    //static int statSize = 0;

    SIGASSERT(cmdPtr, SOCKET);
    SIGASSERT(pss, SOCKETPSS);
    switch (reason) {
        
    case SOCK_CALLBACK_OPEN:
        if (cmdPtr->debug)
            fprintf(stderr, "SOCK:CALLBACK_OPEN: %p\n", pss);
        if (cmdPtr->onOpen) {
            int killcon = 0;
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[2], *ret = Jsi_ValueNew1(interp);
            
            vargs[0] = dump_socket_info(cmdPtr, pss);
            Jsi_IncrRefCount(interp, vargs[0]);
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, 1, 1));
            Jsi_DecrRefCount(interp, vargs[0]);
            Jsi_IncrRefCount(interp, vpargs);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onOpen, vpargs, &ret, NULL);
            if (Jsi_InterpGone(interp))
                return 1;
            if (rc == JSI_OK && Jsi_ValueIsFalse(interp, ret)) {
                if (cmdPtr->debug)
                    fprintf(stderr, "SOCK:KILLING CONNECTION: %p\n", pss);
                killcon = 1;
            }

            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (rc != JSI_OK) {
                Jsi_LogError("socket bad rcv eval");
                return JSI_ERROR;
            }
            if (killcon)
                return JSI_ERROR;
        }
        break;

    case SOCK_CALLBACK_CLOSED:
        if (cmdPtr->debug)
            fprintf(stderr, "SOCK:CALLBACK_CLOSE: %p\n", pss);
        if (!pss) break;
        if (cmdPtr->onClose) {
            /* Pass 2 args: data and id. */
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[2];
            vargs[0] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->id));
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, 1, 0));
            Jsi_IncrRefCount(interp, vpargs);
            
            Jsi_Value *ret = Jsi_ValueNew1(interp);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onClose, vpargs, &ret, NULL);
            if (Jsi_InterpGone(interp))
                return JSI_ERROR;

            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (rc != JSI_OK) {
                Jsi_LogError("websock bad rcv eval");
                return JSI_ERROR;
            }
        }        
        sockDeletePss(pss);
        if (cmdPtr->connectCnt<=0 && cmdPtr->onCloseLast) {
            Jsi_FunctionInvokeBool(interp, cmdPtr->onCloseLast, NULL);
            if (Jsi_InterpGone(interp))
                return JSI_ERROR;
        }
        break;
    case SOCK_CALLBACK_WRITEABLE:
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
            if (cmdPtr->udp)
                n = sendto(pss->fd, data + pss->offset, osiz, cmdPtr->sendFlags, (struct sockaddr*)&cmdPtr->pss.sa.sin, siLen);
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
            } else {
                if (cmdPtr->debug)
                    fprintf(stderr, "ERROR %d writing to socket\n", n);
                pss->state = PSS_SENDERR;
                pss->sentErrCnt++;
                pss->sentErrLast = time(NULL);
                return JSI_ERROR;
            }
        }
        break;
        
    case SOCK_CALLBACK_RECEIVE:
    {
        int src;
        if (cmdPtr->debug)
            fprintf(stderr, "SOCK:RECV: %p\n", pss);

        pss->recvCnt++;
        pss->recvLast = time(NULL);

        if (cmdPtr->onRecv && !Jsi_ValueIsNull(interp, cmdPtr->onRecv)) {
            /* Pass 2 args: data and id. */
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[2];
            vargs[0]  = Jsi_ValueNewString(interp, Jsi_Strdup(inPtr), len);
            vargs[1] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->id));
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, 2, 0));
            Jsi_IncrRefCount(interp, vpargs);
            
            Jsi_Value *ret = Jsi_ValueNew1(interp);
            Jsi_ValueMakeUndef(interp, &ret);
            src = Jsi_FunctionInvoke(interp, cmdPtr->onRecv, vpargs, &ret, NULL);
            if (Jsi_InterpGone(interp))
                return JSI_ERROR;
            if (src == JSI_OK && Jsi_ValueIsUndef(interp, ret)==0) {
                /* TODO: handle callback return data??? */
            }

            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (src != JSI_OK) {
                Jsi_LogError("websock bad rcv eval");
                return rc;
            }
        }

        break;
 
    }
    default:
        break;
    }
    return rc;
}


static int sockService(SocketObj *cmdPtr)
{
    //Jsi_Interp *interp = cmdPtr->interp;
    char buf[BUFSIZ];
    struct sockaddr_in sin;
    fd_set writeSet, readSet;
    int i, n, cnt = 0;
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
        return JSI_ERROR;
    }
    if (cmdPtr->debug)
        fprintf(stderr, "select done: %d\n", n);

    for (i = 0; i < maxfd && cnt < n; ++i) {
        if (FD_ISSET (i, &readSet)) {
            cnt++;
            if (cmdPtr->server && i == cmdPtr->pss.fd && cmdPtr->udp==0) {
                uint c = sizeof(sin);
                int csock = accept(cmdPtr->pss.fd, (struct sockaddr*)&sin, &c);
                if (csock < 0) {
                    perror("accept");
                    return JSI_ERROR;
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
                    return JSI_ERROR;
                continue;
            }
            if (Jsi_StackLen(pss->stack)==0)
                FD_CLR(i, &cmdPtr->writeSet);
        }

    }
    return JSI_OK;
}

#define FN_Socket JSI_INFO("\
Create a socket server or client object.")
static int SocketConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);

static int SocketIdConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) {
        Jsi_LogError("Apply in a non-socket object");
        return JSI_ERROR;
    }
    SocketPss *pss = &cmdPtr->pss;
    int id = 0;
    int argc = Jsi_ValueGetLength(interp, args);
    if (argc) {
        Jsi_Value *valPtr = Jsi_ValueArrayIndex(interp, args, 0);
        Jsi_Number vid;
        if (Jsi_ValueGetNumber(interp, valPtr, &vid) != JSI_OK || vid < 0) {
            Jsi_LogError("Expected number id");
            return JSI_ERROR;
        }
        id = (int)vid;
        pss = sockFindPss(cmdPtr, id);
    }
    if (!pss) {
        Jsi_LogError("No such id: %d", id);
        return JSI_ERROR;
    }
    return Jsi_OptionsConf(interp, PSSOptions, argc>1?Jsi_ValueArrayIndex(interp, args, 1):NULL, pss, ret, 0);
}

static int SocketIdsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr = (SocketObj *)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr) {
        Jsi_LogError("Apply in a non-socket object");
        return JSI_ERROR;
    }
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_DSAppend(&dStr, "{", NULL);
    SocketPss *pss = NULL;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    int cnt = 0;
    for (hPtr = Jsi_HashEntryFirst(cmdPtr->pssTable, &cursor);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&cursor)) {
        pss = (SocketPss *)Jsi_HashValueGet(hPtr);
        SIGASSERT(pss, SOCKETPSS);
        if (pss->state != PSS_DEAD) {
            Jsi_DSPrintf(&dStr, "%s%d", cnt++?",":"", pss->id);
        }
    }
    Jsi_DSAppend(&dStr, "}", NULL);
    int rc = Jsi_JSONParse(interp, Jsi_DSValue(&dStr), ret, 0);
    Jsi_DSFree(&dStr);
    return rc;
}


static Jsi_CmdSpec sockCmds[] = {
    { "Socket",     SocketConstructor, 0,  1, "options:object=void", JSI_CMD_IS_CONSTRUCTOR, .help="Create socket server/client object", .opts=SockOptions, .info=FN_Socket, .retType=(uint)JSI_TT_USEROBJ },
    { "close",      SocketCloseCmd,    0,  1, "", .help="Close socket(s)", .retType=(uint)JSI_TT_VOID },
    { "conf",       SocketConfCmd,     0,  1, "options:string|object=void",.help="Configure options" , .opts=SockOptions, .retType=(uint)JSI_TT_ANY },
    { "names",      SocketIdsCmd,      0,  0, "", .help="Return list of active ids on server", .retType=(uint)JSI_TT_ARRAY },
    { "idconf",     SocketIdConfCmd,   0,  2, "id:number=void, options:string|object=void",.help="Configure options for id" , .opts=PSSOptions, .retType=(uint)JSI_TT_ANY },
    { "recv",       SocketRecvCmd,     0,  1, "id:number=void", .help="Recieve data", .retType=(uint)JSI_TT_STRING },
    { "send",       SocketSendCmd,     1,  2, "data:any, id:number=void", .help="Send a socket message to id", .info=FN_wssend, .retType=(uint)JSI_TT_VOID },
    { "update",     SocketUpdateCmd,   0,  1, "callback:function=void", .help="Service just socket events", .info=FN_wsupdate, .retType=(uint)JSI_TT_VOID },
    { NULL, .help="Commands for managing Socket server/client connections"  }
};


static Jsi_UserObjReg sockobject = {
    "Socket",
    sockCmds,
    socketObjFree,
    socketObjIsTrue,
    socketObjEqual
};

    
static int SocketConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SocketObj *cmdPtr;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    
    cmdPtr = (SocketObj *)Jsi_Calloc(1, sizeof(*cmdPtr));
    SIGINIT(cmdPtr, SOCKET);
    SIGINIT(&cmdPtr->pss, SOCKETPSS);
    cmdPtr->pss.cmdPtr = cmdPtr;
    cmdPtr->port = 9000;
    cmdPtr->pss.fd = -1;
    cmdPtr->interp = interp;
    cmdPtr->rx_buffer_size = 50000;
    cmdPtr->ws_gid = -1;
    cmdPtr->ws_uid = -1;
    cmdPtr->startTime = time(NULL);
    cmdPtr->tv.tv_sec = 1;
    cmdPtr->tv.tv_usec = 10000;
    cmdPtr->hasOpts = (arg != NULL && !Jsi_ValueIsNull(interp,arg));
    if (cmdPtr->hasOpts && Jsi_OptionsProcess(interp, SockOptions, arg, cmdPtr, 0) < 0) {
        cmdPtr->deleted = 1;
        socketObjFree(interp, cmdPtr);
        return JSI_ERROR;
    }
#if 0
    cmdPtr->info.port = (cmdPtr->client ? CONTEXT_PORT_NO_LISTEN : cmdPtr->port);
#endif

    cmdPtr->family = AF_INET;
    SockAddrAll *sap = &cmdPtr->pss.sa;
    memset(sap, 0, sizeof(*sap));
    const char *address = (cmdPtr->address ? Jsi_ValueString(interp, cmdPtr->address, NULL) : "127.0.0.1");
    sap->sin.sin_family = cmdPtr->family;
    sap->sin.sin_addr.s_addr = INADDR_ANY;
    sap->sin.sin_port = htons(cmdPtr->port); 
    
    if (sockParseHostname(address, sap, &cmdPtr->saLen, cmdPtr->port) != JSI_OK) {
        Jsi_LogError("hostname parse");
        socketObjFree(interp, cmdPtr);
        return JSI_ERROR;
    }
    sap->sin.sin_port = htons(cmdPtr->port); 
    FD_ZERO(&cmdPtr->readSet);
    FD_ZERO(&cmdPtr->writeSet);
    FD_ZERO(&cmdPtr->exceptSet);
    int fd;
    if ((fd = socket(cmdPtr->family, cmdPtr->udp ? SOCK_DGRAM : SOCK_STREAM, 0)) < 0) {
        Jsi_LogError("sock create failed");
        socketObjFree(interp, cmdPtr);
        return JSI_ERROR;
    }
    if (fd>cmdPtr->maxfd)
        cmdPtr->maxfd = fd;
    
    if (!cmdPtr->server) { // Client
        cmdPtr->pss.fd = fd;
        if (!cmdPtr->udp) {
            if (connect(fd , (struct sockaddr*)sap , sizeof(sap->sin)) < 0) {
                Jsi_LogError("sock connect failed");
                socketObjFree(interp, cmdPtr);
                return JSI_ERROR;
            }
            FD_SET(fd, &cmdPtr->readSet);
        }
    } else { // Server
        int lfd = cmdPtr->pss.fd = fd;
        if( bind(lfd, (struct sockaddr*)sap , sizeof(sap->sin)) < 0) {
            Jsi_LogError("sock bind failed");
            socketObjFree(interp, cmdPtr);
            return JSI_ERROR;
        }
        if (cmdPtr->udp == 0 && listen(lfd , 3)) {
            Jsi_LogError("sock listen failed");
            socketObjFree(interp, cmdPtr);
            return JSI_ERROR;
        }
        cmdPtr->pssTable = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, sockFreePss);
        FD_SET(lfd, &cmdPtr->readSet);
    }

    cmdPtr->event = Jsi_EventNew(interp, sockUpdate, cmdPtr);
    Jsi_Value *toacc = NULL;
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        toacc = _this;
    } else {
        Jsi_Obj *o = Jsi_ObjNew(interp);
        Jsi_PrototypeObjSet(interp, "Socket", o);
        Jsi_ValueMakeObject(interp, ret, o);
        toacc = *ret;
    }

    Jsi_Obj *fobj = Jsi_ValueGetObj(interp, toacc);
    if ((cmdPtr->objId = Jsi_UserObjNew(interp, &sockobject, fobj, cmdPtr))<0) {
        socketObjFree(interp, cmdPtr);
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_ERROR;
    }
    cmdPtr->fobj = fobj;
    return JSI_OK;
}

int Jsi_DoneSocket(Jsi_Interp *interp)
{
    Jsi_UserObjUnregister(interp, &sockobject);
    return JSI_OK;
}

int Jsi_InitSocket(Jsi_Interp *interp)
{
    Jsi_Hash *wsys;
    if (!(wsys = Jsi_UserObjRegister(interp, &sockobject))) {
        Jsi_LogFatal("Can not init socket\n");
        return JSI_ERROR;
    }

    if (!Jsi_CommandCreateSpecs(interp, sockobject.name, sockCmds, wsys, 0))
        return JSI_ERROR;
    return JSI_OK;
}

#endif //JSI_OMIT_SOCKET
#endif //JSI_LITE_ONLY
