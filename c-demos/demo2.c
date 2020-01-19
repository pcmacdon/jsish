#include "jsi.h"

// Usually don't use the same handler for both methods foo and bar.
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

Jsi_CmdSpec demoCmds[] = {
  { "foo", DemoCmd, 0, -1, "...", .help="foo command" },
  { "bar", DemoCmd, 1, -1, "...", .help="bar command" },
  { NULL, .help="Demo commands" }
};

Jsi_InitProc Jsi_Initdemo1;

Jsi_RC Jsi_Initdemo2(Jsi_Interp *interp, int release) {
    if (release) return JSI_OK;
    Jsi_CommandCreateSpecs(interp, "demos",   demoCmds,   NULL, 0);
    Jsi_PkgProvide(interp, "demo2", 1, Jsi_Initdemo2);
    return JSI_OK;
}
