#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

static Jsi_RC BooleanConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (Jsi_FunctionIsConstructor(funcPtr)) {
        int nv = 0;
        if (Jsi_ValueGetLength(interp, args) > 0) {
            Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
            if (v) {
                nv = Jsi_ValueIsTrue(interp, v);
            }
        }
        if (_this->vt == JSI_VT_OBJECT) {
            _this->d.obj->ot = JSI_OT_BOOL;
            _this->d.obj->d.val = nv;
        }
        Jsi_ValueMakeBool(interp, ret, nv);
        return JSI_OK;
    }
    if (Jsi_ValueGetLength(interp, args) > 0) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
        if (v) {
            Jsi_ValueMakeBool(interp, ret, Jsi_ValueIsTrue(interp, v));
            return JSI_OK;
        }
    }
    Jsi_ValueMakeBool(interp, ret, 0);
    return JSI_OK;
}

static Jsi_CmdSpec booleanCmds[] = {
    { "Boolean",  BooleanConstructor, 0,  1,  "bool:boolean=false", .help="Boolean constructor", .retType=(uint)JSI_TT_BOOLEAN, .flags=JSI_CMD_IS_CONSTRUCTOR },
    { NULL, 0,0,0,0, .help="A Boolean object" }
};

Jsi_RC jsi_InitBoolean(Jsi_Interp *interp, int release)
{
    if (!release)
        interp->Boolean_prototype = Jsi_CommandCreateSpecs(interp, "Boolean", booleanCmds, NULL, JSI_CMDSPEC_ISOBJ);
    return JSI_OK;
}
#endif

