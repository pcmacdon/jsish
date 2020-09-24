#include "jsi.h"

// Example object accessing C-struct data using a Spec definition for fields.

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
static Jsi_OptionSpec MyOptions[] = {
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

Jsi_InitProc Jsi_Initdynspecn;
static int calls = 0;

bool filterproc(Jsi_AccessorSpec *ap, const char *key, Jsi_Value *val) {
    if (!key)
        printf("Accessor for \"%s\" released after %d calls: sets=%d/gets=%d \n", ap->objName, calls, ap->setCnt, ap->getCnt);
    else
       calls++;
    return 0;
}

Jsi_RC Jsi_Initdynspecn(Jsi_Interp *interp, int release) {
    static MyData mydata  = {}, mydata2[4] = {};

    if (release) {
        return JSI_OK;
    }
    Jsi_LogDebug("LOADED DYNSPECN");
    Jsi_PkgProvide(interp, "dynspecn", 1, Jsi_Initdynspecn);

    Jsi_AccessorSpec
        *ap  = Jsi_ObjAccessorWithSpec(interp, "sobj",  MyOptions, (uchar*)&mydata,  NULL, 0);
    if (!ap)
        return JSI_ERROR;

    /* This implicitly cause an eval of "arr[0] = {}", etc  */
    if (JSI_OK != Jsi_NewVariable(interp, "arr", Jsi_ValueNewArray(interp, NULL, 0), 0) ||
        !Jsi_ObjAccessorWithSpec(interp, "arr[0]", MyOptions, (uchar*)&mydata2[0], NULL, 0) ||
        !Jsi_ObjAccessorWithSpec(interp, "arr[1]", MyOptions, (uchar*)&mydata2[1], NULL, 0))
        return JSI_ERROR;


    /* Pre-created vars. */
    if (JSI_OK != Jsi_EvalString(interp, "var darr = [{}]; var dobj = {n:{}};", 0))
        return JSI_ERROR;
    if (!Jsi_ObjAccessorWithSpec(interp, "darr[0]", MyOptions, (uchar*)&mydata2[2], NULL, 0) 
        || !Jsi_ObjAccessorWithSpec(interp, "dobj.n", MyOptions, (uchar*)&mydata2[3], NULL, 0))
        return JSI_ERROR;
    ap->filterProc = filterproc;

    return JSI_OK;
}
