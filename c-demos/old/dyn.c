#include "jsi.h"

Jsi_CmdProcDecl(DynCmd) {
    int i, n = Jsi_ValueGetLength(interp, args);
    printf("Called dyn() with:\n");
    for (i=0; i<n; i++)
        printf("  arguments[%d] = '%s'\n", i, 
            Jsi_ValueArrayIndexToStr(interp, args, i, NULL));
    return JSI_OK;
}

Jsi_InitProc Jsi_Initdyn;

Jsi_RC Jsi_Initdyn(Jsi_Interp *interp, int release) {
    if (release) return JSI_OK;
    printf("LOADED DYN\n" );
    Jsi_PkgProvide(interp, "dyn", 1, Jsi_Initdyn);
    Jsi_CommandCreate(interp, "dyn", DynCmd, NULL);
    return JSI_OK;
}
