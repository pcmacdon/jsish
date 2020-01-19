#include "jsi.h"

JSI_EXTENSION_INI

static Jsi_RC PrimeCheckCmd(Jsi_Interp *interp, Jsi_Value *args, 
    Jsi_Value *_this, Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int val, lim, i;
    bool rc = 1;
    if (Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, 0), &val) != JSI_OK ||
        Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, 1), &lim) != JSI_OK)
        return JSI_ERROR;

    for (i = 2; i <= lim; i++) {
        if ((val % i) == 0) {
            rc = 0;
            break;
        }
    }
    Jsi_ValueMakeBool(interp, ret, rc);
    return JSI_OK;
}

Jsi_InitProc Jsi_InitPrime;

Jsi_RC Jsi_InitPrime(Jsi_Interp *interp, int release) {
  //puts("LOADED PRIME");
  if (release) return JSI_OK;
#ifdef JSI_USE_STUBS
  if (Jsi_StubsInit(interp, JSI_STUBS_STRICT) != JSI_OK)
      return JSI_ERROR;
#endif

  Jsi_PkgProvide(interp, "prime", 1, Jsi_InitPrime);
  Jsi_CommandCreate(interp, "primeCheckNative", PrimeCheckCmd, NULL);
  return JSI_OK;
}
