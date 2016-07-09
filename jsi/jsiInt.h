#ifndef __JSIINT_H__
#define __JSIINT_H__

//#define HIDE_MEMLEAKS /* TODO: fix memory leaks and ref count bugs, ie Jsi_DecrRefCount */
#define JSI_DEBUG_MEMORY
#ifdef JSI_DEBUG_MEMORY
#define Assert(n) assert(n)
#else
#define Assert(n)
#endif
#ifndef JSI_OMIT_SIGNATURES
#define JSI_HAS_SIG
#define JSI_HAS_SIG_HASHENTRY 1
#define JSI_HAS_SIG_LISTENTRY 1
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#define __USE_GNU
#endif
#define VAL_REFCNT
#define VAL_REFCNT2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#ifdef __WIN32 /* TODO: support windows signal??? */
#define JSI_OMIT_SIGNAL
#endif

#ifndef JSI_AMALGAMATION

#ifdef __WIN32
#include "win/compat.h"
#include "win/regex.h"
#else
#define HAVE_REGCOMP
#include <sys/time.h>
#endif
#endif

#include <time.h>

#ifndef JSI_IS64BIT
#ifdef __GNUC__
#ifdef __X86_64__
#define JSI_IS64BIT 1
#endif
#else /* GCC */
#if _WIN64 || __amd64__
#define JSI_IS64BIT 1
#endif
#endif /* GCC */
#endif /* JSI_IS64BIT */

#ifndef JSI_IS64BIT
#define JSI_IS64BIT 0
#endif

#define JSMN_FREE(p) Jsi_Free(p)
#define JSMN_MALLOC(l) Jsi_Malloc(l)
#define JSMN_REALLOC(p,l) Jsi_Realloc(p,l)

#define JSI_HAS_SIG /* Signatures to help with debugging */
#ifdef JSI_HAS_SIG
#ifndef SIGASSERT
#define SIGASSERT(s,n) assert((s) && (s)->sig == (int)JSI_SIG_##n);
#endif
#define SIGINIT(s,n) (s)->sig = JSI_SIG_##n;
#define __VALSIG__ JSI_SIG_VALUE,
#else
#define SIGASSERT(s,n)
#define SIGINIT(s,n)
#define __VALSIG__
#endif

#define JSI_HAS___PROTO__ 1  // Support setting and getting prototypes. 1=set/get funcs, 2=__proto__ assign.

#ifndef uint
typedef unsigned int uint;
#endif
#ifndef uchar
typedef unsigned char uchar;
#endif

#ifndef JSI_AMALGAMATION
#include "jsi.h"
#endif

#define ALLOC_MOD_SIZE 16      /* Increase allocs by increments of 16. */
#define MAX_ARRAY_LIST 100000  /* Default Max size of an array convertable to list form */
#define MAX_LOOP_COUNT 10000000 /* Limit infinite loops */
#define JSI_MAX_ALLOC_BUF  100000000 /* Limit for dynamic memory allocation hunk */
#define JSI_MAX_SCOPE (BUFSIZ/2)
typedef enum {
    JSI_SIG_ITEROBJ=0xdeadbee1, JSI_SIG_FUNCOBJ, JSI_SIG_SCOPE, JSI_SIG_VALUE,
    JSI_SIG_OBJ, JSI_SIG_USERDATA, JSI_SIG_INTERP, JSI_SIG_PARSER,
    JSI_SIG_FILEOBJ, JSI_SIG_INTERPOBJ, JSI_SIG_FUNC, JSI_SIG_CMDSPECITEM, JSI_SIG_HASH,
    JSI_SIG_HASHENTRY, JSI_SIG_TREE, JSI_SIG_TREEENTRY, JSI_SIG_LIST, JSI_SIG_LISTENTRY,
    JSI_SIG_USER_REG, JSI_SIG_EVENT,
    JSI_SIG_ARGTYPE, JSI_SIG_FORINVAR, JSI_SIG_CASELIST, JSI_SIG_CASESTAT,
    JSI_SIG_FASTVAR, JSI_SIG_INTERPSTREVENT, JSI_SIG_ALIASCMD, JSI_SIG_SOCKET, JSI_SIG_SOCKETPSS
} jsi_Sig;

struct OpCode;

#ifdef  JSI_DEBUG_MEMORY
extern void jsi_VALCHK(Jsi_Value *v);
extern void jsi_OBJCHK(Jsi_Obj *o);
#define VALCHK(val) jsi_VALCHK(val)
#define OBJCHK(val) jsi_OBJCHK(val)
#else
#define VALCHK(val)
#define OBJCHK(val)
#endif

/* Scope chain */
typedef struct jsi_ScopeChain_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_Interp *interp;
    Jsi_Value **chains;  /* values(objects) */
    int chains_cnt;         /* count */
} jsi_ScopeChain;

/* Function obj */
/* a Jsi_FuncObj is a raw function with own scope chain */
struct Jsi_FuncObj_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_Interp *interp;
    Jsi_Func *func;
    jsi_ScopeChain *scope;
};

typedef int (Jsi_IterProc)(Jsi_IterObj *iterObj, Jsi_Value *val, Jsi_Value *var, int index);

/* Jsi_IterObj, use only in for-in statement */
struct Jsi_IterObj_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_Interp *interp;
    const char **keys;
    int size; 
    int count;
    int iter;
    int isArrayList;            /* If an array list do not store keys. */
    Jsi_Obj *obj;
    int cur;                    /* Current array cursor. */
    int depth;                  /* Used to create list of keys. */
    Jsi_IterProc *iterCmd;
};

typedef struct UserObjReg_ { /* Per interp userobj registration. */
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_UserObjReg *reg;
    Jsi_Hash* hashPtr;
    int idx;
} UserObjReg;

/* User defined object */
typedef struct Jsi_UserObj_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_Interp *interp;
    Jsi_Hash *id;
    void *data;
    const char *prefix;
    Jsi_UserObjReg *reg;
    struct UserObjReg_ *ureg;
    uint idx;
    Jsi_HashEntry* hPtr;
} Jsi_UserObj;

typedef struct {
    int valueCnt;
    int objCnt;
    int valueAllocCnt;
    int objAllocCnt;
#ifdef JSI_MEM_DEBUG
    Jsi_Hash *valueDebugTbl;
    Jsi_Hash *objDebugTbl;
    int memDebugCallIdx;
#endif
} Jsi_InterpDebug;

#ifdef JSI_MEM_DEBUG
#ifndef JSI_AMALGAMATION
#include "jsiCode.h"
#endif
typedef struct 
{
    const char *fname;
    int line;
    const char *func;
    const char *label;
    const char *label2;
    const char *label3;
    uint Idx;
    uint flags;
    struct OpCode *ip;
    int ipLine;
    Eopcode ipOp;
    const char* ipFname;
    Jsi_HashEntry *hPtr;
    Jsi_Interp *interp;
} jsi_ValueDebug;
#endif

struct Jsi_Obj_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    int refcnt;                 /* reference count */
    Jsi_otype ot:8;             /* object type */
    uint isarrlist:1;           /* Array type. */
    uint isstrkey:1;            /* Key string registered in interp->strKeyTbl (do not free) */
    uint isJSONstr:1;
    uint clearProto:1;          /* Prototype changed, clean it up at exit. */
    uint unused1:2;
    uint unused2:16;
    union {                     /* switched on by value of "ot" */
        int val;
        Jsi_Number num;
        Jsi_String s;
        Jsi_Regex *robj;
        Jsi_FuncObj *fobj;
        Jsi_IterObj *iobj;
        Jsi_UserObj *uobj;
    } d;
    uint arrMaxSize;                 /* Max allocated space for array. */
    uint arrCnt;                     /* Count of actually set keys. */
    Jsi_Value **arr;   /* Array values. */  
    Jsi_Tree *tree;                 /* Tree storage (should be union with array). */
    Jsi_Value *__proto__;           /* TODO: memory leaks when this is changed */
    struct Jsi_Obj_ *constructor;
#ifdef JSI_MEM_DEBUG
    jsi_ValueDebug VD;
#endif
};

/*#pragma pack(1)*/


struct Jsi_Value_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    int refCnt:24;
    Jsi_vtype vt:8;             /* value type */
    union {
        uint flag:8;
        struct vflagbit {
            uint readonly:1;
            uint dontenum:1;  /* Dont enumerate. */
            uint dontdel:1;
            uint innershared:1; /* All above used only for objkeys. */
            uint isarrlist:1;
            uint isstrkey:1;    /* Key string registered in interp->strKeyTbl (do not free) */
            uint unused:1;
            uint onstack:1;     /* Stack value. */  
        } bits;
    } f;
    union {                     /* see above */
        int val;
        Jsi_Number num;
        Jsi_String s;
        Jsi_Obj *obj;
        struct Jsi_Value_ *lval;
    } d;
#ifdef JSI_MEM_DEBUG
    jsi_ValueDebug VD;
#endif
};

#ifndef JSI_SMALL_HASH_TABLE
#define JSI_SMALL_HASH_TABLE 10
#endif

typedef union jsi_HashKey_ {
    void *oneWordValue;
    unsigned long words[1];
    char string[4]; 
} jsi_HashKey;

typedef unsigned int jsi_Hash;

typedef struct Jsi_HashEntry_ {
#ifdef JSI_HAS_SIG_HASHENTRY
    jsi_Sig sig;
#endif
    int typ; // JSI_MAP_HASH
    struct Jsi_HashEntry_ *nextPtr;
    Jsi_Hash *tablePtr;
    jsi_Hash hval;
    void* clientData;
    jsi_HashKey key;
} Jsi_HashEntry;

typedef struct Jsi_Hash_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    int typ; // JSI_MAP_HASH
    Jsi_HashEntry **buckets;
    Jsi_HashEntry *staticBuckets[JSI_SMALL_HASH_TABLE];
    int numBuckets;
    int numEntries;
    int rebuildSize;
    jsi_Hash mask;
    unsigned int downShift;
    int keyType;
    Jsi_HashEntry *(*createProc) (Jsi_Hash *tablePtr, const void *key, int *newPtr);
    Jsi_HashEntry *(*findProc) (Jsi_Hash *tablePtr, const void *key);
    Jsi_HashDeleteProc *freeProc;
    int (*lockProc) (Jsi_Hash *tablePtr, int lock);
    Jsi_Interp *interp;
} Jsi_Hash;

struct Jsi_Tree_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    int typ; // JSI_MAP_TREE
    Jsi_Interp *interp;
    Jsi_TreeEntry *root;
    unsigned int numEntries, keyType, epoch;
    struct {
        unsigned int 
            inserting:1, destroyed:1,
            nonredblack:1,  /* Disable red/black handling on insert/delete. */
            internstr:1,    /* STRINGPTR keys are stored in strHash */
            valuesonly:1,   /* Values must be of type JSI_VALUE */
            unused:28;
    } flags;
    Jsi_Hash* strHash;  /* String hash table to use if INTERNSTR; setup on first Create if not defined. */
    Jsi_RBCreateProc *createProc;
    Jsi_RBCompareProc *compareProc;
    Jsi_DeleteProc *freeProc;
};

typedef union jsi_TreeKey_ {
    void *oneWordValue;
    char string[4];
    const char *stringKey;
} jsi_TreeKey;

typedef struct Jsi_TreeEntry_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    int typ; // JSI_MAP_TREE
    Jsi_Tree *treePtr;
    struct Jsi_TreeEntry_* left;
    struct Jsi_TreeEntry_* right;
    struct Jsi_TreeEntry_* parent;
    union { /* FLAGS: bottom 16 bits for JSI, upper 16 bits for users. First 7 map to JSI_OM_ above. */
        struct { 
            unsigned int readonly:1, dontenum:1, dontdel:1, innershared:1, isarrlist:1, isstrkey:1, unused:1,
                color:1,
                reserve:8,
                user0:8,
                user1:1, user2:1, user3:1, user4:1, user5:1, user6:1, user7:1, user8:1;
        } bits;
        int flags;
    } f;
    void* value;
    jsi_TreeKey key;
} Jsi_TreeEntry;

typedef struct Jsi_List_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    int numEntries;
    Jsi_ListEntry *head;
    Jsi_ListEntry *tail;
    Jsi_Mutex *mutex;
    Jsi_ListAttr attr;
    /*void *data;
    int dataSpace;
    int useMutex;
    Jsi_ListDeleteProc *freeProc; // Related lists share this field.
    Jsi_ListLockProc *lockProc; */
} Jsi_List;

typedef struct Jsi_ListEntry_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    struct Jsi_ListEntry_ *next;
    struct Jsi_ListEntry_ *prev;
    struct Jsi_List_ *list;
    void *value;
} Jsi_ListEntry;


// List is a slightly less efficent thin wrapper around Tree/Hash: abstracts ordered/unordered lists.
typedef struct Jsi_Map_ {
    Jsi_Map_Type typ;
    union {
        Jsi_Hash *hash;
        Jsi_Tree *tree;
    } v;
} Jsi_Map;


typedef struct jsi_ArgValue_ {
    char *name;
    int type;
    Jsi_Value *defValue;
} jsi_ArgValue;

typedef struct Jsi_ScopeStrs_ {
    jsi_ArgValue *args;
    int count;
    int _size;  // Used in allocation only.
    int varargs;
    int typeCnt;
    int firstDef;
    int argCnt;
} Jsi_ScopeStrs;

struct OpCodes_;
typedef struct OpCodes_ OpCodes;

/* Program/parse state(context) */
typedef struct jsi_Pstate_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    int err_count;              /* Jsi_LogError count after parse */
    int eval_flag;              /* 1 if currently executing in an eval function */
    int funcDefs;               /* Count of functions defined. 0 means we can delete this cache (eventually). */
    OpCodes *opcodes;    /* Execution codes. */
    struct Lexer *lexer;        /* seq provider */

    int _context_id;            /* used in FastVar-locating */
    Jsi_Value *last_exception;
    Jsi_Interp *interp;
    Jsi_HashEntry *hPtr;
    Jsi_Hash *argsTbl;
    Jsi_Hash *strTbl;
    Jsi_Hash *fastVarTbl;
    int argType;                // Used during parsing to aggregate type.
} jsi_Pstate;

#ifndef JSI_AMALGAMATION
#include "jsiPstate.h"
#include "parser.h"
#include "jsiCode.h"
#include "jsiLexer.h"
#endif

typedef struct FastVar_ {
    jsi_Sig sig;
    int context_id:31;
    unsigned int isglob:1;
    jsi_Pstate *ps;
    struct {
        char *varname;
        struct Jsi_Value_ *lval;
    } var;
} FastVar;

typedef enum { FC_NORMAL, FC_BUILDIN } Jsi_Func_Type;

/* raw function data, with script function or system Jsi_CmdProc */
struct Jsi_Func_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_Func_Type type;                         /* type */
    struct OpCodes_ *opcodes;    /* FC_NORMAL, codes of this function */
    Jsi_CmdProc *callback;            /* FC_BUILDIN, callback */

    Jsi_ScopeStrs *argnames;                 /* FC_NORMAL, argument names */
    Jsi_ScopeStrs *localnames;               /* FC_NORMAL, local var names */
    union {
        uint flags;
        struct {
            uint res:8, hasattr:1, isobj:1 , iscons:1, res2:4;
        } bits;
    } f;
    union {
        uint i;
        struct {
            uint addargs:1 , iscons:1, isdiscard:1, res:5;
        } bits;
    } callflags;
    int refCnt;
    void *privData;                 /* context data given in create. */
    Jsi_CmdSpec *cmdSpec;
    const char *name;  /* Name for non-anonymous function. */
    uint retType;  /* Type name: or of Jsi_otype*/
    int callCnt;
    Jsi_ScopeStrs *typeArgs;
    const char *script;  /* Script created in. */
    Jsi_Value *bindArgs;
    jsi_Pline bodyline; /* Body line info. */
    Jsi_HashEntry *hPtr;
};

typedef struct {
    char *origFile; /* Short file name. */
    char *fileName; /* Fully qualified name. */
    char *dirName;  /* Directory name. */
    int useCnt;
} jsi_FileInfo;

enum {
    STACK_INIT_SIZE=1024, STACK_INCR_SIZE=1024, STACK_MIN_PAD=100,
    JSI_MAX_CALLFRAME_DEPTH=1000, /* default max nesting depth for procs */
    JSI_MAX_EVAL_DEPTH=1000, /* default max nesting depth for eval */
    JSI_MAX_INCLUDE_DEPTH=50,  JSI_MAX_SUBINTERP_DEPTH=10
    /*,JSI_ON_STACK=0x80*/
};

typedef struct InterpStrEvent_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    int rc, isExec, tryDepth, errLine;
    const char *errFile;
    Jsi_Value *objData;
    Jsi_DString func;
    Jsi_DString data;
    struct InterpStrEvent_ *next;
    void *mutex;
} InterpStrEvent;

typedef enum { jsi_TypeChk_Disable, jsi_TypeChk_Warn, jsi_TypeChk_Error, jsi_TypeChk_Static } jsi_TypeChk;
typedef void (*jsiCallTraceProc)(Jsi_Interp *interp, const char *funcName, const char *file, int line, Jsi_CmdSpec* spec, Jsi_Value* _this, Jsi_Value* args);

struct Jsi_Interp_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    uint deleting:1;
    uint destroying:1;
    uint hasEventHdl:1;
    uint indexLoaded:2;
    int exited;
    int exitCode;
    int refCount;
    int callTrace;
    int opTrace;
    int memDebug;
    Jsi_Bool nDebug; /* Disable asserts */
    Jsi_Bool isMain;
    Jsi_Bool compat;
    Jsi_Bool doUnlock;
    Jsi_Bool isSafe;
    Jsi_Bool noInherit;
    Jsi_Bool noUndef;
    Jsi_Bool fileStrict;  /* Error out on file not found, etc. */
    Jsi_Bool hasCallee;
    Jsi_Bool noreadline;
    Jsi_Bool nocacheOpCodes;
    Jsi_Bool noSubInterps;
    Jsi_Bool privKeys;
    Jsi_Bool subthread;
    Jsi_Bool logAllowDups;
    jsi_TypeChk typeCheck;
   // Jsi_Bool staticCheck;
    int flags;
    int evalFlags;
    Jsi_InterpDebug dbStatic;
    Jsi_InterpDebug *dbPtr;
    jsiCallTraceProc traceHook;
    int opCnt;  /* Count of instructions eval'ed */
    int maxOpCnt;
    int maxUserObjs;
    int userObjCnt;
    int funcCnt;
    int level;  /* Nesting level of eval/func/cmd calls. */
    int maxDepth;/* Max allowed eval recursion. */
    int maxIncDepth;
    int includeDepth;
    int maxInterpDepth;
    int interpDepth;
    int didReturn;
    uint codeCacheHit;
    uint funcCallCnt;
    uint cmdCallCnt;
    uint eventIdx;
#ifdef JSI_MEM_DEBUG
    uint valueDebugIdx;
    Jsi_Hash *codesTbl;
#endif
    jsi_ScopeChain *gsc;
    Jsi_Value *csc;
    struct Jsi_Interp_ *parent, *mainInterp;
    jsi_ScopeChain *ingsc;
    Jsi_Value *incsc;
    Jsi_Value *inthis;
    Jsi_Value *onComplete;
    Jsi_Value *onEval;
    Jsi_Value *onExit;
    Jsi_Value *safeReadDirs;
    Jsi_Value *safeWriteDirs;
    Jsi_Value *execZip;
    void (*logHook)(char *buf, va_list va);
    int (*evalHook)(struct Jsi_Interp_* interp, const char *curFile, int curLine);
    const char *name;

    int tryDepth, withDepth, inParse;
    Jsi_Value *retPtr;       /* Return value from eval */
    jsi_Pstate *ps;
    /*int argc;
    char **argv;*/
    Jsi_Value *argv0;
    Jsi_Value *args;
    Jsi_Value *console;
    Jsi_Value *scriptFile;  /* Start script returned by info.argv0(). */
    const char *scriptStr;
    const char *curFile;
    const char *homeDir;
    char *curDir;
    int asc;    /* "A constructor" used by IsConstructorCall(). */
    int Sp;
    int maxStack;

    Jsi_Hash *onDeleteTbl;  /* Cleanup funcs to call on interp delete. */
    Jsi_Hash *assocTbl;
    Jsi_Hash *cmdSpecTbl; /* Jsi_CmdSpecs registered. */
    Jsi_Hash *codeTbl; /* Scripts compiled table. */
    Jsi_Hash *eventTbl;
    Jsi_Hash *genValueTbl;
    Jsi_Hash *genObjTbl;
    Jsi_Hash *genDataTbl;
    Jsi_Hash *funcObjTbl;
    Jsi_Hash *funcsTbl;
    Jsi_Hash *bindTbl;
    Jsi_Hash *fileTbl;
    Jsi_Hash *lexkeyTbl;
    Jsi_Hash *protoTbl;
    Jsi_Hash *regexpTbl;    
    Jsi_Hash *strKeyTbl;  /* Global strings table. */
    Jsi_Hash *thisTbl;
    Jsi_Hash *userdataTbl;
    Jsi_Hash *varTbl;
    Jsi_Hash *preserveTbl;
    Jsi_Hash *loadTbl;
    Jsi_Hash *optionDataHash;
#ifdef VAL_REFCNT
    Jsi_Value **Stack;
    Jsi_Value **Obj_this;
#else
    Jsi_Value *Stack;
    Jsi_Value *Obj_this;
#endif
            
    Jsi_Value *Object_prototype;
    Jsi_Value *Function_prototype_prototype;
    Jsi_Value *Function_prototype;
    Jsi_Value *String_prototype;
    Jsi_Value *Number_prototype;
    Jsi_Value *Boolean_prototype;
    Jsi_Value *Array_prototype;
    Jsi_Value *RegExp_prototype;
    Jsi_Value *Date_prototype;
    
    Jsi_Value *NaNValue;
    Jsi_Value *InfValue;
    Jsi_Value *NullValue;
    Jsi_Value *nullFuncArg; /* Efficient call of no-arg func */
    Jsi_Value *nullFuncRet;
    Jsi_Value *indexFiles;
    Jsi_Obj* cleanObjs[4];

    const char *logCallback;
    const char *evalCallback;
    
    Jsi_Value *Top_object;
    Jsi_Func *lastFuncIndex;

    Jsi_Value* lastSubscriptFail;
    char* lastSubscriptFailStr;
    Jsi_Obj *lastBindSubscriptObj; /* Used for FUNC.bind() */

    Jsi_ScopeStrs *scopes[JSI_MAX_SCOPE];
    int cur_scope;
    int maxArrayList;
    int delRBCnt;
    Jsi_Func *activeFunc;  // Currently active function call.
    OpCode *curIp;  /* These 2 used for debug Log msgs. */
    char *lastPushStr; 
    char *lastPushVar;
    Jsi_Wide sigmask;
    char errMsgBuf[200];  /* Error message space for when in try. */
    int errLine;
    int errCol;
    const char *errFile;
    Jsi_Mutex* Mutex;
    Jsi_Mutex* QMutex; /* For threads queues */
    void* threadId;
    int threadCnt;
    int threadShrCnt;
    int lockTimeout; /* in milliseconds. */
    uint lockRefCnt;
    int psEpoch;
    Jsi_DString interpEvalQ;
    Jsi_DString interpMsgQ;
    InterpStrEvent *interpStrEvents;
    const char *recvCmd;
    uint threadErrCnt;  /* Count of bad thread event return codes. */
    uint threadEvalCnt;
    uint threadMsgCnt;
    void *sleepData;
    jsi_Pline *parseLine;
};


enum { JSI_REG_GLOB=0x1, JSI_REG_NEWLINE=0x2, JSI_REG_DOT_NEWLINE=0x4, JSI_REG_STATIC=0x100 };

struct Jsi_Regex_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    regex_t reg;
    int eflags;
    int flags;
    char *pattern;
};


/* Entries in interp->cmdSpecTbl. */
typedef struct Jsi_CmdSpecItem_ {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    const char *name;  /* Parent cmd. */
    Jsi_CmdSpec *spec;
    Jsi_Value *proto;
    int flags;
    void *privData;
    Jsi_HashEntry *hPtr;
    struct Jsi_CmdSpecItem_ *next; /* TODO: support user-added sub-commands. */
    const char *help;
    const char *info;
    int isCons;
} Jsi_CmdSpecItem;


/* SCOPE */
//typedef struct jsi_ScopeChain jsi_ScopeChain;

extern jsi_ScopeChain* jsi_ScopeChainNew(Jsi_Interp *interp, int cnt); /*STUB = 176*/
extern Jsi_Value* jsi_ScopeChainObjLookupUni(jsi_ScopeChain *sc, char *key); /*STUB = 177*/
extern jsi_ScopeChain* jsi_ScopeChainDupNext(Jsi_Interp *interp, jsi_ScopeChain *sc, Jsi_Value *next); /*STUB = 178*/
extern void jsi_ScopeChainFree(Jsi_Interp *interp, jsi_ScopeChain *sc); /*STUB = 179*/

extern Jsi_Interp *jsiMainInterp; /* The main interp */
extern Jsi_Interp *jsiDelInterp; /* Interp being delete: Used by cleanup callbacks. */
extern void jsi_CmdSpecDelete(Jsi_Interp *interp, void *ptr);
int jsi_global_eval(Jsi_Interp* interp, jsi_Pstate *ps, char *program,
        jsi_ScopeChain *scope, Jsi_Value *currentScope,
        Jsi_Value *_this, Jsi_Value **ret);

int jsi_FilesysInit(Jsi_Interp *interp);
int jsi_LexerInit(Jsi_Interp *interp);
int jsi_LoadInit(Jsi_Interp *interp);
int jsi_CmdsInit(Jsi_Interp *interp);
int jsi_InterpInit(Jsi_Interp *interp);
int jsi_FileCmdsInit(Jsi_Interp *interp);
int jsi_StringInit(Jsi_Interp *interp);
int jsi_ValueInit(Jsi_Interp *interp);
int jsi_NumberInit(Jsi_Interp *interp);
int jsi_ArrayInit(Jsi_Interp *interp);
int jsi_BooleanInit(Jsi_Interp *interp);
int jsi_MathInit(Jsi_Interp *interp);
int jsi_ProtoInit(Jsi_Interp *interp);
int jsi_RegexpInit(Jsi_Interp *interp);
int jsi_JSONInit(Jsi_Interp *interp);
int Jsi_InitSqlite(Jsi_Interp *interp);
int Jsi_InitMySql(Jsi_Interp *interp);
int jsi_TreeInit(Jsi_Interp *interp);
int Jsi_InitWebSocket(Jsi_Interp *interp);
int Jsi_InitSocket(Jsi_Interp *interp);
int jsi_SignalInit(Jsi_Interp *interp);
int jsi_execCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_DString *dStr, Jsi_DString *cStr, int *code);
int Jsi_InitZvfs(Jsi_Interp *interp);

void jsi_SignalClear(Jsi_Interp *interp, int sigNum);
int jsi_SignalIsSet(Jsi_Interp *interp, int sigNum);
/* excute opcodes
 * 1. ps, program execution context
 * 2. opcodes, codes to be executed
 * 3. scope, current scopechain, not include current scope
 * 4. currentScope, current scope
 * 5. _this, where 'this' indicated
 * 6. vret, return value
 */
extern int jsi_evalcode(jsi_Pstate *ps, OpCodes *opcodes, 
        jsi_ScopeChain *scope, Jsi_Value *currentScope,
        Jsi_Value *_this,
        Jsi_Value **vret);
        
typedef int (*Jsi_Constructor)(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, int flags, void *privData);
extern int jsi_SharedArgs(Jsi_Interp *interp, Jsi_Value *args, Jsi_Func *func, int alloc);
extern void jsi_SetCallee(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *tocall);
extern int jsi_AssertCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
extern int jsi_InterpInfo(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
    
enum {StrKeyAny = 0, StrKeyFunc = 0x1, StrKeyCmd = 0x2, StrKeyVar = 0x2};

//char* jsi_KeyLookup(Jsi_Interp *interp, const char *str);
char* jsi_KeyFind(Jsi_Interp *interp, const char *str, int nocreate, int *isKey);
void jsi_InitLocalVar(Jsi_Interp *interp, Jsi_Value *arguments, Jsi_Func *who);
Jsi_Value *jsi_GlobalContext(Jsi_Interp *interp);
void jsi_AddEventHandler(Jsi_Interp *interp);

extern const char *jsi_ObjectTypeName(Jsi_Interp *interp, Jsi_otype otyp);
extern const char *jsi_ValueTypeName(Jsi_Interp *interp, Jsi_Value *val);
extern int jsi_ObjectToStringCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
extern int jsi_HasOwnPropertyCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
extern int jsi_Md5File(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
extern int jsi_Sha1File(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
extern int jsi_Sha256File(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
extern int jsi_EncryptFile(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
     Jsi_Value **ret, Jsi_Func *funcPtr, int decrypt);

extern const char *jsi_GetHomeDir(Jsi_Interp *interp);
extern int jsi_RegExpValueNew(Jsi_Interp *interp, const char *regtxt, Jsi_Value *ret);
extern void jsi_DumpOptionSpecs(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_OptionSpec* spec);
extern Jsi_Func *jsi_FuncMake(jsi_Pstate *pstate, Jsi_ScopeStrs *args, OpCodes *ops, jsi_Pline *line, char *name);
extern Jsi_Func *jsi_FuncNew(Jsi_Interp *interp);
extern void jsi_FreeOpcodes(OpCodes *ops);
extern void jsi_DelAssocData(Jsi_Interp *interp, void *data);

extern void jsi_UserObjFree (Jsi_Interp *interp, Jsi_UserObj *uobj);
extern int jsi_UserObjIsTrue (Jsi_Interp *interp, Jsi_UserObj *uobj);
extern int jsi_UserObjDump   (Jsi_Interp *interp, const char *argStr, Jsi_Obj *obj);
extern int jsi_UserObjDelete (Jsi_Interp *interp, void *data);
extern void jsi_UserObjToName(Jsi_Interp *interp, Jsi_UserObj *uobj, Jsi_DString *dStr);
extern Jsi_Obj *jsi_UserObjFromName(Jsi_Interp *interp, const char *name);

extern int Zvfs_Mount( Jsi_Interp *interp, Jsi_Value *archive, Jsi_Value *mount, Jsi_Value **ret);
extern Jsi_Value* jsi_ObjArraySetDup(Jsi_Interp *interp, Jsi_Obj *obj, Jsi_Value *value, int arrayindex);
extern void jsi_ValueObjSet(Jsi_Interp *interp, Jsi_Value *target, const char *key, Jsi_Value *value, int flags, int isstrkey);
extern const char* jsi_ValueSubscript(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *key, Jsi_Value **ret, int right_val);
extern void jsi_ValueSubscriptLen(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *key, Jsi_Value **ret, int right_val);
extern Jsi_Value* jsi_ValueObjKeyAssign(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *key, Jsi_Value *value, int flag);
extern void jsi_ValueObjGetKeys(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *ret);
extern Jsi_Value* jsi_ObjArrayLookup(Jsi_Interp *interp, Jsi_Obj *obj, const char *key);
extern void jsi_ValueInsertArray(Jsi_Interp *interp, Jsi_Value *target, int index, Jsi_Value *val, int flags);
extern Jsi_Value* jsi_ProtoObjValueNew1(Jsi_Interp *interp, const char *name);
extern Jsi_Value* jsi_ProtoValueNew(Jsi_Interp *interp, const char *name, const char *parent);
extern Jsi_Value* jsi_ObjValueNew(Jsi_Interp *interp);
extern Jsi_Value* Jsi_ValueDup(Jsi_Interp *interp, Jsi_Value *v);
extern int jsi_ValueToOInt32(Jsi_Interp *interp, Jsi_Value *v);
extern int jsi_FreeOneLoadHandle(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *handle);
extern Jsi_Value* jsi_MakeFuncValue(Jsi_Interp *interp, Jsi_CmdProc *callback, const char *name);
extern Jsi_Value* jsi_MakeFuncValueSpec(Jsi_Interp *interp, Jsi_CmdSpec *cmdSpec, void *privData);
extern int jsi_FileStatCmd(Jsi_Interp *interp, Jsi_Value *fnam, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int islstat);
extern void jsi_ValueToPrimitive(Jsi_Interp *interp, Jsi_Value **vPtr);

extern Jsi_IterObj *jsi_IterObjNew(Jsi_Interp *interp, Jsi_IterProc *iterProc);
extern void jsi_IterObjFree(Jsi_IterObj *iobj);
extern Jsi_FuncObj *jsi_FuncObjNew(Jsi_Interp *interp, Jsi_Func *func);
extern void jsi_FuncObjFree(Jsi_FuncObj *fobj);
extern int jsi_ArglistFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr);
void jsi_FuncFree(Jsi_Interp *interp, Jsi_Func *func);

extern int jsi_ieee_isnormal(Jsi_Number a);
extern int jsi_ieee_isnan(Jsi_Number a);
extern int jsi_ieee_infinity(Jsi_Number a);
extern int jsi_is_integer(Jsi_Number n);
extern int jsi_is_wide(Jsi_Number n);

extern Jsi_Number jsi_ieee_makeinf(int i);
extern Jsi_Number jsi_ieee_makenan(void);

extern void jsi_num_itoa10(int value, char* str);
extern void jsi_num_uitoa10(unsigned int value, char* str);
extern void jsi_num_dtoa2(Jsi_Number value, char* str, int prec);
extern int jsi_num_isNaN(Jsi_Number value);
extern int jsi_num_isFinite(Jsi_Number value);
extern int jsi_num_isEqual(Jsi_Number n1, Jsi_Number n2);

#ifndef MEMCLEAR
#ifndef JSI_MEM_DEBUG
#define MEMCLEAR(ptr)
#else
#define MEMCLEAR(ptr) memset(ptr, 0, sizeof(*ptr)) /* To aid debugging memory.*/
#endif
#endif

#define MAX_SUBREGEX    256
#define HAVE_LONG_LONG
#define UCHAR(s) (unsigned char)(s)
#define StrnCpy(d,s) strncpy(d,s,sizeof(d)), d[sizeof(d)-1] = 0
extern char* jsi_SubstrDup(const char *a, int start, int len);
extern int jsi_typeGet(Jsi_Interp *interp , const char *tname);
extern const char *jsi_typeName(Jsi_Interp *interp, int typ, Jsi_DString *dStr);
extern int jsi_ArgTypeCheck(Jsi_Interp *interp, int typ, Jsi_Value *arg, const char *p1, const char *p2, int index, Jsi_Func *func);
extern void jsi_FuncCallCheck(jsi_Pstate *p, jsi_Pline *line, int argc, int isNew, const char *name, const char *namePre);
extern int jsi_RunFuncCallCheck(Jsi_Interp *interp, Jsi_Func *func, int argc, const char *name, jsi_Pline *line);
extern Jsi_ScopeStrs *jsi_ArgsOptAdd(jsi_Pstate *pstate, Jsi_ScopeStrs *a);
extern Jsi_ScopeStrs *jsi_argInsert(jsi_Pstate *pstate, Jsi_ScopeStrs *a, const char *name, Jsi_Value *defValue);
extern Jsi_ScopeStrs* jsi_ParseArgStr(Jsi_Interp *interp, const char *argStr);

#ifdef JSI_MEM_DEBUG
#define jsi_ValueDebugUpdate(interp, v, tbl, file, line, func) jsi_ValueDebugUpdate_(interp, &v->VD, v, interp->dbPtr->tbl, file, line, func)
#define jsi_ValueDebugLabel(v,l1,l2) jsi_ValueDebugLabel_(&v->VD,l1,l2)

#define Jsi_ValueNew(interp) jsi_ValueNew(interp, __FILE__, __LINE__,__PRETTY_FUNCTION__)
#define Jsi_ValueNew1(interp) jsi_ValueNew1(interp, __FILE__, __LINE__,__PRETTY_FUNCTION__)
#define Jsi_ValueDup(interp,v) jsi_ValueDup(interp, v,__FILE__, __LINE__,__PRETTY_FUNCTION__)
#define Jsi_ObjNew(interp) jsi_ObjNew(interp, __FILE__, __LINE__,__PRETTY_FUNCTION__)

extern Jsi_Value *jsi_ValueNew(Jsi_Interp *interp, const char *fname, int line, const char *func);
extern Jsi_Value *jsi_ValueNew1(Jsi_Interp *interp, const char *fname, int line, const char *func);
extern Jsi_Value *jsi_ValueDup(Jsi_Interp *interp, Jsi_Value *ov, const char *fname, int line, const char *func);
extern Jsi_Obj *jsi_ObjNew(Jsi_Interp *interp, const char *fname, int line, const char *func);
extern void jsi_ValueDebugLabel_(jsi_ValueDebug *vd, const char *l1, const char *l2);
extern void jsi_ValueDebugUpdate_(Jsi_Interp *interp, jsi_ValueDebug *vd, void *v, Jsi_Hash* tbl, const char *fname, int line, const char *func);
extern void jsi_DebugValue(Jsi_Value* v, const char *reason, uint idx, Jsi_Interp *interp);
extern void jsi_DebugObj(Jsi_Obj* o, const char *reason, uint idx, Jsi_Interp *interp);

#define jsi_DebugValueCallIdx() ++interp->dbPtr->memDebugCallIdx
#define VALINIT { __VALSIG__ .vt=JSI_VT_UNDEF, .refCnt=1, .f={JSI_OM_ISSTATIC}, .VD={.fname=__FILE__, .line=__LINE__,.func=__PRETTY_FUNCTION__}  }
#else
#define VALINIT { __VALSIG__ .vt=JSI_VT_UNDEF, .refCnt=1, .f={JSI_OM_ISSTATIC}  }
#define jsi_ValueDebugUpdate(interp, vd, v, tbl, file, line, func)
#define jsi_ValueDebugLabel(v,l1,l2)
#define jsi_DebugValue(v,r,i,t)
#define jsi_DebugObj(o,r,i,t)
#define jsi_DebugValueCallIdx() 0
#define jsi_ValueDebugLabel_(v,l1,l2)
#endif

#define DECL_VALINIT(n) Jsi_Value n = VALINIT

extern void jsi_FilesysDone(Jsi_Interp *interp);
extern int Jsi_DoneZvfs(Jsi_Interp *interp);
void jsi_TraceFuncCall(Jsi_Interp *interp, Jsi_Func *func, OpCode *iPtr, Jsi_Value* args, Jsi_Value *scope);

struct Jsi_Stubs;
extern struct Jsi_Stubs *jsiStubsTblPtr;
extern char *jsi_execName;

#endif /* __JSIINT_H__ */
