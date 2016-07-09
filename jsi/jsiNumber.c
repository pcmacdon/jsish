#include <math.h>
#include <float.h>
#include <stdio.h>
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

int jsi_ieee_isnormal(Jsi_Number a) { return isnormal(a); }
int jsi_ieee_isnan(Jsi_Number a) { return isnan(a); }
int jsi_ieee_infinity(Jsi_Number a) {
#ifndef HAVE_MUSL
    return isinf(a);
#else
    if (!isinf(a))
        return 0;
    return (a<0 ? -1 : 1);
#endif
}
int jsi_is_integer(Jsi_Number n) { return (isnormal(n) ? (Jsi_Number)((int)(n)) == (n) : n==0.0); }
int jsi_is_wide(Jsi_Number n) { return (isnormal(n) && (Jsi_Number)((Jsi_Wide)(n)) == (n)); }

Jsi_Number jsi_ieee_makeinf(int i)
{
    Jsi_Number r = INFINITY;
    if (i < 0) r = -r;
    return r;
}

Jsi_Number jsi_ieee_makenan(void)
{
    return NAN;
}

void jsi_num_itoa10(int value, char* str)
{
    sprintf(str, "%d", value);
    return;
}

void jsi_num_uitoa10(unsigned int value, char* str)
{
    sprintf(str, "%u", value);
}

int jsi_num_isNaN(Jsi_Number value)
{
    /* Hacky test for NaN */
    return ((value != value));
}

int jsi_num_isFinite(Jsi_Number value)
{
    Jsi_Number r = INFINITY;
    return (jsi_num_isNaN(value)==0 && value != r && r != -value);
}

void jsi_num_dtoa2(Jsi_Number value, char* str, int prec)
{
    if (jsi_num_isNaN(value)) {
        Jsi_Strcpy(str,"NaN");
        return;
    }
    sprintf(str, "%.*" JSI_NUMGFMT, prec, value);
}
int jsi_num_isEqual(Jsi_Number n1, Jsi_Number n2)
{
    return (n1 == n2); // TODO: Do we need more?
   /* int n1n = jsi_num_isNan(n1); 
    int n2n = jsi_num_isNan(n2);
    if (n1n && n2n)
        return 1;
    if (n1n || n2n)
        return 0;
    return ((n2-n1)==0);*/
}

#ifndef JSI_LITE_ONLY
static int NumberConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        Jsi_Number nv = 0.0;
        if (Jsi_ValueGetLength(interp, args) > 0) {
            Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
            if (v) {
                Jsi_ValueToNumber(interp, v);
                nv = v->d.num;
            }
        }
        _this->d.obj->ot = JSI_OT_NUMBER;
        _this->d.obj->d.num = nv;
        return JSI_OK;
    }
    if (Jsi_ValueGetLength(interp, args) > 0) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
        if (v) {
            Jsi_ValueDup2(interp, ret, v);
            Jsi_ValueToNumber(interp, *ret);
            return JSI_OK;
        }
    }
    Jsi_ValueMakeNumber(interp, ret, 0.0);
    return JSI_OK;
}

   
#define ChkStringN(_this, funcPtr, dest) \
    if (_this->vt == JSI_VT_OBJECT && _this->d.obj->ot == JSI_OT_FUNCTION &&  \
       _this->d.obj->__proto__ == interp->Number_prototype->d.obj->__proto__ ) { \
        skip = 1; \
        dest = Jsi_ValueArrayIndex(interp, args, 0); \
    } else if (_this->vt != JSI_VT_OBJECT || _this->d.obj->ot != JSI_OT_NUMBER) { \
        Jsi_LogError("apply Number.%s to a non-number object\n", funcPtr->cmdSpec->name); \
        return JSI_ERROR; \
    } else  { \
        dest = _this; \
    }
    
static int NumberToFixedCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[100];
    int prec, skip = 0;
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    if (Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, skip), &prec) != JSI_OK)
        return JSI_ERROR;
    if (prec<0) prec = 0;
    Jsi_GetDoubleFromValue(interp, v, &num);
    sprintf(buf,"%.*" JSI_NUMFFMT, prec, num);
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static int NumberToPrecisionCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[100];
    int prec, skip = 0;
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    if (Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, skip), &prec) != JSI_OK)
        return JSI_ERROR;
    if (prec<0) prec = 0;
    Jsi_GetDoubleFromValue(interp, v, &num);
    sprintf(buf,"%.*" JSI_NUMFFMT, prec, num);
    if (num<0)
        prec++;
    buf[prec+1] = 0;
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static int NumberToExponentialCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[100];
    int prec, len, skip = 0;
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    if (Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, skip), &prec) != JSI_OK)
        return JSI_ERROR;
    if (prec<0) prec = 0;
    Jsi_GetDoubleFromValue(interp, v, &num);
    sprintf(buf,"%.*" JSI_NUMEFMT, prec, num);
    len = strlen(buf);
    if (len >= 4 && buf[len-2] == '0' && buf[len-3] == '+') {
        buf[len-2] = buf[len-1];
        buf[len-1] = 0;
    }
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static int NumberToStringCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[500];
    int radix = 10, skip = 0, argc = Jsi_ValueGetLength(interp, args);
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    Jsi_GetDoubleFromValue(interp, v, &num);
    if (argc>skip && (Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, skip), &radix) != JSI_OK
        || radix<2))
        return JSI_ERROR;
    if (argc==skip)
        return jsi_ObjectToStringCmd(interp, args, _this, ret, funcPtr);
    switch (radix) {
        case 16: sprintf(buf, "%llx", (Jsi_Wide)num); break;
        case 8: sprintf(buf, "%llo", (Jsi_Wide)num); break;
        case 10: sprintf(buf, "%lld", (Jsi_Wide)num); break;
        default: return jsi_ObjectToStringCmd(interp, args, _this, ret, funcPtr);
    }
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static Jsi_CmdSpec numberCmds[] = {
    { "Number",         NumberConstructor,      0, 1, "num:number=0", JSI_CMD_IS_CONSTRUCTOR, .help="Number constructor", .retType=(uint)JSI_TT_NUMBER },
    { "toFixed",        NumberToFixedCmd,       1, 1, "num:number", .help="Formats a number with x numbers of digits after the decimal point", .retType=(uint)JSI_TT_STRING },
    { "toExponential",  NumberToExponentialCmd, 1, 1, "num:number", .help="Converts a number into an exponential notation", .retType=(uint)JSI_TT_STRING },
    { "toPrecision",    NumberToPrecisionCmd,   1, 1, "num:number", .help="Formats a number to x length", .retType=(uint)JSI_TT_STRING },
    { "toString",       NumberToStringCmd,      0, 1, "radix:number=10", .help="Convert to string", .retType=(uint)JSI_TT_STRING }, 
    { NULL,.help="Commands for accessing number objects" }
};

int jsi_NumberInit(Jsi_Interp *interp)
{
    Jsi_Value *val, *global = interp->csc;
    val = interp->Number_prototype = Jsi_CommandCreateSpecs(interp, "Number", numberCmds, NULL, 0);

    Jsi_Value *NaN = Jsi_ValueMakeNumber(interp, NULL, jsi_ieee_makenan());

    Jsi_Value *Inf = Jsi_ValueMakeNumber(interp, NULL, jsi_ieee_makeinf(1));
    
    Jsi_ValueInsertFixed(interp, global, "NaN", NaN);
    Jsi_ValueInsertFixed(interp, global, "Infinity", Inf);
    interp->NaNValue = NaN;
    interp->InfValue = Inf;
#define MCONST(name,v) Jsi_ValueInsert(interp, val, name, Jsi_ValueNewNumber(interp, v), JSI_OM_READONLY)
    MCONST("MAX_VALUE", DBL_MAX);
    MCONST("MIN_VALUE", DBL_MIN);
    MCONST("NEGATIVE_INFINITY", jsi_ieee_makeinf(-1));
    Jsi_ValueInsertFixed(interp, val, "POSITIVE_INFINITY", Inf);
    Jsi_ValueInsertFixed(interp, val, "NaN", NaN);
    return JSI_OK;
}
#endif
