#include "jsi.h"

Jsi_CmdProcDecl(DemoCmd) {
    int i, n = Jsi_ValueGetLength(interp, args);
    printf("demo called with %d args\n", n);
    for (i=0; i<n; i++) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, i);
        const char *cp = Jsi_ValueToString(interp, v, NULL);
        printf("Arg %d = '%s'\n", i, cp);
    }
    return JSI_OK;
}

Jsi_InitProc Jsi_Initdemo1;

Jsi_RC Jsi_Initdemo1(Jsi_Interp *interp, int release) {
    if (release) return JSI_OK;
    puts("LOADED DEMO");
    Jsi_PkgProvide(interp, "demo1", 1, Jsi_Initdemo1);
    Jsi_CommandCreate(interp, "demo", DemoCmd, NULL);
    return JSI_OK;
}
