#include "jsi.h"

// Create object using a Spec definition for data fields.

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

Jsi_InitProc Jsi_Initdynspec;
static int calls = 0;

bool filterproc(Jsi_AccessorSpec *ap, const char *key, Jsi_Value *val) {
    if (!key)
        printf("Accessor released after %d calls: sets=%d/gets=%d \n", calls, ap->setCnt, ap->getCnt);
    else
       calls++;
    return 0;
}

Jsi_RC Jsi_Initdynspec(Jsi_Interp *interp, int release) {
    static MyData mydata;

    if (release) {
        return JSI_OK;
    }
    Jsi_LogDebug("LOADED DYNSPEC");
    Jsi_PkgProvide(interp, "dynspec", 1, Jsi_Initdynspec);

    Jsi_AccessorSpec *ap = Jsi_ObjAccessorWithSpec(interp, "sobj", MyOptions, (uchar*)&mydata, NULL, 0);
    if (!ap)
        return JSI_ERROR;
    ap->filterProc = filterproc;

    return JSI_OK;
}
