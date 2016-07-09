// JSI DATABASE DEMO: "dbdemo.c".  COMPILE WITH: "gcc dbdemo.c -lm -lsqlite3 -lz -lpthread"
#include <sqlite3.h>
#define HAVE_SQLITE 1
//#define JSI_LITE_ONLY  // ie. TO OMIT SCRIPT SUPPORT.
#include "jsi.c"

static const char *markStrs[] = {"","A","B","C","D","F", NULL};
typedef enum { MARK_NONE, MARK_A, MARK_B, MARK_C, MARK_D, MARK_F } MarkType;

// The structure
typedef struct {
    char name[16];
    Jsi_Number max;
    int id;
    Jsi_Number myTime;
    MarkType mark;
    int markSet;
    Jsi_DString desc;
    Jsi_Wide rowid;
    Jsi_Bool isdirty;
} MyData;

// THE SPECIFICATION
static Jsi_OptionSpec MyOptions[] = {
    JSI_OPT(STRBUF,     MyData, name,   .help="Fixed size char buf", .userData="DEFAULT ''" ),
    JSI_OPT(DSTRING,    MyData, desc,   .help="Description field of arbitrary length"),
    JSI_OPT(INT,        MyData, id,     .help="Int id", .userData="DEFAULT 0 CHECK(id>0)"),
    JSI_OPT(DOUBLE,     MyData, max,    .help="Max value"),
    JSI_OPT(DATETIME,   MyData, myTime, .help="milliseconds since 1970" ),
    JSI_OPT(CUSTOM,     MyData, mark,   .help="Marks", .custom=Jsi_Opt_SwitchEnum,   .data=markStrs ),
    JSI_OPT(CUSTOM,     MyData, markSet,.help="A bit set of marks", .custom=Jsi_Opt_SwitchBitset, .data=markStrs ),
    JSI_OPT(WIDE,       MyData, rowid,  .help="DB rowid for update/insert; not stored in db", .flags=JSI_OPT_DB_ROWID),
    JSI_OPT(BOOL,       MyData, isdirty,.help="Dirty bit flag: not stored in db", .flags=JSI_OPT_DB_DIRTY),
    JSI_OPT_END(        MyData, .help="This is a struct for dbdemo")
};

// WRAPPER FUNCTIONS
static Jsi_Db *jdbPtr;

static int ExecMyData(MyData *data, int num, const char *query, int flags) {
    return Jsi_DbQuery(jdbPtr, MyOptions, data, num, query, flags);
}

static int ExecMySemi(MyData **data, int num, const char *query, int flags) {
    return Jsi_DbQuery(jdbPtr, MyOptions, data, 0, query, JSI_DB_PTRS|flags);
}

static int ExecMyDyn(MyData ***data, int num, const char *query, int flags) {
    return Jsi_DbQuery(jdbPtr, MyOptions, data, 0, query, JSI_DB_PTR_PTRS|flags);
}

// MAIN BODY OF THE DEMO
static int DemoMain(Jsi_Interp *interp, const char *arg)
{
    int i, n, cnt, res = JSI_OK;
    Jsi_Db *jdb;
    
    /* SETUP AND DB CREATION */
    if (!interp)
        jdb = Jsi_DbNew("~/mytest.db", 0);
#ifndef JSI_LITE_ONLY
    if (interp) {
        if (JSI_OK != Jsi_EvalString(interp, "var mydb = new Sqlite('/tmp/mytest.db');", 0))
            return JSI_ERROR;
        jdb = (Jsi_Db *)Jsi_UserObjDataFromVar(interp, "mydb");
        if (!jdb)
            return JSI_ERROR;
    }
#endif
    Jsi_DbQuery(jdb, 0, 0, 0, ";DROP TABLE IF EXISTS mytable;", 0);
    Jsi_DbQuery(jdb, 0, 0, 0, ";CREATE TABLE mytable (max FLOAT, name, id INT, myTime INT, mark, markSet, desc);", 0);

    /* STAGE DATA TO-FROM STRUCT. */
    MyData mydata = {.id=99, .max=100.0, .mark=MARK_A, .markSet=6, .name="maryjane"};
    mydata.myTime = time(NULL)*1000LL; // or use Jsi_DateTime() for milliseconds;
    Jsi_DSSet(&mydata.desc, "Some stuff");

    /* STORE 10 ROWS INTO THE DATABASE */
    Jsi_DbQuery(jdb, 0, 0, 0, ";BEGIN", 0);
    for (i=0; i<10; i++) {
        mydata.id++;
        mydata.max--;
        n = Jsi_DbQuery(jdb, MyOptions, &mydata, 1, "INSERT INTO mytable %s", 0); assert(n==1);
    }
    Jsi_DbQuery(jdb, 0, 0, 0, ";COMMIT", 0);

    /* READ THE LAST INSERTED ROW INTO MYDATA2. */
    MyData mydata2 = {};
    mydata2.rowid = mydata.rowid;
    n = Jsi_DbQuery(jdb, MyOptions, &mydata2, 1, "SELECT id,name FROM mytable WHERE rowid=:rowid", 0); assert(n==1);
    n = Jsi_DbQuery(jdb, MyOptions, &mydata2, 1, "SELECT %s FROM mytable WHERE rowid=:rowid", 0); assert(n==1);

    /* MODIFY ALL FIELDS OF LAST INSERTED ROW. */
    mydata.max = -1;
    mydata.myTime = Jsi_DateTime();
    strcpy(mydata.name, "billybob");
    n = Jsi_DbQuery(jdb, MyOptions, &mydata, 1, "UPDATE mytable SET %s WHERE id=:id", 0); assert(n==1);

    /* MODIFY SPECIFIC COLUMNS FOR HALF OF INSERTED ROWS. */
    mydata.id = 105;
    n = Jsi_DbQuery(jdb, MyOptions, &mydata, 1, "UPDATE mytable SET name=:name, max=:max WHERE id<:id", 0); assert(n==5);

    /* DELETE ITEM ID 105. */
    n = Jsi_DbQuery(jdb, MyOptions, &mydata, 1, "DELETE FROM mytable WHERE id=:id", 0); assert(n==1);

    /* ARRAY OF STRUCTS. */
    int num = 10;
    MyData mydatas[10] = {};
    cnt = Jsi_DbQuery(jdb, MyOptions, mydatas, num, "SELECT %s FROM mytable", 0); assert(cnt==9);
    
    for (i=0; i<cnt; i++)
        mydatas[i].id += i;
    n = Jsi_DbQuery(jdb, MyOptions, mydatas, cnt, "UPDATE mytable SET %s WHERE rowid = :rowid", 0); assert(n==9);
    
    /* Update only the dirty rows. */
    for (i=1; i<=3; i++) {
        mydatas[i].isdirty = 1;
        mydatas[i].id += 100*i;
    }
    n = Jsi_DbQuery(jdb, MyOptions, mydatas, cnt, "UPDATE mytable SET %s WHERE rowid = :rowid", JSI_DB_DIRTY_ONLY); assert(n==3);
 
    /* ARRAY OF POINTERS TO STRUCTS. */
    static MyData *mdPtr[3] = {};  /* FIXED LENGTH */
    n = Jsi_DbQuery(jdb, MyOptions, mdPtr, 3, "SELECT %s FROM mytable", JSI_DB_PTRS); assert(n==3);
    printf("%f\n", mdPtr[0]->max);

    MyData **dynPtr = NULL;  /* VARIABLE LENGTH */
    n = Jsi_DbQuery(jdb, MyOptions, &dynPtr, 0, "SELECT %s FROM mytable WHERE rowid < 5", JSI_DB_PTR_PTRS); assert(n==4);
    n = Jsi_DbQuery(jdb, MyOptions, &dynPtr, n, "SELECT %s FROM mytable LIMIT 1000", JSI_DB_PTR_PTRS);  assert(n==9);
    n = Jsi_DbQuery(jdb, MyOptions, &dynPtr, n, NULL, JSI_DB_PTR_PTRS|JSI_DB_MEMFREE);
    assert(!dynPtr);
    
    /* MULTI-BIND INTERFACE, TO BIND VARS FROM OTHER STRUCT(S). */
    Jsi_DbMultipleBind binds[] = {
        { ':', MyOptions, mdPtr, 3 },
        { '$', MyOptions, &mydata },
        {}
    };
    mydata.max = -1;
    n = Jsi_DbQuery(jdb, NULL, binds, 0, "SELECT %s FROM mytable WHERE max=$max", JSI_DB_PTRS); assert(n==3);

    if (!strcmp(arg,"-wrapper")) {
        // Call wrapper functions.
        ExecMyData(mydatas, n, "SELECT %s FROM mytable;", 0);
        ExecMySemi(mdPtr,   n, "SELECT %s FROM mytable;", 0);
        ExecMyDyn(&dynPtr,  n, "SELECT %s FROM mytable;", 0);
    }

    /* MAKE "mdPtr" AVAILABLE AS "mydata" TO JAVASCRIPT "Cdata.names()" */
    if (interp)
        Jsi_CDataRegister(interp, "mydata", MyOptions, mdPtr, num, JSI_DB_PTRS);

    jdbPtr = jdb;
    
    /* LOAD TEST INSERT/SELECT/UPDATE 1,000,000 ROWS. */
    if (!strcmp(arg,"-benchmark")) {
        Jsi_Number stim, etim;
        int bnum = 1000000;
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
        printf("INIT C: %8.3f secs\n", ((etim-stim)/1000.0));
        Jsi_DbQuery(jdb, 0, 0, 0, ";DELETE FROM mytable;", 0);
        stim=etim;
        n = ExecMyData(big, bnum, "INSERT INTO mytable %s", 0); assert(n==bnum);
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    INSERT %d ROWS\n", i/1000.0, bnum*1000/i, n);

        stim=etim;
        memset(big, 0, num*sizeof(MyData));
        n = ExecMyData(big, bnum, "SELECT %s FROM mytable", 0); assert(n==bnum);
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    SELECT %d ROWS \n", i/1000.0, bnum*1000/i, bnum);
        for (i=0; i<bnum; i++) {
            if (b[i].id != i)
                printf("FAILED: Data[%d].id: %d\n", i, b[i].id);
            b[i].id = i+1;
        }

        stim=etim;
        n = ExecMyData(big, bnum, "UPDATE mytable SET id=:id where rowid=:rowid", 0); assert(n==bnum);
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
        n = ExecMyData(big, bnum, "UPDATE mytable SET %s where rowid=:rowid", 0); assert(n==bnum);
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    UPDATE %d ROWS, ALL FIELDS\n", i/1000.0, n*1000/i, n);

        stim=etim;
        n = ExecMyData(big, bnum, "UPDATE mytable SET %s where rowid=:rowid", JSI_DB_DIRTY_ONLY); assert(n==(bnum/10));
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    UPDATE %d DIRTY ROWS\n", i/1000.0, (int)(n*1000.0/i), n);

        stim=etim;
        for (i=0; i<bnum; i++) {
            if ((i%1000)==0)
                b[i].isdirty = 1;
        }
        n = ExecMyData(big, bnum, "UPDATE mytable SET %s where rowid=:rowid", JSI_DB_DIRTY_ONLY); assert(n==(bnum/1000));
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    UPDATE %d DIRTY ROWS\n", i/1000.0, n*1000/i, n);

        stim=etim;
        for (i=0; i<bnum; i++) {
            if ((i%100000)==0)
                b[i].isdirty = 1;
        }
        n = ExecMyData(big, bnum, "UPDATE mytable SET %s where rowid=:rowid", JSI_DB_DIRTY_ONLY); assert(n==(bnum/100000));
        etim = Jsi_DateTime();
        i=(int)(etim-stim);
        printf("%8.3f sec, %8d rows/sec    UPDATE %d DIRTY ROWS\n", i/1000.0, n*1000/i, n);

        Jsi_Free(big);
    }
    // DEMONSTRATES THE NEED FOR WRAPPERS TO DETECT COMPILE ERRORS!
    if (0) Jsi_DbQuery(jdb, MyOptions, "a bad struct", n, "", 0); 

    return res;
}

int main(int argc, char *argv[]) {
    Jsi_Interp *interp = NULL;
#ifdef JSI_LITE_ONLY
    interp = Jsi_InterpNew(NULL, argc, argv, 0);
#endif
    DemoMain(interp, argc>1?argv[1]:"");
    if (interp)
        Jsi_Interactive(interp, JSI_OUTPUT_QUOTE|JSI_OUTPUT_NEWLINES);
    return 0;
}
