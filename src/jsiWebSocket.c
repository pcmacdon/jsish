#ifndef JSI_LITE_ONLY
#if JSI__WEBSOCKET==1
#if JSI__MEMDEBUG
#include "jsiInt.h"
#else

#ifndef JSI_AMALGAMATION
#include "jsi.h"
#ifdef JSI_WEBSOCKET_SHARED
JSI_EXTENSION_INI
#endif
#endif

#define jsi_Sig int

#endif /* JSI__MEMDEBUG */

#include <time.h>
#include <sys/time.h>

#include <ctype.h>

#ifdef CMAKE_BUILD
#include "lws_config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>
#ifdef WIN32
#define _GET_TIME_OF_DAY_H
#ifdef EXTERNAL_POLL
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <stddef.h>

    #include "websock-w32.h"
#endif

#else /* WIN32 */
#include <syslog.h>
#include <unistd.h>
#endif /* WIN32 */

#include <signal.h>

#include <libwebsockets.h>

//#define LWS_NO_EXTENSIONS
static const int jsi_WsPkgVersion = 2;
#ifdef EXTERNAL_POLL
static int max_poll_elements;
static struct pollfd *jsi_wspollfds;
static int *fd_lookup;
static int jsi_wsnum_pollfds;
static int force_exit = 0;
#endif /* EXTERNAL_POLL */

typedef enum {  PWS_DEAD, PWS_HTTP, PWS_CONNECTED, PWS_RECV, PWS_SENT, PWS_SENDERR } pws_state;
enum { JWS_SIG_SYS=0xdeadf000, JWS_SIG_OBJ, JWS_SIG_PWS };

#ifndef NDEBUG
#ifndef _JSI_MEMCLEAR
#define _JSI_MEMCLEAR(s) memset(s, 0, sizeof(*s));
#endif
#else
#define _JSI_MEMCLEAR(s)
#endif
#define WSSIGASSERT(s,n) assert(s->sig == JWS_SIG_##n)

enum {
    /* always first */
    JWS_PROTOCOL_HTTP = 0,
    JWS_PROTOCOL_WEBSOCK,
    /* always last */
    JWS_PROTOCOL__MAX
};

#ifdef interface
#undef interface
#endif

typedef struct {
  int activeCnt;  /* Count of active objects. */ 
  int newCnt;  /* Total number of new. */ 
} ws_ObjCmd;

static ws_ObjCmd wsObjCmd = {};

static Jsi_OptionSpec wsObjCmd_Specs[] =
{
    JSI_OPT(INT,   ws_ObjCmd, activeCnt, .help="Number of active objects"),
    JSI_OPT(INT,   ws_ObjCmd, newCnt,    .help="Number of new calls"),
    JSI_OPT_END(ws_ObjCmd, .help="Options for WebSocket module")
};

typedef struct {
    int sentCnt, recvCnt, recvErrCnt, sentErrCnt, httpCnt, uploadCnt;
    time_t sentLast, recvLast, recvErrLast, sentErrLast, httpLast,
        createTime, uploadStart, uploadLast, uploadEnd, redirLast, eventLast;
    int msgQLen;
    int redirCnt;
    int eventCnt;
    int connectCnt;
    bool isBinary, isFinal;
} jsi_wsStatData;

typedef struct { /* Per server data (or client if client-mode). */
    uint sig;
    Jsi_Interp *interp;
    ws_ObjCmd *_;
    Jsi_Hash *pssTable, *handlers, *fileHash;
    Jsi_Value *onAuth, *onCloseLast, *onClose, *onFilter, *onOpen, *onRecv,
        *onUpload, *onGet, *onUnknown, *onModify, *pathAliases, *udata,
        *rootdir, *interface, *address, *mimeTypes, *extOpts, *headers, *ssiExts;
    bool client, noUpdate, noWebsock, noWarn, use_ssl, local, extHandlers, handlersPkg, inUpdate, noCompress, noConfig, echo;
    Jsi_Value* version;
    int idx;
    int port;
    uint modifySecs;
    int maxUpload;
    int maxDownload;
    int bufferPwr2;
    jsi_wsStatData stats;
    char *iface;
    const char* urlPrefix, *urlRedirect;
    const char *localhostName;
    const char *clientName;
    const char *clientIP;
    const char *useridPass, *server;
    const char *realm, *includeFile, *jsiFnPattern, *jsishPathCache;
    struct lws_context *instCtx;
    Jsi_Value *getRegexp, *post;
    unsigned int oldus;
    int opts;
    int hasOpts;
    int debug;
    int maxConnects;
    int deleted;
    int close_test;
    int createCnt;
    int redirAllCnt;
    bool redirMax;
    int redirDisable;
    int recvBufMax;
    int recvBufCnt;
    int recvBufTimeout;
    int lastRevCnt; // For update
    time_t createLast;
    time_t startTime;
    time_t lastModifyCheck, lastModifyNotify;
    char *cmdName;
    struct lws *wsi_choked[20];
    int num_wsi_choked;
    struct lws *wsi;

    struct lws_context *context;
    struct lws_context_creation_info info;
    Jsi_Event *event;
    Jsi_Obj *fobj;
    int objId;
    const char *protocol;
    struct lws_protocols protocols[JWS_PROTOCOL__MAX+1];
    int ietf_version;
    char *ssl_cert_filepath;
    char *ssl_private_key_filepath;
    int ws_uid;
    int ws_gid;
    const char *clientHost;
    const char *clientOrigin;
    const char *formParams;
    const char *curRoot;
    int sfd;        // File descriptor for http.
    Jsi_DString cName;
} jsi_wsCmdObj;

typedef struct { /* Per session connection (to each server) */
    uint sig;
    jsi_wsCmdObj *cmdPtr;
    pws_state state;
    jsi_wsStatData stats;
    struct lws *wsi;
    Jsi_HashEntry *hPtr;
    Jsi_Value* username;
    void *user;
    int cnt;
    //int fd;
    lws_fop_fd_t fop_fd;
    int wid;
    //int sfd;
    bool isWebsock, echo;
    const char *clientName;
    const char *clientIP;
    int hdrSz[200]; // Space for up to 100 headers
    int hdrNum;     // Num of above.
    time_t deferDel; // TODO: defer delete if output via SSI echo ${#}.
    // Pointers to reset.
    Jsi_DString dHdrs; // Store header string here.
    Jsi_Stack *stack;
    Jsi_DString recvBuf; // To buffer recv when recvJSON is true.
    Jsi_Value *onClose, *onFilter, *onRecv, *onUpload, *onGet, *onUnknown, *rootdir, *headers;
    char *lastData;
    char key[100]; // Lookup key.
#if (LWS_LIBRARY_VERSION_MAJOR>1)
    char filename[PATH_MAX];
    long file_length;
    struct lws_spa *spa;
    Jsi_DString resultStr, paramDS, url;
    Jsi_Value *udata, *query;
    Jsi_Value *queryObj;
    Jsi_RC resultCode;
    char **paramv;
    int paramc;
#endif
} jsi_wsPss;

typedef struct {
    int unused;
} jsi_wsUser;

typedef struct {
    Jsi_Value *val, *objVar;
    int triedLoad;
    int flags;
} jsi_wsHander;

typedef struct {
    Jsi_Value *fileVal;
    time_t loadLast, loadFirst;
    int flags;
} jsi_wsFile;

static const char* const jsi_wsparam_names[] = { "text", "send", "file", "upload" };
static const char* jsi_wsparam_str = "text,send,file,upload";

#ifndef jsi_IIOF
#define jsi_IIOF .flags=JSI_OPT_INIT_ONLY
#define jsi_IIRO .flags=JSI_OPT_READ_ONLY
#endif

static Jsi_OptionSpec WPSStats[] =
{
    JSI_OPT(INT,        jsi_wsStatData, connectCnt,   .help="Number of active connections", jsi_IIRO),
    JSI_OPT(TIME_T,     jsi_wsStatData, createTime,   .help="Time created"),
    JSI_OPT(INT,        jsi_wsStatData, eventCnt,     .help="Number of events of any type"),
    JSI_OPT(TIME_T,     jsi_wsStatData, eventLast,    .help="Time of last event of any type"),
    JSI_OPT(INT,        jsi_wsStatData, httpCnt,      .help="Number of http reqs"),
    JSI_OPT(TIME_T,     jsi_wsStatData, httpLast,     .help="Time of last http reqs"),
    JSI_OPT(BOOL,       jsi_wsStatData, isBinary,     .help="Connection recv data is binary", jsi_IIRO),
    JSI_OPT(BOOL,       jsi_wsStatData, isFinal,      .help="Final data for current message was recieved", jsi_IIRO),
    JSI_OPT(INT,        jsi_wsStatData, msgQLen,      .help="Number of messages in input queue", jsi_IIRO),
    JSI_OPT(INT,        jsi_wsStatData, recvCnt,      .help="Number of recieves", jsi_IIRO),
    JSI_OPT(TIME_T,     jsi_wsStatData, recvLast,     .help="Time of last recv", jsi_IIRO),
    JSI_OPT(TIME_T,     jsi_wsStatData, redirLast,    .help="Time of last redirect", jsi_IIRO),
    JSI_OPT(INT,        jsi_wsStatData, redirCnt,     .help="Count of redirects", jsi_IIRO),
    JSI_OPT(INT,        jsi_wsStatData, sentCnt,      .help="Number of sends", jsi_IIRO),
    JSI_OPT(TIME_T,     jsi_wsStatData, sentLast,     .help="Time of last send", jsi_IIRO),
    JSI_OPT(INT,        jsi_wsStatData, sentErrCnt,   .help="Number of sends", jsi_IIRO),
    JSI_OPT(TIME_T,     jsi_wsStatData, sentErrLast,  .help="Time of last sendErr", jsi_IIRO),
    JSI_OPT(TIME_T,     jsi_wsStatData, sentErrLast,  .help="Time of last sendErr", jsi_IIRO),
    JSI_OPT(INT,        jsi_wsStatData, uploadCnt,    .help="Number of uploads", jsi_IIRO),
    JSI_OPT(TIME_T,     jsi_wsStatData, uploadEnd,    .help="Time of upload end", jsi_IIRO),
    JSI_OPT(TIME_T,     jsi_wsStatData, uploadLast,   .help="Time of last upload input", jsi_IIRO),
    JSI_OPT(TIME_T,     jsi_wsStatData, uploadStart,  .help="Time of upload start", jsi_IIRO),
    JSI_OPT_END(jsi_wsStatData, .help="Per-connection statistics")
};

static Jsi_OptionSpec WPSOptions[] =
{
    JSI_OPT(STRKEY,     jsi_wsPss, clientIP,    .help="Client IP Address", jsi_IIRO),
    JSI_OPT(STRKEY,     jsi_wsPss, clientName,  .help="Client hostname", jsi_IIRO),
    JSI_OPT(BOOL,       jsi_wsPss, echo,        .help="LogInfo outputs all websock Send/Recv messages"),
    JSI_OPT(ARRAY,      jsi_wsPss, headers,     .help="Headers to send to browser on connection: name/value pairs"),
    JSI_OPT(BOOL,       jsi_wsPss, isWebsock,   .help="Is a websocket connection" ),
    JSI_OPT(STRBUF,     jsi_wsPss, key,         .help="String key lookup in ids command for SSI echo ${#}", jsi_IIRO),
    JSI_OPT(FUNC,       jsi_wsPss, onClose,     .help="Function to call when the websocket connection closes", .flags=0, .custom=0, .data=(void*)"ws:userobj|null, id:number"),
    JSI_OPT(FUNC,       jsi_wsPss, onGet,       .help="Function to call to server handle http-get", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, url:string, query:array"),
    JSI_OPT(FUNC,       jsi_wsPss, onUnknown,   .help="Function to call to server out content when no file exists", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, url:string, args:array"),
    JSI_OPT(FUNC,       jsi_wsPss, onRecv,      .help="Function to call when websock data recieved", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, data:string"),
    JSI_OPT(FUNC,       jsi_wsPss, onUpload,    .help="Function to call handle http-post", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, filename:string, data:string, startpos:number, complete:boolean"),
    JSI_OPT(STRING,     jsi_wsPss, rootdir,     .help="Directory to serve html from (\".\")"),
    JSI_OPT(CUSTOM,     jsi_wsPss, stats,       .help="Statistics for connection", jsi_IIRO, .custom=Jsi_Opt_SwitchSuboption, .data=WPSStats),
    JSI_OPT(ARRAY,      jsi_wsPss, query,       .help="Uri arg values for connection"),
    JSI_OPT(OBJ,        jsi_wsPss, queryObj,    .help="Uri arg values for connection as an object"),
    JSI_OPT(OBJ,        jsi_wsPss, udata,       .help="User data"),
    JSI_OPT(DSTRING,    jsi_wsPss, url,         .help="Url for connection"),
    JSI_OPT(STRING,     jsi_wsPss, username,    .help="The login userid for this connection"),
    JSI_OPT_END(jsi_wsPss, .help="Per-connection options")
};

static Jsi_OptionSpec WSOptions[] =
{
    JSI_OPT(STRING, jsi_wsCmdObj, address,    .help="In client-mode the address to connect to (127.0.0.1)" ),
    JSI_OPT(INT,    jsi_wsCmdObj, bufferPwr2, .help="Tune the recv/send buffer: value is a power of 2 in [0-20] (16)"),
    JSI_OPT(BOOL,   jsi_wsCmdObj, client,     .help="Run in client mode", jsi_IIOF),
    JSI_OPT(STRKEY, jsi_wsCmdObj, clientHost, .help="Override host name for client"),
    JSI_OPT(STRKEY, jsi_wsCmdObj, clientOrigin,.help="Override client origin (origin)"),
    JSI_OPT(INT,    jsi_wsCmdObj, debug,      .help="Set debug level. Setting this to 512 will turn on max libwebsocket log levels"),
    JSI_OPT(BOOL,   jsi_wsCmdObj, echo,       .help="LogInfo outputs all websock Send/Recv messages"),
    JSI_OPT(STRKEY, jsi_wsCmdObj, formParams, .help="Comma seperated list of upload form param names ('text,send,file,upload')", jsi_IIRO),
    JSI_OPT(BOOL,   jsi_wsCmdObj, extHandlers,.help="Setup builtin extension-handlers, ie: .htmli, .cssi, .jsi, .mdi", jsi_IIOF),
    JSI_OPT(OBJ,    jsi_wsCmdObj, extOpts,    .help="Key/value store for extension-handlers options", jsi_IIOF),
    JSI_OPT(REGEXP, jsi_wsCmdObj, getRegexp,  .help="Call onGet() only if Url matches pattern"),
//    JSI_OPT(CUSTOM, jsi_wsCmdObj, handlersPkg,.help="Handlers use package() to upgrade string to function object"),
    JSI_OPT(ARRAY,  jsi_wsCmdObj, headers,    .help="Headers to send to browser: name/value pairs", jsi_IIOF),
    JSI_OPT(STRKEY, jsi_wsCmdObj, jsiFnPattern,.help="A glob-match pattern for files to which is appended 'window.jsiWebSocket=true;' (jsig*.js)", jsi_IIRO),
    JSI_OPT(STRING, jsi_wsCmdObj, interface,  .help="Interface for server to listen on, eg. 'eth0' or 'lo'", jsi_IIOF),
    JSI_OPT(BOOL,   jsi_wsCmdObj, local,      .help="Limit connections to localhost addresses on the 127 network"),
    JSI_OPT(STRKEY, jsi_wsCmdObj, localhostName,.help="Client name used by localhost connections ('localhost')"),
    JSI_OPT(INT,    jsi_wsCmdObj, maxConnects,.help="In server mode, max number of client connections accepted"),
    JSI_OPT(INT,    jsi_wsCmdObj, maxDownload,.help="Max size of file download"),
    JSI_OPT(INT,    jsi_wsCmdObj, maxUpload,  .help="Max size of file upload will accept"),
    JSI_OPT(OBJ,    jsi_wsCmdObj, mimeTypes,  .help="Object providing map of file extensions to mime types. eg. {txt:'text/plain', bb:'text/bb'}", jsi_IIOF),
    JSI_OPT(UINT,   jsi_wsCmdObj, modifySecs, .help="Seconds between checking for modified files with onModify (2)"),
    JSI_OPT(BOOL,   jsi_wsCmdObj, noConfig,   .help="Disable use of conf() to change options after options after create", jsi_IIOF),
    JSI_OPT(BOOL,   jsi_wsCmdObj, noCompress, .help="Disable per-message-deflate extension which can truncate large msgs"),
    JSI_OPT(BOOL,   jsi_wsCmdObj, noUpdate,   .help="Disable update event-processing (eg. to exit)"),
    JSI_OPT(BOOL,   jsi_wsCmdObj, noWebsock,  .help="Serve html, but disallow websocket upgrade", jsi_IIOF),
    JSI_OPT(BOOL,   jsi_wsCmdObj, noWarn,     .help="Quietly ignore file related errors"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onAuth,     .help="Function to call for http basic authentication", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, url:string, userpass:string"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onClose,    .help="Function to call when the websocket connection closes", .flags=0, .custom=0, .data=(void*)"ws:userobj|null, id:number"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onCloseLast,.help="Function to call when last websock connection closes. On object delete arg is null", .flags=0, .custom=0, .data=(void*)"ws:userobj|null"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onFilter,   .help="Function to call on a new connection: return false to kill connection", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, url:string, ishttp:boolean"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onGet,      .help="Function to call to server handle http-get", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, url:string, query:array"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onModify,   .help="Function to call when a served-out-file is modified", jsi_IIOF, .custom=0, .data=(void*)"ws:userobj, file:string"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onOpen,     .help="Function to call when the websocket connection occurs", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onUnknown,  .help="Function to call to server out content when no file exists", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, url:string, query:array"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onUpload,   .help="Function to call handle http-post", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, filename:string, data:string, startpos:number, complete:boolean"),
    JSI_OPT(FUNC,   jsi_wsCmdObj, onRecv,     .help="Function to call when websock data recieved", .flags=0, .custom=0, .data=(void*)"ws:userobj, id:number, data:string"),
    JSI_OPT(OBJ,    jsi_wsCmdObj, pathAliases,.help="Path alias lookups", jsi_IIOF),
    JSI_OPT(INT,    jsi_wsCmdObj, port,       .help="Port for server to listen on (8080)", jsi_IIOF),
    JSI_OPT(STRING, jsi_wsCmdObj, post,       .help="Post string to serve", jsi_IIOF),
    JSI_OPT(STRKEY, jsi_wsCmdObj, protocol,   .help="Name of protocol (ws/wss)"),
    JSI_OPT(STRKEY, jsi_wsCmdObj, realm,      .help="Realm for basic auth (jsish)", ),
    JSI_OPT(INT,    jsi_wsCmdObj, recvBufMax, .help="Size limit of a websocket message", jsi_IIOF),
    JSI_OPT(INT,    jsi_wsCmdObj, recvBufTimeout,.help="Timeout for recv of a websock msg", jsi_IIOF),
    JSI_OPT(BOOL,   jsi_wsCmdObj, redirMax,   .help="Temporarily disable redirects when see more than this in 10 minutes"),
    JSI_OPT(STRING, jsi_wsCmdObj, rootdir,    .help="Directory to serve html from (\".\")"),
    JSI_OPT(STRKEY, jsi_wsCmdObj, server,     .help="String to send out int the header SERVER (jsiWebSocket)"),
    JSI_OPT(OBJ,    jsi_wsCmdObj, ssiExts,    .help="Object map of file extensions to apply SSI.  eg. {myext:true, shtml:false} ", jsi_IIOF),
    JSI_OPT(CUSTOM, jsi_wsCmdObj, stats,      .help="Statistical data", jsi_IIRO, .custom=Jsi_Opt_SwitchSuboption, .data=WPSStats),
    JSI_OPT(TIME_T, jsi_wsCmdObj, startTime,  .help="Time of websocket start", jsi_IIRO),
    JSI_OPT(STRKEY, jsi_wsCmdObj, includeFile,.help="Default file when no extension given (include.shtml)"),
    JSI_OPT(OBJ,    jsi_wsCmdObj, udata,      .help="User data"),
    JSI_OPT(STRKEY, jsi_wsCmdObj, urlPrefix,  .help="Prefix in url to strip from path; for reverse proxy"),
    JSI_OPT(STRKEY, jsi_wsCmdObj, urlRedirect,.help="Redirect when no url or / is given. Must match urlPrefix, if given"),
    JSI_OPT(BOOL,   jsi_wsCmdObj, use_ssl,    .help="Use https (for client)", jsi_IIOF),
    JSI_OPT(STRKEY, jsi_wsCmdObj, useridPass, .help="The USERID:PASSWORD to use for basic authentication"),
    JSI_OPT(OBJ,    jsi_wsCmdObj, version,    .help="WebSocket version info", jsi_IIRO),
    JSI_OPT_END(jsi_wsCmdObj, .help="Websocket options")
};

static Jsi_RC jsi_wswebsocketObjFree(Jsi_Interp *interp, void *data);
static bool jsi_wswebsocketObjIsTrue(void *data);
static bool jsi_wswebsocketObjEqual(void *data1, void *data2);
static void jsi_wswebsocketObjErase(jsi_wsCmdObj *cmdPtr);

/* this protocol server (always the first one) just knows how to do HTTP */

static int
jsi_wscallback_http(struct lws *wsi,
      enum lws_callback_reasons reason,
      void *user, void *in, size_t len);
static int
jsi_wscallback_websock(struct lws *wsi,
      enum lws_callback_reasons reason,
      void *user, void *in, size_t len);
      
static Jsi_RC jsi_wsfreeFile(Jsi_Interp *interp, Jsi_HashEntry* hPtr, void *ptr);
static bool jsi_wsAddHeader(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, struct lws *wsi, Jsi_Value *hdrs,
    Jsi_DString *hStr);
    
// Allocate per-connection data using file descriptor.
static jsi_wsPss*
jsi_wsgetPss(jsi_wsCmdObj *cmdPtr, struct lws *wsi, void *user, int create, int ishttp)
{
    Jsi_HashEntry *hPtr;
    bool isNew = 0;
    jsi_wsPss *pss = NULL;
    if (user==NULL)
        return NULL;
    int sfd = lws_get_socket_fd(wsi);
    if (sfd<0) {
        return NULL;
    }
    int sid = ((sfd<<1)|ishttp);
    if (create)
        hPtr = Jsi_HashEntryNew(cmdPtr->pssTable, (void*)(intptr_t)sid, &isNew);
    else
        hPtr = Jsi_HashEntryFind(cmdPtr->pssTable, (void*)(intptr_t)sid);

    if (hPtr && !isNew)
        pss = (typeof(pss))Jsi_HashValueGet(hPtr);

    if (!pss) {
        if (!create)
            return NULL;
        pss = (typeof(pss))Jsi_Calloc(1, sizeof(*pss));
        Jsi_HashValueSet(hPtr, pss);
        pss->sig = JWS_SIG_PWS;
        pss->hPtr = hPtr;
        Jsi_HashValueSet(hPtr, pss);
        pss->cmdPtr = cmdPtr;
        pss->wsi = wsi;
        pss->user = user; /* unused. */
        pss->state = PWS_CONNECTED;
        pss->stats.createTime = time(NULL);
        pss->cnt = cmdPtr->idx++;
        pss->wid = sid;
        //pss->sfd = sfd;
        pss->udata = Jsi_ValueNewObj(cmdPtr->interp, NULL);
        Jsi_IncrRefCount(cmdPtr->interp, pss->udata);

        if (cmdPtr->debug>2)
            fprintf(stderr, "PSS CREATE: %p/%p/%p (http=%d) = %d\n", pss, user, wsi, ishttp, sid);
        if (!ishttp) {
            pss->isWebsock = 1;
            cmdPtr->stats.connectCnt++;
            cmdPtr->createCnt++;
            cmdPtr->createLast = time(NULL);
        }
    }
    if (pss) {
        WSSIGASSERT(pss, PWS);
        pss->stats.eventCnt++;
        pss->stats.eventLast = time(NULL);
        cmdPtr->stats.eventCnt++;
        cmdPtr->stats.eventLast = time(NULL);
    }
    return pss;
}

static Jsi_RC jsi_wsDelPss(Jsi_Interp *interp, void *data) {
    Jsi_Free(data);
    return JSI_OK;
}

static Jsi_RC jsi_wsrecv_flush(jsi_wsCmdObj *cmdPtr, jsi_wsPss *pss);

static void
jsi_wsdeletePss(jsi_wsPss *pss)
{
    jsi_wsCmdObj*cmdPtr = pss->cmdPtr;
    if (pss->sig == 0)
        return;
    WSSIGASSERT(pss, PWS);
    if (pss->state == PWS_DEAD)
        return;
    if (cmdPtr && cmdPtr->debug>3)
        fprintf(stderr, "PSS DELETE: %p\n", pss);

    jsi_wsrecv_flush(cmdPtr, pss);
    if (pss->hPtr) {
        Jsi_HashValueSet(pss->hPtr, NULL);
        Jsi_HashEntryDelete(pss->hPtr);
        pss->hPtr = NULL;
    }
    Jsi_Interp *interp = cmdPtr->interp;
    if (pss->stack) {
        Jsi_StackFreeElements(interp, pss->stack, jsi_wsDelPss);
        Jsi_StackFree(pss->stack);
    }
    Jsi_DSFree(&pss->dHdrs);
    if (pss->isWebsock)
        pss->cmdPtr->stats.connectCnt--;
    Jsi_OptionsFree(cmdPtr->interp, WPSOptions, pss, 0);
    pss->state = PWS_DEAD;
    Jsi_DSFree(&pss->resultStr);
    Jsi_DSFree(&pss->paramDS);
    if (pss->lastData)
        Jsi_Free(pss->lastData);
    pss->lastData = NULL;
    if (pss->spa)
        lws_spa_destroy(pss->spa);
    Jsi_Free(pss);
}

static int jsi_wswrite(jsi_wsPss* pss, struct lws *wsi, unsigned char *buf, int len, enum lws_write_protocol proto)
{
    jsi_wsCmdObj *cmdPtr = pss->cmdPtr;
    int m = lws_write(wsi, buf, len, proto);
    if (m >= 0) {
        cmdPtr->stats.sentCnt++;
        cmdPtr->stats.sentLast = time(NULL);
        pss->stats.sentCnt++;
        pss->stats.sentLast = time(NULL);
    } else {
        pss->state = PWS_SENDERR;
        pss->stats.sentErrCnt++;
        pss->stats.sentErrLast = time(NULL);
        cmdPtr->stats.sentErrCnt++;
        cmdPtr->stats.sentErrLast = time(NULL);
    }
    return m;
}

static int jsi_wsServeHeader(jsi_wsPss *pss, struct lws *wsi, int strLen,
    int code, const char *extra, const char *mime, Jsi_DString *jStr)
{
    uchar ubuf[JSI_BUFSIZ], *p=ubuf, *end = &ubuf[sizeof(ubuf)-1];
    if (!mime) mime = "text/html";
    if (code<=0) code = 200;
    if (lws_add_http_header_status(wsi, code, &p, end))
        return -1;
    const char *srv = pss->cmdPtr->server;
    if (!srv) srv = "jsiWebSocket";
    if (srv[0] && lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_SERVER, (uchar*)srv, Jsi_Strlen(srv), &p, end))
        return -1;
    if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, (uchar *)mime, Jsi_Strlen(mime), &p, end))
        return -1;
    if (lws_add_http_header_content_length(wsi, strLen, &p, end))
        return -1;
    *p = 0;
    Jsi_DSAppend(jStr, (char*)ubuf, extra, NULL);
    p = ubuf;
    if (lws_finalize_http_header(wsi, &p, end))
        return -1;
    *p = 0;
    Jsi_DSAppend(jStr, (char*)ubuf, NULL);
    return 0;
}

static int jsi_wsServeString(jsi_wsPss *pss, struct lws *wsi,
    const char *buf, int code, const char *extra, const char *mime)
{
    int strLen = Jsi_Strlen(buf);
    Jsi_DString jStr = {};
    int rc = jsi_wsServeHeader(pss, wsi, strLen, code, extra, mime, &jStr);
    if (rc>=0) {
        Jsi_DSAppend(&jStr, buf, NULL);
        char *vStr = Jsi_DSValue(&jStr);
        int vLen = Jsi_DSLength(&jStr);
        rc = jsi_wswrite(pss, wsi, (unsigned char*)vStr, vLen, LWS_WRITE_HTTP);
    }
    Jsi_DSFree(&jStr);
    return (rc>=0?1:0);
}

static const char*
jsi_wsHeader(jsi_wsPss *pss, const char *name, int *lenPtr)
{
    int i, nlen = Jsi_Strlen(name);
    const char *ret = NULL, *cp = Jsi_DSValue(&pss->dHdrs);
    for (i=0; i<pss->hdrNum; i+=2) {
        int sz = pss->hdrSz[i];
        int mat = (!Jsi_Strncasecmp(cp, name, nlen) && cp[nlen]=='=');
        cp += 1 + sz;
        if (mat) {
            ret = cp;
            if (lenPtr)
                *lenPtr = pss->hdrSz[i+1];
            break;
        }
        cp += (1 + pss->hdrSz[i+1]);
    }
    return ret;
}


static int
jsi_wsGetHeaders(jsi_wsPss *pss, struct lws *wsi, Jsi_DString* dStr, int lens[], int hmax)
{
    int n = 0, i = 0, nlen;
    char buf[1000];
    const char *cp;
    while ((cp = (char*)lws_token_to_string((enum lws_token_indexes)n))) {
        int len = lws_hdr_copy(wsi, buf, sizeof(buf), ( enum lws_token_indexes)n);
        n++;
        if (i>=(n*2+2)) break;
        if (len<=0) continue;
        buf[sizeof(buf)-1] = 0;
        if (!buf[0]) continue;
        nlen = Jsi_Strlen(cp);
        if (nlen>0 && cp[nlen-1]==' ') nlen--;
        if (nlen>0 && cp[nlen-1]==':') nlen--;
        Jsi_DSAppendLen(dStr, cp, nlen);
        Jsi_DSAppend(dStr, "=", buf, "\n", NULL);
        if (lens) {
            lens[i++] = nlen;
            lens[i++] = Jsi_Strlen(buf);
        }
    }
    //printf("HEE: %d = %s\n", pss->wid, Jsi_DSValue(dStr) );
    return i;
}

static void jsi_wsDumpHeaders(jsi_wsCmdObj *cmdPtr, jsi_wsPss *pss, const char *name, Jsi_Value **ret)
{
    Jsi_Interp *interp = cmdPtr->interp;
    Jsi_Obj *nobj;
    Jsi_Value *nv;
    if (pss->hdrNum<=0)
        return;
    if (!name) {
        nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        Jsi_ValueMakeObject(interp, ret, nobj);
        int nsiz = Jsi_ObjArraySizer(interp, nobj, pss->hdrNum);
        if (nsiz < pss->hdrNum) {
            printf("header failed, %d != %d", nsiz, pss->hdrNum);
            return;
        }
    }
    Jsi_DString dStr = {}, vStr = {};
    int i;
    const char *nam, *val, *cp = Jsi_DSValue(&pss->dHdrs);
    for (i=0; i<pss->hdrNum; i+=2) {
        int sz = pss->hdrSz[i], sz2 = pss->hdrSz[i+1];
        Jsi_DSSetLength(&dStr, 0);
        Jsi_DSSetLength(&vStr, 0);
        nam = Jsi_DSAppendLen(&dStr, cp, sz);
        cp += 1 + sz;
        val = Jsi_DSAppendLen(&vStr, cp, sz2);
        if (name) {
            if (!Jsi_Strcmp(nam, name)) {
                Jsi_ValueMakeStringDup(interp, ret, val);
                break;
            }
        } else {
            nv = Jsi_ValueNewStringDup(interp, val);
            Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringDup(interp, nam), i);
            Jsi_ObjArraySet(interp, nobj, nv, i+1);
        }
        cp += (1 + sz2);
    }
    Jsi_DSFree(&dStr);
    Jsi_DSFree(&vStr);
}

static void jsi_wsDumpQuery(jsi_wsCmdObj *cmdPtr, jsi_wsPss *pss, const char *name, Jsi_Value **ret)
{
    int n = 0;
    Jsi_Interp *interp = cmdPtr->interp;
    Jsi_Obj *nobj;
    Jsi_Value *nv;
    if (!name) {
        nobj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
        Jsi_ValueMakeObject(interp, ret, nobj);
    }
    for (n = 0; n<pss->paramc; n++) {
        const char *cp = lws_spa_get_string(pss->spa, n);
        if (!cp) cp = "";
        if (name) {
            if (Jsi_Strcmp(name, pss->paramv[n])) { n++; continue; }
            Jsi_ValueMakeStringDup(interp, ret, cp);
            return;
        }
        nv = Jsi_ValueNewStringDup(interp, cp);
        Jsi_ObjInsert(interp, nobj, pss->paramv[n], nv, 0);
    }
}

static void jsi_wsgetUriArgValue(Jsi_Interp *interp, struct lws *wsi, Jsi_Value **vPtr, Jsi_Value **oPtr)
{
    int n = 0;
    char buf[JSI_BUFSIZ*8];
    while (lws_hdr_copy_fragment(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_URI_ARGS, n++) > 0) {
        if (!*vPtr) {
            *vPtr = Jsi_ValueNewArray(interp, NULL, 0);
            Jsi_IncrRefCount(interp, *vPtr);
        }
        Jsi_ValueArraySet(interp, *vPtr, Jsi_ValueNewStringDup(interp, buf), n-1);
        if (!*oPtr) {
            *oPtr = Jsi_ValueNewObj(interp, NULL);
            Jsi_IncrRefCount(interp, *oPtr);
        }
        char *cp = Jsi_Strchr(buf, '=');
        if (cp) {
            *cp = 0;
            Jsi_ValueInsertFixed(interp, *oPtr, buf, Jsi_ValueNewStringDup(interp, cp+1));
        } else {
            Jsi_ValueInsertFixed(interp, *oPtr, buf, Jsi_ValueNewNull(interp));
        }
    }
}

static Jsi_RC jsi_wsGetCmd(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, jsi_wsPss* pss, struct lws *wsi,
    const char *inPtr, Jsi_Value *cmd, Jsi_DString *tStr)
{
    Jsi_RC jrc;
    Jsi_Value *retStr = Jsi_ValueNew1(interp);
    // 4 args: ws, id, url, query
    Jsi_Value *vpargs, *vargs[10];
    int n = 0;
    if (cmdPtr->deleted) return JSI_ERROR;
    vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
    vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss?pss->wid:0));
    vargs[n++]  = Jsi_ValueNewStringDup(interp, inPtr);
    vargs[n++]  = (pss->query?pss->query:Jsi_ValueNewArray(interp, NULL, 0));
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, n, 0));
    Jsi_IncrRefCount(interp, vpargs);
    jrc = Jsi_FunctionInvoke(interp, cmd, vpargs, &retStr, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    if (Jsi_InterpGone(interp))
        return JSI_ERROR;
    Jsi_DString dStr = {};
    const char *rstr = "";
    if (jrc != JSI_OK)
        rstr = "Error";
    else if (Jsi_ValueIsFalse(interp, retStr)) {
        rstr = "Access denied";
        jrc = JSI_ERROR;
    } else
        rstr = Jsi_ValueString(interp, retStr, NULL);
    if (rstr && tStr && Jsi_Strncmp(rstr, "!!!", 3)==0) {
        Jsi_DSAppend(tStr, rstr+3, NULL);
        jrc = JSI_CONTINUE;
    } else if (rstr && tStr && Jsi_Strncmp(rstr, ">>>", 3)==0) {
        Jsi_DSAppend(tStr, rstr+3, NULL);
        jrc = JSI_SIGNAL;
    } else if (rstr && rstr[0] != 0)
        jsi_wsServeString(pss, wsi, rstr, jrc==JSI_OK?0:404, NULL, NULL);
    else
        jrc = JSI_BREAK;
    Jsi_DecrRefCount(interp, retStr);
    Jsi_DSFree(&dStr);
    return jrc;
}

static const char* jsw_getReasonStr(enum lws_callback_reasons reason) {
    typedef struct { enum lws_callback_reasons r; const char *name; } ssType;
    static ssType ss[] = {
#define MKLCBS(n) { n, #n }
    MKLCBS(LWS_CALLBACK_PROTOCOL_INIT),
    MKLCBS(LWS_CALLBACK_PROTOCOL_DESTROY), MKLCBS(LWS_CALLBACK_WSI_CREATE),
    MKLCBS(LWS_CALLBACK_WSI_DESTROY),MKLCBS(LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS),
    MKLCBS(LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS),
    MKLCBS(LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION),
    MKLCBS(LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY),
    MKLCBS(LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED), MKLCBS(LWS_CALLBACK_HTTP),
    MKLCBS(LWS_CALLBACK_HTTP_BODY), MKLCBS(LWS_CALLBACK_HTTP_BODY_COMPLETION),
    MKLCBS(LWS_CALLBACK_HTTP_FILE_COMPLETION), MKLCBS(LWS_CALLBACK_HTTP_WRITEABLE),
    MKLCBS(LWS_CALLBACK_CLOSED_HTTP), MKLCBS(LWS_CALLBACK_FILTER_HTTP_CONNECTION),
    MKLCBS(LWS_CALLBACK_ESTABLISHED),
    MKLCBS(LWS_CALLBACK_CLOSED),
    MKLCBS(LWS_CALLBACK_SERVER_WRITEABLE),
    MKLCBS(LWS_CALLBACK_RECEIVE), MKLCBS(LWS_CALLBACK_RECEIVE_PONG),
    MKLCBS(LWS_CALLBACK_WS_PEER_INITIATED_CLOSE), MKLCBS(LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION),
    MKLCBS(LWS_CALLBACK_CONFIRM_EXTENSION_OKAY),
    MKLCBS(LWS_CALLBACK_CLIENT_CONNECTION_ERROR),
    MKLCBS(LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH), MKLCBS(LWS_CALLBACK_CLIENT_ESTABLISHED),
    MKLCBS(LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER), MKLCBS(LWS_CALLBACK_CLIENT_RECEIVE),
    MKLCBS(LWS_CALLBACK_CLIENT_RECEIVE_PONG),
    MKLCBS(LWS_CALLBACK_CLIENT_WRITEABLE), MKLCBS(LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED),
    MKLCBS(LWS_CALLBACK_WS_EXT_DEFAULTS), MKLCBS(LWS_CALLBACK_FILTER_NETWORK_CONNECTION),
    MKLCBS(LWS_CALLBACK_GET_THREAD_ID), MKLCBS(LWS_CALLBACK_ADD_POLL_FD),
    MKLCBS(LWS_CALLBACK_DEL_POLL_FD), MKLCBS(LWS_CALLBACK_CHANGE_MODE_POLL_FD), MKLCBS(LWS_CALLBACK_LOCK_POLL),
    MKLCBS(LWS_CALLBACK_UNLOCK_POLL),
#if (LWS_LIBRARY_VERSION_MAJOR>1)
    MKLCBS(LWS_CALLBACK_CGI),
    MKLCBS(LWS_CALLBACK_CGI_TERMINATED),
    MKLCBS(LWS_CALLBACK_CGI_STDIN_DATA),
    MKLCBS(LWS_CALLBACK_CGI_STDIN_COMPLETED),
    MKLCBS(LWS_CALLBACK_SESSION_INFO),
    MKLCBS(LWS_CALLBACK_GS_EVENT),
    MKLCBS(LWS_CALLBACK_HTTP_PMO),
    MKLCBS(LWS_CALLBACK_RAW_RX),
    MKLCBS(LWS_CALLBACK_RAW_CLOSE),
    MKLCBS(LWS_CALLBACK_RAW_WRITEABLE),
    MKLCBS(LWS_CALLBACK_RAW_ADOPT),
    MKLCBS(LWS_CALLBACK_RAW_ADOPT_FILE),
    MKLCBS(LWS_CALLBACK_ADD_HEADERS),
    MKLCBS(LWS_CALLBACK_CHECK_ACCESS_RIGHTS),
    MKLCBS(LWS_CALLBACK_PROCESS_HTML),
    MKLCBS(LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP),
    MKLCBS(LWS_CALLBACK_CLOSED_CLIENT_HTTP),
    MKLCBS(LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ),
    MKLCBS(LWS_CALLBACK_RECEIVE_CLIENT_HTTP),
    MKLCBS(LWS_CALLBACK_COMPLETED_CLIENT_HTTP),
    MKLCBS(LWS_CALLBACK_CLIENT_HTTP_WRITEABLE),
    MKLCBS(LWS_CALLBACK_HTTP_BIND_PROTOCOL),
    MKLCBS(LWS_CALLBACK_HTTP_DROP_PROTOCOL),
    MKLCBS(LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION),
    MKLCBS(LWS_CALLBACK_RAW_RX_FILE),
    MKLCBS(LWS_CALLBACK_RAW_WRITEABLE_FILE),
    MKLCBS(LWS_CALLBACK_RAW_CLOSE_FILE),
    MKLCBS(LWS_CALLBACK_USER),
#endif
#if (LWS_LIBRARY_VERSION_NUMBER>=3000000)
    MKLCBS(LWS_CALLBACK_SSL_INFO),
    MKLCBS(LWS_CALLBACK_CGI_PROCESS_ATTACH),
    MKLCBS(LWS_CALLBACK_CLIENT_CLOSED),
    MKLCBS(LWS_CALLBACK_TIMER),
    MKLCBS(LWS_CALLBACK_EVENT_WAIT_CANCELLED),
    MKLCBS(LWS_CALLBACK_CHILD_CLOSING),
    MKLCBS(LWS_CALLBACK_CHILD_WRITE_VIA_PARENT),
    MKLCBS(LWS_CALLBACK_VHOST_CERT_AGING),
    MKLCBS(LWS_CALLBACK_VHOST_CERT_UPDATE),
#endif
    {(enum lws_callback_reasons)0, NULL }
    };
    int i = -1;
    while (ss[++i].name)
        if (ss[i].r == reason)
            return ss[i].name;
    return "";
}

static bool jsi_wsAddHeader(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, struct lws *wsi, Jsi_Value *hdrs,
    Jsi_DString *hStr) {
    uchar buffer[JSI_BUFSIZ];
    uchar *p = (unsigned char *)buffer, *end = p + sizeof(buffer);
    int n = 0;
    int i, hvl, argc = Jsi_ValueGetLength(interp, hdrs);
    for (i=0; i<argc; i+=2) {
        const char *hn = Jsi_ValueArrayIndexToStr(interp, hdrs, i, NULL),
            *hv = Jsi_ValueArrayIndexToStr(interp, hdrs, i+1, &hvl);
        if (hn && hv) {
            if (lws_add_http_header_by_name(wsi, (const uchar *)hn, (const uchar *)hv, hvl, &p, end))
                return false;
            n = p - buffer;
            if (n>0)
                Jsi_DSAppendLen(hStr, (char*)buffer, n);
            p = buffer;
        }
    }
    return true;
}

static bool jsi_wsAddStdHeader(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, struct lws *wsi,    Jsi_DString *hStr) {
    uchar buffer[JSI_BUFSIZ];
    uchar *p = (unsigned char *)buffer, *end = p + sizeof(buffer);
    const char *srv = cmdPtr->server;
    if (!srv) srv = "jsiWebSocket";
    int n = 0;
    if (srv[0] && lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_SERVER, (uchar*)srv, Jsi_Strlen(srv), &p, end))
        return false;
    n = p - buffer;
    if (n>0) {
        Jsi_DSAppendLen(hStr, (char*)buffer, n);
        p = buffer;
    }
    return true;
}

static Jsi_RC jsi_wsFileAdd(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, Jsi_Value *name) {
    const char *sname = Jsi_ValueString(interp, name, NULL);
    if (cmdPtr->onModify && sname) {
        bool isNew = 0;
        Jsi_HashEntry *hPtr = Jsi_HashEntryNew(cmdPtr->fileHash, sname, &isNew);
        if (hPtr) {
            jsi_wsFile* fPtr;
            if (!isNew)
                fPtr = Jsi_HashValueGet(hPtr);
            else {
                fPtr = (jsi_wsFile *)Jsi_Calloc(1, sizeof(*fPtr));
                fPtr->fileVal = name;
                fPtr->loadFirst = time(NULL);
                Jsi_IncrRefCount(interp, name);
                fPtr->flags = 0;
                Jsi_HashValueSet(hPtr, fPtr);
            }
            fPtr->loadLast = time(NULL);
        }
    }
    return JSI_OK;
}

// Read only native files inside the rootdir.
static Jsi_RC jsi_wsFileRead(Jsi_Interp *interp, Jsi_Value *name, Jsi_DString *dStr, jsi_wsCmdObj *cmdPtr, jsi_wsPss *pss) {
    Jsi_StatBuf sb;
    Jsi_RC rc = JSI_ERROR;
    int n = Jsi_Stat(interp, name, &sb);
    if (!n && sb.st_size>0) {
        char fdir[PATH_MAX];
        const char* cr = cmdPtr->curRoot, *fpath=NULL;
        if (!Jsi_FSNative(interp, name) || ((fpath= Jsi_Realpath(interp, name, fdir))
            && cr && !Jsi_Strncmp(fpath, cr, Jsi_Strlen(cr)))) {
            rc = Jsi_FileRead(interp, name, dStr);
            if (rc == JSI_OK && cmdPtr->onModify && Jsi_FSNative(interp, name))
                jsi_wsFileAdd(interp, cmdPtr, name);
        } else
            fprintf(stderr, "Skip read file %s in %s\n", Jsi_ValueString(interp, name, NULL), (cr?cr:""));
    }
    if (cmdPtr->noWarn)
        return JSI_OK;
    return rc;
}

static bool jsi_wsIsSSIExt(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, jsi_wsPss *pss, const char *ext) {
    if (cmdPtr->ssiExts) {
        Jsi_Value *mVal = Jsi_ValueObjLookup(interp, cmdPtr->ssiExts, ext, 1);
        if (mVal) {
            bool b = 0;
            if (Jsi_ValueGetBoolean(interp, mVal, &b) != JSI_OK) {
                Jsi_LogWarn("expected bool for ssiExts '%s': disabling all\n", ext);
                Jsi_DecrRefCount(interp, cmdPtr->ssiExts);
                cmdPtr->ssiExts = NULL;
            }
            return b;
        }
    }
    if (ext[0]=='s'  && (!Jsi_Strcmp(ext, "shtml")
        || !Jsi_Strcmp(ext, "scss") || !Jsi_Strcmp(ext, "sjs")))
        return 1;

    return 0;
}

// Support the limited nonstandard SSI: #include, #if, #elif#, #else, #endif
// No expr, just var lookup from query/udata.  And can not nest #if in same file.
static Jsi_RC jsi_wsEvalSSI(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, Jsi_Value *fn, Jsi_DString *dStr,
    int lvl, jsi_wsPss *pss) {
    int flen, rlen;
    char fbuf[PATH_MAX];
    char *fs, *fname = Jsi_ValueString(interp, fn, &flen), *fend = fname;
    if (lvl>10 || !fname || !pss)
        return JSI_ERROR;
    Jsi_Value *fval;
    Jsi_RC rc = JSI_OK;
    Jsi_DString tStr = {}, lStr = {};
    const char *cs = NULL;
    char *root = Jsi_ValueString(interp, cmdPtr->rootdir, &rlen);
    if (!Jsi_Strncmp(root, fname, rlen))
        fname = fname + rlen;
    fs = Jsi_Strrchr(fname, '/');
    if (fs) {
        flen = fs-fname;
        fend = fs+1;
    }
    if (lvl>0) {
        rc = jsi_wsFileRead(interp, fn, &tStr, cmdPtr, pss);
        cs = Jsi_DSValue(&tStr);
    } else {
        snprintf(fbuf, sizeof(fbuf), "%s/%.*s/%s", root, flen, fname, cmdPtr->includeFile);
        fval = Jsi_ValueNewStringConst(interp, fbuf, -1);
        Jsi_IncrRefCount(interp, fval);
        rc = jsi_wsFileRead(interp, fval, &tStr, cmdPtr, pss);
        if (rc == JSI_OK)
            cs = Jsi_DSValue(&tStr);
        Jsi_DecrRefCount(interp, fval);
    }
    
    char *cp, *sp, *se, pref[] = "<!--#", suffix[] = "-->", *msg = NULL;
    struct {
        int inif, inelse, matched, elide;
    } II[11] = {};
    const int maxNest = 10;
    int ii = 0;
    int plen = 5, elen, llen;

    while (rc == JSI_OK && cs) {
        char *ext = NULL, *sfname = fname;
        int sflen = flen;
        sp = Jsi_Strstr(cs, pref);
        if (!sp || !(se=Jsi_Strstr(sp+plen, suffix))) {
            Jsi_DSAppend(dStr, cs, NULL);
            break;
        }
        sp += plen-1;
        llen = se-sp;
        Jsi_DSSetLength(&lStr, 0);
        cp = Jsi_DSAppendLen(&lStr, sp, llen);
        if (Jsi_Strchr(cp, '\n')) { rc = Jsi_LogError("unexpected newline in directive \"%.10s\"", cp); break; }
        if (!II[ii].elide)
            Jsi_DSAppendLen(dStr, cs, sp-cs-4);
        
        if (!Jsi_Strncmp(cp, "#include file=\"", 12) || !Jsi_Strncmp(cp, "#include virtual=\"", 15)) {
            if (cp[llen-1] != '"' || cp[llen-2] == '=') { msg = "missing end quote in"; break; }
            if (!II[ii].elide) {
                Jsi_DSSetLength(&lStr, llen-1);
                int isvirt = (cp[9]=='v');
                cp += (isvirt ? 18 : 15);
                if (cp[0] == '$' && lvl == 0) { // substitute file suffix
                    char sfx[20] = {};
                    uint i;
                    for (i=0; i<sizeof(sfx); i++) {
                        if ((sfx[i] = cp[i+1]) == '"' || !sfx[i]) {
                            sfx[i] = 0;
                            break;
                        }
                    }
                    snprintf(fbuf, sizeof(fbuf), "%s/%.*s/%s/%s%s", root, flen, fname, sfx, fend, sfx);
                } else {
                    snprintf(fbuf, sizeof(fbuf), "%s/%.*s/%s", root, sflen, sfname, cp);
                    ext = Jsi_Strrchr(fbuf, '.');
                }
                fval = Jsi_ValueNewStringConst(interp, fbuf, -1);
                Jsi_IncrRefCount(interp, fval);
                if (!ext || ext[0] != '.' || !jsi_wsIsSSIExt(interp, cmdPtr, pss, ext+1))
                    rc = jsi_wsFileRead(interp, fval, dStr, cmdPtr, pss);
                else
                    rc = jsi_wsEvalSSI(interp, cmdPtr, fval, dStr, lvl+1, pss);
                if (cmdPtr->noWarn)
                    rc = JSI_OK;
                Jsi_DecrRefCount(interp, fval);
            }
        } else if (!Jsi_Strncmp(cp, "#echo \"${", 9)) {
            if (cp[llen-1] != '"' || cp[llen-2] != '}') { msg = "missing end quote"; break; }
            Jsi_DSSetLength(&lStr, llen-2);
            cp += 9;
            llen -= 9;
            if (!Jsi_Strcmp(cp, "#")) {
                if (!pss->key[0])
                    snprintf(pss->key, sizeof(pss->key), "%d%p%d", pss->wid, pss, (int)cmdPtr->startTime);
                Jsi_DSPrintf(dStr, "'%s'", pss->key);
            } else {
                Jsi_Value *val = NULL;
                if (!cmdPtr->udata) {
                    val = Jsi_ValueObjLookup(interp, cmdPtr->udata, cp, 0);
                    if (!val) { msg = "udata lookup failure"; break; }
                    cp = Jsi_ValueString(interp, val, NULL);
                    Jsi_DSPrintf(dStr, "'%s'", cp);
                }
            }

        } else if (!Jsi_Strncmp(cp, "#if expr=\"", 10) || !Jsi_Strncmp(cp, "#elif expr=\"", 12)) {
            if (llen<11 || cp[llen-1] != '"' || cp[llen-2] == '=') { msg = "missing end quote"; break; }
            Jsi_DSSetLength(&lStr, llen-1);
            bool iselif = (cp[1]=='e');
            cp += (iselif?12:10);
            if (!iselif) {
                if (II[ii].inif) {
                    if ((ii+1)>=maxNest) { msg = "nested \"#if\" too deep"; break; }
                    ii++;
                    II[ii] = II[maxNest];
                    II[ii].elide = II[ii-1].elide;
                }
                II[ii].inif = 1;
            } else {
                if (!II[ii].inif) { msg = "unexpected \"#elif\""; break; }
            }
            elen = Jsi_Strlen(cp);
            if (elen<4|| cp[0] != '$' || cp[1] != '{' || cp[elen-1] != '}') {
                msg = "expr must be of form ${X}"; break;
            }
            Jsi_DSSetLength(&lStr, llen-2);
            cp += 2;
            // Extract modifiers before bool var name to lookup.
            bool warn = 0, req = 0, nifval = 0, not = 0, isq=0, isu=0, qfirst=0;
            while (*cp &&  !isalpha(*cp)) {
                bool fail = 0;
                switch (*cp) {
                    case '~': qfirst = 1; break;
                    case ':': isu = 1; break;
                    case '?': isq = 1; break;
                    case '@': warn = !II[ii].matched; break;
                    case '*': req = !II[ii].matched; break;
                    case '!': not = 1; break;
                    default: fail=1; break;
                }
                if (fail) { msg = "modifier must be one of: !:=?@*"; break; }
                cp++;
            }
            Jsi_Value *val = NULL;
            if (!val && qfirst && pss->queryObj)
                val = Jsi_ValueObjLookup(interp, pss->queryObj, cp, 0);
            if (!val && !isq && cmdPtr->udata)
                val = Jsi_ValueObjLookup(interp, cmdPtr->udata, cp, 0);
            if (!val && !qfirst && !isu && pss->queryObj)
                val = Jsi_ValueObjLookup(interp, pss->queryObj, cp, 0);
            if (!val) {
                if (req) { msg = "symbol not found"; break; }
                if (warn) Jsi_LogWarn("symbol \"%s\" not found: %s", cp, fbuf);
            } else if (Jsi_ValueGetBoolean(interp, val, &nifval) != JSI_OK) {
                const char *valStr = NULL;
                if (val) valStr = Jsi_ValueString(interp, val, NULL);
                if (!valStr || Jsi_GetBool(interp, valStr, &nifval) != JSI_OK) {
                    if (!warn) { msg = "symbol not a boolean"; break; }
                    Jsi_LogWarn("symbol \"%s\" should be a boolean: %s", cp, fbuf);
                }
            }
            if (not) nifval = !nifval;
            if (!iselif) {
                if (nifval)
                    II[ii].matched = 1;
                else
                    II[ii].elide = 1;
            } else {
                if (II[ii].matched || !nifval)
                    II[ii].elide = 1;
                else if (nifval) {
                    II[ii].matched = 1;
                    II[ii].elide = (ii?II[ii-1].elide:0);
                }
            }
        } else if (!Jsi_Strncmp(cp, "#else", 5)) {
            if (!II[ii].inif || II[ii].inelse) { msg = "unexpected \"#else\""; break; }
            II[ii].inelse = 1;
            II[ii].elide = (ii&&II[ii-1].elide?1:II[ii].matched);
        } else if (!Jsi_Strncmp(cp, "#endif", 6)) {
            if (!II[ii].inif) { msg = "unexpected \"#endif\"";  break; }
            II[ii].inelse = II[ii].inif = II[ii].elide = II[ii].matched = 0;
            if (ii>0)
                ii--;
        } else {
            msg = "expected directive #include/#if/#elif/#else/#endif";
            break;
        }
        cs = se + 3;
        if (*cs == '\n')
            cs++;
    }
    if (rc == JSI_OK && II[ii].inif && !msg) {
         msg = "missing \"#endif\"";
         sp = "";
    }
    if (msg) {
        while (*fname=='/') fname++;
        rc = Jsi_LogError("SHTML Error in \"%s\": %s: at \"%.40s\" ", fname, msg, sp);
    }
    Jsi_DSFree(&tStr);
    Jsi_DSFree(&lStr);
    return rc;

}

static void jsi_wsPathAlias(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, const char **inPtr, Jsi_DString *dStr) {
    const char *ce, *cp = NULL;
    char *lcp;
    Jsi_Value *val;
    if (cmdPtr->pathAliases) {
        cp = *inPtr;
        if (*cp == '/') cp++;
        if ((ce = Jsi_Strchr(cp, '/'))) {
            int len = ce-cp;
            Jsi_DSSetLength(dStr, 0);
            Jsi_DSAppendLen(dStr, cp, len);
            cp = Jsi_DSValue(dStr);
            if ((val = Jsi_ValueObjLookup(interp, cmdPtr->pathAliases, cp, 0)) &&
                (cp = Jsi_ValueString(interp, val, NULL))) {
                *inPtr += (len+2);
                cmdPtr->curRoot = cp;
                return;
            }
        }
    }
    if (!Jsi_Strncmp(*inPtr, "/jsi/", 5)) {
        // Get/cache path for system load file, eg /zvfs/lib/Jsish.jsi.
        if (!(cp = cmdPtr->jsishPathCache)) {
            Jsi_PkgRequire(interp, "Jsish", 0);
            if (Jsi_PkgVersion(interp, "Jsish", &cp)>=0)
                cmdPtr->jsishPathCache = cp;
        }
        if (cp) {
            Jsi_DSSetLength(dStr, 0);
            Jsi_DSAppend(dStr, cp, NULL);
            cp = Jsi_DSValue(dStr);
            if ((lcp = Jsi_Strrchr(cp, '/'))) {
                *lcp = 0;
                *inPtr += 5;
                cmdPtr->curRoot = cp;
            }
        }
    }
}

static Jsi_RC WebSocketUnaliasCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr)
        return Jsi_LogError("Apply to non-websock object");
    int vlen, nlen;
    const char *kstr, *vstr, *nstr = Jsi_ValueArrayIndexToStr(interp, args, 0, &nlen);
    if (!nstr)
        return Jsi_LogError("arg 1: expected string");
    Jsi_Value *v, *a = cmdPtr->pathAliases;
    if (!a|| !Jsi_ValueIsObjType(interp, a, JSI_OT_OBJECT)) return JSI_OK;
    Jsi_IterObj *io = Jsi_IterObjNew(interp, NULL);
    Jsi_IterGetKeys(interp, cmdPtr->pathAliases, io, 0);
    uint i;
    for (i=0; i<io->count; i++) {
        kstr = io->keys[i];
        v = Jsi_ValueObjLookup(interp, a, kstr, 1);
        if (!v) continue;
        vstr = Jsi_ValueToString(interp, v, &vlen);
        if (!vstr) continue;
        if (nlen<=vlen) continue;
        if (Jsi_Strncmp(vstr, nstr, vlen)) continue;
        Jsi_DString dStr = {};
        Jsi_DSAppend(&dStr, "/", kstr, nstr+vlen, NULL);
        Jsi_ValueFromDS(interp, &dStr, ret);
        break;
    }
    Jsi_IterObjFree(io);
    return JSI_OK;
}

int
jsi_ws_http_redirect(struct lws *wsi, int code, Jsi_DString *tStr, 
                  unsigned char **p, unsigned char *end)
{
    char *loc = Jsi_DSValue(tStr);
    uchar *start = *p;
    char* cookie = Jsi_Strchr(loc, '|');
    if (cookie) { *cookie= 0; cookie++; }

    if (lws_add_http_header_status(wsi, code, p, end)
        || lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_LOCATION, (uchar *)loc, Jsi_Strlen(loc), p, end)
        || lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,(uchar *)"text/html", 9, p,end)
        || lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_LENGTH, (uchar *)"0", 1, p, end))
        return -1;
    if (cookie && lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_SET_COOKIE, (uchar *)cookie, Jsi_Strlen(cookie), p, end))
        return -1;
    if (lws_finalize_http_header(wsi, p, end))
        return -1;
    return lws_write(wsi, start, *p - start, LWS_WRITE_HTTP_HEADERS);
}

// Handle http GET/POST
static int jsi_wsHttp(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, struct lws *wsi, void *user,
    struct lws_context *context, const char* inPtr, Jsi_DString *tStr, jsi_wsPss *pss)
{
    const char *ext = NULL;
    unsigned char buffer[JSI_BUFSIZ];
    const char *mime = NULL;
    time_t now = time(NULL);
    char buf[JSI_BUFSIZ];
    int rc = 0;
    buf[0] = 0;
    uchar *p = buffer, *end = &buffer[sizeof(buffer)-1];
    int n;
    Jsi_Value* fname = NULL;
    bool isJsiWeb = 0, isSSI = 0;
    cmdPtr->stats.httpLast = now;
    
    /* if a legal POST URL, let it continue and accept data */
    if (lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI))
        return 0;
    if (!pss)
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 1);

    int uplen=(cmdPtr->urlPrefix?Jsi_Strlen(cmdPtr->urlPrefix):0);

    if (inPtr && cmdPtr->urlPrefix && !Jsi_Strncmp(inPtr, cmdPtr->urlPrefix, uplen))
        inPtr += uplen;

    if (cmdPtr->redirDisable) {// Try to defray redirect loops.
        if (difftime(now, cmdPtr->stats.redirLast)>=600)
            cmdPtr->redirDisable = 0;
        else
            cmdPtr->redirDisable--;
    }

    if ((cmdPtr->urlRedirect && (inPtr == 0 || *inPtr == 0 || !Jsi_Strcmp(inPtr, "/")) && !cmdPtr->redirDisable)
        && (inPtr = cmdPtr->urlRedirect) && inPtr[0]) {
        cmdPtr->stats.redirCnt++;
        // TODO: system time change can disrupt the following.
        if (cmdPtr->redirMax>0 && !cmdPtr->redirDisable && cmdPtr->redirMax>0 && cmdPtr->stats.redirLast
            && difftime(now, cmdPtr->stats.redirLast)<600 && ++cmdPtr->redirAllCnt>cmdPtr->redirMax)
            cmdPtr->redirDisable = 100;
        cmdPtr->stats.redirLast = now;
        rc = lws_http_redirect(wsi, 301, (uchar*)inPtr, Jsi_Strlen(inPtr), &p, end);
        return (rc == 100 ? 0 : 1);
    }
    if (!inPtr || !*inPtr)
        inPtr = "/";

    if (cmdPtr->useridPass || cmdPtr->onAuth) {
        int ok = 0;
        int alen;
        const char *auth = jsi_wsHeader(pss, "authorization", &alen);
        if (auth && !Jsi_Strncasecmp(auth, "basic ", 6) && !cmdPtr->deleted) {
            auth += 6;
            Jsi_DString eStr = {}, bStr = {};
            Jsi_DSAppendLen(&eStr, auth, alen - 6);
            Jsi_Base64(Jsi_DSValue(&eStr), -1, &bStr, 1);
            const char *bp = Jsi_DSValue(&bStr);
            if (bp && bp[0]) {
                if (!cmdPtr->onAuth)
                    ok = (!Jsi_Strcmp(cmdPtr->useridPass, bp));
                else {
                    /* Pass 4 args: ws, id, url and userid:pass . */
                    Jsi_Obj *oarg1;
                    Jsi_Value *vpargs, *vargs[10];
                    int n = 0;
                    vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
                    vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->wid));
                    vargs[n++] = Jsi_ValueNewStringDup(interp, inPtr);
                    vargs[n++] = Jsi_ValueNewStringDup(interp, bp);
                    vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
                    Jsi_IncrRefCount(interp, vpargs);
                    Jsi_Value *ret = Jsi_ValueNew1(interp);
                    bool rb = 0;
                    rc = Jsi_FunctionInvoke(interp, cmdPtr->onAuth, vpargs, &ret, NULL);
                    if (rc == JSI_OK)
                        rb = !Jsi_ValueIsFalse(interp, ret);

                    Jsi_DecrRefCount(interp, vpargs);
                    Jsi_DecrRefCount(interp, ret);

                    if (rc != JSI_OK) {
                        Jsi_LogError("websock bad rcv eval");
                        return -1;
                    }
                    ok = rb;
                }
            }
            Jsi_DSFree(&eStr);
            Jsi_DSFree(&bStr);
        }
        if (!ok) {
            const char *realm = (cmdPtr->realm?cmdPtr->realm:"jsish");
            int n = snprintf(buf, sizeof(buf), "Basic realm=\"%s\"", realm);
            if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_WWW_AUTHENTICATE,
                    (unsigned char *)buf, n, &p, end))
                return -1;
            if (jsi_wsServeString(pss, wsi, "Password is required to access this page", 401, (char*)buffer, NULL)<0)
                return -1;
            return lws_http_transaction_completed(wsi);
        }
    }

    if (cmdPtr->onGet || pss->onGet) {
        Jsi_RC jrc;
        int rrv = 1;
        if (cmdPtr->getRegexp) {
            rrv = 0;
            jrc = Jsi_RegExpMatch(interp, cmdPtr->getRegexp, inPtr, &rrv, NULL);
            if (jrc != JSI_OK)
                return -1; // Error in regexp.
        }
        if (rrv) {
            jrc = jsi_wsGetCmd(interp, cmdPtr, pss, wsi, inPtr, pss->onGet?pss->onGet:cmdPtr->onGet, tStr);
            switch (jrc) {
                case JSI_ERROR: return -1;
                case JSI_OK: return 0;
                case JSI_SIGNAL:
                    return jsi_ws_http_redirect(wsi, 302, tStr, &p, end);
                case JSI_CONTINUE:
                    inPtr = Jsi_DSValue(tStr); break;
                case JSI_BREAK: break;
                default: break;
            }
        }
    }
    ext = Jsi_Strrchr(inPtr, '.');

    Jsi_Value *rdir = (pss->rootdir?pss->rootdir:cmdPtr->rootdir);
    cmdPtr->curRoot = (rdir?Jsi_ValueString(cmdPtr->interp, rdir, NULL):"./");
    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    jsi_wsPathAlias(interp, cmdPtr, &inPtr, &sStr);

    snprintf(buf, sizeof(buf), "%s/%s", cmdPtr->curRoot, inPtr);
    Jsi_DSFree(&sStr);
    if (cmdPtr->debug>1)
        fprintf(stderr, "FILE: %s in %s | %s\n", buf, cmdPtr->curRoot, Jsi_ValueString(interp, cmdPtr->rootdir, NULL));
    char extBuf[100], *cpde = Jsi_Strrchr(buf, '/');
    isJsiWeb = (cpde && cmdPtr->jsiFnPattern && Jsi_GlobMatch(cmdPtr->jsiFnPattern, cpde+1, 0));
    bool isgzip = 0;
    if (!ext || !ext[1])
        mime = "text/html";
    else {
        const char *eext = ext+1;
        uint elen = Jsi_Strlen(ext);
        if (elen>3 && elen<(sizeof(extBuf)-10) && !Jsi_Strcmp(ext+elen-3,".gz")) {
            Jsi_Strcpy(extBuf, ext);
            extBuf[elen-3] = 0;
            char *ext2 = Jsi_Strrchr(extBuf, '.');
            if (ext2) {
                isgzip = 1;
                ext = ext2;
            }
        }
        Jsi_HashEntry *hPtr;

        if (cmdPtr->mimeTypes) {
            /* Lookup mime type in mimeTypes object. */
            Jsi_Value *mVal = Jsi_ValueObjLookup(interp, cmdPtr->mimeTypes, ext+1, 1);
            if (mVal)
                mime = Jsi_ValueString(interp, mVal, NULL);
        }
        if (!mime) {
            static const char* mtypes[] = {
                "html", "text/html", "js", "application/x-javascript",
                "css", "text/css", "png", "image/png", "ico", "image/icon",
                "gif", "image/gif", "jpeg", "image/jpeg",
                "jpg", "image/jpeg", "svg", "image/svg+xml",
                "json", "application/json", "txt", "text/plain",
                "jsi", "application/x-javascript", "cssi", "text/css",
                "shtml", "text/html",  "scss", "text/css",
                "sjs", "application/x-javascript",
                0, 0
            };
            mime = "text/html";
            int i;
            for (i=0; mtypes[i]; i+=2)
                if (tolower(*eext) == mtypes[i][0] && !Jsi_Strncasecmp(eext, mtypes[i], -1)) {
                    mime = mtypes[i+1];
                    break;
                }
        }

        isSSI = jsi_wsIsSSIExt(interp, cmdPtr, pss, eext);

        if ((hPtr = Jsi_HashEntryFind(cmdPtr->handlers, ext)) && !cmdPtr->deleted) {
            /* Use interprete html eg. using jsi_wpp preprocessor */
            Jsi_DString jStr = {};
            Jsi_Value *vrc = NULL;
            int hrc = 0, strLen, evrc, isalloc=0;
            char *vStr, *hstr = NULL;
            jsi_wsHander *hdlPtr = (jsi_wsHander*)Jsi_HashValueGet(hPtr);
            Jsi_Value *hv = hdlPtr->val;

            if (Jsi_Strchr(buf, '\'') || Jsi_Strchr(buf, '\"')) {
                jsi_wsServeString(pss, wsi, "Can not handle quotes in url", 404, NULL, NULL);
                return -1;
            }
            cmdPtr->handlersPkg=1;

            // Attempt to load package and get function.
            if ((hdlPtr->flags&1) && cmdPtr->handlersPkg && Jsi_ValueIsString(interp, hv)
                && ((hstr = Jsi_ValueString(interp, hv, NULL)))) {
                vrc = Jsi_NameLookup(interp, hstr);
                if (!vrc) {
                    Jsi_Number pver = Jsi_PkgRequire(interp, hstr, 0);
                    if (pver >= 0)
                        vrc = Jsi_NameLookup(interp, hstr);
                }
                if (!vrc || !Jsi_ValueIsFunction(interp, vrc)) {
                    if (vrc)
                        Jsi_DecrRefCount(interp, vrc);
                    Jsi_LogError("Failed to autoload handle: %s", hstr);
                    jsi_wsServeString(pss, wsi, "Failed to autoload handler", 404, NULL, NULL);
                    return -1;
                }
                if (hdlPtr->val)
                    Jsi_DecrRefCount(interp, hdlPtr->val);
                hdlPtr->val = vrc;
                Jsi_IncrRefCount(interp, vrc);
                hv = vrc;
            }

            if ((hdlPtr->flags&2) && !hdlPtr->triedLoad && !hdlPtr->objVar && Jsi_ValueIsFunction(interp, hv)) {
                // Run command and from returned object get the parse function.
                hdlPtr->triedLoad = 1;
                Jsi_DSAppend(&jStr, "[null]", NULL);
                Jsi_DSAppend(&jStr, "]", NULL);
                vrc = Jsi_ValueNew1(interp);
                evrc = Jsi_FunctionInvokeJSON(interp, hv, Jsi_DSValue(&jStr), &vrc);
                if (Jsi_InterpGone(interp))
                    return -1;
                if (evrc != JSI_OK || !vrc || !Jsi_ValueIsObjType(interp, vrc, JSI_OT_OBJECT)) {
                    Jsi_LogError("Failed to load obj: %s", hstr);
                    jsi_wsServeString(pss, wsi, "Failed to load obj", 404, NULL, NULL);
                    return -1;
                }
                Jsi_Value *fvrc = Jsi_ValueObjLookup(interp, vrc, "parse", 0);
                if (!fvrc || !Jsi_ValueIsFunction(interp, fvrc)) {
                    Jsi_LogError("Failed to find parse: %s", hstr);
                    jsi_wsServeString(pss, wsi, "Failed to find parse", 404, NULL, NULL);
                    return -1;
                }
                hdlPtr->objVar = fvrc;
                Jsi_IncrRefCount(interp, fvrc);
                hv = vrc;

            }

            if (hdlPtr->objVar) {  // Call the obj.parse function.
                Jsi_DSAppend(&jStr, "[\"", buf, "\"]", NULL); // TODO: JSON encode.
                vrc = Jsi_ValueNew1(interp);
                evrc = Jsi_FunctionInvokeJSON(interp, hdlPtr->objVar, Jsi_DSValue(&jStr), &vrc);
                isalloc = 1;
            }
            else if (Jsi_ValueIsFunction(interp, hv)) {
                //printf("CNCNN: %s\n", Jsi_DSValue(&cmdPtr->cName));
                Jsi_DSAppend(&jStr, "[\"", buf, "\", {wsName:\"", Jsi_DSValue(&cmdPtr->cName), "\"", "}]", NULL); // TODO: JSON encode.
                vrc = Jsi_ValueNew1(interp);
                evrc = Jsi_FunctionInvokeJSON(interp, hv, Jsi_DSValue(&jStr), &vrc);
                isalloc = 1;
            } else {
                // One shot invoke of string command.
                hstr = Jsi_ValueString(interp, hv, NULL);
                Jsi_DSAppend(&jStr, hstr, "('", buf, "');", NULL);
                evrc = Jsi_EvalString(interp, Jsi_DSValue(&jStr), JSI_EVAL_RETURN);
                if (evrc == JSI_OK)
                    vrc = Jsi_InterpResult(interp);
            }
            // Take result from vrc and return it.
            if (evrc != JSI_OK) {
                Jsi_LogError("failure in websocket handler");
            } else if ((!vrc) ||
                (!(vStr = Jsi_ValueString(interp, vrc, &strLen)))) {
                Jsi_LogError("failed to get result");
            } else {
                hrc = jsi_wsServeString(pss, wsi, vStr, 0, NULL, mime);
            }
            Jsi_DSFree(&jStr);
            if (isalloc)
                Jsi_DecrRefCount(interp, vrc);
            if (hrc<=0)
                return -1;
            return 1;
        }
    }
    if (!buf[0]) {
        if (cmdPtr->debug)
            fprintf(stderr, "empty file: %s\n", inPtr);
        return -1;
    }
    fname = Jsi_ValueNewStringDup(interp, buf);
    Jsi_IncrRefCount(interp, fname);

    Jsi_DString hStr = {};
    Jsi_StatBuf jsb;
    bool native = Jsi_FSNative(interp, fname);
    if ((native && Jsi_InterpSafe(interp) && Jsi_InterpAccess(interp, fname, JSI_INTACCESS_READ) != JSI_OK) ||
        (Jsi_Stat(interp, fname, &jsb) || jsb.st_size<=0)) {
nofile:
        if (cmdPtr->onUnknown || pss->onUnknown) {
            Jsi_Value *uk = (pss->onUnknown?pss->onUnknown:cmdPtr->onUnknown);
            Jsi_RC jrc = jsi_wsGetCmd(interp, cmdPtr, pss, wsi, inPtr, uk, NULL);
            if (jrc == JSI_ERROR)
                goto bail;
            if (jrc == JSI_OK)
                goto done;
        }

        if (0 && Jsi_Strstr(buf, "favicon.ico"))
            rc = jsi_wsServeString(pss, wsi, "data:;base64,iVBORw0KGgo=", 200, NULL, "image/icon");
        else {
            const char *cp = Jsi_Strrchr(buf,'/');
            if (cp && cp[1]) {
                char statPath[PATH_MAX];
                snprintf(statPath, sizeof(statPath), "/zvfs/lib/web%s", cp);
                Jsi_DecrRefCount(interp, fname);
                fname = Jsi_ValueNewStringDup(interp, statPath);
                Jsi_IncrRefCount(interp, fname);
                if (!Jsi_Stat(interp, fname, &jsb) && jsb.st_size>0) {
                    native = 0;
                    goto serve;
                }
            }
            if (cmdPtr->noWarn==0 && !Jsi_Strstr(buf, "favicon.ico"))
                fprintf(stderr, "failed open file for read: %s\n", buf);
            rc = jsi_wsServeString(pss, wsi, "<b style='color:red'>ERROR: can not serve file!</b>", 404, NULL, NULL);
        }
        Jsi_DecrRefCount(interp, fname);
        goto done;
    }
    if (!ext || isSSI)
        goto serve;
    if (S_ISDIR(jsb.st_mode)) {
        if (cmdPtr->noWarn==0)
            fprintf(stderr, "can not serve directory: %s\n", buf);
        rc = jsi_wsServeString(pss, wsi, "<b style='color:red'>ERROR: can not serve directory!</b>", 404, NULL, NULL);
        Jsi_DecrRefCount(interp, fname);
        goto done;
    }

serve:
    n = 0;
    // TODO: add automatic cookie mgmt?
/*
    if (!strcmp((const char *)in, "/") &&
       !lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_COOKIE)) {
        gettimeofday(&tv, NULL);
        n = sprintf(b64, "test=LWS_%u_%u_COOKIE;Max-Age=360000",
            (unsigned int)tv.tv_sec,
            (unsigned int)tv.tv_usec);

        if (lws_add_http_header_by_name(wsi,
            (unsigned char *)"set-cookie:",
            (unsigned char *)b64, n, &p,
            (unsigned char *)buffer + sizeof(buffer)))
            return 1;
    }*/
    static const char stsStr[] = "max-age=15768000 ; includeSubDomains";
    if (lws_is_ssl(wsi) && lws_add_http_header_by_name(wsi,
                    (uchar *) "Strict-Transport-Security:",
                    (uchar *) stsStr,
                    sizeof(stsStr)-1, &p, (uchar *)buffer + sizeof(buffer)))
        goto bail;
    n = p - buffer;
    if (n>0)
        Jsi_DSAppendLen(&hStr, (char*)buffer, n);
    p = buffer;

    if (isgzip) {
        if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_ENCODING,
                    (unsigned char *)"gzip", n, &p, end))
            goto bail;
    }
    if (cmdPtr->headers && !jsi_wsAddHeader(interp, cmdPtr, wsi, cmdPtr->headers, &hStr))
        goto bail;

    if (pss->headers && !jsi_wsAddHeader(interp, cmdPtr, wsi, pss->headers, &hStr))
        goto bail;

    n = Jsi_DSLength(&hStr);

    if (native && !isSSI && !isJsiWeb) {

        if (!jsi_wsAddStdHeader(interp, cmdPtr, wsi, &hStr)) {
            Jsi_DecrRefCount(interp, fname);
            goto bail;
        }
        int hrc = lws_serve_http_file(wsi, buf, mime, Jsi_DSValue(&hStr), Jsi_DSLength(&hStr));
        if (hrc >= 0 && cmdPtr->onModify)
            jsi_wsFileAdd(interp, cmdPtr, fname);
        Jsi_DecrRefCount(interp, fname);
        if (hrc<0) {
            if (cmdPtr->noWarn==0)
                fprintf(stderr, "can not serve file (%d): %s\n", hrc, buf);
            goto bail;
        } else if (hrc > 0 && lws_http_transaction_completed(wsi))
            goto bail;
    } else {
        // Need to read data for non-native files.
        Jsi_DString dStr = {}, fStr = {};
        if (isSSI)
            rc = jsi_wsEvalSSI(interp, cmdPtr, fname, &fStr, 1, pss);
        else {
            rc = jsi_wsFileRead(interp, fname, &fStr, cmdPtr, pss);
            if (isJsiWeb)
                Jsi_DSAppend(&fStr, "\nwindow.jsiWebSocket=true;", NULL);
        }
        if (rc != JSI_OK) {
            Jsi_DSFree(&fStr);
            goto nofile;
        }
        int hrc = jsi_wsServeHeader(pss, wsi, (int)Jsi_DSLength(&fStr), 200, Jsi_DSValue(&hStr), mime, &dStr);
        if (hrc>=0) {
            Jsi_DSAppendLen(&dStr, Jsi_DSValue(&fStr), Jsi_DSLength(&fStr));
            char *strVal = Jsi_DSValue(&dStr);
            int strLen = Jsi_DSLength(&dStr);
            hrc = jsi_wswrite(pss, wsi, (unsigned char*)strVal, strLen, LWS_WRITE_HTTP);
        }
        Jsi_DecrRefCount(interp, fname);
        Jsi_DSFree(&dStr);
        Jsi_DSFree(&fStr);
        if (hrc<0) {
            if (cmdPtr->noWarn==0)
                fprintf(stderr, "can not serve data (%d): %s\n", hrc, buf);
            goto bail;
        } else if (hrc > 0 && lws_http_transaction_completed(wsi))
            goto bail;

    }
done:
    Jsi_DSFree(&hStr);
    return rc;

bail:
    rc = 1;
    goto done;
}

static Jsi_RC jsi_wsrecv_callback(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, jsi_wsPss *pss,
    const char *inPtr, int nlen, bool isClose)
{
    Jsi_Value *vpargs, *vargs[10];
    Jsi_Value* func = NULL;
    if (Jsi_InterpGone(interp) || (cmdPtr->deleted && !isClose)) return JSI_ERROR;
    int n = 0;
    if (isClose)
        func = ((pss && pss->onClose)?pss->onClose:cmdPtr->onClose);
    else
        func = ((pss && pss->onRecv)?pss->onRecv:cmdPtr->onRecv);
    if (!func)
        return JSI_OK;
    vargs[n++] = (cmdPtr->deleted?Jsi_ValueNewNull(interp):Jsi_ValueNewObj(interp, cmdPtr->fobj));
    vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss?pss->wid:0));
    if (!isClose) {
        if (nlen<=0)
            return JSI_OK;
        vargs[n++]  = Jsi_ValueNewBlob(interp, (uchar*)inPtr, nlen);
        if ((cmdPtr->echo||(pss && pss->echo)) && inPtr)
            Jsi_LogInfo("WS-RECV: %s\n", inPtr);
    }
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, n, 0));
    Jsi_IncrRefCount(interp, vpargs);

    Jsi_Value *ret = Jsi_ValueNew1(interp);
    Jsi_RC rc = Jsi_FunctionInvoke(interp, func, vpargs, &ret, NULL);
    if (rc == JSI_OK && Jsi_ValueIsUndef(interp, ret)==0 && !isClose) {
        /* TODO: should we handle callback return data??? */
    }
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    return rc;
}

#if (LWS_LIBRARY_VERSION_MAJOR>1)
static int
jsi_wsFileUploadCB(void *data, const char *name, const char *filename,
                   char *buf, int len, enum lws_spa_fileupload_states state)
{
    jsi_wsPss *pss = (typeof(pss))data;
    jsi_wsCmdObj *cmdPtr = pss->cmdPtr;
    Jsi_Value* callPtr = (pss->onUpload?pss->onUpload:cmdPtr->onUpload);
    Jsi_Interp *interp = cmdPtr->interp;
    const char *str;
    int slen, n = 0;
    if (cmdPtr->deleted) return -1;

    Jsi_Obj *oarg1;
    Jsi_Value *vpargs, *vargs[10];
    if (state == LWS_UFS_OPEN)
        pss->file_length = 0;
    //id:number, filename:string, data:string, startpos:number, complete:boolean
    vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
    vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->wid));
    vargs[n++] = Jsi_ValueNewBlobString(interp, filename);
    vargs[n++] = Jsi_ValueNewBlob(interp, (uchar*)buf, (uint)len);
    vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)pss->file_length);
    vargs[n++] = Jsi_ValueNewBoolean(interp, (state==LWS_UFS_FINAL_CONTENT));
    vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
    Jsi_IncrRefCount(interp, vpargs);
    pss->file_length += len;

    Jsi_Value *ret = Jsi_ValueNew1(interp);
    Jsi_ValueMakeUndef(interp, &ret);
    Jsi_RC rc = Jsi_FunctionInvoke(interp, callPtr, vpargs, &ret, NULL);

    if ((state==LWS_UFS_FINAL_CONTENT || rc != JSI_OK) && (str=Jsi_ValueString(interp, ret, &slen))) {
        Jsi_DSAppendLen(&pss->resultStr, str, slen);
        pss->resultCode = rc;
    }

    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    if (rc != JSI_OK) {
        Jsi_LogError("websock bad rcv eval");
        return -1;
    }
    return 0;
}
#endif

static int jsi_wscallback_http(struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len)
{
    struct lws_context *context = lws_get_context(wsi);
    const char *inPtr = (char*)in;
    char client_name[128], client_ip[128];
    const char *res = "";
#ifdef EXTERNAL_POLL
    int m;
    int fd = (int)(long)user;
#endif
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj *)lws_context_user(context);
    if (!cmdPtr) {
        fprintf(stderr, "bad ws context\n");
        return -1;
    }
    jsi_wsPss *pss = NULL;
    Jsi_Interp *interp = cmdPtr->interp;
    Jsi_Value* callPtr = NULL;
    int rc = 0, deflt = 0;

    WSSIGASSERT(cmdPtr, OBJ);
    if (Jsi_InterpGone(interp))
        cmdPtr->deleted = 1;

    if (cmdPtr->debug>=128)
        fprintf(stderr, "HTTP CALLBACK: len=%d, %p %d:%s\n", (int)len, user, reason, jsw_getReasonStr(reason));

    switch (reason) {
#ifndef EXTERNAL_POLL
    case LWS_CALLBACK_GET_THREAD_ID:
    case LWS_CALLBACK_UNLOCK_POLL:
    case LWS_CALLBACK_PROTOCOL_INIT:
    case LWS_CALLBACK_ADD_POLL_FD:
    case LWS_CALLBACK_DEL_POLL_FD:
    case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
    case LWS_CALLBACK_LOCK_POLL:
        return rc;
#else
        /*
         * callbacks for managing the external poll() array appear in
         * protocol 0 callback
         */

    case LWS_CALLBACK_ADD_POLL_FD:

        if (jsi_wsnum_pollfds >= max_poll_elements) {
            lwsl_err("LWS_CALLBACK_ADD_POLL_FD: too many sockets to track\n");
            return 1;
        }

        fd_lookup[fd] = jsi_wsnum_pollfds;
        jsi_wspollfds[jsi_wsnum_pollfds].fd = fd;
        jsi_wspollfds[jsi_wsnum_pollfds].events = (int)(long)len;
        jsi_wspollfds[jsi_wsnum_pollfds++].revents = 0;
        break;

    case LWS_CALLBACK_DEL_POLL_FD:
        if (!--jsi_wsnum_pollfds)
            break;
        m = fd_lookup[fd];
        /* have the last guy take up the vacant slot */
        jsi_wspollfds[m] = jsi_wspollfds[jsi_wsnum_pollfds];
        fd_lookup[jsi_wspollfds[jsi_wsnum_pollfds].fd] = m;
        break;

#endif

    default:
        deflt = 1;
        break;

    }

    if (deflt && cmdPtr->debug>16 && cmdPtr->debug<128) {
        fprintf(stderr, "HTTP CALLBACK: len=%d, %p %d:%s\n", (int)len, user, reason, jsw_getReasonStr(reason));
    }

    switch (reason) {
    case LWS_CALLBACK_WSI_DESTROY:
        break;

#if (LWS_LIBRARY_VERSION_MAJOR>1)
    // Handle GET file download in client mode.
    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP: {
        char buffer[1024 + LWS_PRE];
        char *px = buffer + LWS_PRE;
        int lenx = sizeof(buffer) - LWS_PRE;

        if (lws_http_client_read(wsi, &px, &lenx) < 0)
            return -1;
        break;
    }
    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
        if (jsi_wsrecv_callback(interp, cmdPtr, pss, inPtr, len, 0) != JSI_OK)
            rc = 1;
        break;

    case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
        if (jsi_wsrecv_callback(interp, cmdPtr, pss, inPtr, len, 1) != JSI_OK)
            rc = 1;
        break;

    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
        if (cmdPtr->post) {
            unsigned char **p = (unsigned char **)in, *end = (*p) + len;
            int n = 0;
            char buf[100];
            Jsi_ValueString(interp, cmdPtr->post, &n);
            snprintf(buf, sizeof(buf), "%d", n);

            if (lws_add_http_header_by_token(wsi,
                    WSI_TOKEN_HTTP_CONTENT_LENGTH,
                    (unsigned char *)buf, 2, p, end))
                return -1;
            if (lws_add_http_header_by_token(wsi,
                    WSI_TOKEN_HTTP_CONTENT_TYPE,
                    (unsigned char *)"application/x-www-form-urlencoded", 33, p, end))
                return -1;

            /* inform lws we have http body to send */
            lws_client_http_body_pending(wsi, 1);
            lws_callback_on_writable(wsi);
        }
        break;

    case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE: {
        int n = 0;
        char *cps = Jsi_ValueString(interp, cmdPtr->post, &n);
        char *buf = (char*)Jsi_Calloc(1, LWS_PRE + n + 1);
        Jsi_Strcpy(buf + LWS_PRE, cps);
        n = lws_write(wsi, (unsigned char *)&buf[LWS_PRE], strlen(&buf[LWS_PRE]), LWS_WRITE_HTTP);
        Jsi_Free(buf);
        if (n < 0)
            return -1;
        /* we only had one thing to send, so inform lws we are done
         * if we had more to send, call lws_callback_on_writable(wsi);
         * and just return 0 from callback.  On having sent the last
         * part, call the below api instead.*/
        lws_client_http_body_pending(wsi, 0);
        break;
    }
#endif

    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        break;
    case LWS_CALLBACK_PROTOCOL_INIT:
        break;
    case LWS_CALLBACK_CLOSED_HTTP:
        if (cmdPtr->debug>2)
            fprintf(stderr, "CLOSED\n");
        if (!pss)
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);
        if (pss)
            jsi_wsdeletePss(pss);
        break;
    case LWS_CALLBACK_WSI_CREATE:
        break;

    case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY:
        break;

    case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
        if (cmdPtr->debug>1)
            fprintf(stderr, "FILTER CONNECTION: %s\n", inPtr);
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 1);
        Jsi_DSSet(&pss->url, inPtr);
        jsi_wsgetUriArgValue(interp, wsi, &pss->query, &pss->queryObj);

        if (cmdPtr->instCtx == context && (cmdPtr->clientName[0] || cmdPtr->clientIP[0])) {
            pss->clientName = cmdPtr->clientName;
            pss->clientIP = cmdPtr->clientIP;
        }

        Jsi_DSSetLength(&pss->dHdrs, 0);
        pss->hdrNum = jsi_wsGetHeaders(pss, wsi, &pss->dHdrs, pss->hdrSz, sizeof(pss->hdrSz)/sizeof(int));

        if (cmdPtr->onFilter && !cmdPtr->deleted) {
            // 4 args: ws, id, url, bool
            int killcon = 0, n = 0;
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[10], *ret = Jsi_ValueNew1(interp);
            vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
            vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->wid));
            vargs[n++] = Jsi_ValueNewBlob(interp, (uchar*)in, len);
            vargs[n++] = Jsi_ValueNewBoolean(interp, 1);
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onFilter, vpargs, &ret, NULL);
            if (rc == JSI_OK && Jsi_ValueIsFalse(interp, ret)) {
                if (cmdPtr->debug>1)
                    fprintf(stderr, "WS:KILLING CONNECTION: %p\n", pss);
                killcon = 1;
            }

            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (rc != JSI_OK) {
                Jsi_LogError("websock bad rcv eval");
                return 1;
            }
            if (killcon)
                return 1;
        }
        break;

    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
        client_name[0] = 0;
        client_ip[0] = 0;
        lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi), client_name,
                                         sizeof(client_name), client_ip, sizeof(client_ip));
        if (client_name[0])
            cmdPtr->clientName = Jsi_KeyAdd(interp, client_name);
        if (client_ip[0])
            cmdPtr->clientIP = Jsi_KeyAdd(interp, client_ip);

        if (cmdPtr->clientName || cmdPtr->clientIP) {
            const char *loname = cmdPtr->localhostName;
            if (!loname) loname = "localhost";
            cmdPtr->instCtx = context;
            if (cmdPtr->debug>1)
                fprintf(stderr,  "Received network connect from %s (%s)\n",
                     cmdPtr->clientName, cmdPtr->clientIP);
#ifndef __WIN32
            if (cmdPtr->local && (cmdPtr->clientName && Jsi_Strcmp(cmdPtr->clientName, loname))) {
                if (cmdPtr->debug>1)
                    fprintf(stderr,  "Dropping non-localhost connection\n");
                return 1;
            }
#endif
        }

        if (cmdPtr->maxConnects && cmdPtr->stats.connectCnt>=cmdPtr->maxConnects) {
            if (cmdPtr->debug>1)
                fprintf(stderr, "maxConnects exceeded: rejecting connection <%p>\n", user);
            rc = -1;
        }
        /* if we returned non-zero from here, we kill the connection */
        break;

    case LWS_CALLBACK_HTTP:
    {
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        if (cmdPtr->debug)
            fprintf(stderr, "HTTP GET: %s\n", inPtr);
        rc = jsi_wsHttp(interp, cmdPtr, wsi, user, context, inPtr, &dStr, pss);
        Jsi_DSFree(&dStr);
        if (rc<0)
            return -1;
        if (rc==1) {
            goto try_to_reuse;
        }
        break;
    }

#if (LWS_LIBRARY_VERSION_MAJOR>1)
    case LWS_CALLBACK_HTTP_BODY: {
        if (!pss)
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);
        if (!pss) break;
        callPtr = (pss->onUpload?pss->onUpload:cmdPtr->onUpload);
        if (cmdPtr->maxUpload<=0 || !callPtr) {
            if (cmdPtr->noWarn==0)
                fprintf(stderr, "Upload disabled: maxUpload=%d, onUpload=%p\n", cmdPtr->maxUpload, callPtr);
            return -1;
        }

        if (!pss->spa) {
            /* create the POST argument parser */
            if (!pss->paramv) {
                if (cmdPtr->formParams && cmdPtr->formParams != jsi_wsparam_str)
                    Jsi_SplitStr(cmdPtr->formParams, &pss->paramc, &pss->paramv, ",", &pss->paramDS);
                else {
                    pss->paramv = (typeof(pss->paramv))jsi_wsparam_names;
                    pss->paramc = ARRAY_SIZE(jsi_wsparam_names);
                }
            }
            pss->spa = lws_spa_create(wsi, (const char*const*)pss->paramv,
                pss->paramc, 4096, jsi_wsFileUploadCB, pss);
            if (!pss->spa)
                    return -1;

            pss->filename[0] = '\0';
            pss->file_length = 0;
        }

        cmdPtr->stats.uploadLast = pss->stats.uploadLast = time(NULL);

        /* let it parse the POST data */
        if (lws_spa_process(pss->spa, inPtr, len))
                return -1;


        if (!pss->stats.uploadStart) {
            cmdPtr->stats.uploadEnd = pss->stats.uploadEnd = 0;
            cmdPtr->stats.uploadStart = pss->stats.uploadStart = time(NULL);
            cmdPtr->stats.uploadCnt++;
            pss->stats.uploadCnt++;
        }
        break;
    }

    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
        if (!pss) {
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);
            callPtr = (pss&&pss->onUpload?pss->onUpload:cmdPtr->onUpload);
        }
        if (pss && pss->spa)
            lws_spa_finalize(pss->spa);
        res = Jsi_DSValue(&pss->resultStr);
        if (!res[0]) {
            if (!pss->resultCode)
                res = "<html><body>Upload complete</body></html>";
            else
                res = "<html><body>Upload error</body></html>";
        }
        jsi_wsServeString(pss, wsi, res, pss->resultCode==JSI_OK?0:500, NULL, NULL);
        if (cmdPtr->maxUpload<=0 || !callPtr) {
            if (cmdPtr->noWarn==0)
                fprintf(stderr, "Upload disabled: maxUpload=%d, onUpload=%p\n", cmdPtr->maxUpload, callPtr);
            return -1;
        }
        cmdPtr->stats.uploadEnd = pss->stats.uploadEnd = time(NULL);
        lws_return_http_status(wsi, HTTP_STATUS_OK, NULL);
        goto try_to_reuse;

    case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);
        if (pss && pss->spa) {
            lws_spa_destroy(pss->spa);
            pss->spa = NULL;
        }
        break;
#endif

    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
        goto try_to_reuse;

    case LWS_CALLBACK_HTTP_WRITEABLE: {
        lwsl_info("LWS_CALLBACK_HTTP_WRITEABLE\n");
        if (!pss)
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);

        if (!pss || !pss->fop_fd)
            goto try_to_reuse;

        /*
         * we can send more of whatever it is we were sending
         */
        int sent = 0;
        unsigned char buffer[JSI_BUFSIZ*10 + LWS_PRE];
        do {
            int n = sizeof(buffer) - LWS_PRE;
            int m = lws_get_peer_write_allowance(wsi);
            if (m == 0)
                goto later;

            if (m != -1 && m < n)
                n = m;

            lws_filepos_t amount = 0;
            n = lws_vfs_file_read(pss->fop_fd, &amount, buffer + LWS_PRE, n);
            if (n < 0) {
                lwsl_err("problem reading file\n");
                goto bail;
            }
            n = (int)amount;
            if (n == 0)
                goto penultimate;
            /*
             * To support HTTP2, must take care about preamble space
             *
             * identification of when we send the last payload frame
             * is handled by the library itself if you sent a
             * content-length header
             */
            m = jsi_wswrite(pss, wsi, buffer + LWS_PRE, n, LWS_WRITE_HTTP);
            if (m < 0) {
                lwsl_err("write failed\n");
                /* write failed, close conn */
                goto bail;
            }
            if (m) /* while still active, extend timeout */
                lws_set_timeout(wsi, PENDING_TIMEOUT_HTTP_CONTENT, 5);
            sent += m;

        } while (!lws_send_pipe_choked(wsi) && (sent < 500 * 1024 * 1024));
later:
        lws_callback_on_writable(wsi);
        break;
penultimate:
        lws_vfs_file_close(&pss->fop_fd);
        goto try_to_reuse;

bail:
        lws_vfs_file_close(&pss->fop_fd);
        rc = -1;
        goto doret;
    }

    default:
        break;
    }

    goto doret;

try_to_reuse:
    if (lws_http_transaction_completed(wsi))
         rc = -1;
    else
        rc = 0;
    goto doret;

doret:
    if (cmdPtr->debug>2)
        fprintf(stderr, "<---HTTP RET = %d\n", rc);
    return rc;
}

static int
jsi_wscallback_websock(struct lws *wsi,
      enum lws_callback_reasons reason,
      void *user, void *in, size_t len)
{
    struct lws_context *context = lws_get_context(wsi);

    jsi_wsPss *pss = NULL;
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj *)lws_context_user(context);
    if (!cmdPtr) {
        fprintf(stderr, "null ws context\n");
        return -1;
    }
    Jsi_Interp *interp = cmdPtr->interp;
    char *inPtr = (char*)in;
    int sLen, n, rc =0;
    WSSIGASSERT(cmdPtr, OBJ);
    if (Jsi_InterpGone(interp))
        cmdPtr->deleted = 1;

    if (cmdPtr->debug>=32) {
        switch (reason) {
            case LWS_CALLBACK_SERVER_WRITEABLE:
            case LWS_CALLBACK_CLIENT_WRITEABLE:
                break;
            default:
                fprintf(stderr, "WS CALLBACK: len=%d, %p %d:%s\n", (int)len, user, reason, jsw_getReasonStr(reason));
        }
    }

    switch (reason) {
    case LWS_CALLBACK_PROTOCOL_INIT:
        if (cmdPtr->noWebsock)
            return 1;
        break;

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 1);
        Jsi_DSSet(&pss->url, inPtr);
        if (cmdPtr->instCtx == context && (cmdPtr->clientName[0] || cmdPtr->clientIP[0])) {
            pss->clientName = cmdPtr->clientName;
            pss->clientIP = cmdPtr->clientIP;
        }
        if (cmdPtr->onFilter && !cmdPtr->deleted) {
            if (!pss)
                pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 0);
            int killcon = 0, n = 0;
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[10], *ret = Jsi_ValueNew1(interp);

            vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
            vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->wid));
            vargs[n++] = Jsi_ValueNewBlob(interp, (uchar*)in, len);
            vargs[n++] = Jsi_ValueNewBoolean(interp, 0);
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onFilter, vpargs, &ret, NULL);
            if (rc == JSI_OK && Jsi_ValueIsFalse(interp, ret)) {
                if (cmdPtr->debug>1)
                    fprintf(stderr, "WS:KILLING CONNECTION: %p\n", user);
                killcon = 1;
            }

            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (rc != JSI_OK) {
                Jsi_LogError("websock bad rcv eval");
                return 1;
            }
            if (killcon)
                return 1;
        }
        break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    case LWS_CALLBACK_ESTABLISHED:
        if (cmdPtr->bufferPwr2>0) {
            char nbuf[100];
            snprintf(nbuf, sizeof(nbuf), "%d", cmdPtr->bufferPwr2);
            lws_set_extension_option(wsi, "permessage-deflate", "rx_buf_size", nbuf);
            lws_set_extension_option(wsi, "permessage-deflate", "tx_buf_size", nbuf);
        }
        if (!pss)
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 0);
        if (cmdPtr->onOpen && !cmdPtr->deleted) {
            /* Pass 2 args: ws id. */
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[10];
            int n = 0;
            vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
            vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->wid));
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);

            Jsi_Value *ret = Jsi_ValueNew1(interp);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onOpen, vpargs, &ret, NULL);

            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (rc != JSI_OK)
                return Jsi_LogError("websock bad rcv eval");
        }
        break;

    case LWS_CALLBACK_WSI_DESTROY:
        break;

    case LWS_CALLBACK_CLOSED:
    case LWS_CALLBACK_PROTOCOL_DESTROY:
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 0);
        if (!pss) break;
        if (cmdPtr->onClose || pss->onClose) {
            rc = jsi_wsrecv_callback(interp, cmdPtr, pss, inPtr, len, 1);
            if (rc != JSI_OK)
                return Jsi_LogError("websock bad rcv eval");
        }
        jsi_wsdeletePss(pss);
        if (cmdPtr->stats.connectCnt<=0 && cmdPtr->onCloseLast && !Jsi_InterpGone(interp)) {
            Jsi_RC jrc;
            Jsi_Value *retStr = Jsi_ValueNew1(interp);
            // 1 args: ws
            Jsi_Value *vpargs, *vargs[10];
            int n = 0;
            vargs[n++] = (cmdPtr->deleted?Jsi_ValueNewNull(interp):Jsi_ValueNewObj(interp, cmdPtr->fobj));
            vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            jrc = Jsi_FunctionInvoke(interp, cmdPtr->onCloseLast, vpargs, &retStr, NULL);
            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, retStr);
            if (Jsi_InterpGone(interp))
                return JSI_ERROR;
            return jrc;
        }
        break;

    case LWS_CALLBACK_CLIENT_WRITEABLE:
    case LWS_CALLBACK_SERVER_WRITEABLE: {
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 0);
        if (!pss || !pss->stack) break;
        if (pss->lastData)
            Jsi_Free(pss->lastData);
        n=0;
        char *data = pss->lastData = (char*)Jsi_StackUnshift(pss->stack);
        unsigned char *p;
        if (data == NULL)
            break;
        pss->stats.msgQLen--;
        pss->state = PWS_SENT;
        p = (unsigned char *)data+LWS_PRE;
        sLen = Jsi_Strlen((char*)p);
        n = jsi_wswrite(pss, wsi, p, sLen, (pss->stats.isBinary?LWS_WRITE_BINARY:LWS_WRITE_TEXT));
        if (cmdPtr->debug>=10)
            fprintf(stderr, "WS:CLIENT WRITE(%p): %d=>%d\n", pss, sLen, n);

        if (n >= 0) {
            cmdPtr->stats.sentCnt++;
            cmdPtr->stats.sentLast = time(NULL);
            pss->stats.sentCnt++;
            pss->stats.sentLast = time(NULL);
        } else {
            lwsl_err("ERROR %d writing to socket\n", n);
            pss->state = PWS_SENDERR;
            pss->stats.sentErrCnt++;
            pss->stats.sentErrLast = time(NULL);
            cmdPtr->stats.sentErrCnt++;
            cmdPtr->stats.sentErrLast = time(NULL);
            rc = 1;
        }
        break;
    }

    case LWS_CALLBACK_CLIENT_RECEIVE:
    case LWS_CALLBACK_RECEIVE:
    {
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 0);
        if (!pss) break;

        pss->stats.recvCnt++;
        pss->stats.recvLast = time(NULL);
        cmdPtr->stats.recvCnt++;
        cmdPtr->stats.recvLast = time(NULL);

        if (cmdPtr->onRecv || pss->onRecv) {
            /* Pass 2 args: id and data. */
            int nlen = len;
            if (nlen<=0)
                return 0;
            int rblen = Jsi_DSLength(&pss->recvBuf),
                bmax = cmdPtr->recvBufMax,
                isfin = pss->stats.isFinal = lws_is_final_fragment(wsi);
            pss->stats.isBinary = lws_frame_is_binary(wsi);
            if (rblen) {
                if (bmax && rblen>bmax) {
                    fprintf(stderr, "WS: Recv exceeds recvBufMax: %d>%d\n", rblen, bmax);
                    rc = 1;
                    break;
                }
                Jsi_DSAppendLen(&pss->recvBuf, inPtr, len);
                if (!isfin) break;
                cmdPtr->recvBufCnt--;
                nlen = Jsi_DSLength(&pss->recvBuf);
                inPtr = Jsi_DSFreeDup(&pss->recvBuf);
            } else {
                if (!isfin) {
                    cmdPtr->recvBufCnt++;
                    Jsi_DSAppendLen(&pss->recvBuf, inPtr, len);
                    break;
                }
            }
            rc = jsi_wsrecv_callback(interp, cmdPtr, pss, inPtr, nlen, 0);
            if (inPtr != in)
                Jsi_Free(inPtr);
            if (rc != JSI_OK) {
                Jsi_LogError("websock bad rcv eval");
                return 1;
            }
        }
        lws_callback_on_writable_all_protocol(cmdPtr->context, lws_get_protocol(wsi));
        break;

    }
    default:
        break;
    }
    return rc;
}


static Jsi_RC WebSocketConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)Jsi_UserObjGetData(interp, _this, funcPtr);

    if (!cmdPtr)
        return Jsi_LogError("Apply in a non-websock object");
    Jsi_Value *opts = Jsi_ValueArrayIndex(interp, args, 0);
    if (cmdPtr->noConfig && opts && !Jsi_ValueIsString(interp, opts))
        return Jsi_LogError("WebSocket conf() is disabled for set");
    return Jsi_OptionsConf(interp, WSOptions, cmdPtr, opts, ret, 0);

}

static Jsi_RC WebSocketIdCmdOp(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int op)
{
    Jsi_RC rc = JSI_OK;
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr)
        return Jsi_LogError("Apply in a non-websock object");
    Jsi_Value *v, *valPtr = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Number vid;
    if (Jsi_ValueGetNumber(interp, valPtr, &vid) != JSI_OK || vid < 0)
        return Jsi_LogError("Expected connection number id");
    int id = (int)vid;
    jsi_wsPss *pss = NULL;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    for (hPtr = Jsi_HashSearchFirst(cmdPtr->pssTable, &cursor);
        hPtr != NULL; hPtr = Jsi_HashSearchNext(&cursor)) {
        jsi_wsPss* tpss = (jsi_wsPss*)Jsi_HashValueGet(hPtr);
        WSSIGASSERT(tpss, PWS);
        if (tpss->wid == id && tpss->state != PWS_DEAD) {
            pss = tpss;
            break;
        }
    }

    if (!pss)
        return Jsi_LogError("No such id: %d", id);
    switch (op) {
        case 0:
            v = Jsi_ValueArrayIndex(interp, args, 1);
            rc = Jsi_OptionsConf(interp, WPSOptions, pss, v, ret, 0);
            break;
        case 1:
            jsi_wsDumpHeaders(cmdPtr, pss, Jsi_ValueArrayIndexToStr(interp, args, 1, NULL), ret);
            break;
        case 2:
            if (!pss->spa) return JSI_OK;
            jsi_wsDumpQuery(cmdPtr, pss, Jsi_ValueArrayIndexToStr(interp, args, 1, NULL), ret);
            break;
    }
    return rc;
}

static Jsi_RC WebSocketIdConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return WebSocketIdCmdOp(interp, args, _this, ret, funcPtr, 0);
}

static Jsi_RC WebSocketHeaderCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return WebSocketIdCmdOp(interp, args, _this, ret, funcPtr, 1);
}

static Jsi_RC WebSocketQueryCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return WebSocketIdCmdOp(interp, args, _this, ret, funcPtr, 2);
}


static Jsi_RC WebSocketIdsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr)
        return Jsi_LogError("Apply in a non-websock object");
    const char *val = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    Jsi_DString dStr = {"["};
    jsi_wsPss *pss = NULL;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    int cnt = 0;
    for (hPtr = Jsi_HashSearchFirst(cmdPtr->pssTable, &cursor);
        hPtr != NULL; hPtr = Jsi_HashSearchNext(&cursor)) {
        pss = (jsi_wsPss*)Jsi_HashValueGet(hPtr);
        WSSIGASSERT(pss, PWS);
        if (pss->state == PWS_DEAD) continue;
        if (val && Jsi_Strcmp(pss->key, val)) continue;
        Jsi_DSPrintf(&dStr, "%s%d", (cnt++?",":""), pss->wid);
        if (val) break;
    }
    Jsi_DSAppend(&dStr, "]", NULL);
    Jsi_RC rc = Jsi_JSONParse(interp, Jsi_DSValue(&dStr), ret, 0);
    Jsi_DSFree(&dStr);
    return rc;
}


static Jsi_RC jsi_wsHandlerAdd(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, const char *ext, const char *cmd, int flags)
{
    Jsi_HashEntry *hPtr;
    jsi_wsHander *hdlPtr;
    Jsi_Value *valPtr = Jsi_ValueNewStringDup(interp, cmd);
    hPtr = Jsi_HashEntryNew(cmdPtr->handlers, ext, NULL);
    if (!hPtr)
        return JSI_ERROR;
    hdlPtr = (jsi_wsHander *)Jsi_Calloc(1, sizeof(*hdlPtr));
    hdlPtr->val = valPtr;
    hdlPtr->flags = flags;
    Jsi_HashValueSet(hPtr, hdlPtr);
    Jsi_IncrRefCount(interp, valPtr);
    return JSI_OK;
}

#define FN_wshandler JSI_INFO("\
With no args, returns list of handlers.  With one arg, returns value for that handler.\n\
Otherwise, sets the handler. When cmd is a string, the call is via runModule([cmd], arg).\n\
If a cmd is a function, it is called with a single arg: the file name.")
static Jsi_RC WebSocketHandlerCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)Jsi_UserObjGetData(interp, _this, funcPtr);
    Jsi_HashEntry *hPtr;
    jsi_wsHander *hdlPtr;
    if (!cmdPtr)
        return Jsi_LogError("Apply in a non-websock object");
    WSSIGASSERT(cmdPtr, OBJ);
    int argc = Jsi_ValueGetLength(interp, args);
    if (argc == 0) {
        Jsi_HashSearch search;
        Jsi_Obj* obj = Jsi_ObjNew(interp);
        for (hPtr = Jsi_HashSearchFirst(cmdPtr->handlers, &search); hPtr; hPtr = Jsi_HashSearchNext(&search)) {
            const char *key = (char*)Jsi_HashKeyGet(hPtr);
            Jsi_Value *val = (Jsi_Value*)Jsi_HashValueGet(hPtr);
            Jsi_ObjInsert(interp, obj, key, val, 0);
        }
        Jsi_ValueMakeObject(interp, ret, obj);
        return JSI_OK;
    }
    if (argc == 1) {
        hPtr = Jsi_HashEntryFind(cmdPtr->handlers, Jsi_ValueArrayIndexToStr(interp, args, 0, NULL));
        if (!hPtr)
            return JSI_OK;
        hdlPtr = (jsi_wsHander*)Jsi_HashValueGet(hPtr);
        Jsi_ValueReplace(interp, ret, hdlPtr->val);
        return JSI_OK;
    }
    const char *key = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    Jsi_Value *valPtr = Jsi_ValueArrayIndex(interp, args, 1);
    if (Jsi_ValueIsNull(interp, valPtr)) {
        hPtr = Jsi_HashEntryFind(cmdPtr->handlers, key);
        if (!hPtr)
            return JSI_OK;
        hdlPtr = (jsi_wsHander*)Jsi_HashValueGet(hPtr);
        if (hdlPtr->val)
            Jsi_DecrRefCount(interp, hdlPtr->val);
        Jsi_HashValueSet(hPtr, NULL);
        Jsi_HashEntryDelete(hPtr);
        Jsi_Free(hdlPtr);
        Jsi_ValueMakeStringDup(interp, ret, key);
        return JSI_OK;
    }
    if (Jsi_ValueIsFunction(interp, valPtr)==0 && Jsi_ValueIsString(interp, valPtr)==0)
        return Jsi_LogError("expected string, function or null");
    Jsi_Value *argPtr = Jsi_ValueArrayIndex(interp, args, 2);
    if (argPtr) {
        if (Jsi_ValueIsNull(interp, argPtr))
            argPtr = NULL;
        else if (!Jsi_ValueIsString(interp, argPtr))
            return Jsi_LogError("expected a string");
    }
    hPtr = Jsi_HashEntryNew(cmdPtr->handlers, key, NULL);
    if (!hPtr)
        return JSI_ERROR;
    hdlPtr = (jsi_wsHander *)Jsi_Calloc(1, sizeof(*hdlPtr));
    Jsi_Value *flagPtr = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Number fl = 0;
    if (flagPtr && Jsi_ValueIsNumber(interp, flagPtr))
        Jsi_ValueGetNumber(interp, flagPtr, &fl);
    hdlPtr->val = valPtr;
    hdlPtr->flags = fl;
    Jsi_HashValueSet(hPtr, hdlPtr);
    Jsi_IncrRefCount(interp, valPtr);
    return JSI_OK;
}

#define FN_wssend JSI_INFO("\
Send a message to one (or all connections if -1). If not already a string, msg is formatted as JSON prior to the send.")

static Jsi_RC WebSocketSendCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr)
        return Jsi_LogError("Apply in a non-websock object");
    WSSIGASSERT(cmdPtr, OBJ);
    jsi_wsPss *pss;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    char *str = Jsi_ValueString(interp, arg, NULL);
    int id = -1, argc = Jsi_ValueGetLength(interp, args);
    Jsi_DString eStr = {};
    if (argc!=2)
        return Jsi_LogError("wrong args");
    Jsi_Number dnum;
    Jsi_Value *darg = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_ValueGetNumber(interp, darg, &dnum) != JSI_OK)
        return Jsi_LogError("invalid id");
    id = (int)dnum;

    if (!str)
        str = (char*)Jsi_ValueGetDString(interp, arg, &eStr, JSI_OUTPUT_JSON);

    if (cmdPtr->echo)
        Jsi_LogInfo("WS-SEND: %s\n", str);

    for (hPtr = Jsi_HashSearchFirst(cmdPtr->pssTable, &cursor);
        hPtr != NULL; hPtr = Jsi_HashSearchNext(&cursor)) {
        pss = (jsi_wsPss*)Jsi_HashValueGet(hPtr);
        WSSIGASSERT(pss, PWS);
        if ((id<0 || pss->wid == id) && pss->state != PWS_DEAD) {
            if (!pss->stack)
                pss->stack = Jsi_StackNew();
            char *msg = (char*)Jsi_Malloc(LWS_PRE + Jsi_Strlen(str) + 1);
            Jsi_Strcpy(msg + LWS_PRE, str);
            Jsi_StackPush(pss->stack, msg);
            pss->stats.msgQLen++;
            if (!cmdPtr->echo && pss->echo)
                Jsi_LogInfo("WS-SEND: %s\n", str);
        }
    }

    Jsi_DSFree(&eStr);
    return JSI_OK;
}

static Jsi_RC jsi_wsrecv_flush(jsi_wsCmdObj *cmdPtr, jsi_wsPss *pss)
{
    int nlen = Jsi_DSLength(&pss->recvBuf);
    if (nlen<=0)
        return JSI_OK;
    cmdPtr->recvBufCnt--;
    const char *inPtr = Jsi_DSFreeDup(&pss->recvBuf);
    Jsi_RC rc = jsi_wsrecv_callback(cmdPtr->interp, cmdPtr, pss, inPtr, nlen, 0);
    if (rc != JSI_OK) {
        pss->stats.recvErrCnt++;
        pss->stats.recvErrLast = time(NULL);
    }
    return rc;
}

static void jsi_wsOnModify(jsi_wsCmdObj *cmdPtr) {
    if (!cmdPtr->stats.httpLast) return;
    time_t now = time(NULL);
    double dt = (difftime(now, cmdPtr->lastModifyCheck));
    if (dt<0) {
        cmdPtr->lastModifyCheck = now;
        return;
    }
    uint secs = cmdPtr->modifySecs;
    if (!secs) secs = 2;
    if (dt<secs) return;
    cmdPtr->lastModifyCheck = now;
    Jsi_Interp *interp = cmdPtr->interp;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch cursor;
    Jsi_Value* changed = NULL;
    time_t ll = cmdPtr->lastModifyNotify;
    if (ll<cmdPtr->stats.httpLast)
        ll = cmdPtr->stats.httpLast;
    for (hPtr = Jsi_HashSearchFirst(cmdPtr->fileHash, &cursor);
        hPtr != NULL; hPtr = Jsi_HashSearchNext(&cursor)) {
        jsi_wsFile* fPtr = (jsi_wsFile*)Jsi_HashValueGet(hPtr);
        if (fPtr && fPtr->fileVal) {
            Jsi_StatBuf sb;
            int n = Jsi_Stat(interp, fPtr->fileVal, &sb);
            if (!n && sb.st_mtime > ll) {
                changed = fPtr->fileVal;
                break;
            }
        }
    }
    cmdPtr->lastModifyCheck = time(NULL);
    if (!changed) return;
    Jsi_Obj *oarg1;
    Jsi_Value *vpargs, *vargs[10];
    int n = 0;
    vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
    vargs[n++] = changed;
    vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
    Jsi_IncrRefCount(interp, vpargs);

    Jsi_Value *ret = Jsi_ValueNew1(interp);
    Jsi_ValueMakeUndef(interp, &ret);
    Jsi_RC rc = Jsi_FunctionInvoke(interp, cmdPtr->onModify, vpargs, &ret, NULL);

    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    if (rc != JSI_OK) {
        Jsi_LogWarn("websock bad onModify eval: disabling");
        Jsi_DecrRefCount(interp, cmdPtr->onModify);
        cmdPtr->onModify = NULL;
    }
    cmdPtr->lastModifyCheck = time(NULL);
    cmdPtr->lastModifyNotify = time(NULL);
}

static int jsi_wsService(jsi_wsCmdObj *cmdPtr)
{
    int n = 0;
    struct timeval tv;
    if (cmdPtr->inUpdate) return 0;
    cmdPtr->inUpdate = 1;

    gettimeofday(&tv, NULL);
    int to = cmdPtr->recvBufTimeout;
    if (to>0 && cmdPtr->recvBufCnt) { // Flush buffered data.
        jsi_wsPss *pss = NULL;
        Jsi_HashEntry *hPtr;
        Jsi_HashSearch cursor;
        for (hPtr = Jsi_HashSearchFirst(cmdPtr->pssTable, &cursor);
            hPtr != NULL; hPtr = Jsi_HashSearchNext(&cursor)) {
            pss = (jsi_wsPss*)Jsi_HashValueGet(hPtr);
            WSSIGASSERT(pss, PWS);
            if (pss->state == PWS_DEAD) continue;
            if (Jsi_DSLength(&pss->recvBuf)<=0) continue;
            if (pss->stats.recvLast && difftime(time(NULL), pss->stats.recvLast)<(double)to) continue;
            jsi_wsrecv_flush(cmdPtr, pss);
        }
    }

    /*
     * This provokes the LWS_CALLBACK_SERVER_WRITEABLE for every
     * live websocket connection using the DUMB_INCREMENT protocol,
     * as soon as it can take more packets (usually immediately)
     */

    if (((unsigned int)tv.tv_usec - cmdPtr->oldus) > 50000) {
        lws_callback_on_writable_all_protocol(cmdPtr->context, &cmdPtr->protocols[JWS_PROTOCOL_WEBSOCK]);
        cmdPtr->oldus = tv.tv_usec;
    }

#ifdef EXTERNAL_POLL

    /*
     * this represents an existing server's single poll action
     * which also includes libwebsocket sockets
     */

    n = poll(jsi_wspollfds, jsi_wsnum_pollfds, 50);
    if (n < 0) {
        n = 0;
        goto done;
    }

    if (n)
        for (n = 0; n < jsi_wsnum_pollfds; n++)
            if (jsi_wspollfds[n].revents)
                /*
                * returns immediately if the fd does not
                * match anything under libwebsockets
                * control
                */
                if (lws_service_fd(context, &jsi_wspollfds[n]) < 0) {
                    n = -1;
                    goto done;
                }
done:
#else
    n = lws_service(cmdPtr->context, 50);
#endif
    if (cmdPtr->onModify) {
        jsi_wsOnModify(cmdPtr);
    }
    cmdPtr->inUpdate = 0;
    return n;
}

static Jsi_RC WebSocketUpdateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr)
        return Jsi_LogError("Apply to non-websock object");
    if (!cmdPtr->noUpdate)
        jsi_wsService(cmdPtr);
    return JSI_OK;
}

static Jsi_RC jsi_wswebsockUpdate(Jsi_Interp *interp, void *data)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)data;
    WSSIGASSERT(cmdPtr,OBJ);
    jsi_wsService(cmdPtr);
    return JSI_OK;
}

static void jsi_wswebsocketObjErase(jsi_wsCmdObj *cmdPtr)
{
    if (cmdPtr->interp) {
        if (cmdPtr->event)
            Jsi_EventFree(cmdPtr->interp, cmdPtr->event);
        cmdPtr->event = NULL;
        if (cmdPtr->hasOpts)
            Jsi_OptionsFree(cmdPtr->interp, WSOptions, cmdPtr, 0);
        cmdPtr->hasOpts = 0;
        if (cmdPtr->handlers)
            Jsi_HashDelete(cmdPtr->handlers);
        cmdPtr->handlers = NULL;
        if (cmdPtr->pssTable)
            Jsi_HashDelete(cmdPtr->pssTable);
        cmdPtr->pssTable = NULL;
        if (cmdPtr->fileHash)
            Jsi_HashDelete(cmdPtr->fileHash);
        cmdPtr->fileHash = NULL;
    }
    cmdPtr->interp = NULL;
}

static Jsi_RC jsi_wswebsocketObjFree(Jsi_Interp *interp, void *data)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)data;
    WSSIGASSERT(cmdPtr,OBJ);
    cmdPtr->deleted = 1;
    struct lws_context *ctx = cmdPtr->context;
    if (ctx)
        lws_context_destroy(ctx);
    cmdPtr->context = NULL;
    cmdPtr->_->activeCnt--;
    jsi_wswebsocketObjErase(cmdPtr);
    _JSI_MEMCLEAR(cmdPtr);
    Jsi_Free(cmdPtr);
    return JSI_OK;
}

static bool jsi_wswebsocketObjIsTrue(void *data)
{
    return 1;
}

static bool jsi_wswebsocketObjEqual(void *data1, void *data2)
{
    return (data1 == data2);
}

static Jsi_RC jsi_wsfreeHandlers(Jsi_Interp *interp, Jsi_HashEntry* hPtr, void *ptr) {
    jsi_wsHander *h = (jsi_wsHander*)ptr;
    if (!h)
        return JSI_OK;
    if (h->val)
        Jsi_DecrRefCount(interp, h->val);
    if (h->objVar)
        Jsi_DecrRefCount(interp, h->objVar);
    Jsi_Free(h);
    return JSI_OK;
}

static Jsi_RC jsi_wsfreePss(Jsi_Interp *interp, Jsi_HashEntry* hPtr, void *ptr) {
    jsi_wsPss *pss = (jsi_wsPss*)ptr;
    WSSIGASSERT(pss, PWS);
    if (pss) {
        pss->hPtr = NULL;
        jsi_wsdeletePss(pss);
    }
    return JSI_OK;
}

static Jsi_RC jsi_wsfreeFile(Jsi_Interp *interp, Jsi_HashEntry* hPtr, void *ptr) {
    jsi_wsFile*h = (jsi_wsFile*)ptr;
    if (!h)
        return JSI_OK;
    if (h->fileVal)
        Jsi_DecrRefCount(interp, h->fileVal);
    Jsi_Free(h);
    return JSI_OK;
}

static Jsi_RC WebSocketVersionCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *verStr = NULL;
    verStr = lws_get_library_version();
    if (verStr) {
        char buf[100], *cp;
        snprintf(buf, sizeof(buf), "%s", verStr);
        cp = Jsi_Strchr(buf, ' ');
        if (cp) *cp = 0;
        Jsi_ValueMakeStringDup(interp, ret, buf);
    }
    return JSI_OK;
}

static Jsi_RC WebSocketFileCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr)
        return Jsi_LogError("Apply to non-websock object");
    Jsi_Value *val = Jsi_ValueArrayIndex(interp, args, 0);
    if (val)
        return jsi_wsFileAdd(interp, cmdPtr, val);
    if (cmdPtr->fileHash)
        return Jsi_HashKeysDump(interp, cmdPtr->fileHash, ret, 0);
    return JSI_OK;
}

static Jsi_RC WebSocketStatusCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj*)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!cmdPtr)
        return Jsi_LogError("Apply to non-websock object");
#ifndef OMIT_LWS_WITH_SERVER_STATUS
    char cbuf[JSI_BUFSIZ*2];
    lws_json_dump_context(cmdPtr->context, cbuf, sizeof(cbuf), 0);
    return Jsi_JSONParse(interp, cbuf, ret, 0);
#else
    return Jsi_LogError("unsupported");
#endif
}

#define FN_WebSocket JSI_INFO("\
Create a websocket server/client object.  The server serves out pages to a web browser,\n\
which can use javascript to upgrade connection to a bidirectional websocket.")
static Jsi_RC WebSocketConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);


static Jsi_CmdSpec websockCmds[] = {
    { "WebSocket",  WebSocketConstructor, 0,  1, "options:object=void", .help="Create websocket server/client object", .retType=(uint)JSI_TT_USEROBJ, .flags=JSI_CMD_IS_CONSTRUCTOR, .info=FN_WebSocket, .opts=WSOptions },
    { "conf",       WebSocketConfCmd,     0,  1, "options:string|object=void",.help="Configure options", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=WSOptions },
    { "handler",    WebSocketHandlerCmd,  0,  3, "extension:string=void, cmd:string|function=void, flags:number=0",
        .help="Get/Set handler command for an extension", .retType=(uint)JSI_TT_FUNCTION|JSI_TT_ARRAY|JSI_TT_STRING|JSI_TT_VOID, .flags=0, .info=FN_wshandler },
    { "ids",        WebSocketIdsCmd,      0,  1, "name:string=void", .help="Return list of ids, or lookup one id", .retType=(uint)JSI_TT_ARRAY},
    { "idconf",     WebSocketIdConfCmd,   1,  2, "id:number, options:string|object=void",.help="Configure options for connect id", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=WPSOptions },
    { "header",     WebSocketHeaderCmd,   1,  2, "id:number, name:string=void",.help="Get one or all input headers for connect id", .retType=(uint)JSI_TT_STRING|JSI_TT_ARRAY|JSI_TT_VOID },
    { "file",       WebSocketFileCmd,     0,  1, "name:string=void",.help="Add file to hash, or with no args return file hash", .retType=(uint)JSI_TT_ARRAY|JSI_TT_VOID },
    { "query",      WebSocketQueryCmd,    1,  2, "id:number, name:string=void",.help="Get one or all query values for connect id", .retType=(uint)JSI_TT_STRING|JSI_TT_OBJECT|JSI_TT_VOID },
    { "send",       WebSocketSendCmd,     2,  2, "id:number, data:any", .help="Send a websocket message to id", .retType=(uint)JSI_TT_VOID, .flags=0, .info=FN_wssend },
    { "status",     WebSocketStatusCmd,   0,  0, "", .help="Return libwebsocket server status", .retType=(uint)JSI_TT_OBJECT|JSI_TT_VOID},
    { "unalias",    WebSocketUnaliasCmd,  1,  1, "path:string", .help="Return alias reverse lookup", .retType=(uint)JSI_TT_STRING|JSI_TT_VOID},
    { "update",     WebSocketUpdateCmd,   0,  0, "", .help="Service events for just this websocket", .retType=(uint)JSI_TT_VOID },
    { "version",    WebSocketVersionCmd,  0,  0, "", .help="Runtime library version string", .retType=(uint)JSI_TT_STRING },
    { NULL, 0,0,0,0, .help="Commands for managing WebSocket server/client connections"  }
};


static Jsi_UserObjReg websockobject = {
    "WebSocket",
    websockCmds,
    jsi_wswebsocketObjFree,
    jsi_wswebsocketObjIsTrue,
    jsi_wswebsocketObjEqual
};

static const struct lws_extension jsi_lws_exts[] = {
    {
        "permessage-deflate",
        lws_extension_callback_pm_deflate,
        "permessage-deflate"
    },
    {
        "deflate-frame",
        lws_extension_callback_pm_deflate,
        "deflate_frame"
    },
    { NULL, NULL, NULL /* terminator */ }
};

static Jsi_RC WebSocketConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_InterpAccess(interp, NULL, JSI_INTACCESS_NETWORK ) != JSI_OK)
        return Jsi_LogError("WebSocket disallowed by Interp.noNetwork option");
    jsi_wsCmdObj *cmdPtr;
    Jsi_Value *toacc = NULL;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);

    cmdPtr = (jsi_wsCmdObj*)Jsi_Calloc(1, sizeof(*cmdPtr));
    cmdPtr->sig = JWS_SIG_OBJ;
    cmdPtr->_ = &wsObjCmd;
    cmdPtr->_->newCnt++;
    cmdPtr->_->activeCnt++;
    cmdPtr->port = 8080;
    cmdPtr->formParams = jsi_wsparam_str;
    cmdPtr->maxUpload = 100000;
    cmdPtr->interp = interp;
    cmdPtr->ietf_version = -1;
    cmdPtr->bufferPwr2 = 0;
    cmdPtr->ws_gid = -1;
    cmdPtr->ws_uid = -1;
    cmdPtr->startTime = time(NULL);
    cmdPtr->hasOpts = 1;
    cmdPtr->includeFile = "include.shtml";
    cmdPtr->jsiFnPattern = "jsig*.js";
    if ((arg != NULL && !Jsi_ValueIsNull(interp,arg))
        && Jsi_OptionsProcess(interp, WSOptions, cmdPtr, arg, 0) < 0) {
bail:
        jsi_wswebsocketObjFree(interp, cmdPtr);
        return JSI_ERROR;
    }
    if (!cmdPtr->udata) {
        cmdPtr->udata = Jsi_ValueNewObj(interp, NULL);
        Jsi_IncrRefCount(interp, cmdPtr->udata);
    }
    Jsi_PathNormalize(interp, &cmdPtr->rootdir);

    if (cmdPtr->headers && (Jsi_ValueGetLength(interp, cmdPtr->headers)%2)) {
        Jsi_LogError("Odd header length");
        goto bail;
    }
    const char *up = cmdPtr->urlPrefix, *ur = cmdPtr->urlRedirect;
    if (up && ur && Jsi_Strncmp(ur, up, Jsi_Strlen(up))) {
        Jsi_LogError("urlRedirect does not start with urlPrefix");
        goto bail;
    }
    const char* subprot = (cmdPtr->protocol&&cmdPtr->protocol[0]?cmdPtr->protocol:"ws");
    if (cmdPtr->protocol && !cmdPtr->protocol[0])
        Jsi_LogWarn("empty protocol string: forcing to 'ws'");
    cmdPtr->protocols[JWS_PROTOCOL_HTTP].name="http-only";
    cmdPtr->protocols[JWS_PROTOCOL_HTTP].callback=jsi_wscallback_http;
    cmdPtr->protocols[JWS_PROTOCOL_HTTP].per_session_data_size=sizeof(jsi_wsUser);
    cmdPtr->protocols[JWS_PROTOCOL_WEBSOCK].name=subprot;
    cmdPtr->protocols[JWS_PROTOCOL_WEBSOCK].callback=jsi_wscallback_websock;
    cmdPtr->protocols[JWS_PROTOCOL_WEBSOCK].per_session_data_size=sizeof(jsi_wsUser);

    if (cmdPtr->bufferPwr2 == 0)
        cmdPtr->bufferPwr2 = 16;
    if (cmdPtr->bufferPwr2>0) {
        if (cmdPtr->bufferPwr2>20) {
            Jsi_LogError("bufferPwr2 not in 0-20: %d", cmdPtr->bufferPwr2);
            goto bail;
        }
        cmdPtr->protocols[JWS_PROTOCOL_WEBSOCK].rx_buffer_size=(1<<cmdPtr->bufferPwr2);
    }

    cmdPtr->pssTable = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, jsi_wsfreePss);
    if (cmdPtr->onModify)
        cmdPtr->fileHash = Jsi_HashNew(interp, JSI_KEYS_STRING, jsi_wsfreeFile);

    cmdPtr->info.port = (cmdPtr->client ? CONTEXT_PORT_NO_LISTEN : cmdPtr->port);
    cmdPtr->info.user = cmdPtr;
    cmdPtr->info.iface = cmdPtr->interface ? Jsi_ValueString(interp, cmdPtr->interface, NULL) : NULL;
    if (cmdPtr->local && !cmdPtr->info.iface)
        cmdPtr->info.iface = "lo";
#ifdef __WIN32
    cmdPtr->info.iface = NULL;
#endif
    cmdPtr->info.protocols = cmdPtr->protocols;
    if (!cmdPtr->noCompress)
        cmdPtr->info.extensions = jsi_lws_exts;

    cmdPtr->info.ssl_cert_filepath = cmdPtr->ssl_cert_filepath;
    cmdPtr->info.ssl_private_key_filepath = cmdPtr->ssl_private_key_filepath;
    cmdPtr->info.gid = cmdPtr->ws_gid;
    cmdPtr->info.uid = cmdPtr->ws_uid;
    cmdPtr->opts = LWS_SERVER_OPTION_SKIP_SERVER_CANONICAL_NAME|LWS_SERVER_OPTION_VALIDATE_UTF8;
    cmdPtr->info.options = cmdPtr->opts;
    cmdPtr->info.max_http_header_pool = 16;
    cmdPtr->info.timeout_secs = 5;
    cmdPtr->info.ssl_cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:"
                   "ECDHE-RSA-AES256-GCM-SHA384:"
                   "DHE-RSA-AES256-GCM-SHA384:"
                   "ECDHE-RSA-AES256-SHA384:"
                   "HIGH:!aNULL:!eNULL:!EXPORT:"
                   "!DES:!MD5:!PSK:!RC4:!HMAC_SHA1:"
                   "!SHA1:!DHE-RSA-AES128-GCM-SHA256:"
                   "!DHE-RSA-AES128-SHA256:"
                   "!AES128-GCM-SHA256:"
                   "!AES128-SHA256:"
                   "!DHE-RSA-AES256-SHA256:"
                   "!AES256-GCM-SHA384:"
                   "!AES256-SHA256";

    lws_set_log_level(cmdPtr->debug>255?cmdPtr->debug/256:0, NULL);
    // TODO: WS2.2 Still leaks a small amount if server port unavailable.
    if (!cmdPtr->client)
        cmdPtr->info.options |= LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
    cmdPtr->context = lws_create_context(&cmdPtr->info);
    if (cmdPtr->context == NULL) {
fail:
        Jsi_LogError("libwebsocket init failed on port %d (try another port?)", cmdPtr->info.port);
        goto bail;
    }
    if (cmdPtr->info.options & LWS_SERVER_OPTION_EXPLICIT_VHOSTS) {
        cmdPtr->info.options &= ~LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
        if (!lws_create_vhost(cmdPtr->context, &cmdPtr->info))
            goto fail;
    }

    if (cmdPtr->client) {
        struct lws_client_connect_info lci = {};
        lci.context = cmdPtr->context;
        lci.address = cmdPtr->address ? Jsi_ValueString(cmdPtr->interp, cmdPtr->address, NULL) : "127.0.0.1";
        lci.port = cmdPtr->port;
        lci.ssl_connection = cmdPtr->use_ssl;
        lci.path = Jsi_ValueString(cmdPtr->interp, cmdPtr->rootdir, NULL);
        lci.host = cmdPtr->clientHost?cmdPtr->clientHost:lws_canonical_hostname( cmdPtr->context );
        lci.origin = cmdPtr->clientOrigin?cmdPtr->clientOrigin:"origin";
        lci.protocol = cmdPtr->protocols[JWS_PROTOCOL_WEBSOCK].name;
        lci.ietf_version_or_minus_one = cmdPtr->ietf_version;
#if (LWS_LIBRARY_VERSION_MAJOR>1)
        if (cmdPtr->post)
            lci.method = "POST";
        else if (!Jsi_Strcmp(subprot, "get"))
            lci.method = "GET";
#endif

        if (NULL == lws_client_connect_via_info(&lci))
        {
            Jsi_LogError("websock connect failed");
            jsi_wswebsocketObjFree(interp, cmdPtr);
            return JSI_ERROR;
        }
    } else if (cmdPtr->port == 0) {
        // Extract actually used port.
        char *cp, cbuf[JSI_BUFSIZ*2];
        cbuf[0] = 0;
        lws_json_dump_context(cmdPtr->context, cbuf, sizeof(cbuf), 0);
        cp = Jsi_Strstr(cbuf, "\"port\":\"");
        if (cp)
            cmdPtr->port = atoi(cp+8);
    }

    cmdPtr->event = Jsi_EventNew(interp, jsi_wswebsockUpdate, cmdPtr);
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        toacc = _this;
    } else {
        Jsi_Obj *o = Jsi_ObjNew(interp);
        Jsi_PrototypeObjSet(interp, "WebSocket", o);
        Jsi_ValueMakeObject(interp, ret, o);
        toacc = *ret;
    }

    Jsi_Obj *fobj = Jsi_ValueGetObj(interp, toacc);
    if ((cmdPtr->objId = Jsi_UserObjNew(interp, &websockobject, fobj, cmdPtr))<0) {
        goto bail;
    }
    Jsi_UserObjName(interp, toacc, &cmdPtr->cName);

    cmdPtr->handlers = Jsi_HashNew(interp, JSI_KEYS_STRING, jsi_wsfreeHandlers);
    if (cmdPtr->extHandlers) {
        jsi_wsHandlerAdd(interp, cmdPtr, ".jsi",   "Jspp",     1);
        jsi_wsHandlerAdd(interp, cmdPtr, ".htmli", "Htmlpp",   1);
        jsi_wsHandlerAdd(interp, cmdPtr, ".cssi",  "Csspp",    1);
    }
    cmdPtr->fobj = fobj;
#ifdef LWS_LIBRARY_VERSION_NUMBER
    Jsi_JSONParseFmt(interp, &cmdPtr->version, "{libVer:\"%s\", hdrVer:\"%s\", hdrNum:%d, pkgVer:%d}",
        (char *)lws_get_library_version(), LWS_LIBRARY_VERSION, LWS_LIBRARY_VERSION_NUMBER, jsi_WsPkgVersion);
#endif
    return JSI_OK;
}

static Jsi_RC Jsi_DoneWebSocket(Jsi_Interp *interp)
{
    Jsi_UserObjUnregister(interp, &websockobject);
    Jsi_PkgProvide(interp, "WebSocket", -1, NULL);
    return JSI_OK;
}

Jsi_RC Jsi_InitWebSocket(Jsi_Interp *interp, int release)
{
    if (release)
        return Jsi_DoneWebSocket(interp);
#ifdef LWS_OPENSSL_SUPPORT
    Jsi_InterpAccess(interp, NULL, JSI_INTACCESS_SETSSL );
#endif
    Jsi_Hash *wsys;
    const char *libver = lws_get_library_version();
    int lvlen = sizeof(LWS_LIBRARY_VERSION)-1;
    if (Jsi_Strncmp(libver, LWS_LIBRARY_VERSION, lvlen) || !isspace(libver[lvlen]))
        return Jsi_LogError("Library version mismatch: HDR:%s != LIB:%s", LWS_LIBRARY_VERSION, libver);
#if JSI_USE_STUBS
  if (Jsi_StubsInit(interp, 0) != JSI_OK)
    return JSI_ERROR;
#endif
    Jsi_Value *info = Jsi_ValueNew1(interp);
    Jsi_JSONParseFmt(interp, &info, "{libVer:\"%s\", hdrVer:\"%s\", pkgVer:%d}",
        libver, LWS_LIBRARY_VERSION, jsi_WsPkgVersion);
    Jsi_PkgOpts wsPkgOpts = { wsObjCmd_Specs, &wsObjCmd, websockCmds, info };
    Jsi_RC rc = Jsi_PkgProvideEx(interp, "WebSocket", jsi_WsPkgVersion, Jsi_InitWebSocket, &wsPkgOpts);
    Jsi_DecrRefCount(interp, info);
    if (rc != JSI_OK)
        return JSI_ERROR;
    if (!(wsys = Jsi_UserObjRegister(interp, &websockobject))) {
        Jsi_LogBug("Can not init webSocket");
        return JSI_ERROR;
    }

    if (!Jsi_CommandCreateSpecs(interp, websockobject.name, websockCmds, wsys, JSI_CMDSPEC_ISOBJ))
        return JSI_ERROR;
    return JSI_OK;
}

#endif
#endif
