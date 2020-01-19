#include "jsi.h"
#include "demo.h"

JSI_EXTENSION_INI
DEMO_EXTENSION_INI

Jsi_CmdProcDecl(UserCmd) {
    int i, n = Jsi_ValueGetLength(interp, args);
    i = Demo_Incr(1);
    i = Demo_Decr(1);
    printf("demo called with %d args\n", n);
    for (i=0; i<n; i++) {
        char *cp = Jsi_ValueArrayIndexToStr(interp, args, i, NULL);
        printf("Arg %d = '%s'\n", i, cp);
    }
    return JSI_OK;
}

int Jsi_Inituser(Jsi_Interp *interp, int release) {
    if (release) {
        puts("UNLOAD USER");
        return JSI_OK;
    }
    puts("LOADED USER");
    if (Jsi_StubsInit(interp, JSI_STUBS_STRICT) != JSI_OK)
        return JSI_ERROR;
    if (Demo_StubsInit(interp, JSI_STUBS_STRICT) != JSI_OK)
        return JSI_ERROR;
    Jsi_CommandCreate(interp, "user", UserCmd, NULL);
    return JSI_OK;
}
