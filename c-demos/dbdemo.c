// JSI DATABASE DEMO: "dbdemo.c".  

//#define JSI_LITE_ONLY  // ie. TO OMIT SCRIPT SUPPORT.
#include <assert.h>
#include <string.h>
#include "jsi.h"

static const char *markStrs[] = {"","A","B","C","D","F", NULL};
typedef enum { MARK_NONE, MARK_A, MARK_B, MARK_C, MARK_D, MARK_F } MarkType;

// The structure
typedef struct {
    char name[16];
    Jsi_Number max;
    int id;
    MarkType mark;
    int markSet;
    Jsi_Number myTime;
    Jsi_DString desc;
    Jsi_Wide rowid;
    bool isdirty;
} MyData;

// THE SPECIFICATION
static Jsi_StructSpec MyOptions[] = {
    JSI_OPT(STRBUF,     MyData, name,   .help="Fixed size char buf" ),
    JSI_OPT(DSTRING,    MyData, desc,   .help="Description field of arbitrary length"),
    JSI_OPT(INT,        MyData, id,     .help="Int id" ),
    JSI_OPT(DOUBLE,     MyData, max,    .help="Max value"),
    JSI_OPT(TIME_D,     MyData, myTime, .help="milliseconds since 1970" ),
    JSI_OPT(CUSTOM,     MyData, mark,   .help="Marks", .flags=0, .custom=Jsi_Opt_SwitchEnum,   .data=markStrs ),
    JSI_OPT(CUSTOM,     MyData, markSet,.help="A bit set of marks", .flags=0, .custom=Jsi_Opt_SwitchBitset, .data=markStrs ),
    JSI_OPT(INT64,      MyData, rowid,  .help="DB rowid for update/insert; not stored in db", .flags=JSI_OPT_DB_ROWID),
    JSI_OPT(BOOL,       MyData, isdirty,.help="Dirty bit flag: not stored in db", .flags=JSI_OPT_DB_DIRTY),
    JSI_OPT_END(        MyData)
};

#if 0
// WRAPPER FUNCTIONS
static Jsi_Db *jdbPtr;

static int ExecMyData(MyData *data, int num, const char *query, Jsi_DbOpts *opts) {
    return Jsi_DbQuery(jdbPtr, MyOptions, data, num, query, opts);
}

static int ExecMySemi(MyData **data, int num, const char *query, Jsi_DbOpts *opts) {
    Jsi_DbOpts o = {};
    if (opts)
        o = *opts;
    o.ptrs = 1;
    return Jsi_DbQuery(jdbPtr, MyOptions, data, 0, query, &o);
}

static int ExecMyDyn(MyData ***data, int num, const char *query, Jsi_DbOpts *opts) {
    Jsi_DbOpts o = {};
    if (opts)
        o = *opts;
    o.ptr_to_ptrs = 1;
    return Jsi_DbQuery(jdbPtr, MyOptions, data, 0, query, &o);
}
#endif
// MAIN BODY OF THE DEMO
static Jsi_RC DemoMain(Jsi_Interp *interp, const char *arg)
{
    int i, n, cnt;
    Jsi_RC res = JSI_OK;
    Jsi_Db *jdb;
    
    /* SETUP AND DB CREATION */
    if (!interp)
        jdb = Jsi_DbNew("~/mytest.db", 0);
#ifndef JSI_LITE_ONLY
    if (interp) {
        if (JSI_OK != Jsi_EvalString(interp, "var mydb = new Sqlite(null);", 0))
        //if (JSI_OK != Jsi_EvalString(interp, "var mydb = new Sqlite('/tmp/mytest.db');", 0))
            return JSI_ERROR;
        jdb = (Jsi_Db *)Jsi_UserObjDataFromVar(interp, "mydb");
        if (!jdb)
            return JSI_ERROR;
    }
#endif
    Jsi_DbQuery(jdb, NULL, ";DROP TABLE IF EXISTS mytable;");
    Jsi_DbQuery(jdb, NULL, ";CREATE TABLE mytable (max FLOAT, name, id INT, myTime INT, mark, markSet, desc);");

    /* STAGE DATA TO-FROM STRUCT. */
    MyData mydata = {"maryjane", .max=100.0, .id=99, .mark=MARK_A, .markSet=6};
    mydata.myTime = time(NULL)*1000LL; // or use Jsi_DateTime() for milliseconds;
    Jsi_DSSet(&mydata.desc, "Some stuff");

    /* STORE 10 ROWS INTO THE DATABASE */
    Jsi_DbQuery(jdb, NULL, ";BEGIN");
    Jsi_CDataDb d[] = {{MyOptions, &mydata }, {}};
    for (i=0; i<10; i++) {
        mydata.id++;
        mydata.max--;
        n = Jsi_DbQuery(jdb, d, "INSERT INTO mytable %v" ); assert(n==1);
    }
    Jsi_DbQuery(jdb, NULL, ";COMMIT");

    /* READ THE LAST INSERTED ROW INTO MYDATA2. */
    MyData mydata2 = {};
    mydata2.rowid = mydata.rowid;
    Jsi_CDataDb d2[] ={ {MyOptions, &mydata2 }, {}};
    n = Jsi_DbQuery(jdb, d2, "SELECT id,name FROM mytable WHERE rowid=:rowid"); assert(n==1);
    n = Jsi_DbQuery(jdb, d2, "SELECT %v FROM mytable WHERE rowid=:rowid"); assert(n==1);

    /* MODIFY ALL FIELDS OF LAST INSERTED ROW. */
    mydata.max = -1;
    mydata.myTime = Jsi_DateTime();
    strcpy(mydata.name, "billybob");
    n = Jsi_DbQuery(jdb, d, "UPDATE mytable SET %v WHERE id=:id"); assert(n==1);
    /* MODIFY SPECIFIC COLUMNS FOR HALF OF INSERTED ROWS. */
    mydata.id = 105;
    n = Jsi_DbQuery(jdb, d, "UPDATE mytable SET name=:name, max=:max WHERE id<:id"); assert(n==5);

    /* DELETE ITEM ID 105. */
    n = Jsi_DbQuery(jdb, d, "DELETE FROM mytable WHERE id=:id"); assert(n==1);

    /* ARRAY OF STRUCTS. */
    int num = 10;
    static MyData mydatas[10] = {};
    Jsi_CDataDb d3[] ={ {MyOptions, mydatas, num }, {}};
    cnt = Jsi_DbQuery(jdb, d3, "SELECT %v FROM mytable"); assert(cnt==9);
    
    for (i=0; i<cnt; i++)
        mydatas[i].id += i;
    d3->arrSize = cnt;
    n = Jsi_DbQuery(jdb, d3, "UPDATE mytable SET %v WHERE rowid = :rowid"); assert(n==9);
    
    /* Update only the dirty rows. */
    for (i=1; i<=3; i++) {
        mydatas[i].isdirty = 1;
        mydatas[i].id += 100*i;
    }
    d3->dirtyOnly = 1;
    n = Jsi_DbQuery(jdb, d3, "UPDATE mytable SET %v WHERE rowid = :rowid"); assert(n==3);
 
#ifndef JSI_LITE_ONLY
   /* if (interp) {
        Jsi_CarrayRegister(interp, "mydata", MyOptions, mydatas, num, &opts);
        Jsi_EvalString(interp, "mydb.query('select * from mytable', {cdata:'mydata'})", 0);
    }*/
#endif

    /* ARRAY OF POINTERS TO STRUCTS. */
    static MyData *mdPtr[3] = {};  /* FIXED LENGTH */
    Jsi_CDataDb d4[] ={ {MyOptions, mdPtr, 3 }, {}};
    d4->isPtrs = 1;
    n = Jsi_DbQuery(jdb, d4, "SELECT %v FROM mytable"); assert(n==3);
    printf("%f\n", mdPtr[0]->max);

    MyData **dynPtr = NULL;  /* VARIABLE LENGTH */
    Jsi_CDataDb d5[] ={ {MyOptions, &dynPtr }, {}};
    d5->isPtr2 = 1;
    d5->arrSize = n = Jsi_DbQuery(jdb, d5, "SELECT %v FROM mytable WHERE rowid < 5"); assert(n==4);
    d5->arrSize = n = Jsi_DbQuery(jdb, d5, "SELECT %v FROM mytable LIMIT 1000");  assert(n==9);
    d5->memFree = 1;
    n = Jsi_DbQuery(jdb, d5, NULL);
    assert(!dynPtr);
    
    /* MULTI-BIND INTERFACE, TO BIND VARS FROM OTHER STRUCT(S). */
    Jsi_CDataDb binds[] = {
        { MyOptions, mdPtr, 3, ':' },
        { MyOptions, &mydata, 0, '$' },
        {}
    };
    mydata.max = -1;
    binds->isPtrs = 1;
    n = Jsi_DbQuery(jdb, binds, "SELECT %v FROM mytable WHERE max=$max"); assert(n==3);
#if 0
    if (!strcmp(arg,"-wrapper")) {
        ExecMyData(mydatas, n, "SELECT %v FROM mytable;");
        ExecMySemi(mdPtr,   n, "SELECT %v FROM mytable;");
        ExecMyDyn(&dynPtr,  n, "SELECT %v FROM mytable;");
    }

    /* MAKE "mdPtr" AVAILABLE AS "mydata" TO JAVASCRIPT "CData.names()" */
    jdbPtr = jdb;
#endif    
    /* LOAD TEST INSERT/SELECT/UPDATE 1,000,000 ROWS. */
    if (!strcmp(arg,"-benchmark")) {
        Jsi_Number stim, etim;
        int bnum = 100000;
        MyData *big = (MyData *)Jsi_Calloc(bnum, sizeof(MyData)), *b = big;
        *b = mydata;
        printf("BENCHMARK %d ROWS\n", bnum);

        stim = Jsi_DateTime();
        b[0].id = 0;
        for (i=1; i<bnum; i++) {
            big[i] = *b;
            big[i].id = i;
        }
        etim = Jsi_DateTime();
        Jsi_CDataDb d6[] ={ {MyOptions, big, bnum }, {}};
        printf("INIT C: %8.3f secs\n", ((etim-stim)/1000.0));
        Jsi_DbQuery(jdb, NULL, ";DELETE FROM mytable;");
        stim=etim;
        n = Jsi_DbQuery(jdb, d6, "INSERT INTO mytable %v"); assert(n==bnum);
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    INSERT %d ROWS\n", i/1000.0, bnum*1000/i, n);

        stim=etim;
        memset(big, 0, num*sizeof(MyData));
        n = Jsi_DbQuery(jdb, d6, "SELECT %v FROM mytable"); assert(n==bnum);
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    SELECT %d ROWS \n", i/1000.0, bnum*1000/i, bnum);
        for (i=0; i<bnum; i++) {
            if (b[i].id != i)
                printf("FAILED: Data[%d].id: %d\n", i, b[i].id);
            b[i].id = i+1;
        }

        stim=etim;
        n = Jsi_DbQuery(jdb, d6, "UPDATE mytable SET id=:id where rowid=:rowid"); assert(n==bnum);
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    UPDATE %d ROWS, 1 FIELD\n", i/1000.0, bnum*1000/i, n);
        for (i=0; i<bnum; i++) {
            if (b[i].id != (i+1))
                printf("FAILED: Data[%d].id: %d\n", i, b[i].id);
            b[i].id = i+2;
            if ((i%10)==0)
                b[i].isdirty = 1;
        }

        stim=etim;
        n = Jsi_DbQuery(jdb, d6, "UPDATE mytable SET %v where rowid=:rowid"); assert(n==bnum);
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    UPDATE %d ROWS, ALL FIELDS\n", i/1000.0, n*1000/i, n);

        stim=etim;
        d6->dirtyOnly = 1;
        n = Jsi_DbQuery(jdb, d6, "UPDATE mytable SET %v where rowid=:rowid"); assert(n==(bnum/10));
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    UPDATE %d DIRTY ROWS\n", i/1000.0, (int)(n*1000.0/i), n);

        stim=etim;
        for (i=0; i<bnum; i++) {
            if ((i%1000)==0)
                b[i].isdirty = 1;
        }
        n = Jsi_DbQuery(jdb, d6, "UPDATE mytable SET %v where rowid=:rowid"); assert(n==(bnum/1000));
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    UPDATE %d DIRTY ROWS\n", i/1000.0, n*1000/i, n);

        stim=etim;
        for (i=0; i<bnum; i++) {
            if ((i%100000)==0)
                b[i].isdirty = 1;
        }
        n = Jsi_DbQuery(jdb, d6, "UPDATE mytable SET %v where rowid=:rowid"); assert(n==(bnum/100000));
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    UPDATE %d DIRTY ROWS\n", i/1000.0, n*1000/i, n);

        Jsi_Free(big);
    }

    return res;
}

int main(int argc, char *argv[]) {
    Jsi_Interp *interp = NULL;
#ifndef JSI_LITE_ONLY
    interp = Jsi_InterpMain(argc, argv, NULL);
#endif
    DemoMain(interp, argc>1?argv[1]:"");
#ifndef JSI_LITE_ONLY
    if (interp)
        Jsi_Interactive(interp, JSI_OUTPUT_QUOTE|JSI_OUTPUT_NEWLINES);
#endif
    return 0;
}
