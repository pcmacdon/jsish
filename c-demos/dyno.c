#include "jsi.h"

// Create object where data is handled by C-callback

Jsi_CmdProcDecl(DynObjCmd) {
    static struct { double foo, bar; } data = {0, 0};
    
    int n = Jsi_ValueGetLength(interp, args);    
    if (n != 1 && n != 2)
        return Jsi_LogError("expected 1 or 2 args");

    const char *name = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    double *d = (!Jsi_Strcmp(name,"foo")? &data.foo : &data.bar);

    if (n==1)
        Jsi_ValueMakeNumber(interp, ret, *d);
    else {
        if (Jsi_GetNumberFromValue(interp, Jsi_ValueArrayIndex(interp, args, 1), d) != JSI_OK)
            return Jsi_LogError("expected number arg");
    }
    printf("[access=%s]  %s => %g\n", (n==1?"get":"set"), name, *d);
    return JSI_OK;
}

Jsi_InitProc Jsi_Initdyno;

Jsi_RC Jsi_Initdyno(Jsi_Interp *interp, int release) {
    static Jsi_Value *ocmd = NULL, *dynObj = NULL;

    if (release) {
        if (ocmd) {
            Jsi_DecrRefCount(interp, ocmd);
            Jsi_DecrRefCount(interp, dynObj);
        }
        ocmd = dynObj = NULL;
        return JSI_OK;
    }
    printf("LOADED DYNO\n" );
    Jsi_PkgProvide(interp, "dyno", 1, Jsi_Initdyno);

    Jsi_EvalString(interp, "var dynObj = {}; Object.freeze(dynObj);", 0);

    ocmd = Jsi_ValueNewFunction(interp, DynObjCmd, NULL, NULL);
    dynObj = Jsi_NameLookup(interp, "dynObj");
    Jsi_IncrRefCount(interp, ocmd);
    Jsi_IncrRefCount(interp, dynObj);

    Jsi_Obj *obj = Jsi_ValueGetObj(interp, dynObj);
    Jsi_ObjAccessor(interp, obj, 1, "foo", ocmd);
    Jsi_ObjAccessor(interp, obj, 0, "foo", ocmd);
    Jsi_ObjAccessor(interp, obj, 1, "bar", ocmd);
    Jsi_ObjAccessor(interp, obj, 0, "bar", ocmd);
    return JSI_OK;
}
