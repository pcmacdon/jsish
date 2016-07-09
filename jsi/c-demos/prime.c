#include "jsi.h"

JSI_EXTENSION_INI

static int PrimeCheckCmd(Jsi_Interp *interp, Jsi_Value *args, 
    Jsi_Value *_this, Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int val, lim;
    Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, 0), &val);
    Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, 1), &lim);
    int i;
    Jsi_Bool rc = 1;

    for (i = 2; i <= lim; i++) {
        if ((val % i) == 0) {
            rc = 0;
            break;
        }
    }
    Jsi_ValueMakeBool(interp, ret, rc);
    return JSI_OK;
}

int Jsi_InitPrime(Jsi_Interp *interp) {
  //puts("LOADED PRIME");
#ifdef JSI_USE_STUBS
  if (Jsi_StubsInit(interp, JSI_STUBS_STRICT) != JSI_OK)
      return JSI_ERROR;
#endif

  Jsi_CommandCreate(interp, "primeCheckNative", PrimeCheckCmd, NULL);
  return JSI_OK;
}
