#include <string.h>
#include <assert.h>
#include "jsi.h"
#include "demo.h"
#include "jsiStubs.h"
#include "demoStubs.h"


static int Demo_Stubs__initialize(Jsi_Interp *interp, double version, const char* name, int flags, 
    const char *md5, int bldFlags, int stubSize, int structSizes, void **ptr);
    
Demo_Stubs demoStubsTbl = { __DEMO_STUBS_INIT__ };
Demo_Stubs *demoStubsTblPtr = &demoStubsTbl;

static int Demo_Stubs__initialize(Jsi_Interp *interp, double version, const char* name, int flags, 
    const char *md5, int bldFlags, int stubSize, int structSizes, void **ptr)
{
    if (strcmp(name,"demo")) { /* Sub-stub support */
        int rc = Jsi_StubLookup(interp, name, ptr);
        if (rc != JSI_OK)
            return JSI_ERROR;
        Jsi_Stubs *sp = *ptr;
        if (sp->_Jsi_Stubs__initialize && sp->sig == JSI_STUBS_SIG &&
            sp->_Jsi_Stubs__initialize != demoStubsTbl._Demo_Stubs__initialize)
            return (*sp->_Jsi_Stubs__initialize)(interp, version, name, flags, md5, bldFlags, stubSize, structSizes, ptr);
        Jsi_LogError("failed to find stub for %s", name);
        return JSI_ERROR;
    }
    int structSize = DEMO_STUBS_STRUCTSIZES;
    int strict = (flags & JSI_STUBS_STRICT);
    int sizediff = (sizeof(Demo_Stubs) - stubSize);
    assert(demoStubsTbl.sig == JSI_STUBS_SIG);
    if (sizediff<0 || (strict && (strcmp(md5, DEMO_STUBS_MD5) || sizediff || structSize != structSizes))) {
        fprintf(stderr, "%s: extension from incompatible build: %s\n", name, (sizediff ? "size changed": md5));
        return JSI_ERROR;
    }
    if (bldFlags != demoStubsTbl.bldFlags) {
        fprintf(stderr, "%s: extension build flags mismatch (different libc?)\n", name);
        if (strict)
            return JSI_ERROR;
    }
    if (version != DEMO_VERSION && strict) {
        fprintf(stderr, "%s: extension version newer than user (%g > %g)\n", name, version, DEMO_VERSION);
        return JSI_ERROR;
    }
    return JSI_OK;
}
