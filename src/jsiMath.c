#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#ifndef JSI_OMIT_MATH
#include <math.h>

#define MFUNC1(fname, func)  \
static Jsi_RC fname (Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,\
    Jsi_Value **ret, Jsi_Func *funcPtr)\
{\
    Jsi_Value *val = Jsi_ValueArrayIndex(interp, args, 0);\
    Jsi_Number num;\
    if (Jsi_GetNumberFromValue(interp, val, &num) != JSI_OK)\
        return JSI_ERROR;\
    Jsi_ValueMakeNumber(interp, ret, func (num));\
    return JSI_OK;\
}

#define MFUNC2(fname, func)  \
static Jsi_RC fname (Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,\
    Jsi_Value **ret, Jsi_Func *funcPtr)\
{\
    Jsi_Value *val1 = Jsi_ValueArrayIndex(interp, args, 0);\
    Jsi_Number num1;\
    if (Jsi_GetNumberFromValue(interp, val1, &num1) != JSI_OK)\
        return JSI_ERROR;\
    Jsi_Value *val2 = Jsi_ValueArrayIndex(interp, args, 1);\
    Jsi_Number num2;\
    if (Jsi_GetNumberFromValue(interp, val2, &num2) != JSI_OK)\
        return JSI_ERROR;\
    num2 = func (num1,num2); \
    Jsi_ValueMakeNumber(interp, ret, (Jsi_NumberIsNormal(num2)? num2 : NAN));\
    return JSI_OK;\
}

static Jsi_RC MathMinCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int i, argc = Jsi_ValueGetLength(interp, args);
    Jsi_Value *val;
    Jsi_Number n = 0, num;
    if (argc<=0)
        return JSI_OK;
    for (i=0; i<argc; i++) {
        val = Jsi_ValueArrayIndex(interp, args, i);
        if (Jsi_GetNumberFromValue(interp, val, &num) != JSI_OK)
            return JSI_ERROR;
        if (i==0)
            n = num;
        else
            n =  (num>n ? n : num);
    }
    Jsi_ValueMakeNumber(interp, ret, n);
    return JSI_OK;
}


static Jsi_RC MathMaxCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int i, argc = Jsi_ValueGetLength(interp, args);
    Jsi_Value *val;
    Jsi_Number n = 0, num;
    if (argc<=0)
        return JSI_OK;
    for (i=0; i<argc; i++) {
        val = Jsi_ValueArrayIndex(interp, args, i);
        if (Jsi_GetNumberFromValue(interp, val, &num) != JSI_OK)
            return JSI_ERROR;
        if (i==0)
            n = num;
        else
            n =  (num<n ? n : num);
    }
    Jsi_ValueMakeNumber(interp, ret, n);
    return JSI_OK;
}

static int jsi_SrandInit = 0;

static Jsi_RC MathRandomCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
#ifndef __WIN32
    if (!jsi_SrandInit) {
        jsi_SrandInit = 1;
        srand48((long)time(NULL));
    }
    Jsi_ValueMakeNumber(interp, ret, drand48());
#else
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)rand());
#endif
    return JSI_OK;
}

static Jsi_RC MathSrandCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Number n = 0;
#ifndef __WIN32
    Jsi_GetNumberFromValue(interp, Jsi_ValueArrayIndex(interp, args, 0), &n);
    srand48((long)n);
#endif
    jsi_SrandInit = 1;
    Jsi_ValueMakeNumber(interp, ret, n);
    return JSI_OK;
}

MFUNC1(MathAbsCmd,  fabs)
MFUNC1(MathAcosCmd, acos)
MFUNC1(MathAsinCmd, asin)
MFUNC1(MathAtanCmd, atan)
MFUNC1(MathCeilCmd, ceil)
MFUNC1(MathCosCmd,  cos)
MFUNC1(MathExpCmd,  exp)
MFUNC1(MathFloorCmd,floor)
MFUNC1(MathLogCmd,  log)
MFUNC1(MathRoundCmd,round)
MFUNC1(MathSinCmd,  sin)
MFUNC1(MathSqrtCmd, sqrt)
MFUNC1(MathTanCmd,  tan)
MFUNC2(MathAtan2Cmd,atan2)
MFUNC2(MathPowCmd,  powl)

static Jsi_CmdSpec mathCmds[] = {
    { "abs",    MathAbsCmd,     1, 1, "num:number", .help="Returns the absolute value of x", .retType=(uint)JSI_TT_NUMBER },
    { "acos",   MathAcosCmd,    1, 1, "num:number", .help="Returns the arccosine of x, in radians", .retType=(uint)JSI_TT_NUMBER },
    { "asin",   MathAsinCmd,    1, 1, "num:number", .help="Returns the arcsine of x, in radians", .retType=(uint)JSI_TT_NUMBER },
    { "atan",   MathAtanCmd,    1, 1, "num:number", .help="Returns the arctangent of x as a numeric value between -PI/2 and PI/2 radians", .retType=(uint)JSI_TT_NUMBER },
    { "atan2",  MathAtan2Cmd,   2, 2, "x:number, y:number", .help="Returns the arctangent of the quotient of its arguments", .retType=(uint)JSI_TT_NUMBER },
    { "ceil",   MathCeilCmd,    1, 1, "num:number", .help="Returns x, rounded upwards to the nearest integer", .retType=(uint)JSI_TT_NUMBER },
    { "cos",    MathCosCmd,     1, 1, "num:number", .help="Returns the cosine of x (x is in radians)", .retType=(uint)JSI_TT_NUMBER },
    { "exp",    MathExpCmd,     1, 1, "num:number", .help="Returns the value of Ex", .retType=(uint)JSI_TT_NUMBER },
    { "floor",  MathFloorCmd,   1, 1, "num:number", .help="Returns x, rounded downwards to the nearest integer", .retType=(uint)JSI_TT_NUMBER },
    { "log",    MathLogCmd,     1, 1, "num:number", .help="Returns the natural logarithm (base E) of x", .retType=(uint)JSI_TT_NUMBER },
    { "max",    MathMaxCmd,     2,-1, "x:number, y:number, ...", .help="Returns the number with the highest value", .retType=(uint)JSI_TT_NUMBER },
    { "min",    MathMinCmd,     2,-1, "x:number, y:number, ...", .help="Returns the number with the lowest value", .retType=(uint)JSI_TT_NUMBER },
    { "pow",    MathPowCmd,     2, 2, "x:number, y:number", .help="Returns the value of x to the power of y", .retType=(uint)JSI_TT_NUMBER },
    { "random", MathRandomCmd,  0, 0, "", .help="Returns a random number between 0 and 1", .retType=(uint)JSI_TT_NUMBER },
    { "round",  MathRoundCmd,   1, 1, "num:number", .help="Rounds x to the nearest integer", .retType=(uint)JSI_TT_NUMBER },
    { "sin",    MathSinCmd,     1, 1, "num:number", .help="Returns the sine of x (x is in radians)", .retType=(uint)JSI_TT_NUMBER },
    { "sqrt",   MathSqrtCmd,    1, 1, "num:number", .help="Returns the square root of x", .retType=(uint)JSI_TT_NUMBER },
    { "srand",  MathSrandCmd,   1, 1, "seed:number", .help="Set random seed", .retType=(uint)JSI_TT_NUMBER },
    { "tan",    MathTanCmd,     1, 1, "num:number", .help="Returns the tangent of an angle", .retType=(uint)JSI_TT_NUMBER },
    { NULL, 0,0,0,0, .help="Commands performing math operations on numbers" }
};

    
Jsi_RC jsi_InitMath(Jsi_Interp *interp, int release)
{
    if (release) return JSI_OK;
    Jsi_Value *val = Jsi_CommandCreateSpecs(interp, "Math",    mathCmds,    NULL, 0);
#define MCONST(name,v) Jsi_ValueInsert(interp, val, name, Jsi_ValueNewNumber(interp, v), JSI_OM_READONLY)
    MCONST("PI", M_PI);
    MCONST("LN2", M_LN2);
    MCONST("LN10", M_LN10);
    MCONST("LOG2E", M_LOG2E);
    MCONST("LOG10E", M_LOG10E);
    MCONST("SQRT2", M_SQRT2);
    MCONST("SQRT1_2", M_SQRT1_2);
    MCONST("E", M_E);
    return JSI_OK;
}
#endif
#endif
