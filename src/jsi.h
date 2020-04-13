/* jsi.h : External API header file for Jsi. */
#ifndef __JSI_H__
#define __JSI_H__

#define JSI_VERSION_MAJOR   3
#define JSI_VERSION_MINOR   0
#define JSI_VERSION_RELEASE 7

#define JSI_VERSION (JSI_VERSION_MAJOR + ((Jsi_Number)JSI_VERSION_MINOR/100.0) + ((Jsi_Number)JSI_VERSION_RELEASE/10000.0))

#ifndef JSI_EXTERN
#define JSI_EXTERN extern
#endif

#ifdef offsetof
#define Jsi_Offset(type, field) ((long) offsetof(type, field))
#else
#define Jsi_Offset(type, field) ((long) ((char *) &((type *) 0)->field))
#endif

#ifndef __GNUC__
#define __attribute__(X)
#endif

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef __WIN32
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>
#include <stdio.h> 
#include <dirent.h>

/* --TYPEDEFS-- */
typedef int64_t Jsi_Wide;
typedef uint64_t Jsi_UWide;
typedef long double ldouble;
#ifdef JSI_USE_LONG_DOUBLE
typedef ldouble Jsi_Number;
#define JSI_NUMLMOD "L"
#else
typedef double Jsi_Number;
#define JSI_NUMLMOD
#endif
typedef double time_d;
typedef int64_t time_w;
typedef uint32_t Jsi_Sig; // Signature field

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned short ushort;
#define JSI_NUMGFMT JSI_NUMLMOD "g"
#define JSI_NUMFFMT JSI_NUMLMOD "f"
#define JSI_NUMEFMT JSI_NUMLMOD "e"
/* -- */


/* --ENUMS-- */
typedef enum {
    /* Jsi Return Codes. */
    JSI_OK=0, JSI_ERROR=1, JSI_RETURN=2, JSI_BREAK=3,
    JSI_CONTINUE=4, JSI_SIGNAL=5, JSI_EXIT=6, JSI_EVAL=7,    
} Jsi_RC;

typedef enum {
    JSI_MAP_NONE, JSI_MAP_HASH, JSI_MAP_TREE, JSI_MAP_LIST /*, JSI_MAP_STACK */
} Jsi_Map_Type;

typedef enum {
    JSI_KEYS_STRING = 0,    // A string that gets stored in hash.
    JSI_KEYS_STRINGKEY = 1, // A pointer to strings in another hash such as Jsi_KeyLookup()
    JSI_KEYS_ONEWORD = 2,   // A pointer.
    JSI_KEYS_RESERVED = 3,  // Unused.
    JSI_KEYS_STRUCT_MINSIZE = 4 // Any number >= 4 is the number of bytes in a struct/key.
} Jsi_Key_Type;

typedef enum {
    JSI_OT_UNDEF,       /* Undefined */
    JSI_OT_BOOL,        /* Boolean object, use d.val */
    JSI_OT_NUMBER,      /* Number object, use d.num */
    JSI_OT_STRING,      /* String object, use d.str */
    JSI_OT_OBJECT,      /* Common object */
    JSI_OT_ARRAY,       /* NOT A REAL TYPE: is just an JSI_OT_OBJECT with array elements */
    JSI_OT_FUNCTION,    /* Function object, use d.fobj */
    JSI_OT_REGEXP,      /* RegExp object, use d.robj */
    JSI_OT_ITER,        /* Iter object, use d.iobj */
    JSI_OT_USEROBJ,     /* UserDefined object, use d.uobj */
    JSI_OT__MAX = JSI_OT_USEROBJ
} Jsi_otype;

typedef enum {          /* TYPE         CONSTRUCTOR JSI_VALUE-DATA  IMPLICIT-PROTOTYPE  */
    JSI_VT_UNDEF,       /* undefined    none        none            none                */
    JSI_VT_BOOL,        /* boolean      Boolean     d.val           none                */
    JSI_VT_NUMBER,      /* number       Number      d.num           Number.prototype    */
    JSI_VT_STRING,      /* string       String      d.str           String.prototype    */
    JSI_VT_OBJECT,      /* object       Jsi_Obj     d.obj           Jsi_Obj.prototype   */
    JSI_VT_NULL,        /* null         none        none            none                */
    JSI_VT_VARIABLE,    /* lvalue       none        d.lval          none                */
    JSI_VT__MAX = JSI_VT_VARIABLE
} Jsi_vtype;

typedef enum {
    JSI_TT_UNDEFINED= (1<<JSI_OT_UNDEF),    //  0x1
    JSI_TT_BOOLEAN  = (1<<JSI_OT_BOOL),     //  0x2
    JSI_TT_NUMBER   = (1<<JSI_OT_NUMBER),   //  0x4
    JSI_TT_STRING   = (1<<JSI_OT_STRING),   //  0x8
    JSI_TT_OBJECT   = (1<<JSI_OT_OBJECT),   //  0x10
    JSI_TT_ARRAY    = (1<<JSI_OT_ARRAY),    //  0x20
    JSI_TT_FUNCTION = (1<<JSI_OT_FUNCTION), //  0x40
    JSI_TT_REGEXP   = (1<<JSI_OT_REGEXP),   //  0x80
    JSI_TT_ITEROBJ  = (1<<JSI_OT_ITER),     //  0x100
    JSI_TT_USEROBJ  = (1<<JSI_OT_USEROBJ),  //  0x200
    JSI_TT_NULL     = (1<<(JSI_OT_USEROBJ+1)),//0x400
    JSI_TT_ANY      = (1<<(JSI_OT_USEROBJ+2)),//0x800
    JSI_TT_VOID     = (1<<(JSI_OT_USEROBJ+3)) //0x1000
} Jsi_ttype;

typedef enum {
    /* General flags. */
    JSI_NONE=0, JSI_NO_ERRMSG=1, 
    JSI_CMP_NOCASE=1, JSI_CMP_CHARSET_SCAN=2,
    JSI_CMP_EXACT=0x4,
    JSI_EVAL_ARGV0=0x1, JSI_EVAL_GLOBAL=0x2, JSI_EVAL_NOSKIPBANG=0x4, JSI_EVAL_AUTOINDEX=0x8,
    JSI_EVAL_RETURN         =0x10, // Return top of stack as result
    JSI_EVAL_ONCE           =0x20, // Source files only once.
    JSI_EVAL_ISMAIN         =0x40, // Set isMain to true.
    JSI_EVAL_EXISTS         =0x80, // Source if exists.
    JSI_EVAL_ERRIGNORE      =0x100,// Source ignores errors.

    /* Flags for Jsi_CmdProc */
    JSI_CALL_CONSTRUCTOR    =0x1,
    JSI_CALL_BUILTIN        =0x2,
    
    JSI_CMDSPEC_ISOBJ       = 0x1,
    JSI_CMDSPEC_PROTO       = 0x2,
    JSI_CMDSPEC_NONTHIS     = 0x4,
    JSI_CMDSPEC_SUBCMDS     = 0x8,      // Has sub-commands.
    
    JSI_CMD_HAS_ATTR        = 0x100,
    JSI_CMD_IS_CONSTRUCTOR  = 0x200,
    JSI_CMD_IS_OBJ          = 0x400,
    JSI_CMD_LOG_TEST        = 0x1000,
    JSI_CMD_LOG_DEBUG       = 0x2000,
    JSI_CMD_LOG_TRACE       = 0x4000,
    JSI_CMD_MASK            = 0xffff,
    
    JSI_OM_READONLY         = 0x01,     /* ecma read-only */
    JSI_OM_DONTENUM         = 0x02,     /* ecma emumerable */
    JSI_OM_DONTDEL          = 0x04,     /* ecma configurable */
    JSI_OM_INNERSHARED      = 0x08,
    JSI_OM_ISARRAYLIST      = 0x10,
    JSI_OM_ISSTRKEY         = 0x20,
    JSI_OM_UNUSED           = 0x40,
    JSI_OM_ISSTATIC         = 0x80,
    
    JSI_INTACCESS_READ      = 0x0,
    JSI_INTACCESS_WRITE     = 0x1,
    JSI_INTACCESS_NETWORK   = 0x2,
    JSI_INTACCESS_SETSSL    = 0x3,
    JSI_INTACCESS_MAININTERP= 0x4,
    JSI_INTACCESS_CREATE    = 0x5,
    
    JSI_LOG_BUG=0,   JSI_LOG_ERROR,   JSI_LOG_WARN,
    JSI_LOG_INFO,    JSI_LOG_UNUSED,  JSI_LOG_PARSE,
    JSI_LOG_TEST,    JSI_LOG_DEBUG,   JSI_LOG_TRACE,
    JSI__LOGLAST=JSI_LOG_TRACE,
    
    JSI_SORT_NOCASE = 0x1, JSI_SORT_DESCEND = 0x2, JSI_SORT_DICT = 0x4,
    
    JSI_NAME_FUNCTIONS = 0x1, JSI_NAME_DATA = 0x2,
    
    JSI_TREE_ORDER_IN=0, JSI_TREE_ORDER_PRE=0x10, JSI_TREE_ORDER_POST=0x20, // Jsi_TreeSearchFirst()
    JSI_TREE_ORDER_LEVEL=0x30, JSI_TREE_ORDER_MASK=0x30,
    JSI_TREE_SEARCH_KEY=0x10, // Use key even if NULL
    JSI_TREE_USERFLAG_MASK=0x7f,
    JSI_LIST_REVERSE=0x8, // Jsi_ListSearchFirst
    JSI_MUTEX_RECURSIVE=2,
    
    JSI_FS_NOCLOSE=0x1, JSI_FS_READONLY=0x2, JSI_FS_WRITEONLY=0x4, JSI_FS_APPEND=0x8,
    JSI_FS_COMPRESS=0x100,
    JSI_FSMODESIZE=15,
    JSI_FILE_TYPE_FILES=0x1, JSI_FILE_TYPE_DIRS=0x2,    JSI_FILE_TYPE_MOUNT=0x4,
    JSI_FILE_TYPE_LINK=0x8,  JSI_FILE_TYPE_PIPE=0x10,   JSI_FILE_TYPE_BLOCK=0x20,
    JSI_FILE_TYPE_CHAR=0x40, JSI_FILE_TYPE_SOCKET=0x80, JSI_FILE_TYPE_HIDDEN=0x100,
    
    JSI_OUTPUT_QUOTE = 0x1,
    JSI_OUTPUT_JSON = 0x2,
    JSI_OUTPUT_NEWLINES = 0x4,
    JSI_OUTPUT_STDERR = 0x8,
    JSI_JSON_STATIC_DEFAULT =100,
    JSI_JSON_STRICT   = 0x101, /* property names must be quoted. */
    JSI_STUBS_STRICT  = 0x1, JSI_STUBS_SIG = 0xdeadaa00, JSI_SIG_TYPEDEF,
    JSI_SIG_OPTS = 0xdeadab00,
    JSI_SIG_OPTS_STRUCT, JSI_SIG_OPTS_ENUM, JSI_SIG_OPTS_VARDEF, JSI_SIG_OPTS_FIELD,
    JSI_SIG_OPTS_USER1=0xdeadab20,

    JSI_EVENT_TIMER=0, JSI_EVENT_SIGNAL=1, JSI_EVENT_ALWAYS=2,
    JSI_ZIP_MAIN=0x1,  JSI_ZIP_INDEX=0x2,

    JSI_DBI_READONLY     =0x0001, /* Db is created readonly */
    JSI_DBI_NOCREATE     =0x0002, /* Db must already exist. */
    JSI_DBI_NO_MUTEX     =0x0004, /* Disable mutex. */
    JSI_DBI_FULL_MUTEX   =0x0008, /* Use full mutex. */
    
    JSI_MAX_NUMBER_STRING=100,
    JSI_BUFSIZ=8192

} Jsi_Enums; /* Debugging is easier with enums than #define. */

/* -- */


/* --STRUCTS-- */

typedef struct Jsi_Interp Jsi_Interp;
typedef struct Jsi_Obj Jsi_Obj;
typedef struct Jsi_Value Jsi_Value;
typedef struct Jsi_Func Jsi_Func;
typedef struct Jsi_IterObj Jsi_IterObj;
typedef struct Jsi_FuncObj Jsi_FuncObj;
typedef struct Jsi_UserObjReg Jsi_UserObjReg;
typedef struct Jsi_UserObj Jsi_UserObj;
typedef struct Jsi_HashEntry Jsi_HashEntry;
typedef struct Jsi_Hash Jsi_Hash;
typedef struct Jsi_HashSearch Jsi_HashSearch;
typedef struct Jsi_TreeEntry Jsi_TreeEntry;
typedef struct Jsi_Tree Jsi_Tree;
typedef struct Jsi_TreeSearch Jsi_TreeSearch;
typedef struct Jsi_List Jsi_List;
typedef struct Jsi_ListEntry Jsi_ListEntry;
typedef struct Jsi_ListSearch Jsi_ListSearch;
typedef struct Jsi_Map Jsi_Map;
typedef struct Jsi_MapEntry Jsi_MapEntry;
typedef struct Jsi_MapSearch Jsi_MapSearch;
typedef struct Jsi_Regex_ Jsi_Regex;
typedef struct Jsi_Db Jsi_Db;
typedef struct Jsi_DbBinds Jsi_DbBinds;
typedef struct Jsi_Mutex Jsi_Mutex;
typedef struct Jsi_ScopeStrs Jsi_ScopeStrs;
typedef struct Jsi_OpCodes Jsi_OpCodes;
typedef struct Jsi_Chan* Jsi_Channel;
typedef struct Jsi_CS_Ctx Jsi_CS_Ctx;
typedef struct Jsi_OptionSpec Jsi_OptionSpec;

typedef struct Jsi_OptionSpec Jsi_StructSpec;
typedef struct Jsi_OptionSpec Jsi_FieldSpec;
typedef struct Jsi_OptionSpec Jsi_EnumSpec;
typedef struct Jsi_OptionSpec Jsi_VarSpec;

typedef Jsi_RC (Jsi_InitProc)(Jsi_Interp *interp, int release); // When release>1, the main interp is exiting.
typedef Jsi_RC (Jsi_DeleteProc)(Jsi_Interp *interp, void *data);
typedef Jsi_RC (Jsi_EventHandlerProc)(Jsi_Interp *interp, void *data);
typedef Jsi_RC (Jsi_ValueHandlerProc)(Jsi_Interp *interp, Jsi_Value *v, struct Jsi_OptionSpec* spec, void *record);
typedef void (Jsi_DeleteVoidProc)(void *data);
typedef Jsi_RC (Jsi_csgset)(Jsi_Interp *interp, void *data, Jsi_Wide *s, Jsi_OptionSpec *spec, int idx, bool isSet);
typedef int (Jsi_IterProc)(Jsi_IterObj *iterObj, Jsi_Value *val, Jsi_Value *var, int index);

/* -- */


/* --INTERP-- */

/* Options and flags for Jsi_InterpNew/Jsi_Main */
typedef struct {
    int argc;                   // Arguments from main().
    char **argv;                // ...
    Jsi_InitProc* initProc;     // Initialization proc
    uint mem_debug:2;           // Memory debug level;
    bool no_interactive:1;      // Jsi_Main: does not default to interactive mode when no script arg given.
    bool auto_delete:1;         // Jsi_Main: auto delete interp upon return.
    bool no_exit:1;             // Do not exit, even on error.
    uint reserved:11;           // Reserved for future use.
    int exitCode:16;            // Call exit with this code.
    Jsi_Interp* interp;         // Jsi_InterpNew sets this to let Jsi_Main use this interp.
    void *reserved2[8];         // Reserved for future
} Jsi_InterpOpts;

JSI_EXTERN Jsi_Interp* Jsi_InterpNew(Jsi_InterpOpts *opts); /*STUB = 1*/
JSI_EXTERN void Jsi_InterpDelete( Jsi_Interp* interp); /*STUB = 2*/
JSI_EXTERN void Jsi_InterpOnDelete(Jsi_Interp *interp, Jsi_DeleteProc *freeProc, void *ptr);  /*STUB = 3*/
JSI_EXTERN Jsi_RC Jsi_Interactive(Jsi_Interp* interp, int flags); /*STUB = 4*/
JSI_EXTERN bool Jsi_InterpGone( Jsi_Interp* interp); /*STUB = 5*/
JSI_EXTERN Jsi_Value* Jsi_InterpResult(Jsi_Interp *interp); /*STUB = 6*/
JSI_EXTERN const char* Jsi_InterpLastError(Jsi_Interp *interp, const char **errFilePtr, int *errLinePtr); /*STUB = 7*/
JSI_EXTERN void* Jsi_InterpGetData(Jsi_Interp *interp, const char *key, Jsi_DeleteProc **proc); /*STUB = 8*/
JSI_EXTERN void Jsi_InterpSetData(Jsi_Interp *interp, const char *key, void *data, Jsi_DeleteProc *proc); /*STUB = 9*/
JSI_EXTERN void Jsi_InterpFreeData(Jsi_Interp *interp, const char *key); /*STUB = 10*/
JSI_EXTERN bool Jsi_InterpSafe(Jsi_Interp *interp); /*STUB = 11*/
JSI_EXTERN Jsi_RC Jsi_InterpAccess(Jsi_Interp *interp, Jsi_Value* resource, int aflag); /*STUB = 12*/
JSI_EXTERN Jsi_Interp* Jsi_Main(Jsi_InterpOpts *opts); /*STUB = 13*/
/* -- */


/* --MEMORY-- */
JSI_EXTERN void* Jsi_Malloc(uint size); /*STUB = 14*/
JSI_EXTERN void* Jsi_Calloc(uint n, uint size); /*STUB = 15*/
JSI_EXTERN void* Jsi_Realloc(void *m, uint size); /*STUB = 16*/
JSI_EXTERN void  Jsi_Free(void *m); /*STUB = 17*/
JSI_EXTERN int Jsi_ObjIncrRefCount(Jsi_Interp* interp, Jsi_Obj *obj); /*STUB = 18*/
JSI_EXTERN int Jsi_ObjDecrRefCount(Jsi_Interp* interp, Jsi_Obj *obj); /*STUB = 19*/
JSI_EXTERN int Jsi_IncrRefCount(Jsi_Interp* interp, Jsi_Value *v); /*STUB = 20*/
JSI_EXTERN int Jsi_DecrRefCount(Jsi_Interp* interp, Jsi_Value *v); /*STUB = 21*/
JSI_EXTERN bool Jsi_IsShared(Jsi_Interp* interp, Jsi_Value *v); /*STUB = 22*/
JSI_EXTERN Jsi_RC Jsi_DeleteData(Jsi_Interp* interp, void *m); /*STUB = 23*/
/* -- */


/* --STRINGS-- */
JSI_EXTERN uint Jsi_Strlen(const char *str); /*STUB = 24*/
JSI_EXTERN uint Jsi_StrlenSet(const char *str, uint len); /*STUB = 25*/
JSI_EXTERN int Jsi_Strcmp(const char *str1, const char *str2); /*STUB = 26*/
JSI_EXTERN int Jsi_Strncmp(const char *str1, const char *str2, int n); /*STUB = 27*/
JSI_EXTERN int Jsi_Strncasecmp(const char *str1, const char *str2, int n); /*STUB = 28*/
JSI_EXTERN int Jsi_StrcmpDict(const char *str1, const char *str2, int nocase, int dict); /*STUB = 29*/
JSI_EXTERN char* Jsi_Strcpy(char *dst, const char *src); /*STUB = 30*/
JSI_EXTERN char* Jsi_Strncpy(char *dst, const char *src, int len); /*STUB = 31*/
JSI_EXTERN char* Jsi_Strdup(const char *n); /*STUB = 32*/
JSI_EXTERN char* Jsi_StrdupLen(const char *str, int len); /*STUB = 407*/
JSI_EXTERN char* Jsi_Strrchr(const char *str, int c); /*STUB = 33*/
JSI_EXTERN char* Jsi_Strstr(const char *str, const char *sub); /*STUB = 34*/
JSI_EXTERN char* Jsi_Strrstr(const char *str, const char *sub); /*STUB = 233*/ 
JSI_EXTERN char* Jsi_Strchr(const char *str, int c); /*STUB = 36*/
JSI_EXTERN int Jsi_Strpos(const char *str, int start, const char *nid, int nocase); /*STUB = 37*/
JSI_EXTERN int Jsi_Strrpos(const char *str, int start, const char *nid, int nocase); /*STUB = 38*/
JSI_EXTERN bool Jsi_StrIsAlnum(const char *cp); /*STUB = 416*/
#define Jsi_Stzcpy(buf,src) Jsi_Strncpy(buf, src, sizeof(buf))

/* Dynamic strings. */
#ifndef JSI_DSTRING_STATIC_SIZE
#define JSI_DSTRING_STATIC_SIZE 200
#endif

typedef struct {
#define JSI_DSTRING_DECL_FIELDS(siz) \
    const char *strA; /* Allocated string, or = {"string"}.*/ \
    uint len;       /* Length of string. */ \
    uint spaceAvl;  /* Amount of space available or allocated. */ \
    uint staticSize;/* The sizeof "Str", or 0 if used "= {}" */ \
    char Str[siz];  /* Static string */
    JSI_DSTRING_DECL_FIELDS(JSI_DSTRING_STATIC_SIZE)
} Jsi_DString;

/* Declares a custom Jsi_DString* variable with other than default size... */
#define JSI_DSTRING_VAR(namPtr, siz) \
    struct { JSI_DSTRING_DECL_FIELDS(siz) } _STATIC_##namPtr; \
    Jsi_DString *namPtr = (Jsi_DString *)&_STATIC_##namPtr; \
    namPtr->staticSize = siz; namPtr->strA=0; \
    namPtr->Str[0] = 0; namPtr->spaceAvl = namPtr->len = 0

JSI_EXTERN char*   Jsi_DSAppendLen(Jsi_DString *dsPtr,const char *bytes, int length);  /*STUB = 39*/
JSI_EXTERN char*   Jsi_DSAppend(Jsi_DString *dsPtr, const char *str, ...)  /*STUB = 40*/  __attribute__((sentinel));
JSI_EXTERN void    Jsi_DSFree(Jsi_DString *dsPtr);  /*STUB = 41*/
JSI_EXTERN char*   Jsi_DSFreeDup(Jsi_DString *dsPtr);  /*STUB = 42*/
JSI_EXTERN void    Jsi_DSInit(Jsi_DString *dsPtr);  /*STUB = 43*/
JSI_EXTERN uint    Jsi_DSLength(Jsi_DString *dsPtr);  /*STUB = 44*/
JSI_EXTERN char*   Jsi_DSPrintf(Jsi_DString *dsPtr, const char *fmt, ...)  /*STUB = 45*/ __attribute__((format (printf,2,3)));
JSI_EXTERN char*   Jsi_DSSet(Jsi_DString *dsPtr, const char *str);  /*STUB = 46*/
JSI_EXTERN uint    Jsi_DSSetLength(Jsi_DString *dsPtr, uint length);  /*STUB = 47*/
JSI_EXTERN char*   Jsi_DSValue(Jsi_DString *dsPtr);  /*STUB = 48*/
/* -- */


/* --FUNC/VAR/CMD-- */
typedef void (Jsi_DelCmdProc)(Jsi_Interp *interp, void *privData);
typedef Jsi_RC (Jsi_CmdProc)(Jsi_Interp *interp, Jsi_Value *args, 
    Jsi_Value *_this, Jsi_Value **ret, Jsi_Func *funcPtr);
#define Jsi_CmdProcDecl(name,...) Jsi_RC name(Jsi_Interp *interp, Jsi_Value *args, \
    Jsi_Value *_this, Jsi_Value **ret, Jsi_Func *funcPtr, ##__VA_ARGS__)

typedef struct Jsi_CmdSpec {
    const char *name;       /* Cmd name */
    Jsi_CmdProc *proc;      /* Command handler */
    int minArgs;
    int maxArgs;            /* Max args or -1 */
    const char *argStr;     /* Argument description */
    const char *help;       /* Short help string. */
    uint retType;           /* Return type(s) or'ed Jsi_otype. */
    int flags;              /* JSI_CMD_* flags. */
    const char *info;       /* Detailed description. Use JSI_DETAIL macro. */
    Jsi_OptionSpec *opts;   /* Options for arg, default is first. */
    Jsi_DelCmdProc *delProc;/* Callback to handle command delete. */
    void *reserved[4];      /* Reserved for internal use. */
} Jsi_CmdSpec;

typedef struct {
    bool Test;
    bool Debug;
    bool Trace;
    int traceCall;
    bool coverage;
    bool profile;
} Jsi_ModuleConf;

typedef struct {
    struct Jsi_OptionSpec *spec;
    void *data;
    Jsi_CmdSpec *cmdSpec;
    Jsi_Value *info;
    void *reserved[3]; // Reserved for future use.
    Jsi_ModuleConf modConf;
    void *reserved2[3]; // Reserved for future use.
} Jsi_PkgOpts;

typedef struct {
    char *str;
    int32_t len;
    uint32_t flags;
} Jsi_String;

JSI_EXTERN Jsi_Value* Jsi_CommandCreate(Jsi_Interp *interp, const char *name, Jsi_CmdProc *cmdProc, void *privData); /*STUB = 49*/
JSI_EXTERN Jsi_Value* Jsi_CommandCreateSpecs(Jsi_Interp *interp, const char *name, Jsi_CmdSpec *cmdSpecs, void *privData, int flags); /*STUB = 50*/
JSI_EXTERN void* Jsi_CommandNewObj(Jsi_Interp *interp, const char *name, const char *arg1, const char *opts, const char *var);  /*STUB = 51*/
JSI_EXTERN Jsi_RC Jsi_CommandInvokeJSON(Jsi_Interp *interp, const char *cmd, const char *json, Jsi_Value **ret); /*STUB = 52*/
JSI_EXTERN Jsi_RC Jsi_CommandInvoke(Jsi_Interp *interp, const char *cmdstr, Jsi_Value *args, Jsi_Value **ret); /*STUB = 53*/
JSI_EXTERN Jsi_RC Jsi_CommandDelete(Jsi_Interp *interp, const char *name); /*STUB = 54*/
JSI_EXTERN Jsi_CmdSpec* Jsi_FunctionGetSpecs(Jsi_Func *funcPtr); /*STUB = 55*/
JSI_EXTERN bool Jsi_FunctionIsConstructor(Jsi_Func *funcPtr); /*STUB = 56*/
JSI_EXTERN bool Jsi_FunctionReturnIgnored(Jsi_Interp *interp, Jsi_Func *funcPtr); /*STUB = 57*/
JSI_EXTERN void* Jsi_FunctionPrivData(Jsi_Func *funcPtr); /*STUB = 58*/
JSI_EXTERN Jsi_RC Jsi_FunctionArguments(Jsi_Interp *interp, Jsi_Value *func, int *argcPtr); /*STUB = 59*/
JSI_EXTERN Jsi_RC Jsi_FunctionApply(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, Jsi_Value **ret); /*STUB = 60*/
JSI_EXTERN Jsi_RC Jsi_FunctionInvoke(Jsi_Interp *interp, Jsi_Value *tocall, Jsi_Value *args, Jsi_Value **ret, Jsi_Value *_this); /*STUB = 61*/
JSI_EXTERN Jsi_RC Jsi_FunctionInvokeJSON(Jsi_Interp *interp, Jsi_Value *tocall, const char *json, Jsi_Value **ret); /*STUB = 62*/
JSI_EXTERN int Jsi_FunctionInvokeBool(Jsi_Interp *interp, Jsi_Value *func, Jsi_Value *arg); /*STUB = 63*/
JSI_EXTERN Jsi_RC Jsi_FunctionInvokeString(Jsi_Interp *interp, Jsi_Value *func, Jsi_Value *arg, Jsi_DString *dStr); /*STUB = 64*/
JSI_EXTERN Jsi_Value* Jsi_VarLookup(Jsi_Interp *interp, const char *varname); /*STUB = 65*/
JSI_EXTERN Jsi_Value* Jsi_NameLookup(Jsi_Interp *interp, const char *varname); /*STUB = 66*/
JSI_EXTERN Jsi_Value* Jsi_NameLookup2(Jsi_Interp *interp, const char *name, const char *inObj); /*STUB = 67*/
JSI_EXTERN Jsi_RC Jsi_PkgProvideEx(Jsi_Interp *interp, const char *name, Jsi_Number version, Jsi_InitProc *initProc, Jsi_PkgOpts* popts); /*STUB = 68*/
JSI_EXTERN Jsi_Number Jsi_PkgRequireEx(Jsi_Interp *interp, const char *name, Jsi_Number version, Jsi_PkgOpts **poptsPtr); /*STUB = 69*/
JSI_EXTERN Jsi_Number Jsi_PkgVersion(Jsi_Interp *interp, const char *name, const char **filePtr); /*STUB = 70*/
#define Jsi_PkgRequire(i,n,v) Jsi_PkgRequireEx(i,n,v,NULL)
#define Jsi_PkgProvide(i,n,v,p) Jsi_PkgProvideEx(i,n,v,p,NULL)
/* -- */

/* UTF-8 and Unicode */
typedef int32_t Jsi_UniChar;
JSI_EXTERN uint Jsi_NumUtfBytes(char c); /*STUB = 71*/
JSI_EXTERN uint Jsi_NumUtfChars(const char *utf, int length); /*STUB = 72*/
JSI_EXTERN uint Jsi_UtfGetIndex(const char *utf, int index, char outbuf[5]); /*STUB = 73*/
JSI_EXTERN const char* Jsi_UtfAtIndex(const char *utf, int index); /*STUB = 74*/
JSI_EXTERN uint Jsi_UniCharToUtf(Jsi_UniChar uc, char *dest); /*STUB = 75*/
JSI_EXTERN uint Jsi_UtfToUniChar(const char *utf, Jsi_UniChar *ch); /*STUB = 76*/
JSI_EXTERN uint Jsi_UtfToUniCharCase(const char *utf, Jsi_UniChar *ch, int upper); /*STUB = 77*/
JSI_EXTERN uint Jsi_UtfDecode(const char *str, char* oututf); /*STUB = 78*/
JSI_EXTERN uint Jsi_UtfEncode(const char *utf, char *outstr); /*STUB = 79*/
JSI_EXTERN char* Jsi_UtfSubstr(const char *str, int n, int len, Jsi_DString *dStr); /*STUB = 80*/
JSI_EXTERN int Jsi_UtfIndexToOffset(const char *utf, int index); /*STUB = 81*/
/* -- */


/* --OBJECT-- */
JSI_EXTERN Jsi_Obj* Jsi_ObjNew(Jsi_Interp* interp); /*STUB = 82*/
JSI_EXTERN Jsi_Obj* Jsi_ObjNewType(Jsi_Interp* interp, Jsi_otype type); /*STUB = 83*/
JSI_EXTERN void Jsi_ObjFree(Jsi_Interp* interp, Jsi_Obj *obj); /*STUB = 84*/
JSI_EXTERN Jsi_Obj* Jsi_ObjNewObj(Jsi_Interp *interp, Jsi_Value **items, int count); /*STUB = 85*/
JSI_EXTERN Jsi_Obj* Jsi_ObjNewArray(Jsi_Interp *interp, Jsi_Value **items, int count, int copy); /*STUB = 86*/

JSI_EXTERN bool      Jsi_ObjIsArray(Jsi_Interp *interp, Jsi_Obj *o); /*STUB = 87*/
JSI_EXTERN void     Jsi_ObjSetLength(Jsi_Interp *interp, Jsi_Obj *obj, uint len); /*STUB = 88*/
JSI_EXTERN int      Jsi_ObjGetLength(Jsi_Interp *interp, Jsi_Obj *obj); /*STUB = 89*/
JSI_EXTERN const char* Jsi_ObjTypeStr(Jsi_Interp *interp, Jsi_Obj *obj); /*STUB = 90*/
JSI_EXTERN Jsi_otype Jsi_ObjTypeGet(Jsi_Obj *obj); /*STUB = 91*/
JSI_EXTERN void     Jsi_ObjListifyArray(Jsi_Interp *interp, Jsi_Obj *obj); /*STUB = 92*/
JSI_EXTERN Jsi_RC      Jsi_ObjArraySet(Jsi_Interp *interp, Jsi_Obj *obj, Jsi_Value *value, int arrayindex); /*STUB = 93*/
JSI_EXTERN Jsi_RC      Jsi_ObjArrayAdd(Jsi_Interp *interp, Jsi_Obj *o, Jsi_Value *v); /*STUB = 94*/
JSI_EXTERN Jsi_TreeEntry* Jsi_ObjInsert(Jsi_Interp *interp, Jsi_Obj *obj, const char *key, Jsi_Value *nv, int flags); /*STUB = 95*/
JSI_EXTERN void    Jsi_ObjFromDS(Jsi_DString *dsPtr, Jsi_Obj *obj);  /*STUB = 96*/
JSI_EXTERN Jsi_IterObj* Jsi_IterObjNew(Jsi_Interp *interp, Jsi_IterProc *iterProc); /*STUB = 412*/
JSI_EXTERN void Jsi_IterObjFree(Jsi_IterObj *iobj); /*STUB = 413*/
JSI_EXTERN void Jsi_IterGetKeys(Jsi_Interp *interp, Jsi_Value *target, Jsi_IterObj *iterobj, int depth); /*STUB = 414*/
JSI_EXTERN int Jsi_ObjArraySizer(Jsi_Interp *interp, Jsi_Obj *obj, uint n); /*STUB = 35*/

struct Jsi_IterObj {
    Jsi_Interp *interp;
    const char **keys;
    uint size; 
    uint count;
    uint iter;
    bool isArrayList;            /* If an array list do not store keys. */
    bool isof;
    Jsi_Obj *obj;
    uint cur;                    /* Current array cursor. */
    int depth;                  /* Used to create list of keys. */
    Jsi_IterProc *iterCmd;
};

/* -- */


/* --VALUE-- */
JSI_EXTERN Jsi_Value* Jsi_ValueNew(Jsi_Interp *interp); /*STUB = 97*/
JSI_EXTERN Jsi_Value* Jsi_ValueNew1(Jsi_Interp *interp); /*STUB = 98*/
JSI_EXTERN void Jsi_ValueFree(Jsi_Interp *interp, Jsi_Value* v); /*STUB = 99*/

JSI_EXTERN Jsi_Value* Jsi_ValueNewNull(Jsi_Interp *interp); /*STUB = 100*/
JSI_EXTERN Jsi_Value* Jsi_ValueNewBoolean(Jsi_Interp *interp, int bval); /*STUB = 101*/
JSI_EXTERN Jsi_Value* Jsi_ValueNewNumber(Jsi_Interp *interp, Jsi_Number n); /*STUB = 102*/
JSI_EXTERN Jsi_Value* Jsi_ValueNewBlob(Jsi_Interp *interp, uchar *s, uint len); /*STUB = 103*/
JSI_EXTERN Jsi_Value* Jsi_ValueNewString(Jsi_Interp *interp, const char *s, int len); /*STUB = 104*/
JSI_EXTERN Jsi_Value* Jsi_ValueNewStringKey(Jsi_Interp *interp, const char *s); /*STUB = 105*/
JSI_EXTERN Jsi_Value* Jsi_ValueNewStringConst(Jsi_Interp *interp, const char *s, int len); /*STUB = 409*/
JSI_EXTERN Jsi_Value* Jsi_ValueNewStringDup(Jsi_Interp *interp, const char *s); /*STUB = 106*/
JSI_EXTERN Jsi_Value* Jsi_ValueNewArray(Jsi_Interp *interp, const char **items, int count); /*STUB = 107*/
JSI_EXTERN Jsi_Value* Jsi_ValueNewObj(Jsi_Interp *interp, Jsi_Obj *o) ; /*STUB = 108*/
#define Jsi_ValueNewBlobString(interp, s) Jsi_ValueNewBlob(interp, (uchar*)s, Jsi_Strlen(s))
#define Jsi_ValueNewArrayObj(interp, items, count, copy) Jsi_ValueNewObj(interp, Jsi_ObjNewArray(interp, items, count, copy))

JSI_EXTERN Jsi_RC Jsi_GetStringFromValue(Jsi_Interp* interp, Jsi_Value *value, const char **s); /*STUB = 109*/
JSI_EXTERN Jsi_RC Jsi_GetNumberFromValue(Jsi_Interp* interp, Jsi_Value *value, Jsi_Number *n); /*STUB = 110*/
JSI_EXTERN Jsi_RC Jsi_GetBoolFromValue(Jsi_Interp* interp, Jsi_Value *value, bool *n); /*STUB = 111*/
JSI_EXTERN Jsi_RC Jsi_GetIntFromValue(Jsi_Interp* interp, Jsi_Value *value, int *n); /*STUB = 112*/
JSI_EXTERN Jsi_RC Jsi_GetLongFromValue(Jsi_Interp* interp, Jsi_Value *value, long *n); /*STUB = 113*/
JSI_EXTERN Jsi_RC Jsi_GetWideFromValue(Jsi_Interp* interp, Jsi_Value *value, Jsi_Wide *n); /*STUB = 114*/
JSI_EXTERN Jsi_RC Jsi_GetDoubleFromValue(Jsi_Interp* interp, Jsi_Value *value, Jsi_Number *n); /*STUB = 115*/
JSI_EXTERN Jsi_RC Jsi_GetIntFromValueBase(Jsi_Interp* interp, Jsi_Value *value, int *n, int base, int flags); /*STUB = 116*/
JSI_EXTERN Jsi_RC Jsi_ValueGetBoolean(Jsi_Interp *interp, Jsi_Value *pv, bool *val); /*STUB = 117*/
JSI_EXTERN Jsi_RC Jsi_ValueGetNumber(Jsi_Interp *interp, Jsi_Value *pv, Jsi_Number *val); /*STUB = 118*/

JSI_EXTERN bool Jsi_ValueIsType(Jsi_Interp *interp, Jsi_Value *pv, Jsi_vtype vtype); /*STUB = 119*/
JSI_EXTERN bool Jsi_ValueIsObjType(Jsi_Interp *interp, Jsi_Value *v, Jsi_otype otype); /*STUB = 120*/
JSI_EXTERN bool Jsi_ValueIsTrue(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 121*/
JSI_EXTERN bool Jsi_ValueIsFalse(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 122*/
JSI_EXTERN bool Jsi_ValueIsNumber(Jsi_Interp *interp, Jsi_Value *pv); /*STUB = 123*/
JSI_EXTERN bool Jsi_ValueIsArray(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 124*/
JSI_EXTERN bool Jsi_ValueIsBoolean(Jsi_Interp *interp, Jsi_Value *pv); /*STUB = 125*/
JSI_EXTERN bool Jsi_ValueIsNull(Jsi_Interp *interp, Jsi_Value *pv); /*STUB = 126*/
JSI_EXTERN bool Jsi_ValueIsUndef(Jsi_Interp *interp, Jsi_Value *pv); /*STUB = 127*/
JSI_EXTERN bool Jsi_ValueIsFunction(Jsi_Interp *interp, Jsi_Value *pv); /*STUB = 128*/
JSI_EXTERN bool Jsi_ValueIsString(Jsi_Interp *interp, Jsi_Value *pv); /*STUB = 129*/

JSI_EXTERN Jsi_Value* Jsi_ValueMakeObject(Jsi_Interp *interp, Jsi_Value **v, Jsi_Obj *o); /*STUB = 130*/
JSI_EXTERN Jsi_Value* Jsi_ValueMakeArrayObject(Jsi_Interp *interp, Jsi_Value **v, Jsi_Obj *o); /*STUB = 131*/
JSI_EXTERN Jsi_Value* Jsi_ValueMakeNumber(Jsi_Interp *interp, Jsi_Value **v, Jsi_Number n); /*STUB = 132*/
JSI_EXTERN Jsi_Value* Jsi_ValueMakeBool(Jsi_Interp *interp, Jsi_Value **v, int b); /*STUB = 133*/
JSI_EXTERN Jsi_Value* Jsi_ValueMakeString(Jsi_Interp *interp, Jsi_Value **v, const char *s); /*STUB = 134*/
JSI_EXTERN Jsi_Value* Jsi_ValueMakeStringKey(Jsi_Interp *interp, Jsi_Value **v, const char *s); /*STUB = 135*/
JSI_EXTERN Jsi_Value* Jsi_ValueMakeBlob(Jsi_Interp *interp, Jsi_Value **v, uchar *s, int len); /*STUB = 136*/
JSI_EXTERN Jsi_Value* Jsi_ValueMakeNull(Jsi_Interp *interp, Jsi_Value **v); /*STUB = 137*/
JSI_EXTERN Jsi_Value* Jsi_ValueMakeUndef(Jsi_Interp *interp, Jsi_Value **v); /*STUB = 138*/
JSI_EXTERN Jsi_Value* Jsi_ValueMakeDStringObject(Jsi_Interp *interp, Jsi_Value **v, Jsi_DString *dsPtr); /*STUB = 139*/
JSI_EXTERN bool Jsi_ValueIsStringKey(Jsi_Interp* interp, Jsi_Value *key); /*STUB = 140*/
#define Jsi_ValueMakeStringDup(interp, v, s) Jsi_ValueMakeString(interp, v, Jsi_Strdup(s))

JSI_EXTERN const char*  Jsi_ValueToString(Jsi_Interp *interp, Jsi_Value *v, int *lenPtr); /*STUB = 141*/
JSI_EXTERN Jsi_RC       Jsi_ValueToBool(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 142*/
JSI_EXTERN Jsi_RC       Jsi_ValueToNumber(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 143*/
JSI_EXTERN Jsi_Number   Jsi_ValueToNumberInt(Jsi_Interp *interp, Jsi_Value *v, int isInt); /*STUB = 144*/
JSI_EXTERN Jsi_RC       Jsi_ValueToObject(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 145*/

JSI_EXTERN void     Jsi_ValueReset(Jsi_Interp *interp, Jsi_Value **v); /*STUB = 146*/
JSI_EXTERN const char* Jsi_ValueGetDString(Jsi_Interp* interp, Jsi_Value* v, Jsi_DString *dStr, int quote); /*STUB = 147*/
JSI_EXTERN char*    Jsi_ValueString(Jsi_Interp* interp, Jsi_Value* v, int *lenPtr); /*STUB = 148*/
JSI_EXTERN uchar*   Jsi_ValueBlob(Jsi_Interp *interp, Jsi_Value* v, int *lenPtr); /*STUB = 149*/
JSI_EXTERN char*    Jsi_ValueGetStringLen(Jsi_Interp *interp, Jsi_Value *pv, int *lenPtr); /*STUB = 150*/
JSI_EXTERN int      Jsi_ValueStrlen(Jsi_Value* v); /*STUB = 151*/
JSI_EXTERN void     Jsi_ValueFromDS(Jsi_Interp *interp, Jsi_DString *dsPtr, Jsi_Value **ret);  /*STUB = 152*/
JSI_EXTERN int      Jsi_ValueInstanceOf( Jsi_Interp *interp, Jsi_Value* v1, Jsi_Value* v2); /*STUB = 153*/
JSI_EXTERN Jsi_Obj* Jsi_ValueGetObj(Jsi_Interp* interp, Jsi_Value* v); /*STUB = 154*/
JSI_EXTERN Jsi_vtype Jsi_ValueTypeGet(Jsi_Value *pv); /*STUB = 155*/
JSI_EXTERN const char* Jsi_ValueTypeStr(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 156*/
JSI_EXTERN int      Jsi_ValueCmp(Jsi_Interp *interp, Jsi_Value *v1, Jsi_Value* v2, int cmpFlags); /*STUB = 157*/
JSI_EXTERN Jsi_RC Jsi_ValueGetIndex( Jsi_Interp *interp, Jsi_Value *valPtr, const char **tablePtr, const char *msg, int flags, int *indexPtr); /*STUB = 158*/

JSI_EXTERN Jsi_RC Jsi_ValueArraySort(Jsi_Interp *interp, Jsi_Value *val, int sortFlags); /*STUB = 159*/
JSI_EXTERN Jsi_Value* Jsi_ValueArrayConcat(Jsi_Interp *interp, Jsi_Value *arg1, Jsi_Value *arg2); /*STUB = 160*/
JSI_EXTERN Jsi_RC Jsi_ValueArrayPush(Jsi_Interp *interp, Jsi_Value *arg1, Jsi_Value *arg2); /*STUB = 161*/
JSI_EXTERN Jsi_Value* Jsi_ValueArrayPop(Jsi_Interp *interp, Jsi_Value *arg1); /*STUB = 162*/
JSI_EXTERN void Jsi_ValueArrayShift(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 163*/
JSI_EXTERN Jsi_Value* Jsi_ValueArrayUnshift(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 164*/
JSI_EXTERN Jsi_Value* Jsi_ValueArrayIndex(Jsi_Interp *interp, Jsi_Value *args, int index); /*STUB = 165*/
JSI_EXTERN char* Jsi_ValueArrayIndexToStr(Jsi_Interp *interp, Jsi_Value *args, int index, int *lenPtr); /*STUB = 166*/
#define Jsi_ValueArraySet(interp, dest, value, index) Jsi_ObjArraySet(interp, Jsi_ValueGetObj(interp, dest), value, index)

#define Jsi_ValueInsertFixed(i,t,k,v) Jsi_ValueInsert(i,t,k,v,JSI_OM_READONLY | JSI_OM_DONTDEL | JSI_OM_DONTENUM)
JSI_EXTERN Jsi_RC Jsi_ValueInsert(Jsi_Interp *interp, Jsi_Value *target, const char *key, Jsi_Value *val, int flags); /*STUB = 167*/
JSI_EXTERN Jsi_RC Jsi_ValueInsertArray(Jsi_Interp *interp, Jsi_Value *target, int index, Jsi_Value *val, int flags); /*STUB = 411*/
JSI_EXTERN int Jsi_ValueGetLength(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 168*/
JSI_EXTERN Jsi_Value* Jsi_ValueObjLookup(Jsi_Interp *interp, Jsi_Value *target, const char *key, int iskeystr); /*STUB = 169*/
JSI_EXTERN bool Jsi_ValueKeyPresent(Jsi_Interp *interp, Jsi_Value *target, const char *k, int isstrkey); /*STUB = 170*/
JSI_EXTERN Jsi_RC Jsi_ValueGetKeys(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *ret); /*STUB = 171*/

JSI_EXTERN void Jsi_ValueCopy(Jsi_Interp *interp, Jsi_Value *to, Jsi_Value *from ); /*STUB = 172*/
JSI_EXTERN void Jsi_ValueReplace(Jsi_Interp *interp, Jsi_Value **to, Jsi_Value *from ); /*STUB = 173*/
JSI_EXTERN void Jsi_ValueDup2(Jsi_Interp *interp, Jsi_Value **to, Jsi_Value *from); /*STUB = 174*/
JSI_EXTERN Jsi_Value* Jsi_ValueDupJSON(Jsi_Interp *interp, Jsi_Value *val); /*STUB = 175*/
JSI_EXTERN void Jsi_ValueMove(Jsi_Interp *interp, Jsi_Value *to, Jsi_Value *from); /*STUB = 176*/
JSI_EXTERN bool  Jsi_ValueIsEqual(Jsi_Interp *interp, Jsi_Value *v1, Jsi_Value* v2); /*STUB = 177*/
/* -- */


/* --USEROBJ-- */
typedef bool (Jsi_UserObjIsTrueProc)(void *data);
typedef bool (Jsi_UserObjIsEquProc)(void *data1, void *data2);
typedef Jsi_Obj* (Jsi_UserGetObjProc)(Jsi_Interp *interp, void *data);

typedef struct Jsi_UserObjReg {
    const char *name;
    Jsi_CmdSpec *spec;
    Jsi_DeleteProc *freefun;
    Jsi_UserObjIsTrueProc *istrue;
    Jsi_UserObjIsEquProc *isequ;
} Jsi_UserObjReg;

JSI_EXTERN Jsi_Hash* Jsi_UserObjRegister    (Jsi_Interp *interp, Jsi_UserObjReg *reg); /*STUB = 178*/
JSI_EXTERN Jsi_RC Jsi_UserObjUnregister  (Jsi_Interp *interp, Jsi_UserObjReg *reg); /*STUB = 179*/
JSI_EXTERN int Jsi_UserObjNew    (Jsi_Interp *interp, Jsi_UserObjReg* reg, Jsi_Obj *obj, void *data); /*STUB = 180*/
JSI_EXTERN void* Jsi_UserObjGetData(Jsi_Interp *interp, Jsi_Value* value, Jsi_Func *funcPtr); /*STUB = 181*/
JSI_EXTERN Jsi_RC Jsi_UserObjName(Jsi_Interp *interp, Jsi_Value *v, Jsi_DString *dStr); /*STUB = 418*/ /*LAST*/
/* -- */


/* --UTILITY-- */
#define JSI_NOTUSED(n) (void)n /* Eliminate annoying compiler warning. */
JSI_EXTERN char* Jsi_NumberToString(Jsi_Interp *interp, Jsi_Number d, char *buf, int bsiz); /*STUB = 182*/
JSI_EXTERN Jsi_Number Jsi_Version(void); /*STUB = 183*/
JSI_EXTERN Jsi_Value* Jsi_ReturnValue(Jsi_Interp *interp); /*STUB = 184*/
JSI_EXTERN Jsi_RC Jsi_Mount( Jsi_Interp *interp, Jsi_Value *archive, Jsi_Value *mount, Jsi_Value **ret); /*STUB = 185*/
JSI_EXTERN Jsi_Value* Jsi_Executable(Jsi_Interp *interp); /*STUB = 186*/
JSI_EXTERN Jsi_Regex* Jsi_RegExpNew(Jsi_Interp *interp, const char *regtxt, int flag); /*STUB = 187*/
JSI_EXTERN void Jsi_RegExpFree(Jsi_Regex* re); /*STUB = 188*/
JSI_EXTERN Jsi_RC Jsi_RegExpMatch( Jsi_Interp *interp,  Jsi_Value *pattern, const char *str, int *rc, Jsi_DString *dStr); /*STUB = 189*/
JSI_EXTERN Jsi_RC Jsi_RegExpMatches(Jsi_Interp *interp, Jsi_Value *pattern, const char *str, int slen, Jsi_Value *ret); /*STUB = 190*/
JSI_EXTERN bool Jsi_GlobMatch(const char *pattern, const char *string, int nocase); /*STUB = 191*/
JSI_EXTERN char* Jsi_FileRealpath(Jsi_Interp *interp, Jsi_Value *path, char *newpath); /*STUB = 192*/
JSI_EXTERN char* Jsi_FileRealpathStr(Jsi_Interp *interp, const char *path, char *newpath); /*STUB = 193*/
JSI_EXTERN char* Jsi_NormalPath(Jsi_Interp *interp, const char *path, Jsi_DString *dStr); /*STUB = 194*/
JSI_EXTERN char* Jsi_ValueNormalPath(Jsi_Interp *interp, Jsi_Value *path, Jsi_DString *dStr); /*STUB = 195*/
JSI_EXTERN Jsi_RC Jsi_PathNormalize(Jsi_Interp *interp, Jsi_Value **pathPtr); /*STUB = 410*/
JSI_EXTERN Jsi_RC Jsi_JSONParse(Jsi_Interp *interp, const char *js, Jsi_Value **ret, int flags); /*STUB = 196*/
JSI_EXTERN Jsi_RC Jsi_JSONParseFmt(Jsi_Interp *interp, Jsi_Value **ret, const char *fmt, ...) /*STUB = 197*/ __attribute__((format (printf,3,4)));
JSI_EXTERN char* Jsi_JSONQuote(Jsi_Interp *interp, const char *str, int len, Jsi_DString *dStr); /*STUB = 198*/
JSI_EXTERN Jsi_RC Jsi_EvalString(Jsi_Interp* interp, const char *str, int flags); /*STUB = 199*/
JSI_EXTERN Jsi_RC Jsi_EvalFile(Jsi_Interp* interp, Jsi_Value *fname, int flags); /*STUB = 200*/
JSI_EXTERN Jsi_RC Jsi_EvalCmdJSON(Jsi_Interp *interp, const char *cmd, const char *jsonArgs, Jsi_DString *dStr, int flags); /*STUB = 201*/
JSI_EXTERN Jsi_RC Jsi_EvalZip(Jsi_Interp *interp, const char *exeFile, const char *mntDir, int *jsFound); /*STUB = 202*/
JSI_EXTERN int Jsi_DictionaryCompare(const char *left, const char *right); /*STUB = 203*/
JSI_EXTERN Jsi_RC Jsi_GetBool(Jsi_Interp* interp, const char *string, bool *n); /*STUB = 204*/
JSI_EXTERN Jsi_RC Jsi_GetInt(Jsi_Interp* interp, const char *string, int *n, int base); /*STUB = 205*/
JSI_EXTERN Jsi_RC Jsi_GetWide(Jsi_Interp* interp, const char *string, Jsi_Wide *n, int base); /*STUB = 206*/
JSI_EXTERN Jsi_RC Jsi_GetDouble(Jsi_Interp* interp, const char *string, Jsi_Number *n); /*STUB = 207*/
JSI_EXTERN Jsi_RC Jsi_FormatString(Jsi_Interp *interp, Jsi_Value *args, Jsi_DString *dStr); /*STUB = 208*/
JSI_EXTERN void Jsi_SplitStr(const char *str, int *argcPtr, char ***argvPtr,  const char *splitCh, Jsi_DString *dStr); /*STUB = 209*/
JSI_EXTERN Jsi_RC Jsi_Sleep(Jsi_Interp *interp, Jsi_Number dtim); /*STUB = 210*/
JSI_EXTERN void Jsi_Preserve(Jsi_Interp* interp, void *data); /*STUB = 211*/
JSI_EXTERN void Jsi_Release(Jsi_Interp* interp, void *data); /*STUB = 212*/
JSI_EXTERN void Jsi_EventuallyFree(Jsi_Interp* interp, void *data, Jsi_DeleteProc* proc); /*STUB = 213*/
JSI_EXTERN void Jsi_ShiftArgs(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 214*/
JSI_EXTERN Jsi_Value* Jsi_StringSplit(Jsi_Interp *interp, const char *str, const char *spliton); /*STUB = 215*/
JSI_EXTERN Jsi_RC Jsi_GetIndex( Jsi_Interp *interp, const char *str, const char **tablePtr, const char *msg, int flags, int *indexPtr); /*STUB = 216*/
JSI_EXTERN void* Jsi_PrototypeGet(Jsi_Interp *interp, const char *key); /*STUB = 217*/
JSI_EXTERN Jsi_RC  Jsi_PrototypeDefine(Jsi_Interp *interp, const char *key, Jsi_Value *proto); /*STUB = 218*/
JSI_EXTERN Jsi_RC Jsi_PrototypeObjSet(Jsi_Interp *interp, const char *key, Jsi_Obj *obj); /*STUB = 219*/
JSI_EXTERN Jsi_RC Jsi_ThisDataSet(Jsi_Interp *interp, Jsi_Value *_this, void *value); /*STUB = 220*/
JSI_EXTERN void* Jsi_ThisDataGet(Jsi_Interp *interp, Jsi_Value *_this); /*STUB = 221*/
JSI_EXTERN Jsi_RC Jsi_FuncObjToString(Jsi_Interp *interp, Jsi_Func *f, Jsi_DString *dStr, int flags); /*STUB = 222*/
JSI_EXTERN void* Jsi_UserObjDataFromVar(Jsi_Interp *interp, const char *var); /*STUB = 223*/
JSI_EXTERN const char* Jsi_KeyAdd(Jsi_Interp *interp, const char *str); /*STUB = 224*/
JSI_EXTERN const char* Jsi_KeyLookup(Jsi_Interp *interp, const char *str); /*STUB = 225*/
JSI_EXTERN bool Jsi_IsReserved(Jsi_Interp *interp, const char* str, bool sql); /*STUB = 415*/
JSI_EXTERN Jsi_RC Jsi_SqlObjBinds(Jsi_Interp* interp, Jsi_DString* zStr, const char *varName, bool addTypes, bool addDefaults, bool nullDefaults); /*STUB = 417*/
JSI_EXTERN Jsi_RC Jsi_DatetimeFormat(Jsi_Interp *interp, Jsi_Number date, const char *fmt, int isUtc, Jsi_DString *dStr);  /*STUB = 226*/
JSI_EXTERN Jsi_RC Jsi_DatetimeParse(Jsi_Interp *interp, const char *str, const char *fmt, int isUtc, Jsi_Number *datePtr, bool noMsg); /*STUB = 227*/
JSI_EXTERN Jsi_Number Jsi_DateTime(void); /*STUB = 228*/
#define JSI_DATE_JULIAN2UNIX(d)  (Jsi_Number)(((Jsi_Number)d - 2440587.5)*86400.0)
#define JSI_DATE_UNIX2JULIAN(d)  (Jsi_Number)((Jsi_Number)d/86400.0+2440587.5)

typedef enum { Jsi_CHash_SHA2_256, Jsi_CHash_SHA1, Jsi_CHash_MD5, Jsi_CHash_SHA3_224, 
    Jsi_CHash_SHA3_384, Jsi_CHash_SHA3_512, Jsi_CHash_SHA3_256 } Jsi_CryptoHashType;

JSI_EXTERN Jsi_RC Jsi_Encrypt(Jsi_Interp *interp, Jsi_DString *inout, const char *key, uint keyLen, bool decrypt); /*STUB = 229*/
JSI_EXTERN Jsi_RC Jsi_CryptoHash(char *outbuf, const char *str, int len, Jsi_CryptoHashType type, uint strength, bool noHex, int *sizPtr); /*STUB = 230*/
JSI_EXTERN Jsi_RC Jsi_Base64(const char *str, int len, Jsi_DString *buf, bool decode); /*STUB = 231*/
JSI_EXTERN int Jsi_HexStr(const uchar *data, int len, Jsi_DString *dStr, bool decode); /*STUB = 232*/
JSI_EXTERN uint32_t Jsi_Crc32(uint32_t crc, const void *ptr, size_t buf_len); /*STUB = 234*/
JSI_EXTERN Jsi_RC Jsi_FileRead(Jsi_Interp *interp, Jsi_Value *name, Jsi_DString *dStr); /*STUB = 408*/

JSI_EXTERN int Jsi_NumberIsInfinity(Jsi_Number a);  /*STUB = 235*/
JSI_EXTERN bool Jsi_NumberIsEqual(Jsi_Number n1, Jsi_Number n2);  /*STUB = 236*/
JSI_EXTERN bool Jsi_NumberIsFinite(Jsi_Number value);  /*STUB = 237*/
JSI_EXTERN bool Jsi_NumberIsInteger(Jsi_Number n);  /*STUB = 238*/
JSI_EXTERN bool Jsi_NumberIsNaN(Jsi_Number a);  /*STUB = 239*/
JSI_EXTERN bool Jsi_NumberIsNormal(Jsi_Number a);  /*STUB = 240*/
JSI_EXTERN bool Jsi_NumberIsSubnormal(Jsi_Number a);  /*STUB = 241*/
JSI_EXTERN bool Jsi_NumberIsWide(Jsi_Number n);  /*STUB = 242*/
JSI_EXTERN Jsi_Number Jsi_NumberInfinity(int i);  /*STUB = 243*/
JSI_EXTERN Jsi_Number Jsi_NumberNaN(void);  /*STUB = 244*/
JSI_EXTERN void Jsi_NumberDtoA(Jsi_Interp *interp, Jsi_Number value, char* buf, int bsiz, int prec);  /*STUB = 245*/
JSI_EXTERN void Jsi_NumberItoA10(Jsi_Wide value, char* buf, int bsiz);  /*STUB = 246*/
JSI_EXTERN void Jsi_NumberUtoA10(Jsi_UWide, char* buf, int bsiz);  /*STUB = 247*/

/* -- */

#define JSI_WORDKEY_CAST (void*)(uintptr_t)

struct Jsi_MapOpts;

typedef Jsi_RC (Jsi_HashDeleteProc)(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *data);
typedef Jsi_RC (Jsi_TreeDeleteProc)(Jsi_Interp *interp, Jsi_TreeEntry *hPtr, void *data);
typedef Jsi_RC (Jsi_MapDeleteProc)(Jsi_Interp *interp, Jsi_MapEntry *hPtr, void *data);
typedef Jsi_Value *(Jsi_MapFmtKeyProc)(Jsi_MapEntry* hPtr, struct Jsi_MapOpts *opts, int flags);
typedef Jsi_RC (Jsi_TreeWalkProc)(Jsi_Tree* treePtr, Jsi_TreeEntry* hPtr, void *data);
typedef int (Jsi_RBCompareProc)(Jsi_Tree *treePtr, const void *key1, const void *key2);

typedef struct Jsi_MapOpts {
    Jsi_Map_Type mapType; // Read-only
    Jsi_Key_Type keyType; // Read-only
    Jsi_Interp *interp;
    Jsi_Wide flags;
    void *user, *user2;
    Jsi_MapFmtKeyProc *fmtKeyProc;
    Jsi_RBCompareProc *compareTreeProc;
    union {
        Jsi_RC (*freeHashProc)(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *data);
        Jsi_RC (*freeTreeProc)(Jsi_Interp *interp, Jsi_TreeEntry *hPtr, void *data);
        Jsi_RC (*freeListProc)(Jsi_Interp *interp, Jsi_ListEntry *hPtr, void *data);
    };
    union {
        Jsi_RC (*lockHashProc) (Jsi_Hash *tablePtr, int lock);
        Jsi_RC (*lockTreeProc) (Jsi_Tree *tablePtr, int lock);
    };
} Jsi_MapOpts;

/* --HASH-- */
typedef struct Jsi_HashSearch {
    Jsi_Hash *tablePtr;
    unsigned long nextIndex; 
    Jsi_HashEntry *nextEntryPtr;
} Jsi_HashSearch;

JSI_EXTERN Jsi_Hash* Jsi_HashNew(Jsi_Interp *interp, uint keyType, Jsi_HashDeleteProc *freeProc); /*STUB = 248*/
JSI_EXTERN Jsi_RC Jsi_HashConf(Jsi_Hash *hashPtr, Jsi_MapOpts *opts, bool set); /*STUB = 249*/
JSI_EXTERN void Jsi_HashDelete(Jsi_Hash *hashPtr); /*STUB = 250*/
JSI_EXTERN void Jsi_HashClear(Jsi_Hash *hashPtr); /*STUB = 251*/
JSI_EXTERN Jsi_HashEntry* Jsi_HashSet(Jsi_Hash *hashPtr, const void *key, void *value); /*STUB = 252*/
JSI_EXTERN void* Jsi_HashGet(Jsi_Hash *hashPtr, const void *key, int flags); /*STUB = 253*/
JSI_EXTERN bool Jsi_HashUnset(Jsi_Hash *tbl, const void *key); /*STUB = 254*/
JSI_EXTERN void* Jsi_HashKeyGet(Jsi_HashEntry *h); /*STUB = 255*/
JSI_EXTERN Jsi_RC Jsi_HashKeysDump(Jsi_Interp *interp, Jsi_Hash *hashPtr, Jsi_Value **ret, int flags); /*STUB = 256*/
JSI_EXTERN void* Jsi_HashValueGet(Jsi_HashEntry *h); /*STUB = 257*/
JSI_EXTERN void Jsi_HashValueSet(Jsi_HashEntry *h, void *value); /*STUB = 258*/
JSI_EXTERN Jsi_HashEntry* Jsi_HashEntryFind (Jsi_Hash *hashPtr, const void *key); /*STUB = 259*/
JSI_EXTERN Jsi_HashEntry* Jsi_HashEntryNew (Jsi_Hash *hashPtr, const void *key, bool *isNew); /*STUB = 260*/
JSI_EXTERN int Jsi_HashEntryDelete (Jsi_HashEntry *entryPtr); /*STUB = 261*/
JSI_EXTERN Jsi_HashEntry* Jsi_HashSearchFirst (Jsi_Hash *hashPtr, Jsi_HashSearch *searchPtr); /*STUB = 262*/
JSI_EXTERN Jsi_HashEntry* Jsi_HashSearchNext (Jsi_HashSearch *searchPtr); /*STUB = 263*/
JSI_EXTERN uint Jsi_HashSize(Jsi_Hash *hashPtr); /*STUB = 264*/
/* -- */


/* --TREE-- */

typedef struct Jsi_TreeSearch {
    Jsi_Tree *treePtr;
    uint top, max, left, epoch; 
    int flags;
    Jsi_TreeEntry *staticPtrs[200], *current;
    Jsi_TreeEntry **Ptrs;
} Jsi_TreeSearch;

JSI_EXTERN Jsi_Tree* Jsi_TreeNew(Jsi_Interp *interp, uint keyType, Jsi_TreeDeleteProc *freeProc); /*STUB = 265*/
JSI_EXTERN Jsi_RC Jsi_TreeConf(Jsi_Tree *treePtr, Jsi_MapOpts *opts, bool set); /*STUB = 266*/
JSI_EXTERN void Jsi_TreeDelete(Jsi_Tree *treePtr); /*STUB = 267*/
JSI_EXTERN void Jsi_TreeClear(Jsi_Tree *treePtr); /*STUB = 268*/
JSI_EXTERN Jsi_TreeEntry* Jsi_TreeObjSetValue(Jsi_Obj* obj, const char *key, Jsi_Value *val, int isstrkey); /*STUB = 269*/
JSI_EXTERN Jsi_Value*     Jsi_TreeObjGetValue(Jsi_Obj* obj, const char *key, int isstrkey); /*STUB = 270*/
JSI_EXTERN void* Jsi_TreeValueGet(Jsi_TreeEntry *hPtr); /*STUB = 271*/
JSI_EXTERN void Jsi_TreeValueSet(Jsi_TreeEntry *hPtr, void *value); /*STUB = 272*/
JSI_EXTERN void* Jsi_TreeKeyGet(Jsi_TreeEntry *hPtr); /*STUB = 273*/
JSI_EXTERN Jsi_TreeEntry* Jsi_TreeEntryFind(Jsi_Tree *treePtr, const void *key); /*STUB = 274*/
JSI_EXTERN Jsi_TreeEntry* Jsi_TreeEntryNew(Jsi_Tree *treePtr, const void *key, bool *isNew); /*STUB = 275*/
JSI_EXTERN int Jsi_TreeEntryDelete(Jsi_TreeEntry *entryPtr); /*STUB = 276*/
JSI_EXTERN Jsi_TreeEntry* Jsi_TreeSearchFirst(Jsi_Tree *treePtr, Jsi_TreeSearch *searchPtr, int flags, const void *startKey); /*STUB = 277*/
JSI_EXTERN Jsi_TreeEntry* Jsi_TreeSearchNext(Jsi_TreeSearch *searchPtr); /*STUB = 278*/
JSI_EXTERN void Jsi_TreeSearchDone(Jsi_TreeSearch *searchPtr); /*STUB = 279*/
JSI_EXTERN int Jsi_TreeWalk(Jsi_Tree* treePtr, Jsi_TreeWalkProc* callback, void *data, int flags); /*STUB = 280*/
JSI_EXTERN Jsi_TreeEntry* Jsi_TreeSet(Jsi_Tree *treePtr, const void *key, void *value); /*STUB = 281*/
JSI_EXTERN void* Jsi_TreeGet(Jsi_Tree *treePtr, void *key, int flags); /*STUB = 282*/
JSI_EXTERN bool Jsi_TreeUnset(Jsi_Tree *treePtr, void *key); /*STUB = 283*/
JSI_EXTERN uint Jsi_TreeSize(Jsi_Tree *treePtr); /*STUB = 284*/ 
JSI_EXTERN Jsi_Tree* Jsi_TreeFromValue(Jsi_Interp *interp, Jsi_Value *v); /*STUB = 285*/
JSI_EXTERN Jsi_RC Jsi_TreeKeysDump(Jsi_Interp *interp, Jsi_Tree *hashPtr, Jsi_Value **ret, int flags); /*STUB = 286*/
/* -- */


/* --LIST-- */
typedef struct Jsi_List {
    uint sig;
    int numEntries;
    Jsi_ListEntry *head;
    Jsi_ListEntry *tail;
    Jsi_MapOpts opts;
} Jsi_List;

typedef struct Jsi_ListEntry {
    uint sig;
    Jsi_Map_Type typ;    
    struct Jsi_ListEntry *next;
    struct Jsi_ListEntry *prev;
    Jsi_List *list;
    void *value;
} Jsi_ListEntry;

typedef struct Jsi_ListSearch {
    int flags;
    Jsi_List *tablePtr;
    unsigned long nextIndex; 
    Jsi_ListEntry *nextEntryPtr;
} Jsi_ListSearch;

JSI_EXTERN Jsi_List* Jsi_ListNew(Jsi_Interp *interp, Jsi_Wide flags, Jsi_HashDeleteProc *freeProc); /*STUB = 287*/
JSI_EXTERN Jsi_RC Jsi_ListConf(Jsi_List *list, Jsi_MapOpts *opts, bool set); /*STUB = 288*/
JSI_EXTERN void Jsi_ListDelete(Jsi_List *list); /*STUB = 289*/
JSI_EXTERN void Jsi_ListClear(Jsi_List *list); /*STUB = 290*/
//#define Jsi_ListSet(l, before, value) Jsi_ListPush(l, before, Jsi_ListEntryNew(l, value))
//#define Jsi_ListGet(l, le) (le)->value 
//#define Jsi_ListKeyGet(le) (le)
//#define Jsi_ListKeysDump(interp, list, ret, flags) JSI_ERROR
JSI_EXTERN void* Jsi_ListValueGet(Jsi_ListEntry *list); /*STUB = 291*/
JSI_EXTERN void Jsi_ListValueSet(Jsi_ListEntry *list, const void *value); /*STUB = 292*/
//#define Jsi_ListEntryFind(l, le) (le)
JSI_EXTERN Jsi_ListEntry* Jsi_ListEntryNew(Jsi_List *list, const void *value, Jsi_ListEntry *before); /*STUB = 293*/
JSI_EXTERN int Jsi_ListEntryDelete(Jsi_ListEntry *entry); /*STUB = 294*/
JSI_EXTERN Jsi_ListEntry* Jsi_ListSearchFirst (Jsi_List *list, Jsi_ListSearch *search, int flags); /*STUB = 295*/
JSI_EXTERN Jsi_ListEntry* Jsi_ListSearchNext (Jsi_ListSearch *search); /*STUB = 296*/
JSI_EXTERN uint Jsi_ListSize(Jsi_List *list); /*STUB = 297*/
/* end of hash-compat functions. */

JSI_EXTERN Jsi_ListEntry* Jsi_ListPush(Jsi_List *list, Jsi_ListEntry *entry, Jsi_ListEntry *before); /*STUB = 298*/
JSI_EXTERN Jsi_ListEntry* Jsi_ListPop(Jsi_List *list, Jsi_ListEntry *entry); /*STUB = 299*/
#define Jsi_ListPushFront(list,entry)   Jsi_ListPush(list, entry, list->head)
#define Jsi_ListPushBack(list,entry)    Jsi_ListPush(list, entry, NULL)
#define Jsi_ListPushFrontNew(list,v)    Jsi_ListEntryNew(list, v, list->head)
#define Jsi_ListPushBackNew(list,v)     Jsi_ListEntryNew(list, v, NULL)
#define Jsi_ListPopFront(list)          Jsi_ListPop(list, list->head)
#define Jsi_ListPopBack(list)           Jsi_ListPop(list, list->tail)
#define Jsi_ListEntryNext(entry)        (entry)->next 
#define Jsi_ListEntryPrev(entry)        (entry)->prev
#define Jsi_ListGetFront(list)          (list)->head
#define Jsi_ListGetBack(list)           (list)->tail

/* -- */


/* --STACK-- */
typedef struct {
    int len;
    int maxlen;
    void **vector;
} Jsi_Stack;

JSI_EXTERN Jsi_Stack* Jsi_StackNew(void); /*STUB = 300*/
JSI_EXTERN void Jsi_StackFree(Jsi_Stack *stack); /*STUB = 301*/
JSI_EXTERN int Jsi_StackSize(Jsi_Stack *stack); /*STUB = 302*/
JSI_EXTERN void Jsi_StackPush(Jsi_Stack *stack, void *element); /*STUB = 303*/
JSI_EXTERN void* Jsi_StackPop(Jsi_Stack *stack); /*STUB = 304*/
JSI_EXTERN void* Jsi_StackPeek(Jsi_Stack *stack); /*STUB = 305*/
JSI_EXTERN void* Jsi_StackUnshift(Jsi_Stack *stack); /*STUB = 306*/
JSI_EXTERN void* Jsi_StackHead(Jsi_Stack *stack); /*STUB = 307*/
JSI_EXTERN void Jsi_StackFreeElements(Jsi_Interp *interp, Jsi_Stack *stack, Jsi_DeleteProc *freeFunc); /*STUB = 308*/
/* -- */

/* --MAP-- */
typedef struct Jsi_MapSearch {
    Jsi_Map_Type typ;
    union {
        Jsi_TreeSearch tree;
        Jsi_HashSearch hash;
        Jsi_ListSearch list;
    } v;
} Jsi_MapSearch;

JSI_EXTERN Jsi_Map* Jsi_MapNew(Jsi_Interp *interp, Jsi_Map_Type mapType, Jsi_Key_Type keyType, Jsi_MapDeleteProc *freeProc); /*STUB = 309*/
JSI_EXTERN Jsi_RC Jsi_MapConf(Jsi_Map *mapPtr, Jsi_MapOpts *opts, bool set); /*STUB = 310*/
JSI_EXTERN void Jsi_MapDelete (Jsi_Map *mapPtr); /*STUB = 311*/
JSI_EXTERN void Jsi_MapClear (Jsi_Map *mapPtr); /*STUB = 312*/
JSI_EXTERN Jsi_MapEntry* Jsi_MapSet(Jsi_Map *mapPtr, const void *key, const void *value); /*STUB = 313*/
JSI_EXTERN void* Jsi_MapGet(Jsi_Map *mapPtr, const void *key, int flags); /*STUB = 314*/
JSI_EXTERN void* Jsi_MapKeyGet(Jsi_MapEntry *h, int flags); /*STUB = 315*/
JSI_EXTERN Jsi_RC Jsi_MapKeysDump(Jsi_Interp *interp, Jsi_Map *mapPtr, Jsi_Value **ret, int flags); /*STUB = 316*/
JSI_EXTERN void* Jsi_MapValueGet(Jsi_MapEntry *h); /*STUB = 317*/
JSI_EXTERN void Jsi_MapValueSet(Jsi_MapEntry *h, const void *value); /*STUB = 318*/
JSI_EXTERN Jsi_MapEntry* Jsi_MapEntryFind (Jsi_Map *mapPtr, const void *key); /*STUB = 319*/
JSI_EXTERN Jsi_MapEntry* Jsi_MapEntryNew (Jsi_Map *mapPtr, const void *key, bool *isNew); /*STUB = 320*/
JSI_EXTERN int Jsi_MapEntryDelete (Jsi_MapEntry *entryPtr); /*STUB = 321*/
JSI_EXTERN Jsi_MapEntry* Jsi_MapSearchFirst (Jsi_Map *mapPtr, Jsi_MapSearch *searchPtr, int flags); /*STUB = 322*/
JSI_EXTERN Jsi_MapEntry* Jsi_MapSearchNext (Jsi_MapSearch *srchPtr); /*STUB = 323*/
JSI_EXTERN void Jsi_MapSearchDone (Jsi_MapSearch *searchPtr);  /*STUB = 324*/
JSI_EXTERN uint Jsi_MapSize(Jsi_Map *mapPtr); /*STUB = 325*/


// Define typed wrappers for 5 main Map functions: Set, Get, KeyGet, EntryFind, EntryNew
#define JSI_MAP_EXTN(Prefix, keyType, valType) \
JSI_EXTERN Jsi_MapEntry *Prefix ## _Set(Jsi_Map *mapPtr, keyType key, valType value); \
JSI_EXTERN valType Prefix ## _Get(Jsi_Map *mapPtr, keyType key); \
JSI_EXTERN keyType Prefix ## _KeyGet(Jsi_MapEntry *h); \
JSI_EXTERN Jsi_MapEntry* Prefix ## _EntryFind (Jsi_Map *mapPtr, keyType key); \
JSI_EXTERN Jsi_MapEntry* Prefix ## _EntryNew (Jsi_Map *mapPtr, keyType key, int *isNew);

#define JSI_MAP_DEFN(Prefix, keyType, valType) \
Jsi_MapEntry *Prefix ## _Set(Jsi_Map *mapPtr, keyType key, valType value) { return Jsi_MapSet(mapPtr, (void*)key, (void*)value); } \
valType Prefix ## _Get(Jsi_Map *mapPtr, keyType key) { return (valType)Jsi_MapGet(mapPtr, (void*)key); } \
keyType Prefix ## _KeyGet(Jsi_MapEntry *h) { return (keyType)Jsi_MapKeyGet(h); } \
Jsi_MapEntry* Prefix ## _EntryFind (Jsi_Map *mapPtr, keyType key) { return  Jsi_MapEntryFind(mapPtr, (void*)key); } \
Jsi_MapEntry* Prefix ## _EntryNew (Jsi_Map *mapPtr, keyType key, int *isNew) { return  Jsi_MapEntryNew(mapPtr, (void*)key, isNew); }
   
/* -- */


/* --OPTIONS-- */
typedef Jsi_RC (Jsi_OptionParseProc) (
    Jsi_Interp *interp, Jsi_OptionSpec *spec, Jsi_Value *value, const char *str, void *record, Jsi_Wide flags);
typedef Jsi_RC (Jsi_OptionFormatProc) (
    Jsi_Interp *interp, Jsi_OptionSpec *spec, Jsi_Value **retValue, Jsi_DString *retStr, void *record, Jsi_Wide flags);
typedef Jsi_RC (Jsi_OptionFormatStringProc) (
    Jsi_Interp *interp, Jsi_OptionSpec *spec, Jsi_DString **retValue, void *record);
typedef void (Jsi_OptionFreeProc) (Jsi_Interp *interp, Jsi_OptionSpec *spec, void *ptr);

typedef Jsi_RC (Jsi_OptionBitOp)(Jsi_Interp *interp, Jsi_OptionSpec *spec, void *data, Jsi_Wide *s, int isSet);

typedef struct {
    const char *name;
    Jsi_OptionParseProc *parseProc;
    Jsi_OptionFormatProc *formatProc;
    Jsi_OptionFreeProc *freeProc;
    const char *help;
    const char *info;
    void* data;
} Jsi_OptionCustom;

typedef enum {
    JSI_OPTION_BOOL=1,
    JSI_OPTION_INT8,  JSI_OPTION_INT16,  JSI_OPTION_INT32,  JSI_OPTION_INT64,
    JSI_OPTION_UINT8, JSI_OPTION_UINT16, JSI_OPTION_UINT32, JSI_OPTION_UINT64,
    JSI_OPTION_FLOAT,
    JSI_OPTION_DOUBLE,    // Same as NUMBER when !JSI_USE_LONG_DOUBLE.
    JSI_OPTION_LDOUBLE,   // A long double
    JSI_OPTION_STRBUF,    // Fixed size string buffer.
    JSI_OPTION_TIME_W,    // Jsi_Wide: milliseconds since Jan 1, 1970.
    JSI_OPTION_TIME_D,    // double: milliseconds since Jan 1, 1970.
    // Non-portable fields start here
    JSI_OPTION_TIME_T,    // time_t: seconds since Jan 1, 1970. 
    JSI_OPTION_SIZE_T,
    JSI_OPTION_SSIZE_T,
    JSI_OPTION_INTPTR_T,  // Int big enough to store a pointer.
    JSI_OPTION_UINTPTR_T, 
    JSI_OPTION_NUMBER,    // Same as DOUBLE when !JSI_USE_LONG_DOUBLE.
    JSI_OPTION_INT, JSI_OPTION_UINT,
    JSI_OPTION_LONG, JSI_OPTION_ULONG, JSI_OPTION_SHORT, JSI_OPTION_USHORT,
    JSI_OPTION_STRING, JSI_OPTION_DSTRING, JSI_OPTION_STRKEY,
    JSI_OPTION_VALUE, JSI_OPTION_VAR, JSI_OPTION_OBJ, JSI_OPTION_ARRAY, JSI_OPTION_REGEXP,
    JSI_OPTION_FUNC,      // Note: .data can contain string args to check
    JSI_OPTION_USEROBJ,   // Note: .data can contain string obj name to check
    JSI_OPTION_CUSTOM,    // Note: set .custom, .data, etc.
    JSI_OPTION_END
} Jsi_OptionId;

typedef const char* Jsi_Strkey;
#ifdef __cplusplus
typedef void* Jsi_Strbuf;
#else
typedef char Jsi_Strbuf[];
#endif

typedef union {
    bool           BOOL;
    int8_t         INT8;
    int16_t        INT16;
    int32_t        INT32;
    int64_t        INT64;
    uint8_t        UINT8;
    uint16_t       UINT16;
    uint32_t       UINT32;
    uint64_t       UINT64;
    float          FLOAT;
    double         DOUBLE;
    ldouble        LDOUBLE;
    Jsi_Number     NUMBER;
    char*          STRBUF;
    time_d         TIME_D;
    time_w         TIME_W;
    time_t         TIME_T;
    size_t         SIZE_T;
    ssize_t        SSIZE_T;
    intptr_t       INTPTR_T;
    uintptr_t      UINTPTR_T;
    int            INT;
    uint           UINT;
    long           LONG;
    ulong          ULONG;
    short          SHORT;
    ushort         USHORT;
    Jsi_DString    DSTRING;
    const char*    STRKEY;
    Jsi_Value*     STRING;
    Jsi_Value*     VALUE;
    Jsi_Value*     VAR;
    Jsi_Value*     OBJ;
    Jsi_Value*     ARRAY;
    Jsi_Value*     REGEXP;
    Jsi_Value*     FUNC;
    Jsi_Value*     USEROBJ;
    void*          CUSTOM;
    Jsi_csgset*    OPT_BITS;
    struct Jsi_OptionSpec* OPT_CARRAY;
} Jsi_OptionValue;

typedef union { /* Field used at compile-time by JSI_OPT() to provide type checking for the var */
    bool           *BOOL;
    int8_t         *INT8;
    int16_t        *INT16;
    int32_t        *INT32;
    int64_t        *INT64;
    uint8_t        *UINT8;
    uint16_t       *UINT16;
    uint32_t       *UINT32;
    uint64_t       *UINT64;
    float          *FLOAT;
    double         *DOUBLE;
    ldouble        *LDOUBLE;
    Jsi_Number     *NUMBER;
#ifdef __cplusplus
    Jsi_Strbuf      STRBUF;
#else
    Jsi_Strbuf      *STRBUF;
#endif
    time_t         *TIME_T;
    time_w         *TIME_W;
    time_d         *TIME_D;
    size_t         *SIZE_T;
    ssize_t        *SSIZE_T;
    intptr_t       *INTPTR_T;
    uintptr_t      *UINTPTR_T;
    int            *INT;
    uint           *UINT;
    long           *LONG;
    ulong          *ULONG;
    short          *SHORT;
    ushort         *USHORT;
    Jsi_DString    *DSTRING;
    const char*    *STRKEY;
    Jsi_Value*     *VALUE;
    Jsi_Value*     *STRING;
    Jsi_Value*     *VAR;
    Jsi_Value*     *OBJ;
    Jsi_Value*     *ARRAY;
    Jsi_Value*     *REGEXP;
    Jsi_Value*     *FUNC;
    Jsi_Value*     *USEROBJ;
    void           *CUSTOM;
    Jsi_csgset     *OPT_BITS;
    struct Jsi_OptionSpec *OPT_CARRAY;
} Jsi_OptionInitVal;

typedef struct {
    Jsi_Sig sig;
    Jsi_OptionId id;
    const char *idName, *cName;
    int size;
    const char *fmt, *xfmt, *sfmt, *help;
    Jsi_OptionInitVal init;
    Jsi_Wide flags;
    Jsi_Wide user;
    const char *userData;       /* User data. */ \
    uchar *extData;             /* Extension data. */
    uchar *extra;
    Jsi_HashEntry *hPtr;
} Jsi_OptionTypedef;

struct Jsi_OptionSpec {
    Jsi_Sig sig;                /* Signature field. */
    Jsi_OptionId id;
    const char *name;           /* The field name. */
    uint offset;                /* Jsi_Offset of field. */
    uint size;                  /* The sizeof() of field. */
    Jsi_OptionInitVal init;     /* Initialization value */
    const char *help;           /* A short one-line help string, without newlines. */
    Jsi_Wide flags;             /* Lower 32 bits: the JSI_OPTS_* flags below. Upper 32 for custom/other. */
    Jsi_OptionCustom *custom;   /* Custom handler. */
    void *data;                 /* User data for custom options: eg. the bit for BOOLBIT. */
    const char *info;           /* Longer command description. Use JSI_DETAIL macro to allow compile-out.*/
    const char *tname;          /* Type name for field or external name used by the DB interface. */
    Jsi_Wide value;             /* Value field. */
    uint32_t bits;              /* Size of bitfield */
    uint32_t boffset;           /* Bit offset of field (or struct) */
    uint32_t idx;               /* Index (of field) */
    uint32_t ssig;              /* Signature (for struct) */
    uint32_t crc;               /* Crc (for struct) */
    uint32_t arrSize;           /* Size of array */
    const char *userData;       /* User data. */
    uchar *extData;             /* Extension data. */
    uchar *extra;               /* Extra pointer (currently unused). */
    const Jsi_OptionTypedef *type;
};

/* JSI_OPT is a macro used for option definitions, eg:
 * 
 *      typedef struct { int debug; int bool; } MyStruct;
 * 
 *      Jsi_OptionSpec MyOptions[] = {
 *          JSI_OPT(BOOL,  MyStruct,  debug ),
 *          JSI_OPT(INT,   MyStruct,  max,   .help="Max value"),
 *          JSI_OPT_END(   MyStruct, .help="My first struct" )
 *      }
*/

#define JSI_OPT_(s, typ, strct, nam, ...) \
    { .sig=s, .id=JSI_OPTION_##typ, .name=#nam, .offset=Jsi_Offset(strct, nam), .size=sizeof(((strct *) 0)->nam), \
      .init={.typ=(&((strct *) 0)->nam)}, ##__VA_ARGS__ }

#define JSI_OPT_END_(s, strct, ...) { .sig=s, .id=JSI_OPTION_END, .name=#strct, .offset=__LINE__, .size=sizeof(strct), \
      .init={.CUSTOM=(void*)__FILE__}, ##__VA_ARGS__}

#define JSI_OPT_BITS_(s, strct, nam, hlp, flgs, bsget, fidx, tnam, bdata) \
    { .sig=s, .id=JSI_OPTION_CUSTOM, .name=#nam, .offset=0, .size=0, \
        .init={.OPT_BITS=&bsget}, .help=hlp, .flags=flgs, .custom=Jsi_Opt_SwitchBitfield, .data=bdata,\
        .info=0, .tname=#nam, .value=0, .bits=0, .boffset=0, .idx=fidx }

#define JSI_OPT_CARRAY_(s, strct, nam, hlp, flgs, aropt, asiz, tnam, sinit) \
    { .sig=s, .id=JSI_OPTION_CUSTOM, .name=#nam, .offset=Jsi_Offset(strct, nam), .size=sizeof(((strct *) 0)->nam), \
        .init={.OPT_CARRAY=aropt}, .help=hlp, .flags=flgs, .custom=Jsi_Opt_SwitchCArray, .data=0,\
        .info=0, .tname=tnam, .value=0, .bits=0, .boffset=0, .idx=0, .ssig=0, .crc=0, .arrSize=asiz, .extData=sinit, .extra=0 }

#define JSI_OPT_CARRAY_ITEM_(s, typ, strct, nam, ...) \
    { .sig=s, .id=JSI_OPTION_##typ, .name=#nam, .offset=0, .size=sizeof(((strct *) 0)->nam), \
      .init={.typ=(&((strct *) 0)->nam[0])}, ##__VA_ARGS__ }

#define JSI_OPT(typ, strct, nam, ...) JSI_OPT_(JSI_SIG_OPTS, typ, strct, nam, ##__VA_ARGS__) 
#define JSI_OPT_END(strct, ...) JSI_OPT_END_(JSI_SIG_OPTS, strct, ##__VA_ARGS__)
#define JSI_OPT_BITS(strct, nam, hlp, flgs, bsget, fidx, tnam, bdata) JSI_OPT_BITS_(JSI_SIG_OPTS, strct, nam, hlp, flgs, bsget, fidx, tnam, bdata)
#define JSI_OPT_CARRAY(strct, nam, hlp, flgs, aropt, asiz, tnam, sinit) JSI_OPT_CARRAY_(JSI_SIG_OPTS, strct, nam, hlp, flgs, aropt, asiz, tnam, sinit)
#define JSI_OPT_CARRAY_ITEM(typ, strct, nam, ...) JSI_OPT_CARRAY_ITEM_(JSI_SIG_OPTS, typ, strct, nam, ##__VA_ARGS__)

#define JSI_OPT_END_IDX(opt) ((sizeof(opt)/sizeof(opt[0]))-1)

/* builtin handler for Custom. */
#define Jsi_Opt_SwitchEnum          (Jsi_OptionCustom*)0x1 /* An Enum: choices are in .data=stringlist */
#define Jsi_Opt_SwitchBitset        (Jsi_OptionCustom*)0x2 /* Bits in an int: choices are in .data=stringlist */
#define Jsi_Opt_SwitchSuboption     (Jsi_OptionCustom*)0x3 /* Sub-structs: subspec is in .data={...} */
#define Jsi_Opt_SwitchBitfield      (Jsi_OptionCustom*)0x4 /* Struct bitfields: used by "jsish -c" */
#define Jsi_Opt_SwitchValueVerify   (Jsi_OptionCustom*)0x5 /* Callback to verify Jsi_Value* correctness in .data=func. */
#define Jsi_Opt_SwitchCArray        (Jsi_OptionCustom*)0x6 /* C Array described in .data=type. */
#define Jsi_Opt_SwitchNull          (Jsi_OptionCustom*)0x7 /* Set is ignored, and get returns null */
#define Jsi_Opt_SwitchParentFunc    (Jsi_OptionCustom*)0x8 /* Name of a func in parent. Sig string is in .data*/

enum {
    /* Jsi_OptionsProcess() flags */
    JSI_OPTS_PREFIX         =   (1<<27), /* Allow matching unique prefix of object members. */
    JSI_OPTS_IS_UPDATE      =   (1<<28), /* This is an update/conf (do not reset the specified flags) */
    JSI_OPTS_IGNORE_EXTRA   =   (1<<29), /* Ignore extra members not found in spec. */
    JSI_OPTS_FORCE_STRICT   =   (1<<30), /* Override Interp->compat to disable JSI_OPTS_IGNORE_EXTRA. */
    JSI_OPTS_VERBOSE        =   (1<<31), /* Dump verbose options */
    JSI_OPTS_INCR           =   (1<<7),  /* Options is an increment. */

    /* Jsi_OptionSpec flags. */
    JSI_OPT_IS_SPECIFIED    =   (1<<0),   /* User set the option. */
    JSI_OPT_INIT_ONLY       =   (1<<1),   /* Allow set only at init, disallowing update/conf. */
    JSI_OPT_READ_ONLY       =   (1<<2),   /* Value can not be set. */
    JSI_OPT_NO_DUPVALUE     =   (1<<3),   /* Values are not to be duped. */
    JSI_OPT_NO_CLEAR        =   (1<<4),   /* Values are not to be cleared: watch for memory leaks */
    JSI_OPT_REQUIRED        =   (1<<5),  /* Field must be specified (if not IS_UPDATE). */
    JSI_OPT_PASS2           =   (1<<6),   /* Options to be processed only on pass2. */
    JSI_OPT_DB_DIRTY        =   (1<<8),   /* Used to limit DB updates. */
    JSI_OPT_DB_IGNORE       =   (1<<9),   /* Field is not to be used for DB. */
    JSI_OPT_DB_ROWID        =   (1<<10),  /* Field used by DB to store rowid. */
    JSI_OPT_CUST_NOCASE     =   (1<<11),  /* Ignore case (eg. for ENUM and BITSET). */
    JSI_OPT_FORCE_INT       =   (1<<12),  /* Force int instead of text for enum/bitset. */
    JSI_OPT_BITSET_ENUM     =   (1<<13),  /* Mark field as a bitset/enum map custom field. */
    JSI_OPT_TIME_DATEONLY   =   (1<<14),  /* Time field is date only. */
    JSI_OPT_TIME_TIMEONLY   =   (1<<15),  /* Time field is time only. */
    JSI_OPT_IS_BITS         =   (1<<16),  /* Is a C bit-field. */
    JSI_OPT_FMT_STRING      =   (1<<17),  /* Format value (eg. time) as string. */
    JSI_OPT_FMT_NUMBER      =   (1<<18),  /* Format value (eg. enum) as number. */
    JSI_OPT_FMT_HEX         =   (1<<19),  /* Format number in hex. */
    JSI_OPT_STRICT          =   (1<<20),  /* Strict mode. */
    JSI_OPT_LOCKSAFE        =   (1<<21),  /* Field may not be configured when isSafe. */
    JSI_OPT_COERCE          =   (1<<22),  /* Coerce input value to required type. */
    JSI_OPT_NO_SIG          =   (1<<23),  /* No signature. */
    JSI_OPT_ENUM_SPEC       =   (1<<24),  /* Enum has spec rather than a list of strings. */
    JSI_OPT_ENUM_UNSIGNED   =   (1<<25),  /* Enum value is unsigned. */
    JSI_OPT_ENUM_EXACT      =   (1<<26),  /* Enum must be an exact match. */
    JSI_OPTIONS_USER_FIRSTBIT  =   48,    /* First bit of user flags: the lower 48 bits are internal. */
};

JSI_EXTERN const Jsi_OptionTypedef* Jsi_OptionTypeInfo(Jsi_OptionId typ); /*STUB = 326*/
JSI_EXTERN Jsi_OptionTypedef* Jsi_TypeLookup(Jsi_Interp* interp, const char *typ); /*STUB = 327*/
JSI_EXTERN int Jsi_OptionsProcess(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, Jsi_Value *value, Jsi_Wide flags); /*STUB = 328*/
JSI_EXTERN int Jsi_OptionsProcessJSON(Jsi_Interp *interp, Jsi_OptionSpec *opts, void *data, const char *json, Jsi_Wide flags); /*STUB = 329*/
JSI_EXTERN Jsi_RC Jsi_OptionsConf(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, Jsi_Value *value, Jsi_Value **ret, Jsi_Wide flags); /*STUB = 330*/
JSI_EXTERN void Jsi_OptionsFree(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, Jsi_Wide flags); /*STUB = 331*/
JSI_EXTERN Jsi_RC Jsi_OptionsGet(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, const char *option, Jsi_Value** valuePtr, Jsi_Wide flags); /*STUB = 332*/
JSI_EXTERN Jsi_RC Jsi_OptionsSet(Jsi_Interp *interp, Jsi_OptionSpec *specs, void* data, const char *option, Jsi_Value *valuePtr, Jsi_Wide flags); /*STUB = 333*/
JSI_EXTERN Jsi_RC Jsi_OptionsDump(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, Jsi_Value** ret, Jsi_Wide flags); /*STUB = 334*/
JSI_EXTERN int Jsi_OptionsChanged(Jsi_Interp *interp, Jsi_OptionSpec *specs, const char *pattern, ...) /*STUB = 335*/ __attribute__((sentinel));
JSI_EXTERN bool Jsi_OptionsValid(Jsi_Interp *interp, Jsi_OptionSpec* spec);  /*STUB = 336*/
JSI_EXTERN const char* Jsi_OptionsData(Jsi_Interp *interp, Jsi_OptionSpec *specs, Jsi_DString *dStr, bool schema);
JSI_EXTERN Jsi_OptionSpec* Jsi_OptionsFind(Jsi_Interp *interp, Jsi_OptionSpec *specs, const char *name, Jsi_Wide flags); /*STUB = 337*/
JSI_EXTERN Jsi_Value* Jsi_OptionsCustomPrint(void* clientData, Jsi_Interp *interp, const char *optionName, void *data, int offset); /*STUB = 338*/
JSI_EXTERN Jsi_OptionCustom* Jsi_OptionCustomBuiltin(Jsi_OptionCustom* cust); /*STUB = 339*/
/* Create a duplicate of static specs.   Use this for threaded access to Jsi_OptionsChanged(). */
JSI_EXTERN Jsi_OptionSpec* Jsi_OptionsDup(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs); /*STUB = 340*/
JSI_EXTERN const Jsi_OptionSpec* Jsi_OptionSpecsCached(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs);  /*STUB = 341*/
/* -- */


/* --THREADS/MUTEX-- */
JSI_EXTERN Jsi_RC Jsi_MutexLock(Jsi_Interp *interp, Jsi_Mutex *mtx); /*STUB = 342*/
JSI_EXTERN void Jsi_MutexUnlock(Jsi_Interp *interp, Jsi_Mutex *mtx); /*STUB = 343*/
JSI_EXTERN void Jsi_MutexDelete(Jsi_Interp *interp, Jsi_Mutex *mtx); /*STUB = 344*/
JSI_EXTERN Jsi_Mutex* Jsi_MutexNew(Jsi_Interp *interp, int timeout, int flags); /*STUB = 345*/
JSI_EXTERN void* Jsi_CurrentThread(void); /*STUB = 346*/
JSI_EXTERN void* Jsi_InterpThread(Jsi_Interp *interp); /*STUB = 347*/
/* -- */


/* --LOGGING-- */
#define Jsi_LogBug(fmt,...) Jsi_LogMsg(interp, JSI_LOG_BUG, fmt, ##__VA_ARGS__)
#define Jsi_LogError(fmt,...) Jsi_LogMsg(interp, JSI_LOG_ERROR, fmt, ##__VA_ARGS__)
#define Jsi_LogParse(fmt,...) Jsi_LogMsg(interp, JSI_LOG_PARSE, fmt, ##__VA_ARGS__)
#define Jsi_LogWarn(fmt,...) Jsi_LogMsg(interp, JSI_LOG_WARN, fmt, ##__VA_ARGS__)
#define Jsi_LogInfo(fmt,...) Jsi_LogMsg(interp, JSI_LOG_INFO, fmt, ##__VA_ARGS__)
#define Jsi_LogDebug(fmt,...) Jsi_LogMsg(interp, JSI_LOG_DEBUG, fmt, ##__VA_ARGS__)
#define Jsi_LogTrace(fmt,...) Jsi_LogMsg(interp, JSI_LOG_TRACE, fmt, ##__VA_ARGS__)
#define Jsi_LogTest(fmt,...) Jsi_LogMsg(interp, JSI_LOG_TEST, fmt, ##__VA_ARGS__)

JSI_EXTERN Jsi_RC Jsi_LogMsg(Jsi_Interp *interp, uint level, const char *format,...)  /*STUB = 348*/ __attribute__((format (printf,3,4)));


/* --EVENTS-- */
typedef struct {
    Jsi_Sig sig;
    uint id;
    int evType;                 /* Is signal handler. */
    int sigNum;
    int once;                   /* Execute once */
    long initialms;             /* initial relative timer value */
    long when_sec;              /* seconds */
    long when_ms;               /* milliseconds */
    bool busy;                  /* In event callback. */
    uint count;                 /* Times executed */
    Jsi_HashEntry *hPtr;
    Jsi_Value *funcVal;         /* JS Function to call. */
    Jsi_EventHandlerProc *handler;  /* C-function handler. */
    void *data;
} Jsi_Event;

JSI_EXTERN Jsi_Event* Jsi_EventNew(Jsi_Interp *interp, Jsi_EventHandlerProc *callback, void* data); /*STUB = 349*/
JSI_EXTERN void Jsi_EventFree(Jsi_Interp *interp, Jsi_Event* event); /*STUB = 350*/
JSI_EXTERN int Jsi_EventProcess(Jsi_Interp *interp, int maxEvents); /*STUB = 351*/
/* -- */


/* --JSON-- */
#define JSI_JSON_DECLARE(p, tokens, maxsz) \
    Jsi_JsonParser p = {0}; \
    Jsi_JsonTok tokens[maxsz>0?maxsz:JSI_JSON_STATIC_DEFAULT]; \
    Jsi_JsonInit(&p, tokens, maxsz>0?maxsz:JSI_JSON_STATIC_DEFAULT)

typedef enum {
    JSI_JTYPE_PRIMITIVE = 0,
    JSI_JTYPE_OBJECT = 1,
    JSI_JTYPE_ARRAY = 2,
    JSI_JTYPE_STRING = 3,
    JSI_JTYPE_INVALID=-1
} Jsi_JsonTypeEnum;

typedef enum {
    JSI_JSON_ERR_NOMEM = -1,
    JSI_JSON_ERR_INVAL = -2,
    JSI_JSON_ERR_PART = -3,
    JSI_JSON_ERR_NONE = 0
} Jsi_JsonErrEnum;

typedef struct {
    Jsi_JsonTypeEnum type;
    int start;
    int end;
    uint size;
    int parent;
} Jsi_JsonTok;

typedef struct {
    uint pos;           /* offset in the JSON string */
    uint toknext;       /* next token to allocate */
    int toksuper;       /* superior token node, e.g parent object or array */
    Jsi_JsonTok *tokens, *static_tokens;
    uint num_tokens;
    int no_malloc;      /* Disable parser dynamic growth tokens array. */
    bool strict;/* Strict parsing. */
    Jsi_Wide flags;
    const char *errStr;
    void *reserved[4];     /* Reserved for future */
} Jsi_JsonParser;


JSI_EXTERN void Jsi_JsonInit(Jsi_JsonParser *parser, Jsi_JsonTok *static_tokens, uint num_tokens); /*STUB = 352*/
JSI_EXTERN void Jsi_JsonReset(Jsi_JsonParser *parser); /*STUB = 353*/
JSI_EXTERN void Jsi_JsonFree(Jsi_JsonParser *parser); /*STUB = 354*/
JSI_EXTERN Jsi_JsonErrEnum Jsi_JsonParse(Jsi_JsonParser *parser, const char *js); /*STUB = 355*/
JSI_EXTERN Jsi_JsonTok* Jsi_JsonGetToken(Jsi_JsonParser *parser, uint index); /*STUB = 356*/
JSI_EXTERN Jsi_JsonTypeEnum Jsi_JsonGetType(Jsi_JsonParser *parser, uint index); /*STUB = 357*/
JSI_EXTERN int Jsi_JsonTokLen(Jsi_JsonParser *parser, uint index); /*STUB = 358*/
JSI_EXTERN const char* Jsi_JsonGetTokstr(Jsi_JsonParser *parser, const char *js, uint index, uint *len); /*STUB = 359*/
JSI_EXTERN const char* Jsi_JsonGetTypename(int type); /*STUB = 360*/
JSI_EXTERN const char* Jsi_JsonGetErrname(int code); /*STUB = 361*/
JSI_EXTERN void Jsi_JsonDump(Jsi_JsonParser *parser, const char *js); /*STUB = 362*/
/* -- */


/* --VFS-- */
struct Jsi_LoadHandle; struct Jsi_LoadHandle;

typedef struct Jsi_LoadHandle Jsi_LoadHandle;
typedef struct stat Jsi_StatBuf;
typedef struct dirent Jsi_Dirent;

typedef int (Jsi_FSStatProc) (Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf);
typedef int (Jsi_FSAccessProc) (Jsi_Interp *interp, Jsi_Value* path, int mode);
typedef int (Jsi_FSChmodProc) (Jsi_Interp *interp, Jsi_Value* path, int mode);
typedef Jsi_Channel (Jsi_FSOpenProc) (Jsi_Interp *interp, Jsi_Value* path, const char* modes);
typedef int (Jsi_FSLstatProc) (Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf);
typedef int (Jsi_FSCreateDirectoryProc) (Jsi_Interp *interp, Jsi_Value* path);
typedef int (Jsi_FSRemoveProc) (Jsi_Interp *interp, Jsi_Value* path, int flags);
typedef int (Jsi_FSCopyDirectoryProc) (Jsi_Interp *interp, Jsi_Value *srcPathPtr, Jsi_Value *destPathPtr, Jsi_Value **errorPtr);
typedef int (Jsi_FSCopyFileProc) (Jsi_Interp *interp, Jsi_Value *srcPathPtr, Jsi_Value *destPathPtr);
typedef int (Jsi_FSRemoveDirectoryProc) (Jsi_Interp *interp, Jsi_Value* path, int recursive, Jsi_Value **errorPtr);
typedef int (Jsi_FSRenameProc) (Jsi_Interp *interp, Jsi_Value *srcPathPtr, Jsi_Value *destPathPtr);
typedef Jsi_Value * (Jsi_FSListVolumesProc) (Jsi_Interp *interp);
typedef char* (Jsi_FSRealPathProc) (Jsi_Interp *interp, Jsi_Value* path, char *newPath);
typedef int (Jsi_FSLinkProc) (Jsi_Interp *interp, Jsi_Value* path, Jsi_Value *toPath, int linkType);
typedef int (Jsi_FSReadlinkProc)(Jsi_Interp *interp, Jsi_Value *path, char *buf, int size);
typedef int (Jsi_FSReadProc)(Jsi_Channel chan, char *buf, int size);
typedef int (Jsi_FSGetcProc)(Jsi_Channel chan);
typedef int (Jsi_FSEofProc)(Jsi_Channel chan);
typedef int (Jsi_FSTruncateProc)(Jsi_Channel chan, uint len);
typedef int (Jsi_FSUngetcProc)(Jsi_Channel chan, int ch);
typedef char *(Jsi_FSGetsProc)(Jsi_Channel chan, char *s, int size);
typedef int (Jsi_FSPutsProc)(Jsi_Channel chan, const char* str);
typedef int (Jsi_FSWriteProc)(Jsi_Channel chan, const char *buf, int size);
typedef int (Jsi_FSFlushProc)(Jsi_Channel chan);
typedef int (Jsi_FSSeekProc)(Jsi_Channel chan, Jsi_Wide offset, int mode);
typedef int (Jsi_FSTellProc)(Jsi_Channel chan);
typedef int (Jsi_FSCloseProc)(Jsi_Channel chan);
typedef int (Jsi_FSRewindProc)(Jsi_Channel chan);
typedef bool (Jsi_FSPathInFilesystemProc) (Jsi_Interp *interp, Jsi_Value* path,void* *clientDataPtr);
typedef int (Jsi_FSScandirProc)(Jsi_Interp *interp, Jsi_Value *path, Jsi_Dirent ***namelist,
  int (*filter)(const Jsi_Dirent *), int (*compar)(const Jsi_Dirent **, const Jsi_Dirent**));

typedef struct Jsi_Filesystem {
    const char *typeName;
    int structureLength;    
    int version;
    Jsi_FSPathInFilesystemProc *pathInFilesystemProc;
    Jsi_FSRealPathProc *realpathProc;
    Jsi_FSStatProc *statProc;
    Jsi_FSLstatProc *lstatProc;
    Jsi_FSAccessProc *accessProc;
    Jsi_FSChmodProc *chmodProc;
    Jsi_FSOpenProc *openProc;
    Jsi_FSScandirProc *scandirProc;
    Jsi_FSReadProc *readProc;
    Jsi_FSWriteProc *writeProc;
    Jsi_FSGetsProc *getsProc;
    Jsi_FSGetcProc *getcProc;
    Jsi_FSUngetcProc *ungetcProc;
    Jsi_FSPutsProc *putsProc;
    
    Jsi_FSFlushProc *flushProc;
    Jsi_FSSeekProc *seekProc;
    Jsi_FSTellProc *tellProc;
    Jsi_FSEofProc *eofProc;
    Jsi_FSTruncateProc *truncateProc;
    Jsi_FSRewindProc *rewindProc;
    Jsi_FSCloseProc *closeProc;
    Jsi_FSLinkProc *linkProc;
    Jsi_FSReadlinkProc *readlinkProc;
    Jsi_FSListVolumesProc *listVolumesProc;
    Jsi_FSCreateDirectoryProc *createDirectoryProc;
    Jsi_FSRemoveProc *removeProc;
    Jsi_FSRenameProc *renameProc;
    void *reserved[10];     /* Reserved for future */
} Jsi_Filesystem;

typedef struct Jsi_Chan {
    FILE *fp;
    const char *fname;  /* May be set by fs or by source */
    Jsi_Filesystem *fsPtr;
    int isNative;
    int flags;
    char modes[JSI_FSMODESIZE];
    void *data;
    void *reserved[4];     /* Reserved for future */
    ssize_t resInt[2];
} Jsi_Chan;

JSI_EXTERN Jsi_RC Jsi_FSRegister(Jsi_Filesystem *fsPtr, void *data); /*STUB = 363*/
JSI_EXTERN Jsi_RC Jsi_FSUnregister(Jsi_Filesystem *fsPtr); /*STUB = 364*/
JSI_EXTERN Jsi_Channel Jsi_FSNameToChannel(Jsi_Interp *interp, const char *name); /*STUB = 365*/
JSI_EXTERN char* Jsi_GetCwd(Jsi_Interp *interp, Jsi_DString *cwdPtr); /*STUB = 366*/
JSI_EXTERN int Jsi_Lstat(Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf); /*STUB = 367*/
JSI_EXTERN int Jsi_Stat(Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf); /*STUB = 368*/
JSI_EXTERN int Jsi_Access(Jsi_Interp *interp, Jsi_Value* path, int mode); /*STUB = 369*/
JSI_EXTERN int Jsi_Remove(Jsi_Interp *interp, Jsi_Value* path, int flags); /*STUB = 370*/
JSI_EXTERN int Jsi_Rename(Jsi_Interp *interp, Jsi_Value *src, Jsi_Value *dst); /*STUB = 371*/
JSI_EXTERN int Jsi_Chdir(Jsi_Interp *interp, Jsi_Value* path); /*STUB = 372*/
JSI_EXTERN Jsi_Channel Jsi_Open(Jsi_Interp *interp, Jsi_Value *file, const char *modeString); /*STUB = 373*/
JSI_EXTERN int Jsi_Eof(Jsi_Interp *interp, Jsi_Channel chan); /*STUB = 374*/
JSI_EXTERN int Jsi_Close(Jsi_Interp *interp, Jsi_Channel chan); /*STUB = 375*/
JSI_EXTERN int Jsi_Read(Jsi_Interp *interp, Jsi_Channel chan, char *bufPtr, int toRead); /*STUB = 376*/
JSI_EXTERN int Jsi_Write(Jsi_Interp *interp, Jsi_Channel chan, const char *bufPtr, int slen); /*STUB = 377*/
JSI_EXTERN Jsi_Wide Jsi_Seek(Jsi_Interp *interp, Jsi_Channel chan, Jsi_Wide offset, int mode); /*STUB = 378*/
JSI_EXTERN Jsi_Wide Jsi_Tell(Jsi_Interp *interp, Jsi_Channel chan); /*STUB = 379*/
JSI_EXTERN int Jsi_Truncate(Jsi_Interp *interp, Jsi_Channel chan, uint len); /*STUB = 380*/
JSI_EXTERN Jsi_Wide Jsi_Rewind(Jsi_Interp *interp, Jsi_Channel chan); /*STUB = 381*/
JSI_EXTERN int Jsi_Flush(Jsi_Interp *interp, Jsi_Channel chan); /*STUB = 382*/
JSI_EXTERN int Jsi_Getc(Jsi_Interp *interp, Jsi_Channel chan); /*STUB = 383*/
JSI_EXTERN int Jsi_Printf(Jsi_Interp *interp, Jsi_Channel chan, const char *fmt, ...) /*STUB = 384*/ __attribute__((format (printf,3,4))); 
JSI_EXTERN int Jsi_Ungetc(Jsi_Interp *interp, Jsi_Channel chan, int ch); /*STUB = 385*/
JSI_EXTERN char* Jsi_Gets(Jsi_Interp *interp, Jsi_Channel chan, char *s, int size); /*STUB = 386*/
JSI_EXTERN int Jsi_Puts(Jsi_Interp *interp, Jsi_Channel chan, const char *str, int size); /*STUB = 387*/

typedef int (Jsi_ScandirFilter)(const Jsi_Dirent *);
typedef int (Jsi_ScandirCompare)(const Jsi_Dirent **, const Jsi_Dirent**);
JSI_EXTERN int Jsi_Scandir(Jsi_Interp *interp, Jsi_Value *path, Jsi_Dirent ***namelist, Jsi_ScandirFilter *filter, Jsi_ScandirCompare *compare ); /*STUB = 388*/
JSI_EXTERN int Jsi_SetChannelOption(Jsi_Interp *interp, Jsi_Channel chan, const char *optionName, const char *newValue); /*STUB = 389*/
JSI_EXTERN char* Jsi_Realpath(Jsi_Interp *interp, Jsi_Value *path, char *newname); /*STUB = 390*/
JSI_EXTERN int Jsi_Readlink(Jsi_Interp *interp, Jsi_Value* path, char *ret, int len); /*STUB = 391*/
JSI_EXTERN Jsi_Channel Jsi_GetStdChannel(Jsi_Interp *interp, int id); /*STUB = 392*/
JSI_EXTERN bool Jsi_FSNative(Jsi_Interp *interp, Jsi_Value* path); /*STUB = 393*/
JSI_EXTERN int Jsi_Link(Jsi_Interp *interp, Jsi_Value* src, Jsi_Value *dest, int typ); /*STUB = 394*/
JSI_EXTERN int Jsi_Chmod(Jsi_Interp *interp, Jsi_Value* path, int mode); /*STUB = 395*/

JSI_EXTERN Jsi_RC Jsi_StubLookup(Jsi_Interp *interp, const char *name, void **ptr); /*STUB = 396*/
JSI_EXTERN Jsi_RC Jsi_DllLookup(Jsi_Interp *interp, const char *module, const char *name, void **ptr); /*STUB = 404*/
JSI_EXTERN Jsi_RC Jsi_LoadLibrary(Jsi_Interp *interp, const char *pathName, bool noInit); /*STUB = 405*/
JSI_EXTERN int Jsi_AddAutoFiles(Jsi_Interp *interp, const char *dir);  /*STUB = 397*/

/* -- */



/* --DATABASE-- */

JSI_EXTERN Jsi_Db* Jsi_DbNew(const char *zFile, int inFlags); /*STUB = 398*/
JSI_EXTERN void* Jsi_DbHandle(Jsi_Interp *interp, Jsi_Db* db); /*STUB = 399*/

/* -- */


/* --CData-- */

#define  JSI_CDATA_OPTION_CHANGED(name) \
        (vrPtr->spec && Jsi_OptionsChanged(interp, vrPtr->spec, #name, NULL))
#define  JSI_CDATA_OPTION_RESET(name) \
        (cmdPtr->queryOpts.mode && !options->name && JSI_CDATA_OPTION_CHANGED(name))
 
typedef struct Jsi_CData_Static {
    const char* name;
    Jsi_StructSpec* structs;
    Jsi_EnumSpec* enums;
    Jsi_VarSpec *vars;
    Jsi_OptionTypedef* types;
    struct Jsi_CData_Static* nextPtr;
} Jsi_CData_Static;

/* Struct for Carray to bind Data/Option pairs to individual SQLite binding chars. */
typedef struct Jsi_CDataDb {
#define JSI_DBDATA_FIELDS \
    Jsi_StructSpec *sf;     /* Struct fields for data. */ \
    void *data;             /* Data pointer for array/map */ \
    uint arrSize;           /* If an array, number of elements: 0 means 1. */ \
    char prefix;            /* Sqlite char bind prefix. One of: '@' '$' ':' '?' or 0 for any */ \
    Jsi_StructSpec* slKey;  /* Struct for key (for map using a struct key). */ \
    int (*callback)(Jsi_Interp *interp, struct Jsi_CDataDb* obPtr, void *data); /* C callback for select queries. */ \
    uint maxSize;           /* Limit size of array/map*/ \
    bool noAuto;            /* Do not auto-create map keys. */ \
    bool isPtrs;            /* "data" an array of pointers. */ \
    bool isPtr2;            /* "data" is pointer to pointers, which is updated. */ \
    bool isMap;             /* "data" is a map: use Jsi_MapConf() for details. */ \
    bool memClear;          /* Before query free and zero all data (eg. DStrings). */ \
    bool memFree;           /* Reset as per mem_clear, then free data items. Query may be empty. */ \
    bool dirtyOnly;         /* Sqlite dirty filter for UPDATE/INSERT/REPLACE. */ \
    bool noBegin;           /* Disable wrapping UPDATE in BEGIN/COMMIT. */ \
    bool noCache;           /* Disable Db caching statement. */ \
    bool noStatic;          /* Disable binding text with SQLITE_STATIC. */ \
    intptr_t reserved[4];   /* Internal use. */
JSI_DBDATA_FIELDS
} Jsi_CDataDb;

JSI_EXTERN int Jsi_DbQuery(Jsi_Db *jdb, Jsi_CDataDb *cd, const char *query); /*STUB = 400*/
JSI_EXTERN Jsi_CDataDb* Jsi_CDataLookup(Jsi_Interp *interp, const char *name); /*STUB = 401*/
JSI_EXTERN Jsi_RC Jsi_CDataRegister(Jsi_Interp *interp, Jsi_CData_Static *statics); /*STUB = 402*/
JSI_EXTERN Jsi_RC Jsi_CDataStructInit(Jsi_Interp *interp, uchar* data, const char *sname); /*STUB = 403*/
JSI_EXTERN Jsi_StructSpec* Jsi_CDataStruct(Jsi_Interp *interp, const char *name); /*STUB = 406*/
/* -- */


/* String */
typedef char STRING1[(1<<0)+1]; // Include a char for the null byte.
typedef char STRING2[(1<<1)+1];
typedef char STRING4[(1<<2)+1];
typedef char STRING8[(1<<3)+1];
typedef char STRING16[(1<<4)+1];
typedef char STRING32[(1<<5)+1];
typedef char STRING64[(1<<6)+1];
typedef char STRING128[(1<<7)+1];
typedef char STRING256[(1<<8)+1];
typedef char STRING512[(1<<9)+1];
typedef char STRING1024[(1<<10)+1];
typedef char STRING2048[(1<<11)+1];
typedef char STRING4096[(1<<12)+1];
typedef char STRING8192[(1<<13)+1];
typedef char STRING16384[(1<<14)+1];
typedef char STRING32768[(1<<15)+1];
typedef char STRING65536[(1<<16)+1];

/* -- */


#define JSI_STUBS_STRUCTSIZES (sizeof(Jsi_MapSearch)+sizeof(Jsi_TreeSearch) \
    +sizeof(Jsi_HashSearch)+sizeof(Jsi_Filesystem)+sizeof(Jsi_Chan)+sizeof(Jsi_Event) \
    +sizeof(Jsi_CDataDb)+sizeof(Jsi_Stack)+sizeof(Jsi_OptionSpec)+sizeof(Jsi_CmdSpec) \
    +sizeof(Jsi_UserObjReg)+sizeof(Jsi_String) + sizeof(Jsi_PkgOpts))

#ifndef JSI_OMIT_STUBS
#ifdef JSI_USE_STUBS
#ifndef JSISTUBCALL
#define JSISTUBCALL(ptr,func) ptr->func
#endif
#include "jsiStubs.h"
#else
#define JSI_EXTENSION_INI
#define Jsi_StubsInit(i,f) JSI_OK
#endif
#endif


/* Optional compile-out commands/options string information. */
#ifdef JSI_OMIT_INFO
#define JSI_INFO(n) NULL
#endif
#ifndef JSI_INFO
#define JSI_INFO(n) n
#endif

#endif /* __JSI_H__ */


