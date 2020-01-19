// Example build for multiple .jsc files.
#ifdef JSI_CDATA_SHARED
#define JSI_USE_STUBS 1
#include "jsi.h"
#else
#include "jsiOne.c"
#undef JSI_CDATA_MAIN
#endif
#define JSI_CDATA_IMPL
#include "demo0.c"
#include "demo1.c"
#include "demo2.c"

Jsi_InitProc Jsi_InitDemo;

Jsi_RC Jsi_InitDemo(Jsi_Interp *interp, int release)
{
#if JSI_USE_STUBS
    if (Jsi_StubsInit(interp, 0) != JSI_OK)
        return JSI_ERROR;
#endif
    if (release)
        return JSI_OK;
    if (jsi_c_init_demo0(interp) && jsi_c_init_demo1(interp)
        && jsi_c_init_demo2(interp))
        return Jsi_PkgProvide(interp, "demo", 1.0, Jsi_InitDemo);
    return JSI_ERROR;
}

#ifndef JSI_CDATA_SHARED
int main(int argc, char *argv[])
{
    Jsi_InterpOpts opts = {.argc=argc, .argv=argv, .initProc=Jsi_InitDemo};
    Jsi_Interp *interp = Jsi_InterpNew(&opts);
    if (!interp)
        return 1;
    bar.bbit++;
    foo.val1 = 3.2;

    Bar *bs[3];
    int i;
    char key[50];
    for (i=0; i<3; i++) {
        sprintf(key, "Key%d", i);
        bs[i]= (Bar*)Jsi_Calloc(1, sizeof(Bar));
        bs[i]->bfld1 = i;
        Jsi_MapSet(bars, (void*)key, bs[i]);
    }

    Jsi_Main(&opts);
    
    // User code here ...
    
    Jsi_InterpDelete(interp);
    return(0);
}

#endif
