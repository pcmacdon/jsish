#include <math.h>
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

bool Jsi_NumberIsSubnormal(Jsi_Number a) { return fpclassify(a) == FP_SUBNORMAL; }

bool Jsi_NumberIsNormal(Jsi_Number a) { return (fpclassify(a) == FP_ZERO || isnormal(a)); }

bool Jsi_NumberIsNaN(Jsi_Number n)
{
    return isnan(n);
}

int Jsi_NumberIsInfinity(Jsi_Number a) {
#if JSI__MUSL==1 || defined(__FreeBSD__) || defined(__WIN32)
    if (!isinf(a))
        return 0;
    return (a<0 ? -1 : 1);
#else
    return isinf(a);
#endif
}
bool Jsi_NumberIsInteger(Jsi_Number n) { return (isnormal(n) ? (Jsi_Number)((Jsi_Wide)(n)) == (n) : n==0.0); }

bool Jsi_NumberIsSafeInteger(Jsi_Number n) {
    Jsi_Number n1 = fabsl(n),
    n2 = 9007199254740991LL;
    return (Jsi_NumberIsInteger(n) && n1<=n2);
}

bool Jsi_NumberIsWide(Jsi_Number n) { return (isnormal(n) && (Jsi_Number)((Jsi_Wide)(n)) == (n)); }

Jsi_Number Jsi_NumberInfinity(int i)
{
    Jsi_Number r = INFINITY;
    if (i < 0) r = -r;
    return r;
}

Jsi_Number Jsi_NumberNaN(void)
{
    return NAN;
}

void Jsi_NumberItoA10(Jsi_Wide value, char* buf, int bsiz)
{
    snprintf(buf, bsiz, "%" PRId64, value);
}

void Jsi_NumberUtoA10(Jsi_UWide value, char* buf, int bsiz)
{
    snprintf(buf, bsiz, "%" PRIu64, value);
}

bool Jsi_NumberIsFinite(Jsi_Number value)
{
    Jsi_Number r = INFINITY;
    return (Jsi_NumberIsNaN(value)==0 && value != r && r != -value);
}

void Jsi_NumberDtoA(Jsi_Interp *interp, Jsi_Number value, char* buf, int bsiz, int prec)
{
    int dp = interp->subOpts.dblPrec-1, dm = __DBL_DECIMAL_DIG__;
    if (prec==0)
        prec = (dp<=0?dm+dp:dp);
    else if (prec<0)
            prec = dm+prec;
    if (prec<=0)
        prec = dm-1;
    if (Jsi_NumberIsNaN(value))
        Jsi_Strcpy(buf,"NaN");
    else
        snprintf(buf, bsiz, "%.*" JSI_NUMGFMT, prec, value);
}

bool Jsi_NumberIsEqual(Jsi_Number n1, Jsi_Number n2)
{
    return (n1 == n2); // TODO: Do we need more than this?
}

#ifndef JSI_LITE_ONLY
static Jsi_RC NumberConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        Jsi_Number nv = 0.0;
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
        if (v) {
            Jsi_ValueToNumber(interp, v);
            nv = v->d.num;
        }
        _this->d.obj->ot = JSI_OT_NUMBER;
        _this->d.obj->d.num = nv;
        Jsi_ValueToObject(interp, _this);
        Jsi_ValueMakeNumber(interp, ret, nv);
        return JSI_OK;
    }
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    if (v) {
        Jsi_ValueToNumber(interp, v);
        Jsi_ValueDup2(interp, ret, v);
        Jsi_ValueToObject(interp, *ret);
        return JSI_OK;
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
        Jsi_LogError("apply Number.%s to a non-number object", funcPtr->cmdSpec->name); \
        return JSI_ERROR; \
    } else  { \
        dest = _this; \
    }
    
static Jsi_RC NumberToFixedCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[JSI_MAX_NUMBER_STRING+1];
    int prec = 0, skip = 0;
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    Jsi_Value *pa = Jsi_ValueArrayIndex(interp, args, skip);
    if (pa && Jsi_GetIntFromValue(interp, pa, &prec) != JSI_OK)
        return JSI_ERROR;
    if (prec<0) prec = 0;
    Jsi_GetDoubleFromValue(interp, v, &num);
    snprintf(buf, sizeof(buf), "%.*" JSI_NUMFFMT, prec, num);
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static Jsi_RC NumberToPrecisionCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[JSI_MAX_NUMBER_STRING*2];
    int prec = 0, skip = 0;
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    if (Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, skip), &prec) != JSI_OK)
        return JSI_ERROR;
    if (prec<=0 || prec>JSI_MAX_NUMBER_STRING) return Jsi_LogError("precision must be between 1 and %d", JSI_MAX_NUMBER_STRING);
    Jsi_GetDoubleFromValue(interp, v, &num);
    snprintf(buf, sizeof(buf),"%.*" JSI_NUMFFMT, prec, num);
    if (num<0)
        prec++;
    buf[prec+1] = 0;
    if (buf[prec] == '.')
        buf[prec] = 0;
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static Jsi_RC NumberToExponentialCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[JSI_MAX_NUMBER_STRING+1];
    int prec = 0, skip = 0;
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    if (Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, skip), &prec) != JSI_OK)
        return JSI_ERROR;
    if (prec<0) prec = 0;
    Jsi_GetDoubleFromValue(interp, v, &num);
    snprintf(buf, sizeof(buf), "%.*" JSI_NUMEFMT, prec, num);
#ifdef __WIN32
    char *e = strrchr(buf, 'e');
    if (e && (e[1]=='+' || e[1]=='-')) {
        e++;
        int eNum = atoi(e);
        if (e[0]=='-')
            eNum = -eNum;
        e++;
        snprintf(e, (e-buf), "%02d", eNum);
    }
#endif
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static Jsi_RC NumberToStringCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[JSI_MAX_NUMBER_STRING+1];
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
        case 16: snprintf(buf, sizeof(buf), "%" PRIx64, (Jsi_Wide)num); break;
        case 8: snprintf(buf, sizeof(buf), "%" PRIo64, (Jsi_Wide)num); break;
        case 10: snprintf(buf, sizeof(buf), "%" PRId64, (Jsi_Wide)num); break;
        default: return jsi_ObjectToStringCmd(interp, args, _this, ret, funcPtr);
    }
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}

static Jsi_RC jsi_NumberIsFiniteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int op)
{
    Jsi_Number num;
    Jsi_Value *v;
    bool b = 0;
    int skip = 0;
    ChkStringN(_this, funcPtr, v);
    if (Jsi_GetNumberFromValue(interp, v, &num) != JSI_OK)
        return JSI_ERROR;
    switch (op) {
        case 1: b = Jsi_NumberIsFinite(num); break;
        case 2: b = Jsi_NumberIsInteger(num); break;
        case 3: b = Jsi_NumberIsNaN(num); break;
        case 4: b = Jsi_NumberIsSafeInteger(num); break;
    }
    Jsi_ValueMakeBool(interp, ret, b);
    skip++;
    return JSI_OK;
}

static Jsi_RC NumberIsFiniteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_NumberIsFiniteCmd(interp, args, _this, ret, funcPtr, 1);
}
static Jsi_RC NumberIsIntegerCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_NumberIsFiniteCmd(interp, args, _this, ret, funcPtr, 2);
}
static Jsi_RC NumberIsNaNCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_NumberIsFiniteCmd(interp, args, _this, ret, funcPtr, 3);
}
static Jsi_RC NumberIsSafeIntegerCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_NumberIsFiniteCmd(interp, args, _this, ret, funcPtr, 4);
}
static Jsi_CmdSpec numberCmds[] = {
    { "Number",         NumberConstructor,      0, 1, "num:string=0", .help="Number constructor", .retType=(uint)JSI_TT_NUMBER, .flags=JSI_CMD_IS_CONSTRUCTOR },
    { "isFinite",       NumberIsFiniteCmd,      0, 0, "", .help="Return true if is finite", .retType=(uint)JSI_TT_BOOLEAN },
    { "isInteger",      NumberIsIntegerCmd,     0, 0, "", .help="Return true if is an integer", .retType=(uint)JSI_TT_BOOLEAN },
    { "isNaN",          NumberIsNaNCmd,         0, 0, "", .help="Return true if is NaN", .retType=(uint)JSI_TT_BOOLEAN },
    { "isSafeInteger",  NumberIsSafeIntegerCmd, 0, 0, "", .help="Return true if is a safe integer", .retType=(uint)JSI_TT_BOOLEAN },
    { "toFixed",        NumberToFixedCmd,       0, 1, "num:number=0", .help="Formats a number with x numbers of digits after the decimal point", .retType=(uint)JSI_TT_STRING },
    { "toExponential",  NumberToExponentialCmd, 1, 1, "num:number", .help="Converts a number into an exponential notation", .retType=(uint)JSI_TT_STRING },
    { "toPrecision",    NumberToPrecisionCmd,   1, 1, "num:number", .help="Formats a number to x length", .retType=(uint)JSI_TT_STRING },
    { "toString",       NumberToStringCmd,      0, 1, "radix:number=10", .help="Convert to string", .retType=(uint)JSI_TT_STRING }, 
    { NULL, 0,0,0,0, .help="Commands for accessing number objects" }
};

Jsi_RC jsi_InitNumber(Jsi_Interp *interp, int release)
{
    if (release) return JSI_OK;
    Jsi_Value *val, *global = interp->csc;
    val = interp->Number_prototype = Jsi_CommandCreateSpecs(interp, "Number", numberCmds, NULL, JSI_CMDSPEC_ISOBJ);

    Jsi_Value *NaN = Jsi_ValueMakeNumber(interp, NULL, Jsi_NumberNaN());

    Jsi_Value *Inf = Jsi_ValueMakeNumber(interp, NULL, Jsi_NumberInfinity(1));
    
    Jsi_ValueInsertFixed(interp, global, "NaN", NaN);
    Jsi_ValueInsertFixed(interp, global, "Infinity", Inf);
    interp->NaNValue = NaN;
    interp->InfValue = Inf;
#define MCONST(name,v) Jsi_ValueInsert(interp, val, name, Jsi_ValueNewNumber(interp, v), JSI_OM_READONLY)
    MCONST("MAX_VALUE", DBL_MAX);
    MCONST("MIN_VALUE", DBL_MIN);
    MCONST("NEGATIVE_INFINITY", Jsi_NumberInfinity(-1));
    Jsi_ValueInsertFixed(interp, val, "POSITIVE_INFINITY", Inf);
    Jsi_ValueInsertFixed(interp, val, "NaN", NaN);
    return JSI_OK;
}
#endif
