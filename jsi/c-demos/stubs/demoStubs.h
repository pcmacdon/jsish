#ifndef __DEMO_STUBS_H__
#define __DEMO_STUBS_H__
#include "demo.h"

#define DEMO_STUBS_MD5 "b0e75d66d020ea195aa6893aaaf41c6f"

#undef DEMO_EXTENSION_INI
#define DEMO_EXTENSION_INI Demo_Stubs *demoStubsPtr = NULL;

#ifdef HAVE_MUSL
#define DEMO_STUBS_BLDFLAGS 1
#else
#define DEMO_STUBS_BLDFLAGS 0
#endif
#ifndef Demo_StubsInit
#define Demo_StubsInit(interp,flags) (jsiStubsPtr && jsiStubsPtr->sig == \
  JSI_STUBS_SIG?jsiStubsPtr->_Jsi_Stubs__initialize(interp, flags, "demo", \
  DEMO_VERSION, DEMO_STUBS_MD5, DEMO_STUBS_BLDFLAGS, sizeof(Demo_Stubs), DEMO_STUBS_STRUCTSIZES, (void**)&demoStubsPtr):JSI_ERROR)
#endif

typedef struct Demo_Stubs {
    uint sig;
    const char* name;
    int size;
    int bldFlags;
    const char* md5;
    void *hook;
    int(*_Demo_Stubs__initialize)(Jsi_Interp *interp, double version, const char* name, int flags, const char *md5, int bldFlags, int stubSize, int structSizes, void **ptr);
    int(*_Demo_Incr)(int n);
    int(*_Demo_Decr)(int n);
    void *endPtr;
} Demo_Stubs;

extern Demo_Stubs* demoStubsPtr;

#define __DEMO_STUBS_INIT__\
    JSI_STUBS_SIG,    "demo",    sizeof(Demo_Stubs),     DEMO_STUBS_BLDFLAGS,    DEMO_STUBS_MD5,    NULL,\
    Demo_Stubs__initialize,\
    Demo_Incr,\
    Demo_Decr,\
    NULL

#ifdef JSI_USE_STUBS

#define Demo_Incr(n0) JSISTUBCALL(demoStubsPtr, _Demo_Incr(n0))
#define Demo_Decr(n0) JSISTUBCALL(demoStubsPtr, _Demo_Decr(n0))

#endif

#endif
