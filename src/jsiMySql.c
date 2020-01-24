#if JSI__MYSQL==1
/* JSI Javascript Interface to MySql. */

typedef enum { MYSQL_SIG_DB = 0xbeefdeaa, MYSQL_SIG_FUNC, MYSQL_SIG_EXEC, MYSQL_SIG_STMT } MySql_Sig;

#define SQLSIGASSERT(s,n) assert(s->sig == MYSQL_SIG_##n)
#define SQLSIGINIT(s,n) s->sig = MYSQL_SIG_##n

#ifndef NDEBUG
#ifndef _JSI_MEMCLEAR
#define _JSI_MEMCLEAR(s) memset(s, 0, sizeof(*s));
#endif
#else
#define _JSI_MEMCLEAR(s)
#endif
#ifndef JSI_DB_DSTRING_SIZE
#define JSI_DB_DSTRING_SIZE 2000
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <mysql/my_config.h>

#if JSI__MEMDEBUG
#include "jsiInt.h"
#else
#include "jsi.h"
JSI_EXTENSION_INI
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

typedef struct mydb_ObjCmd {
    int init;
    int activeCnt;  /* Count of active objects. */ 
    int newCnt;  /* Total number of new. */ 
} mydb_ObjCmd;

static mydb_ObjCmd mydbObjCmd = {};

static Jsi_OptionSpec mydb_ObjCmd_Specs[] =
{
    JSI_OPT(INT,   mydb_ObjCmd, init, .help="Init counter"),
    JSI_OPT(INT,   mydb_ObjCmd, activeCnt, .help="Number of active objects"),
    JSI_OPT(INT,   mydb_ObjCmd, newCnt,    .help="Number of new calls"),
    JSI_OPT_END(mydb_ObjCmd, .help="Options for Sqlite module")
};

/*
** New SQL functions can be created as JSI scripts.  Each such function
** is described by an instance of the following structure.
*/
typedef struct SqlFunc SqlFunc;
struct SqlFunc {
    MySql_Sig sig;
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
    MySql_Sig sig;
    Jsi_Interp  *interp;   /* The JSI interpret to execute the function */
    Jsi_Value   *zScript;  /* The function to be run */
    SqlCollate  *pNext;    /* Next function on the list of them all */
};

typedef struct SqlFieldResults {
    MYSQL_FIELD *field;
    my_bool isnull;
    unsigned long len;
    int vsize;
    enum enum_field_types mapType; // Maps to one of DOUBLE, BOOL, VARCHAR or TIMESTAMP
    Jsi_OptionId jsiTypeMap;  // Type of data for Jsi.
    union { // Space for data.
        char vchar;
        double vdouble;  // Must convert if Jsi_Number is a long double.
        long long vlonglong;
        my_bool vbool;
        char *vstring;
        MYSQL_TIME timestamp;
    } buffer;
} SqlFieldResults;

/*
    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &result_int;
    result_bind[0].buffer_length = sizeof(result_int);
    result_bind[0].length = &result_len;
    result_bind[0].is_null = &result_is_null[0];
*/

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
typedef struct MysqlPrep MysqlPrep;
struct MysqlPrep {
    MySql_Sig sig;
    int deleting;
    MysqlPrep *pNext;  /* Next in linked list */
    MysqlPrep *pPrev;  /* Previous on the list */
    MYSQL_STMT    *myStmt;  /* The prepared statement */
    MYSQL_RES     *resultMetaData;
    MYSQL_RES     *paramMetaData;
    MYSQL_BIND    *bindParam;
    MYSQL_BIND    *bindResult;
    SqlFieldResults *fieldResult;
    SqlFieldResults *fieldParam;
    //int resultColumns, paramCount;
    uint nSql;                /* chars in zSql[] */
    char *zSql;        /* Text of the SQL statement */
    const char *zRawSql;     /* SQL before named params extracted. */
    
    int numParam; // Count of input params.
    int numCol; // Count of columns in results
    Jsi_HashEntry *entry;
    Jsi_ListEntry *elPtr;
    char **colNames;       /* List of column names. */
    Jsi_OptionId *colTypes;
    // Following used by named params.
    char *origSql;
    char **paramNames;       /* List of param names. */
    int *paramMyTypes;
    int paramCnt;
    Jsi_DString *naStr;
};

static const char *myexecFmtStrs[] = {
    "rows", "arrays", "array1d", "list", "column", "json",
    "json2", "html", "csv", "insert", "line", "tabs", "none", NULL
};

typedef enum {
    _mdb_EF_ROWS, _mdb_EF_ARRAYS, _mdb_EF_ARRAY1D, _mdb_EF_LIST, _mdb_EF_COLUMN, _mdb_EF_JSON,
    _mdb_EF_JSON2, _mdb_EF_HTML, _mdb_EF_CSV, _mdb_EF_INSERT, _mdb_EF_LINE, _mdb_EF_TABS, _mdb_EF_NONE
} mdbOutput_Mode;


const char *mdbTypeChkStrs[] = { "convert", "error", "warn", "disable", NULL };

typedef enum { mdbTypeCheck_Cast, mdbTypeCheck_Error, mdbTypeCheck_Warn,  mdbTypeCheck_None } mdbTypeCheck_Mode;

typedef struct QueryOpts {
    MySql_Sig sig;
    Jsi_Value *callback, *values;
    Jsi_Value *paramVar;
    int limit, objOpts;
    mdbOutput_Mode mode;
    mdbTypeCheck_Mode typeCheck;
    bool mapundef, nocache, headers, noNamedParams, prefetch;
    const char *separator;
    const char *nullvalue;
    const char *table;
    //const char *CData; // Name of cdata to use for query.
    const char* objName;
    Jsi_Value *width;
    int maxString;
} QueryOpts;

static const char *trcModeStrs[] = {"eval", "delete", "prepare", "step", NULL}; // Bit-set packed into an int.
static const char *objSqlModeStrs[] = { "getSql", "noTypes", "noDefaults", "nullDefaults", NULL };
enum {mdbTMODE_EVAL=0x1, mdbTMODE_DELETE=0x2, mdbTMODE_PREPARE=0x4, mdbTMODE_STEP=0x4};
enum {OBJMODE_SQLONLY=0x1, OBJMODE_NOTYPES=0x2, OBJMODE_NODEFAULTS=0x4, OBJMODE_NULLDEFAULTS=0x8};


typedef struct MySqlObj {
    MySql_Sig sig;
    MYSQL  *db;               /* The "real" database structure. MUST BE FIRST */
    Jsi_Interp *interp;        /* The interpreter used for this database */
    mydb_ObjCmd *_;
    Jsi_Value *host;
    const char *user;
    const char *password;
    const char *database;
    int port;
    char *zNull;               /* Text to substitute for an SQL NULL value */
    SqlFunc *pFunc;            /* List of SQL functions */
    int rc;                    /* Return code of most recent mysql_exec() */
    Jsi_List *stmtCache;
//    MysqlPrep *stmtList; /* List of prepared statements*/
//    MysqlPrep *stmtLast; /* Last statement in the list */
    Jsi_Hash *stmtHash;        /* Hash table for statements. */
    int maxStmts;               /* The next maximum number of stmtList */
    int numStmts;                 /* Number of statements in stmtList */
    bool bindWarn, forceInt, reconnect, enableMulti;
    int nStep, nSort;          /* Statistics for most recent operation */
    int nTransaction;          /* Number of nested [transaction] methods */
    int errorCnt;               /* Count of errors. */
    Jsi_Value *key;             /* Key, for codec. */
    int hasOpts;
    Jsi_Obj *userObjPtr;
    QueryOpts queryOpts, *optPtr;
    int objId;
    int debug;
    int deleted;
    Jsi_Event *event;
    //int trace;
    Jsi_DString name;
    int last_errno;
    Jsi_Number version;
    int dbflags;
    Jsi_Value* udata;
    Jsi_Value *sslKey, *sslCert, *sslCA, *sslCAPath, *sslCipher;
    Jsi_Hash *typeNameHash;
} MySqlObj;

typedef struct MyDbEvalContext {
    MySqlObj *jdb;                /* Database handle */
    Jsi_DString *dzSql;               /* Object holding string zSql */
    const char *zSql;               /* Remaining SQL to execute */
    MysqlPrep *prep;      /* Current statement */
    int nCol;                       /* Number of columns returned by pStmt */
    Jsi_Value *tocall;
    Jsi_Value *ret;
    /*OBS */
    Jsi_Value *pArray;              /* Name of array variable */
    Jsi_Value *pValVar;             /* Name of list for values. */
    int nocache;
    int namedParams;
} MyDbEvalContext;

static Jsi_RC mdbIsNumArray(Jsi_Interp *interp, Jsi_Value *value);
static Jsi_RC mdbPrepareAndBind(MyDbEvalContext *p );
static void mdbReleaseStmt( MySqlObj *jdb, MysqlPrep *prep, int discard );

void mdbTypeNameHashInit(MySqlObj *jdb) {
    Jsi_Interp *interp = jdb->interp;
    Jsi_Hash *hPtr = jdb->typeNameHash = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    Jsi_HashSet(hPtr, (void*)"string", (void*)MYSQL_TYPE_STRING);
    Jsi_HashSet(hPtr, (void*)"double", (void*)MYSQL_TYPE_DOUBLE);
    Jsi_HashSet(hPtr, (void*)"integer", (void*)MYSQL_TYPE_LONGLONG);
    Jsi_HashSet(hPtr, (void*)"bool", (void*)MYSQL_TYPE_TINY);
    Jsi_HashSet(hPtr, (void*)"blob", (void*)MYSQL_TYPE_BLOB);
    Jsi_HashSet(hPtr, (void*)"date", (void*)MYSQL_TYPE_DATE);
    Jsi_HashSet(hPtr, (void*)"time", (void*)MYSQL_TYPE_TIME);
    Jsi_HashSet(hPtr, (void*)"timestamp", (void*)MYSQL_TYPE_TIMESTAMP);
    Jsi_HashSet(hPtr, (void*)"datetime", (void*)MYSQL_TYPE_DATETIME);
}

static Jsi_OptionSpec QueryFmtOptions[] =
{
    JSI_OPT(FUNC,   QueryOpts, callback, .help="Function to call with each row result", .flags=0, .custom=0, .data=(void*)"values:object" ),
    JSI_OPT(BOOL,   QueryOpts, headers, .help="First row returned contains column labels"),
    JSI_OPT(INT,    QueryOpts, limit, .help="Maximum number of returned values"),
    JSI_OPT(BOOL,   QueryOpts, mapundef, .help="In variable binds, map an 'undefined' var to null"),
    JSI_OPT(INT,    QueryOpts, maxString, .help="If not using prefetch, the maximum string value size (0=8K)"),
    JSI_OPT(CUSTOM, QueryOpts, mode, .help="Set output mode of returned data", .flags=0, .custom=Jsi_Opt_SwitchEnum,  .data=myexecFmtStrs),
    JSI_OPT(BOOL,   QueryOpts, nocache, .help="Disable query cache"),
    JSI_OPT(BOOL,   QueryOpts, noNamedParams, .help="Disable translating sql to support named params"),
    JSI_OPT(STRKEY, QueryOpts, nullvalue, .help="Null string output (for non-json mode)"),
    JSI_OPT(STRKEY, QueryOpts, objName,  .help="Object var name for CREATE/INSERT: replaces %s with fields in query" ),
    JSI_OPT(CUSTOM, QueryOpts, objOpts,     .help="Options for objName", .flags=0,  .custom=Jsi_Opt_SwitchBitset,  .data=objSqlModeStrs),
    JSI_OPT(ARRAY,  QueryOpts, paramVar, .help="Array var to use for parameters" ),
    JSI_OPT(BOOL,   QueryOpts, prefetch, .help="Let client library cache entire results"),
    JSI_OPT(STRKEY, QueryOpts, separator, .help="Separator string (for csv and text mode)"),
    //JSI_OPT(STRKEY, QueryOpts, CData, .help="Name of CData object to use"),
    JSI_OPT(STRKEY, QueryOpts, table, .help="Table name for mode=insert"),
    JSI_OPT(CUSTOM, QueryOpts, typeCheck,   .help="Type check mode (error)", .flags=0, .custom=Jsi_Opt_SwitchEnum, .data=mdbTypeChkStrs),
    JSI_OPT(ARRAY,  QueryOpts, values, .help="Values for ? bind parameters" ),
    JSI_OPT(CUSTOM, QueryOpts, width, .help="In column mode, set column widths", .flags=0, .custom=Jsi_Opt_SwitchValueVerify, .data=(void*)mdbIsNumArray),
    JSI_OPT_END(QueryOpts, .help="MySql query options")
};


#ifndef jsi_IIOF
#define jsi_IIOF .flags=JSI_OPT_INIT_ONLY
#define jsi_IIRO .flags=JSI_OPT_READ_ONLY
#endif
static Jsi_OptionSpec SqlOptions[] =
{
    JSI_OPT(BOOL,   MySqlObj, bindWarn, .help="Treat failed variable binds as a warning", jsi_IIOF),
    JSI_OPT(STRKEY, MySqlObj, database, .help="Database to use", jsi_IIOF ),
    JSI_OPT(CUSTOM, MySqlObj, debug,    .help="Enable debug trace for various operations", .flags=0, .custom=Jsi_Opt_SwitchBitset,  .data=trcModeStrs),
    JSI_OPT(BOOL,   MySqlObj, enableMulti,.help="Accept muiltiple semi-colon separated statements in eval()", jsi_IIOF),
    JSI_OPT(INT,    MySqlObj, errorCnt, .help="Count of errors", .flags=JSI_OPT_READ_ONLY),
    JSI_OPT(CUSTOM, MySqlObj, queryOpts, .help="Default options for exec", .flags=0, .custom=Jsi_Opt_SwitchSuboption, .data=QueryFmtOptions),
    JSI_OPT(BOOL,   MySqlObj, forceInt, .help="Bind float as int if possible"),
    JSI_OPT(STRING, MySqlObj, host,     .help="IP address or host name for mysqld (default is 127.0.0.1)"),
    JSI_OPT(INT,    MySqlObj, maxStmts, .help="Max cache size for compiled statements"),
    JSI_OPT(DSTRING,MySqlObj, name,     .help="Name for this db handle"),
    JSI_OPT(INT,    MySqlObj, numStmts, .help="Current size of compiled statement cache", .flags=JSI_OPT_READ_ONLY),
    JSI_OPT(STRKEY, MySqlObj, password, .help="Database password.", jsi_IIOF ),
    JSI_OPT(INT,    MySqlObj, port,     .help="IP port for mysqld", jsi_IIOF),
    JSI_OPT(BOOL,   MySqlObj, reconnect, .help="Reconnect"),
    JSI_OPT(STRING, MySqlObj, sslKey, .help="SSL key"),
    JSI_OPT(STRING, MySqlObj, sslCert, .help="SSL Cert"),
    JSI_OPT(STRING, MySqlObj, sslCA, .help="SSL CA"),
    JSI_OPT(STRING, MySqlObj, sslCAPath, .help="SSL CA path"),
    JSI_OPT(STRING, MySqlObj, sslCipher, .help="SSL Cipher"),
    JSI_OPT(OBJ,    MySqlObj, udata,    .help="User data." ),
    JSI_OPT(STRKEY, MySqlObj, user,     .help="Database user name. Default is current user-name.", jsi_IIOF ),
    JSI_OPT(DOUBLE, MySqlObj, version,  .help="Mysql version number", .flags=JSI_OPT_READ_ONLY),
    JSI_OPT_END(MySqlObj, .help="MySql options")
};

/* Start of code. */

// Convert MySql time to JS unix time in ms. TODO: handle "neg" and years outside of unix time.
static Jsi_Number mdbMyTimeToJS(MYSQL_TIME* mtm)
{
    struct tm tm;
    tm.tm_sec = mtm->second;
    tm.tm_min = mtm->minute;
    tm.tm_hour = mtm->hour;
    tm.tm_mday = mtm->day;
    tm.tm_mon = mtm->month-1;
    tm.tm_year = mtm->year - 1900;
    time_t tim = mktime(&tm);
    if (tim == (time_t)-1)
        return -1;
    return (Jsi_Number)tim*1000 + (Jsi_Number)mtm->second_part/1000000.0;
}

static void mdbJsToMyTime(Jsi_Number time, MYSQL_TIME* mtm, int utc)
{
    struct tm tm;
    time_t tim = (time_t)(time/1000);
    if (utc)
        gmtime_r(&tim, &tm);
    else
        localtime_r(&tim, &tm);
    mtm->second = tm.tm_sec;
    mtm->minute = tm.tm_min;
    mtm->hour = tm.tm_hour;
    mtm->day = tm.tm_mday;
    mtm->month = tm.tm_mon+1;
    mtm->year = tm.tm_year + 1900;
    Jsi_Number secs = (tim/1000.0);
    mtm->second_part = (int)((secs-(int)secs)*1000000);
}

static Jsi_RC mdbEvalInit(
    Jsi_Interp *interp,
    MyDbEvalContext *p,               /* Pointer to structure to initialize */
    MySqlObj *jdb,                  /* Database handle */
    const char* zSql,                /* Value containing SQL script */
    Jsi_DString *dStr,
    Jsi_Obj *pArray,                /* Name of Jsi array to set (*) element of */
    Jsi_Obj *pValVar                  /* Name element in array for list. */
) {
    p->dzSql = dStr;
    p->zSql = Jsi_DSAppend(p->dzSql, zSql?zSql:"", NULL);
    p->jdb = jdb;
    return JSI_OK;
}

static void mdbEvalFinalize(MyDbEvalContext *p) {
    if( p->prep) {
        mdbReleaseStmt(p->jdb, p->prep, p->nocache);
        p->prep = 0;
    }
    Jsi_DSFree(p->dzSql);
}

static void mdbEvalRowInfo( MyDbEvalContext *eval, int *pnCol, char ***papColName, Jsi_OptionId **papColType) {
    if (!papColName) {
        //TODO: Array ???
    }
    *papColName = eval->prep->colNames;
    *papColType = eval->prep->colTypes;
    *pnCol = eval->prep->numCol;
}

/* Step statement. Return JSI_OK if there is a ROW result, JSI_BREAK if done, else JSI_ERROR. */
static Jsi_RC mdbEvalStepSub(MyDbEvalContext *eval, int release, int *erc) {
    MySqlObj *jdb = eval->jdb;
    Jsi_Interp *interp = jdb->interp;
    MysqlPrep *prep = eval->prep;
    SQLSIGASSERT(prep, STMT);
    MYSQL_STMT *myStmt = prep->myStmt;

    if (jdb->debug & mdbTMODE_STEP)
        JSI_DBQUERY_PRINTF( "DEBUG: step: %s\n", prep->zSql);
    int m = mysql_stmt_fetch(myStmt);
    if (m == MYSQL_NO_DATA)
        return JSI_BREAK;
    if (m) 
        return Jsi_LogError("fetch failed: %s", mysql_error(jdb->db));
    return JSI_OK;
#if 0
    if( eval->pArray ) {
        mdbEvalRowInfo(eval, 0, 0, 0);
    }
    if (release==0)
        return JSI_BREAK;
    eval->prep = 0;
    return JSI_OK;
#endif
}

static void mdbRelease1Stmt( MySqlObj *jdb, MysqlPrep *prep ) {
    // TODO: split out parts reusable by cached query.
    int i;
    if (prep->deleting)
        return;
    prep->deleting = 1;
    if (prep->myStmt)
        mysql_stmt_close(prep->myStmt);
    if (prep->resultMetaData)
        mysql_free_result(prep->resultMetaData);
    if (prep->paramMetaData)
        mysql_free_result(prep->paramMetaData);
    if (prep->bindParam)
        Jsi_Free(prep->bindParam);
    if (prep->fieldParam)
        Jsi_Free(prep->fieldParam);
    if (prep->bindResult) {
        for (i=0; i<prep->numCol; i++) {
            MYSQL_BIND *bind = prep->bindResult+i;
            if (bind->buffer_type == MYSQL_TYPE_STRING && bind->buffer)
                Jsi_Free(bind->buffer);
        }
        Jsi_Free(prep->bindResult);
    }
    if (prep->fieldResult)
        Jsi_Free(prep->fieldResult);
    if (prep->colTypes)
        Jsi_Free(prep->colTypes);
    if (prep->colNames)
        Jsi_Free(prep->colNames);
    if (prep->zSql)
        Jsi_Free(prep->zSql);
    if (prep->naStr) {
        Jsi_Free(prep->origSql);
        Jsi_DSFree(prep->naStr);
        Jsi_Free(prep->naStr);
    }
    if (prep->entry)
        Jsi_HashEntryDelete(prep->entry);
    if (prep->elPtr) {
        Jsi_ListEntry *pPtr = prep->elPtr;
        prep->elPtr = NULL;
        Jsi_ListEntryDelete(pPtr);
    }
    Jsi_Free(prep);
    jdb->numStmts--;
}


static Jsi_RC mdbStmtFreeProc(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *value) {
    MySqlObj *jdb = (MySqlObj*)interp;
    Jsi_ListEntry *l = (Jsi_ListEntry*)hPtr;
    mdbRelease1Stmt(jdb, (MysqlPrep*)Jsi_ListValueGet(l));
    return JSI_OK;
}

static void mdbStmtLimit( MySqlObj *jdb)
{
    while(jdb->numStmts>jdb->maxStmts ) {
        Jsi_ListEntry *l = Jsi_ListPopBack(jdb->stmtCache);
        mdbRelease1Stmt(jdb, (MysqlPrep*)Jsi_ListValueGet(l));
        jdb->numStmts = Jsi_ListSize(jdb->stmtCache);
    }
}

/*
** Finalize and free a list of prepared statements
*/
static void mdbFlushStmtCache( MySqlObj *jdb ) {
    Jsi_ListClear(jdb->stmtCache);
    jdb->numStmts = 0;
}

/*
** Release a statement reference obtained by calling mdbPrepareAndBind().
** There should be exactly one call to this function for each call to
** mdbPrepareAndBind().
**
** If the discard parameter is non-zero, then the statement is deleted
** immediately. Otherwise it is added to the LRU list and may be returned
** by a subsequent call to mdbPrepareAndBind().
*/
static void mdbReleaseStmt( MySqlObj *jdb, MysqlPrep *prep, int discard ) {
    if( jdb->maxStmts<=0 || discard ) {
        /* If the cache is turned off, deallocated the statement */
        mdbRelease1Stmt(jdb, prep);
    } else {
        /* Add the prepared statement to the beginning of the cache list. */
        if (!prep->elPtr)
            prep->elPtr = Jsi_ListPushFrontNew(jdb->stmtCache, prep);
        else
            Jsi_ListPushFront(jdb->stmtCache, prep->elPtr);
        mdbStmtLimit(jdb);
    }
}

long long mdbLastInsertRowid(MySqlObj* jdb)
{
    return mysql_insert_id(jdb->db);
}


static const char *mdbFindEndStr(const char *cp) {
    const char endc = *cp;
    cp++;
    while (*cp && *cp != endc) {
        if (*cp == '\\' && cp[1]) cp++;
        cp++;
    }
    if (*cp != endc)
        return NULL;
    return ++cp;
}
// Collect named parameters and translate Sql to use ?
static Jsi_RC MySqlExtractParmNames(MySqlObj* jdb, const char *sql, Jsi_DString *sStr, Jsi_DString *nStr) {
    const char *ocp, *cp = sql;
    int cnt = 0;
    while (*cp) {
        if (*cp == '\"'||*cp == '\'') {
            ocp = cp;
            cp = mdbFindEndStr(cp);
            if (!cp)
                return JSI_ERROR;
            Jsi_DSAppendLen(sStr, ocp, cp-ocp+1);
        } else if ((*cp == '@' || *cp == ':' || *cp == '$' ) && (isalpha(cp[1]) || cp[1] == '_')) {
            ocp = cp;
            cp+=2;
            while (*cp && (isalnum(*cp) || *cp == '_'))
                cp++;
            if (*ocp == '$' && *cp == '(') {
                const char *ttp = NULL, *ttb = NULL, *eq = NULL;
                cp++;
                if (*cp == '[')
                    eq = cp++;
                while (*cp && (isalnum(*cp) || *cp == '_' || *cp == ':' || *cp == '.' || *cp == ']')) {
                    if (*cp == ':') {
                        if (ttp)
                            return JSI_ERROR;
                        ttp = cp;
                    }
                    if (*cp == ']') {
                        if (ttb)
                            return JSI_ERROR;
                        ttb = cp;
                        if (cp[1] != ')' && cp[1] != ':' && cp[1] != '.')
                            return JSI_ERROR;
                    }
                    cp++;
                }
                if (*cp != ')')
                    return JSI_ERROR;
                if (eq && !ttb)
                    return JSI_ERROR;
                if (ttp) {
                    Jsi_DString tStr = {};
                    Jsi_DSAppendLen(&tStr, ttp+1, (cp - ttp - 1));
                    if (!jdb->typeNameHash) mdbTypeNameHashInit(jdb);
                    int rc = (Jsi_HashEntryFind(jdb->typeNameHash, Jsi_DSValue(&tStr)) != NULL);
                    if (!rc) {
                        Jsi_DString eStr = {};
                        Jsi_HashEntry *hPtr;
                        Jsi_HashSearch search;
                        Jsi_Interp *interp = jdb->interp;
                        int n = 0;
                        for (hPtr = Jsi_HashSearchFirst(jdb->typeNameHash, &search);
                            hPtr != NULL; hPtr = Jsi_HashSearchNext(&search)) {
                            const char *key = (char*)Jsi_HashKeyGet(hPtr);
                            Jsi_DSAppend(&eStr, (n++?", ":""), key, NULL);
                        }
                        Jsi_LogWarn("bind type \"%s\" is not one of: %s", Jsi_DSValue(&tStr), Jsi_DSValue(&eStr));
                        Jsi_DSFree(&eStr);
                    }
                    Jsi_DSFree(&tStr);
                    if (!rc)
                        return JSI_ERROR;
                }
            } else
                cp--;
            if (cnt++)
                Jsi_DSAppendLen(nStr, " ", 1);
            Jsi_DSAppendLen(nStr, ocp, cp-ocp+1);
            Jsi_DSAppendLen(sStr, "?", 1);
        } else if (*cp == '\\' && cp[1]) {
            Jsi_DSAppendLen(sStr, cp, 2);
            cp++;
        } else {
            Jsi_DSAppendLen(sStr, cp, 1);
        }
        cp++;
    }
    return JSI_OK;
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
static Jsi_RC mdbPrepareStmt(MyDbEvalContext *p)
{
    MySqlObj *jdb = p->jdb;
    //int namedParams = !jdb->optPtr->noNamedParams;
    const char *zSql = p->zSql;         /* Pointer to first SQL statement in zIn */
    MYSQL_STMT *myStmt;            /* Prepared statement object */
    MysqlPrep *prep = 0;  /* Pointer to cached statement */
    Jsi_RC rc = JSI_OK;
    Jsi_Interp *interp = jdb->interp;


    Jsi_HashEntry *entry = Jsi_HashEntryFind(jdb->stmtHash, zSql);
    if (entry && ((prep = (MysqlPrep*)Jsi_HashValueGet(entry)))) {
        
        if (jdb->debug & mdbTMODE_PREPARE)
            JSI_DBQUERY_PRINTF( "DEBUG: prepare cache-hit: %s\n", zSql);
        myStmt = prep->myStmt;

        /* When a prepared statement is found, unlink it from the
        ** cache list.  It will later be added back to the beginning
        ** of the cache list in order to implement LRU replacement.
        */
        Jsi_ListPop(jdb->stmtCache, prep->elPtr);
        jdb->numStmts = Jsi_ListSize(jdb->stmtCache);
        
        // Sanity check for schema check: right now we just use number of columns
        if (prep->numCol != (int)mysql_stmt_field_count(prep->myStmt)) {
            mdbRelease1Stmt(jdb, prep);
            prep = NULL;
        }
    }
    
    /* If no prepared statement was found. Compile the SQL text. Also allocate
    ** a new MysqlPrep structure.  */
    if (!prep) {
        myStmt = mysql_stmt_init(jdb->db);
        if (!myStmt) 
            return Jsi_LogError("can't get statement: %s", mysql_error(jdb->db));
        char **paramNames;
        int paramCnt;
        int namedParams = 0;
        Jsi_DString *naStr = NULL;
        if (p->namedParams)
        {
            Jsi_RC rc = JSI_OK;
            Jsi_DString nsStr, nnStr;
            Jsi_DSInit(&nsStr);
            Jsi_DSInit(&nnStr);
            if (MySqlExtractParmNames(jdb, zSql, &nsStr, &nnStr) != JSI_OK)
                rc = Jsi_LogError("parsing names from query: %s", zSql);
            else if (Jsi_DSLength(&nnStr)) {
                namedParams = 1;
                zSql = Jsi_DSFreeDup(&nsStr);
                naStr = (Jsi_DString*)Jsi_Calloc(1, sizeof(*naStr));
                Jsi_SplitStr(Jsi_DSValue(&nnStr), &paramCnt, &paramNames, " ", naStr);
            }
            Jsi_DSFree(&nsStr);
            Jsi_DSFree(&nnStr);
            if (rc != JSI_OK)
                return rc;
        }

        if (mysql_stmt_prepare(myStmt, zSql, Jsi_Strlen(zSql)) )
        {
            Jsi_LogError("error in sql: %s", mysql_error(jdb->db));
            mysql_stmt_close(myStmt);
            if (namedParams) {
                Jsi_DSFree(naStr);
                Jsi_Free(naStr);
                Jsi_Free((char*)zSql);
            }
            return JSI_ERROR;
        }

        if (jdb->debug & mdbTMODE_PREPARE)
            JSI_DBQUERY_PRINTF( "DEBUG: prepare new: %s\n", zSql);
        assert( prep==0 );
        prep = (MysqlPrep*)Jsi_Calloc(1, sizeof(MysqlPrep));
        jdb->numStmts++;
        prep->sig = MYSQL_SIG_STMT;
        prep->myStmt = myStmt;
        if (!namedParams)
            prep->zSql = Jsi_Strdup(zSql);
        else {
            prep->zSql = (char*)zSql;
            prep->origSql = Jsi_Strdup(p->zSql);
            prep->naStr = naStr;
            prep->paramCnt = paramCnt;
            prep->paramNames = paramNames;
        }
        prep->paramMetaData = mysql_stmt_param_metadata(myStmt);
        prep->resultMetaData = mysql_stmt_result_metadata(myStmt);
        prep->numCol = mysql_stmt_field_count(myStmt);
        if (prep->numCol>0)
            prep->bindResult = (MYSQL_BIND *)Jsi_Calloc(prep->numCol, sizeof(MYSQL_BIND));
        prep->numParam = mysql_stmt_param_count(myStmt);
        if (prep->numParam>0 && !prep->bindParam) {
            prep->bindParam = (MYSQL_BIND *)Jsi_Calloc(prep->numParam, sizeof(MYSQL_BIND));
            prep->fieldParam = (SqlFieldResults*)Jsi_Calloc(prep->numParam, sizeof(*prep->fieldParam));
        }
        bool isNew = 0;
        prep->entry = Jsi_HashEntryNew(jdb->stmtHash, p->zSql, &isNew);
        if (!isNew)
            JSI_DBQUERY_PRINTF( "mysql dup stmt entry");
        Jsi_HashValueSet(prep->entry, prep);
    }
    p->prep = prep;
    return rc;
}

/*
** Return one of JSI_OK, JSI_BREAK or JSI_ERROR. If JSI_ERROR is
** returned, then an error message is stored in the interpreter before
** returning.
**
** A return value of JSI_OK means there is a row of data available. The
** data may be accessed using mdbEvalRowInfo() and dbEvalColumnValue(). This
** is analogous to a return of _MYSQLN_(ROW) from mysql_step(). If JSI_BREAK
** is returned, then the SQL script has finished executing and there are
** no further rows available. This is similar to _MYSQLN_(DONE).
*/
static Jsi_RC mdbEvalPrep(MyDbEvalContext *p) {
    MysqlPrep *prep = p->prep;
    Jsi_Interp *interp = p->jdb->interp;
    Jsi_RC rc = JSI_OK;
    if( p->prep==0 ) {
        rc = mdbPrepareAndBind(p); //p->jdb, p->zSql, &p->zSql, &p->prep);
        if( rc!=JSI_OK )
            return rc;
        prep = p->prep;
        if (p->jdb->optPtr->prefetch) {
            my_bool aBool = 1;
            mysql_stmt_attr_set(prep->myStmt, STMT_ATTR_UPDATE_MAX_LENGTH, &aBool);
        }
        if (mysql_stmt_execute(prep->myStmt)) {
            Jsi_Interp *interp = p->jdb->interp;
            rc = Jsi_LogError("execute failed: %s", mysql_error(p->jdb->db));
        }
        if (p->jdb->optPtr->prefetch && mysql_stmt_store_result(prep->myStmt)) {
            Jsi_LogWarn("prefetch failed, disabling: %s", mysql_error(p->jdb->db));
            p->jdb->optPtr->prefetch = 0;
        }
        MYSQL_RES *res = mysql_stmt_result_metadata(prep->myStmt);
        MYSQL_FIELD *field;
        if (res) {
            // Setup field mappings to/from Jsi.
            prep->fieldResult = (SqlFieldResults*)Jsi_Calloc(res->field_count, sizeof(*prep->fieldResult));
            prep->colNames = (char**)Jsi_Calloc(res->field_count, sizeof(char*));
            prep->colTypes = (Jsi_OptionId*)Jsi_Calloc(res->field_count, sizeof(int));
            int iCnt = 0;
            while((field = mysql_fetch_field(res)))
            {
                assert(iCnt<prep->numCol);
                SqlFieldResults *fres = prep->fieldResult+iCnt;
                MYSQL_BIND *bindResult = prep->bindResult+iCnt;
                
                bindResult->buffer = &fres->buffer.vchar;
                fres->field = field;
                prep->colNames[iCnt] = field->name;
                switch (field->type) {
                    case MYSQL_TYPE_TINY:
                    case MYSQL_TYPE_BIT:
                        if (field->length == 1) {
                            fres->jsiTypeMap = JSI_OPTION_BOOL;
                            fres->mapType = MYSQL_TYPE_DOUBLE;
                            fres->vsize = 1;
                            break;
                            
                        }
                    case MYSQL_TYPE_SHORT:
                    case MYSQL_TYPE_LONG:
                    case MYSQL_TYPE_DECIMAL:
                        fres->jsiTypeMap = JSI_OPTION_INT64;
                        fres->mapType = MYSQL_TYPE_LONG;
                        fres->vsize = sizeof(long long);
                        break;
                    case MYSQL_TYPE_LONGLONG:
                        fres->jsiTypeMap = JSI_OPTION_INT64;
                        fres->mapType = MYSQL_TYPE_LONGLONG;
                        fres->vsize = sizeof(long long);
                        break;
                    
                    case MYSQL_TYPE_FLOAT:
                    case MYSQL_TYPE_DOUBLE:
                        fres->jsiTypeMap = JSI_OPTION_DOUBLE;
                        fres->mapType = MYSQL_TYPE_DOUBLE;
                        fres->vsize = sizeof(double);
                        break;
                    case MYSQL_TYPE_TIME:
                    case MYSQL_TYPE_DATE:
                    case MYSQL_TYPE_DATETIME:
                    case MYSQL_TYPE_TIMESTAMP:
                        fres->jsiTypeMap = JSI_OPTION_TIME_D; //TODO: time
                        fres->mapType = MYSQL_TYPE_DATETIME;
                        fres->vsize = sizeof(MYSQL_TIME);
                        break;
                    case MYSQL_TYPE_STRING:
                    default:
                        if (IS_NUM(field->type)) {
                            fres->mapType = MYSQL_TYPE_LONGLONG;
                            fres->jsiTypeMap = JSI_OPTION_DOUBLE;
                            fres->vsize = sizeof(double);
                        } else {
                            fres->jsiTypeMap = JSI_OPTION_STRING;
                            fres->mapType = MYSQL_TYPE_STRING;
                            if (p->jdb->optPtr->prefetch)
                                fres->vsize = field->max_length;
                            else
                                fres->vsize = p->jdb->optPtr->maxString;
                            if (fres->vsize <= 0)
                                fres->vsize = JSI_BUFSIZ;
                            bindResult->buffer = fres->buffer.vstring = (char*)Jsi_Malloc(fres->vsize);
                            fres->buffer.vstring[0] = 0;
                        }
                        break;
                }
                prep->colTypes[iCnt] = fres->jsiTypeMap;
                bindResult->buffer_type = fres->mapType;
                bindResult->buffer_length = fres->vsize;
                bindResult->length = &fres->len;
                bindResult->is_null = &fres->isnull;
                iCnt++;
            }
        }
        else return JSI_BREAK;
        if (mysql_stmt_bind_result(prep->myStmt, prep->bindResult)) {
            fprintf(stderr, "mysql_stmt_bind_Result(), failed. Error:%s\n", mysql_stmt_error(prep->myStmt));
            return JSI_ERROR;
        }
        mysql_free_result(res);
    }
    return rc;
}

static Jsi_RC mdbEvalStep(MyDbEvalContext *p) {
    Jsi_RC rc = JSI_OK;
    if( p->prep==0)
        rc = mdbEvalPrep(p);
    if (rc == JSI_BREAK)
        return JSI_BREAK;
    if (rc == JSI_OK)
        rc = mdbEvalStepSub(p, 1, NULL);
    return rc;
}

const char *mysqlGetbindParamName(MysqlPrep* pStmt, int n) {
    if (n>=1 && n<=pStmt->paramCnt)
        return pStmt->paramNames[n-1];
    return NULL;
}
  
/*
enum enum_field_types { MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
                        MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
                        MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
                        MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
                        MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
                        MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
                        MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
                        MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
                        MYSQL_TYPE_BIT,
                        MYSQL_TYPE_NEWDECIMAL=246,
                        MYSQL_TYPE_ENUM=247,
                        MYSQL_TYPE_SET=248,
                        MYSQL_TYPE_TINY_BLOB=249,
                        MYSQL_TYPE_MEDIUM_BLOB=250,
                        MYSQL_TYPE_LONG_BLOB=251,
                        MYSQL_TYPE_BLOB=252,
                        MYSQL_TYPE_VAR_STRING=253,
                        MYSQL_TYPE_STRING=254,
                        MYSQL_TYPE_GEOMETRY=255

};
*/
/*
 * MYSQL_TYPE_TINY  1
MYSQL_TYPE_SHORT    2
MYSQL_TYPE_LONG 4
MYSQL_TYPE_LONGLONG 8
MYSQL_TYPE_FLOAT    4
MYSQL_TYPE_DOUBLE   8
MYSQL_TYPE_TIME sizeof(MYSQL_TIME)
MYSQL_TYPE_DATE sizeof(MYSQL_TIME)
MYSQL_TYPE_DATETIME sizeof(MYSQL_TIME)
MYSQL_TYPE_STRING   data length
MYSQL_TYPE_BLOB data_length
*/


static Jsi_RC mdbPrepareAndBind(MyDbEvalContext *p)
{
    if (mdbPrepareStmt(p) != JSI_OK)
        return JSI_ERROR;
    MysqlPrep *prep = p->prep;
    MySqlObj *jdb = p->jdb;
    Jsi_Interp *interp = jdb->interp;
    Jsi_Value *pv = NULL, *apv = NULL;
    char tname[50];

    Jsi_RC rc = JSI_OK;
    int i, n, nVar = prep->numParam;

    if (nVar<=0)
        return rc;
    if (!prep->bindParam)
        prep->bindParam = (MYSQL_BIND*)Jsi_Calloc(nVar, sizeof(MYSQL_BIND));
    else
        memset(prep->bindParam, 0, (nVar * sizeof(MYSQL_BIND)));
    for(i=1; i<=nVar; i++) {
        int btype = 0;
        int isInt = 0;
        int isBlob = 0;
        const char *zVar = mysqlGetbindParamName(prep, i);
        tname[0] = 0;
        if (zVar == NULL) {
            if (!jdb->optPtr || !(apv=jdb->optPtr->values))
                return Jsi_LogError("? bind without values for param %d", i);
            if (!(pv =Jsi_ValueArrayIndex(interp, apv, i-1))) 
                return Jsi_LogError("array element %d missing", nVar);
        }
        else if ((zVar[0]!='$' && zVar[0]!=':' && zVar[0]!='@') ) 
            return Jsi_LogError("can not find bind var %s", zVar); else {
           
            int zvLen = Jsi_Strlen(zVar);
            char *zcp;
            if (zVar[0] =='$' && ((zcp = Jsi_Strchr(zVar,'('))) && zVar[zvLen-1] == ')')
            {
                Jsi_DString vStr;
                Jsi_DSInit(&vStr);
                Jsi_DSAppendLen(&vStr, zVar+1, (zcp-zVar-1));
                int slen = Jsi_Strlen(zcp);
                const char *ttp;
                if (jdb->optPtr->typeCheck!=mdbTypeCheck_None && (ttp = Jsi_Strchr(zVar,':'))) {
                    // Extract bind-type.
                    Jsi_DString tStr = {};
                    int tlen = Jsi_Strlen(ttp+1);
                    Jsi_DSAppendLen(&tStr, ttp+1, tlen-1);
                    snprintf(tname, sizeof(tname), "%s", Jsi_DSValue(&tStr));
                    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(jdb->typeNameHash, tname);
                    assert(hPtr);
                    btype = (long)Jsi_HashValueGet(hPtr);
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
        }
        Jsi_Number r;
        SqlFieldResults *fres = prep->fieldParam+i-1;
        MYSQL_BIND *bind = prep->bindParam+i-1;
        memset(bind, 0, sizeof(*bind));
        // Now create binding.
        if(!pv ) {
            if (!jdb->bindWarn) {
                rc = Jsi_LogError("unknown bind param: %s", zVar);
                break;
            } else
                Jsi_LogWarn("unknown bind param: %s", zVar);
        } else {
            if (btype && !Jsi_ValueIsUndef(interp, pv)) {
                int done = 0, match = 1, cast = (jdb->optPtr->typeCheck==mdbTypeCheck_Cast);
                switch (btype) {
                    case MYSQL_TYPE_BLOB:
                        isBlob = 1;
                    case MYSQL_TYPE_STRING:
                        if (cast)
                            Jsi_ValueToString(interp, pv, &n);
                        else
                            match = Jsi_ValueIsString(interp, pv); 
                        break;
                    case MYSQL_TYPE_DOUBLE:
                        if (cast)
                            Jsi_ValueToNumber(interp, pv);
                        else
                            match = Jsi_ValueIsNumber(interp, pv); 
                        break;
                    case MYSQL_TYPE_LONGLONG:
                        isInt = 1;
                        if (cast)
                            Jsi_ValueToNumber(interp, pv);
                        else
                            match = Jsi_ValueIsNumber(interp, pv); 
                        break;
                    case MYSQL_TYPE_TINY:
                        if (cast)
                            Jsi_ValueToBool(interp, pv);
                        else
                            match = Jsi_ValueIsBoolean(interp, pv); 
                        break;
                    case MYSQL_TYPE_TIME:
                    case MYSQL_TYPE_DATE:
                    case MYSQL_TYPE_DATETIME:
                    case MYSQL_TYPE_TIMESTAMP:
                        if (cast)
                            Jsi_ValueToNumber(interp, pv);
                        else if (!Jsi_ValueIsNumber(interp, pv))
                            goto errout;
                        Jsi_GetNumberFromValue(interp, pv, &r);
                        bind->buffer_type = (enum enum_field_types)btype;
                        bind->buffer=&fres->buffer.timestamp;
                        bind->buffer_length = sizeof(fres->buffer.timestamp);
                        bind->length = NULL;
                        mdbJsToMyTime(r, &fres->buffer.timestamp, 1);
                        done = 1;
                        break;
                    default:
                        Jsi_LogBug("Unhandled bind type: %s = %d", tname, btype);
                }
                if (done)
                    continue;
                 if (cast == 0 && match == 0) 
errout:
                 {
                    int ltyp = (jdb->optPtr->typeCheck==mdbTypeCheck_Error?JSI_LOG_ERROR:JSI_LOG_WARN);
                    Jsi_LogMsg(interp, ltyp, "bind param \"%s\" type is not \"%s\"", zVar, tname);
                    if (ltyp == JSI_LOG_ERROR)
                        return JSI_ERROR;
                }
            }
            
            if (Jsi_ValueIsBoolean(interp, pv)) {
                bool nb;
                Jsi_GetBoolFromValue(interp, pv, &nb);
                n = nb;
                bind->buffer_type = MYSQL_TYPE_TINY;
                bind->buffer = &fres->buffer.vchar;
                bind->buffer_length = sizeof(fres->buffer.vchar);
                bind->length = &fres->len;
                //bind->is_null = &fres->isnull;
                fres->buffer.vchar = n;
                
            } else if (Jsi_ValueIsNumber(interp, pv)) {
                Jsi_Number r;
                Jsi_Wide wv;
                Jsi_GetNumberFromValue(interp, pv, &r);
                wv = (Jsi_Wide)r;
                bind->buffer_type = MYSQL_TYPE_DOUBLE;
                bind->buffer = &fres->buffer.vdouble;
                bind->buffer_length = sizeof(fres->buffer.vdouble);
                bind->length = &fres->len;
                fres->buffer.vdouble = (double)r;
                if (isInt || (jdb->forceInt && (((Jsi_Number)wv)-r)==0)) {
                    bind->buffer = &fres->buffer.vlonglong;
                    bind->buffer_type = MYSQL_TYPE_LONGLONG;
                    bind->buffer_length = sizeof(fres->buffer.vlonglong);
                    fres->buffer.vlonglong = wv;
                }
            } else if (Jsi_ValueIsNull(interp, pv) || (Jsi_ValueIsUndef(interp, pv) && jdb->optPtr->mapundef)) {
bindnull:
                bind->buffer_type = MYSQL_TYPE_NULL;
                bind->buffer = &fres->buffer.vchar;
                bind->buffer_length = sizeof(fres->buffer.vchar);
                bind->length = &fres->len;
                fres->buffer.vchar = 0;

            } else if (Jsi_ValueIsString(interp, pv)) {
                char *sstr = Jsi_ValueGetStringLen(interp, pv, &n);
                bind->buffer_type = MYSQL_TYPE_STRING;
                bind->buffer=sstr;
                bind->buffer_length = n;
                bind->length = NULL;
                if (isBlob)
                    bind->buffer_type = MYSQL_TYPE_BLOB;
            } else {
                if (!jdb->bindWarn) {
                    rc = Jsi_LogError("bind param must be string/number/bool/null: %s", zVar);
                    break;
                } else {
                    Jsi_LogWarn("bind param must be string/number/bool/null: %s", zVar);
                    goto bindnull;
                }
            }
        }
    }
    if (mysql_stmt_bind_param(prep->myStmt, prep->bindParam))
        rc = Jsi_LogError("bind failed: %s", mysql_error(jdb->db));
    return rc;
}


static void mdbClose(MYSQL  *db) {
        mysql_close(db);
}

static Jsi_RC mdbIsNumArray(Jsi_Interp *interp, Jsi_Value *value)
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
** JSI calls this procedure when an MYSQL  database command is
** deleted.
*/
static void mdbDeleteCmd(MySqlObj *jdb)
{
    Jsi_Interp *interp = jdb->interp;
    if (jdb->debug & mdbTMODE_DELETE)
        JSI_DBQUERY_PRINTF( "DEBUG: delete\n");
    if (jdb->stmtCache)
        mdbFlushStmtCache(jdb);
    if (jdb->stmtHash)
        Jsi_HashDelete(jdb->stmtHash);
    //closeIncrblobChannels(jdb);
    if (jdb->db) {
        mdbClose(jdb->db);
    }
    while( jdb->pFunc ) {
        SqlFunc *pFunc = jdb->pFunc;
        jdb->pFunc = pFunc->pNext;
        Jsi_DSFree(&pFunc->dScript);
        Jsi_DecrRefCount(interp, pFunc->tocall);
        Jsi_Free((char*)pFunc);
    }
    if( jdb->zNull ) {
        Jsi_Free(jdb->zNull);
    }
    if (jdb->typeNameHash)
        Jsi_HashDelete(jdb->typeNameHash);
    Jsi_OptionsFree(interp, SqlOptions, jdb, 0);
    if (jdb->stmtCache)
        Jsi_ListDelete(jdb->stmtCache);
}

/*
** Return a JSON formatted value for the iCol'th column of the row currently pointed to by
** the MyDbEvalContext structure passed as the first argument.
*/
static void mdbEvalSetColumnJSON(MyDbEvalContext *p, int iCol, Jsi_DString *dStr) {
    Jsi_Interp *interp = p->jdb->interp;
    char nbuf[200];
    MysqlPrep *prep = p->prep;
    SqlFieldResults *field = prep->fieldResult+iCol;
    if (field->isnull) {
        Jsi_DSAppend(dStr, "null", NULL);
        return;
    }
    const char *zBlob = "";
    int bytes = 0;

    switch(field->jsiTypeMap) {
        case JSI_OPTION_BOOL: {
            snprintf(nbuf, sizeof(nbuf), "%s", field->buffer.vchar?"true":"false");
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        case JSI_OPTION_INT64: {
            snprintf(nbuf, sizeof(nbuf), "%lld", field->buffer.vlonglong);
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        case JSI_OPTION_DOUBLE: {
            Jsi_NumberToString(interp, field->buffer.vdouble, nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        //case JSI_OPTION_TIME_T:
        case JSI_OPTION_TIME_D:
        case JSI_OPTION_TIME_W: {
            Jsi_Number jtime = mdbMyTimeToJS(&field->buffer.timestamp);
            Jsi_NumberToString(interp, jtime, nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        case JSI_OPTION_STRING:
            zBlob = field->buffer.vstring;
        default:
        {
            if( !zBlob ) {
                Jsi_DSAppend(dStr, "null", NULL);
                return;
            }
            Jsi_JSONQuote(interp, zBlob, bytes, dStr);
            return;
        }
    }
}

static void mdbEvalSetColumn(MyDbEvalContext *p, int iCol, Jsi_DString *dStr) {
    //Jsi_Interp *interp = p->jdb->interp;
    char nbuf[200];
    MysqlPrep *prep = p->prep;
    SqlFieldResults *field = prep->fieldResult+iCol;
    Jsi_Interp *interp = p->jdb->interp;
    if (field->isnull)
        return;
        
    switch(field->jsiTypeMap) {
        case JSI_OPTION_STRING: {
            int bytes = field->len;
            const char *zBlob = field->buffer.vstring;
    
            if( !zBlob ) {
                const char *nv = p->jdb->optPtr->nullvalue;
                Jsi_DSAppend(dStr, nv?nv:"null", NULL);
                return;
            }
            Jsi_DSAppendLen(dStr, zBlob, bytes);
            return;
        }
        case JSI_OPTION_BOOL: {
            snprintf(nbuf, sizeof(nbuf), "%s", field->buffer.vchar?"true":"false");
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        case JSI_OPTION_INT64: {
            snprintf(nbuf, sizeof(nbuf), "%lld", field->buffer.vlonglong);
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        //case JSI_OPTION_TIME_T:
        case JSI_OPTION_TIME_D:
        case JSI_OPTION_TIME_W: {
            Jsi_Number jtime = mdbMyTimeToJS(&field->buffer.timestamp);
            Jsi_NumberToString(interp, jtime, nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        case JSI_OPTION_DOUBLE: {
            Jsi_NumberToString(interp, field->buffer.vdouble, nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        default:
            Jsi_LogWarn("unknown type: %d", field->jsiTypeMap);
    
    }
}


static Jsi_Value* mdbEvalSetColumnValue(MyDbEvalContext *p, int iCol, Jsi_Value **val) {
    Jsi_Interp *interp = p->jdb->interp;
    MysqlPrep *prep = p->prep;
    SqlFieldResults *field = prep->fieldResult+iCol;
    if (field->isnull)
        return Jsi_ValueMakeNull(interp, val);
        
    switch(field->jsiTypeMap) {
        case JSI_OPTION_STRING: {
            int bytes = field->len;
            char *zBlob = field->buffer.vstring;
            if( !zBlob ) {
                return Jsi_ValueMakeNull(interp, val);
            }
            zBlob = (char*)Jsi_Malloc(bytes+1);
            memcpy(zBlob, field->buffer.vstring, bytes);
            zBlob[bytes] = 0;
            return Jsi_ValueMakeBlob(interp, val, (unsigned char*)zBlob, bytes+1);
        }
        case JSI_OPTION_BOOL:
            return Jsi_ValueMakeBool(interp, val, field->buffer.vchar);
        case JSI_OPTION_INT64:
             return Jsi_ValueMakeNumber(interp, val, (Jsi_Number)field->buffer.vlonglong);
        //case JSI_OPTION_TIME_T:
        case JSI_OPTION_TIME_D:
        case JSI_OPTION_TIME_W: {
            Jsi_Number jtime = mdbMyTimeToJS(&field->buffer.timestamp);
            return Jsi_ValueMakeNumber(interp, val, jtime);
        }
        case JSI_OPTION_DOUBLE:
             return Jsi_ValueMakeNumber(interp, val, (Jsi_Number)field->buffer.vdouble);
        default:
            Jsi_LogWarn("unknown type: %d", field->jsiTypeMap);
    }
    return Jsi_ValueNew1(interp);
}

static Jsi_RC mdbEvalCallCmd( MyDbEvalContext *p, Jsi_Interp *interp, Jsi_RC result)
{
    int cnt = 0;
    Jsi_RC rc = result;
    Jsi_Value *varg1;
    Jsi_Obj *argso;
    char **apColName = NULL;
    Jsi_OptionId *apColType = NULL;
    if (p->jdb->debug & mdbTMODE_EVAL)
        JSI_DBQUERY_PRINTF( "DEBUG: eval\n");

    while( (rc==JSI_OK) && JSI_OK==(rc = mdbEvalStep(p)) ) {
        int i;
        int nCol;

        cnt++;
        mdbEvalRowInfo(p, &nCol, &apColName, &apColType);
        if (nCol<=0)
            continue;
        if (Jsi_ValueIsNull(interp,p->tocall))
            continue;
        /* Single object containing sql result members. */
        varg1 = Jsi_ValueMakeObject(interp, NULL, argso = Jsi_ObjNew(interp));
        for(i=0; i<nCol; i++) {
            Jsi_Value *nnv = mdbEvalSetColumnValue(p, i, NULL);
            Jsi_ObjInsert(interp, argso, apColName[i], nnv, 0);
        }
        Jsi_IncrRefCount(interp, varg1);
        bool rb = Jsi_FunctionInvokeBool(interp, p->tocall, varg1);
        if (Jsi_InterpGone(interp))
            return JSI_ERROR;
        Jsi_DecrRefCount(interp, varg1);
        if (rb)
            break;
    }
    //mdbEvalFinalize(p);

    if( rc==JSI_OK || rc==JSI_BREAK ) {
        //Jsi_ResetResult(interp);
        rc = JSI_OK;
    }
    return rc;
}

static MySqlObj *_mysql_getDbHandle(Jsi_Interp *interp, Jsi_Value *_this, Jsi_Func *funcPtr)
{
    MySqlObj *jdb = (MySqlObj*)Jsi_UserObjGetData(interp, _this, funcPtr);
    if (!jdb) {
        Jsi_LogError("MySql call to a non-mysql object");
        return NULL;
    }
    if (!jdb->db)
    {
        Jsi_LogError("MySql db is closed");
        return NULL;
    }
    return jdb;
}

static void mysqlObjErase(MySqlObj *jdb)
{
    mdbDeleteCmd(jdb);
    jdb->db = NULL;
}

static Jsi_RC mysqlObjFree(Jsi_Interp *interp, void *data)
{
    MySqlObj *jdb = (MySqlObj*)data;
    SQLSIGASSERT(jdb,DB);
    jdb->_->activeCnt--;
    mysqlObjErase(jdb);
    _JSI_MEMCLEAR(jdb);
    Jsi_Free(jdb);
    return JSI_OK;
}

static bool mysqlObjIsTrue(void *data)
{
    MySqlObj *jdb = (MySqlObj*)data;
    SQLSIGASSERT(jdb,DB);
    if (!jdb->db) return 0;
    else return 1;
}

static bool mysqlObjEqual(void *data1, void *data2)
{
    //SQLSIGASSERT(data1,DB);
    //SQLSIGASSERT(data2,DB);
    return (data1 == data2);
}
static MYSQL* mdbConnect(Jsi_Interp *interp, MySqlObj* jdb)
{
    return mysql_real_connect(jdb->db,
        jdb->host?Jsi_ValueString(interp, jdb->host, NULL):NULL,
        jdb->user, jdb->password, jdb->database,
        jdb->port, 0, jdb->dbflags);
}

#define FN_MySql JSI_INFO("\
Create a mysql client.")
static Jsi_RC MySqlConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);

static Jsi_RC MySqlReconnectCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                         Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb  = _mysql_getDbHandle(interp, _this, funcPtr);
    if (!jdb) return JSI_ERROR;
    int oldMax = jdb->maxStmts;
    jdb->maxStmts = 0;
    mdbStmtLimit(jdb);
    jdb->maxStmts = oldMax;
    mysql_close(jdb->db);
    jdb->db = mysql_init(NULL);
    if (!mdbConnect(interp, jdb)) 
        return Jsi_LogError("reconnect failed: %s", mysql_error(jdb->db));
    return JSI_OK;
}

#define FN_evaluate JSI_INFO("\
Variable binding is NOT performed.  \
Returns number of modified fields")
static Jsi_RC MySqlEvalCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                         Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb  = _mysql_getDbHandle(interp, _this, funcPtr);
    if (!jdb) return JSI_ERROR;
    int zLen, cnt = 0;
    Jsi_RC rc = JSI_OK;
    const char *zSql = Jsi_ValueArrayIndexToStr(interp, args, 0, &zLen);

    if (mysql_real_query(jdb->db, zSql, zLen))
        rc = Jsi_LogError("mysql error: %s", mysql_error(jdb->db));
    else if (jdb->enableMulti) {
        MYSQL_RES *results;
        int sr = mysql_next_result(jdb->db);
        while (sr == 0 && (results = mysql_store_result(jdb->db)))
            mysql_free_result(results);
        cnt = mysql_field_count(jdb->db);
    }
        
    if (rc == JSI_OK)
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)cnt);
    return rc;
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
static void mdbOutputCsv(QueryOpts *p, const char *z, Jsi_DString *dStr, int bSep)
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

static void mdbOutputHtmlString(QueryOpts *p, const char *z, Jsi_DString *dStr)
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
static void mdbOutputQuotedString(Jsi_DString *dStr, const char *z) {
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

static Jsi_RC MySqlQueryCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                             Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_RC rc = JSI_OK;
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *vSql = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_DString eStr = {};
    JSI_DSTRING_VAR(dStr, JSI_DB_DSTRING_SIZE);
    const char *zSql = Jsi_ValueGetDString(interp, vSql, &eStr, 0);
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    int cnt = 0;
    char **apColName = NULL;
    Jsi_OptionId *apColType = NULL;
    int isopts = 0;
    MyDbEvalContext sEval = {};
    QueryOpts opts, *oEopt = NULL;
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
        else  {
            rc = Jsi_LogError("argument must be null, a function, string, array or options");
            goto bail;
        }
    }

    if (isopts) {
        if (Jsi_OptionsProcess(interp, QueryFmtOptions, &opts, arg, 0) < 0) {
            rc = JSI_ERROR;
            goto bail;
        }
        callback = (opts.callback ? opts.callback : jdb->queryOpts.callback);
        width = (opts.width ? opts.width : jdb->queryOpts.width);
    }
/*    if (jdb->queryOpts.CData) {
        char *cdata = (char*)jdb->queryOpts.CData;
        MySqlObjMultipleBind* copts = Jsi_CarrayLookup(interp, cdata);
        if (!copts) 
            return Jsi_LogError("unknown CData option: %s", jdb->queryOpts.CData);
        int n = MySqlObjQuery(jdb, copts->opts, copts->data, copts->numData, zSql, copts->flags);
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)n);
        return JSI_OK;
    } */
    if (opts.objName) {
        if (Jsi_SqlObjBinds(interp, &eStr, opts.objName,  !(opts.objOpts&OBJMODE_NOTYPES), 
        !(opts.objOpts&OBJMODE_NODEFAULTS), (opts.objOpts&OBJMODE_NULLDEFAULTS)!=0) != JSI_OK)
            goto bail;
        zSql = Jsi_DSValue(&eStr);
    }
    if (!opts.separator) {
        switch (opts.mode) {
            case _mdb_EF_LIST: opts.separator = "|"; break;
            case _mdb_EF_COLUMN: opts.separator = " "; break;
            case _mdb_EF_TABS: opts.separator = "\t"; break;
            default: opts.separator = ",";
        }
    }
    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    sEval.nocache = opts.nocache;
    if (mdbEvalInit(interp, &sEval, jdb, zSql, &sStr, 0, 0) != JSI_OK) {
        rc = JSI_ERROR;
        goto bail;
    }
    sEval.namedParams = (opts.noNamedParams==0 && !opts.values);
    sEval.ret = *ret;
    oEopt = jdb->optPtr;
    jdb->optPtr = &opts;
    
    if (sEval.namedParams) {
        rc = mdbEvalPrep(&sEval);
        if (rc == JSI_ERROR)
            goto bail;
        if (rc == JSI_BREAK) {
            rc = JSI_OK;
            goto bail;
        }
    }
    if (opts.mode == _mdb_EF_NONE)
        goto bail;
    if (callback) {
        sEval.tocall = callback;
        if (opts.mode != _mdb_EF_ROWS)
            rc = Jsi_LogError("'mode' must be 'rows' with 'callback'");
        else 
            rc = mdbEvalCallCmd(&sEval, interp, JSI_OK);
        goto bail;
    }
    switch (opts.mode) {
        case _mdb_EF_NONE:
            while(JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            goto bail;
            break;
        case _mdb_EF_JSON:
            if (opts.headers) {
                Jsi_DSAppend(dStr, "[ ", NULL);
                while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                    int i;
                    int nCol;
                    mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
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
                        mdbEvalSetColumnJSON(&sEval, i, dStr);
                    }
                    Jsi_DSAppend(dStr, "]", NULL);
                    cnt++;
                    if (opts.limit && cnt>opts.limit) break;
                }
                Jsi_DSAppend(dStr, " ]", NULL);
                
            } else {
                Jsi_DSAppend(dStr, "[ ", NULL);
                while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                    int i;
                    int nCol;
                    mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
                    if (cnt)
                        Jsi_DSAppend(dStr, ", ", NULL);
                    Jsi_DSAppend(dStr, "{", NULL);
                    for(i=0; i<nCol; i++) {
                        if (i)
                            Jsi_DSAppend(dStr, ", ", NULL);
                        Jsi_JSONQuote(interp, apColName[i], -1, dStr);
                        Jsi_DSAppend(dStr, ":", NULL);
                        mdbEvalSetColumnJSON(&sEval, i, dStr);
                    }
                    Jsi_DSAppend(dStr, "}", NULL);
                    cnt++;
                    if (opts.limit && cnt>=opts.limit) break;
                }
                Jsi_DSAppend(dStr, " ]", NULL);
            }
            break;
            
        case _mdb_EF_JSON2: {
                while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                    int i;
                    int nCol;
                    mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
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
                        mdbEvalSetColumnJSON(&sEval, i, dStr);
                    }
                    Jsi_DSAppend(dStr, " ]", NULL);
                    cnt++;
                    if (opts.limit && cnt>=opts.limit) break;
                }
                if (cnt)
                    Jsi_DSAppend(dStr, " ] } ", NULL);
            }
            break;
            
        case _mdb_EF_LIST:
            while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
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
                    mdbEvalSetColumn(&sEval, i, dStr);
                }
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            break;
            
        case _mdb_EF_COLUMN: {
            int *wids = NULL;
            Jsi_DString vStr = {};
            while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                int i, w;
                int nCol;
                
                mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
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
                    mdbEvalSetColumn(&sEval, i, &vStr);
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
        
        case _mdb_EF_INSERT: {
            Jsi_DString vStr = {};    
            while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                const char *tbl = (opts.table ? opts.table : "table");
                if (cnt)
                    Jsi_DSAppend(dStr, "\n", NULL);
                Jsi_DSAppend(dStr, "INSERT INTO ", tbl, " VALUES(", NULL);
                mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
                for(i=0; i<nCol; i++) {
                    Jsi_Number dv;
                    const char *azArg;
                    Jsi_DSSetLength(&vStr, 0);
                    mdbEvalSetColumn(&sEval, i, &vStr);
                    
                    MysqlPrep *prep = sEval.prep;
                    Jsi_OptionId ptype = prep->fieldResult[i].jsiTypeMap;
                    
                    azArg = Jsi_DSValue(&vStr);
                    const char *zSep = i>0 ? ",": "";
                    if (azArg[i]==0 && ptype != JSI_OPTION_STRING) {
                      Jsi_DSAppend(dStr, zSep, "NULL", NULL);
                    } else if( ptype ==JSI_OPTION_STRING) {
                      if( zSep[0] ) Jsi_DSAppend(dStr,zSep, NULL);
                      mdbOutputQuotedString(dStr, azArg);
                    } else if (ptype==JSI_OPTION_BOOL || ptype ==JSI_OPTION_DOUBLE) {
                      Jsi_DSAppend(dStr, zSep, azArg, NULL);
                    } else if( Jsi_GetDouble(interp, azArg, &dv) == JSI_OK ) {
                      Jsi_DSAppend(dStr, zSep, azArg, NULL);
                    } else {
                      if( zSep[0] ) Jsi_DSAppend(dStr,zSep, NULL);
                      mdbOutputQuotedString(dStr, azArg);
                    }
                }
                Jsi_DSAppend(dStr, ");", NULL);
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            Jsi_DSFree(&vStr);
        }
    
        case _mdb_EF_TABS:
        case _mdb_EF_CSV: {
            Jsi_DString vStr = {};  
            while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
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
                    mdbEvalSetColumn(&sEval, i, &vStr);
                    if (opts.mode == _mdb_EF_CSV)
                        mdbOutputCsv(&opts, Jsi_DSValue(&vStr), dStr, 0);
                    else
                        Jsi_DSAppend(dStr, Jsi_DSValue(&vStr), NULL);
                }
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            Jsi_DSFree(&vStr);
            break;
        }
            
        case _mdb_EF_LINE: {
            int i, w = 5, ww;
            int nCol;
            Jsi_DString vStr = {};   
            while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
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
                    mdbEvalSetColumn(&sEval, i, &vStr);
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
            
        case _mdb_EF_HTML: {
            Jsi_DString vStr = {};   
            while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
                if (cnt == 0 && opts.headers) {
                    Jsi_DSAppend(dStr, "<TR>", NULL);
                    for(i=0; i<nCol; i++) {
                        Jsi_DSAppend(dStr, "<TH>", NULL);
                        mdbOutputHtmlString(&opts, apColName[i], dStr);
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
                    mdbEvalSetColumn(&sEval, i, &vStr);
                    mdbOutputHtmlString(&opts, Jsi_DSValue(&vStr), dStr);
                    Jsi_DSAppend(dStr, "</TD>", NULL);
                }
                Jsi_DSAppend(dStr, "</TR>", NULL);
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            Jsi_DSFree(&vStr);
            break;
        }
            
        case _mdb_EF_ROWS:
        {
            Jsi_Value *vcur, *vrow;
            int cnt = 0;
            Jsi_Obj *oall, *ocur;
            Jsi_ValueMakeArrayObject(interp, ret, oall = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
    
            while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
                ocur = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
                vrow = Jsi_ValueMakeObject(interp, NULL, ocur);
                for(i=0; i<nCol; i++) {
                    vcur = mdbEvalSetColumnValue(&sEval, i, NULL);
                    Jsi_ObjInsert(interp, ocur, apColName[i], vcur, 0);
                }
                Jsi_ObjArrayAdd(interp, oall, vrow);
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            mdbEvalFinalize(&sEval);
            if (rc != JSI_ERROR)
                rc = JSI_OK;
            goto bail;
            break;
        }
        case _mdb_EF_ARRAYS:
        {
            Jsi_Value *vcur, *vrow;
            int cnt = 0;
            Jsi_Obj *oall, *ocur;
            Jsi_ValueMakeArrayObject(interp, ret, oall = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
    
            while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
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
                    vcur = mdbEvalSetColumnValue(&sEval, i, NULL);
                    Jsi_ObjArrayAdd(interp, ocur, vcur);
                }
                Jsi_ObjArrayAdd(interp, oall, vrow);
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            mdbEvalFinalize(&sEval);
            if (rc != JSI_ERROR)
                rc = JSI_OK;
            goto bail;
            break;
        }
        case _mdb_EF_ARRAY1D:
        {
            Jsi_Value *vcur;
            int cnt = 0;
            Jsi_Obj *oall;
            Jsi_ValueMakeArrayObject(interp, ret, oall = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
    
            while( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(&sEval, &nCol, &apColName, &apColType);
                if (cnt == 0 && opts.headers) {
                    for(i=0; i<nCol; i++) {
                        vcur = Jsi_ValueNewStringDup(interp, apColName[i]);
                        Jsi_ObjArrayAdd(interp, oall, vcur);
                    }
                }
                for(i=0; i<nCol; i++) {
                    vcur = mdbEvalSetColumnValue(&sEval, i, NULL);
                    Jsi_ObjArrayAdd(interp, oall, vcur);
                }
                cnt++;
                if (opts.limit && cnt>=opts.limit) break;
            }
            mdbEvalFinalize(&sEval);
            if (rc != JSI_ERROR)
                rc = JSI_OK;
            goto bail;
            break;
        }
    }
    if( rc==JSI_BREAK ) {
        rc = JSI_OK;
    }
    Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(dStr));
bail:
    mdbEvalFinalize(&sEval);
    if (isopts) {
        Jsi_OptionsFree(interp, QueryFmtOptions, &opts, 0);
    }
    Jsi_DSFree(dStr);
    Jsi_DSFree(&eStr);
    jdb->optPtr = oEopt;

    return rc;
}

static Jsi_RC MySqlOnecolumnCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                          Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_RC rc;
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *vSql = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    MyDbEvalContext sEval = {};
    const char *zSql = Jsi_ValueGetDString(interp, vSql, &dStr, 0);

    sEval.nocache = jdb->queryOpts.nocache;
    if (mdbEvalInit(interp, &sEval, jdb, zSql, &sStr, 0, 0) != JSI_OK)
        return JSI_ERROR;
    sEval.ret = *ret;
    sEval.tocall = NULL;
    int cnt = 0;


    if( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
        int nCol = sEval.prep->numCol;
        if (nCol>0)
            mdbEvalSetColumnValue(&sEval, 0, ret);
        cnt++;
    }
    mdbEvalFinalize(&sEval);
    if( rc==JSI_BREAK ) {
        rc = JSI_OK;
    }
    Jsi_DSFree(&dStr);
    return rc;
}

static Jsi_RC MySqlExistsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                           Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_RC rc;
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *vSql = Jsi_ValueArrayIndex(interp, args, 0);
    const char *zSql;
    Jsi_DString dStr = {};
    MyDbEvalContext sEval = {};
    zSql = Jsi_ValueGetDString(interp, vSql, &dStr, 0);

    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    sEval.nocache = jdb->queryOpts.nocache;
    if (mdbEvalInit(interp, &sEval, jdb, zSql, &sStr, 0, 0) != JSI_OK)
        return JSI_ERROR;
    sEval.ret = *ret;
    int cnt = 0;


    if( JSI_OK==(rc = mdbEvalStep(&sEval)) ) {
        int nCol = sEval.prep->numCol;
        if (nCol>0)
            cnt++;
    }
    mdbEvalFinalize(&sEval);
    if( rc==JSI_BREAK ) {
        rc = JSI_OK;
    }
    Jsi_DSFree(&dStr);
    Jsi_ValueMakeBool(interp, ret, cnt);
    return rc;
}

static Jsi_RC MySqlLastRowidCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                                    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Wide rowid;
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    rowid = mdbLastInsertRowid(jdb);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)rowid);
    return JSI_OK;
}

/*
static Jsi_RC MySqlInterruptCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                              Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    _SQL_LITE_N_(_interrupt)(jdb->db);
    return JSI_OK;
} */


static Jsi_RC MySqlCompleteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                             Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Value *s = Jsi_ValueArrayIndex(interp, args, 0);
    const char *str =  Jsi_ValueString(interp, s, NULL);
    int isComplete = 0;
    if (str)
        isComplete = 0; // sqlite3_complete( str );
    Jsi_ValueMakeBool(interp, ret, isComplete);
    return JSI_OK;
}

static Jsi_RC MySqlErrorNoCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                              Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    int n = mysql_errno(jdb->db);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)n);
    return JSI_OK;
}

static Jsi_RC MySqlErrorStateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                              Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    const char *str = mysql_sqlstate(jdb->db);
    if (str)
        Jsi_ValueMakeStringDup(interp, ret, str);
    return JSI_OK;
}

static Jsi_RC MySqlLastQueryCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                              Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    const char *str = mysql_info(jdb->db);
    if (str)
        Jsi_ValueMakeStringDup(interp, ret, str);
    return JSI_OK;
}

static Jsi_RC MySqlResetCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                              Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
#if (MYSQL_VERSION_ID >= 50703 && !defined(JSI_NO_MYSQL_RESET))
    int oldMax = jdb->maxStmts;
    jdb->maxStmts = 0;
    mdbStmtLimit(jdb);
    jdb->maxStmts = oldMax;
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)mysql_reset_connection(jdb->db));
#else
    Jsi_LogWarn("mysql reset unavailable: requires version 5.7.3+");
#endif
    return JSI_OK;    
}


static Jsi_RC MySqlPingCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                              Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    int n = mysql_ping(jdb->db);
    bool noErr = 0;
    Jsi_Value *val = Jsi_ValueArrayIndex(interp, args, 0);
    if (val)
        Jsi_ValueGetBoolean(interp, val, &noErr);
    if (n && noErr==0) 
        return Jsi_LogError("ping failed: (%d) %s", n, mysql_error(jdb->db));
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)n);

    return JSI_OK;    
}

static Jsi_RC MySqlAffectedRowsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                              Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)mysql_affected_rows(jdb->db));
    return JSI_OK;
}
static Jsi_RC MySqlInfoCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                              Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    Jsi_Obj *nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeObject(interp, ret, nobj);
    const char *str, *svals[20];
    int i = 0;
    svals[i++] = "clientInfo";
    svals[i++] = mysql_get_client_info();
    svals[i++] = "hostInfo";
    svals[i++] = mysql_get_host_info(jdb->db);
    svals[i++] = "serverInfo";
    svals[i++] = mysql_get_server_info(jdb->db);
    svals[i++] = "stat";
    svals[i++] = mysql_stat(jdb->db);
    svals[i++] = 0;
    i = 0;
    while (svals[i]) {
        str = svals[i+1];
        Jsi_ObjInsert(interp, nobj, svals[i], str?Jsi_ValueNewStringDup(interp, str):Jsi_ValueNewNull(interp), 0);
    }
    Jsi_ObjInsert(interp, nobj, "threadId", Jsi_ValueNewNumber(interp, (Jsi_Number)mysql_thread_id(jdb->db)), 0);
    Jsi_ObjInsert(interp, nobj, "protocolVersion", Jsi_ValueNewNumber(interp, (Jsi_Number)mysql_get_proto_info(jdb->db)), 0);
    Jsi_ObjInsert(interp, nobj, "clientVersion", Jsi_ValueNewNumber(interp, (Jsi_Number)mysql_get_client_version()), 0);
    Jsi_ObjInsert(interp, nobj, "serverVersion", Jsi_ValueNewNumber(interp, (Jsi_Number)mysql_get_server_version(jdb->db)), 0);
    Jsi_ObjInsert(interp, nobj, "warningCount", Jsi_ValueNewNumber(interp, (Jsi_Number)mysql_warning_count(jdb->db)), 0);
    return JSI_OK;
}

static Jsi_RC MySqlConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                         Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    if (!(jdb = _mysql_getDbHandle(interp, _this, funcPtr))) return JSI_ERROR;
    int oms = jdb->maxStmts;
    const char *oldu = jdb->user, *oldpw = jdb->password, *olddb = jdb->database;
    Jsi_RC rc = Jsi_OptionsConf(interp, SqlOptions, jdb, Jsi_ValueArrayIndex(interp, args, 0), ret, 0);
    if (jdb->maxStmts<0 || jdb->maxStmts>MAX_PREPARED_STMTS) {
        JSI_DBQUERY_PRINTF( "option maxStmts value %d is not in range 0..%d", jdb->maxStmts, MAX_PREPARED_STMTS);
        jdb->maxStmts = oms;
        rc = JSI_ERROR;
    }
    if (oldu != jdb->user || oldpw != jdb->password || olddb != jdb->database)
        if (!mysql_change_user(jdb->db, jdb->user, jdb->password, jdb->database)) {
            rc = JSI_ERROR;
            jdb->user = oldu;
            jdb->password = oldpw;
            jdb->database = olddb;
        }
    mdbStmtLimit(jdb);
    return rc;
}

static Jsi_CmdSpec mysqlCmds[] = {
    { "MySql",          MySqlConstructor,    0,  1,  "options:object=void",  
        .help="Create a new db connection to a MySql database:", .retType=(uint)JSI_TT_USEROBJ, .flags=JSI_CMD_IS_CONSTRUCTOR, .info=0, .opts=SqlOptions },
    { "affectedRows",   MySqlAffectedRowsCmd,0, 0, "", .help="Return affected rows", .retType=(uint)JSI_TT_NUMBER },
    { "complete",       MySqlCompleteCmd,   1,  1, "sql:string", .help="Return true if sql is complete", .retType=(uint)JSI_TT_BOOLEAN },
    { "conf",           MySqlConfCmd,       0,  1, "options:string|object=void", .help="Configure options", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=SqlOptions },
    { "errorNo",        MySqlErrorNoCmd,    0,  0, "", .help = "Return error code returned by most recent call to mysql3_exec()", .retType=(uint)JSI_TT_NUMBER },
    { "errorState",     MySqlErrorStateCmd, 0,  0, "", .help = "Return the mysql error state str" , .retType=(uint)JSI_TT_STRING},
    { "eval",           MySqlEvalCmd,       1,  1, "sql:string", .help="Run sql commands without input/output", .retType=(uint)JSI_TT_NUMBER },
    { "exists",         MySqlExistsCmd,     1,  1, "sql:string", .help="Execute sql, and return true if there is at least one result value", .retType=(uint)JSI_TT_BOOLEAN },
    { "info",           MySqlInfoCmd,       0,  0, "", .help="Return info about last query", .retType=(uint)JSI_TT_OBJECT },
    { "lastQuery",      MySqlLastQueryCmd,  0,  0, "", .help="Return info string about most recently executed statement", .retType=(uint)JSI_TT_STRING },
    { "lastRowid",      MySqlLastRowidCmd,  0,  0, "", .help="Return rowid of last insert", .retType=(uint)JSI_TT_NUMBER },
    { "onecolumn",      MySqlOnecolumnCmd,  1,  1, "sql:string", .help="Execute sql, and return a single value", .retType=(uint)JSI_TT_ANY },
    { "ping",           MySqlPingCmd,       0,  1, "noError:boolean=false", .help="Ping connection", .retType=(uint)JSI_TT_NUMBER },
    { "query",          MySqlQueryCmd,      1,  2, "sql:string, options:function|string|array|object=void", .help="Run sql query with input and/or outputs.", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=QueryFmtOptions },
    { "reconnect",      MySqlReconnectCmd,  0,  0, "", .help="Reconnect with current settings", .retType=(uint)JSI_TT_VOID },
    { "reset",          MySqlResetCmd,      0,  0, "", .help="Reset connection", .retType=(uint)JSI_TT_NUMBER },
    { NULL, 0,0,0,0, .help="Commands for accessing mysql databases" }
};

    
static Jsi_RC mysqlObjFree(Jsi_Interp *interp, void *data);
static bool  mysqlObjEqual(void *data1, void *data2);
static bool  mysqlObjIsTrue(void *data);

static Jsi_UserObjReg mysqlobject = {
    .name   = "MySql",
    .spec   = mysqlCmds,
    .freefun= mysqlObjFree,
    .istrue = mysqlObjIsTrue,
    .isequ  = mysqlObjEqual
};

static Jsi_RC MySqlConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    MySqlObj *jdb;
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_LogTest("Creating new MySql");
        
    jdb = (MySqlObj*)Jsi_Calloc(1, sizeof(*jdb));
    SQLSIGINIT(jdb, DB);
    const char *groupname = "mysqljsi";
    jdb->_ = &mydbObjCmd;
    jdb->_->newCnt++;
    jdb->_->activeCnt++;

    jdb->maxStmts = NUM_PREPARED_STMTS;
    jdb->forceInt = 1;
    jdb->interp = interp;
    jdb->hasOpts = (arg != NULL && !Jsi_ValueIsNull(interp,arg));
    if (jdb->hasOpts && Jsi_OptionsProcess(interp, SqlOptions, jdb, arg, 0) < 0) {
        jdb->deleted = 1;
        mysqlObjFree(interp, jdb);
        return JSI_ERROR;
    }
    if (!jdb->udata) {
        jdb->udata = Jsi_ValueNewObj(interp, NULL);
        Jsi_IncrRefCount(interp, jdb->udata);
    }
    jdb->db = mysql_init(NULL);
    jdb->version = (MYSQL_VERSION_MAJOR + ((Jsi_Number)MYSQL_VERSION_MINOR/100.0) + ((Jsi_Number)MYSQL_VERSION_PATCH/10000.0));

#if (MYSQL_VERSION_ID>=32350)
    if (jdb->reconnect)
    {
      my_bool reconnect = 1;
      mysql_options(jdb->db, MYSQL_OPT_RECONNECT, &reconnect);
    }
    mysql_options(jdb->db, MYSQL_READ_DEFAULT_GROUP, groupname);
#endif

#if (MYSQL_VERSION_ID >= 40107)
    if (jdb->sslKey) {
        const char *sslcert = Jsi_ValueString(interp, jdb->sslCert, NULL),
            *sslca = Jsi_ValueString(interp, jdb->sslCA, NULL),
            *sslcapath = Jsi_ValueString(interp, jdb->sslCAPath, NULL),
            *sslcipher = Jsi_ValueString(interp, jdb->sslCipher, NULL),
            *sslkey = Jsi_ValueString(interp, jdb->sslKey, NULL);
        mysql_ssl_set(jdb->db, sslkey, sslcert, sslca, sslcapath, sslcipher);
        jdb->dbflags |= CLIENT_SSL;
    }
#endif

    if (!mdbConnect(interp, jdb)) {
        Jsi_LogError("connect failed %s", mysql_error(jdb->db));
        mysqlObjFree(interp, jdb);
        return JSI_ERROR;
    }

    if (jdb->enableMulti) {
        if (mysql_set_server_option(jdb->db, MYSQL_OPTION_MULTI_STATEMENTS_ON))
            Jsi_LogWarn("multi on failed %s", mysql_error(jdb->db));
    }
    //jdb->event = Jsi_EventNew(interp, mysqlUpdate, jdb); //TODO: events
    Jsi_Value *toacc = NULL;
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        toacc = _this;
    } else {
        Jsi_Obj *o = Jsi_ObjNew(interp);
        Jsi_PrototypeObjSet(interp, "MySql", o);
        Jsi_ValueMakeObject(interp, ret, o);
        toacc = *ret;
    }

    Jsi_Obj *fobj = Jsi_ValueGetObj(interp, toacc);
    if ((jdb->objId = Jsi_UserObjNew(interp, &mysqlobject, fobj, jdb))<0) {
        mysqlObjFree(interp, jdb);
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_ERROR;
    }
    jdb->stmtHash = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    jdb->userObjPtr = fobj;
    jdb->optPtr = &jdb->queryOpts;
    jdb->stmtCache = Jsi_ListNew((Jsi_Interp*)jdb, 0, mdbStmtFreeProc);
    return JSI_OK;
}


static Jsi_RC Jsi_DoneMySql(Jsi_Interp *interp)
{
    if (Jsi_UserObjUnregister(interp, &mysqlobject) != JSI_OK)
        return JSI_ERROR;
    Jsi_PkgProvide(interp, "MySql", -1, NULL);
    return JSI_OK;
}

Jsi_RC Jsi_InitMySql(Jsi_Interp *interp, int release)
{
    if (release) {
        if (!--mydbObjCmd.init)
            mysql_library_end();
        return Jsi_DoneMySql(interp);
    }
    Jsi_Hash* dbSys;
#if JSI_USE_STUBS
  if (Jsi_StubsInit(interp, 0) != JSI_OK)
    return JSI_ERROR;
#endif
#ifndef JSI_OMIT_THREADS
    if (mydbObjCmd.init == 0 && mysql_library_init(0, NULL, NULL))
        return Jsi_LogError("failed to initialize MySQL library\n");
#else
    return Jsi_LogError("Threads required for mysql");
#endif

    Jsi_Value *info = Jsi_ValueNew1(interp);
    Jsi_JSONParseFmt(interp, &info, "{pkgVer:%d}", MYSQL_VERSION_ID);
    Jsi_PkgOpts dbPkgOpts = { mydb_ObjCmd_Specs, &mydbObjCmd, mysqlCmds, info};
    Jsi_RC rc = Jsi_PkgProvideEx(interp, "MySql", 1.1, Jsi_InitMySql, &dbPkgOpts);
    Jsi_DecrRefCount(interp, info);
    if (rc != JSI_OK)
        rc = JSI_ERROR;
    else if (!(dbSys = Jsi_UserObjRegister(interp, &mysqlobject))) 
        rc = Jsi_LogError("Failed to init mysql extension");
    else if (!Jsi_CommandCreateSpecs(interp, mysqlobject.name, mysqlCmds, dbSys, JSI_CMDSPEC_ISOBJ))
        rc = JSI_ERROR;
    if (rc == JSI_OK)
        mydbObjCmd.init++;
    else
        mysql_library_end();
    return rc;
}

#endif
