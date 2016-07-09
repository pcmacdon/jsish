#include "jsi.h"

Jsi_CmdProcDecl(DynCmd) {
    int i, n = Jsi_ValueGetLength(interp, args);
    printf("Called dyn() with:\n");
    for (i=0; i<n; i++)
        printf("  arguments[%d] = '%s'\n", i, 
            Jsi_ValueArrayIndexToStr(interp, args, i, NULL));
    return JSI_OK;
}

int Jsi_Initdyn(Jsi_Interp *interp) {
    printf("LOADED DYN\n" );
    Jsi_CommandCreate(interp, "dyn", DynCmd, NULL);
    return JSI_OK;
}
