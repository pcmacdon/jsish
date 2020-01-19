#include "jsi.h"
#include "demo.h"

JSI_EXTENSION_INI

int Demo_Incr(int n) {
    puts("INCR");
    return n+1;
}

int Demo_Decr(int n) {
    puts("DECR");
    return n-1;
}

Jsi_CmdProcDecl(DemoCmd) {
    int i, n = Jsi_ValueGetLength(interp, args);
    printf("demo called with %d args\n", n);
    for (i=0; i<n; i++) {
        char *cp = Jsi_ValueArrayIndexToStr(interp, args, i, NULL);
        printf("Arg %d = '%s'\n", i, cp);
    }
    return JSI_OK;
}

int Jsi_Initdemo(Jsi_Interp *interp, int release) {
    if (release) {
        puts("UNLOAD DEMO");
        return JSI_OK;
    }
    puts("LOADED DEMO");
    if (Jsi_StubsInit(interp, JSI_STUBS_STRICT) != JSI_OK)
        return JSI_ERROR;
    Jsi_CommandCreate(interp, "demo", DemoCmd, NULL);
    return JSI_OK;
}
