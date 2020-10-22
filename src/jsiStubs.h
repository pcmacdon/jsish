#ifndef __JSI_STUBS_H__
#define __JSI_STUBS_H__
#ifndef JSI_AMALGAMATION
#include "jsi.h"
#endif


#define JSI_STUBS_MD5 "35c36c383d326b0ea9203585674dc009"

#undef JSI_EXTENSION_INI
#define JSI_EXTENSION_INI Jsi_Stubs *jsiStubsPtr = NULL;

#ifdef JSI__MUSL
#define JSI_STUBS_BLDFLAGS 1
#else
#define JSI_STUBS_BLDFLAGS 0
#endif
#ifndef Jsi_StubsInit
#define Jsi_StubsInit(interp,flags) (jsiStubsPtr && jsiStubsPtr->sig == \
  JSI_STUBS_SIG?jsiStubsPtr->_Jsi_Stubs__initialize(interp, flags, "jsi", \
  JSI_VERSION, JSI_STUBS_MD5, JSI_STUBS_BLDFLAGS, sizeof(Jsi_Stubs), JSI_STUBS_STRUCTSIZES, (void**)&jsiStubsPtr):JSI_ERROR)
#endif

typedef struct Jsi_Stubs {
    uint sig;
    const char* name;
    int size;
    int bldFlags;
    const char* md5;
    void *hook;
    int(*_Jsi_Stubs__initialize)(Jsi_Interp *interp, double version, const char* name, int flags, const char *md5, int bldFlags, int stubSize, int structSizes, void **ptr);
    Jsi_Interp*(*_Jsi_InterpNew)(Jsi_InterpOpts *opts);
    void(*_Jsi_InterpDelete)( Jsi_Interp* interp);
    void(*_Jsi_InterpOnDelete)(Jsi_Interp *interp, Jsi_DeleteProc *freeProc, void *ptr);
    Jsi_RC(*_Jsi_Interactive)(Jsi_Interp* interp, int flags);
    bool(*_Jsi_InterpGone)( Jsi_Interp* interp);
    Jsi_Value*(*_Jsi_InterpResult)(Jsi_Interp *interp);
    const char*(*_Jsi_InterpLastError)(Jsi_Interp *interp, const char **errFilePtr, int *errLinePtr);
    void*(*_Jsi_InterpGetData)(Jsi_Interp *interp, const char *key, Jsi_DeleteProc **proc);
    void(*_Jsi_InterpSetData)(Jsi_Interp *interp, const char *key, void *data, Jsi_DeleteProc *proc);
    void(*_Jsi_InterpFreeData)(Jsi_Interp *interp, const char *key);
    bool(*_Jsi_InterpSafe)(Jsi_Interp *interp);
    Jsi_RC(*_Jsi_InterpAccess)(Jsi_Interp *interp, Jsi_Value* resource, int aflag);
    Jsi_Interp*(*_Jsi_Main)(Jsi_InterpOpts *opts);
    void*(*_Jsi_Malloc)(uint size);
    void*(*_Jsi_Calloc)(uint n, uint size);
    void*(*_Jsi_Realloc)(void *m, uint size);
    void (*_Jsi_Free)(void *m);
    int(*_Jsi_ObjIncrRefCount)(Jsi_Interp* interp, Jsi_Obj *obj);
    int(*_Jsi_ObjDecrRefCount)(Jsi_Interp* interp, Jsi_Obj *obj);
    int(*_Jsi_IncrRefCount)(Jsi_Interp* interp, Jsi_Value *v);
    int(*_Jsi_DecrRefCount)(Jsi_Interp* interp, Jsi_Value *v);
    bool(*_Jsi_IsShared)(Jsi_Interp* interp, Jsi_Value *v);
    Jsi_RC(*_Jsi_DeleteData)(Jsi_Interp* interp, void *m);
    uint(*_Jsi_Strlen)(const char *str);
    uint(*_Jsi_StrlenSet)(const char *str, uint len);
    int(*_Jsi_Strcmp)(const char *str1, const char *str2);
    int(*_Jsi_Strncmp)(const char *str1, const char *str2, int n);
    int(*_Jsi_Strncasecmp)(const char *str1, const char *str2, int n);
    int(*_Jsi_StrcmpDict)(const char *str1, const char *str2, int nocase, int dict);
    char*(*_Jsi_Strcpy)(char *dst, const char *src);
    char*(*_Jsi_Strncpy)(char *dst, const char *src, int len);
    char*(*_Jsi_Strdup)(const char *n);
    char*(*_Jsi_Strrchr)(const char *str, int c);
    char*(*_Jsi_Strstr)(const char *str, const char *sub);
    int(*_Jsi_ObjArraySizer)(Jsi_Interp *interp, Jsi_Obj *obj, uint n);
    char*(*_Jsi_Strchr)(const char *str, int c);
    int(*_Jsi_Strpos)(const char *str, int start, const char *nid, int nocase);
    int(*_Jsi_Strrpos)(const char *str, int start, const char *nid, int nocase);
    char*  (*_Jsi_DSAppendLen)(Jsi_DString *dsPtr,const char *bytes, int length);
    char*  (*_Jsi_DSAppend)(Jsi_DString *dsPtr, const char *str, ...);
    void   (*_Jsi_DSFree)(Jsi_DString *dsPtr);
    char*  (*_Jsi_DSFreeDup)(Jsi_DString *dsPtr);
    void   (*_Jsi_DSInit)(Jsi_DString *dsPtr);
    uint   (*_Jsi_DSLength)(Jsi_DString *dsPtr);
    char*  (*_Jsi_DSPrintf)(Jsi_DString *dsPtr, const char *fmt, ...);
    char*  (*_Jsi_DSSet)(Jsi_DString *dsPtr, const char *str);
    uint   (*_Jsi_DSSetLength)(Jsi_DString *dsPtr, uint length);
    char*  (*_Jsi_DSValue)(Jsi_DString *dsPtr);
    Jsi_Value*(*_Jsi_CommandCreate)(Jsi_Interp *interp, const char *name, Jsi_CmdProc *cmdProc, void *privData);
    Jsi_Value*(*_Jsi_CommandCreateSpecs)(Jsi_Interp *interp, const char *name, Jsi_CmdSpec *cmdSpecs, void *privData, int flags);
    void*(*_Jsi_CommandNewObj)(Jsi_Interp *interp, const char *name, const char *arg1, const char *opts, const char *var);
    Jsi_RC(*_Jsi_CommandInvokeJSON)(Jsi_Interp *interp, const char *cmd, const char *json, Jsi_Value **ret);
    Jsi_RC(*_Jsi_CommandInvoke)(Jsi_Interp *interp, const char *cmdstr, Jsi_Value *args, Jsi_Value **ret);
    Jsi_RC(*_Jsi_CommandDelete)(Jsi_Interp *interp, const char *name);
    Jsi_CmdSpec*(*_Jsi_FunctionGetSpecs)(Jsi_Func *funcPtr);
    bool(*_Jsi_FunctionIsConstructor)(Jsi_Func *funcPtr);
    bool(*_Jsi_FunctionReturnIgnored)(Jsi_Interp *interp, Jsi_Func *funcPtr);
    void*(*_Jsi_FunctionPrivData)(Jsi_Func *funcPtr);
    Jsi_RC(*_Jsi_FunctionArguments)(Jsi_Interp *interp, Jsi_Value *func, int *argcPtr);
    Jsi_RC(*_Jsi_FunctionApply)(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, Jsi_Value **ret);
    Jsi_RC(*_Jsi_FunctionInvoke)(Jsi_Interp *interp, Jsi_Value *tocall, Jsi_Value *args, Jsi_Value **ret, Jsi_Value *_this);
    Jsi_RC(*_Jsi_FunctionInvokeJSON)(Jsi_Interp *interp, Jsi_Value *tocall, const char *json, Jsi_Value **ret, Jsi_Value *_this);
    int(*_Jsi_FunctionInvokeBool)(Jsi_Interp *interp, Jsi_Value *func, Jsi_Value *arg, Jsi_Value* _this);
    Jsi_RC(*_Jsi_FunctionInvokeString)(Jsi_Interp *interp, Jsi_Value *func, Jsi_Value *arg, Jsi_DString *dStr, Jsi_Value* _this);
    Jsi_Value*(*_Jsi_VarLookup)(Jsi_Interp *interp, const char *varname);
    Jsi_Value*(*_Jsi_NameLookup)(Jsi_Interp *interp, const char *varname);
    Jsi_Value*(*_Jsi_NameLookup2)(Jsi_Interp *interp, const char *name, const char *inObj);
    Jsi_RC(*_Jsi_PkgProvideEx)(Jsi_Interp *interp, const char *name, Jsi_Number version, Jsi_InitProc *initProc, Jsi_PkgOpts* popts);
    Jsi_Number(*_Jsi_PkgRequireEx)(Jsi_Interp *interp, const char *name, Jsi_Number version, Jsi_PkgOpts **poptsPtr);
    Jsi_Number(*_Jsi_PkgVersion)(Jsi_Interp *interp, const char *name, const char **filePtr);
    uint(*_Jsi_NumUtfBytes)(char c);
    uint(*_Jsi_NumUtfChars)(const char *utf, int length);
    uint(*_Jsi_UtfGetIndex)(const char *utf, int index, char outbuf[5]);
    const char*(*_Jsi_UtfAtIndex)(const char *utf, int index);
    uint(*_Jsi_UniCharToUtf)(Jsi_UniChar uc, char *dest);
    uint(*_Jsi_UtfToUniChar)(const char *utf, Jsi_UniChar *ch);
    uint(*_Jsi_UtfToUniCharCase)(const char *utf, Jsi_UniChar *ch, int upper);
    uint(*_Jsi_UtfDecode)(const char *str, char* oututf);
    uint(*_Jsi_UtfEncode)(const char *utf, char *outstr);
    char*(*_Jsi_UtfSubstr)(const char *str, int n, int len, Jsi_DString *dStr);
    int(*_Jsi_UtfIndexToOffset)(const char *utf, int index);
    Jsi_Obj*(*_Jsi_ObjNew)(Jsi_Interp* interp);
    Jsi_Obj*(*_Jsi_ObjNewType)(Jsi_Interp* interp, Jsi_otype type);
    void(*_Jsi_ObjFree)(Jsi_Interp* interp, Jsi_Obj *obj);
    Jsi_Obj*(*_Jsi_ObjNewObj)(Jsi_Interp *interp, Jsi_Value **items, int count);
    Jsi_Obj*(*_Jsi_ObjNewArray)(Jsi_Interp *interp, Jsi_Value **items, int count, int copy);
    bool     (*_Jsi_ObjIsArray)(Jsi_Interp *interp, Jsi_Obj *o);
    void    (*_Jsi_ObjSetLength)(Jsi_Interp *interp, Jsi_Obj *obj, uint len);
    int     (*_Jsi_ObjGetLength)(Jsi_Interp *interp, Jsi_Obj *obj);
    const char*(*_Jsi_ObjTypeStr)(Jsi_Interp *interp, Jsi_Obj *obj);
    Jsi_otype(*_Jsi_ObjTypeGet)(Jsi_Obj *obj);
    void    (*_Jsi_ObjListifyArray)(Jsi_Interp *interp, Jsi_Obj *obj);
    Jsi_RC  (*_Jsi_ObjArraySet)(Jsi_Interp *interp, Jsi_Obj *obj, Jsi_Value *value, int arrayindex);
    Jsi_RC  (*_Jsi_ObjArrayAdd)(Jsi_Interp *interp, Jsi_Obj *o, Jsi_Value *v);
    Jsi_RC  (*_Jsi_ObjInsert)(Jsi_Interp *interp, Jsi_Obj *obj, const char *key, Jsi_Value *nv, int flags);
    void   (*_Jsi_ObjFromDS)(Jsi_DString *dsPtr, Jsi_Obj *obj);
    Jsi_Value*(*_Jsi_ValueNew)(Jsi_Interp *interp);
    Jsi_Value*(*_Jsi_ValueNew1)(Jsi_Interp *interp);
    void(*_Jsi_ValueFree)(Jsi_Interp *interp, Jsi_Value* v);
    Jsi_Value*(*_Jsi_ValueNewNull)(Jsi_Interp *interp);
    Jsi_Value*(*_Jsi_ValueNewBoolean)(Jsi_Interp *interp, int bval);
    Jsi_Value*(*_Jsi_ValueNewNumber)(Jsi_Interp *interp, Jsi_Number n);
    Jsi_Value*(*_Jsi_ValueNewBlob)(Jsi_Interp *interp, uchar *s, uint len);
    Jsi_Value*(*_Jsi_ValueNewString)(Jsi_Interp *interp, const char *s, int len);
    Jsi_Value*(*_Jsi_ValueNewStringKey)(Jsi_Interp *interp, const char *s);
    Jsi_Value*(*_Jsi_ValueNewStringDup)(Jsi_Interp *interp, const char *s);
    Jsi_Value*(*_Jsi_ValueNewArray)(Jsi_Interp *interp, const char **items, int count);
    Jsi_Value*(*_Jsi_ValueNewObj)(Jsi_Interp *interp, Jsi_Obj *o);
    Jsi_RC(*_Jsi_GetStringFromValue)(Jsi_Interp* interp, Jsi_Value *value, const char **sPtr, int *lenPtr);
    Jsi_RC(*_Jsi_GetNumberFromValue)(Jsi_Interp* interp, Jsi_Value *value, Jsi_Number *n);
    Jsi_RC(*_Jsi_GetBoolFromValue)(Jsi_Interp* interp, Jsi_Value *value, bool *n);
    Jsi_RC(*_Jsi_GetIntFromValue)(Jsi_Interp* interp, Jsi_Value *value, int *n);
    Jsi_RC(*_Jsi_GetLongFromValue)(Jsi_Interp* interp, Jsi_Value *value, long *n);
    Jsi_RC(*_Jsi_GetWideFromValue)(Jsi_Interp* interp, Jsi_Value *value, Jsi_Wide *n);
    Jsi_RC(*_Jsi_GetDoubleFromValue)(Jsi_Interp* interp, Jsi_Value *value, Jsi_Number *n);
    Jsi_RC(*_Jsi_GetIntFromValueBase)(Jsi_Interp* interp, Jsi_Value *value, int *n, int base, int flags);
    Jsi_RC(*_Jsi_ValueGetBoolean)(Jsi_Interp *interp, Jsi_Value *pv, bool *val);
    Jsi_RC(*_Jsi_ValueGetNumber)(Jsi_Interp *interp, Jsi_Value *pv, Jsi_Number *val);
    bool(*_Jsi_ValueIsType)(Jsi_Interp *interp, Jsi_Value *pv, Jsi_vtype vtype);
    bool(*_Jsi_ValueIsObjType)(Jsi_Interp *interp, Jsi_Value *v, Jsi_otype otype);
    bool(*_Jsi_ValueIsTrue)(Jsi_Interp *interp, Jsi_Value *v);
    bool(*_Jsi_ValueIsFalse)(Jsi_Interp *interp, Jsi_Value *v);
    bool(*_Jsi_ValueIsNumber)(Jsi_Interp *interp, Jsi_Value *pv);
    bool(*_Jsi_ValueIsArray)(Jsi_Interp *interp, Jsi_Value *v);
    bool(*_Jsi_ValueIsBoolean)(Jsi_Interp *interp, Jsi_Value *pv);
    bool(*_Jsi_ValueIsNull)(Jsi_Interp *interp, Jsi_Value *pv);
    bool(*_Jsi_ValueIsUndef)(Jsi_Interp *interp, Jsi_Value *pv);
    bool(*_Jsi_ValueIsFunction)(Jsi_Interp *interp, Jsi_Value *pv);
    bool(*_Jsi_ValueIsString)(Jsi_Interp *interp, Jsi_Value *pv);
    Jsi_Value*(*_Jsi_ValueMakeObject)(Jsi_Interp *interp, Jsi_Value **v, Jsi_Obj *o);
    Jsi_Value*(*_Jsi_ValueMakeArrayObject)(Jsi_Interp *interp, Jsi_Value **v, Jsi_Obj *o);
    Jsi_Value*(*_Jsi_ValueMakeNumber)(Jsi_Interp *interp, Jsi_Value **v, Jsi_Number n);
    Jsi_Value*(*_Jsi_ValueMakeBool)(Jsi_Interp *interp, Jsi_Value **v, int b);
    Jsi_Value*(*_Jsi_ValueMakeString)(Jsi_Interp *interp, Jsi_Value **v, const char *s);
    Jsi_Value*(*_Jsi_ValueMakeStringKey)(Jsi_Interp *interp, Jsi_Value **v, const char *s);
    Jsi_Value*(*_Jsi_ValueMakeBlob)(Jsi_Interp *interp, Jsi_Value **v, uchar *s, int len);
    Jsi_Value*(*_Jsi_ValueMakeNull)(Jsi_Interp *interp, Jsi_Value **v);
    Jsi_Value*(*_Jsi_ValueMakeUndef)(Jsi_Interp *interp, Jsi_Value **v);
    Jsi_Value*(*_Jsi_ValueMakeDStringObject)(Jsi_Interp *interp, Jsi_Value **v, Jsi_DString *dsPtr);
    bool(*_Jsi_ValueIsStringKey)(Jsi_Interp* interp, Jsi_Value *key);
    const char* (*_Jsi_ValueToString)(Jsi_Interp *interp, Jsi_Value *v, int *lenPtr);
    Jsi_RC      (*_Jsi_ValueToBool)(Jsi_Interp *interp, Jsi_Value *v);
    Jsi_RC      (*_Jsi_ValueToNumber)(Jsi_Interp *interp, Jsi_Value *v);
    Jsi_Number  (*_Jsi_ValueToNumberInt)(Jsi_Interp *interp, Jsi_Value *v, int isInt);
    Jsi_RC      (*_Jsi_ValueToObject)(Jsi_Interp *interp, Jsi_Value *v);
    void    (*_Jsi_ValueReset)(Jsi_Interp *interp, Jsi_Value **v);
    const char*(*_Jsi_ValueGetDString)(Jsi_Interp* interp, Jsi_Value* v, Jsi_DString *dStr, int quote);
    char*   (*_Jsi_ValueString)(Jsi_Interp* interp, Jsi_Value* v, int *lenPtr);
    uchar*  (*_Jsi_ValueBlob)(Jsi_Interp *interp, Jsi_Value* v, int *lenPtr);
    char*   (*_Jsi_ValueGetStringLen)(Jsi_Interp *interp, Jsi_Value *pv, int *lenPtr);
    int     (*_Jsi_ValueStrlen)(Jsi_Value* v);
    void    (*_Jsi_ValueFromDS)(Jsi_Interp *interp, Jsi_DString *dsPtr, Jsi_Value **ret);
    int     (*_Jsi_ValueInstanceOf)( Jsi_Interp *interp, Jsi_Value* v1, Jsi_Value* v2);
    Jsi_Obj*(*_Jsi_ValueGetObj)(Jsi_Interp* interp, Jsi_Value* v);
    Jsi_vtype(*_Jsi_ValueTypeGet)(Jsi_Value *pv);
    const char*(*_Jsi_ValueTypeStr)(Jsi_Interp *interp, Jsi_Value *v);
    int     (*_Jsi_ValueCmp)(Jsi_Interp *interp, Jsi_Value *v1, Jsi_Value* v2, int cmpFlags);
    Jsi_RC(*_Jsi_ValueGetIndex)( Jsi_Interp *interp, Jsi_Value *valPtr, const char **tablePtr, const char *msg, int flags, int *indexPtr);
    Jsi_RC(*_Jsi_ValueArraySort)(Jsi_Interp *interp, Jsi_Value *val, int sortFlags);
    Jsi_Value*(*_Jsi_ValueArrayConcat)(Jsi_Interp *interp, Jsi_Value *arg1, Jsi_Value *arg2);
    Jsi_RC(*_Jsi_ValueArrayPush)(Jsi_Interp *interp, Jsi_Value *arg1, Jsi_Value *arg2);
    Jsi_Value*(*_Jsi_ValueArrayPop)(Jsi_Interp *interp, Jsi_Value *arg1);
    void(*_Jsi_ValueArrayShift)(Jsi_Interp *interp, Jsi_Value *v);
    Jsi_Value*(*_Jsi_ValueArrayUnshift)(Jsi_Interp *interp, Jsi_Value *v);
    Jsi_Value*(*_Jsi_ValueArrayIndex)(Jsi_Interp *interp, Jsi_Value *args, int index);
    char*(*_Jsi_ValueArrayIndexToStr)(Jsi_Interp *interp, Jsi_Value *args, int index, int *lenPtr);
    Jsi_RC(*_Jsi_ValueInsert)(Jsi_Interp *interp, Jsi_Value *target, const char *key, Jsi_Value *val, int flags);
    int(*_Jsi_ValueGetLength)(Jsi_Interp *interp, Jsi_Value *v);
    Jsi_Value*(*_Jsi_ValueObjLookup)(Jsi_Interp *interp, Jsi_Value *target, const char *key, int iskeystr);
    bool(*_Jsi_ValueKeyPresent)(Jsi_Interp *interp, Jsi_Value *target, const char *k, int isstrkey);
    Jsi_RC(*_Jsi_ValueGetKeys)(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *ret);
    void(*_Jsi_ValueCopy)(Jsi_Interp *interp, Jsi_Value *to, Jsi_Value *from );
    void(*_Jsi_ValueReplace)(Jsi_Interp *interp, Jsi_Value **to, Jsi_Value *from );
    void(*_Jsi_ValueDup2)(Jsi_Interp *interp, Jsi_Value **to, Jsi_Value *from);
    Jsi_Value*(*_Jsi_ValueDupJSON)(Jsi_Interp *interp, Jsi_Value *val);
    void(*_Jsi_ValueMove)(Jsi_Interp *interp, Jsi_Value *to, Jsi_Value *from);
    bool (*_Jsi_ValueIsEqual)(Jsi_Interp *interp, Jsi_Value *v1, Jsi_Value* v2);
    Jsi_Hash*(*_Jsi_UserObjRegister)(Jsi_Interp *interp, Jsi_UserObjReg *reg);
    Jsi_RC(*_Jsi_UserObjUnregister)(Jsi_Interp *interp, Jsi_UserObjReg *reg);
    int(*_Jsi_UserObjNew)(Jsi_Interp *interp, Jsi_UserObjReg* reg, Jsi_Obj *obj, void *data);
    void*(*_Jsi_UserObjGetData)(Jsi_Interp *interp, Jsi_Value* value, Jsi_Func *funcPtr);
    char*(*_Jsi_NumberToString)(Jsi_Interp *interp, Jsi_Number d, char *buf, int bsiz);
    Jsi_Number(*_Jsi_Version)(void);
    Jsi_Value*(*_Jsi_ReturnValue)(Jsi_Interp *interp);
    Jsi_RC(*_Jsi_Mount)( Jsi_Interp *interp, Jsi_Value *archive, Jsi_Value *mount, Jsi_Value **ret);
    Jsi_Value*(*_Jsi_Executable)(Jsi_Interp *interp);
    Jsi_Regex*(*_Jsi_RegExpNew)(Jsi_Interp *interp, const char *regtxt, int flag);
    void(*_Jsi_RegExpFree)(Jsi_Regex* re);
    Jsi_RC(*_Jsi_RegExpMatch)( Jsi_Interp *interp,  Jsi_Value *pattern, const char *str, int *rc, Jsi_DString *dStr);
    Jsi_RC(*_Jsi_RegExpMatches)(Jsi_Interp *interp, Jsi_Value *pattern, const char *str, int slen, Jsi_Value *ret);
    bool(*_Jsi_GlobMatch)(const char *pattern, const char *string, int nocase);
    char*(*_Jsi_FileRealpath)(Jsi_Interp *interp, Jsi_Value *path, char *newpath);
    char*(*_Jsi_FileRealpathStr)(Jsi_Interp *interp, const char *path, char *newpath);
    char*(*_Jsi_NormalPath)(Jsi_Interp *interp, const char *path, Jsi_DString *dStr);
    char*(*_Jsi_ValueNormalPath)(Jsi_Interp *interp, Jsi_Value *path, Jsi_DString *dStr);
    Jsi_RC(*_Jsi_JSONParse)(Jsi_Interp *interp, const char *js, Jsi_Value **ret, int flags);
    Jsi_RC(*_Jsi_JSONParseFmt)(Jsi_Interp *interp, Jsi_Value **ret, const char *fmt, ...);
    char*(*_Jsi_JSONQuote)(Jsi_Interp *interp, const char *str, int len, Jsi_DString *dStr);
    Jsi_RC(*_Jsi_EvalString)(Jsi_Interp* interp, const char *str, int flags);
    Jsi_RC(*_Jsi_EvalFile)(Jsi_Interp* interp, Jsi_Value *fname, int flags);
    Jsi_RC(*_Jsi_EvalCmdJSON)(Jsi_Interp *interp, const char *cmd, const char *jsonArgs, Jsi_DString *dStr, int flags);
    Jsi_RC(*_Jsi_EvalZip)(Jsi_Interp *interp, const char *exeFile, const char *mntDir, int *jsFound);
    int(*_Jsi_DictionaryCompare)(const char *left, const char *right);
    Jsi_RC(*_Jsi_GetBool)(Jsi_Interp* interp, const char *string, bool *n);
    Jsi_RC(*_Jsi_GetInt)(Jsi_Interp* interp, const char *string, int *n, int base);
    Jsi_RC(*_Jsi_GetWide)(Jsi_Interp* interp, const char *string, Jsi_Wide *n, int base);
    Jsi_RC(*_Jsi_GetDouble)(Jsi_Interp* interp, const char *string, Jsi_Number *n);
    Jsi_RC(*_Jsi_FormatString)(Jsi_Interp *interp, Jsi_Value *args, Jsi_DString *dStr);
    void(*_Jsi_SplitStr)(const char *str, int *argcPtr, char ***argvPtr,  const char *splitCh, Jsi_DString *dStr);
    Jsi_RC(*_Jsi_Sleep)(Jsi_Interp *interp, Jsi_Number dtim);
    void(*_Jsi_Preserve)(Jsi_Interp* interp, void *data);
    void(*_Jsi_Release)(Jsi_Interp* interp, void *data);
    void(*_Jsi_EventuallyFree)(Jsi_Interp* interp, void *data, Jsi_DeleteProc* proc);
    void(*_Jsi_ShiftArgs)(Jsi_Interp *interp, Jsi_Value *v);
    Jsi_Value*(*_Jsi_StringSplit)(Jsi_Interp *interp, const char *str, const char *spliton);
    Jsi_RC(*_Jsi_GetIndex)( Jsi_Interp *interp, const char *str, const char **tablePtr, const char *msg, int flags, int *indexPtr);
    void*(*_Jsi_PrototypeGet)(Jsi_Interp *interp, const char *key);
    Jsi_RC (*_Jsi_PrototypeDefine)(Jsi_Interp *interp, const char *key, Jsi_Value *proto);
    Jsi_RC(*_Jsi_PrototypeObjSet)(Jsi_Interp *interp, const char *key, Jsi_Obj *obj);
    Jsi_RC(*_Jsi_ThisDataSet)(Jsi_Interp *interp, Jsi_Value *_this, void *value);
    void*(*_Jsi_ThisDataGet)(Jsi_Interp *interp, Jsi_Value *_this);
    Jsi_RC(*_Jsi_FuncObjToString)(Jsi_Interp *interp, Jsi_Func *f, Jsi_DString *dStr, int flags);
    void*(*_Jsi_UserObjDataFromVar)(Jsi_Interp *interp, const char *var);
    const char*(*_Jsi_KeyAdd)(Jsi_Interp *interp, const char *str);
    const char*(*_Jsi_KeyLookup)(Jsi_Interp *interp, const char *str);
    Jsi_RC(*_Jsi_DatetimeFormat)(Jsi_Interp *interp, Jsi_Number date, const char *fmt, int isUtc, Jsi_DString *dStr);
    Jsi_RC(*_Jsi_DatetimeParse)(Jsi_Interp *interp, const char *str, const char *fmt, int isUtc, Jsi_Number *datePtr, bool noMsg);
    Jsi_Number(*_Jsi_DateTime)(void);
    Jsi_RC(*_Jsi_Encrypt)(Jsi_Interp *interp, Jsi_DString *inout, const char *key, uint keyLen, bool decrypt);
    Jsi_RC(*_Jsi_CryptoHash)(char *outbuf, const char *str, int len, Jsi_CryptoHashType type, uint strength, bool noHex, int *sizPtr);
    Jsi_RC(*_Jsi_Base64)(const char *str, int len, Jsi_DString *buf, bool decode);
    int(*_Jsi_HexStr)(const uchar *data, int len, Jsi_DString *dStr, bool decode);
    char*(*_Jsi_Strrstr)(const char *str, const char *sub);
    uint32_t(*_Jsi_Crc32)(uint32_t crc, const void *ptr, size_t buf_len);
    int(*_Jsi_NumberIsInfinity)(Jsi_Number a);
    bool(*_Jsi_NumberIsEqual)(Jsi_Number n1, Jsi_Number n2);
    bool(*_Jsi_NumberIsFinite)(Jsi_Number value);
    bool(*_Jsi_NumberIsInteger)(Jsi_Number n);
    bool(*_Jsi_NumberIsNaN)(Jsi_Number a);
    bool(*_Jsi_NumberIsNormal)(Jsi_Number a);
    bool(*_Jsi_NumberIsSubnormal)(Jsi_Number a);
    bool(*_Jsi_NumberIsWide)(Jsi_Number n);
    Jsi_Number(*_Jsi_NumberInfinity)(int i);
    Jsi_Number(*_Jsi_NumberNaN)(void);
    void(*_Jsi_NumberDtoA)(Jsi_Interp *interp, Jsi_Number value, char* buf, int bsiz, int prec);
    void(*_Jsi_NumberItoA10)(Jsi_Wide value, char* buf, int bsiz);
    void(*_Jsi_NumberUtoA10)(Jsi_UWide, char* buf, int bsiz);
    Jsi_Hash*(*_Jsi_HashNew)(Jsi_Interp *interp, uint keyType, Jsi_HashDeleteProc *freeProc);
    Jsi_RC(*_Jsi_HashConf)(Jsi_Hash *hashPtr, Jsi_MapOpts *opts, bool set);
    int(*_Jsi_HashDelete)(Jsi_Hash *hashPtr);
    void(*_Jsi_HashClear)(Jsi_Hash *hashPtr);
    Jsi_HashEntry*(*_Jsi_HashSet)(Jsi_Hash *hashPtr, const void *key, void *value);
    void*(*_Jsi_HashGet)(Jsi_Hash *hashPtr, const void *key, int flags);
    bool(*_Jsi_HashUnset)(Jsi_Hash *tbl, const void *key);
    void*(*_Jsi_HashKeyGet)(Jsi_HashEntry *h);
    Jsi_RC(*_Jsi_HashKeysDump)(Jsi_Interp *interp, Jsi_Hash *hashPtr, Jsi_Value **ret, int flags);
    void*(*_Jsi_HashValueGet)(Jsi_HashEntry *h);
    void(*_Jsi_HashValueSet)(Jsi_HashEntry *h, void *value);
    Jsi_HashEntry*(*_Jsi_HashEntryFind)(Jsi_Hash *hashPtr, const void *key);
    Jsi_HashEntry*(*_Jsi_HashEntryNew)(Jsi_Hash *hashPtr, const void *key, bool *isNew);
    int(*_Jsi_HashEntryDelete)(Jsi_HashEntry *entryPtr);
    Jsi_HashEntry*(*_Jsi_HashSearchFirst)(Jsi_Hash *hashPtr, Jsi_HashSearch *searchPtr);
    Jsi_HashEntry*(*_Jsi_HashSearchNext)(Jsi_HashSearch *searchPtr);
    uint(*_Jsi_HashSize)(Jsi_Hash *hashPtr);
    Jsi_Tree*(*_Jsi_TreeNew)(Jsi_Interp *interp, uint keyType, Jsi_TreeDeleteProc *freeProc);
    Jsi_RC(*_Jsi_TreeConf)(Jsi_Tree *treePtr, Jsi_MapOpts *opts, bool set);
    int(*_Jsi_TreeDelete)(Jsi_Tree *treePtr);
    void(*_Jsi_TreeClear)(Jsi_Tree *treePtr);
    Jsi_TreeEntry*(*_Jsi_TreeObjSetValue)(Jsi_Obj* obj, const char *key, Jsi_Value *val, int isstrkey);
    Jsi_Value*    (*_Jsi_TreeObjGetValue)(Jsi_Obj* obj, const char *key, int isstrkey);
    void*(*_Jsi_TreeValueGet)(Jsi_TreeEntry *hPtr);
    void(*_Jsi_TreeValueSet)(Jsi_TreeEntry *hPtr, void *value);
    void*(*_Jsi_TreeKeyGet)(Jsi_TreeEntry *hPtr);
    Jsi_TreeEntry*(*_Jsi_TreeEntryFind)(Jsi_Tree *treePtr, const void *key);
    Jsi_TreeEntry*(*_Jsi_TreeEntryNew)(Jsi_Tree *treePtr, const void *key, bool *isNew);
    int(*_Jsi_TreeEntryDelete)(Jsi_TreeEntry *entryPtr);
    Jsi_TreeEntry*(*_Jsi_TreeSearchFirst)(Jsi_Tree *treePtr, Jsi_TreeSearch *searchPtr, int flags, const void *startKey);
    Jsi_TreeEntry*(*_Jsi_TreeSearchNext)(Jsi_TreeSearch *searchPtr);
    void(*_Jsi_TreeSearchDone)(Jsi_TreeSearch *searchPtr);
    int(*_Jsi_TreeWalk)(Jsi_Tree* treePtr, Jsi_TreeWalkProc* callback, void *data, int flags);
    Jsi_TreeEntry*(*_Jsi_TreeSet)(Jsi_Tree *treePtr, const void *key, void *value);
    void*(*_Jsi_TreeGet)(Jsi_Tree *treePtr, void *key, int flags);
    bool(*_Jsi_TreeUnset)(Jsi_Tree *treePtr, void *key);
    uint(*_Jsi_TreeSize)(Jsi_Tree *treePtr);
    Jsi_Tree*(*_Jsi_TreeFromValue)(Jsi_Interp *interp, Jsi_Value *v);
    Jsi_RC(*_Jsi_TreeKeysDump)(Jsi_Interp *interp, Jsi_Tree *hashPtr, Jsi_Value **ret, int flags);
    Jsi_List*(*_Jsi_ListNew)(Jsi_Interp *interp, Jsi_Wide flags, Jsi_HashDeleteProc *freeProc);
    Jsi_RC(*_Jsi_ListConf)(Jsi_List *list, Jsi_MapOpts *opts, bool set);
    int(*_Jsi_ListDelete)(Jsi_List *list);
    void(*_Jsi_ListClear)(Jsi_List *list);
    void*(*_Jsi_ListValueGet)(Jsi_ListEntry *list);
    void(*_Jsi_ListValueSet)(Jsi_ListEntry *list, const void *value);
    Jsi_ListEntry*(*_Jsi_ListEntryNew)(Jsi_List *list, const void *value, Jsi_ListEntry *before);
    int(*_Jsi_ListEntryDelete)(Jsi_ListEntry *entry);
    Jsi_ListEntry*(*_Jsi_ListSearchFirst)(Jsi_List *list, Jsi_ListSearch *search, int flags);
    Jsi_ListEntry*(*_Jsi_ListSearchNext)(Jsi_ListSearch *search);
    uint(*_Jsi_ListSize)(Jsi_List *list);
    Jsi_ListEntry*(*_Jsi_ListPush)(Jsi_List *list, Jsi_ListEntry *entry, Jsi_ListEntry *before);
    Jsi_ListEntry*(*_Jsi_ListPop)(Jsi_List *list, Jsi_ListEntry *entry);
    Jsi_Stack*(*_Jsi_StackNew)(void);
    void(*_Jsi_StackFree)(Jsi_Stack *stack);
    int(*_Jsi_StackSize)(Jsi_Stack *stack);
    void(*_Jsi_StackPush)(Jsi_Stack *stack, void *element);
    void*(*_Jsi_StackPop)(Jsi_Stack *stack);
    void*(*_Jsi_StackPeek)(Jsi_Stack *stack);
    void*(*_Jsi_StackUnshift)(Jsi_Stack *stack);
    void*(*_Jsi_StackHead)(Jsi_Stack *stack);
    void(*_Jsi_StackFreeElements)(Jsi_Interp *interp, Jsi_Stack *stack, Jsi_DeleteProc *freeFunc);
    Jsi_Map*(*_Jsi_MapNew)(Jsi_Interp *interp, Jsi_Map_Type mapType, Jsi_Key_Type keyType, Jsi_MapDeleteProc *freeProc);
    Jsi_RC(*_Jsi_MapConf)(Jsi_Map *mapPtr, Jsi_MapOpts *opts, bool set);
    int(*_Jsi_MapDelete)(Jsi_Map *mapPtr);
    void(*_Jsi_MapClear)(Jsi_Map *mapPtr);
    Jsi_MapEntry*(*_Jsi_MapSet)(Jsi_Map *mapPtr, const void *key, const void *value);
    void*(*_Jsi_MapGet)(Jsi_Map *mapPtr, const void *key, int flags);
    void*(*_Jsi_MapKeyGet)(Jsi_MapEntry *h, int flags);
    Jsi_RC(*_Jsi_MapKeysDump)(Jsi_Interp *interp, Jsi_Map *mapPtr, Jsi_Value **ret, int flags);
    void*(*_Jsi_MapValueGet)(Jsi_MapEntry *h);
    void(*_Jsi_MapValueSet)(Jsi_MapEntry *h, const void *value);
    Jsi_MapEntry*(*_Jsi_MapEntryFind)(Jsi_Map *mapPtr, const void *key);
    Jsi_MapEntry*(*_Jsi_MapEntryNew)(Jsi_Map *mapPtr, const void *key, bool *isNew);
    int(*_Jsi_MapEntryDelete)(Jsi_MapEntry *entryPtr);
    Jsi_MapEntry*(*_Jsi_MapSearchFirst)(Jsi_Map *mapPtr, Jsi_MapSearch *searchPtr, int flags);
    Jsi_MapEntry*(*_Jsi_MapSearchNext)(Jsi_MapSearch *srchPtr);
    void(*_Jsi_MapSearchDone)(Jsi_MapSearch *searchPtr);
    uint(*_Jsi_MapSize)(Jsi_Map *mapPtr);
    const Jsi_OptionTypedef*(*_Jsi_OptionTypeInfo)(Jsi_OptionId typ);
    Jsi_OptionTypedef*(*_Jsi_TypeLookup)(Jsi_Interp* interp, const char *typ);
    int(*_Jsi_OptionsProcess)(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, Jsi_Value *value, Jsi_Wide flags);
    int(*_Jsi_OptionsProcessJSON)(Jsi_Interp *interp, Jsi_OptionSpec *opts, void *data, const char *json, Jsi_Wide flags);
    Jsi_RC(*_Jsi_OptionsConf)(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, Jsi_Value *value, Jsi_Value **ret, Jsi_Wide flags);
    void(*_Jsi_OptionsFree)(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, Jsi_Wide flags);
    Jsi_RC(*_Jsi_OptionsGet)(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, const char *option, Jsi_Value** valuePtr, Jsi_Wide flags);
    Jsi_RC(*_Jsi_OptionsSet)(Jsi_Interp *interp, Jsi_OptionSpec *specs, void* data, const char *option, Jsi_Value *valuePtr, Jsi_Wide flags);
    Jsi_RC(*_Jsi_OptionsDump)(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *data, Jsi_Value** ret, Jsi_Wide flags);
    int(*_Jsi_OptionsChanged)(Jsi_Interp *interp, Jsi_OptionSpec *specs, const char *pattern, ...);
    bool(*_Jsi_OptionsValid)(Jsi_Interp *interp, Jsi_OptionSpec* spec);
    Jsi_OptionSpec*(*_Jsi_OptionsFind)(Jsi_Interp *interp, Jsi_OptionSpec *specs, const char *name, Jsi_Wide flags);
    Jsi_Value*(*_Jsi_OptionsCustomPrint)(void* clientData, Jsi_Interp *interp, const char *optionName, void *data, int offset);
    Jsi_OptionCustom*(*_Jsi_OptionCustomBuiltin)(Jsi_OptionCustom* cust);
    Jsi_OptionSpec*(*_Jsi_OptionsDup)(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs);
    const Jsi_OptionSpec*(*_Jsi_OptionSpecsCached)(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs);
    Jsi_RC(*_Jsi_MutexLock)(Jsi_Interp *interp, Jsi_Mutex *mtx);
    void(*_Jsi_MutexUnlock)(Jsi_Interp *interp, Jsi_Mutex *mtx);
    void(*_Jsi_MutexDelete)(Jsi_Interp *interp, Jsi_Mutex *mtx);
    Jsi_Mutex*(*_Jsi_MutexNew)(Jsi_Interp *interp, int timeout, int flags);
    void*(*_Jsi_CurrentThread)(void);
    void*(*_Jsi_InterpThread)(Jsi_Interp *interp);
    Jsi_RC(*_Jsi_LogMsg)(Jsi_Interp *interp, Jsi_PkgOpts* popts, uint level, const char *format,...);
    Jsi_Event*(*_Jsi_EventNew)(Jsi_Interp *interp, Jsi_EventHandlerProc *callback, void* data);
    void(*_Jsi_EventFree)(Jsi_Interp *interp, Jsi_Event* event);
    int(*_Jsi_EventProcess)(Jsi_Interp *interp, int maxEvents);
    void(*_Jsi_JsonInit)(Jsi_JsonParser *parser, Jsi_JsonTok *static_tokens, uint num_tokens);
    void(*_Jsi_JsonReset)(Jsi_JsonParser *parser);
    void(*_Jsi_JsonFree)(Jsi_JsonParser *parser);
    Jsi_JsonErrEnum(*_Jsi_JsonParse)(Jsi_JsonParser *parser, const char *js);
    Jsi_JsonTok*(*_Jsi_JsonGetToken)(Jsi_JsonParser *parser, uint index);
    Jsi_JsonTypeEnum(*_Jsi_JsonGetType)(Jsi_JsonParser *parser, uint index);
    int(*_Jsi_JsonTokLen)(Jsi_JsonParser *parser, uint index);
    const char*(*_Jsi_JsonGetTokstr)(Jsi_JsonParser *parser, const char *js, uint index, uint *len);
    const char*(*_Jsi_JsonGetTypename)(int type);
    const char*(*_Jsi_JsonGetErrname)(int code);
    void(*_Jsi_JsonDump)(Jsi_JsonParser *parser, const char *js);
    Jsi_RC(*_Jsi_FSRegister)(Jsi_Filesystem *fsPtr, void *data);
    Jsi_RC(*_Jsi_FSUnregister)(Jsi_Filesystem *fsPtr);
    Jsi_Channel(*_Jsi_FSNameToChannel)(Jsi_Interp *interp, const char *name);
    char*(*_Jsi_GetCwd)(Jsi_Interp *interp, Jsi_DString *cwdPtr);
    int(*_Jsi_Lstat)(Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf);
    int(*_Jsi_Stat)(Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf);
    int(*_Jsi_Access)(Jsi_Interp *interp, Jsi_Value* path, int mode);
    int(*_Jsi_Remove)(Jsi_Interp *interp, Jsi_Value* path, int flags);
    int(*_Jsi_Rename)(Jsi_Interp *interp, Jsi_Value *src, Jsi_Value *dst);
    int(*_Jsi_Chdir)(Jsi_Interp *interp, Jsi_Value* path);
    Jsi_Channel(*_Jsi_Open)(Jsi_Interp *interp, Jsi_Value *file, const char *modeString);
    int(*_Jsi_Eof)(Jsi_Interp *interp, Jsi_Channel chan);
    int(*_Jsi_Close)(Jsi_Interp *interp, Jsi_Channel chan);
    int(*_Jsi_Read)(Jsi_Interp *interp, Jsi_Channel chan, char *bufPtr, int toRead);
    int(*_Jsi_Write)(Jsi_Interp *interp, Jsi_Channel chan, const char *bufPtr, int slen);
    Jsi_Wide(*_Jsi_Seek)(Jsi_Interp *interp, Jsi_Channel chan, Jsi_Wide offset, int mode);
    Jsi_Wide(*_Jsi_Tell)(Jsi_Interp *interp, Jsi_Channel chan);
    int(*_Jsi_Truncate)(Jsi_Interp *interp, Jsi_Channel chan, uint len);
    Jsi_Wide(*_Jsi_Rewind)(Jsi_Interp *interp, Jsi_Channel chan);
    int(*_Jsi_Flush)(Jsi_Interp *interp, Jsi_Channel chan);
    int(*_Jsi_Getc)(Jsi_Interp *interp, Jsi_Channel chan);
    int(*_Jsi_Printf)(Jsi_Interp *interp, Jsi_Channel chan, const char *fmt, ...);
    int(*_Jsi_Ungetc)(Jsi_Interp *interp, Jsi_Channel chan, int ch);
    char*(*_Jsi_Gets)(Jsi_Interp *interp, Jsi_Channel chan, char *s, int size);
    int(*_Jsi_Puts)(Jsi_Interp *interp, Jsi_Channel chan, const char *str, int size);
    int(*_Jsi_Scandir)(Jsi_Interp *interp, Jsi_Value *path, Jsi_Dirent ***namelist, Jsi_ScandirFilter *filter, Jsi_ScandirCompare *compare );
    int(*_Jsi_SetChannelOption)(Jsi_Interp *interp, Jsi_Channel chan, const char *optionName, const char *newValue);
    char*(*_Jsi_Realpath)(Jsi_Interp *interp, Jsi_Value *path, char *newname);
    int(*_Jsi_Readlink)(Jsi_Interp *interp, Jsi_Value* path, char *ret, int len);
    Jsi_Channel(*_Jsi_GetStdChannel)(Jsi_Interp *interp, int id);
    bool(*_Jsi_FSNative)(Jsi_Interp *interp, Jsi_Value* path);
    int(*_Jsi_Link)(Jsi_Interp *interp, Jsi_Value* src, Jsi_Value *dest, int typ);
    int(*_Jsi_Chmod)(Jsi_Interp *interp, Jsi_Value* path, int mode);
    Jsi_RC(*_Jsi_StubLookup)(Jsi_Interp *interp, const char *name, void **ptr);
    int(*_Jsi_AddAutoFiles)(Jsi_Interp *interp, const char *dir);
    Jsi_Db*(*_Jsi_DbNew)(const char *zFile, int inFlags);
    void*(*_Jsi_DbHandle)(Jsi_Interp *interp, Jsi_Db* db);
    int(*_Jsi_DbQuery)(Jsi_Db *jdb, Jsi_CDataDb *cd, const char *query);
    Jsi_CDataDb*(*_Jsi_CDataLookup)(Jsi_Interp *interp, const char *name);
    Jsi_RC(*_Jsi_CDataRegister)(Jsi_Interp *interp, Jsi_CData_Static *statics);
    Jsi_RC(*_Jsi_CDataStructInit)(Jsi_Interp *interp, uchar* data, const char *sname);
    Jsi_RC(*_Jsi_DllLookup)(Jsi_Interp *interp, const char *module, const char *name, void **ptr);
    Jsi_RC(*_Jsi_LoadLibrary)(Jsi_Interp *interp, const char *pathName, bool noInit);
    Jsi_StructSpec*(*_Jsi_CDataStruct)(Jsi_Interp *interp, const char *name);
    char*(*_Jsi_StrdupLen)(const char *str, int len);
    Jsi_RC(*_Jsi_FileRead)(Jsi_Interp *interp, Jsi_Value *name, Jsi_DString *dStr);
    Jsi_Value*(*_Jsi_ValueNewStringConst)(Jsi_Interp *interp, const char *s, int len);
    Jsi_RC(*_Jsi_PathNormalize)(Jsi_Interp *interp, Jsi_Value **pathPtr);
    Jsi_RC(*_Jsi_ValueInsertArray)(Jsi_Interp *interp, Jsi_Value *target, int index, Jsi_Value *val, int flags);
    Jsi_IterObj*(*_Jsi_IterObjNew)(Jsi_Interp *interp, Jsi_IterProc *iterProc);
    void(*_Jsi_IterObjFree)(Jsi_IterObj *iobj);
    void(*_Jsi_IterGetKeys)(Jsi_Interp *interp, Jsi_Value *target, Jsi_IterObj *iterobj, int depth);
    bool(*_Jsi_IsReserved)(Jsi_Interp *interp, const char* str, bool sql);
    bool(*_Jsi_StrIsAlnum)(const char *cp);
    Jsi_RC(*_Jsi_SqlObjBinds)(Jsi_Interp* interp, Jsi_DString* zStr, Jsi_SqlObjOpts*opts);
    Jsi_RC(*_Jsi_UserObjName)(Jsi_Interp *interp, Jsi_Value *v, Jsi_DString *dStr);
    Jsi_Value*(*_Jsi_ValueNewRegExp)(Jsi_Interp *interp, const char *regtxt, const char* modifiers);
    Jsi_RC(*_Jsi_ObjGetValues)(Jsi_Interp *interp, Jsi_Obj *obj, Jsi_Value *outVal, Jsi_Value *_this);
    Jsi_PkgOpts*(*_Jsi_CommandPkgOpts)(Jsi_Interp *interp, Jsi_Func *func);
    Jsi_Interp*(*_Jsi_InterpMain)(int argc, char **argv, Jsi_InitProc* initProc);
    Jsi_RC(*_Jsi_ObjFreeze)(Jsi_Interp *interp, Jsi_Obj *obj, bool freeze, bool modifyOk, bool readCheck);
    Jsi_Hash*(*_Jsi_ObjAccessor)(Jsi_Interp *interp, Jsi_Obj *obj, const char *name, bool isSet, Jsi_Value* callback, int flags);
    Jsi_Value*(*_Jsi_ValueNewFunction)(Jsi_Interp *interp, Jsi_CmdProc *callback, const char *name, void *privData);
    Jsi_AccessorSpec*(*_Jsi_ObjAccessorWithSpec)(Jsi_Interp *interp, const char* objName, Jsi_OptionSpec *spec, void *dataPtr, Jsi_Value* callback, int flags);
    Jsi_Func*(*_Jsi_FunctionFromValue)(Jsi_Interp *interp, Jsi_Value* value);
    Jsi_RC (*_Jsi_NewVariable)(Jsi_Interp *interp, const char *name, Jsi_Value *val, int flags);
    void *endPtr;
} Jsi_Stubs;

extern Jsi_Stubs* jsiStubsPtr;

#define __JSI_STUBS_INIT__\
    JSI_STUBS_SIG,    "jsi",    sizeof(Jsi_Stubs),     JSI_STUBS_BLDFLAGS,    JSI_STUBS_MD5,    NULL,\
    Jsi_Stubs__initialize,\
    Jsi_InterpNew,\
    Jsi_InterpDelete,\
    Jsi_InterpOnDelete,\
    Jsi_Interactive,\
    Jsi_InterpGone,\
    Jsi_InterpResult,\
    Jsi_InterpLastError,\
    Jsi_InterpGetData,\
    Jsi_InterpSetData,\
    Jsi_InterpFreeData,\
    Jsi_InterpSafe,\
    Jsi_InterpAccess,\
    Jsi_Main,\
    Jsi_Malloc,\
    Jsi_Calloc,\
    Jsi_Realloc,\
    Jsi_Free,\
    Jsi_ObjIncrRefCount,\
    Jsi_ObjDecrRefCount,\
    Jsi_IncrRefCount,\
    Jsi_DecrRefCount,\
    Jsi_IsShared,\
    Jsi_DeleteData,\
    Jsi_Strlen,\
    Jsi_StrlenSet,\
    Jsi_Strcmp,\
    Jsi_Strncmp,\
    Jsi_Strncasecmp,\
    Jsi_StrcmpDict,\
    Jsi_Strcpy,\
    Jsi_Strncpy,\
    Jsi_Strdup,\
    Jsi_Strrchr,\
    Jsi_Strstr,\
    Jsi_ObjArraySizer,\
    Jsi_Strchr,\
    Jsi_Strpos,\
    Jsi_Strrpos,\
    Jsi_DSAppendLen,\
    Jsi_DSAppend,\
    Jsi_DSFree,\
    Jsi_DSFreeDup,\
    Jsi_DSInit,\
    Jsi_DSLength,\
    Jsi_DSPrintf,\
    Jsi_DSSet,\
    Jsi_DSSetLength,\
    Jsi_DSValue,\
    Jsi_CommandCreate,\
    Jsi_CommandCreateSpecs,\
    Jsi_CommandNewObj,\
    Jsi_CommandInvokeJSON,\
    Jsi_CommandInvoke,\
    Jsi_CommandDelete,\
    Jsi_FunctionGetSpecs,\
    Jsi_FunctionIsConstructor,\
    Jsi_FunctionReturnIgnored,\
    Jsi_FunctionPrivData,\
    Jsi_FunctionArguments,\
    Jsi_FunctionApply,\
    Jsi_FunctionInvoke,\
    Jsi_FunctionInvokeJSON,\
    Jsi_FunctionInvokeBool,\
    Jsi_FunctionInvokeString,\
    Jsi_VarLookup,\
    Jsi_NameLookup,\
    Jsi_NameLookup2,\
    Jsi_PkgProvideEx,\
    Jsi_PkgRequireEx,\
    Jsi_PkgVersion,\
    Jsi_NumUtfBytes,\
    Jsi_NumUtfChars,\
    Jsi_UtfGetIndex,\
    Jsi_UtfAtIndex,\
    Jsi_UniCharToUtf,\
    Jsi_UtfToUniChar,\
    Jsi_UtfToUniCharCase,\
    Jsi_UtfDecode,\
    Jsi_UtfEncode,\
    Jsi_UtfSubstr,\
    Jsi_UtfIndexToOffset,\
    Jsi_ObjNew,\
    Jsi_ObjNewType,\
    Jsi_ObjFree,\
    Jsi_ObjNewObj,\
    Jsi_ObjNewArray,\
    Jsi_ObjIsArray,\
    Jsi_ObjSetLength,\
    Jsi_ObjGetLength,\
    Jsi_ObjTypeStr,\
    Jsi_ObjTypeGet,\
    Jsi_ObjListifyArray,\
    Jsi_ObjArraySet,\
    Jsi_ObjArrayAdd,\
    Jsi_ObjInsert,\
    Jsi_ObjFromDS,\
    Jsi_ValueNew,\
    Jsi_ValueNew1,\
    Jsi_ValueFree,\
    Jsi_ValueNewNull,\
    Jsi_ValueNewBoolean,\
    Jsi_ValueNewNumber,\
    Jsi_ValueNewBlob,\
    Jsi_ValueNewString,\
    Jsi_ValueNewStringKey,\
    Jsi_ValueNewStringDup,\
    Jsi_ValueNewArray,\
    Jsi_ValueNewObj,\
    Jsi_GetStringFromValue,\
    Jsi_GetNumberFromValue,\
    Jsi_GetBoolFromValue,\
    Jsi_GetIntFromValue,\
    Jsi_GetLongFromValue,\
    Jsi_GetWideFromValue,\
    Jsi_GetDoubleFromValue,\
    Jsi_GetIntFromValueBase,\
    Jsi_ValueGetBoolean,\
    Jsi_ValueGetNumber,\
    Jsi_ValueIsType,\
    Jsi_ValueIsObjType,\
    Jsi_ValueIsTrue,\
    Jsi_ValueIsFalse,\
    Jsi_ValueIsNumber,\
    Jsi_ValueIsArray,\
    Jsi_ValueIsBoolean,\
    Jsi_ValueIsNull,\
    Jsi_ValueIsUndef,\
    Jsi_ValueIsFunction,\
    Jsi_ValueIsString,\
    Jsi_ValueMakeObject,\
    Jsi_ValueMakeArrayObject,\
    Jsi_ValueMakeNumber,\
    Jsi_ValueMakeBool,\
    Jsi_ValueMakeString,\
    Jsi_ValueMakeStringKey,\
    Jsi_ValueMakeBlob,\
    Jsi_ValueMakeNull,\
    Jsi_ValueMakeUndef,\
    Jsi_ValueMakeDStringObject,\
    Jsi_ValueIsStringKey,\
    Jsi_ValueToString,\
    Jsi_ValueToBool,\
    Jsi_ValueToNumber,\
    Jsi_ValueToNumberInt,\
    Jsi_ValueToObject,\
    Jsi_ValueReset,\
    Jsi_ValueGetDString,\
    Jsi_ValueString,\
    Jsi_ValueBlob,\
    Jsi_ValueGetStringLen,\
    Jsi_ValueStrlen,\
    Jsi_ValueFromDS,\
    Jsi_ValueInstanceOf,\
    Jsi_ValueGetObj,\
    Jsi_ValueTypeGet,\
    Jsi_ValueTypeStr,\
    Jsi_ValueCmp,\
    Jsi_ValueGetIndex,\
    Jsi_ValueArraySort,\
    Jsi_ValueArrayConcat,\
    Jsi_ValueArrayPush,\
    Jsi_ValueArrayPop,\
    Jsi_ValueArrayShift,\
    Jsi_ValueArrayUnshift,\
    Jsi_ValueArrayIndex,\
    Jsi_ValueArrayIndexToStr,\
    Jsi_ValueInsert,\
    Jsi_ValueGetLength,\
    Jsi_ValueObjLookup,\
    Jsi_ValueKeyPresent,\
    Jsi_ValueGetKeys,\
    Jsi_ValueCopy,\
    Jsi_ValueReplace,\
    Jsi_ValueDup2,\
    Jsi_ValueDupJSON,\
    Jsi_ValueMove,\
    Jsi_ValueIsEqual,\
    Jsi_UserObjRegister,\
    Jsi_UserObjUnregister,\
    Jsi_UserObjNew,\
    Jsi_UserObjGetData,\
    Jsi_NumberToString,\
    Jsi_Version,\
    Jsi_ReturnValue,\
    Jsi_Mount,\
    Jsi_Executable,\
    Jsi_RegExpNew,\
    Jsi_RegExpFree,\
    Jsi_RegExpMatch,\
    Jsi_RegExpMatches,\
    Jsi_GlobMatch,\
    Jsi_FileRealpath,\
    Jsi_FileRealpathStr,\
    Jsi_NormalPath,\
    Jsi_ValueNormalPath,\
    Jsi_JSONParse,\
    Jsi_JSONParseFmt,\
    Jsi_JSONQuote,\
    Jsi_EvalString,\
    Jsi_EvalFile,\
    Jsi_EvalCmdJSON,\
    Jsi_EvalZip,\
    Jsi_DictionaryCompare,\
    Jsi_GetBool,\
    Jsi_GetInt,\
    Jsi_GetWide,\
    Jsi_GetDouble,\
    Jsi_FormatString,\
    Jsi_SplitStr,\
    Jsi_Sleep,\
    Jsi_Preserve,\
    Jsi_Release,\
    Jsi_EventuallyFree,\
    Jsi_ShiftArgs,\
    Jsi_StringSplit,\
    Jsi_GetIndex,\
    Jsi_PrototypeGet,\
    Jsi_PrototypeDefine,\
    Jsi_PrototypeObjSet,\
    Jsi_ThisDataSet,\
    Jsi_ThisDataGet,\
    Jsi_FuncObjToString,\
    Jsi_UserObjDataFromVar,\
    Jsi_KeyAdd,\
    Jsi_KeyLookup,\
    Jsi_DatetimeFormat,\
    Jsi_DatetimeParse,\
    Jsi_DateTime,\
    Jsi_Encrypt,\
    Jsi_CryptoHash,\
    Jsi_Base64,\
    Jsi_HexStr,\
    Jsi_Strrstr,\
    Jsi_Crc32,\
    Jsi_NumberIsInfinity,\
    Jsi_NumberIsEqual,\
    Jsi_NumberIsFinite,\
    Jsi_NumberIsInteger,\
    Jsi_NumberIsNaN,\
    Jsi_NumberIsNormal,\
    Jsi_NumberIsSubnormal,\
    Jsi_NumberIsWide,\
    Jsi_NumberInfinity,\
    Jsi_NumberNaN,\
    Jsi_NumberDtoA,\
    Jsi_NumberItoA10,\
    Jsi_NumberUtoA10,\
    Jsi_HashNew,\
    Jsi_HashConf,\
    Jsi_HashDelete,\
    Jsi_HashClear,\
    Jsi_HashSet,\
    Jsi_HashGet,\
    Jsi_HashUnset,\
    Jsi_HashKeyGet,\
    Jsi_HashKeysDump,\
    Jsi_HashValueGet,\
    Jsi_HashValueSet,\
    Jsi_HashEntryFind,\
    Jsi_HashEntryNew,\
    Jsi_HashEntryDelete,\
    Jsi_HashSearchFirst,\
    Jsi_HashSearchNext,\
    Jsi_HashSize,\
    Jsi_TreeNew,\
    Jsi_TreeConf,\
    Jsi_TreeDelete,\
    Jsi_TreeClear,\
    Jsi_TreeObjSetValue,\
    Jsi_TreeObjGetValue,\
    Jsi_TreeValueGet,\
    Jsi_TreeValueSet,\
    Jsi_TreeKeyGet,\
    Jsi_TreeEntryFind,\
    Jsi_TreeEntryNew,\
    Jsi_TreeEntryDelete,\
    Jsi_TreeSearchFirst,\
    Jsi_TreeSearchNext,\
    Jsi_TreeSearchDone,\
    Jsi_TreeWalk,\
    Jsi_TreeSet,\
    Jsi_TreeGet,\
    Jsi_TreeUnset,\
    Jsi_TreeSize,\
    Jsi_TreeFromValue,\
    Jsi_TreeKeysDump,\
    Jsi_ListNew,\
    Jsi_ListConf,\
    Jsi_ListDelete,\
    Jsi_ListClear,\
    Jsi_ListValueGet,\
    Jsi_ListValueSet,\
    Jsi_ListEntryNew,\
    Jsi_ListEntryDelete,\
    Jsi_ListSearchFirst,\
    Jsi_ListSearchNext,\
    Jsi_ListSize,\
    Jsi_ListPush,\
    Jsi_ListPop,\
    Jsi_StackNew,\
    Jsi_StackFree,\
    Jsi_StackSize,\
    Jsi_StackPush,\
    Jsi_StackPop,\
    Jsi_StackPeek,\
    Jsi_StackUnshift,\
    Jsi_StackHead,\
    Jsi_StackFreeElements,\
    Jsi_MapNew,\
    Jsi_MapConf,\
    Jsi_MapDelete,\
    Jsi_MapClear,\
    Jsi_MapSet,\
    Jsi_MapGet,\
    Jsi_MapKeyGet,\
    Jsi_MapKeysDump,\
    Jsi_MapValueGet,\
    Jsi_MapValueSet,\
    Jsi_MapEntryFind,\
    Jsi_MapEntryNew,\
    Jsi_MapEntryDelete,\
    Jsi_MapSearchFirst,\
    Jsi_MapSearchNext,\
    Jsi_MapSearchDone,\
    Jsi_MapSize,\
    Jsi_OptionTypeInfo,\
    Jsi_TypeLookup,\
    Jsi_OptionsProcess,\
    Jsi_OptionsProcessJSON,\
    Jsi_OptionsConf,\
    Jsi_OptionsFree,\
    Jsi_OptionsGet,\
    Jsi_OptionsSet,\
    Jsi_OptionsDump,\
    Jsi_OptionsChanged,\
    Jsi_OptionsValid,\
    Jsi_OptionsFind,\
    Jsi_OptionsCustomPrint,\
    Jsi_OptionCustomBuiltin,\
    Jsi_OptionsDup,\
    Jsi_OptionSpecsCached,\
    Jsi_MutexLock,\
    Jsi_MutexUnlock,\
    Jsi_MutexDelete,\
    Jsi_MutexNew,\
    Jsi_CurrentThread,\
    Jsi_InterpThread,\
    Jsi_LogMsg,\
    Jsi_EventNew,\
    Jsi_EventFree,\
    Jsi_EventProcess,\
    Jsi_JsonInit,\
    Jsi_JsonReset,\
    Jsi_JsonFree,\
    Jsi_JsonParse,\
    Jsi_JsonGetToken,\
    Jsi_JsonGetType,\
    Jsi_JsonTokLen,\
    Jsi_JsonGetTokstr,\
    Jsi_JsonGetTypename,\
    Jsi_JsonGetErrname,\
    Jsi_JsonDump,\
    Jsi_FSRegister,\
    Jsi_FSUnregister,\
    Jsi_FSNameToChannel,\
    Jsi_GetCwd,\
    Jsi_Lstat,\
    Jsi_Stat,\
    Jsi_Access,\
    Jsi_Remove,\
    Jsi_Rename,\
    Jsi_Chdir,\
    Jsi_Open,\
    Jsi_Eof,\
    Jsi_Close,\
    Jsi_Read,\
    Jsi_Write,\
    Jsi_Seek,\
    Jsi_Tell,\
    Jsi_Truncate,\
    Jsi_Rewind,\
    Jsi_Flush,\
    Jsi_Getc,\
    Jsi_Printf,\
    Jsi_Ungetc,\
    Jsi_Gets,\
    Jsi_Puts,\
    Jsi_Scandir,\
    Jsi_SetChannelOption,\
    Jsi_Realpath,\
    Jsi_Readlink,\
    Jsi_GetStdChannel,\
    Jsi_FSNative,\
    Jsi_Link,\
    Jsi_Chmod,\
    Jsi_StubLookup,\
    Jsi_AddAutoFiles,\
    Jsi_DbNew,\
    Jsi_DbHandle,\
    Jsi_DbQuery,\
    Jsi_CDataLookup,\
    Jsi_CDataRegister,\
    Jsi_CDataStructInit,\
    Jsi_DllLookup,\
    Jsi_LoadLibrary,\
    Jsi_CDataStruct,\
    Jsi_StrdupLen,\
    Jsi_FileRead,\
    Jsi_ValueNewStringConst,\
    Jsi_PathNormalize,\
    Jsi_ValueInsertArray,\
    Jsi_IterObjNew,\
    Jsi_IterObjFree,\
    Jsi_IterGetKeys,\
    Jsi_IsReserved,\
    Jsi_StrIsAlnum,\
    Jsi_SqlObjBinds,\
    Jsi_UserObjName,\
    Jsi_ValueNewRegExp,\
    Jsi_ObjGetValues,\
    Jsi_CommandPkgOpts,\
    Jsi_InterpMain,\
    Jsi_ObjFreeze,\
    Jsi_ObjAccessor,\
    Jsi_ValueNewFunction,\
    Jsi_ObjAccessorWithSpec,\
    Jsi_FunctionFromValue,\
    Jsi_NewVariable,\
    NULL

#ifdef JSI_USE_STUBS

#define Jsi_InterpNew(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpNew(n0))
#define Jsi_InterpDelete(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpDelete(n0))
#define Jsi_InterpOnDelete(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpOnDelete(n0,n1,n2))
#define Jsi_Interactive(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Interactive(n0,n1))
#define Jsi_InterpGone(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpGone(n0))
#define Jsi_InterpResult(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpResult(n0))
#define Jsi_InterpLastError(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpLastError(n0,n1,n2))
#define Jsi_InterpGetData(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpGetData(n0,n1,n2))
#define Jsi_InterpSetData(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpSetData(n0,n1,n2,n3))
#define Jsi_InterpFreeData(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpFreeData(n0,n1))
#define Jsi_InterpSafe(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpSafe(n0))
#define Jsi_InterpAccess(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpAccess(n0,n1,n2))
#define Jsi_Main(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_Main(n0))
#define Jsi_Malloc(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_Malloc(n0))
#define Jsi_Calloc(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Calloc(n0,n1))
#define Jsi_Realloc(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Realloc(n0,n1))
#define Jsi_Free(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_Free(n0))
#define Jsi_ObjIncrRefCount(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjIncrRefCount(n0,n1))
#define Jsi_ObjDecrRefCount(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjDecrRefCount(n0,n1))
#define Jsi_IncrRefCount(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_IncrRefCount(n0,n1))
#define Jsi_DecrRefCount(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_DecrRefCount(n0,n1))
#define Jsi_IsShared(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_IsShared(n0,n1))
#define Jsi_DeleteData(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_DeleteData(n0,n1))
#define Jsi_Strlen(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_Strlen(n0))
#define Jsi_StrlenSet(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_StrlenSet(n0,n1))
#define Jsi_Strcmp(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Strcmp(n0,n1))
#define Jsi_Strncmp(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Strncmp(n0,n1,n2))
#define Jsi_Strncasecmp(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Strncasecmp(n0,n1,n2))
#define Jsi_StrcmpDict(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_StrcmpDict(n0,n1,n2,n3))
#define Jsi_Strcpy(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Strcpy(n0,n1))
#define Jsi_Strncpy(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Strncpy(n0,n1,n2))
#define Jsi_Strdup(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_Strdup(n0))
#define Jsi_Strrchr(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Strrchr(n0,n1))
#define Jsi_Strstr(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Strstr(n0,n1))
#define Jsi_ObjArraySizer(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjArraySizer(n0,n1,n2))
#define Jsi_Strchr(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Strchr(n0,n1))
#define Jsi_Strpos(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Strpos(n0,n1,n2,n3))
#define Jsi_Strrpos(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Strrpos(n0,n1,n2,n3))
#define Jsi_DSAppendLen(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_DSAppendLen(n0,n1,n2))
#define Jsi_DSAppend(n0,n1,...) JSISTUBCALL(jsiStubsPtr, _Jsi_DSAppend(n0,n1,##__VA_ARGS__))
#define Jsi_DSFree(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_DSFree(n0))
#define Jsi_DSFreeDup(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_DSFreeDup(n0))
#define Jsi_DSInit(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_DSInit(n0))
#define Jsi_DSLength(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_DSLength(n0))
#define Jsi_DSPrintf(n0,n1,...) JSISTUBCALL(jsiStubsPtr, _Jsi_DSPrintf(n0,n1,##__VA_ARGS__))
#define Jsi_DSSet(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_DSSet(n0,n1))
#define Jsi_DSSetLength(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_DSSetLength(n0,n1))
#define Jsi_DSValue(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_DSValue(n0))
#define Jsi_CommandCreate(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_CommandCreate(n0,n1,n2,n3))
#define Jsi_CommandCreateSpecs(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_CommandCreateSpecs(n0,n1,n2,n3,n4))
#define Jsi_CommandNewObj(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_CommandNewObj(n0,n1,n2,n3,n4))
#define Jsi_CommandInvokeJSON(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_CommandInvokeJSON(n0,n1,n2,n3))
#define Jsi_CommandInvoke(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_CommandInvoke(n0,n1,n2,n3))
#define Jsi_CommandDelete(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_CommandDelete(n0,n1))
#define Jsi_FunctionGetSpecs(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionGetSpecs(n0))
#define Jsi_FunctionIsConstructor(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionIsConstructor(n0))
#define Jsi_FunctionReturnIgnored(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionReturnIgnored(n0,n1))
#define Jsi_FunctionPrivData(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionPrivData(n0))
#define Jsi_FunctionArguments(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionArguments(n0,n1,n2))
#define Jsi_FunctionApply(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionApply(n0,n1,n2,n3))
#define Jsi_FunctionInvoke(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionInvoke(n0,n1,n2,n3,n4))
#define Jsi_FunctionInvokeJSON(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionInvokeJSON(n0,n1,n2,n3,n4))
#define Jsi_FunctionInvokeBool(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionInvokeBool(n0,n1,n2,n3))
#define Jsi_FunctionInvokeString(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionInvokeString(n0,n1,n2,n3,n4))
#define Jsi_VarLookup(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_VarLookup(n0,n1))
#define Jsi_NameLookup(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_NameLookup(n0,n1))
#define Jsi_NameLookup2(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_NameLookup2(n0,n1,n2))
#define Jsi_PkgProvideEx(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_PkgProvideEx(n0,n1,n2,n3,n4))
#define Jsi_PkgRequireEx(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_PkgRequireEx(n0,n1,n2,n3))
#define Jsi_PkgVersion(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_PkgVersion(n0,n1,n2))
#define Jsi_NumUtfBytes(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumUtfBytes(n0))
#define Jsi_NumUtfChars(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_NumUtfChars(n0,n1))
#define Jsi_UtfGetIndex(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_UtfGetIndex(n0,n1,n2))
#define Jsi_UtfAtIndex(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_UtfAtIndex(n0,n1))
#define Jsi_UniCharToUtf(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_UniCharToUtf(n0,n1))
#define Jsi_UtfToUniChar(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_UtfToUniChar(n0,n1))
#define Jsi_UtfToUniCharCase(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_UtfToUniCharCase(n0,n1,n2))
#define Jsi_UtfDecode(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_UtfDecode(n0,n1))
#define Jsi_UtfEncode(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_UtfEncode(n0,n1))
#define Jsi_UtfSubstr(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_UtfSubstr(n0,n1,n2,n3))
#define Jsi_UtfIndexToOffset(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_UtfIndexToOffset(n0,n1))
#define Jsi_ObjNew(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjNew(n0))
#define Jsi_ObjNewType(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjNewType(n0,n1))
#define Jsi_ObjFree(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjFree(n0,n1))
#define Jsi_ObjNewObj(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjNewObj(n0,n1,n2))
#define Jsi_ObjNewArray(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjNewArray(n0,n1,n2,n3))
#define Jsi_ObjIsArray(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjIsArray(n0,n1))
#define Jsi_ObjSetLength(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjSetLength(n0,n1,n2))
#define Jsi_ObjGetLength(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjGetLength(n0,n1))
#define Jsi_ObjTypeStr(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjTypeStr(n0,n1))
#define Jsi_ObjTypeGet(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjTypeGet(n0))
#define Jsi_ObjListifyArray(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjListifyArray(n0,n1))
#define Jsi_ObjArraySet(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjArraySet(n0,n1,n2,n3))
#define Jsi_ObjArrayAdd(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjArrayAdd(n0,n1,n2))
#define Jsi_ObjInsert(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjInsert(n0,n1,n2,n3,n4))
#define Jsi_ObjFromDS(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjFromDS(n0,n1))
#define Jsi_ValueNew(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNew(n0))
#define Jsi_ValueNew1(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNew1(n0))
#define Jsi_ValueFree(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueFree(n0,n1))
#define Jsi_ValueNewNull(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewNull(n0))
#define Jsi_ValueNewBoolean(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewBoolean(n0,n1))
#define Jsi_ValueNewNumber(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewNumber(n0,n1))
#define Jsi_ValueNewBlob(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewBlob(n0,n1,n2))
#define Jsi_ValueNewString(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewString(n0,n1,n2))
#define Jsi_ValueNewStringKey(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewStringKey(n0,n1))
#define Jsi_ValueNewStringDup(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewStringDup(n0,n1))
#define Jsi_ValueNewArray(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewArray(n0,n1,n2))
#define Jsi_ValueNewObj(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewObj(n0,n1))
#define Jsi_GetStringFromValue(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_GetStringFromValue(n0,n1,n2,n3))
#define Jsi_GetNumberFromValue(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_GetNumberFromValue(n0,n1,n2))
#define Jsi_GetBoolFromValue(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_GetBoolFromValue(n0,n1,n2))
#define Jsi_GetIntFromValue(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_GetIntFromValue(n0,n1,n2))
#define Jsi_GetLongFromValue(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_GetLongFromValue(n0,n1,n2))
#define Jsi_GetWideFromValue(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_GetWideFromValue(n0,n1,n2))
#define Jsi_GetDoubleFromValue(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_GetDoubleFromValue(n0,n1,n2))
#define Jsi_GetIntFromValueBase(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_GetIntFromValueBase(n0,n1,n2,n3,n4))
#define Jsi_ValueGetBoolean(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueGetBoolean(n0,n1,n2))
#define Jsi_ValueGetNumber(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueGetNumber(n0,n1,n2))
#define Jsi_ValueIsType(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsType(n0,n1,n2))
#define Jsi_ValueIsObjType(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsObjType(n0,n1,n2))
#define Jsi_ValueIsTrue(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsTrue(n0,n1))
#define Jsi_ValueIsFalse(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsFalse(n0,n1))
#define Jsi_ValueIsNumber(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsNumber(n0,n1))
#define Jsi_ValueIsArray(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsArray(n0,n1))
#define Jsi_ValueIsBoolean(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsBoolean(n0,n1))
#define Jsi_ValueIsNull(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsNull(n0,n1))
#define Jsi_ValueIsUndef(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsUndef(n0,n1))
#define Jsi_ValueIsFunction(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsFunction(n0,n1))
#define Jsi_ValueIsString(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsString(n0,n1))
#define Jsi_ValueMakeObject(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeObject(n0,n1,n2))
#define Jsi_ValueMakeArrayObject(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeArrayObject(n0,n1,n2))
#define Jsi_ValueMakeNumber(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeNumber(n0,n1,n2))
#define Jsi_ValueMakeBool(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeBool(n0,n1,n2))
#define Jsi_ValueMakeString(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeString(n0,n1,n2))
#define Jsi_ValueMakeStringKey(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeStringKey(n0,n1,n2))
#define Jsi_ValueMakeBlob(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeBlob(n0,n1,n2,n3))
#define Jsi_ValueMakeNull(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeNull(n0,n1))
#define Jsi_ValueMakeUndef(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeUndef(n0,n1))
#define Jsi_ValueMakeDStringObject(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMakeDStringObject(n0,n1,n2))
#define Jsi_ValueIsStringKey(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsStringKey(n0,n1))
#define Jsi_ValueToString(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueToString(n0,n1,n2))
#define Jsi_ValueToBool(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueToBool(n0,n1))
#define Jsi_ValueToNumber(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueToNumber(n0,n1))
#define Jsi_ValueToNumberInt(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueToNumberInt(n0,n1,n2))
#define Jsi_ValueToObject(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueToObject(n0,n1))
#define Jsi_ValueReset(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueReset(n0,n1))
#define Jsi_ValueGetDString(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueGetDString(n0,n1,n2,n3))
#define Jsi_ValueString(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueString(n0,n1,n2))
#define Jsi_ValueBlob(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueBlob(n0,n1,n2))
#define Jsi_ValueGetStringLen(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueGetStringLen(n0,n1,n2))
#define Jsi_ValueStrlen(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueStrlen(n0))
#define Jsi_ValueFromDS(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueFromDS(n0,n1,n2))
#define Jsi_ValueInstanceOf(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueInstanceOf(n0,n1,n2))
#define Jsi_ValueGetObj(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueGetObj(n0,n1))
#define Jsi_ValueTypeGet(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueTypeGet(n0))
#define Jsi_ValueTypeStr(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueTypeStr(n0,n1))
#define Jsi_ValueCmp(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueCmp(n0,n1,n2,n3))
#define Jsi_ValueGetIndex(n0,n1,n2,n3,n4,n5) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueGetIndex(n0,n1,n2,n3,n4,n5))
#define Jsi_ValueArraySort(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueArraySort(n0,n1,n2))
#define Jsi_ValueArrayConcat(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueArrayConcat(n0,n1,n2))
#define Jsi_ValueArrayPush(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueArrayPush(n0,n1,n2))
#define Jsi_ValueArrayPop(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueArrayPop(n0,n1))
#define Jsi_ValueArrayShift(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueArrayShift(n0,n1))
#define Jsi_ValueArrayUnshift(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueArrayUnshift(n0,n1))
#define Jsi_ValueArrayIndex(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueArrayIndex(n0,n1,n2))
#define Jsi_ValueArrayIndexToStr(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueArrayIndexToStr(n0,n1,n2,n3))
#define Jsi_ValueInsert(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueInsert(n0,n1,n2,n3,n4))
#define Jsi_ValueGetLength(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueGetLength(n0,n1))
#define Jsi_ValueObjLookup(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueObjLookup(n0,n1,n2,n3))
#define Jsi_ValueKeyPresent(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueKeyPresent(n0,n1,n2,n3))
#define Jsi_ValueGetKeys(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueGetKeys(n0,n1,n2))
#define Jsi_ValueCopy(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueCopy(n0,n1,n2))
#define Jsi_ValueReplace(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueReplace(n0,n1,n2))
#define Jsi_ValueDup2(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueDup2(n0,n1,n2))
#define Jsi_ValueDupJSON(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueDupJSON(n0,n1))
#define Jsi_ValueMove(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueMove(n0,n1,n2))
#define Jsi_ValueIsEqual(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueIsEqual(n0,n1,n2))
#define Jsi_UserObjRegister(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_UserObjRegister(n0,n1))
#define Jsi_UserObjUnregister(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_UserObjUnregister(n0,n1))
#define Jsi_UserObjNew(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_UserObjNew(n0,n1,n2,n3))
#define Jsi_UserObjGetData(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_UserObjGetData(n0,n1,n2))
#define Jsi_NumberToString(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberToString(n0,n1,n2,n3))
#define Jsi_Version(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_Version(n0))
#define Jsi_ReturnValue(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ReturnValue(n0))
#define Jsi_Mount(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Mount(n0,n1,n2,n3))
#define Jsi_Executable(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_Executable(n0))
#define Jsi_RegExpNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_RegExpNew(n0,n1,n2))
#define Jsi_RegExpFree(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_RegExpFree(n0))
#define Jsi_RegExpMatch(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_RegExpMatch(n0,n1,n2,n3,n4))
#define Jsi_RegExpMatches(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_RegExpMatches(n0,n1,n2,n3,n4))
#define Jsi_GlobMatch(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_GlobMatch(n0,n1,n2))
#define Jsi_FileRealpath(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_FileRealpath(n0,n1,n2))
#define Jsi_FileRealpathStr(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_FileRealpathStr(n0,n1,n2))
#define Jsi_NormalPath(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_NormalPath(n0,n1,n2))
#define Jsi_ValueNormalPath(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNormalPath(n0,n1,n2))
#define Jsi_JSONParse(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_JSONParse(n0,n1,n2,n3))
#define Jsi_JSONParseFmt(n0,n1,n2,...) JSISTUBCALL(jsiStubsPtr, _Jsi_JSONParseFmt(n0,n1,n2,##__VA_ARGS__))
#define Jsi_JSONQuote(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_JSONQuote(n0,n1,n2,n3))
#define Jsi_EvalString(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_EvalString(n0,n1,n2))
#define Jsi_EvalFile(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_EvalFile(n0,n1,n2))
#define Jsi_EvalCmdJSON(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_EvalCmdJSON(n0,n1,n2,n3,n4))
#define Jsi_EvalZip(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_EvalZip(n0,n1,n2,n3))
#define Jsi_DictionaryCompare(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_DictionaryCompare(n0,n1))
#define Jsi_GetBool(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_GetBool(n0,n1,n2))
#define Jsi_GetInt(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_GetInt(n0,n1,n2,n3))
#define Jsi_GetWide(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_GetWide(n0,n1,n2,n3))
#define Jsi_GetDouble(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_GetDouble(n0,n1,n2))
#define Jsi_FormatString(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_FormatString(n0,n1,n2))
#define Jsi_SplitStr(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_SplitStr(n0,n1,n2,n3,n4))
#define Jsi_Sleep(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Sleep(n0,n1))
#define Jsi_Preserve(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Preserve(n0,n1))
#define Jsi_Release(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Release(n0,n1))
#define Jsi_EventuallyFree(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_EventuallyFree(n0,n1,n2))
#define Jsi_ShiftArgs(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ShiftArgs(n0,n1))
#define Jsi_StringSplit(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_StringSplit(n0,n1,n2))
#define Jsi_GetIndex(n0,n1,n2,n3,n4,n5) JSISTUBCALL(jsiStubsPtr, _Jsi_GetIndex(n0,n1,n2,n3,n4,n5))
#define Jsi_PrototypeGet(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_PrototypeGet(n0,n1))
#define Jsi_PrototypeDefine(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_PrototypeDefine(n0,n1,n2))
#define Jsi_PrototypeObjSet(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_PrototypeObjSet(n0,n1,n2))
#define Jsi_ThisDataSet(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ThisDataSet(n0,n1,n2))
#define Jsi_ThisDataGet(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ThisDataGet(n0,n1))
#define Jsi_FuncObjToString(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_FuncObjToString(n0,n1,n2,n3))
#define Jsi_UserObjDataFromVar(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_UserObjDataFromVar(n0,n1))
#define Jsi_KeyAdd(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_KeyAdd(n0,n1))
#define Jsi_KeyLookup(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_KeyLookup(n0,n1))
#define Jsi_DatetimeFormat(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_DatetimeFormat(n0,n1,n2,n3,n4))
#define Jsi_DatetimeParse(n0,n1,n2,n3,n4,n5) JSISTUBCALL(jsiStubsPtr, _Jsi_DatetimeParse(n0,n1,n2,n3,n4,n5))
#define Jsi_DateTime(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_DateTime(n0))
#define Jsi_Encrypt(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_Encrypt(n0,n1,n2,n3,n4))
#define Jsi_CryptoHash(n0,n1,n2,n3,n4,n5,n6) JSISTUBCALL(jsiStubsPtr, _Jsi_CryptoHash(n0,n1,n2,n3,n4,n5,n6))
#define Jsi_Base64(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Base64(n0,n1,n2,n3))
#define Jsi_HexStr(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_HexStr(n0,n1,n2,n3))
#define Jsi_Strrstr(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Strrstr(n0,n1))
#define Jsi_Crc32(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Crc32(n0,n1,n2))
#define Jsi_NumberIsInfinity(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberIsInfinity(n0))
#define Jsi_NumberIsEqual(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberIsEqual(n0,n1))
#define Jsi_NumberIsFinite(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberIsFinite(n0))
#define Jsi_NumberIsInteger(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberIsInteger(n0))
#define Jsi_NumberIsNaN(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberIsNaN(n0))
#define Jsi_NumberIsNormal(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberIsNormal(n0))
#define Jsi_NumberIsSubnormal(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberIsSubnormal(n0))
#define Jsi_NumberIsWide(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberIsWide(n0))
#define Jsi_NumberInfinity(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberInfinity(n0))
#define Jsi_NumberNaN(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberNaN(n0))
#define Jsi_NumberDtoA(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberDtoA(n0,n1,n2,n3,n4))
#define Jsi_NumberItoA10(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberItoA10(n0,n1,n2))
#define Jsi_NumberUtoA10(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_NumberUtoA10(n0,n1,n2))
#define Jsi_HashNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_HashNew(n0,n1,n2))
#define Jsi_HashConf(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_HashConf(n0,n1,n2))
#define Jsi_HashDelete(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_HashDelete(n0))
#define Jsi_HashClear(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_HashClear(n0))
#define Jsi_HashSet(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_HashSet(n0,n1,n2))
#define Jsi_HashGet(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_HashGet(n0,n1,n2))
#define Jsi_HashUnset(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_HashUnset(n0,n1))
#define Jsi_HashKeyGet(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_HashKeyGet(n0))
#define Jsi_HashKeysDump(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_HashKeysDump(n0,n1,n2,n3))
#define Jsi_HashValueGet(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_HashValueGet(n0))
#define Jsi_HashValueSet(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_HashValueSet(n0,n1))
#define Jsi_HashEntryFind(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_HashEntryFind(n0,n1))
#define Jsi_HashEntryNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_HashEntryNew(n0,n1,n2))
#define Jsi_HashEntryDelete(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_HashEntryDelete(n0))
#define Jsi_HashSearchFirst(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_HashSearchFirst(n0,n1))
#define Jsi_HashSearchNext(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_HashSearchNext(n0))
#define Jsi_HashSize(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_HashSize(n0))
#define Jsi_TreeNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeNew(n0,n1,n2))
#define Jsi_TreeConf(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeConf(n0,n1,n2))
#define Jsi_TreeDelete(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeDelete(n0))
#define Jsi_TreeClear(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeClear(n0))
#define Jsi_TreeObjSetValue(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeObjSetValue(n0,n1,n2,n3))
#define Jsi_TreeObjGetValue(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeObjGetValue(n0,n1,n2))
#define Jsi_TreeValueGet(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeValueGet(n0))
#define Jsi_TreeValueSet(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeValueSet(n0,n1))
#define Jsi_TreeKeyGet(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeKeyGet(n0))
#define Jsi_TreeEntryFind(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeEntryFind(n0,n1))
#define Jsi_TreeEntryNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeEntryNew(n0,n1,n2))
#define Jsi_TreeEntryDelete(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeEntryDelete(n0))
#define Jsi_TreeSearchFirst(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeSearchFirst(n0,n1,n2,n3))
#define Jsi_TreeSearchNext(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeSearchNext(n0))
#define Jsi_TreeSearchDone(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeSearchDone(n0))
#define Jsi_TreeWalk(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeWalk(n0,n1,n2,n3))
#define Jsi_TreeSet(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeSet(n0,n1,n2))
#define Jsi_TreeGet(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeGet(n0,n1,n2))
#define Jsi_TreeUnset(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeUnset(n0,n1))
#define Jsi_TreeSize(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeSize(n0))
#define Jsi_TreeFromValue(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeFromValue(n0,n1))
#define Jsi_TreeKeysDump(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_TreeKeysDump(n0,n1,n2,n3))
#define Jsi_ListNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ListNew(n0,n1,n2))
#define Jsi_ListConf(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ListConf(n0,n1,n2))
#define Jsi_ListDelete(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ListDelete(n0))
#define Jsi_ListClear(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ListClear(n0))
#define Jsi_ListValueGet(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ListValueGet(n0))
#define Jsi_ListValueSet(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ListValueSet(n0,n1))
#define Jsi_ListEntryNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ListEntryNew(n0,n1,n2))
#define Jsi_ListEntryDelete(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ListEntryDelete(n0))
#define Jsi_ListSearchFirst(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ListSearchFirst(n0,n1,n2))
#define Jsi_ListSearchNext(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ListSearchNext(n0))
#define Jsi_ListSize(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_ListSize(n0))
#define Jsi_ListPush(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ListPush(n0,n1,n2))
#define Jsi_ListPop(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_ListPop(n0,n1))
#define Jsi_StackNew(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_StackNew(n0))
#define Jsi_StackFree(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_StackFree(n0))
#define Jsi_StackSize(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_StackSize(n0))
#define Jsi_StackPush(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_StackPush(n0,n1))
#define Jsi_StackPop(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_StackPop(n0))
#define Jsi_StackPeek(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_StackPeek(n0))
#define Jsi_StackUnshift(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_StackUnshift(n0))
#define Jsi_StackHead(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_StackHead(n0))
#define Jsi_StackFreeElements(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_StackFreeElements(n0,n1,n2))
#define Jsi_MapNew(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_MapNew(n0,n1,n2,n3))
#define Jsi_MapConf(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_MapConf(n0,n1,n2))
#define Jsi_MapDelete(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_MapDelete(n0))
#define Jsi_MapClear(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_MapClear(n0))
#define Jsi_MapSet(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_MapSet(n0,n1,n2))
#define Jsi_MapGet(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_MapGet(n0,n1,n2))
#define Jsi_MapKeyGet(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_MapKeyGet(n0,n1))
#define Jsi_MapKeysDump(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_MapKeysDump(n0,n1,n2,n3))
#define Jsi_MapValueGet(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_MapValueGet(n0))
#define Jsi_MapValueSet(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_MapValueSet(n0,n1))
#define Jsi_MapEntryFind(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_MapEntryFind(n0,n1))
#define Jsi_MapEntryNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_MapEntryNew(n0,n1,n2))
#define Jsi_MapEntryDelete(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_MapEntryDelete(n0))
#define Jsi_MapSearchFirst(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_MapSearchFirst(n0,n1,n2))
#define Jsi_MapSearchNext(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_MapSearchNext(n0))
#define Jsi_MapSearchDone(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_MapSearchDone(n0))
#define Jsi_MapSize(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_MapSize(n0))
#define Jsi_OptionTypeInfo(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionTypeInfo(n0))
#define Jsi_TypeLookup(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_TypeLookup(n0,n1))
#define Jsi_OptionsProcess(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsProcess(n0,n1,n2,n3,n4))
#define Jsi_OptionsProcessJSON(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsProcessJSON(n0,n1,n2,n3,n4))
#define Jsi_OptionsConf(n0,n1,n2,n3,n4,n5) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsConf(n0,n1,n2,n3,n4,n5))
#define Jsi_OptionsFree(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsFree(n0,n1,n2,n3))
#define Jsi_OptionsGet(n0,n1,n2,n3,n4,n5) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsGet(n0,n1,n2,n3,n4,n5))
#define Jsi_OptionsSet(n0,n1,n2,n3,n4,n5) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsSet(n0,n1,n2,n3,n4,n5))
#define Jsi_OptionsDump(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsDump(n0,n1,n2,n3,n4))
#define Jsi_OptionsChanged(n0,n1,n2,...) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsChanged(n0,n1,n2,##__VA_ARGS__))
#define Jsi_OptionsValid(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsValid(n0,n1))
#define Jsi_OptionsFind(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsFind(n0,n1,n2,n3))
#define Jsi_OptionsCustomPrint(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsCustomPrint(n0,n1,n2,n3,n4))
#define Jsi_OptionCustomBuiltin(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionCustomBuiltin(n0))
#define Jsi_OptionsDup(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionsDup(n0,n1))
#define Jsi_OptionSpecsCached(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_OptionSpecsCached(n0,n1))
#define Jsi_MutexLock(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_MutexLock(n0,n1))
#define Jsi_MutexUnlock(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_MutexUnlock(n0,n1))
#define Jsi_MutexDelete(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_MutexDelete(n0,n1))
#define Jsi_MutexNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_MutexNew(n0,n1,n2))
#define Jsi_CurrentThread(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_CurrentThread(n0))
#define Jsi_InterpThread(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpThread(n0))
#define Jsi_LogMsg(n0,n1,n2,n3,...) JSISTUBCALL(jsiStubsPtr, _Jsi_LogMsg(n0,n1,n2,n3,##__VA_ARGS__))
#define Jsi_EventNew(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_EventNew(n0,n1,n2))
#define Jsi_EventFree(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_EventFree(n0,n1))
#define Jsi_EventProcess(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_EventProcess(n0,n1))
#define Jsi_JsonInit(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonInit(n0,n1,n2))
#define Jsi_JsonReset(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonReset(n0))
#define Jsi_JsonFree(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonFree(n0))
#define Jsi_JsonParse(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonParse(n0,n1))
#define Jsi_JsonGetToken(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonGetToken(n0,n1))
#define Jsi_JsonGetType(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonGetType(n0,n1))
#define Jsi_JsonTokLen(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonTokLen(n0,n1))
#define Jsi_JsonGetTokstr(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonGetTokstr(n0,n1,n2,n3))
#define Jsi_JsonGetTypename(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonGetTypename(n0))
#define Jsi_JsonGetErrname(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonGetErrname(n0))
#define Jsi_JsonDump(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_JsonDump(n0,n1))
#define Jsi_FSRegister(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_FSRegister(n0,n1))
#define Jsi_FSUnregister(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_FSUnregister(n0))
#define Jsi_FSNameToChannel(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_FSNameToChannel(n0,n1))
#define Jsi_GetCwd(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_GetCwd(n0,n1))
#define Jsi_Lstat(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Lstat(n0,n1,n2))
#define Jsi_Stat(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Stat(n0,n1,n2))
#define Jsi_Access(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Access(n0,n1,n2))
#define Jsi_Remove(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Remove(n0,n1,n2))
#define Jsi_Rename(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Rename(n0,n1,n2))
#define Jsi_Chdir(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Chdir(n0,n1))
#define Jsi_Open(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Open(n0,n1,n2))
#define Jsi_Eof(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Eof(n0,n1))
#define Jsi_Close(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Close(n0,n1))
#define Jsi_Read(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Read(n0,n1,n2,n3))
#define Jsi_Write(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Write(n0,n1,n2,n3))
#define Jsi_Seek(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Seek(n0,n1,n2,n3))
#define Jsi_Tell(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Tell(n0,n1))
#define Jsi_Truncate(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Truncate(n0,n1,n2))
#define Jsi_Rewind(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Rewind(n0,n1))
#define Jsi_Flush(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Flush(n0,n1))
#define Jsi_Getc(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_Getc(n0,n1))
#define Jsi_Printf(n0,n1,n2,...) JSISTUBCALL(jsiStubsPtr, _Jsi_Printf(n0,n1,n2,##__VA_ARGS__))
#define Jsi_Ungetc(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Ungetc(n0,n1,n2))
#define Jsi_Gets(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Gets(n0,n1,n2,n3))
#define Jsi_Puts(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Puts(n0,n1,n2,n3))
#define Jsi_Scandir(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_Scandir(n0,n1,n2,n3,n4))
#define Jsi_SetChannelOption(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_SetChannelOption(n0,n1,n2,n3))
#define Jsi_Realpath(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Realpath(n0,n1,n2))
#define Jsi_Readlink(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Readlink(n0,n1,n2,n3))
#define Jsi_GetStdChannel(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_GetStdChannel(n0,n1))
#define Jsi_FSNative(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_FSNative(n0,n1))
#define Jsi_Link(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_Link(n0,n1,n2,n3))
#define Jsi_Chmod(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_Chmod(n0,n1,n2))
#define Jsi_StubLookup(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_StubLookup(n0,n1,n2))
#define Jsi_AddAutoFiles(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_AddAutoFiles(n0,n1))
#define Jsi_DbNew(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_DbNew(n0,n1))
#define Jsi_DbHandle(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_DbHandle(n0,n1))
#define Jsi_DbQuery(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_DbQuery(n0,n1,n2))
#define Jsi_CDataLookup(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_CDataLookup(n0,n1))
#define Jsi_CDataRegister(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_CDataRegister(n0,n1))
#define Jsi_CDataStructInit(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_CDataStructInit(n0,n1,n2))
#define Jsi_DllLookup(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_DllLookup(n0,n1,n2,n3))
#define Jsi_LoadLibrary(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_LoadLibrary(n0,n1,n2))
#define Jsi_CDataStruct(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_CDataStruct(n0,n1))
#define Jsi_StrdupLen(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_StrdupLen(n0,n1))
#define Jsi_FileRead(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_FileRead(n0,n1,n2))
#define Jsi_ValueNewStringConst(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewStringConst(n0,n1,n2))
#define Jsi_PathNormalize(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_PathNormalize(n0,n1))
#define Jsi_ValueInsertArray(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueInsertArray(n0,n1,n2,n3,n4))
#define Jsi_IterObjNew(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_IterObjNew(n0,n1))
#define Jsi_IterObjFree(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_IterObjFree(n0))
#define Jsi_IterGetKeys(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_IterGetKeys(n0,n1,n2,n3))
#define Jsi_IsReserved(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_IsReserved(n0,n1,n2))
#define Jsi_StrIsAlnum(n0) JSISTUBCALL(jsiStubsPtr, _Jsi_StrIsAlnum(n0))
#define Jsi_SqlObjBinds(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_SqlObjBinds(n0,n1,n2))
#define Jsi_UserObjName(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_UserObjName(n0,n1,n2))
#define Jsi_ValueNewRegExp(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewRegExp(n0,n1,n2))
#define Jsi_ObjGetValues(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjGetValues(n0,n1,n2,n3))
#define Jsi_CommandPkgOpts(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_CommandPkgOpts(n0,n1))
#define Jsi_InterpMain(n0,n1,n2) JSISTUBCALL(jsiStubsPtr, _Jsi_InterpMain(n0,n1,n2))
#define Jsi_ObjFreeze(n0,n1,n2,n3,n4) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjFreeze(n0,n1,n2,n3,n4))
#define Jsi_ObjAccessor(n0,n1,n2,n3,n4,n5) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjAccessor(n0,n1,n2,n3,n4,n5))
#define Jsi_ValueNewFunction(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_ValueNewFunction(n0,n1,n2,n3))
#define Jsi_ObjAccessorWithSpec(n0,n1,n2,n3,n4,n5) JSISTUBCALL(jsiStubsPtr, _Jsi_ObjAccessorWithSpec(n0,n1,n2,n3,n4,n5))
#define Jsi_FunctionFromValue(n0,n1) JSISTUBCALL(jsiStubsPtr, _Jsi_FunctionFromValue(n0,n1))
#define Jsi_NewVariable(n0,n1,n2,n3) JSISTUBCALL(jsiStubsPtr, _Jsi_NewVariable(n0,n1,n2,n3))

#endif

#endif
