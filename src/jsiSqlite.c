#define JSI_SQLITE_DB_VFS "SQLITE_DB_VFS"

#ifndef JSI_AMALGAMATION
#ifdef JSI_MEM_DEBUG
#include "jsiInt.h"
#else
#include "jsi.h"
#ifdef JSI_SQLITE_SHARED
JSI_EXTENSION_INI
#endif
#endif
#endif

typedef struct jsi_DbVfs {
    int sig; 
    int (*dbcQuery)(Jsi_Db *jdb, Jsi_CDataDb *dbc, const char *query);
    void *(*dbHandle)(Jsi_Interp *interp, Jsi_Db* jdb);
    Jsi_Db* (*dbNew)(const char *zFile, int inFlags /* JSI_DBI_* */);
} jsi_DbVfs;

jsi_DbVfs SqliteDbVfs = {
    0x1234, &Jsi_DbQuery, &Jsi_DbHandle, Jsi_DbNew
};


#if JSI__SQLITE==1
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** A JSI (Javascript) Interface to SQLite.
*/

typedef enum { SQLITE_SIG_DB = 0xbeefdead, SQLITE_SIG_FUNC, SQLITE_SIG_EXEC, SQLITE_SIG_STMT } Sqlite_Sig;

#define SQLSIGASSERT(s,n) assert(s->sig == SQLITE_SIG_##n)
#ifndef _JSI_MEMCLEAR
#ifndef NDEBUG
#define _JSI_MEMCLEAR(s) memset(s, 0, sizeof(*s));
#else
#define _JSI_MEMCLEAR(s)
#endif
#endif
#ifndef JSI_DB_DSTRING_SIZE
#define JSI_DB_DSTRING_SIZE 2000
#endif

#ifndef JSI_DBQUERY_BEGIN_STR
#define JSI_DBQUERY_BEGIN_STR "BEGIN;"
#endif
#ifndef JSI_DBQUERY_ROLLBACK_STR
#define JSI_DBQUERY_ROLLBACK_STR "ROLLBACK;"
#endif
#ifndef JSI_DBQUERY_COMMIT_STR
#define JSI_DBQUERY_COMMIT_STR "COMMIT;"
#endif

#include <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>

#ifndef SQLITE_AMALGAMATION
#include "sqlite3.h"
#endif


#ifndef NUM_PREPARED_STMTS
#define NUM_PREPARED_STMTS 100
#endif
#ifndef MAX_PREPARED_STMTS
#define MAX_PREPARED_STMTS 10000
#endif

#ifndef JSI_DBQUERY_PRINTF
#define JSI_DBQUERY_PRINTF(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#endif
/*
** When JSI uses UTF-8 and SQLite is configured to use iso8859, then we
** have to do a translation when going between the two.  Set the
** UTF_TRANSLATION_NEEDED macro to indicate that we need to do
** this translation.
*/
#if defined(JSI_UTF_MAX) && !defined(SQLITE_UTF8)
# define UTF_TRANSLATION_NEEDED 1
#endif

typedef struct db_ObjCmd {
  int activeCnt;  /* Count of active objects. */ 
  int newCnt;  /* Total number of new. */ 
} db_ObjCmd;

#ifndef JSI_LITE_ONLY
static db_ObjCmd dbObjCmd = {};

static Jsi_OptionSpec db_ObjCmd_Specs[] =
{
    JSI_OPT(INT,   db_ObjCmd, activeCnt, .help="Number of active objects"),
    JSI_OPT(INT,   db_ObjCmd, newCnt,    .help="Number of new calls"),
    JSI_OPT_END(db_ObjCmd, .help="Options for Sqlite module")
};
#endif

/*
** New SQL functions can be created as JSI scripts.  Each such function
** is described by an instance of the following structure.
*/
typedef struct SqlFunc SqlFunc;
struct SqlFunc {
    Sqlite_Sig sig;
    Jsi_Interp  *interp;    /* The JSI interpret to execute the function */
    Jsi_Value   *tocall;    /* Callee */
    char        *pScript;   /* The char* representation of the script */
    Jsi_DString dScript;
    char        *zName;     /* Name of this function */
    SqlFunc     *pNext;     /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as JSI scripts.  Each such
** function is described by an instance of the following structure.
*/
typedef struct SqlCollate SqlCollate;
struct SqlCollate {
    Sqlite_Sig sig;
    Jsi_Interp  *interp;   /* The JSI interpret to execute the function */
    Jsi_Value   *zScript;  /* The function to be run */
    Jsi_Db      *jdb;
    SqlCollate  *pNext;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
typedef struct SqlPreparedStmt SqlPreparedStmt;
struct SqlPreparedStmt {
    Sqlite_Sig sig;
    int deleting;
    SqlPreparedStmt *pNext;  /* Next in linked list */
    SqlPreparedStmt *pPrev;  /* Previous on the list */
    sqlite3_stmt    *pStmt;  /* The prepared statement */
    uint nSql;                /* chars in zSql[] */
    const char *zSql;        /* Text of the SQL statement */
    Jsi_HashEntry *entry;
    Jsi_ListEntry *elPtr;
    //int nParm;               /* Size of apParm array */
    //Jsi_Value **apParm;      /* Array of referenced object pointers */
};

#ifndef JSI_LITE_ONLY
static const char *sqexecFmtStrs[] = {
    "rows", "arrays", "array1d", "list", "column", "json",
    "json2", "html", "csv", "insert", "line", "tabs", "none", NULL
};
static const char *mtxStrs[] = { "default", "none", "full", NULL };
static const char *trcModeStrs[] = {"eval", "delete", "prepare", "step", NULL}; // Bit-set packed into an int.
static const char *dbTypeChkStrs[] = { "convert", "warn", "error", "disable", NULL };
static const char *objSqlModeStrs[] = { "getSql", "noTypes", "noDefaults", "nullDefaults", NULL };
#endif

enum {OBJMODE_SQLONLY=0x1, OBJMODE_NOTYPES=0x2, OBJMODE_NODEFAULTS=0x4, OBJMODE_NULLDEFAULTS=0x8};
enum {TMODE_EVAL=0x1, TMODE_DELETE=0x2, TMODE_PREPARE=0x4, TMODE_STEP=0x4};
typedef enum { MUTEX_DEFAULT, MUTEX_NONE, MUTEX_FULL } Mutex_Type;
typedef enum { dbTypeCheck_Cast, dbTypeCheck_Warn, dbTypeCheck_Error, dbTypeCheck_None } dbTypeCheck_Mode;
typedef enum {
    _JSI_EF_ROWS, _JSI_EF_ARRAYS, _JSI_EF_ARRAY1D, _JSI_EF_LIST, _JSI_EF_COLUMN, _JSI_EF_JSON,
    _JSI_EF_JSON2, _JSI_EF_HTML, _JSI_EF_CSV, _JSI_EF_INSERT, _JSI_EF_LINE, _JSI_EF_TABS, _JSI_EF_NONE
} Output_Mode;

typedef struct QueryOpts {
    Sqlite_Sig sig;
    Jsi_Value *callback, *values;
    int limit, objOpts;
    Output_Mode mode;
    dbTypeCheck_Mode typeCheck;
    bool mapundef, nocache, headers, retChanged, echo;
    const char *separator;
    const char *nullvalue;
    const char *table;
    const char *cdata; // Name of C data array to use for query.
    const char *objName;
    Jsi_Value *width;
} QueryOpts;


/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite JSI interface.
*/
typedef struct Jsi_Db {
    Sqlite_Sig sig;
    sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
    Jsi_Interp *interp;        /* The interpreter used for this database */
    db_ObjCmd *_;              // Module data.
    Jsi_Value *onBusy;               /* The busy callback routine */
    Jsi_Value *onCommit;             /* The commit hook callback routine */
    Jsi_Value *onTrace;              /* The trace callback routine */
    Jsi_Value *onProfile;            /* The profile callback routine */
    Jsi_Value *onProgress;           /* The progress callback routine */
    Jsi_Value *onAuth;               /* The authorization callback routine */
    Jsi_Value *onUpdate;      /* Update hook script (if any) */
    Jsi_Value *onRollback;    /* Rollback hook script (if any) */
    Jsi_Value *onWalHook;        /* Wal hook script (if any) */
    Jsi_Value *onNeedCollate;   /* Collation needed script */
    Jsi_Value *onUnlock;    /* Unlock notify script (if any) */
    Jsi_Value *udata;
    int disableAuth;           /* Disable the authorizer if it exists */
    uint progressSteps;
    SqlFunc *pFunc;            /* List of SQL functions */
    SqlCollate *pCollate;      /* List of SQL collation functions */
    int rc;                    /* Return code of most recent sqlite3_exec() */
    Jsi_Hash *stmtHash;        /* Hash table for statements. */
    Jsi_List *stmtCache;
    int stmtCacheMax;               /* The next maximum number of stmtList */
    int stmtCacheCnt;                 /* Number of statements in stmtList */
    /*IncrblobChannel *pIncrblob; * Linked list of open incrblob channels */
    Jsi_Hash *strKeyTbl;       /* Used with JSI_LITE_ONLY */
    bool noJsonConv;
    bool bindWarn;
    bool forceInt;
    bool readonly;
    bool noCreate;
    bool noConfig;
    bool load;
    bool echo;
    int timeout;
    int stepCnt, sortCnt;          /* Statistics for most recent operation */
    int nTransaction;          /* Number of nested [transaction] methods */
    int errCnt;               /* Count of errors. */
    Jsi_Value *vfs;
    int hasOpts;
    Jsi_Obj *fobj;
    QueryOpts queryOpts, *optPtr;
    int objId;
    Mutex_Type mutex;
    uint64_t lastInsertId;
    int debug, changeCnt, changeCntAll, errorCode;
    Jsi_Value *version;
    Jsi_DString name;
    Jsi_Hash *typeNameHash;
} Jsi_Db;

static const int jsi_DbPkgVersion = 2;
/*
** Structure used with dbEvalXXX() functions:
**
**   dbEvalInit(interp,)
**   dbEvalStep()
**   dbEvalFinalize()
**   dbEvalRowInfo()
**   dbEvalColumnValue()
*/
#define SQL_MAX_STATIC_TYPES 100
typedef struct DbEvalContext {
    Jsi_Db *jdb;                /* Database handle */
    Jsi_DString *dSql;               /* Object holding string zSql */
    const char *zSql;               /* Remaining SQL to execute */
    SqlPreparedStmt *pPreStmt;      /* Current statement */
    int nCol;                       /* Number of columns returned by pStmt */
    char **apColName;             /* Array of column names */
    int *apColType;
    char staticColNames[JSI_BUFSIZ];  /* Attempt to avoid mallocing space for name storage. */
    int staticColTypes[SQL_MAX_STATIC_TYPES];
    Jsi_Value *tocall;
    Jsi_Value *ret;
    /*OBS */
    Jsi_Value *pArray;              /* Name of array variable */
    Jsi_Value *pValVar;             /* Name of list for values. */
    int nocache;
} DbEvalContext;

#ifndef JSI_LITE_ONLY

static Jsi_RC dbIsNumArray(Jsi_Interp *interp, Jsi_Value *value, Jsi_OptionSpec* spec, void *record);


static Jsi_OptionSpec ExecFmtOptions[] =
{
    JSI_OPT(FUNC,   QueryOpts, callback,    .help="Function to call with each row result", .flags=0, .custom=0, .data=(void*)"values:object" ),
    JSI_OPT(STRKEY, QueryOpts, cdata,       .help="Name of Cdata array object to use"),
    JSI_OPT(BOOL,   QueryOpts, echo,        .help="Output query string to log"),
    JSI_OPT(BOOL,   QueryOpts, headers,     .help="First row returned contains column labels"),
    JSI_OPT(INT,    QueryOpts, limit,       .help="Maximum number of returned values"),
    JSI_OPT(BOOL,   QueryOpts, mapundef,    .help="In variable bind, map an 'undefined' var to null"),
    JSI_OPT(CUSTOM, QueryOpts, mode,        .help="Set output mode of returned data", .flags=0, .custom=Jsi_Opt_SwitchEnum,  .data=(void*)sqexecFmtStrs),
    JSI_OPT(BOOL,   QueryOpts, nocache,     .help="Disable query cache"),
    JSI_OPT(STRKEY, QueryOpts, nullvalue,   .help="Null string output (for non js/json mode)"),
    JSI_OPT(STRKEY, QueryOpts, objName,     .help="Object var name for CREATE/INSERT: replaces %s with fields in query" ),
    JSI_OPT(CUSTOM, QueryOpts, objOpts,     .help="Options for objName", .flags=0,  .custom=Jsi_Opt_SwitchBitset,  .data=objSqlModeStrs),
    JSI_OPT(BOOL,   QueryOpts, retChanged,  .help="Query returns value of sqlite3_changed()"),
    JSI_OPT(STRKEY, QueryOpts, separator,   .help="Separator string (for csv and text mode)"),
    JSI_OPT(CUSTOM, QueryOpts, typeCheck,   .help="Type check mode (warn)", .flags=0, .custom=Jsi_Opt_SwitchEnum, .data=(void*)dbTypeChkStrs),
    JSI_OPT(STRKEY, QueryOpts, table,       .help="Table name for mode=insert"),
    JSI_OPT(ARRAY,  QueryOpts, values,      .help="Values for ? bind parameters" ),
    JSI_OPT(CUSTOM, QueryOpts, width,       .help="In column mode, set column widths", .flags=0, .custom=Jsi_Opt_SwitchValueVerify, .data=(void*)dbIsNumArray),
    JSI_OPT_END(QueryOpts, .help="Options for query()")
};

#ifndef jsi_IIOF
#define jsi_IIOF .flags=JSI_OPT_INIT_ONLY
#define jsi_IIRO .flags=JSI_OPT_READ_ONLY
#endif
static Jsi_OptionSpec SqlOptions[] =
{
    JSI_OPT(BOOL,   Jsi_Db, bindWarn,   .help="Treat failed variable binds as a warning", jsi_IIOF),
    JSI_OPT(INT,    Jsi_Db, changeCnt,  .help="The number of rows modified, inserted, or deleted by last command"),
    JSI_OPT(INT,    Jsi_Db, changeCntAll,.help="Total number of rows modified, inserted, or deleted since db opened"),
    JSI_OPT(CUSTOM, Jsi_Db, debug,      .help="Enable debug trace for various operations", .flags=0,  .custom=Jsi_Opt_SwitchBitset,  .data=trcModeStrs),
    JSI_OPT(BOOL,   Jsi_Db, echo,       .help="Output query/eval string to log"),
    JSI_OPT(INT,    Jsi_Db, errCnt,     .help="Count of errors in script callbacks", jsi_IIRO),
    JSI_OPT(INT,    Jsi_Db, errorCode,  .help="Numeric error code returned by the most recent call to sqlite3_exec"),
    JSI_OPT(BOOL,   Jsi_Db, forceInt,   .help="Bind float as int if possible"),
    JSI_OPT(BOOL,   Jsi_Db, noJsonConv, .help="Do not JSON auto-convert array and object in CHARJSON columns" ),
    JSI_OPT(UINT64, Jsi_Db, lastInsertId,.help="The rowid of last insert"),
    JSI_OPT(BOOL,   Jsi_Db, load,       .help="Extensions can be loaded" ),
    JSI_OPT(CUSTOM, Jsi_Db, mutex,      .help="Mutex type to use", jsi_IIOF, .custom=Jsi_Opt_SwitchEnum, .data=mtxStrs),
    JSI_OPT(DSTRING,Jsi_Db, name,       .help="The dbname to use instead of 'main'", jsi_IIOF),
    JSI_OPT(BOOL,   Jsi_Db, noConfig,   .help="Disable use of Sqlite.conf to change options after create", jsi_IIOF),
    JSI_OPT(BOOL,   Jsi_Db, noCreate,   .help="Database is must already exist (false)", jsi_IIOF),
    JSI_OPT(FUNC,   Jsi_Db, onAuth,     .help="Function to call for auth", .flags=0, .custom=0, .data=(void*)"db:userobj, code:string, descr1:string, decr2:string, dbname:string, trigname:string"),
    JSI_OPT(FUNC,   Jsi_Db, onBusy,     .help="Function to call when busy", .flags=0, .custom=0, .data=(void*)"db:userobj, tries:number"),
    JSI_OPT(FUNC,   Jsi_Db, onCommit,   .help="Function to call on commit", .flags=0, .custom=0, .data=(void*)"db:userobj"),
    JSI_OPT(FUNC,   Jsi_Db, onNeedCollate,.help="Function to call for collation", .flags=0, .custom=0, .data=(void*)"db:userobj, name:string"),
    JSI_OPT(FUNC,   Jsi_Db, onProfile,  .help="Function to call for profile", .flags=0, .custom=0, .data=(void*)"db:userobj, sql:string, time:number"),
    JSI_OPT(FUNC,   Jsi_Db, onProgress, .help="Function to call for progress: progressSteps must be >0", .flags=0, .custom=0, .data=(void*)"db:userobj"),
    JSI_OPT(FUNC,   Jsi_Db, onRollback, .help="Function to call for rollback", .flags=0, .custom=0, .data=(void*)"db:userobj"),
    JSI_OPT(FUNC,   Jsi_Db, onTrace,    .help="Function to call for trace", .flags=0, .custom=0, .data=(void*)"db:userobj, sql:string"),
    JSI_OPT(FUNC,   Jsi_Db, onUpdate,   .help="Function to call for update", .flags=0, .custom=0, .data=(void*)"db:userobj, op:string, dbname:string, table:string, rowid:number"),
    JSI_OPT(FUNC,   Jsi_Db, onWalHook,  .help="Function to call for WAL", .flags=0, .custom=0, .data=(void*)"db:userobj, dbname:string, entry:number"),
    JSI_OPT(UINT,   Jsi_Db, progressSteps,.help="Number of steps between calling onProgress: 0 is disabled", ),
    JSI_OPT(CUSTOM, Jsi_Db, queryOpts,  .help="Default options for to use with query()", .flags=0, .custom=Jsi_Opt_SwitchSuboption, .data=ExecFmtOptions),
    JSI_OPT(BOOL,   Jsi_Db, readonly,   .help="Database opened in readonly mode", jsi_IIOF),
    JSI_OPT(INT,    Jsi_Db, sortCnt,    .help="Number of sorts in most recent operation", jsi_IIRO),
    JSI_OPT(INT,    Jsi_Db, stepCnt,    .help="Number of steps in most recent operation", jsi_IIRO),
    JSI_OPT(INT,    Jsi_Db, stmtCacheCnt,.help="Current size of compiled statement cache", jsi_IIRO),
    JSI_OPT(INT,    Jsi_Db, stmtCacheMax,.help="Max cache size for compiled statements"),
    JSI_OPT(INT,    Jsi_Db, timeout,    .help="Amount of time to wait when file is locked, in ms"),
    JSI_OPT(OBJ,    Jsi_Db, udata,      .help="User data" ),
    JSI_OPT(OBJ,    Jsi_Db, version,    .help="Sqlite version info"),
    JSI_OPT(INT,    Jsi_Db, timeout,    .help="Amount of time to wait when file is locked, in ms"),
    JSI_OPT(STRING, Jsi_Db, vfs,        .help="VFS to use", jsi_IIOF),
    JSI_OPT_END(Jsi_Db, .help="Options for source command")
};


#endif

void dbTypeNameHashInit(Jsi_Db *jdb) {
    Jsi_Interp *interp = jdb->interp;
    Jsi_Hash *hPtr = jdb->typeNameHash = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    Jsi_HashSet(hPtr, (void*)"blob", (void*)JSI_OPTION_STRBUF);
    Jsi_HashSet(hPtr, (void*)"string", (void*)JSI_OPTION_STRING);
    Jsi_HashSet(hPtr, (void*)"double", (void*)JSI_OPTION_DOUBLE);
    Jsi_HashSet(hPtr, (void*)"integer", (void*)JSI_OPTION_INT64);
    Jsi_HashSet(hPtr, (void*)"bool", (void*)JSI_OPTION_BOOL);
    Jsi_HashSet(hPtr, (void*)"time_d", (void*)JSI_OPTION_TIME_D);
    Jsi_HashSet(hPtr, (void*)"time_w", (void*)JSI_OPTION_TIME_W);
    Jsi_HashSet(hPtr, (void*)"time_t", (void*)JSI_OPTION_TIME_T);
    Jsi_HashSet(hPtr, (void*)"date", (void*)JSI_OPTION_TIME_W);
    Jsi_HashSet(hPtr, (void*)"time", (void*)JSI_OPTION_TIME_W);
    Jsi_HashSet(hPtr, (void*)"datetime", (void*)JSI_OPTION_TIME_W);
}

#define SQLITE_OMIT_INCRBLOB

// Return 1 if ok, else return 0 and set erc to -1 or -2 for timeout.
static int dbExecCmd(Jsi_Db *jdb, const char *zQuery, int *erc)
{
    int rc = sqlite3_exec(jdb->db, zQuery, 0, 0, 0);
    if (rc == SQLITE_BUSY) {
        if (erc) *erc = -2;
    } else if (rc != SQLITE_OK) {
        if (erc) *erc = -1;
    } else
        return 1;
    return 0;
}

static void dbEvalRowInfo(
    DbEvalContext *p,               /* Evaluation context */
    int *pnCol,                     /* OUT: Number of column names */
    char ***papColName,           /* OUT: Array of column names */
    int **papColType
) {
    /* Compute column names */
    // Jsi_Interp *interp = p->jdb->interp;

    if( 0==p->apColName ) {
        sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
        uint i;                        /* Iterator variable */
        uint nCol;                     /* Number of columns returned by pStmt */
        char **apColName = 0;      /* Array of column names */
        int *apColType = 0;
        const char *zColName;         /* Column name */
        int numRid = 0;               /* Number of times rowid seen. */

        p->nCol = nCol = sqlite3_column_count(pStmt);
        if( nCol>0 && (papColName || p->pArray) ) {
            uint cnLen = sizeof(char*)*nCol, cnStart = cnLen;
            for(i=0; i<nCol && cnLen<sizeof(p->staticColNames); i++)
                cnLen += Jsi_Strlen(sqlite3_column_name(pStmt,i))+1;
            if (cnLen>=sizeof(p->staticColNames)) {
                apColName = (char**)Jsi_Calloc(nCol, sizeof(char*) );
                cnStart = 0;
            } else {
                apColName = (char**)p->staticColNames;
            }
            if (papColType) {
                if (nCol < SQL_MAX_STATIC_TYPES)
                    apColType = p->staticColTypes;
                else
                    apColType = (int*)Jsi_Calloc(nCol, sizeof(int));
            }
            for(i=0; i<nCol; i++) {
                zColName = sqlite3_column_name(pStmt,i);
                if (cnStart==0)
                    apColName[i] = Jsi_Strdup(zColName);
                else {
                    apColName[i] = p->staticColNames+cnStart;
                    Jsi_Strcpy(apColName[i], zColName);
                    cnStart += Jsi_Strlen(zColName)+1;
                }
                if (apColType)
                    apColType[i] = sqlite3_column_type(pStmt,i);
                /* Check if rowid appears first, and more than once. */
                if ((i == 0 || numRid>0) &&
                        (zColName[0] == 'r' && Jsi_Strcmp(zColName,"rowid") == 0)) {
                    numRid++;
                }
            }
            /* Change first rowid to oid. */
            if (numRid > 1) {
                if (apColName != (char**)p->staticColNames) {
                    Jsi_Free(apColName[0]);
                    apColName[0] = Jsi_Strdup("oid");
                } else {
                    Jsi_Strcpy(apColName[0], "oid");
                }
            }
            p->apColName = apColName;
            p->apColType = apColType;
        }
    }
    if( papColName ) {
        *papColName = p->apColName;
    }
    if( papColType ) {
        *papColType = p->apColType;
    }
    if( pnCol ) {
        *pnCol = p->nCol;
    }
}

#ifndef JSI_LITE_ONLY
static Jsi_RC dbPrepareAndBind( Jsi_Db *jdb, char const *zIn, char const **pzOut,  SqlPreparedStmt **ppPreStmt );
#endif
static void dbReleaseColumnNames(DbEvalContext *p);
static void dbReleaseStmt( Jsi_Db *jdb, SqlPreparedStmt *pPreStmt, int discard );


/* Step statement. Return JSI_OK if there is a ROW result, JSI_BREAK if done, else JSI_ERROR. */
static Jsi_RC dbEvalStepSub(DbEvalContext *p, int release, int *erc) {
    int rcs;
    Jsi_Db *jdb = p->jdb;
    Jsi_Interp *interp = jdb->interp;
    JSI_NOTUSED(interp);
    SqlPreparedStmt *pPreStmt = p->pPreStmt;
    SQLSIGASSERT(pPreStmt, STMT);
    sqlite3_stmt *pStmt = pPreStmt->pStmt;

    if (jdb->debug & TMODE_STEP)
        JSI_DBQUERY_PRINTF( "DEBUG: step: %s\n", pPreStmt->zSql);
    rcs = sqlite3_step(pStmt);
    if( rcs==SQLITE_BUSY ) {
        if (erc) *erc = -2;
        return JSI_ERROR;
    }
    if( rcs==SQLITE_ROW ) {
        return JSI_OK;
    }
    if( p->pArray ) {
        dbEvalRowInfo(p, 0, 0, 0);
    }
    rcs = sqlite3_reset(pStmt);

    jdb->stepCnt = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
    jdb->sortCnt = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
    if (release==0 && rcs==SQLITE_OK)
        return JSI_BREAK;
    dbReleaseColumnNames(p);
    p->pPreStmt = 0;

    if( rcs!=SQLITE_OK ) {
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        Jsi_LogError("%s", sqlite3_errmsg(jdb->db));
        dbReleaseStmt(jdb, pPreStmt, 1);
        return JSI_ERROR;
    } else {
        dbReleaseStmt(jdb, pPreStmt, p->nocache);
    }
    return JSI_BREAK;
}

static Jsi_RC dbEvalInit(
    Jsi_Interp *interp,
    DbEvalContext *p,               /* Pointer to structure to initialize */
    Jsi_Db *jdb,                  /* Database handle */
    const char* zSql,                /* Value containing SQL script */
    Jsi_DString *dStr,
    Jsi_Obj *pArray,                /* Name of Jsi array to set (*) element of */
    Jsi_Obj *pValVar                  /* Name element in array for list. */
) {
    p->dSql = dStr;
    p->zSql = Jsi_DSAppend(p->dSql, zSql?zSql:"", NULL);
    p->jdb = jdb;
    return JSI_OK;
}

static void dbPrepStmtFree( Jsi_Db *jdb, SqlPreparedStmt *prep)
{
    if (prep->deleting)
        return;
    prep->deleting = 1;
    if (prep->pStmt)
        sqlite3_finalize( prep->pStmt );
    if (prep->entry) {
        Jsi_HashEntry *hPtr = prep->entry;
        prep->entry = NULL;
        Jsi_HashEntryDelete(hPtr);
    }
    if (prep->elPtr)
        Jsi_ListEntryDelete(prep->elPtr);
    Jsi_Free( (char*)prep );
    jdb->stmtCacheCnt--;
}

/*
** Finalize and free a list of prepared statements
*/

static void dbPrepStmtLimit( Jsi_Db *jdb)
{
    while(jdb->stmtCacheCnt>jdb->stmtCacheMax ) {
        Jsi_ListEntry *l = Jsi_ListPopBack(jdb->stmtCache);
        dbPrepStmtFree(jdb, (SqlPreparedStmt*)Jsi_ListValueGet(l));
        jdb->stmtCacheCnt = Jsi_ListSize(jdb->stmtCache);
    }
}


static Jsi_RC dbStmtFreeProc(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *value) {
    Jsi_Db *jdb = (Jsi_Db*)interp;
    Jsi_ListEntry *l = (Jsi_ListEntry*)hPtr;
    SqlPreparedStmt *prep = (SqlPreparedStmt *)Jsi_ListValueGet(l);
    prep->elPtr = NULL;
    dbPrepStmtFree(jdb, prep);
    return JSI_OK;
}

#ifndef JSI_LITE_ONLY

/*
** Finalize and free a list of prepared statements
*/
static void dbFlushStmtCache( Jsi_Db *jdb ) {
    Jsi_ListClear(jdb->stmtCache);
    jdb->stmtCacheCnt = 0;
}

#endif

/*
** Release a statement reference obtained by calling dbPrepareAndBind().
** There should be exactly one call to this function for each call to
** dbPrepareAndBind().
**
** If the discard parameter is non-zero, then the statement is deleted
** immediately. Otherwise it is added to the LRU list and may be returned
** by a subsequent call to dbPrepareAndBind().
*/
static void dbReleaseStmt(
    Jsi_Db *jdb,                  /* Database handle */
    SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
    int discard                     /* True to delete (not cache) the pPreStmt */
) {
    //int i;
    //Jsi_Interp *interp = jdb->interp;

    /* Free the bound string and blob parameters */
    /*for(i=0; i<pPreStmt->nParm; i++) {
        Jsi_DecrRefCount(interp, pPreStmt->apParm[i]);
    }*/
    //pPreStmt->nParm = 0;

    if( jdb->stmtCacheMax<=0 || discard ) {
        /* If the cache is turned off, deallocated the statement */
        dbPrepStmtFree(jdb, pPreStmt);
    } else {
        /* Add the prepared statement to the beginning of the cache list, then limit. */
        if (!pPreStmt->elPtr)
            pPreStmt->elPtr = Jsi_ListPushFrontNew(jdb->stmtCache, pPreStmt);
        else
            Jsi_ListPushFront(jdb->stmtCache, pPreStmt->elPtr);
        dbPrepStmtLimit(jdb);
        jdb->stmtCacheCnt = Jsi_ListSize(jdb->stmtCache);
    }
}

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
static void dbReleaseColumnNames(DbEvalContext *p) {
    //Jsi_Interp *interp = p->jdb->interp;

    if( p->apColName && p->apColName != (char**)p->staticColNames) {
        int i;
        for(i=0; i<p->nCol; i++) {
            Jsi_Free(p->apColName[i]);
        }
        Jsi_Free((char *)p->apColName);
    }
    if( p->apColType && p->apColType != p->staticColTypes) {
        Jsi_Free((char *)p->apColType);
    }
    p->apColName = NULL;
    p->apColType = NULL;
    p->nCol = 0;
}

/*
** Search the cache for a prepared-statement object that implements the
** first SQL statement in the buffer pointed to by parameter zIn. If
** no such prepared-statement can be found, allocate and prepare a new
** one. In either case, bind the current values of the relevant Jsi
** variables to any $var, :var or @var variables in the statement. Before
** returning, set *ppPreStmt to point to the prepared-statement object.
**
** Output parameter *pzOut is set to point to the next SQL statement in
** buffer zIn, or to the '\0' byte at the end of zIn if there is no
** next statement.
**
** If successful, JSI_OK is returned. Otherwise, JSI_ERROR is returned
** and an error message loaded into interpreter jdb->interp.
*/
static Jsi_RC dbPrepareStmt(
    Jsi_Db *jdb,                  /* Database object */
    char const *zIn,                /* SQL to compile */
    char const **pzOut,             /* OUT: Pointer to next SQL statement */
    SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
) {
    const char *zSql = zIn;         /* Pointer to first SQL statement in zIn */
    sqlite3_stmt *pStmt;            /* Prepared statement object */
    SqlPreparedStmt *pPreStmt = 0;  /* Pointer to cached statement */
   // int nSql;                       /* Length of zSql in bytes */
    //int nVar;                       /* Number of variables in statement */
    //int iParm = 0;                  /* Next free entry in apParm */
    Jsi_RC rc = JSI_OK;
    Jsi_Interp *interp = jdb->interp;
    JSI_NOTUSED(interp);

    *ppPreStmt = 0;

    /* Trim spaces from the start of zSql and calculate the remaining length. */
    while( isspace(zSql[0]) ) {
        zSql++;
    }
    //nSql = Jsi_Strlen(zSql);
    Jsi_HashEntry *entry = Jsi_HashEntryFind(jdb->stmtHash, zSql);
    if (entry && ((pPreStmt = (SqlPreparedStmt*)Jsi_HashValueGet(entry)))) {
        
        if (jdb->debug & TMODE_PREPARE)
            JSI_DBQUERY_PRINTF( "DEBUG: prepare cache-hit: %s\n", zSql);
        pStmt = pPreStmt->pStmt;
        *pzOut = &zSql[pPreStmt->nSql];

        /* When a prepared statement is found, unlink it from the
        ** cache list.  It will later be added back to the beginning
        ** of the cache list in order to implement LRU replacement.
        */
        Jsi_ListPop(jdb->stmtCache, pPreStmt->elPtr);
        jdb->stmtCacheCnt = Jsi_ListSize(jdb->stmtCache);

    }

    /* If no prepared statement was found. Compile the SQL text. Also allocate
    ** a new SqlPreparedStmt structure.  */
    if( pPreStmt==0 ) {
        int nByte;

        if( SQLITE_OK!=sqlite3_prepare_v2(jdb->db, zSql, -1, &pStmt, pzOut) )
        
            return Jsi_LogError("PREPARE: %s", sqlite3_errmsg(jdb->db));
        if( pStmt==0 ) {
            if( SQLITE_OK!=sqlite3_errcode(jdb->db) ) {
                /* A compile-time error in the statement. */
                Jsi_LogError("PREP: %s", sqlite3_errmsg(jdb->db));
                return JSI_ERROR;
            } else {
                /* The statement was a no-op.  Continue to the next statement
                ** in the SQL string.
                */
                return JSI_OK;
            }
        }

        if (jdb->debug & TMODE_PREPARE)
            JSI_DBQUERY_PRINTF( "DEBUG: prepare new: %s\n", zSql);
        assert( pPreStmt==0 );
        //nVar = sqlite3_bind_parameter_count(pStmt);
        jdb->stmtCacheCnt++;
        nByte = sizeof(SqlPreparedStmt); // + nVar*sizeof(Jsi_Obj *);
        pPreStmt = (SqlPreparedStmt*)Jsi_Calloc(1, nByte);
        pPreStmt->sig = SQLITE_SIG_STMT;

        pPreStmt->pStmt = pStmt;
        pPreStmt->nSql = (*pzOut - zSql);
        pPreStmt->zSql = sqlite3_sql(pStmt);
        bool isNew = 0;
        pPreStmt->entry = Jsi_HashEntryNew(jdb->stmtHash, zSql, &isNew);
        if (!isNew)
            JSI_DBQUERY_PRINTF( "sqlite dup stmt entry");
        Jsi_HashValueSet(pPreStmt->entry, pPreStmt);
            
        //pPreStmt->apParm = (Jsi_Value **)&pPreStmt[1];
    }
    assert( pPreStmt );
    assert( Jsi_Strlen(pPreStmt->zSql)==pPreStmt->nSql );
    assert( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );
    *ppPreStmt = pPreStmt;
    //pPreStmt->nParm = iParm; 
    return rc;
}


#ifndef JSI_LITE_ONLY

/*
** Return one of JSI_OK, JSI_BREAK or JSI_ERROR. If JSI_ERROR is
** returned, then an error message is stored in the interpreter before
** returning.
**
** A return value of JSI_OK means there is a row of data available. The
** data may be accessed using dbEvalRowInfo() and dbEvalColumnValue(). This
** is analogous to a return of SQLITE_ROW from sqlite3_step(). If JSI_BREAK
** is returned, then the SQL script has finished executing and there are
** no further rows available. This is similar to SQLITE_DONE.
*/
static Jsi_RC dbEvalStep(DbEvalContext *p) {
    while( p->zSql[0] || p->pPreStmt ) {
        Jsi_RC rc;
        if( p->pPreStmt==0 ) {
            rc = dbPrepareAndBind(p->jdb, p->zSql, &p->zSql, &p->pPreStmt);
            if( rc!=JSI_OK ) return rc;
        }
        rc = dbEvalStepSub(p, 1, NULL);
        if (rc != JSI_BREAK)
            return rc;
    }
    
    /* Finished */
    return JSI_BREAK;
}

static Jsi_RC dbBindStmt(Jsi_Db *jdb, SqlPreparedStmt *prep)
{
    sqlite3_stmt *pStmt = prep->pStmt;    /* Object used to cache statement */
    Jsi_Interp *interp = jdb->interp;
    int i, btype = 0, bindArr=0, n;
    Jsi_RC rc = JSI_OK;
    Jsi_Number r;
    Jsi_Wide wv;
 
    Jsi_Value *pv = NULL, *apv = NULL;
    int nVar = sqlite3_bind_parameter_count(pStmt);
    char tname[50];
    
   /* Bind values to parameters that begin with @, $, :, or ? */
    for(i=1; i<=nVar; i++) {
        tname[0] = 0;
        int isInt = 0, isBlob = 0;
        const char *zVar = sqlite3_bind_parameter_name(pStmt, i);
        if (zVar == NULL) {
            if (!jdb->optPtr || !(apv=jdb->optPtr->values)) 
                return Jsi_LogError("? bind without values for param %d", i);
            if (!(pv =Jsi_ValueArrayIndex(interp, apv, i-1))) 
                return Jsi_LogError("array element %d missing", nVar);
        }
        else if((zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ) {
            int zvLen = Jsi_Strlen(zVar);
            char *zcp;
            if (zVar[0] =='$' && ((zcp = (char*)Jsi_Strchr(zVar,'('))) && zVar[zvLen-1] == ')')
            {
                bindArr = 1;
                Jsi_DString vStr;
                Jsi_DSInit(&vStr);
                Jsi_DSAppendLen(&vStr, zVar+1, (zcp-zVar-1));
                int slen = Jsi_Strlen(zcp);
                const char *ttp;
                if ((ttp = Jsi_Strchr(zVar,':'))) { // Extract bind-type.
                    Jsi_DString tStr = {};
                    int tlen = Jsi_Strlen(ttp+1);
                    Jsi_DSAppendLen(&tStr, ttp+1, tlen-1);
                    if (!jdb->typeNameHash)
                        dbTypeNameHashInit(jdb);
                    Jsi_HashEntry *htPtr = Jsi_HashEntryFind(jdb->typeNameHash, Jsi_DSValue(&tStr));
                    int rc = ( htPtr != NULL);
                    if (!htPtr) {
                        Jsi_DString eStr = {};
                        Jsi_HashSearch search;
                        Jsi_Interp *interp = jdb->interp;
                        int n = 0;
                        Jsi_HashEntry *hPtr;
                        for (hPtr = Jsi_HashSearchFirst(jdb->typeNameHash, &search);
                            hPtr != NULL; hPtr = Jsi_HashSearchNext(&search)) {
                            const char *key = (char*)Jsi_HashKeyGet(hPtr);
                            Jsi_DSAppend(&eStr, (n++?", ":""), key, NULL);
                        }
                        Jsi_LogWarn("bind type \"%s\" is not one of: %s", Jsi_DSValue(&tStr), Jsi_DSValue(&eStr));
                        Jsi_DSFree(&eStr);
                    }
                    Jsi_Strcpy(tname, Jsi_DSValue(&tStr));
                    Jsi_DSFree(&tStr);
                    if (!rc)
                        return JSI_ERROR;

                    btype = (uintptr_t)Jsi_HashValueGet(htPtr);
                    Jsi_DSFree(&tStr);
                    slen -= tlen;
                }

                if (isdigit(zcp[1])) {
                    Jsi_DSAppendLen(&vStr, "[", 1);
                    Jsi_DSAppendLen(&vStr, zcp+1, slen-2);
                    Jsi_DSAppendLen(&vStr, "]", 1);
                } else {
                    if (zcp[1] != '[')
                        Jsi_DSAppendLen(&vStr, ".", 1);
                    Jsi_DSAppendLen(&vStr, zcp+1, slen-2);
                }
                pv = Jsi_NameLookup(interp, Jsi_DSValue(&vStr));
                Jsi_DSFree(&vStr);
            } else
                pv = Jsi_VarLookup(interp, &zVar[1]);
        } else 
            return Jsi_LogError("can not find bind var %s", zVar);
            
        if(!pv ) {
            if (!jdb->bindWarn) {
                Jsi_LogError("unknown bind param: %s", zVar);
                rc = JSI_ERROR;
                break;
            } else
                Jsi_LogWarn("unknown bind param: %s", zVar);
        } else {
            int match = 1, cast = (jdb->optPtr->typeCheck==dbTypeCheck_Cast);
            if (btype && !Jsi_ValueIsUndef(interp, pv)) {
                switch (btype) {
                    case JSI_OPTION_STRBUF:
                        isBlob = 1;
                    case JSI_OPTION_STRING:
                        if (cast)
                            Jsi_ValueToString(interp, pv, &n);
                        else
                            match = Jsi_ValueIsString(interp, pv);
                        break;
                    case JSI_OPTION_NUMBER:
                    case JSI_OPTION_DOUBLE:
                        if (cast)
                            Jsi_ValueToNumber(interp, pv);
                        else
                            match = Jsi_ValueIsNumber(interp, pv);
                        break;
                    case JSI_OPTION_TIME_W:
                    case JSI_OPTION_TIME_T:
                    case JSI_OPTION_INT64:
                        isInt = 1;
                        if (cast)
                            Jsi_ValueToNumber(interp, pv);
                        else
                            match = Jsi_ValueIsNumber(interp, pv);
                        break;
                    case JSI_OPTION_BOOL:
                        if (cast)
                            Jsi_ValueToBool(interp, pv);
                        else
                            match = Jsi_ValueIsNumber(interp, pv);
                        break;
                    case JSI_OPTION_TIME_D:
                        if (cast)
                            Jsi_ValueToNumber(interp, pv); //TODO: do something more for dates?
                        else
                            match = Jsi_ValueIsNumber(interp, pv);
                        break;
                    default:
                        Jsi_LogBug("Unhandled bind type: %s = %d", tname, btype);
                }
                if (cast == 0 && match == 0) {
                    int ltyp = (jdb->optPtr->typeCheck==dbTypeCheck_Error?JSI_LOG_ERROR:JSI_LOG_WARN);
                    Jsi_LogMsg(interp, ltyp, "bind param \"%s\" type is not \"%s\"", zVar, tname);
                    if (ltyp == JSI_LOG_ERROR)
                        return JSI_ERROR;
                }
            }
            bool bn, isArr;
            const char *dectyp;
            if (Jsi_ValueIsBoolean(interp, pv)) {
                Jsi_GetBoolFromValue(interp, pv, &bn);
                sqlite3_bind_int(pStmt, i, bn);
            } else if (Jsi_ValueIsNumber(interp, pv)) {
                Jsi_GetNumberFromValue(interp, pv, &r);
                wv = (Jsi_Wide)r;
                if (isInt || (jdb->forceInt && (((Jsi_Number)wv)-r)==0))
                    sqlite3_bind_int64(pStmt, i,wv);
                else
                    sqlite3_bind_double(pStmt, i,(double)r);
            } else if (Jsi_ValueIsNull(interp, pv) || (Jsi_ValueIsUndef(interp, pv) && jdb->queryOpts.mapundef)) {
                sqlite3_bind_null(pStmt, i);
            } else if (Jsi_ValueIsString(interp, pv)) {
                const char *sstr = Jsi_ValueGetStringLen(interp, pv, &n);
                if (!sstr) sstr = "";
                if (isBlob)
                    sqlite3_bind_blob(pStmt, i, (char *)sstr, n, SQLITE_TRANSIENT );
                else
                    sqlite3_bind_text(pStmt, i, (char *)sstr, n, SQLITE_TRANSIENT );
            } else if (!jdb->noJsonConv && bindArr && ((isArr=Jsi_ValueIsArray(interp, pv))
                || Jsi_ValueIsObjType(interp, pv, JSI_OT_OBJECT))
                && (((dectyp = sqlite3_column_decltype(pStmt, i))==NULL) || 
                    !Jsi_Strncasecmp(dectyp,"charjson",8))) {
                    // Limitation: on INSERT can not access decltype.
                    Jsi_DString jStr = {};
                    Jsi_ValueGetDString(interp, pv, &jStr, JSI_OUTPUT_JSON|JSI_JSON_STRICT);
                    n = Jsi_DSLength(&jStr);
                    sqlite3_bind_text(pStmt, i, Jsi_DSValue(&jStr), n, SQLITE_TRANSIENT );
                    Jsi_DSFree(&jStr);
            } else {
                if (!jdb->bindWarn) {
                    Jsi_LogError("bind param must be string/number/bool/null: %s", zVar);
                    rc = JSI_ERROR;
                    break;
                } else
                    Jsi_LogWarn("bind param must be string/number/bool/null: %s", zVar);
                sqlite3_bind_null(pStmt, i);
            }

        }
    }
    return rc;
}

static Jsi_RC dbPrepareAndBind(
    Jsi_Db *jdb,                  /* Database object */
    char const *zIn,                /* SQL to compile */
    char const **pzOut,             /* OUT: Pointer to next SQL statement */
    SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
) {
    if (dbPrepareStmt(jdb, zIn, pzOut, ppPreStmt) != JSI_OK)
        return JSI_ERROR;
    return dbBindStmt(jdb, *ppPreStmt);
}
#endif

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit(interp,).
*/
static void dbEvalFinalize(DbEvalContext *p) {
//  Jsi_Interp *interp = p->jdb->interp;

    if( p->pPreStmt ) {
        sqlite3_reset(p->pPreStmt->pStmt);
        dbReleaseStmt(p->jdb, p->pPreStmt, p->nocache);
        p->pPreStmt = 0;
    }
    if (p->dSql)
        Jsi_DSFree(p->dSql);
    dbReleaseColumnNames(p);
}

static void DbClose(sqlite3 *db) {
        sqlite3_close(db);
}

#ifndef JSI_LITE_ONLY

static Jsi_RC sqliteObjFree(Jsi_Interp *interp, void *data);
static bool  sqliteObjEqual(void *data1, void *data2);
static bool  sqliteObjIsTrue(void *data);

static Jsi_RC dbIsNumArray(Jsi_Interp *interp, Jsi_Value *value, Jsi_OptionSpec* spec, void *record)
{
    if (!Jsi_ValueIsArray(interp, value)) 
        return Jsi_LogError("expected array of numbers");
    int i, argc = Jsi_ValueGetLength(interp, value);
    for (i=0; i<argc; i++) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, value, i);
        if (!Jsi_ValueIsNumber(interp, v)) 
            return Jsi_LogError("expected array of numbers");
    }
    return JSI_OK;
}


/*
** JSI calls this procedure when an sqlite3 database command is
** deleted.
*/
static void dbDeleteCmd(Jsi_Db *jdb)
{
    Jsi_Interp *interp = jdb->interp;
    if (jdb->debug & TMODE_DELETE)
        JSI_DBQUERY_PRINTF( "DEBUG: delete\n");
    dbFlushStmtCache(jdb);
    if (jdb->stmtHash)
        Jsi_HashDelete(jdb->stmtHash);
    //closeIncrblobChannels(jdb);
    if (jdb->db) {
        DbClose(jdb->db);
    }
    while( jdb->pFunc ) {
        SqlFunc *pFunc = jdb->pFunc;
        jdb->pFunc = pFunc->pNext;
        Jsi_DSFree(&pFunc->dScript);
        Jsi_DecrRefCount(interp, pFunc->tocall);
        Jsi_Free((char*)pFunc);
    }
    while( jdb->pCollate ) {
        SqlCollate *pCollate = jdb->pCollate;
        jdb->pCollate = pCollate->pNext;
        Jsi_Free((char*)pCollate);
    }

    Jsi_OptionsFree(interp, SqlOptions, jdb, 0);
    if (jdb->stmtCache)
        Jsi_ListDelete(jdb->stmtCache);
}

static int dbGetIntBool(Jsi_Interp *interp, Jsi_Value* v)
{
    if (Jsi_ValueIsNumber(interp, v)) {
        Jsi_Number d;
        Jsi_ValueGetNumber(interp, v, &d);
        return (int)d;
    }
    if (Jsi_ValueIsBoolean(interp, v)) {
        bool n;
        Jsi_ValueGetBoolean(interp, v, &n);
        return n;
    }
    return 0;
}


/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
static int dbBusyHandler(void *cd, int nTries) {
    int rc;
    Jsi_Db *jdb = (Jsi_Db*)cd;
    Jsi_Value *vpargs, *items[3] = {}, *ret;
    Jsi_Interp *interp = jdb->interp;

    items[0] = Jsi_ValueNewObj(interp, jdb->fobj);
    items[1] = Jsi_ValueMakeNumber(interp, NULL, (Jsi_Number)nTries);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 2, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew1(interp);
    rc = Jsi_FunctionInvoke(interp, jdb->onBusy, vpargs, &ret, NULL);
    if( JSI_OK!=rc ) {
        jdb->errCnt++;
        rc = 1;
    } else
        rc = dbGetIntBool(interp, ret);
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    return rc;
}

/*
** This routine is invoked as the 'progress callback' for the database.
*/
static int dbProgressHandler(void *cd) {
    Jsi_Db *jdb = (Jsi_Db*)cd;
    Jsi_Value *vpargs, *items[3] = {}, *ret;
    Jsi_Interp *interp = jdb->interp;

    items[0] = Jsi_ValueNewObj(interp, jdb->fobj);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 1, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew1(interp);
    int rc = Jsi_FunctionInvoke(interp, jdb->onProgress, vpargs, &ret, NULL);
    if( JSI_OK!=rc ) {
        jdb->errCnt++;
        rc = 1;
    } else
        rc = dbGetIntBool(interp, ret);
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    return rc;
}

/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The JSI script in jdb->onTrace is executed.
*/
static void dbTraceHandler(void *cd, const char *zSql)
{
    int rc;
    Jsi_Db *jdb = (Jsi_Db*)cd;
    Jsi_Value *vpargs, *items[2] = {}, *ret;
    Jsi_Interp *interp = jdb->interp;
    items[0] = Jsi_ValueNewObj(interp, jdb->fobj);
    items[1] = Jsi_ValueMakeStringDup(interp, NULL, zSql);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 2, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew1(interp);
    rc = Jsi_FunctionInvoke(interp, jdb->onTrace, vpargs, &ret, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    if (rc != JSI_OK)
        jdb->errCnt++;
}

/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The JSI script in jdb->onProfile is evaluated.
*/
static void dbProfileHandler(void *cd, const char *zSql, sqlite_uint64 tm) {
    int rc;
    Jsi_Db *jdb = (Jsi_Db*)cd;
    Jsi_Interp *interp = jdb->interp;
    Jsi_Value *vpargs, *items[3] = {}, *ret;

    items[0] = Jsi_ValueNewObj(interp, jdb->fobj);
    items[1] = Jsi_ValueMakeStringDup(interp, NULL, zSql);
    items[2] = Jsi_ValueMakeNumber(interp, NULL, (Jsi_Number)tm);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 3, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew1(interp);
    rc = Jsi_FunctionInvoke(interp, jdb->onProfile, vpargs, &ret, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    if (rc != JSI_OK)
        jdb->errCnt++;
}

/*
** This routine is called when a transaction is committed.  The
** JSI script in jdb->onCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
static int dbCommitHandler(void *cd) {
    int rc = 0;
    Jsi_Db *jdb = (Jsi_Db*)cd;
    Jsi_Interp *interp = jdb->interp;
    Jsi_Value *vpargs, *items[2] = {}, *ret = Jsi_ValueNew1(interp);
    
    items[0] = Jsi_ValueNewObj(interp, jdb->fobj);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 1, 0));
    Jsi_IncrRefCount(interp, vpargs);
    if( JSI_OK!=Jsi_FunctionInvoke(interp, jdb->onCommit, NULL, &ret, NULL) ) {
        jdb->errCnt++;
        rc = 1;
    } else
        rc = dbGetIntBool(interp, ret);
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    return rc;
}

/*
** This procedure handles wal_hook callbacks.
*/
static int dbWalHandler( void *cd, sqlite3 *db, const char *zDb, int nEntry ){
    int rc;
    Jsi_Db *jdb = (Jsi_Db*)cd;
    Jsi_Interp *interp = jdb->interp;
    Jsi_Value *vpargs, *items[3] = {}, *ret;

    items[0] = Jsi_ValueNewObj(interp, jdb->fobj);
    items[1] = Jsi_ValueMakeStringDup(interp, NULL, zDb);
    items[2] = Jsi_ValueMakeNumber(interp, NULL, (Jsi_Number)nEntry);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 3, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew(interp);
    rc = Jsi_FunctionInvoke(interp, jdb->onWalHook, vpargs, &ret, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    if (rc != JSI_OK) {
        jdb->errCnt++;
        rc = 1;
    } else
        rc = dbGetIntBool(jdb->interp, ret);
    Jsi_DecrRefCount(interp, ret);
    return rc;
}
 
static void dbRollbackHandler(void *cd) {
    Jsi_Db *jdb = (Jsi_Db*)cd;
    Jsi_Interp *interp = jdb->interp;
    Jsi_Value *vpargs, *items[2] = {}, *ret = Jsi_ValueNew1(interp);
    
    items[0] = Jsi_ValueNewObj(interp, jdb->fobj);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 1, 0));
    Jsi_IncrRefCount(interp, vpargs);
    Jsi_FunctionInvoke(interp, jdb->onRollback, NULL, &ret, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
}


static void dbUpdateHandler(
    void *p,
    int op,
    const char *zDb,
    const char *zTbl,
    sqlite_int64 rowid
) {
    Jsi_Db *jdb = (Jsi_Db *)p;
    Jsi_Interp *interp = jdb->interp;
    int rc, i = 0;
    Jsi_Value *vpargs, *items[10] = {}, *ret;
    
    assert( op==SQLITE_INSERT || op==SQLITE_UPDATE || op==SQLITE_DELETE );
    items[i++] = Jsi_ValueNewObj(interp, jdb->fobj);
    items[i++] = Jsi_ValueMakeStringDup(interp, NULL, (op==SQLITE_INSERT)?"INSERT":(op==SQLITE_UPDATE)?"UPDATE":"DELETE");
    items[i++] = Jsi_ValueMakeStringDup(interp, NULL, zDb);
    items[i++] = Jsi_ValueMakeStringDup(interp, NULL, zTbl);
    items[i++] = Jsi_ValueMakeNumber(interp, NULL, (Jsi_Number)rowid);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, i, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew1(interp);
    rc = Jsi_FunctionInvoke(interp, jdb->onUpdate, vpargs, &ret, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    if (rc != JSI_OK)
        jdb->errCnt++;
}

static void dbCollateNeeded(
    void *cd,
    sqlite3 *db,
    int enc,
    const char *zName
) {
    int rc;
    Jsi_Db *jdb = (Jsi_Db*)cd;
    Jsi_Interp *interp = jdb->interp;
    Jsi_Value *vpargs, *items[2], *ret;
    items[0] = Jsi_ValueNewObj(interp, jdb->fobj);
    items[1] = Jsi_ValueMakeStringDup(interp, NULL, zName);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 2, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew1(interp);
    rc = Jsi_FunctionInvoke(interp, jdb->onNeedCollate, vpargs,& ret, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    if (rc != JSI_OK)
        jdb->errCnt++;

}

/*
** This routine is called to evaluate an SQL collation function implemented
** using JSI script.
*/
static int dbSqlCollate(
    void *pCtx,
    int nA,
    const void *zA,
    int nB,
    const void *zB
) {
    SqlCollate *p = (SqlCollate *)pCtx;
    Jsi_Interp *interp = p->interp;

    int rc;
    //Jsi_Db *jdb = (Jsi_Db*)cd;
    Jsi_Value *vpargs, *items[3], *ret;

    items[0] = Jsi_ValueNewObj(interp, p->jdb->fobj);
    items[1] = Jsi_ValueMakeStringDup(interp, NULL, (char*)zA);
    items[2] = Jsi_ValueMakeStringDup(interp, NULL, (char*)zB);
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 3, 0));
    ret = Jsi_ValueNew1(interp);
    rc = Jsi_FunctionInvoke(interp, p->zScript, vpargs, &ret, NULL);
    if( JSI_OK!=rc ) {
        //jdb->errCnt++;
        rc = 0;
    } else
        rc = dbGetIntBool(interp, ret);
    Jsi_DecrRefCount(interp, vpargs);
    Jsi_DecrRefCount(interp, ret);
    return rc;
}

static Jsi_Value* dbGetValueGet(Jsi_Interp *interp, sqlite3_value *pIn)
{
    Jsi_Value *v = Jsi_ValueNew(interp);
    switch (sqlite3_value_type(pIn)) {
    case SQLITE_BLOB: {
        int bytes;
        bytes = sqlite3_value_bytes(pIn);
        const char *zBlob = (char*) sqlite3_value_blob(pIn);
        if(!zBlob ) {
            return Jsi_ValueMakeNull(interp, &v);
        }
        unsigned char *uptr = (unsigned char*)Jsi_Malloc(bytes+1);
        memcpy(uptr, zBlob, bytes);
        uptr[bytes] = 0;
        return Jsi_ValueMakeBlob(interp, &v, uptr, bytes);
    }
    case SQLITE_INTEGER: {
        sqlite_int64 n = sqlite3_value_int64(pIn);
        if( n>=-2147483647 && n<=2147483647 ) {
            return Jsi_ValueMakeNumber(interp, &v, n);
        } else {
            return Jsi_ValueMakeNumber(interp, &v, n);
        }
    }
    case SQLITE_FLOAT: {
        return Jsi_ValueMakeNumber(interp, &v, (Jsi_Number)sqlite3_value_double(pIn));
    }
    case SQLITE_NULL: {
        return Jsi_ValueMakeNull(interp, &v);
    }
    default:
        return Jsi_ValueMakeStringDup(interp, &v, (char *)sqlite3_value_text(pIn));
    }
    return v;
}

static void jsiSqlFuncUnixTime(sqlite3_context *context, int argc, sqlite3_value**argv) {
    Jsi_Db *jdb = (Jsi_Db*)sqlite3_user_data(context);
    SQLSIGASSERT(jdb,DB);
    Jsi_Interp *interp = jdb->interp;
    if (argc>3) {
        Jsi_LogWarn("sqlite unixtime, expected: str fmt isutc");
        return;
    }
    const char *str = NULL, *fmt = NULL;
    bool isUtc = 0;
    if (argc>=1)
        str = (char *)sqlite3_value_text(argv[0]);
    if (argc>=2)
        fmt = (char *)sqlite3_value_text(argv[1]);
    if (argc>=3)
        isUtc = (bool)sqlite3_value_int64(argv[2]);
    Jsi_Number d = 0;
    Jsi_DatetimeParse(interp, str, fmt, isUtc, &d, 0);
    sqlite3_result_double(context, (double)d);
}

static void jsiSqlFunc(sqlite3_context *context, int argc, sqlite3_value**argv) {
    SqlFunc *p = (SqlFunc*)sqlite3_user_data(context);
    int i;
    int rc;
    Jsi_Interp *interp = p->interp;
    Jsi_Value *vpargs, *itemsStatic[100], **items = itemsStatic, *ret;
    if (argc>100)
        items = (Jsi_Value**)Jsi_Calloc(argc, sizeof(Jsi_Value*));

    for(i=0; i<argc; i++) {
        items[i] = dbGetValueGet(interp, argv[i]);
    }
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, argc, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew1(interp);
    rc = Jsi_FunctionInvoke(interp, p->tocall, vpargs, &ret, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    if (items != itemsStatic)
        Jsi_Free(items);

    bool b;
    if( rc != JSI_OK) {
        char buf[250];
        snprintf(buf, sizeof(buf), "error in function: %.200s", p->zName);
        sqlite3_result_error(context, buf, -1);

    } else if (Jsi_ValueIsBoolean(interp, ret)) {
        Jsi_GetBoolFromValue(interp, ret, &b);
        sqlite3_result_int(context, b);
    } else if (Jsi_ValueIsNumber(interp, ret)) {
        Jsi_Number d;
        // if (Jsi_GetIntFromValueBase(interp, ret, &i, 0, JSI_NO_ERRMSG);
        // sqlite3_result_int64(context, v);
        Jsi_GetNumberFromValue(interp, ret, &d);
        sqlite3_result_double(context, (double)d);
    } else {
        const char * data;
        if (!(data = Jsi_ValueGetStringLen(interp, ret, &i))) {
            //TODO: handle objects???
            data = Jsi_ValueToString(interp, ret, NULL);
            i = Jsi_Strlen(data);
        }
        sqlite3_result_text(context, (char *)data, i, SQLITE_TRANSIENT );
    }
    Jsi_DecrRefCount(interp, ret);
}

/*
** This is the authentication function.  It appends the authentication
** type code and the two arguments to zCmd[] then invokes the result
** on the interpreter.  The reply is examined to determine if the
** authentication fails or succeeds.
*/
static int dbAuthCallback(
    void *pArg,
    int code,
    const char *zArg1,
    const char *zArg2,
    const char *zArg3,
    const char *zArg4
) {
    const char *zCode;
    int rc;
    const char *zReply;
    Jsi_Db *jdb = (Jsi_Db*)pArg;
    Jsi_Interp *interp = jdb->interp;
    if( jdb->disableAuth ) return SQLITE_OK;

    switch( code ) {
    case SQLITE_COPY              :
        zCode="SQLITE_COPY";
        break;
    case SQLITE_CREATE_INDEX      :
        zCode="SQLITE_CREATE_INDEX";
        break;
    case SQLITE_CREATE_TABLE      :
        zCode="SQLITE_CREATE_TABLE";
        break;
    case SQLITE_CREATE_TEMP_INDEX :
        zCode="SQLITE_CREATE_TEMP_INDEX";
        break;
    case SQLITE_CREATE_TEMP_TABLE :
        zCode="SQLITE_CREATE_TEMP_TABLE";
        break;
    case SQLITE_CREATE_TEMP_TRIGGER:
        zCode="SQLITE_CREATE_TEMP_TRIGGER";
        break;
    case SQLITE_CREATE_TEMP_VIEW  :
        zCode="SQLITE_CREATE_TEMP_VIEW";
        break;
    case SQLITE_CREATE_TRIGGER    :
        zCode="SQLITE_CREATE_TRIGGER";
        break;
    case SQLITE_CREATE_VIEW       :
        zCode="SQLITE_CREATE_VIEW";
        break;
    case SQLITE_DELETE            :
        zCode="SQLITE_DELETE";
        break;
    case SQLITE_DROP_INDEX        :
        zCode="SQLITE_DROP_INDEX";
        break;
    case SQLITE_DROP_TABLE        :
        zCode="SQLITE_DROP_TABLE";
        break;
    case SQLITE_DROP_TEMP_INDEX   :
        zCode="SQLITE_DROP_TEMP_INDEX";
        break;
    case SQLITE_DROP_TEMP_TABLE   :
        zCode="SQLITE_DROP_TEMP_TABLE";
        break;
    case SQLITE_DROP_TEMP_TRIGGER :
        zCode="SQLITE_DROP_TEMP_TRIGGER";
        break;
    case SQLITE_DROP_TEMP_VIEW    :
        zCode="SQLITE_DROP_TEMP_VIEW";
        break;
    case SQLITE_DROP_TRIGGER      :
        zCode="SQLITE_DROP_TRIGGER";
        break;
    case SQLITE_DROP_VIEW         :
        zCode="SQLITE_DROP_VIEW";
        break;
    case SQLITE_INSERT            :
        zCode="SQLITE_INSERT";
        break;
    case SQLITE_PRAGMA            :
        zCode="SQLITE_PRAGMA";
        break;
    case SQLITE_READ              :
        zCode="SQLITE_READ";
        break;
    case SQLITE_SELECT            :
        zCode="SQLITE_SELECT";
        break;
    case SQLITE_TRANSACTION       :
        zCode="SQLITE_TRANSACTION";
        break;
    case SQLITE_UPDATE            :
        zCode="SQLITE_UPDATE";
        break;
    case SQLITE_ATTACH            :
        zCode="SQLITE_ATTACH";
        break;
    case SQLITE_DETACH            :
        zCode="SQLITE_DETACH";
        break;
    case SQLITE_ALTER_TABLE       :
        zCode="SQLITE_ALTER_TABLE";
        break;
    case SQLITE_REINDEX           :
        zCode="SQLITE_REINDEX";
        break;
    case SQLITE_ANALYZE           :
        zCode="SQLITE_ANALYZE";
        break;
    case SQLITE_CREATE_VTABLE     :
        zCode="SQLITE_CREATE_VTABLE";
        break;
    case SQLITE_DROP_VTABLE       :
        zCode="SQLITE_DROP_VTABLE";
        break;
    case SQLITE_FUNCTION          :
        zCode="SQLITE_FUNCTION";
        break;
    case SQLITE_SAVEPOINT         :
        zCode="SQLITE_SAVEPOINT";
        break;
    default                       :
        zCode="????";
        break;
    }
    int i = 0;
    Jsi_Value *vpargs, *items[10] = {}, *ret;
    items[i++] = Jsi_ValueNewObj(interp, jdb->fobj);
    items[i++] = Jsi_ValueMakeStringDup(interp, NULL, zCode);
    items[i++] = Jsi_ValueMakeStringDup(interp, NULL, zArg1 ? zArg1 : "");
    items[i++] = Jsi_ValueMakeStringDup(interp, NULL, zArg2 ? zArg2 : "");
    items[i++] = Jsi_ValueMakeStringDup(interp, NULL, zArg3 ? zArg3 : "");
    items[i++] = Jsi_ValueMakeStringDup(interp, NULL, zArg4 ? zArg4 : "");
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, i, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew(interp);
    rc = Jsi_FunctionInvoke(interp, jdb->onAuth, vpargs, &ret, NULL);
    Jsi_DecrRefCount(interp, vpargs);

    if (rc == JSI_OK && (zReply = Jsi_ValueGetStringLen(interp, ret, NULL)))
    {
        if( Jsi_Strcmp(zReply,"SQLITE_OK")==0 ) {
            rc = SQLITE_OK;
        } else if( Jsi_Strcmp(zReply,"SQLITE_DENY")==0 ) {
            rc = SQLITE_DENY;
        } else if( Jsi_Strcmp(zReply,"SQLITE_IGNORE")==0 ) {
            rc = SQLITE_IGNORE;
        } else {
            rc = 999;
        }
    }
    Jsi_DecrRefCount(interp, ret);
    return rc;
}

/*
** This routine reads a line of text from FILE in, stores
** the text in memory obtained from malloc() and returns a pointer
** to the text.  NULL is returned at end of file, or if malloc()
** fails.
**
** The interface is like "readline" but no command-line editing
** is done.
**
** copied from shell.c from '.import' command
*/
static char *dbLocalGetline(Jsi_Interp *interp, char *zPrompt, Jsi_Channel in) {
    char *zLine;
    int nLine;
    int n;
    int eol;

    nLine = 100;
    zLine = (char*)Jsi_Malloc( nLine );
    if( zLine==0 ) return 0;
    n = 0;
    eol = 0;
    while( !eol ) {
        if( n+100>nLine ) {
            nLine = nLine*2 + 100;
            zLine = (char*)Jsi_Realloc(zLine, nLine);
            if( zLine==0 ) return 0;
        }
        if( Jsi_Gets(interp, in, &zLine[n], nLine - n)==0 ) {
            if( n==0 ) {
                Jsi_Free(zLine);
                return 0;
            }
            zLine[n] = 0;
            eol = 1;
            break;
        }
        while( zLine[n] ) {
            n++;
        }
        if( n>0 && zLine[n-1]=='\n' ) {
            n--;
            zLine[n] = 0;
            eol = 1;
        }
    }
    zLine = (char*)Jsi_Realloc( zLine, n+1 );
    return zLine;
}


/*
** This function is part of the implementation of the command:
**
**   $db transaction [-deferred|-immediate|-exclusive] SCRIPT
**
** It is invoked after evaluating the script SCRIPT to commit or rollback
** the transaction or savepoint opened by the [transaction] command.
*/
static Jsi_RC dbTransPostCmd(
    Jsi_Db *jdb,                       /* Sqlite3Db for $db */
    Jsi_Interp *interp,                  /* Jsi interpreter */
    Jsi_RC result                           /* Result of evaluating SCRIPT */
) {
    static const char *azEnd[] = {
        "RELEASE _jsi_transaction",        /* rc==JSI_ERROR, nTransaction!=0 */
        "COMMIT",                          /* rc!=JSI_ERROR, nTransaction==0 */
        "ROLLBACK TO _jsi_transaction ; RELEASE _jsi_transaction",
        "ROLLBACK"                         /* rc==JSI_ERROR, nTransaction==0 */
    };
    Jsi_RC rc = result;
    const char *zEnd;

    jdb->nTransaction--;
    zEnd = azEnd[(rc==JSI_ERROR)*2 + (jdb->nTransaction==0)];

    jdb->disableAuth++;
    if( sqlite3_exec(jdb->db, zEnd, 0, 0, 0)) {
        /* This is a tricky scenario to handle. The most likely cause of an
        ** error is that the exec() above was an attempt to commit the
        ** top-level transaction that returned SQLITE_BUSY. Or, less likely,
        ** that an IO-error has occured. In either case, throw a Jsi exception
        ** and try to rollback the transaction.
        **
        ** But it could also be that the user executed one or more BEGIN,
        ** COMMIT, SAVEPOINT, RELEASE or ROLLBACK commands that are confusing
        ** this method's logic. Not clear how this would be best handled.
        */
        if( rc!=JSI_ERROR ) {
            Jsi_LogError("%s", sqlite3_errmsg(jdb->db));
            rc = JSI_ERROR;
        }
        sqlite3_exec(jdb->db, "ROLLBACK", 0, 0, 0);
    }
    jdb->disableAuth--;

    return rc;
}



#if 0
static void dbEvalRowInfo(
    DbEvalContext *p,               /* Evaluation context */
    int *pnCol,                     /* OUT: Number of column names */
    char ***papColName,           /* OUT: Array of column names */
    int **papColType
) {
    /* Compute column names */
    // Jsi_Interp *interp = p->jdb->interp;

    if( 0==p->apColName ) {
        sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
        int i;                        /* Iterator variable */
        int nCol;                     /* Number of columns returned by pStmt */
        char **apColName = 0;      /* Array of column names */
        int *apColType = 0;
        const char *zColName;         /* Column name */
        int numRid = 0;               /* Number of times rowid seen. */

        p->nCol = nCol = sqlite3_column_count(pStmt);
        if( nCol>0 && (papColName || p->pArray) ) {
            int cnLen = sizeof(char*)*nCol, cnStart = cnLen;
            for(i=0; i<nCol && cnLen<sizeof(p->staticColNames); i++)
                cnLen += Jsi_Strlen(sqlite3_column_name(pStmt,i))+1;
            if (cnLen>=sizeof(p->staticColNames)) {
                apColName = (char**)Jsi_Calloc(nCol, sizeof(char*) );
                cnStart = 0;
            } else {
                apColName = (char**)p->staticColNames;
            }
            if (papColType) {
                if (nCol < SQL_MAX_STATIC_TYPES)
                    apColType = p->staticColTypes;
                else
                    apColType = (int*)Jsi_Calloc(nCol, sizeof(int));
            }
            for(i=0; i<nCol; i++) {
                zColName = sqlite3_column_name(pStmt,i);
                if (cnStart==0)
                    apColName[i] = Jsi_Strdup(zColName);
                else {
                    apColName[i] = p->staticColNames+cnStart;
                    Jsi_Strcpy(apColName[i], zColName);
                    cnStart += Jsi_Strlen(zColName)+1;
                }
                if (apColType)
                    apColType[i] = sqlite3_column_type(pStmt,i);
                /* Check if rowid appears first, and more than once. */
                if ((i == 0 || numRid>0) &&
                        (zColName[0] == 'r' && Jsi_Strcmp(zColName,"rowid") == 0)) {
                    numRid++;
                }
            }
            /* Change first rowid to oid. */
            if (numRid > 1) {
                if (apColName != (char**)p->staticColNames) {
                    Jsi_Free(apColName[0]);
                    apColName[0] = Jsi_Strdup("oid");
                } else {
                    Jsi_Strcpy(apColName[0], "oid");
                }
            }
            p->apColName = apColName;
            p->apColType = apColType;
        }
    }
    if( papColName ) {
        *papColName = p->apColName;
    }
    if( papColType ) {
        *papColType = p->apColType;
    }
    if( pnCol ) {
        *pnCol = p->nCol;
    }
}
#endif

/*
** Return a JSON formatted value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
static void dbEvalSetColumnJSON(DbEvalContext *p, int iCol, Jsi_DString *dStr) {
    Jsi_Interp *interp = p->jdb->interp;
    char nbuf[200];

    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;

    switch( sqlite3_column_type(pStmt, iCol) ) {
    case SQLITE_BLOB: {
        int bytes = sqlite3_column_bytes(pStmt, iCol);
        const char *zBlob = (char*)sqlite3_column_blob(pStmt, iCol);
        if( !zBlob ) {
            Jsi_DSAppend(dStr, "null", NULL);
            return;
        }
        Jsi_JSONQuote(interp, zBlob, bytes, dStr);
        return;
    }
    case SQLITE_INTEGER: {
        sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
        if (v==0 || v==1) {
            const char *dectyp = sqlite3_column_decltype(pStmt, iCol);
            if (dectyp && !Jsi_Strncasecmp(dectyp,"bool", 4)) {
                Jsi_DSAppend(dStr, (v?"true":"false"), NULL);
                return;
            }
        }
#ifdef __WIN32
        snprintf(nbuf, sizeof(nbuf), "%" PRId64, (Jsi_Wide)v);
#else
        snprintf(nbuf, sizeof(nbuf), "%lld", v);
#endif
        Jsi_DSAppend(dStr, nbuf, NULL);
        return;
    }
    case SQLITE_FLOAT: {
        Jsi_NumberToString(interp, sqlite3_column_double(pStmt, iCol), nbuf, sizeof(nbuf));
        Jsi_DSAppend(dStr, nbuf, NULL);
        return;
    }
    case SQLITE_NULL: {
        Jsi_DSAppend(dStr, "null", NULL);
        return;
    }
    }
    const char *str = (char*)sqlite3_column_text(pStmt, iCol );
    if (!str)
        str = p->jdb->optPtr->nullvalue;
    Jsi_JSONQuote(interp, str?str:"", -1, dStr);
}

static void dbEvalSetColumn(DbEvalContext *p, int iCol, Jsi_DString *dStr) {
    Jsi_Interp *interp = p->jdb->interp;
    char nbuf[200];

    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;

    switch( sqlite3_column_type(pStmt, iCol) ) {
    case SQLITE_BLOB: {
        int bytes = sqlite3_column_bytes(pStmt, iCol);
        const char *zBlob = (char*)sqlite3_column_blob(pStmt, iCol);
        if( !zBlob ) {
            return;
        }
        Jsi_DSAppendLen(dStr, zBlob, bytes);
        return;
    }
    case SQLITE_INTEGER: {
        sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
        if (v==0 || v==1) {
            const char *dectyp = sqlite3_column_decltype(pStmt, iCol);
            if (dectyp && !Jsi_Strncasecmp(dectyp,"bool", 4)) {
                Jsi_DSAppend(dStr, (v?"true":"false"), NULL);
                return;
            }
        }
#ifdef __WIN32
        snprintf(nbuf, sizeof(nbuf), "%" PRId64, (Jsi_Wide)v);
#else
        snprintf(nbuf, sizeof(nbuf), "%lld", v);
#endif
        Jsi_DSAppend(dStr, nbuf, NULL);
        return;
    }
    case SQLITE_FLOAT: {
        Jsi_NumberToString(interp, sqlite3_column_double(pStmt, iCol), nbuf, sizeof(nbuf));
        Jsi_DSAppend(dStr, nbuf, NULL);
        return;
    }
    case SQLITE_NULL: {
        return;
    }
    }
    const char *str = (char*)sqlite3_column_text(pStmt, iCol );
    if (!str)
        str = p->jdb->optPtr->nullvalue;
    Jsi_DSAppend(dStr, str?str:"", NULL);
}


static Jsi_Value* dbEvalSetColumnValue(DbEvalContext *p, int iCol, Jsi_Value **val) {
    Jsi_Interp *interp = p->jdb->interp;

    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    const char *str;
    
    switch( sqlite3_column_type(pStmt, iCol) ) {
    case SQLITE_BLOB: {
        int bytes = sqlite3_column_bytes(pStmt, iCol);
        const char *zBlob = (char*)sqlite3_column_blob(pStmt, iCol);
        if( !zBlob )
            return Jsi_ValueMakeNull(interp, val);
        unsigned char *uptr = (unsigned char*)Jsi_Malloc(bytes+1);
        memcpy(uptr, zBlob, bytes);
        uptr[bytes] = 0;
        return Jsi_ValueMakeBlob(interp, val, uptr, bytes);
        break;
    }
    case SQLITE_INTEGER: {
        sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
        if (v==0 ||v==1) {
            const char *dectyp = sqlite3_column_decltype(pStmt, iCol);
            if (dectyp && !Jsi_Strncasecmp(dectyp,"bool", 4)) {
                return Jsi_ValueMakeBool(interp, val, v);
            }
        }
        if( v>=-2147483647 && v<=2147483647 ) {
            return Jsi_ValueMakeNumber(interp, val, v);
        } else {
            return Jsi_ValueMakeNumber(interp, val, v);
        }
        break;
    }
    case SQLITE_FLOAT: {
        return Jsi_ValueMakeNumber(interp, val, (Jsi_Number)sqlite3_column_double(pStmt, iCol));
        break;
    }
    case SQLITE_NULL: {
        return Jsi_ValueMakeNull(interp, val);
        break;;
    }
    case SQLITE_TEXT: {
        if (!p->jdb->noJsonConv) {
            const char *dectyp = sqlite3_column_decltype(pStmt, iCol);
            if (dectyp && !Jsi_Strncasecmp(dectyp, "charjson", 8)) {
                Jsi_Value *v = Jsi_ValueNew(interp);// NULL; //Jsi_ValueNew1(interp);
                str = (char*)sqlite3_column_text(pStmt, iCol );
                if (JSI_OK != Jsi_JSONParse(interp, str, &v, 0))
                    Jsi_LogWarn("JSON parse failure for CHARJSON column");
                return v;
            }
        }
    }
    default:
        str = (char*)sqlite3_column_text(pStmt, iCol );
        if (!str)
            str = p->jdb->optPtr->nullvalue;
        return Jsi_ValueMakeStringDup(interp, val, str?str:"");
    }
    return Jsi_ValueNew1(interp);
}


# define SQLITE_JSI_NRE 0
# define DbUseNre() 0
# define Jsi_NRAddCallback(a,b,c,d,e,f) 0
# define Jsi_NREvalObj(a,b,c) 0
# define Jsi_NRCreateCommand(a,b,c,d,e,f) 0

#include <stdio.h>

static Jsi_RC dbEvalCallCmd( DbEvalContext *p, Jsi_Interp *interp, Jsi_RC result)
{
    int cnt = 0;
    Jsi_RC rc = result;
    Jsi_Value *varg1;
    Jsi_Obj *argso;
    char **apColName = NULL;
    int *apColType = NULL;
    if (p->jdb->debug & TMODE_EVAL)
        JSI_DBQUERY_PRINTF( "DEBUG: eval\n");

    while( (rc==JSI_OK) && JSI_OK==(rc = dbEvalStep(p)) ) {
        int i;
        int nCol;

        cnt++;
        dbEvalRowInfo(p, &nCol, &apColName, &apColType);
        if (nCol<=0)
            continue;
        if (Jsi_ValueIsNull(interp,p->tocall))
            continue;
        /* Single object containing sql result members. */
        varg1 = Jsi_ValueMakeObject(interp, NULL, argso = Jsi_ObjNew(interp));
        for(i=0; i<nCol; i++) {
            Jsi_Value *nnv = dbEvalSetColumnValue(p, i, NULL);
            Jsi_ObjInsert(interp, argso, apColName[i], nnv, 0);
        }
        Jsi_IncrRefCount(interp, varg1);
        bool rb = Jsi_FunctionInvokeBool(interp, p->tocall, varg1);
        Jsi_DecrRefCount(interp, varg1);
        if (Jsi_InterpGone(interp))
            return JSI_ERROR;
        if (rb)
            break;
    }
    //dbEvalFinalize(p);

    if( rc==JSI_OK || rc==JSI_BREAK ) {
        //Jsi_ResetResult(interp);
        rc = JSI_OK;
    }
    return rc;
}

static Jsi_Db *dbGetDbHandle(Jsi_Interp *interp, Jsi_Value *_this, Jsi_Func *funcPtr)
{
    Jsi_Db *jdb = (Jsi_Db*)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!jdb) {
        Jsi_LogError("Sqlite call to a non-sqlite object");
        return NULL;
    }
    if (!jdb->db)
    {
        Jsi_LogError("Sqlite db closed");
        return NULL;
    }
    return jdb;
}

static void sqliteObjErase(Jsi_Db *jdb)
{
    dbDeleteCmd(jdb);
    jdb->db = NULL;
}

static Jsi_RC sqliteObjFree(Jsi_Interp *interp, void *data)
{
    Jsi_Db *db = (Jsi_Db*)data;
    SQLSIGASSERT(db,DB);
    db->_->activeCnt--;
    sqliteObjErase(db);
    _JSI_MEMCLEAR(db);
    Jsi_Free(db);
    return JSI_OK;
}

static bool sqliteObjIsTrue(void *data)
{
    Jsi_Db *db = (Jsi_Db*)data;
    SQLSIGASSERT(db,DB);
    if (!db->db) return 0;
    else return 1;
}

static bool sqliteObjEqual(void *data1, void *data2)
{
    //SQLSIGASSERT(data1,DB);
    //SQLSIGASSERT(data2,DB);
    return (data1 == data2);
}

static Jsi_RC dbStmtFreeProc(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *value);

static Jsi_RC SqliteConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                             Jsi_Value **ret, Jsi_Func *funcPtr);

static Jsi_RC SqliteCollateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                            Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Db *jdb;
    Jsi_Value *func;

    SqlCollate *pCollate;
    char *zName;
    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;

    zName = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    func = Jsi_ValueArrayIndex(interp, args, 1);
    pCollate = (SqlCollate*)Jsi_Calloc(1, sizeof(*pCollate));
    if( pCollate==0 ) return JSI_ERROR;
    pCollate->jdb = jdb;
    pCollate->interp = interp;
    pCollate->pNext = jdb->pCollate;
    pCollate->zScript = func; /*(char*)&pCollate[1];*/
    jdb->pCollate = pCollate;

    if( sqlite3_create_collation(jdb->db, zName, SQLITE_UTF8, pCollate, dbSqlCollate) )
    
        return Jsi_LogError("%s", (char *)sqlite3_errmsg(jdb->db));
    return JSI_OK;
}

static const char *copyConflictStrs[] = {
    "ROLLBACK", "ABORT", "FAIL", "IGNORE", "REPLACE", 0
};
enum { CC_ROLLBACK, CC_ABORT, CC_FAIL, CC_IGNORE, CC_REPLACE, CC__MAX };

typedef struct ImportData {
    int limit;
    int conflict;
    bool csv;
    bool headers;
    const char *separator;
    const char *nullvalue;
} ImportData;

static Jsi_OptionSpec ImportOptions[] =
{
    JSI_OPT(BOOL,   ImportData, headers, .help="First row contains column labels"),
    JSI_OPT(BOOL,   ImportData, csv, .help="Treat input values as CSV"),
    JSI_OPT(CUSTOM, ImportData, conflict, .help="Set conflict resolution", .flags=0, .custom=Jsi_Opt_SwitchEnum,  .data=copyConflictStrs),
    JSI_OPT(INT,    ImportData, limit, .help="Maximum number of lines to load"),
    JSI_OPT(STRKEY, ImportData, nullvalue, .help="Null string"),
    JSI_OPT(STRKEY, ImportData, separator, .help="Separator string; default is comma if csv, else tabs"),
    JSI_OPT_END(ImportData, .help="Options for the Sqlite import command")
};

#define FN_import JSI_INFO("\
Import data from a file into table. SqlOptions include the 'separator' \
to use, which defaults to commas for csv, or tabs otherwise.\n\
If a column contains a null string, or the \
value of 'nullvalue', a null is inserted for the column.\n\
A 'conflict' is one of the sqlite conflict algorithms: \
   rollback, abort, fail, ignore, replace\n\
On success, return the number of lines processed, not necessarily same \
as 'changeCnt' due to the conflict algorithm selected. \
")

static Jsi_RC SqliteImportCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                         Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Db *jdb;
    Jsi_RC rv = JSI_OK;
    int rc;
    char *zTable;               /* Insert data into this table */
    char *zFile;                /* The file from which to extract data */
    const char *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    int nCol;                   /* Number of columns in the table */
    int nByte;                  /* Number of bytes in an SQL string */
    int i, j;                   /* Loop counters */
    int nSep;                   /* Number of bytes in zSep[] */
    int nNull;                  /* Number of bytes in zNull[] */
    char *zSql;                 /* An SQL statement */
    char *zLine;                /* A single line of input from the file */
    char **azCol;               /* zLine[] broken up into columns */
    const char *onCommit;              /* How to commit changes */
    Jsi_Channel in;                   /* The input file */
    int lineno = 0;             /* Line number of input file */
    int created = 0;
    const char *zSep;
    const char *zNull;

    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 2);
    ImportData opts = {};

    if (arg) {
        if (Jsi_OptionsProcess(interp, ImportOptions, &opts, arg, 0) < 0)
            return JSI_ERROR;
    }
    zConflict = copyConflictStrs[opts.conflict];
    
    if(opts.separator ) {
        zSep = opts.separator;
    } else {
        zSep = (opts.csv ? "," : "\t");
    }
    if(opts.nullvalue ) {
        zNull = opts.nullvalue;
    } else {
        zNull = "";
    }
    zTable = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 1);
    zFile = Jsi_ValueArrayIndexToStr(interp, args, 1, NULL);
    nSep = Jsi_Strlen(zSep);
    nNull = Jsi_Strlen(zNull);
    if( nSep==0 ) 
        return Jsi_LogError("Error: non-null separator required for copy");


    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    if (zSql==0) 
        return Jsi_LogError("Error: bad table: %s", zTable);
    
    if (opts.headers) {
        in = Jsi_Open(interp, fname, "rb");
        if( in==0 ) 
            return Jsi_LogError("Error: cannot open file: %s", zFile);
        if ((zLine = dbLocalGetline(interp, 0, in))==0 ) {
            Jsi_Close(interp, in);
            return JSI_ERROR;
        }
        Jsi_Close(interp, in);
        char *zn, *ze, *z = zLine;
        Jsi_DString cStr = {};
        int zlen = 0, icnt = 0;
        Jsi_DSAppend(&cStr, "CREATE TABLE IF NOT EXISTS '", zTable, "' (", NULL);
        while (1) {
            zn = Jsi_Strstr(z, zSep);
            if (!zn) zlen = Jsi_Strlen(z);
            else zlen = zn-z;
            if (zlen<=0) break;
            ze = z+zlen-1;
            Jsi_DSAppend(&cStr, (icnt?",":""), "'", NULL);
            icnt++;
            if (opts.csv && *z=='"' && zn>z && *ze == '"')
                Jsi_DSAppendLen(&cStr, z+1, zlen-2);
            else
                Jsi_DSAppendLen(&cStr, z, zlen);
            Jsi_DSAppend(&cStr, "'", NULL);
            if (!zn) break;
            z = zn+nSep;
        }
        Jsi_DSAppend(&cStr, ");", NULL);
        Jsi_Free(zLine);
        if (zlen<=0) {
            Jsi_DSFree(&cStr);
            Jsi_LogError("null header problem");
            return JSI_ERROR;
        }
        rc = sqlite3_exec(jdb->db, Jsi_DSValue(&cStr), 0, 0, 0);
        Jsi_DSFree(&cStr);
        if (rc) 
            return Jsi_LogError("%s", sqlite3_errmsg(jdb->db));
        created = 1;
    }
    
    nByte = Jsi_Strlen(zSql);
    rc = sqlite3_prepare(jdb->db, zSql, -1, &pStmt, 0);
        sqlite3_free(zSql);
    if( rc ) {
        Jsi_LogError("%s", sqlite3_errmsg(jdb->db));
        nCol = 0;
    } else {
        nCol = sqlite3_column_count(pStmt);
    }
    sqlite3_finalize(pStmt);
    if( nCol==0 ) {
        rc = JSI_ERROR;
        goto bail;
    }
    zSql = (char*)Jsi_Malloc( nByte + 50 + nCol*2 );
    if( zSql==0 ) {
        Jsi_LogError("Error: can't malloc()");
        rc = JSI_ERROR;
        goto bail;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
                     zConflict, zTable);
    j = Jsi_Strlen(zSql);
    for(i=1; i<nCol; i++) {
        zSql[j++] = ',';
        zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(jdb->db, zSql, -1, &pStmt, 0);
    Jsi_Free(zSql);
    if( rc ) {
        Jsi_LogError("Error: %s", sqlite3_errmsg(jdb->db));
        sqlite3_finalize(pStmt);
        return JSI_ERROR;
    }
    in = Jsi_Open(interp, fname, "rb");
    if( in==0 ) {
        Jsi_LogError("Error: cannot open file: %s", zFile);
        sqlite3_finalize(pStmt);
        return JSI_ERROR;
    }
    azCol = (char**)Jsi_Malloc( sizeof(azCol[0])*(nCol+1) );
    if( azCol==0 ) {
        Jsi_LogError("Error: can't malloc()");
        Jsi_Close(interp, in);
        rc = JSI_ERROR;
        goto bail;
    }
    (void)sqlite3_exec(jdb->db, "BEGIN", 0, 0, 0);
    onCommit = "COMMIT";
    while ((zLine = dbLocalGetline(interp, 0, in))!=0 ) {
        char *z;
        i = 0;
        lineno++;
        if (opts.limit>0 && lineno > opts.limit) {
            Jsi_Free(zLine);
            break;
        }
        if (lineno == 1 && opts.headers) {
            Jsi_Free(zLine);
            continue;
        }
        if (opts.csv && Jsi_Strchr(zLine,'"')) 
        {
            char *zn, *z = zLine;
            Jsi_DString sStr = {};
            int qcnt = 0;
            i = -1;
            while (*z) if (*z++ == '"') qcnt++;
            z = zLine;
            if (qcnt%2) { /* aggregate quote spanning newlines */
                Jsi_DSAppend(&sStr, zLine, NULL);
                do {
                    lineno++;
                    Jsi_DSAppend(&sStr, "\n", NULL);
                    Jsi_Free(zLine);
                    if (((zLine = dbLocalGetline(interp, 0, in)))==0)
                        break;
                    Jsi_DSAppend(&sStr, zLine, NULL);
                    z = zLine;
                    while (*z) if (*z++ == '"') qcnt++;
                } while (qcnt%2);
                z = Jsi_DSValue(&sStr);
            }
            if (qcnt%2) {
                Jsi_DSFree(&sStr);
                Jsi_Free(zLine);
                Jsi_Close(interp, in);
                Jsi_LogError("unterminated string at line: %d", lineno);
                break;
            }
            while (z) {
                if (*z != '\"') { /* Handle un-quoted value */
                    zn = Jsi_Strstr(z, zSep);
                    azCol[++i] = z;
                    if (!zn)
                        break;
                    *zn = 0;
                    z = zn+nSep;
                    continue;
                }
                /* Handle quoted value */
                zn = ++z;
                Jsi_DString cStr = {};
                while (1) {
                    if (!zn)
                        break;
                    if (*zn != '"')
                        Jsi_DSAppendLen(&cStr, zn, 1);
                    else {
                        if (zn[1] == '"') {
                            zn++;
                            Jsi_DSAppendLen(&cStr, "\"", 1);
                        } else if (zn[1] == 0) {
                            break;
                        } else if (Jsi_Strncmp(zn+1,zSep, nSep)==0) {
                            *zn = 0;
                            zn += (nSep + 1);
                            break;
                        } else {
                            /* Invalid, comma should be right after close quote, so just eat quote. */
                            Jsi_DSAppendLen(&cStr, zn, 1);
                        }
                    }
                    zn++;
                }
                Jsi_Strcpy(z, Jsi_DSValue(&cStr));
                Jsi_DSFree(&cStr);
                azCol[++i] = z;
                z = zn;
            }
        } else {
            azCol[0] = zLine;
            for(i=0, z=zLine; *z; z++) {
                if( *z==zSep[0] && Jsi_Strncmp(z, zSep, nSep)==0 ) {
                    *z = 0;
                    i++;
                    if( i<nCol ) {
                        azCol[i] = &z[nSep];
                        z += nSep-1;
                    }
                }
            }
        }
        if( i+1!=nCol ) {
            Jsi_LogError("%s line %d: expected %d columns of data but found %d",
                 zFile, lineno, nCol, i+1);
            onCommit = "ROLLBACK";
            break;
        }
        for(i=0; i<nCol; i++) {
            /* check for null data, if so, bind as null */
            if( (nNull>0 && Jsi_Strcmp(azCol[i], zNull)==0)
                    || Jsi_Strlen(azCol[i])==0
              ) {
                sqlite3_bind_null(pStmt, i+1);
            } else {
                sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC );
            }
        }
        sqlite3_step(pStmt);
        rc = sqlite3_reset(pStmt);
        if (zLine)
            Jsi_Free(zLine);
        if( rc!=SQLITE_OK ) {
            Jsi_LogError("%s at line: %d", sqlite3_errmsg(jdb->db), lineno);
            onCommit = "ROLLBACK";
            break;
        }
    }
    Jsi_Free(azCol);
    Jsi_Close(interp, in);
    sqlite3_finalize(pStmt);
    (void)sqlite3_exec(jdb->db, onCommit, 0, 0, 0);

    if( onCommit[0] == 'C' ) {
        /* success, set result as number of lines processed */
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)lineno);
        rv = JSI_OK;
    } else {
        rv = JSI_ERROR;
    }
    
bail:
    if (rc != JSI_OK && created && opts.conflict == CC_ROLLBACK) {
        Jsi_DString cStr = {};
        Jsi_DSAppend(&cStr, "DROP TABLE IF EXISTS '", zTable, "';", NULL);
        (void)sqlite3_exec(jdb->db, Jsi_DSValue(&cStr), 0, 0, 0);
        Jsi_DSFree(&cStr);
    }
    return rv;
}

#define FN_evaluate JSI_INFO("\
Supports multiple semicolon seperated commands.\n\
Variable binding is NOT performed, results are discarded, and  \
returns sqlite3_changes()")
static Jsi_RC SqliteEvalCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                         Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int rc = SQLITE_OK, rc2;
    Jsi_Db *jdb;
    sqlite3_stmt *pStmt = NULL;

    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    sqlite3 *db = jdb->db;
    const char *zSql = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    const char *zStart = zSql, *zLeftover = NULL, *zErrMsg = NULL;
    int lnum = 1;
    if (jdb->echo && zSql)
        Jsi_LogInfo("SQL-EVAL: %s\n", zSql); 

    while( zSql && zSql[0] && (SQLITE_OK == rc) ) {
        rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);

        if( SQLITE_OK != rc ) {
            break;
        } else {
            if( !pStmt ) {
                /* this happens for a comment or white-space */
                zSql = zLeftover;
                while( isspace(zSql[0]) ) zSql++;
                continue;
            }

            do {
                if (jdb->debug & TMODE_STEP)
                    JSI_DBQUERY_PRINTF( "DEBUG: step: %s\n", zSql);
                rc = sqlite3_step(pStmt);
            } while( rc == SQLITE_ROW );
            rc2 = sqlite3_finalize(pStmt);
            if( rc!=SQLITE_NOMEM ) rc = rc2;
            if( rc==SQLITE_OK ) {
                zSql = zLeftover;
                while( isspace(zSql[0]) ) zSql++;
            } else {
            }
        }
    }
 
    if (rc == SQLITE_OK) {
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)sqlite3_changes(jdb->db));
        return JSI_OK;
    }
    while (zSql && zStart<zSql) {
        if (zStart[0] == '\n') lnum++;
        zStart++;
    }
    zErrMsg = sqlite3_errmsg(db);
    Jsi_LogError("sqlite error: %s in statement at line %d", (zErrMsg ? zErrMsg : ""), lnum);
    return JSI_ERROR;
}

/*
** If a field contains any character identified by a 1 in the following
** array, then the string must be quoted for CSV.
*/
static const char needCsvQuote[] = {
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 0, 1, 0, 0, 0, 0, 1,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 1, 
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
};

/*
** Output a single term of CSV.  Actually, p->separator is used for
** the separator, which may or may not be a comma.  p->nullvalue is
** the null value.  Strings are quoted if necessary.
*/
static void dbOutputCsv(QueryOpts *p, const char *z, Jsi_DString *dStr, int bSep)
{
    if( z==0 ) {
        Jsi_DSAppend(dStr,  p->nullvalue?p->nullvalue:"", NULL);
    } else {
        int i;
        int nSep = Jsi_Strlen(p->separator);
        for(i=0; z[i]; i++) {
            if( needCsvQuote[((unsigned char*)z)[i]] || 
                (z[i]==p->separator[0] && (nSep==1 || memcmp(z, p->separator, nSep)==0)) ) {
                i = 0;
                break;
            }
        }
        if( i==0 ) {
            Jsi_DSAppend(dStr, "\"", NULL);
            for(i=0; z[i]; i++) {
                if( z[i]=='"' ) Jsi_DSAppend(dStr, "\"", NULL);
                Jsi_DSAppendLen(dStr, z+i, 1);
            }
            Jsi_DSAppend(dStr, "\"", NULL);
        } else {
            Jsi_DSAppend(dStr, z, NULL);
        }
    }
    if( bSep ) {
        Jsi_DSAppend(dStr, p->separator, NULL);
    }
}

static void dbOutputHtmlString(QueryOpts *p, const char *z, Jsi_DString *dStr)
{
    while( *z ) {
        switch (*z) {
        case '<':
            Jsi_DSAppend(dStr, "&lt;", NULL);
            break;
        case '>':
            Jsi_DSAppend(dStr, "&gt;", NULL);
            break;
        case '&':
            Jsi_DSAppend(dStr, "&amp;", NULL);
            break;
        case '\"':
            Jsi_DSAppend(dStr, "&quot;", NULL);
            break;
        case '\'':
            Jsi_DSAppend(dStr, "&#39;", NULL);
            break;
        default:
            Jsi_DSAppendLen(dStr, z, 1);
            break;
        }
        z++;
    }
}
/*
** Output the given string as a quoted string using SQL quoting conventions.
*/
static void dbOutputQuotedString(Jsi_DString *dStr, const char *z) {
    int i;
    int nSingle = 0;
    for(i=0; z[i]; i++) {
        if( z[i]=='\'' ) nSingle++;
    }
    if( nSingle==0 ) {
        Jsi_DSAppend(dStr,"'", z, "'", NULL);
    } else {
        Jsi_DSAppend(dStr,"'", NULL);
        while( *z ) {
            for(i=0; z[i] && z[i]!='\''; i++) {}
            if( i==0 ) {
                Jsi_DSAppend(dStr,"''", NULL);
                z++;
            } else if( z[i]=='\'' ) {
                Jsi_DSAppendLen(dStr,z, i);
                Jsi_DSAppend(dStr,"''", NULL);
                z += i+1;
            } else {
                Jsi_DSAppend(dStr, z, NULL);
                break;
            }
        }
        Jsi_DSAppend(dStr,"'", NULL);
    }
}
/*
** Output the given string as a hex-encoded blob (eg. X'1234' )
*/
static void dbOutputHexBlob(Jsi_DString *dStr, const void *pBlob, int nBlob){
  int i;
  char out[100], *zBlob = (char *)pBlob;
  Jsi_DSAppend(dStr, "X'", NULL);
  for(i=0; i<nBlob; i++){ snprintf(out, sizeof(out),"%02x",zBlob[i]&0xff);Jsi_DSAppend(dStr, out, NULL); }
  Jsi_DSAppend(dStr, "'", NULL);
}

bool dbIsAlnumStr(const char *cp)
{
    if (!cp || !*cp) return 0;
    while (*cp)
        if (isalnum(*cp) || *cp == '_')
            cp++;
        else
            return 0;
    return 1;
}

#define FN_sqlexec JSI_INFO("\
Return values in formatted as JSON, HTML, etc. , optionally calling function with a result object")
static Jsi_RC SqliteQueryCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                             Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_RC rc = JSI_OK;
    Jsi_Db *jdb;
    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *vSql = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_DString eStr = {};
#ifdef JSI_DB_DSTRING_SIZE
    JSI_DSTRING_VAR(dStr, JSI_DB_DSTRING_SIZE);
#else
    Jsi_DString ddStr, *dStr = &ddStr;
    Jsi_DSInit(dStr);
#endif
    const char *zSql = Jsi_ValueGetDString(interp, vSql, &eStr, 0);
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    int cnt = 0;
    char **apColName = NULL;
    int *apColType = NULL, isopts = 0;
    DbEvalContext sEval = {};
    QueryOpts opts, *oEopt = jdb->optPtr;
    opts = jdb->queryOpts;
    opts.callback = NULL;
    opts.width = NULL;
    Jsi_Value *callback = NULL, *width = NULL;
            
    if (arg) {
        if (Jsi_ValueIsFunction(interp,arg))
            callback = opts.callback = arg;
        else if (Jsi_ValueIsString(interp, arg))
            opts.objName = Jsi_ValueString(interp, arg, NULL);
        else if (Jsi_ValueIsObjType(interp, arg, JSI_OT_ARRAY))
            opts.values = arg;
        else if (Jsi_ValueIsObjType(interp, arg, JSI_OT_OBJECT))
            isopts = 1;
        else {
            rc = Jsi_LogError("arg 2: expected function, string, array or options");
            goto bail;
        }
    }

    if (isopts) {
        if (Jsi_OptionsProcess(interp, ExecFmtOptions, &opts, arg, 0) < 0) {
            rc = JSI_ERROR;
            goto bail;
        }
        callback = (opts.callback ? opts.callback : jdb->queryOpts.callback);
        width = (opts.width ? opts.width : jdb->queryOpts.width);
    }
    if (opts.retChanged) {
        if (opts.callback) {
            rc = Jsi_LogError("can not use retChanged with callback");
            goto bail;
        }
        opts.mode = _JSI_EF_NONE;
        opts.headers = 0;
    }
    if (opts.cdata) {
        Jsi_CDataDb* copts = Jsi_CDataLookup(interp, opts.cdata);
        if (!copts)
            rc = Jsi_LogError("unknown cdata name: %s", opts.cdata);
        else {
            int n = Jsi_DbQuery(jdb, copts, zSql);
            Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)n);
        }
        goto bail;
    }
    if (opts.objName) {
        if (Jsi_SqlObjBinds(interp, &eStr, opts.objName, !(opts.objOpts&OBJMODE_NOTYPES), 
            !(opts.objOpts&OBJMODE_NODEFAULTS), (opts.objOpts&OBJMODE_NULLDEFAULTS)!=0) != JSI_OK)
            goto bail;
        zSql = Jsi_DSValue(&eStr);
    }
    if ((jdb->echo || opts.echo) && zSql)
        Jsi_LogInfo("SQL-ECHO: %s\n", zSql); 
    if ((opts.objOpts&OBJMODE_SQLONLY)) {
        if (opts.objName)
            Jsi_ValueMakeStringDup(interp, ret, zSql);
        else
            rc = Jsi_LogError("'objOpts.sqlOnly' can only be used with 'objName'");
        goto bail;
    }
    if (!opts.separator) {
        switch (opts.mode) {
            case _JSI_EF_LIST: opts.separator = "|"; break;
            case _JSI_EF_COLUMN: opts.separator = " "; break;
            case _JSI_EF_TABS: opts.separator = "\t"; break;
            default: opts.separator = ",";
        }
    }
    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    sEval.nocache = opts.nocache;
    if ((rc = dbEvalInit(interp, &sEval, jdb, zSql, &sStr, 0, 0)) != JSI_OK)
        goto bail;
    sEval.ret = *ret;
    jdb->optPtr = &opts;
    if (callback) {
        sEval.tocall = callback;
        if (opts.mode != _JSI_EF_ROWS) {
            Jsi_LogError("'mode' must be 'rows' with 'callback'");
            rc = JSI_ERROR;
            goto bail;
        }
        rc = dbEvalCallCmd(&sEval, interp, JSI_OK);
        goto bail;
    } else
    switch (opts.mode) {
    case _JSI_EF_NONE:
        while(JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        if (opts.retChanged)
            Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)sqlite3_changes(jdb->db));
        if (rc == JSI_BREAK)
            rc = JSI_OK;
        goto bail;
        break;
    case _JSI_EF_JSON:
        if (opts.headers) {
            Jsi_DSAppend(dStr, "[ ", NULL);
            while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
                if (cnt == 0) {
                    Jsi_DSAppend(dStr, "[", NULL);
                    for(i=0; i<nCol; i++) {
                        if (i)
                            Jsi_DSAppend(dStr, ", ", NULL);
                        Jsi_JSONQuote(interp, apColName[i], -1, dStr);
                    }
                    Jsi_DSAppend(dStr, "]", NULL);
                    cnt++;
                }
                if (cnt)
                    Jsi_DSAppend(dStr, ", ", NULL);
                Jsi_DSAppend(dStr, "[", NULL);
                for(i=0; i<nCol; i++) {
                    if (i)
                        Jsi_DSAppend(dStr, ", ", NULL);
                    dbEvalSetColumnJSON(&sEval, i, dStr);
                }
                Jsi_DSAppend(dStr, "]", NULL);
                cnt++;
                if (opts.limit && cnt>opts.limit) break;
            }
            Jsi_DSAppend(dStr, " ]", NULL);
            
        } else {
            Jsi_DSAppend(dStr, "[ ", NULL);
            while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
                if (cnt)
                    Jsi_DSAppend(dStr, ", ", NULL);
                Jsi_DSAppend(dStr, "{", NULL);
                for(i=0; i<nCol; i++) {
                    if (i)
                        Jsi_DSAppend(dStr, ", ", NULL);
                    Jsi_JSONQuote(interp, apColName[i], -1, dStr);
                    Jsi_DSAppend(dStr, ":", NULL);
                    dbEvalSetColumnJSON(&sEval, i, dStr);
                }
                Jsi_DSAppend(dStr, "}", NULL);
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            Jsi_DSAppend(dStr, " ]", NULL);
        }
        break;
        
    case _JSI_EF_JSON2: {
            while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
                if (cnt == 0 && 1) {
                    Jsi_DSAppend(dStr, "{ \"names\": [ ", NULL);
                    for(i=0; i<nCol; i++) {
                        if (i)
                            Jsi_DSAppend(dStr, ", ", NULL);
                        Jsi_JSONQuote(interp, apColName[i], -1, dStr);
                    }
                    Jsi_DSAppend(dStr, " ], \"values\": [ ", NULL);
                }
                if (cnt)
                    Jsi_DSAppend(dStr, ", ", NULL);
                Jsi_DSAppend(dStr, "[", NULL);
                for(i=0; i<nCol; i++) {
                    if (i)
                        Jsi_DSAppend(dStr, ", ", NULL);
                    dbEvalSetColumnJSON(&sEval, i, dStr);
                }
                Jsi_DSAppend(dStr, " ]", NULL);
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            if (cnt)
                Jsi_DSAppend(dStr, " ] } ", NULL);
        }
        break;
        
    case _JSI_EF_LIST:
        while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            int i;
            int nCol;
            dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
            if (cnt == 0 && opts.headers) {
                for(i=0; i<nCol; i++) {
                    if (i)
                        Jsi_DSAppend(dStr, opts.separator, NULL);
                    Jsi_DSAppend(dStr, apColName[i], NULL);
                }
            }

            if (cnt || opts.headers)
                Jsi_DSAppend(dStr, "\n", NULL);
            for(i=0; i<nCol; i++) {
                if (i)
                    Jsi_DSAppend(dStr, opts.separator, NULL);
                dbEvalSetColumn(&sEval, i, dStr);
            }
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        break;
        
    case _JSI_EF_COLUMN: {
        int *wids = NULL;
        Jsi_DString vStr = {};
        while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            int i, w;
            int nCol;
            
            dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
            if (cnt == 0 && nCol>0) {
                Jsi_DString sStr;
                wids = (int*)Jsi_Calloc(nCol, sizeof(int));
                Jsi_DSInit(&sStr);
                for(i=0; i<nCol; i++) {
                    int j = Jsi_Strlen(apColName[i]);
                    wids[i] = (j<10?10:j);
                    if (width) {
                        Jsi_Value *wv = Jsi_ValueArrayIndex(interp, width, i);
                        if (wv) {
                            Jsi_Number dv;
                            Jsi_ValueGetNumber(interp, wv, &dv);
                            if (dv>0)
                                wids[i] = (int)dv;
                        }
                    }
                    w = (j<wids[i] ? j : wids[i]);
                    Jsi_DSAppendLen(dStr, apColName[i], w);
                    w = (j<wids[i] ? wids[i]-j+1 : 0);
                    while (w-- > 0)
                        Jsi_DSAppend(dStr, " ", NULL);
                }
                for(i=0; i<nCol && opts.headers; i++) {
                    w = wids[i];
                    w -= Jsi_Strlen(apColName[i]);
                    if (i) {
                        Jsi_DSAppend(dStr, opts.separator, NULL);
                        Jsi_DSAppend(&sStr, opts.separator, NULL);
                    }
                    w = wids[i];
                    while (w-- > 0)
                        Jsi_DSAppend(&sStr, "-", NULL);
                }
                if (opts.headers)
                    Jsi_DSAppend(dStr, "\n", Jsi_DSValue(&sStr), "\n", NULL);
                Jsi_DSFree(&sStr);
            }

            if (cnt)
                Jsi_DSAppend(dStr, "\n", NULL);
            for(i=0; i<nCol; i++) {
                if (i)
                    Jsi_DSAppend(dStr, opts.separator, NULL);
                Jsi_DSSetLength(&vStr, 0);
                dbEvalSetColumn(&sEval, i, &vStr);
                int nl = Jsi_DSLength(&vStr);
                if (nl > wids[i]) {
                    Jsi_DSSetLength(&vStr, wids[i]);
                    w = 0;
                } else {
                    w = wids[i]-nl;
                }
                Jsi_DSAppend(dStr, Jsi_DSValue(&vStr), NULL);
                while (w-- > 0)
                    Jsi_DSAppend(dStr, " ", NULL);
            }
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        Jsi_DSFree(&vStr);
        if (wids)
            Jsi_Free(wids);
        break;
    }
    
    case _JSI_EF_INSERT: {
        Jsi_DString vStr = {};    
        while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            int i;
            int nCol;
            const char *tbl = (opts.table ? opts.table : "table");
            Jsi_DSAppend(dStr, "INSERT INTO \"", tbl, "\" VALUES(", NULL);
            dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
            for(i=0; i<nCol; i++) {
                Jsi_Number dv;
                const char *azArg;
                Jsi_DSSetLength(&vStr, 0);
                dbEvalSetColumn(&sEval, i, &vStr);
                sqlite3_stmt *pStmt = sEval.pPreStmt->pStmt;
                int ptype = sqlite3_column_type(pStmt, i);
                
                azArg = Jsi_DSValue(&vStr);
                const char *zSep = i>0 ? ",": "";
                if(apColType && apColType[i]==SQLITE_NULL) {
                  Jsi_DSAppend(dStr, zSep, "NULL", NULL);
                }else if( ptype ==SQLITE_TEXT ) {
                  if( zSep[0] ) Jsi_DSAppend(dStr,zSep, NULL);
                  dbOutputQuotedString(dStr, azArg);
                }else if (ptype==SQLITE_INTEGER || ptype ==SQLITE_FLOAT) {
                  Jsi_DSAppend(dStr, zSep, azArg, NULL);
                }else if (ptype ==SQLITE_BLOB) {
                  const void *pBlob = sqlite3_column_blob(pStmt, i );
                  int nBlob = sqlite3_column_bytes(pStmt, i);
                  if( zSep[0] ) Jsi_DSAppend(dStr,zSep, NULL);
                  dbOutputHexBlob(dStr, pBlob, nBlob);
                }else if( Jsi_GetDouble(interp, azArg, &dv) == JSI_OK ){
                  Jsi_DSAppend(dStr, zSep, azArg, NULL);
                }else{
                  if( zSep[0] ) Jsi_DSAppend(dStr,zSep, NULL);
                  dbOutputQuotedString(dStr, azArg);
                }
            }
            Jsi_DSAppend(dStr, ");\n", NULL);
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        Jsi_DSFree(&vStr);
    }

    case _JSI_EF_TABS:
    case _JSI_EF_CSV: {
        Jsi_DString vStr = {};  
        while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            int i;
            int nCol;
            dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
            if (cnt == 0 && opts.headers) {
                for(i=0; i<nCol; i++) {
                    if (i)
                        Jsi_DSAppend(dStr, opts.separator, NULL);
                    Jsi_DSAppend(dStr, apColName[i], NULL);
                }
            }

            if (cnt || opts.headers)
                Jsi_DSAppend(dStr, "\n", NULL);
            for(i=0; i<nCol; i++) {
                if (i)
                    Jsi_DSAppend(dStr, opts.separator, NULL);
                Jsi_DSSetLength(&vStr, 0);
                dbEvalSetColumn(&sEval, i, &vStr);
                if (opts.mode == _JSI_EF_CSV)
                    dbOutputCsv(&opts, Jsi_DSValue(&vStr), dStr, 0);
                else
                    Jsi_DSAppend(dStr, Jsi_DSValue(&vStr), NULL);
            }
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        Jsi_DSFree(&vStr);
        break;
    }
        
    case _JSI_EF_LINE: {
        int i, w = 5, ww;
        int nCol;
        Jsi_DString vStr = {};   
        while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
            if (cnt == 0) {
                for(i=0; i<nCol; i++) {
                    ww = Jsi_Strlen(apColName[i]);
                    if (ww>w)
                        w = ww;
                }
            }

            for(i=0; i<nCol; i++) {
                Jsi_DString eStr;
                Jsi_DSInit(&eStr);
                Jsi_DSSetLength(&vStr, 0);
                dbEvalSetColumn(&sEval, i, &vStr);
                Jsi_DSPrintf(&eStr, "%*s = %s", w, apColName[i], Jsi_DSValue(&vStr));
                Jsi_DSAppend(dStr, (cnt?"\n":""), Jsi_DSValue(&eStr), NULL);
                Jsi_DSFree(&eStr);
            }
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        Jsi_DSFree(&vStr);
        break;
    }
        
    case _JSI_EF_HTML: {
        Jsi_DString vStr = {};   
        while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            int i;
            int nCol;
            dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
            if (cnt == 0 && opts.headers) {
                Jsi_DSAppend(dStr, "<TR>", NULL);
                for(i=0; i<nCol; i++) {
                    Jsi_DSAppend(dStr, "<TH>", NULL);
                    dbOutputHtmlString(&opts, apColName[i], dStr);
                    Jsi_DSAppend(dStr, "</TH>", NULL);
                }
                Jsi_DSAppend(dStr, "</TR>", NULL);
            }
            if (cnt || opts.headers)
                Jsi_DSAppend(dStr, "\n", NULL);
            Jsi_DSAppend(dStr, "<TR>", NULL);
            for(i=0; i<nCol; i++) {
                Jsi_DSAppend(dStr, "<TD>", NULL);
                Jsi_DSSetLength(&vStr, 0);
                dbEvalSetColumn(&sEval, i, &vStr);
                dbOutputHtmlString(&opts, Jsi_DSValue(&vStr), dStr);
                Jsi_DSAppend(dStr, "</TD>", NULL);
            }
            Jsi_DSAppend(dStr, "</TR>", NULL);
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        Jsi_DSFree(&vStr);
        break;
    }
        
    case _JSI_EF_ROWS:
    {
        Jsi_Value *vcur, *vrow;
        int cnt = 0;
        Jsi_Obj *oall, *ocur;
        Jsi_ValueMakeArrayObject(interp, ret, oall = Jsi_ObjNewType(interp, JSI_OT_ARRAY));

        while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            int i;
            int nCol;
            dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
            ocur = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
            vrow = Jsi_ValueMakeObject(interp, NULL, ocur);
            for(i=0; i<nCol; i++) {
                vcur = dbEvalSetColumnValue(&sEval, i, NULL);
                Jsi_ObjInsert(interp, ocur, apColName[i], vcur, 0);
            }
            Jsi_ObjArrayAdd(interp, oall, vrow);
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        dbEvalFinalize(&sEval);
        if (rc != JSI_ERROR)
            rc = JSI_OK;
        goto bail;
        break;
    }
    case _JSI_EF_ARRAYS:
    {
        Jsi_Value *vcur, *vrow;
        int cnt = 0;
        Jsi_Obj *oall, *ocur;
        Jsi_ValueMakeArrayObject(interp, ret, oall = Jsi_ObjNewType(interp, JSI_OT_ARRAY));

        while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            int i;
            int nCol;
            dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
            if (cnt == 0 && opts.headers) {
                vrow = Jsi_ValueMakeArrayObject(interp, NULL, ocur = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
                for(i=0; i<nCol; i++) {
                    vcur = Jsi_ValueNewStringDup(interp, apColName[i]);
                    Jsi_ObjArrayAdd(interp, ocur, vcur);
                }
                Jsi_ObjArrayAdd(interp, oall, vrow);
            }
            vrow = Jsi_ValueMakeArrayObject(interp, NULL, ocur = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
            for(i=0; i<nCol; i++) {
                vcur = dbEvalSetColumnValue(&sEval, i, NULL);
                Jsi_ObjArrayAdd(interp, ocur, vcur);
            }
            Jsi_ObjArrayAdd(interp, oall, vrow);
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        dbEvalFinalize(&sEval);
        if (rc != JSI_ERROR)
            rc = JSI_OK;
        goto bail;
        break;
    }
    case _JSI_EF_ARRAY1D:
    {
        Jsi_Value *vcur;
        int cnt = 0;
        Jsi_Obj *oall;
        Jsi_ValueMakeArrayObject(interp, ret, oall = Jsi_ObjNewType(interp, JSI_OT_ARRAY));

        while( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
            int i;
            int nCol;
            dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
            if (cnt == 0 && opts.headers) {
                for(i=0; i<nCol; i++) {
                    vcur = Jsi_ValueNewStringDup(interp, apColName[i]);
                    Jsi_ObjArrayAdd(interp, oall, vcur);
                }
            }
            for(i=0; i<nCol; i++) {
                vcur = dbEvalSetColumnValue(&sEval, i, NULL);
                Jsi_ObjArrayAdd(interp, oall, vcur);
            }
            cnt++;
            if (opts.limit && cnt>=opts.limit) break;
        }
        dbEvalFinalize(&sEval);
        if (rc != JSI_ERROR)
            rc = JSI_OK;
        goto bail;
        break;
    }
    }
    if (opts.retChanged)
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)sqlite3_changes(jdb->db));
    else
        Jsi_ValueFromDS(interp, dStr, ret);
    if( rc==JSI_BREAK )
        rc = JSI_OK;
bail:
    dbEvalFinalize(&sEval);
    if (isopts) {
        Jsi_OptionsFree(interp, ExecFmtOptions, &opts, 0);
    }
    Jsi_DSFree(dStr);
    Jsi_DSFree(&eStr);
    jdb->optPtr = oEopt;
    return rc;
}

static Jsi_RC SqliteOnecolumnCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                          Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_RC rc;
    Jsi_Db *jdb;
    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *vSql = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    DbEvalContext sEval = {};
    const char *zSql = Jsi_ValueGetDString(interp, vSql, &dStr, 0);

    sEval.nocache = jdb->queryOpts.nocache;
    if ((rc = dbEvalInit(interp, &sEval, jdb, zSql, &sStr, 0, 0)) != JSI_OK)
        return rc;
    sEval.ret = *ret;
    sEval.tocall = NULL;
    int cnt = 0;


    if( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
        sqlite3_stmt *pStmt = sEval.pPreStmt->pStmt;
        int nCol = sqlite3_column_count(pStmt);
        if (nCol>0)
            dbEvalSetColumnValue(&sEval, 0, ret);
        cnt++;
    }
    dbEvalFinalize(&sEval);
    if( rc==JSI_BREAK ) {
        rc = JSI_OK;
    }
    Jsi_DSFree(&dStr);
    return rc;
}

static Jsi_RC SqliteExistsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                           Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_RC rc;
    Jsi_Db *jdb;
    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *vSql = Jsi_ValueArrayIndex(interp, args, 0);
    const char *zSql;
    Jsi_DString dStr = {};
    DbEvalContext sEval = {};
    zSql = Jsi_ValueGetDString(interp, vSql, &dStr, 0);

    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    sEval.nocache = jdb->queryOpts.nocache;
    if (dbEvalInit(interp, &sEval, jdb, zSql, &sStr, 0, 0) != JSI_OK)
        return JSI_ERROR;
    sEval.ret = *ret;
    int cnt = 0;


    if( JSI_OK==(rc = dbEvalStep(&sEval)) ) {
        sqlite3_stmt *pStmt = sEval.pPreStmt->pStmt;
        int nCol = sqlite3_column_count(pStmt);
        if (nCol>0)
            cnt++;
    }
    dbEvalFinalize(&sEval);
    if( rc==JSI_BREAK ) {
        rc = JSI_OK;
    }
    Jsi_DSFree(&dStr);
    Jsi_ValueMakeBool(interp, ret, cnt);
    return rc;
}

static Jsi_RC SqliteFilenameCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                             Jsi_Value **ret, Jsi_Func *funcPtr)
{
#if (SQLITE_VERSION_NUMBER>3007009)
    const char *zName = "main";
    int argc = Jsi_ValueGetLength(interp, args);
    Jsi_Db *jdb;

    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    if (argc)
        zName = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    zName = sqlite3_db_filename(jdb->db, zName);
    if (zName)
        Jsi_ValueMakeStringDup(interp, ret, zName);
#endif
    return JSI_OK;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
static SqlFunc *dbFindSqlFunc(Jsi_Db *jdb, const char *zName) {
    SqlFunc *p, *pNew;
    int i;
    pNew = (SqlFunc*)Jsi_Calloc(1, sizeof(*pNew) + Jsi_Strlen(zName) + 1 );
    pNew->sig = SQLITE_SIG_FUNC;
    pNew->zName = (char*)&pNew[1];
    for(i=0; zName[i]; i++) {
        pNew->zName[i] = tolower(zName[i]);
    }
    pNew->zName[i] = 0;
    for(p=jdb->pFunc; p; p=p->pNext) {
        if( Jsi_Strcmp(p->zName, pNew->zName)==0 ) {
            Jsi_Free((char*)pNew);
            return p;
        }
    }
    pNew->interp = jdb->interp;
    pNew->pScript = 0;
    Jsi_DSInit(&pNew->dScript);
    pNew->pNext = jdb->pFunc;
    jdb->pFunc = pNew;
    return pNew;
}

static Jsi_RC SqliteFunctionCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                             Jsi_Value **ret, Jsi_Func *funcPtr)
{
    SqlFunc *pFunc;
    Jsi_Value *tocall, *nVal;
    char *zName;
    int rc, nArg = -1, argc;
    argc = Jsi_ValueGetLength(interp, args);
    Jsi_Db *jdb;

    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;

    zName = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    tocall = Jsi_ValueArrayIndex(interp, args, 1);
    if (zName == NULL) 
        return Jsi_LogError("expected name");
    if (!Jsi_ValueIsFunction(interp, tocall)) 
        return Jsi_LogError("expected function");
    if (argc == 3) {
        nVal = Jsi_ValueArrayIndex(interp, args, 2);
        if (Jsi_GetIntFromValue(interp, nVal, &nArg) != JSI_OK)
            return JSI_ERROR;
    } else {
        Jsi_FunctionArguments(interp, tocall, &nArg);
    }
    if (nArg > SQLITE_LIMIT_FUNCTION_ARG) 
        return Jsi_LogError("to many args");
    /*  if( argc==6 ){
        const char *z = Jsi_GetString(objv[3]);
        int n = Jsi_Strlen(z);
        if( n>2 && strncmp(z, "-argcount",n)==0 ){
          if( Jsi_GetIntFromObj(interp, objv[4], &nArg) ) return JSI_ERROR;
          if( nArg<0 )
              return Jsi_LogError( "number of arguments must be non-negative");
        }
        pScript = objv[5];
      }else if( argc!=4 ){
        Jsi_WrongNumArgs(interp, 2, objv, "NAME [-argcount N] SCRIPT");
        return JSI_ERROR;
      }else{
        pScript = objv[3];
      }*/
    pFunc = dbFindSqlFunc(jdb, zName);
    if( pFunc==0 ) return JSI_ERROR;
    SQLSIGASSERT(pFunc,FUNC);

    pFunc->tocall = tocall;
    Jsi_IncrRefCount(interp, pFunc->tocall);
    rc = sqlite3_create_function(jdb->db, zName, nArg, SQLITE_UTF8,
                                 pFunc, jsiSqlFunc, 0, 0);
                                 
    if( rc!=SQLITE_OK ) {
        rc = JSI_ERROR;
        Jsi_LogError("function create error: %s", (char *)sqlite3_errmsg(jdb->db));
    }
    return JSI_OK;
}


static Jsi_RC SqliteInterruptCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                              Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Db *jdb;
    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    sqlite3_interrupt(jdb->db);
    return JSI_OK;
}


static Jsi_RC SqliteCompleteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                             Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Db *jdb;
    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *s = Jsi_ValueArrayIndex(interp, args, 0);
    const char *str =  Jsi_ValueString(interp, s, NULL);
    int isComplete = 0;
    if (str)
        isComplete = sqlite3_complete( str );
    Jsi_ValueMakeBool(interp, ret, isComplete);
    return JSI_OK;
}

#define FN_restore JSI_INFO("\
   db.restore(FILENAME, ?,DATABASE? ) \
\n\
Open a database file named FILENAME.  Transfer the content \
of FILENAME into the local database DATABASE (default: 'main').")

static Jsi_RC SqliteRestoreCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                            Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Db *jdb;
    const char *zSrcFile;
    const char *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    int nTimeout = 0;
    int rc;

    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *vFile = Jsi_ValueArrayIndex(interp, args, 0);
    int argc = Jsi_ValueGetLength(interp, args);
    if( argc==1 ) {
        zDestDb = "main";
    } else {
        zDestDb = Jsi_ValueArrayIndexToStr(interp, args, 1, NULL);
    }
    Jsi_DString dStr = {};
    if (!vFile)
        zSrcFile = ":memory:";
    else {
        zSrcFile = Jsi_ValueNormalPath(interp, vFile, &dStr);
        if (zSrcFile == NULL) 
            return Jsi_LogError("bad or missing file name");
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc, SQLITE_OPEN_READONLY, 0);

    if( rc!=SQLITE_OK ) {
        Jsi_LogError("cannot open source database: %s", sqlite3_errmsg(pSrc));
        DbClose(pSrc);
        Jsi_DSFree(&dStr);
        return JSI_ERROR;
    }
    pBackup = sqlite3_backup_init(jdb->db, zDestDb, pSrc, "main");
    if( pBackup==0 ) {
        Jsi_LogError("restore failed: %s", sqlite3_errmsg(jdb->db));
        DbClose(pSrc);
        Jsi_DSFree(&dStr);
        return JSI_ERROR;
    }
    while( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
            || rc==SQLITE_BUSY ) {
        if( rc==SQLITE_BUSY ) {
            if( nTimeout++ >= 3 ) break;
            sqlite3_sleep(100);
        }
    }
    sqlite3_backup_finish(pBackup);
    Jsi_RC rv;
    if( rc==SQLITE_DONE ) {
        rv = JSI_OK;
    } else if( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ) {
        Jsi_LogError("restore failed: source database busy");
        rv = JSI_ERROR;
    } else {
        Jsi_LogError("restore failed: %s", sqlite3_errmsg(jdb->db));
        rv = JSI_ERROR;
    }
    Jsi_DSFree(&dStr);
    DbClose(pSrc);
    return rv;
}

#define FN_transaction JSI_INFO("\
Start a new transaction (if we are not already in the midst of a \
transaction) and execute the JS function FUNC.\n\
After FUNC completes, either commit the transaction or roll it back if FUNC throws an exception.\n\
Or if no new transation was started, do nothing. \
pass the exception on up the stack.")
static Jsi_RC SqliteTransactionCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                                Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int rc;
    Jsi_Db *jdb;

    int argc = Jsi_ValueGetLength(interp, args);
    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;

    Jsi_Value *pScript;
    const char *zBegin = "SAVEPOINT _jsi_transaction";

    if( jdb->nTransaction==0 && argc==2 ) {
        Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
        static const char *TTYPE_strs[] = {
            "deferred",   "exclusive",  "immediate", 0
        };
        enum TTYPE_enum {
            TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
        };
        int ttype;
        if( Jsi_ValueGetIndex(interp, arg, TTYPE_strs, "transaction type",
                              0, &ttype) ) {
            return JSI_ERROR;
        }
        switch( (enum TTYPE_enum)ttype ) {
        case TTYPE_DEFERRED:    /* no-op */
            ;
            break;
        case TTYPE_EXCLUSIVE:
            zBegin = "BEGIN EXCLUSIVE";
            break;
        case TTYPE_IMMEDIATE:
            zBegin = "BEGIN IMMEDIATE";
            break;
        }
    }
    pScript = Jsi_ValueArrayIndex(interp, args, argc-1);
    if(!Jsi_ValueIsFunction(interp, pScript)) 
        return Jsi_LogError("expected function");

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    jdb->disableAuth++;
    rc = sqlite3_exec(jdb->db, zBegin, 0, 0 ,0);
    jdb->disableAuth--;
    if( rc!=SQLITE_OK ) 
        return Jsi_LogError("%s", sqlite3_errmsg(jdb->db));
    jdb->nTransaction++;

    /* Evaluate the function , then
    ** call function dbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    Jsi_RC rv = Jsi_FunctionInvoke(interp, pScript, NULL, NULL, NULL);
    rv = dbTransPostCmd(jdb, interp, rv);
    return rv;
}

#define FN_backup JSI_INFO("\
Open or create a database file named FILENAME.\n\
Transfer the content of local database DATABASE (default: 'main') into the \
FILENAME database.")

static Jsi_RC SqliteBackupCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                           Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Db *jdb;
    Jsi_RC rv = JSI_OK;
    int rc;
    const char *zDestFile;
    const char *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *vFile = Jsi_ValueArrayIndex(interp, args, 0);
    int argc = Jsi_ValueGetLength(interp, args);
    if( argc==1 ) {
        zSrcDb = "main";
    } else {
        zSrcDb = Jsi_ValueArrayIndexToStr(interp, args, 1, NULL);
    }
    Jsi_DString dStr = {};
    if (!vFile)
        zDestFile = ":memory:";
    else {
        zDestFile = Jsi_ValueNormalPath(interp, vFile, &dStr);
        if (zDestFile == NULL) 
            return Jsi_LogError("bad or missing file name");
    }
    rc = sqlite3_open(zDestFile, &pDest);
    if( rc!=SQLITE_OK ) {
        Jsi_LogError("cannot open target database %s: %s", zDestFile, sqlite3_errmsg(pDest));
        DbClose(pDest);
        Jsi_DSFree(&dStr);
        return JSI_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", jdb->db, zSrcDb);
    if( pBackup==0 ) {
        Jsi_LogError("backup failed: %s", sqlite3_errmsg(pDest));
        DbClose(pDest);
        Jsi_DSFree(&dStr);
        return JSI_ERROR;
    }
    while(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ) {}
    sqlite3_backup_finish(pBackup);
    if( rc==SQLITE_DONE ) {
        rv = JSI_OK;
    } else {
        Jsi_LogError("backup failed: %s", sqlite3_errmsg(pDest));
        rv = JSI_ERROR;
    }
    Jsi_DSFree(&dStr);
    DbClose(pDest);
    return rv;
}

void dbSetupCallbacks(Jsi_Db *jdb, Jsi_Db *ojdb)
{
    if (jdb->onAuth && (!ojdb || !ojdb->onAuth) )
        sqlite3_set_authorizer(jdb->db, dbAuthCallback, jdb);
    else
        sqlite3_set_authorizer(jdb->db, 0, 0);

    if (jdb->onCommit && (!ojdb || !ojdb->onCommit) )
        sqlite3_commit_hook(jdb->db, dbCommitHandler, jdb);
    else
        sqlite3_commit_hook(jdb->db, 0, 0);

    if (jdb->onBusy && (!ojdb || !ojdb->onBusy) )
        sqlite3_busy_handler(jdb->db, dbBusyHandler, jdb);
    else
        sqlite3_busy_handler(jdb->db, 0, 0);
    
    if (jdb->onTrace && (!ojdb || !ojdb->onTrace) )
        sqlite3_trace(jdb->db, dbTraceHandler, jdb);
    else
        sqlite3_trace(jdb->db, 0, 0);

    if (jdb->onNeedCollate && (!ojdb || !ojdb->onNeedCollate) )
        sqlite3_collation_needed(jdb->db, jdb, dbCollateNeeded);
    else
        sqlite3_collation_needed(jdb->db, 0, 0);

    if (jdb->onUpdate && (!ojdb || !ojdb->onUpdate) )
        sqlite3_update_hook(jdb->db, dbUpdateHandler, jdb);
    else
        sqlite3_update_hook(jdb->db, 0, 0);

    if (jdb->onWalHook && (!ojdb || !ojdb->onWalHook) )
        sqlite3_wal_hook(jdb->db, dbWalHandler, jdb);
    else
        sqlite3_wal_hook(jdb->db, 0, 0);

    if (jdb->onRollback && (!ojdb || !ojdb->onRollback) )
        sqlite3_rollback_hook(jdb->db, dbRollbackHandler, jdb);
    else
        sqlite3_rollback_hook(jdb->db, 0, 0);

    if (jdb->onProfile && (!ojdb || !ojdb->onProfile) )
        sqlite3_profile(jdb->db, dbProfileHandler, jdb);
    else
        sqlite3_profile(jdb->db, 0, 0);

    if (jdb->onProgress && jdb->progressSteps && (!ojdb || !ojdb->onProgress || ojdb->progressSteps != jdb->progressSteps) )
        sqlite3_progress_handler(jdb->db, jdb->progressSteps, dbProgressHandler, jdb);
    else
        sqlite3_progress_handler(jdb->db, 0, 0, 0);
    
    if (!ojdb || jdb->load != ojdb->load)
        sqlite3_enable_load_extension(jdb->db, jdb->load);

    if (!ojdb || jdb->timeout != ojdb->timeout)
        sqlite3_busy_timeout( jdb->db, jdb->timeout );

/*    if (jdb->onUnlock && (!ojdb || !ojdb->onUnlock) )
        sqlite3_unlock_notify(jdb->db, dbUnlockNotify, (void*)jdb);
    else
        sqlite3_unlock_notify(jdb->db, 0, 0);
        */
}


static Jsi_RC SqliteConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                         Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Db *jdb, ojdb;
    if (!(jdb = dbGetDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *opts = Jsi_ValueArrayIndex(interp, args, 0);
    if (jdb->noConfig && opts && !Jsi_ValueIsString(interp, opts))
        return Jsi_LogError("Socket conf() is disabled for set");
    ojdb = *jdb;
    
    jdb->lastInsertId = sqlite3_last_insert_rowid(jdb->db);
    jdb->changeCnt = sqlite3_changes(jdb->db);
    jdb->changeCntAll = sqlite3_total_changes(jdb->db);
    jdb->errorCode = sqlite3_errcode(jdb->db);
    
    Jsi_RC rc = Jsi_OptionsConf(interp, SqlOptions, jdb, opts, ret, 0);
    
    if (jdb->stmtCacheMax<0 || jdb->stmtCacheMax>MAX_PREPARED_STMTS) {
        JSI_DBQUERY_PRINTF( "option stmtCacheMax value %d is not in range 0..%d", jdb->stmtCacheMax, MAX_PREPARED_STMTS);
        jdb->stmtCacheMax = ojdb.stmtCacheMax;
        rc = JSI_ERROR;
    }
    dbSetupCallbacks(jdb, &ojdb);
    dbPrepStmtLimit(jdb);
    return rc;
}

static Jsi_CmdSpec sqliteCmds[] = {
    { "Sqlite",         SqliteConstructor,      0,  2,  "file:null|string=void, options:object=void",
        .help="Create a new db connection to the named file or :memory:",
        .retType=(uint)JSI_TT_USEROBJ, .flags=JSI_CMD_IS_CONSTRUCTOR, .info=0, .opts=SqlOptions },
    { "backup",         SqliteBackupCmd,        1,  2, "file:string, dbname:string='main'", .help="Backup db to file", .retType=(uint)JSI_TT_VOID, .flags=0, .info=FN_backup },
    { "collate",        SqliteCollateCmd,       2,  2, "name:string, callback:function", .help="Create new SQL collation command", .retType=(uint)JSI_TT_VOID },
    { "complete",       SqliteCompleteCmd,      1,  1, "sql:string", .help="Return true if sql is complete", .retType=(uint)JSI_TT_BOOLEAN },
    { "conf",           SqliteConfCmd,          0,  1, "options:string|object=void", .help="Configure options", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=SqlOptions },
    { "eval",           SqliteEvalCmd,          1,  1, "sql:string", .help="Run sql commands without input/output", .retType=(uint)JSI_TT_NUMBER, .flags=0, .info=FN_evaluate },
    { "exists",         SqliteExistsCmd,        1,  1, "sql:string", .help="Execute sql, and return true if there is at least one result value", .retType=(uint)JSI_TT_BOOLEAN },
    { "filename",       SqliteFilenameCmd,      0,  1, "name:string='main'", .help="Return filename for named or all attached databases", .retType=(uint)JSI_TT_STRING },
    { "func",           SqliteFunctionCmd,      2,  3, "name:string, callback:function, numArgs:number=void", .help="Register a new function with database", .retType=(uint)JSI_TT_VOID },
    { "import",         SqliteImportCmd,        2,  3, "table:string, file:string, options:object=void", .help="Import data from file into table ", .retType=(uint)JSI_TT_NUMBER, .flags=0, .info=FN_import, .opts=ImportOptions },
    { "interrupt",      SqliteInterruptCmd,     0,  0, "", .help="Interrupt in progress statement", .retType=(uint)JSI_TT_VOID },
    { "onecolumn",      SqliteOnecolumnCmd,     1,  1, "sql:string", .help="Execute sql, and return a single value", .retType=(uint)JSI_TT_ANY },
    { "query",          SqliteQueryCmd,         1,  2, "sql:string, options:function|string|array|object=void", .help="Evaluate an sql query with bindings", .retType=(uint)JSI_TT_ANY, .flags=0, .info=FN_sqlexec, .opts=ExecFmtOptions },
    { "restore",        SqliteRestoreCmd,       1,  2, "file:string, dbname:string", .help="Restore db from file (default db is 'main')", .retType=(uint)JSI_TT_VOID, .flags=0, .info=FN_restore },
    { "transaction",    SqliteTransactionCmd,   1,  2, "callback:function, type:string=void", .help="Call function inside db tranasaction. Type is: 'deferred', 'exclusive', 'immediate'", .retType=(uint)JSI_TT_VOID, .flags=0, .info=FN_transaction },
    { NULL, 0,0,0,0, .help="Commands for accessing sqlite databases" }
};

//static Jsi_CmdSpec sqliteCmds[];


static Jsi_UserObjReg sqliteobject = {
    .name   = "Sqlite",
    .spec   = sqliteCmds,
    .freefun= sqliteObjFree,
    .istrue = sqliteObjIsTrue,
    .isequ  = sqliteObjEqual
};

/**   new Sqlite(FILENAME,?-vfs VFSNAME?,?-key KEY?,?-readonly BOOLEAN?,
**                           ?-create BOOLEAN?,?-nomutex BOOLEAN?)
**
** This is the sqlite constructior called  using "new Sqlite".
**
** The first argument is the name of the database file.
**
*/

static Jsi_RC SqliteConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                             Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_RC rc = JSI_ERROR;
    /* void *cd = clientData; */
    int  flags;
    char *zErrMsg;
    const char *zFile = NULL, *vfs = 0;
    /* In normal use, each JSI interpreter runs in a single thread.  So
    ** by default, we can turn of mutexing on SQLite database connections.
    ** However, for testing purposes it is useful to have mutexes turned
    ** on.  So, by default, mutexes default off.  But if compiled with
    ** SQLITE_JSI_DEFAULT_FULLMUTEX then mutexes default on.
    */
    flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
#ifdef SQLITE_JSI_DEFAULT_FULLMUTEX
    flags |= SQLITE_OPEN_FULLMUTEX;
#else
    flags |= SQLITE_OPEN_NOMUTEX;
#endif

    Jsi_Value *vFile = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_DString dStr = {};
    int ismem = 0, create = 0;
    Jsi_Obj *fobj;
    Jsi_Value *toacc;
    const char *vf;
    const char *dbname = NULL;
    
    if (vFile==NULL || Jsi_ValueIsNull(interp, vFile) ||
        ((vf = Jsi_ValueString(interp, vFile, NULL)) && !Jsi_Strcmp(vf,":memory:"))) {
        zFile = ":memory:";
        ismem = 1;
    } else {
        zFile = Jsi_ValueNormalPath(interp, vFile, &dStr);
        if (zFile == NULL) 
            return Jsi_LogError("bad or missing file name");
        Jsi_StatBuf st = {};
        st.st_uid = -1;
        create = (Jsi_Lstat(interp, vFile, &st) != 0);
    }
    zErrMsg = 0;
    Jsi_Db *db = (Jsi_Db*)Jsi_Calloc(1, sizeof(*db) );
    if( db==0 ) {
        Jsi_DSFree(&dStr);
        Jsi_LogError("malloc failed");
        return JSI_ERROR;
    }
    db->sig = SQLITE_SIG_DB;
    db->_ = &dbObjCmd;
    db->_->newCnt++;
    db->_->activeCnt++;
    db->stmtCacheMax = NUM_PREPARED_STMTS;
    db->hasOpts = 1;
    if ((arg != NULL && !Jsi_ValueIsNull(interp,arg))
        && Jsi_OptionsProcess(interp, SqlOptions, db, arg, 0) < 0) {
        rc = JSI_ERROR;
        goto bail;
    }
    if (ismem == 0 &&
        (Jsi_InterpAccess(interp, vFile, (db->readonly ? JSI_INTACCESS_READ : JSI_INTACCESS_WRITE)) != JSI_OK
        || (create && Jsi_InterpAccess(interp, vFile, JSI_INTACCESS_CREATE) != JSI_OK))) {
        Jsi_LogError("Safe accces denied");
        goto bail;
    }

    if (db->stmtCacheMax<0 || db->stmtCacheMax>MAX_PREPARED_STMTS) {
        Jsi_LogError("option stmtCacheMax value %d is not in range 0..%d", db->stmtCacheMax, MAX_PREPARED_STMTS);
        goto bail;
    }
    if (!db->udata) {
        db->udata = Jsi_ValueNewObj(interp, NULL);
        Jsi_IncrRefCount(interp, db->udata);
    }
    if (db->readonly) {
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
    } else {
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
        if (db->noCreate) {
            flags &= ~SQLITE_OPEN_CREATE;
        }
    }
    if (db->vfs)
        vfs = Jsi_ValueToString(interp, db->vfs, NULL);
    if(db->mutex == MUTEX_NONE) {
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
    } else {
        flags &= ~SQLITE_OPEN_NOMUTEX;
    }
    if(db->mutex ==MUTEX_FULL) {
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
    } else {
        flags &= ~SQLITE_OPEN_FULLMUTEX;
    }
  
    if (SQLITE_OK != sqlite3_open_v2(zFile, &db->db, flags, vfs)) {
        Jsi_LogError("db open failed: %s", zFile);
        goto bail;
    }
    //Jsi_DSFree(&translatedFilename);

    if( SQLITE_OK!=sqlite3_errcode(db->db) ) {
        zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(db->db));
        DbClose(db->db);
        db->db = 0;
    }
    if( db->db==0 ) {
        sqlite3_free(zErrMsg);
        goto bail;
    }
;
    toacc = NULL;
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        toacc = _this;
    } else {
        Jsi_Obj *o = Jsi_ObjNew(interp);
        Jsi_PrototypeObjSet(interp, "Sqlite", o);
        Jsi_ValueMakeObject(interp, ret, o);
        toacc = *ret;
    }
    sqlite3_create_function(db->db, "unixtime", -1, SQLITE_UTF8, db, jsiSqlFuncUnixTime, 0, 0);
    
    fobj = Jsi_ValueGetObj(interp, toacc /* constructor obj*/);
    if ((db->objId = Jsi_UserObjNew(interp, &sqliteobject, fobj, db))<0)
        goto bail;
    db->stmtHash = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    db->fobj = fobj;
    //dbSys->cnt = Jsi_UserObjCreate(interp, sqliteobject.name /*dbSys*/, fobj, db);
    db->interp = interp;
    db->optPtr = &db->queryOpts;
    db->stmtCache = Jsi_ListNew((Jsi_Interp*)db, 0, dbStmtFreeProc);
    rc = JSI_OK;
    dbname = Jsi_DSValue(&db->name);
    if (dbname[0])
        sqlite3_db_config(db->db, SQLITE_DBCONFIG_MAINDBNAME, dbname);
    Jsi_JSONParseFmt(interp, &db->version, "{libVer:\"%s\", hdrVer:\"%s\", hdrNum:%d, hdrSrcId:\"%s\", pkgVer:%d}",
        (char *)sqlite3_libversion(), SQLITE_VERSION, SQLITE_VERSION_NUMBER, SQLITE_SOURCE_ID, jsi_DbPkgVersion); 
    dbSetupCallbacks(db, NULL);
    
bail:
    if (rc != JSI_OK) {
        if (db->hasOpts)
            Jsi_OptionsFree(interp, SqlOptions, db, 0);
        db->_->activeCnt--;
        Jsi_Free(db);
    }
    Jsi_DSFree(&dStr);
    Jsi_ValueMakeUndef(interp, ret);
    return rc;
}

#endif

typedef struct {
    Jsi_CDataDb *binds;
    Jsi_StructSpec *rowidPtr, *dirtyPtr;
    int optLen;             /* Length of binds[0].binds */
} OptionBind;

const char *jsi_DbOptionTypeStr(Jsi_OptionId typ, bool cname)
{
    const Jsi_OptionTypedef* ti = Jsi_OptionTypeInfo(typ);
    if (ti)
        return (cname?ti->cName:ti->idName);
    return NULL;
}

static Jsi_RC dbBindOptionStmt(Jsi_Db *jdb, sqlite3_stmt *pStmt, OptionBind *obPtr,
                            int dataIdx, int bindMax, Jsi_CDataDb *dbopts)
{
    Jsi_Interp *interp = jdb->interp;
    int j, k, cnt = 0, idx, sidx = -1, rc = 0;
    Jsi_StructSpec *specPtr, *specs;
    void *rec;
    Jsi_DString *eStr;
    const char *bName;
    int lastBind = sqlite3_bind_parameter_count(pStmt);
    if (lastBind<=0)
        return JSI_OK;
    int structSize = 0;
    Jsi_Wide flags = 0;
    sqlite3_destructor_type statFlags = ((dbopts->noStatic)?SQLITE_TRANSIENT:SQLITE_STATIC);
    specPtr = dbopts[0].sf;
    structSize = specPtr[obPtr->optLen].size;
    
    for (j=1; j<=lastBind; j++) {
        bName = sqlite3_bind_parameter_name(pStmt, j);
        if (bName==NULL || bName[0]==0 || bName[1]==0)
            continue;
        idx = j;
        if (dbopts[0].prefix==0)
            k = 0;
        else {
            for (k=0; dbopts[k].sf; k++) {
                if (bName[0] == dbopts[k].prefix)
                    break;
            }
            if (bindMax>0 && k>=bindMax)
                continue;
            if (!dbopts[k].sf) {
                Jsi_LogError("bad bind: %s", bName);
                continue;
            }
        }
        specs = dbopts[k].sf;
        rec = dbopts[k].data;
        if (k==0) {
            if (dbopts->isMap) {
                Jsi_Map *map = *(typeof(map)*)rec;
                rec = Jsi_MapEntryFind(map, (void*)(intptr_t)dataIdx);
                if (!rec) return JSI_ERROR;
            } else if (dbopts->isPtrs)
                rec = ((void**)rec)[dataIdx];
            else
                rec = ((uchar*)rec)+ (dataIdx * structSize);
        }
        if (bName[0] == '?')
            sidx = atoi(bName+1);
        for (specPtr = specs, cnt=1; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END; specPtr++, cnt++) {
            if (specPtr->flags&JSI_OPT_DB_IGNORE)
                continue;
            if (bName[0] == '?') {
                if (cnt == sidx)
                    break;
            } else {
                const char *sName = specPtr->name;
                if (bName[1] == sName[0] && !Jsi_Strcmp(bName+1, sName))
                    break;
            }
        }
        if (specPtr->id<JSI_OPTION_BOOL || specPtr->id>=JSI_OPTION_END) 
            return Jsi_LogError("unknown bind: %s", bName);

        char *ptr = (char *)rec + specPtr->offset;
        switch (specPtr->id) {
        case JSI_OPTION_BOOL:
            rc = sqlite3_bind_int(pStmt, idx, *(int*)ptr);
            break;
        case JSI_OPTION_INT: rc = sqlite3_bind_int64(pStmt, idx, *(int*)ptr); break;
        case JSI_OPTION_UINT:rc = sqlite3_bind_int64(pStmt, idx, *(uint*)ptr); break;
        case JSI_OPTION_INT8: rc = sqlite3_bind_int64(pStmt, idx, *(int8_t*)ptr); break;
        case JSI_OPTION_UINT8:rc = sqlite3_bind_int64(pStmt, idx, *(uint8_t*)ptr); break;
        case JSI_OPTION_INT16: rc = sqlite3_bind_int64(pStmt, idx, *(int16_t*)ptr); break;
        case JSI_OPTION_UINT16:rc = sqlite3_bind_int64(pStmt, idx, *(uint16_t*)ptr); break;
        case JSI_OPTION_INT32: rc = sqlite3_bind_int64(pStmt, idx, *(int32_t*)ptr); break;
        case JSI_OPTION_UINT32:rc = sqlite3_bind_int64(pStmt, idx, *(uint32_t*)ptr); break;
        case JSI_OPTION_TIME_W:
        case JSI_OPTION_INT64: rc = sqlite3_bind_int64(pStmt, idx, *(int64_t*)ptr); break;
        case JSI_OPTION_UINT64:rc = sqlite3_bind_int64(pStmt, idx, *(uint64_t*)ptr); break;
        case JSI_OPTION_USHORT:rc = sqlite3_bind_int64(pStmt, idx, *(ushort*)ptr); break;
        case JSI_OPTION_SHORT:rc = sqlite3_bind_int64(pStmt, idx, *(short*)ptr); break;
        case JSI_OPTION_LONG:rc = sqlite3_bind_int64(pStmt, idx, *(long*)ptr); break;
        case JSI_OPTION_ULONG:rc = sqlite3_bind_int64(pStmt, idx, *(ulong*)ptr); break;
        case JSI_OPTION_INTPTR_T:rc = sqlite3_bind_int64(pStmt, idx, *(intptr_t*)ptr); break;
        case JSI_OPTION_UINTPTR_T:rc = sqlite3_bind_int64(pStmt, idx, *(uintptr_t*)ptr); break;
        case JSI_OPTION_SIZE_T:rc = sqlite3_bind_int64(pStmt, idx, *(size_t*)ptr); break;
        case JSI_OPTION_SSIZE_T:rc = sqlite3_bind_int64(pStmt, idx, *(ssize_t*)ptr); break;
        case JSI_OPTION_LDOUBLE:rc = sqlite3_bind_int64(pStmt, idx, *(ldouble*)ptr); break;
        case JSI_OPTION_FLOAT:rc = sqlite3_bind_int64(pStmt, idx, *(float*)ptr); break;
        case JSI_OPTION_TIME_T:
            rc = sqlite3_bind_int64(pStmt, idx, (Jsi_Wide)*(time_t*)ptr);
            break;
        case JSI_OPTION_NUMBER:
            rc = sqlite3_bind_double(pStmt, idx, (double)*(Jsi_Number*)ptr);
            break;
        case JSI_OPTION_TIME_D:
        case JSI_OPTION_DOUBLE:
            rc = sqlite3_bind_double(pStmt, idx, (double)*(Jsi_Number*)ptr);
            break;
        case JSI_OPTION_CUSTOM: {
            Jsi_OptionCustom* cust = Jsi_OptionCustomBuiltin(specPtr->custom);
            if (cust && cust->formatProc) {
                Jsi_DString dStr;
                Jsi_DSInit(&dStr);
                if ((*cust->formatProc)(interp, (Jsi_OptionSpec*)specPtr, NULL, &dStr, rec, flags) != JSI_OK) {
                    Jsi_DSFree(&dStr);
                    return JSI_ERROR;
                }
                rc = sqlite3_bind_text(pStmt, idx, Jsi_DSValue(&dStr), -1, SQLITE_TRANSIENT );
                Jsi_DSFree(&dStr);
            } else 
                return Jsi_LogError("missing or invalid custom for \"%s\"", specPtr->name);
            break;
        }
        case JSI_OPTION_DSTRING:
            eStr = (Jsi_DString*)ptr;
            if (jdb->optPtr->nullvalue && !Jsi_Strcmp(jdb->optPtr->nullvalue, Jsi_DSValue(eStr)))
                rc = sqlite3_bind_text(pStmt, idx, NULL, -1, statFlags );
            else
                rc = sqlite3_bind_text(pStmt, idx, Jsi_DSValue(eStr), -1, statFlags );
            break;
        case JSI_OPTION_STRBUF:
            if (jdb->optPtr->nullvalue && ptr && !Jsi_Strcmp(jdb->optPtr->nullvalue, (char*)ptr))
                rc = sqlite3_bind_text(pStmt, idx, NULL, -1, statFlags );
            else
                rc = sqlite3_bind_text(pStmt, idx, (char*)ptr, -1, statFlags );
            break;
        case JSI_OPTION_STRKEY:
            rc = sqlite3_bind_text(pStmt, idx, *(char**)ptr, -1, SQLITE_STATIC );
            break;
#ifndef JSI_LITE_ONLY
        case JSI_OPTION_STRING:
            rc = sqlite3_bind_text(pStmt, idx, Jsi_ValueString(interp, *((Jsi_Value **)ptr), NULL), -1, statFlags );
            break;
#else
        case JSI_OPTION_STRING:
#endif
        case JSI_OPTION_VALUE: /* Unsupported. */
        case JSI_OPTION_VAR:
        case JSI_OPTION_OBJ:
        case JSI_OPTION_ARRAY:
        case JSI_OPTION_REGEXP:
        case JSI_OPTION_FUNC:
#ifdef __cplusplus
        case JSI_OPTION_END:
        case JSI_OPTION_USEROBJ:
#else
        default:
#endif
            Jsi_LogError("unsupported jdb option type \"%s\" for \"%s\"", jsi_DbOptionTypeStr(specPtr->id, 0), specPtr->name);
            return JSI_ERROR;

        }
        if (rc != SQLITE_OK)
            Jsi_LogError("bind failure: %s", sqlite3_errmsg(jdb->db));
    }
    cnt++;
    return JSI_OK;
}

/* Prepare, bind, then step.
 * If there are results return JSI_OK. On error return JSI_ERROR;
 */
static Jsi_RC dbEvalStepOption(DbEvalContext *p, OptionBind *obPtr, int *cntPtr, int dataIdx, int bindMax, Jsi_CDataDb *dbopts, int *erc) {
    Jsi_Db *jdb = p->jdb;
    int cnt = 0;
    while( p->zSql[0] || p->pPreStmt ) {
        Jsi_RC rc;
        cnt++;
        if( p->pPreStmt==0 ) {
            rc = dbPrepareStmt(p->jdb, p->zSql, &p->zSql, &p->pPreStmt);
            if( rc!=JSI_OK ) return rc;
        }
        if (bindMax!=0) {
            rc = dbBindOptionStmt(jdb, p->pPreStmt->pStmt, obPtr, dataIdx, bindMax, dbopts);
            if( rc!=JSI_OK ) return rc;
        }
        rc = dbEvalStepSub(p, 1, erc);
        if (rc != JSI_BREAK)
            return rc;
        *cntPtr = cnt;
    }
    
    /* Finished */
    return JSI_BREAK;
}

static Jsi_StructSpec* dbLookupSpecFromName(Jsi_StructSpec *specs, const char *name) {
    Jsi_StructSpec *specPtr = NULL;
    for (specPtr = specs; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END; specPtr++) {
        if  (specPtr->flags&JSI_OPT_DB_IGNORE)
            continue;
        const char *cname = specPtr->name;
        if (cname[0] == name[0] && !Jsi_Strncasecmp(cname, name, -1))
            return specPtr;
    }
    return NULL;
}

const char* Jsi_DbKeyAdd(Jsi_Db *jdb, const char *str)
{
#ifndef JSI_LITE_ONLY
    if (jdb->interp)
        return Jsi_KeyAdd(jdb->interp, str);
#endif
    Jsi_HashEntry *hPtr;
    bool isNew;
    hPtr = Jsi_HashEntryNew(jdb->strKeyTbl, str, &isNew);
    assert(hPtr) ;
    return (const char*)Jsi_HashKeyGet(hPtr);
}

static int dbOptSelect(Jsi_Db *jdb, const char *cmd, OptionBind *obPtr, Jsi_CDataDb *dbopts)
{
    void *rec = dbopts[0].data, **recPtrPtr = NULL;
    Jsi_Interp *interp = jdb->interp;
    Jsi_StructSpec *specPtr, *specs = dbopts[0].sf;
    DbEvalContext sEval = {};
    int ccnt = 0;
    Jsi_Wide flags = 0;
    const char *cPtr = Jsi_Strstr(cmd, " %s");
    if (!cPtr) cPtr = Jsi_Strstr(cmd, "\t%s");
    Jsi_DString *eStr;
#ifdef JSI_DB_DSTRING_SIZE
    JSI_DSTRING_VAR(dStr, JSI_DB_DSTRING_SIZE);
#else
    Jsi_DString sStr, *dStr = &sStr;
    Jsi_DSInit(dStr);
#endif
    dbEvalInit(interp, &sEval, jdb, NULL, dStr, 0, 0);
    if (dbopts->noCache)
        sEval.nocache = 1;
    Jsi_DSAppendLen(dStr, cmd, cPtr?(cPtr-cmd):-1);
    if (cPtr) {
        Jsi_DSAppend(dStr, " ", NULL);
        for (specPtr = specs; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END; specPtr++) {
            if (specPtr == obPtr->dirtyPtr || (specPtr->flags&JSI_OPT_DB_IGNORE))
                continue;
            if (ccnt)
                Jsi_DSAppendLen(dStr, ",", 1);
            Jsi_DSAppend(dStr, "[", specPtr->name, "]", NULL);
            ccnt++; 
        }
        Jsi_DSAppend(dStr, cPtr+3, NULL);
    }
    sEval.zSql = Jsi_DSValue(dStr);
    sEval.nocache = jdb->optPtr->nocache;
    int rc = JSI_ERROR, erc = -1, structSize = 0;
    int cnt = 0, dataMax = (dbopts->isPtr2?0:1);
    int multi = (dbopts->isPtr2!=0);
    int dnum = dbopts[0].arrSize;
    if (dnum<=0 && !dbopts->isPtr2) {
        dataMax = dnum = 1;
    }
    if (dnum>1) {
        multi = 1;
        dataMax = dbopts[0].arrSize;
    }
    if (dbopts->isPtr2) {
        recPtrPtr = (void**)rec; /* This is really a void***, but this gets recast below. */
        rec = *recPtrPtr;
    }
    structSize = specs[obPtr->optLen].size;

    cnt = 0;
    int ncnt = 0, bindMax = -1, dataIdx = -1;
    while(1) {
        dataIdx++;
        if (dataIdx>=dataMax) {
            if (!dbopts->isPtr2)
                break;
            else {
            /* Handle fully dynamic allocation of memory. */
#ifndef JSI_DB_MAXDYN_SIZE
#define JSI_DB_MAXDYN_SIZE 100000000
#endif
#ifndef JSI_DB_DYN_INCR
#define JSI_DB_DYN_INCR 16
#endif
                int ddMax = (dbopts->maxSize>0?dbopts->maxSize:JSI_DB_MAXDYN_SIZE);
                if (dataMax>=ddMax)
                    break;
                int olddm = dataMax;
                dataMax += JSI_DB_DYN_INCR;
                if (dataMax>ddMax)
                    dataMax = ddMax;
                if (!olddm)
                    rec = Jsi_Calloc(dataMax+1, sizeof(void*));
                else {
                    rec = Jsi_Realloc(rec, (dataMax+1)*sizeof(void*));
                    memset((char*)rec+olddm*sizeof(void*), 0, (dataMax-olddm+1)*sizeof(void*));
                }
                *recPtrPtr = rec;
            }
        }

        rc = dbEvalStepOption(&sEval, obPtr, &ncnt, dataIdx, bindMax, dbopts, &erc);
        if (rc == JSI_ERROR)
            break;
        if (rc != JSI_OK)
            break;
        cnt += ncnt;
        sqlite3_stmt *pStmt = sEval.pPreStmt->pStmt;
        int idx;
        int nCol;
        char **apColName;
        const char *str;
        int *apColType;
        void *prec = rec;
        bindMax = 0;

        if (dbopts->isPtr2 || dbopts->isPtrs) {
            prec = ((void**)rec)[dataIdx];
            if (!prec)
                ((void**)rec)[dataIdx] = prec = Jsi_Calloc(1, structSize);
        } else
                prec = (char*)rec + (dataIdx * structSize);
        dbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
        for (idx=0; idx<nCol; idx++) {
            specPtr = dbLookupSpecFromName(specs, apColName[idx]);
            if (!specPtr) {
                Jsi_LogError("unknown column name: %s", apColName[idx]);
                goto bail;
            }          
            if (specPtr->id<JSI_OPTION_BOOL || specPtr->id>=JSI_OPTION_END) {
                Jsi_LogError("unknown option type \"%d\" for \"%s\"", specPtr->id, specPtr->name);
                goto bail;
            }
            char *ptr = (char*)prec + specPtr->offset;

            switch (specPtr->id) {
                case JSI_OPTION_BOOL:
                    *(int*)ptr = sqlite3_column_int(pStmt, idx);
                    break;
                case JSI_OPTION_INT: *(int*)ptr = (int)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_UINT: *(uint*)ptr = (uint)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_INTPTR_T: *(intptr_t*)ptr = (intptr_t)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_UINTPTR_T: *(uintptr_t*)ptr = (uintptr_t)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_SIZE_T: *(size_t*)ptr = (size_t)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_SSIZE_T: *(ssize_t*)ptr = (ssize_t)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_SHORT: *(short*)ptr = (int)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_USHORT: *(ushort*)ptr = (uint)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_LONG: *(long*)ptr = (int)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_ULONG: *(ulong*)ptr = (uint)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_INT8: *(int8_t*)ptr = (int)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_UINT8: *(uint8_t*)ptr = (uint)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_INT16: *(int16_t*)ptr = (int)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_UINT16: *(uint16_t*)ptr = (uint)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_INT32: *(int32_t*)ptr = (int)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_UINT32: *(uint32_t*)ptr = (uint)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_INT64: *(int64_t*)ptr = (int)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_TIME_W:
                case JSI_OPTION_UINT64: *(uint64_t*)ptr = (uint)sqlite3_column_int64(pStmt, idx); break;
                case JSI_OPTION_TIME_T:
                    *(time_t*)ptr = (time_t)sqlite3_column_int64(pStmt, idx);
                    break;
                case JSI_OPTION_NUMBER:
                    *(Jsi_Number*)ptr = (Jsi_Number)sqlite3_column_double(pStmt, idx);
                    break;
                case JSI_OPTION_TIME_D:
                case JSI_OPTION_FLOAT:
                    *(float*)ptr = (float)sqlite3_column_double(pStmt, idx);
                    break;
                case JSI_OPTION_DOUBLE:
                    *(double*)ptr = (double)sqlite3_column_double(pStmt, idx);
                    break;
                case JSI_OPTION_LDOUBLE:
                    *(ldouble*)ptr = (ldouble)sqlite3_column_double(pStmt, idx);
                    break;
                case JSI_OPTION_DSTRING:
                    eStr = (Jsi_DString*)ptr;
                    str = (char*)sqlite3_column_text(pStmt, idx );
                    if (!str)
                        str = jdb->optPtr->nullvalue;
                    Jsi_DSSet(eStr, str?str:"");
                    break;
                case JSI_OPTION_STRBUF:
                    str = (char*)sqlite3_column_text(pStmt, idx );
                    if (!str)
                        str = jdb->optPtr->nullvalue;
                    strncpy((char*)ptr, str?str:"", specPtr->size);
                    ((char*)ptr)[specPtr->size-1] = 0;
                    break;
                case JSI_OPTION_CUSTOM: {
                    Jsi_OptionCustom* cust = Jsi_OptionCustomBuiltin(specPtr->custom);
                    if (cust && cust->parseProc) {
                        str = (char*)sqlite3_column_text(pStmt, idx );
                        if ((*cust->parseProc)(interp, (Jsi_OptionSpec*)specPtr, NULL, str, prec, flags) != JSI_OK) {
                            goto bail;
                        }
                    } else {
                        Jsi_LogError("missing or invalid custom for \"%s\"", specPtr->name);
                        goto bail;
                    }
                    break;
                }
                case JSI_OPTION_STRKEY:
                    str = (char*)sqlite3_column_text(pStmt, idx );
                    if (!str)
                        str = jdb->optPtr->nullvalue;
                    *(char**)ptr = (str?(char*)Jsi_DbKeyAdd(jdb, str):NULL);
                    break;
#ifndef JSI_LITE_ONLY
                case JSI_OPTION_STRING: {
                    Jsi_Value *vPtr = *((Jsi_Value **)ptr);
                    if (!(specPtr->flags&JSI_OPT_NO_DUPVALUE)) {
                        if (vPtr) Jsi_DecrRefCount(interp, vPtr);
                        *((Jsi_Value **)ptr) = NULL;
                    }
                    str = (char*)sqlite3_column_text(pStmt, idx );
                    if (!str)
                        str = jdb->optPtr->nullvalue;
                    if (str) {
                        vPtr = Jsi_ValueNewStringDup(interp, str);
                        *((Jsi_Value **)ptr) = vPtr;
                    }
                    break;
                }
#else
                case JSI_OPTION_STRING:        
#endif
                case JSI_OPTION_VALUE: /* The rest are unsupported. */
                case JSI_OPTION_VAR:
                case JSI_OPTION_OBJ:
                case JSI_OPTION_ARRAY:
                case JSI_OPTION_REGEXP:
                case JSI_OPTION_FUNC:
                
#ifdef __cplusplus
                case JSI_OPTION_USEROBJ:
                case JSI_OPTION_END:
#else
                default:
#endif
                    JSI_DBQUERY_PRINTF( "unsupported type: %d:%s\n", specPtr->id,
                        jsi_DbOptionTypeStr(specPtr->id, 0));
                    break;
            }
        }
        if (dbopts[0].callback)
            dbopts[0].callback(interp, dbopts, prec);
        cnt++;
        if (!multi)
            break;
    }
    dbEvalFinalize(&sEval);
    if( rc==JSI_BREAK ) {
        rc = JSI_OK;
    }
    return (rc==JSI_OK?cnt:erc);

bail:
    dbEvalFinalize(&sEval);
    return erc;
}

static int jsi_DbQuery(Jsi_Db *jdb, Jsi_CDataDb *dbopts, const char *query)
{
    int k, cnt, erc = -1;
    Jsi_CDataDb statbinds[] = {{}, {}};
    if (!dbopts) dbopts = statbinds;
    OptionBind ob = {.binds = dbopts};
    Jsi_StructSpec *specPtr, *specs;
    Jsi_Interp *interp = jdb->interp;
    if (!query) query="";
    if (query[0]==';') {
        if (!dbExecCmd(jdb, query+1, &erc)) {
            Jsi_LogError("EXEC ERROR=\"%s\", SQL=\"%s\"", sqlite3_errmsg(jdb->db), query);
            return erc;
        }
        return 0;
    }
    const char *cPtr = Jsi_Strstr(query, " %s");
    if (!cPtr) cPtr = Jsi_Strstr(query, "\t%s");
    if (!dbopts) {
        Jsi_LogError("dbopts may not be null");
        return -1;
    }
    if (!dbopts[0].data) {
        Jsi_LogError("data may not be null");
        return -1;
    }
    if (!dbopts[0].sf) {
        Jsi_LogError("specs may not be null");
        return -1;
    }
    for (k=0; dbopts[k].sf; k++) {
        if (dbopts[k].arrSize>1 || k==0) {
            int scnt = 0;
            for (specPtr = dbopts[k].sf, scnt=0; specPtr->id>=JSI_OPTION_BOOL
                && specPtr->id < JSI_OPTION_END; specPtr++, scnt++) {
                if (specPtr->flags&JSI_OPT_DB_IGNORE)
                    continue;
                if (k==0) {
                    if (specPtr->flags&JSI_OPT_DB_ROWID) {
                        if (specPtr->id != JSI_OPTION_INT64) {
                            Jsi_LogError("rowid flag must be a wide field: %s", specPtr->name);
                            return -1;
                        }
                        ob.rowidPtr = specPtr;
                    }
                    if (specPtr->flags&JSI_OPT_DB_DIRTY) {
                        if (specPtr->id == JSI_OPTION_BOOL || specPtr->id == JSI_OPTION_INT) {
                            ob.dirtyPtr = specPtr;
                        } else {
                            Jsi_LogError("dirty flag must be a int/bool field: %s", specPtr->name);
                            return -1;
                        }
                    }
                            
                }
            }
            if (k==0)
                ob.optLen = scnt;
            assert(specPtr->id == JSI_OPTION_END);
        }
        if (!dbopts[k].prefix) break;
    }
    specs = dbopts[0].sf;
    int structSize = specs[ob.optLen].size;
    if (dbopts->memClear || dbopts->memFree) {
        cnt = dbopts[0].arrSize;
        void *rec = dbopts[0].data, *prec = rec;
        void **recPtrPtr = NULL;
        if (dbopts->isPtr2) {
            recPtrPtr = (void**)rec; /* This is really a void***, but this gets recast below. */
            rec = *recPtrPtr;
        }
        if (cnt<=0 && rec && dbopts->isPtr2) {
            for (cnt=0; ((void**)rec)[cnt]!=NULL; cnt++);
        }
        for (k=0; k<cnt; k++) {
            if (dbopts->isPtr2 || dbopts->isPtrs)
                prec = ((void**)rec)[k];
            else
                prec = (char*)rec + (k * structSize);
            if (!prec)
                continue;
            Jsi_OptionsFree(interp, (Jsi_OptionSpec*)specs, prec, 0);
            if (dbopts->isPtr2 || dbopts->isPtrs) {
                Jsi_Free(prec);
            }
        }
        if (recPtrPtr) {
            Jsi_Free(*recPtrPtr);
            *recPtrPtr = NULL;
        }
        if (query == NULL || query[0] == 0)
            return 0;
    }
    
    if (!Jsi_Strncasecmp(query, "SELECT", 6))
        return dbOptSelect(jdb, query, &ob, dbopts);
        
    DbEvalContext sEval = {};
    int insert = 0, replace = 0, update = 0;
    char nbuf[100], *bPtr;
#ifdef JSI_DB_DSTRING_SIZE
    JSI_DSTRING_VAR(dStr, JSI_DB_DSTRING_SIZE);
#else
    Jsi_DString sStr, *dStr = &sStr;
    Jsi_DSInit(dStr);
#endif
    if (dbopts->noCache)
        sEval.nocache = 1;
    if (dbEvalInit(interp, &sEval, jdb, NULL, dStr, 0, 0) != JSI_OK)
        return -1;
    int dataMax = dbopts[0].arrSize;
    cnt = 0;
    if (dataMax==0)
        dataMax = 1;
    char ch[2];

    ch[0] = dbopts[0].prefix;
    ch[1] = 0;
    if (!ch[0])
        ch[0] = ':';
    if ((update=(Jsi_Strncasecmp(query, "UPDATE", 6)==0))) {
        Jsi_DSAppendLen(dStr, query, cPtr?(cPtr-query):-1);
        if (cPtr) {
            Jsi_DSAppend(dStr, " ", NULL);
            int cidx = 0;
            int killf = (JSI_OPT_DB_IGNORE|JSI_OPT_READ_ONLY|JSI_OPT_INIT_ONLY);
            for (specPtr = specs; specPtr->id != JSI_OPTION_END; specPtr++, cidx++) {
                if (specPtr == ob.rowidPtr || specPtr == ob.dirtyPtr || (specPtr->flags&killf))
                    continue;
                const char *fname = specPtr->name;
                if (ch[0] == '?')
                    snprintf(bPtr=nbuf, sizeof(nbuf), "%d", cidx+1);
                else
                    bPtr = (char*)specPtr->name;
                Jsi_DSAppend(dStr, (cnt?",":""), "[", fname, "]=",
                    ch, bPtr, NULL);
                cnt++;
            }
            Jsi_DSAppend(dStr, cPtr+3, NULL);
        }
    } else if ((insert=(Jsi_Strncasecmp(query, "INSERT", 6)==0))
        || (replace=(Jsi_Strncasecmp(query, "REPLACE", 7)==0))) {
        Jsi_DSAppendLen(dStr, query, cPtr?(cPtr-query):-1);
        if (cPtr) {
            Jsi_DSAppend(dStr, " (", NULL);
            int killf = JSI_OPT_DB_IGNORE;
            if (replace)
                killf |= (JSI_OPT_READ_ONLY|JSI_OPT_INIT_ONLY);
            for (specPtr = specs; specPtr->id != JSI_OPTION_END; specPtr++) {
                if (specPtr == ob.rowidPtr || specPtr == ob.dirtyPtr || specPtr->flags&killf)
                    continue;
                const char *fname = specPtr->name;
                Jsi_DSAppend(dStr, (cnt?",":""), "[", fname, "]", NULL);
                cnt++;
            }
            Jsi_DSAppendLen(dStr,") VALUES(", -1);
            cnt = 0;
            int cidx = 0;
            for (specPtr = specs; specPtr->id != JSI_OPTION_END; specPtr++, cidx++) {
                if (specPtr == ob.rowidPtr || specPtr == ob.dirtyPtr
                    || specPtr->flags&killf)
                    continue;
                if (ch[0] == '?')
                    snprintf(bPtr=nbuf, sizeof(nbuf), "%d", cidx+1);
                else
                    bPtr = (char*)specPtr->name;
                Jsi_DSAppend(dStr, (cnt?",":""), ch, bPtr, NULL);
                cnt++;
            }
            Jsi_DSAppend(dStr,")", cPtr+3, NULL);
        }
    } else if (!Jsi_Strncasecmp(query, "DELETE", 6)) {
        Jsi_DSAppend(dStr, query, NULL);
    } else {
        Jsi_LogError("unrecognized query \"%s\": expected one of: SELECT, UPDATE, INSERT, REPLACE or DELETE", query);
        return -1;
    }
    sEval.zSql = Jsi_DSValue(dStr);
    if (jdb->echo && sEval.zSql)
        Jsi_LogInfo("SQL-ECHO: %s\n", sEval.zSql); 

    int rc, bindMax = -1, dataIdx = 0;
    cnt = 0;
    int ismodify = (replace||insert||update);
    int isnew = (replace||insert);
    int didBegin = 0;
    DbEvalContext *p = &sEval;
    rc = dbPrepareStmt(p->jdb, p->zSql, &p->zSql, &p->pPreStmt);
    if( rc!=JSI_OK ) return -1;
    if (dataMax>1 && !dbopts->noBegin) {
        didBegin = 1;
        if (!dbExecCmd(jdb, JSI_DBQUERY_BEGIN_STR, &erc))
            goto bail;
    }
    while (dataIdx<dataMax) {
        if (ismodify && ob.dirtyPtr && (dbopts->dirtyOnly)) { /* Check to limit updates to dirty values only. */
            void *rec = dbopts[0].data;
            if (dbopts->isPtrs || dbopts->isPtr2)
                rec = ((void**)rec)[dataIdx];
            else
                rec = (char*)rec + (dataIdx * structSize);
            char *ptr = (char*)rec + ob.dirtyPtr->offset;
            int isDirty = *(int*)ptr;
            int bit = 0;
            if (ob.dirtyPtr->id == JSI_OPTION_BOOL)
                bit = (uintptr_t)ob.dirtyPtr->data;
            if (!(isDirty&(1<<(bit)))) {
                dataIdx++;
                continue;
            }
            isDirty &= ~(1<<(bit));
            *(int*)ptr = isDirty; /* Note that the dirty bit is cleared, even upon error.*/
        }
        rc = dbBindOptionStmt(jdb, p->pPreStmt->pStmt, &ob, dataIdx, bindMax, dbopts);
        if( rc!=JSI_OK )
            goto bail;
        bindMax = 1;
        rc = dbEvalStepSub(p, (dataIdx>=dataMax), &erc);
        if (rc == JSI_ERROR)
            goto bail;
        cnt += sqlite3_changes(jdb->db);
        if (rc != JSI_OK && rc != JSI_BREAK)
            break;
        if (ob.rowidPtr && isnew) {
            void *rec = dbopts[0].data;
            if (dbopts->isPtrs || dbopts->isPtr2)
                rec = ((void**)rec)[dataIdx];
            else
                rec = (char*)rec + (dataIdx * structSize);
            char *ptr = (char*)rec + ob.rowidPtr->offset;
            *(Jsi_Wide*)ptr = sqlite3_last_insert_rowid(jdb->db);
        }
        dataIdx++;
    }
    if (didBegin && !dbExecCmd(jdb, JSI_DBQUERY_COMMIT_STR, &erc))
        rc = JSI_ERROR;
    dbEvalFinalize(&sEval);
    if( rc==JSI_BREAK ) {
        rc = JSI_OK;
    }
    return (rc==JSI_OK?cnt:erc);

bail:
    dbEvalFinalize(&sEval);
    if (didBegin)
        dbExecCmd(jdb, JSI_DBQUERY_ROLLBACK_STR, NULL);
    return erc;
}

#ifdef Jsi_DbQuery
#undef Jsi_DbQuery
#undef Jsi_DbHandle
#undef Jsi_DbNew
#endif
int
Jsi_DbQuery(Jsi_Db *jdb, Jsi_CDataDb *dbopts, const char *query)
{
    int rc = jsi_DbQuery(jdb, dbopts, query);
#ifdef JSI_DBQUERY_ERRORCMD
    if (rc<0)
        rc = JSI_DBQUERY_ERRORCMD(jdb, specs, data, arrSize, query, dopts, rc);
#endif
    return rc;
}

void *Jsi_DbHandle(Jsi_Interp *interp, Jsi_Db* jdb)
{
    SQLSIGASSERT(jdb,DB);
    return jdb->db;
}

/* This is the non-script, JSI_LITE_ONLY creator for Jsi_Db */
Jsi_Db* Jsi_DbNew(const char *zFile, int inFlags /* JSI_DBI_* */)
{
    char *zErrMsg;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
#ifdef SQLITE_JSI_DEFAULT_FULLMUTEX
    flags |= SQLITE_OPEN_FULLMUTEX;
#else
    flags |= SQLITE_OPEN_NOMUTEX;
#endif
    if (!zFile)
        zFile = ":memory:";
    zErrMsg = 0;
    Jsi_Db *db = (Jsi_Db*)Jsi_Calloc(1, sizeof(*db) );
    if( db==0 ) {
        JSI_DBQUERY_PRINTF( "malloc failed\n");
        return NULL;
    }
    db->sig = SQLITE_SIG_DB;
    db->stmtCacheMax = NUM_PREPARED_STMTS;
    db->optPtr = &db->queryOpts;

    if (inFlags&JSI_DBI_READONLY) {
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
    } else {
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
        if (inFlags&JSI_DBI_NOCREATE) {
            flags &= ~SQLITE_OPEN_CREATE;
        }
    }
    if(inFlags&JSI_DBI_NO_MUTEX) {
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
    } else {
        flags &= ~SQLITE_OPEN_NOMUTEX;
    }
    if(inFlags&JSI_DBI_FULL_MUTEX) {
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
    } else {
        flags &= ~SQLITE_OPEN_FULLMUTEX;
    }
    char cpath[PATH_MAX];
    char *npath = Jsi_FileRealpathStr(NULL, zFile, cpath);
    
    if (SQLITE_OK != sqlite3_open_v2(npath, &db->db, flags, NULL)) {
        JSI_DBQUERY_PRINTF( "db open failed: %s\n", npath);
        goto bail;
    }
    //Jsi_DSFree(&translatedFilename);

    if( SQLITE_OK!=sqlite3_errcode(db->db) ) {
        zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(db->db));
        DbClose(db->db);
        db->db = 0;
    }
    if( db->db==0 ) {
        JSI_DBQUERY_PRINTF( "Db open failed %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        goto bail;
    }
    db->stmtHash = Jsi_HashNew(NULL, JSI_KEYS_STRING, NULL);
    db->strKeyTbl = Jsi_HashNew(NULL, JSI_KEYS_STRING, NULL);
    db->stmtCache = Jsi_ListNew((Jsi_Interp*)db, 0, dbStmtFreeProc);
    return db;
    
bail:
    return NULL;
}

#ifndef JSI_LITE_ONLY

static Jsi_RC Jsi_DoneSqlite(Jsi_Interp *interp)
{
    Jsi_UserObjUnregister(interp, &sqliteobject);
    const char *provide = "Sqlite";
    Jsi_PkgProvide(interp, provide, -1, NULL);
    return JSI_OK;
}
#ifdef JSI_DB_TEST
#include "c-demos/dbdemo.c"
#endif

Jsi_RC Jsi_InitSqlite(Jsi_Interp *interp, int release)
{
    if (release) return Jsi_DoneSqlite(interp);
    Jsi_Hash* dbSys;
#if JSI_USE_STUBS
    if (Jsi_StubsInit(interp, 0) != JSI_OK)
        return JSI_ERROR;
#endif

    if (!(dbSys = Jsi_UserObjRegister(interp, &sqliteobject))) 
        return Jsi_LogError("Failed to init sqlite extension");

    Jsi_Value *info = Jsi_ValueNew1(interp);
    Jsi_JSONParseFmt(interp, &info, "{libVer:\"%s\", hdrVer:\"%s\", hdrNum:%d, hdrSrcId:\"%s\", pkgVer:%d}",
        (char *)sqlite3_libversion(), SQLITE_VERSION, SQLITE_VERSION_NUMBER, SQLITE_SOURCE_ID, jsi_DbPkgVersion);
    Jsi_PkgOpts dbPkgOpts = { db_ObjCmd_Specs, &dbObjCmd, sqliteCmds, info };
    Jsi_RC rc = Jsi_PkgProvideEx(interp, "Sqlite", jsi_DbPkgVersion, Jsi_InitSqlite, &dbPkgOpts);
    Jsi_DecrRefCount(interp, info);
    if (rc != JSI_OK)
        return JSI_ERROR;

    if (!Jsi_CommandCreateSpecs(interp, sqliteobject.name, sqliteCmds, dbSys, JSI_CMDSPEC_ISOBJ))
        return JSI_ERROR;        

#ifdef JSI_DB_TEST
    if (getenv("RUN_DB_TEST"))
        TestSqlite(interp);
#endif
    jsi_DbVfs **dbVfsPtrPtr = (jsi_DbVfs **)Jsi_InterpGetData(interp, JSI_SQLITE_DB_VFS, NULL);
    if (dbVfsPtrPtr)
        *dbVfsPtrPtr = &SqliteDbVfs;
    return JSI_OK;
}
#endif //JSI_LITE_ONLY

#else // !JSI__SQLITE
/* Linking for when Sqlite is not compiled-in. */

static jsi_DbVfs *jsi_dbVfsPtr = NULL;
#ifndef JSI_LITE_ONLY


Jsi_RC Jsi_initSqlite(Jsi_Interp *interp, int release)
{
    if (!release) Jsi_InterpSetData(interp, JSI_SQLITE_DB_VFS, &jsi_dbVfsPtr, NULL);
    return JSI_OK;
}

Jsi_RC Jsi_doneSqlite(Jsi_Interp *interp)
{
    Jsi_InterpFreeData(interp, JSI_SQLITE_DB_VFS);
    return JSI_OK;
}
#endif

int
Jsi_DbQuery(Jsi_Db *jdb, Jsi_CDataDb *dPtr, const char *query)
{
    if (!jsi_dbVfsPtr) {
        printf( "Sqlite unsupported\n");
        return -1;
    }
    return jsi_dbVfsPtr->dbcQuery(jdb, dPtr, query);
}

void *Jsi_DbHandle(Jsi_Interp *interp, Jsi_Db* jdb)
{
    if (!jsi_dbVfsPtr) {
        printf( "Sqlite unsupported\n");
        return NULL;
    }
    return jsi_dbVfsPtr->dbHandle(interp, jdb);
}

Jsi_Db* Jsi_DbNew(const char *zFile, int inFlags /* JSI_DBI_* */)
{
    if (!jsi_dbVfsPtr) {
        printf( "Sqlite unsupported\n");
        return NULL;
    }
    return jsi_dbVfsPtr->dbNew(zFile, inFlags);
}

#endif
