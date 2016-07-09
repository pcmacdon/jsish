#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

static struct { const char *name; const char *type; } optTypeInfo[JSI_OPTION_END+1] = {
    {"NONE",""}, {"BOOL","char"}, {"INT","int"}, {"WIDE", "Jsi_Wide"},
    {"BYTE", "Jsi_BYTE"}, {"WORD", "Jsi_WORD"}, {"DWORD", "Jsi_DWORD"}, {"QWORD","Jsi_QWORD"},
    {"DOUBLE", "Jsi_Number"}, {"STRING", "Jsi_Value*"}, {"DSTRING", "Jsi_DString"},
    {"STRKEY","const char*"}, {"STRBUF","Jsi_Strbuf"},
    {"VALUE", "Jsi_Value*"}, {"VAR","Jsi_Value*"}, {"OBJ","Jsi_Obj*"}, {"ARRAY", "Jsi_Value*"},
    {"FUNC", "Jsi_Value*"}, {"DATETIME","Jsi_Number"}, {"DATE","Jsi_Number"},
    {"TIME","Jsi_Number"}, {"TIMESTAMP","time_t"}, {"CUSTOM",""},
    {NULL}
};

int Jsi_OptionsValid(Jsi_Interp *interp,  Jsi_OptionSpec* spec)
{
    int i = 0;
    while (spec[i].type>JSI_OPTION_NONE && spec[i].type < JSI_OPTION_END) {
        if (spec[i].help && strchr(spec[i].help, '\n')) {
            if (interp)
                Jsi_LogError("item \"%s\": help contains newline", spec[i].name);
            return 0;
        }
        i++;
    }
    return 1;
}

#ifndef JSI_LITE_ONLY

static void DumpOptionSpec(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_OptionSpec* spec, int addName);

static int DeleteSpecCacheTable(Jsi_Interp *interp, void *clientData)
{
  Jsi_Hash *tablePtr = (Jsi_Hash *) clientData;
  Jsi_HashEntry *entryPtr;
  Jsi_HashSearch search;

  for (entryPtr = Jsi_HashEntryFirst(tablePtr,&search); entryPtr != NULL;
      entryPtr = Jsi_HashEntryNext(&search)) {

    Jsi_Free(Jsi_HashValueGet(entryPtr));
  }
  Jsi_HashDelete(tablePtr);
  return JSI_OK;
}

static Jsi_OptionSpec * GetCachedOptionSpecs(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs)
{
    Jsi_OptionSpec *cachedSpecs;
    Jsi_Hash *specCacheTablePtr;
    Jsi_HashEntry *entryPtr;
    int isNew;

    specCacheTablePtr = (Jsi_Hash*)Jsi_InterpGetData(interp, "jsi:OptionSpec", NULL);
    if (specCacheTablePtr == NULL) {
        specCacheTablePtr = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, 0);
        Jsi_InterpSetData(interp, "jsi:OptionSpec",
                         DeleteSpecCacheTable, specCacheTablePtr);
    }
    
    entryPtr = Jsi_HashEntryNew(specCacheTablePtr, (char *) staticSpecs, &isNew);
    if (isNew) {
        unsigned int entrySpace = sizeof(Jsi_OptionSpec);
        const Jsi_OptionSpec *staticSpecPtr;

        for (staticSpecPtr=staticSpecs; staticSpecPtr->type>=JSI_OPTION_NONE && staticSpecPtr->type!=JSI_OPTION_END; staticSpecPtr++)
            entrySpace += sizeof(Jsi_OptionSpec);
        assert(staticSpecPtr->type==JSI_OPTION_END);

        cachedSpecs = (Jsi_OptionSpec *) Jsi_Malloc(entrySpace);
        memcpy((void *) cachedSpecs, (void *) staticSpecs, entrySpace);
        Jsi_HashValueSet(entryPtr, cachedSpecs);

    } else {
        cachedSpecs = (Jsi_OptionSpec *) Jsi_HashValueGet(entryPtr);
    }

    return cachedSpecs;
}

const Jsi_OptionSpec *
Jsi_GetCachedOptionSpecs(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs)
{
#ifdef NO_CACHED_SPECS
    return (Jsi_OptionSpec*)staticSpecs;
#else
    /* If we aren't master interp, need to cache due to init and modified flags if Jsi_OptionsChanged is called. */
    if (interp->mainInterp == NULL) {
        interp->mainInterp = interp;
    }
    if (interp == interp->mainInterp) {
        return staticSpecs;
    }
    return GetCachedOptionSpecs(interp, staticSpecs);
#endif
}

/**********************************/

static Jsi_OptionSpec *
FindOptionSpec(Jsi_Interp *interp, Jsi_OptionSpec *specs, const char *name, int flags)
{
    Jsi_OptionSpec *specPtr;
    char c;      /* First character of current argument. */
    Jsi_OptionSpec *matchPtr; /* Matching spec, or NULL. */
    size_t length;

    
    if (name == NULL) {
        Jsi_LogError("Null name for option");
        return NULL;
    }
    c = name[0];
    length = strlen(name);
    matchPtr = NULL;
    for (specPtr = specs; specPtr->type>JSI_OPTION_NONE && specPtr->type < JSI_OPTION_END && specPtr->name; specPtr++) {

        if ((specPtr->name[0] != c)
                || (strncmp(specPtr->name, name, length) != 0)) {
            continue;
        }
        if (specPtr->name[length] == 0) {
            return specPtr;   /* Stop on a perfect match. */
        }
        if (matchPtr != NULL) {
            Jsi_LogError("ambiguous option \"%s\"", name);
            return (Jsi_OptionSpec *) NULL;
        }
        matchPtr = specPtr;
    }

    if (matchPtr == NULL) {
        Jsi_DString dStr = {};
        Jsi_DSAppend(&dStr, "unknown option \"", name, "\" not one of: ", NULL);
    
        for (specPtr = specs; specPtr->type>JSI_OPTION_NONE && specPtr->type < JSI_OPTION_END && specPtr->name; specPtr++) {
            if (specPtr->name == NULL) {
                break;
            }
            if (name[0] != '?' || specPtr->type < 0 || specPtr->type >= JSI_OPTION_END) {
                Jsi_DSAppend(&dStr, specPtr->name, " ", NULL);
            } else {
                if (optTypeInfo[JSI_OPTION_END].name != 0) {
                    fprintf(stderr, "Jsi_OptionTypes out of sync with enum\n");
                    continue;
                }
                Jsi_DSAppend(&dStr, "?", specPtr->name, " <", optTypeInfo[specPtr->type].name, ">? ", NULL);
            }
        }
        assert(specPtr->type == JSI_OPTION_END);
        Jsi_LogError("%s", Jsi_DSValue(&dStr));
        Jsi_DSFree(&dStr);
        return (Jsi_OptionSpec *) NULL;
    }
    return matchPtr;
}


static int
SetOption(Jsi_Interp *interp, Jsi_OptionSpec *specPtr, const char *string /*UNUSED*/, void* rec, Jsi_Value *argValue)
{
    char *ptr;
    int n, flags = specPtr->flags;
    Jsi_Wide wcount = 0;
    char *record = (char*)rec;

    ptr = (char *)record + specPtr->offset;
    if (specPtr->type<=JSI_OPTION_NONE || specPtr->type>=JSI_OPTION_END) {
        Jsi_LogError("unknown option type \"%d\" for \"%s\"", specPtr->type, specPtr->name);
        return JSI_ERROR;
    }
    if (specPtr->custom) {
        Jsi_OptionCustom* cust = Jsi_OptionCustomBuiltin(specPtr->custom);
        if (cust && cust->parseProc) {
            if ((*cust->parseProc)(interp, specPtr, argValue, NULL, record) != JSI_OK)
                return JSI_ERROR;
        } else {
            Jsi_LogError("missing or bad custom for \"%s\"", specPtr->name);
            return JSI_ERROR;
        }
        goto done;
    }

    switch (specPtr->type) {
    case JSI_OPTION_CUSTOM:
    case JSI_OPTION_NONE:
    case JSI_OPTION_END:
        break;

    case JSI_OPTION_BOOL: {
        if (!argValue)
            *(char*)ptr = 0;
        else if (Jsi_GetBoolFromValue(interp, argValue, &n) != JSI_OK)
            return JSI_ERROR;
        else
            *(char*)ptr = n;
        break;
    }

    case JSI_OPTION_INT:
        if (!argValue)
            *(int*)ptr = 0;
        else if (Jsi_GetIntFromValue(interp, argValue, (int *)ptr) != JSI_OK) {
            return JSI_ERROR;
        }
        break;
        
    case JSI_OPTION_WIDE:
        if (argValue) {
            if (Jsi_GetWideFromValue(interp, argValue, &wcount) != JSI_OK) {
                return JSI_ERROR;
            }
        }
        *((int *)ptr) = (int)wcount;
        break;

    case JSI_OPTION_BYTE:
        if (argValue) {
            if (Jsi_GetWideFromValue(interp, argValue, &wcount) != JSI_OK) {
                return JSI_ERROR;
            }
        }
        *((Jsi_BYTE *)ptr) = (Jsi_BYTE)wcount;
        break;
    case JSI_OPTION_WORD:
        if (argValue) {
            if (Jsi_GetWideFromValue(interp, argValue, &wcount) != JSI_OK) {
                return JSI_ERROR;
            }
        }
        *((Jsi_WORD *)ptr) = (Jsi_WORD)wcount;
        break;
    case JSI_OPTION_DWORD:
        if (argValue) {
            if (Jsi_GetWideFromValue(interp, argValue, &wcount) != JSI_OK) {
                return JSI_ERROR;
            }
        }
        *((Jsi_DWORD *)ptr) = (Jsi_DWORD)wcount;
        break;
    case JSI_OPTION_QWORD:
        if (argValue) {
            if (Jsi_GetWideFromValue(interp, argValue, &wcount) != JSI_OK) {
                return JSI_ERROR;
            }
        }
        *((Jsi_QWORD *)ptr) = (Jsi_QWORD)wcount;
        break;

    case JSI_OPTION_DOUBLE:
        if (!argValue)
            *(Jsi_Number*)ptr = 0;
        else if (Jsi_GetDoubleFromValue(interp, argValue, (Jsi_Number *)ptr) != JSI_OK)
            return JSI_ERROR;
        break;
    case JSI_OPTION_STRKEY:
    {
        if (!argValue)
            *(const char**)ptr = NULL;
        else {
            const char *scp;
            if (Jsi_GetStringFromValue(interp, argValue, &scp) != JSI_OK) {
                return JSI_ERROR;
            }
            *(const char**)ptr = Jsi_KeyAdd(interp,scp);
        }
    }
    break;
    case JSI_OPTION_STRBUF:
    {
        if (!argValue)
            *(char*)ptr = 0;
        else {
            const char *scp;
            if (Jsi_GetStringFromValue(interp, argValue, &scp) != JSI_OK) {
                return JSI_ERROR;
            }
            strncpy((char*)ptr, scp, specPtr->size);
            ((char*)ptr)[specPtr->size-1] = 0;
        }
    }
    break;
#define _JSI_OPT_ARGSET(argValue, ptr) \
    if (!(flags&JSI_OPT_NO_DUPVALUE)) {\
        Jsi_IncrRefCount(interp, argValue); \
        if (*((Jsi_Value **)ptr)) Jsi_DecrRefCount(interp, *((Jsi_Value **)ptr)); \
    }\
    *((Jsi_Value **)ptr) = argValue;
    
    case JSI_OPTION_STRING:
        if (!argValue)
            *(Jsi_Value**)ptr = NULL;
        else {
            if (!Jsi_ValueIsString(interp, argValue)) {
                Jsi_LogError("expected a string");
                return JSI_ERROR;
            }
            _JSI_OPT_ARGSET(argValue, ptr);
        }
        break;
        
    case JSI_OPTION_DSTRING:
        Jsi_DSInit((Jsi_DString *)ptr);
        if (argValue)
        {
            const char *scp;
            if (Jsi_GetStringFromValue(interp, argValue, &scp) != JSI_OK) {
                return JSI_ERROR;
            }
            Jsi_DSAppend((Jsi_DString *)ptr, scp, NULL);
        }
        break;
    
    case JSI_OPTION_DATE:
    case JSI_OPTION_TIME:
    case JSI_OPTION_DATETIME: {
       if (argValue)
        {
            if (Jsi_ValueIsNumber(interp, argValue)) {
                Jsi_GetNumberFromValue(interp, argValue, (Jsi_Number*)ptr);
            } else {
                const char *scp;
                if (Jsi_GetStringFromValue(interp, argValue, &scp) != JSI_OK) {
                    return JSI_ERROR;
                }
                if (JSI_OK != Jsi_DatetimeParse(interp, scp, "", 0, (Jsi_Number*)ptr))
                    return JSI_ERROR;
            }
        } else {
            *(Jsi_Number*)ptr = 0;
        }
        break;
    }
    case JSI_OPTION_TIMESTAMP: {
       if (argValue)
        {
            if (Jsi_ValueIsNumber(interp, argValue)) {
                Jsi_Number num;
                Jsi_GetNumberFromValue(interp, argValue, &num);
                *(time_t*)ptr = (time_t)num;
            } else {
                const char *scp;
                if (Jsi_GetStringFromValue(interp, argValue, &scp) != JSI_OK) {
                    return JSI_ERROR;
                }
                Jsi_Number nval;
                if (JSI_OK != Jsi_DatetimeParse(interp, scp, "", 0, &nval))
                    return JSI_ERROR;
                *(time_t*)ptr = nval/1000LL;
            }
        } else {
            *(time_t*)ptr = 0;
        }
        break;
    }
    
    case JSI_OPTION_VAR:
        if (!argValue)
            *(Jsi_Value**)ptr = NULL;
        else {
            if (argValue->vt != JSI_VT_NULL && argValue->vt != JSI_VT_VARIABLE) {
                Jsi_LogError("expected a var");
                return JSI_ERROR;
            }
            _JSI_OPT_ARGSET(argValue, ptr);
        }
        break;

    case JSI_OPTION_FUNC:
        if (!argValue)
            *(Jsi_Value**)ptr = NULL;
        else {
            if (argValue->vt != JSI_VT_NULL && (argValue->vt != JSI_VT_OBJECT || argValue->d.obj->ot != JSI_OT_FUNCTION)) {
                Jsi_LogError("expected a func value");
                return JSI_ERROR;
            }
            _JSI_OPT_ARGSET(argValue, ptr);
        }
        break;

    case JSI_OPTION_OBJ:
        if (!argValue)
            *(Jsi_Obj**)ptr = NULL;
        else {
            if (argValue->vt != JSI_VT_OBJECT) {
                Jsi_LogError("expected an object");
                return JSI_ERROR;
            }
        }
    case JSI_OPTION_VALUE:
        if (!argValue)
            *(Jsi_Value**)ptr = NULL;
        else {
            _JSI_OPT_ARGSET(argValue, ptr);
        }
        break;
    case JSI_OPTION_ARRAY:
        if (!argValue)
            *(Jsi_Value**)ptr = NULL;
        else {
            if (argValue->vt != JSI_VT_OBJECT || !argValue->d.obj->isarrlist) {
                Jsi_LogError("expected an array");
                return JSI_ERROR;
            }
            _JSI_OPT_ARGSET(argValue, ptr);
        }
        break;

    }
done:
    specPtr->flags |= JSI_OPT_IS_SPECIFIED;
    return JSI_OK;
}

int
Jsi_OptionsSet(Jsi_Interp *interp, Jsi_OptionSpec *specs, const char *option, void* rec, Jsi_Value *argValue, int flags)
{
    char *record = (char*)rec;
    Jsi_OptionSpec *specPtr;
    specs = GetCachedOptionSpecs(interp, specs);
    specPtr = FindOptionSpec(interp, specs, option, flags);
    if (!specPtr)
        return JSI_ERROR;
    return SetOption(interp, specPtr, option, record, argValue);
}

static int
GetOption(Jsi_Interp *interp, Jsi_OptionSpec *specPtr, void* record, Jsi_Value **valuePtr, const char *option)
{
    char *ptr;
    
    if (specPtr == NULL) {
        Jsi_LogError("no such option: %s", option);
        return JSI_ERROR;
    }
    //isNull = ((*string == '\0') && (specPtr->flags & JSI_OPTION_NULL_OK));
    
    ptr = (char *)record + specPtr->offset;
    if (specPtr->type<0 || specPtr->type>=JSI_OPTION_END) {
        Jsi_LogError("no such option: %s", option);
        return JSI_ERROR;
    }
    if (specPtr->custom) {
        Jsi_OptionCustom* cust = Jsi_OptionCustomBuiltin(specPtr->custom);
        if (cust->formatProc)
            return (*cust->formatProc) (interp, specPtr, valuePtr, NULL, record);
    }

    switch (specPtr->type) {
    case JSI_OPTION_NONE:
        Jsi_LogError("invalid option type 0");
        return JSI_ERROR;
        break;
    case JSI_OPTION_END:
        break;
    case JSI_OPTION_BOOL:
        Jsi_ValueMakeBool(interp, valuePtr,*(char*)ptr );
        break;
    case JSI_OPTION_INT:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(int *)ptr));
        break;
    case JSI_OPTION_WIDE:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(Jsi_Wide *)ptr));
        break;
    case JSI_OPTION_BYTE:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(Jsi_BYTE *)ptr));
        break;
    case JSI_OPTION_WORD:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(Jsi_WORD *)ptr));
        break;
    case JSI_OPTION_DWORD:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(Jsi_DWORD *)ptr));
        break;
    case JSI_OPTION_QWORD:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(Jsi_QWORD *)ptr));
        break;
    case JSI_OPTION_DOUBLE:
        Jsi_ValueMakeNumber(interp, valuePtr, *(Jsi_Number *)ptr);
        break;
        
    case JSI_OPTION_DSTRING:
        Jsi_ValueMakeStringDup(interp, valuePtr, Jsi_DSValue((Jsi_DString*)ptr));
        break;
    
    case JSI_OPTION_DATE:
    case JSI_OPTION_TIME:
    case JSI_OPTION_DATETIME: {
        Jsi_DString dStr = {};
        Jsi_DatetimeFormat(interp, *(Jsi_Wide*)ptr, "", 0, &dStr);
        Jsi_ValueMakeStringDup(interp, valuePtr, Jsi_DSValue(&dStr));
        Jsi_DSFree(&dStr);
        break;
    }
    case JSI_OPTION_TIMESTAMP: {
        Jsi_DString dStr = {};
        Jsi_DatetimeFormat(interp, 1000LL* (Jsi_Wide)*(time_t*)ptr, "%Y-%m-%d %H:%M:%S", 0, &dStr);
        Jsi_ValueMakeStringDup(interp, valuePtr, Jsi_DSValue(&dStr));
        Jsi_DSFree(&dStr);
        break;
    }
    case JSI_OPTION_STRBUF:
        if (ptr)
            Jsi_ValueMakeStringDup(interp, valuePtr, ptr);
        else
            Jsi_ValueMakeNull(interp, valuePtr);
        break;

    case JSI_OPTION_STRKEY:
        ptr = *(char **)ptr;
        if (ptr)
            Jsi_ValueMakeStringDup(interp, valuePtr, ptr);
        else
            Jsi_ValueMakeNull(interp, valuePtr);
        break;

    case JSI_OPTION_STRING:
    case JSI_OPTION_VAR:
    case JSI_OPTION_FUNC:
    case JSI_OPTION_OBJ:
    case JSI_OPTION_VALUE:
    case JSI_OPTION_ARRAY:
        if (*(Jsi_Value **)ptr)
            Jsi_ValueReplace(interp, valuePtr, *(Jsi_Value **)ptr);
        else
            Jsi_ValueMakeNull(interp, valuePtr);
        break;

    case JSI_OPTION_CUSTOM:
        break;

    }

    return JSI_OK;
}

Jsi_Value *
Jsi_OptionsCustomPrint(void* clientData, Jsi_Interp *interp, const char *name, void *rec, int offset)
{
    char *record = (char*)rec;
    Jsi_Value *valuePtr;
    valuePtr = *(Jsi_Value **)(record + offset);
    return valuePtr;
}


int
Jsi_OptionsGet(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *rec, const char *option, Jsi_Value** valuePtr, int flags)
{
    char *record = (char*)rec;
    Jsi_OptionSpec *specPtr;

    specPtr = FindOptionSpec(interp, specs, option, flags);
    if (specPtr == NULL || GetOption(interp, specPtr, record, valuePtr, option) != JSI_OK) {
        return JSI_ERROR;
    }

    return JSI_OK;

}

int
Jsi_OptionsDump(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *rec, Jsi_Value **ret, int flags)
{
    char *record = (char*)rec;
    Jsi_OptionSpec *specPtr = specs;
    int len = 0, i = 0, count = 0;
    if (!Jsi_OptionsValid(interp, specs))
        return JSI_ERROR;
    
    while (specPtr->type>JSI_OPTION_NONE && specPtr->type < JSI_OPTION_END && specPtr->name) {
        specPtr++;
        len+=2;
    }
    if (!len)
        return JSI_OK;
    Jsi_Value *rv = Jsi_ValueMakeObject(interp, NULL, NULL);
    Jsi_IncrRefCount(interp, rv);
    Jsi_Obj *obj = rv->d.obj;
    specPtr = specs;
    while (specPtr->type>JSI_OPTION_NONE && specPtr->type < JSI_OPTION_END && specPtr->name) {
        Jsi_Value  *vv = Jsi_ValueNew1(interp);
        if (GetOption(interp, specPtr, record, &vv, NULL) != JSI_OK) {
            Jsi_DecrRefCount(interp, vv);
            Jsi_DecrRefCount(interp, rv);
            return JSI_ERROR;
        }
        if (flags&JSI_OPTS_VERBOSE) {
            // dump: type,value,help,info,init
            Jsi_Value *vrv = Jsi_ValueMakeObject(interp, NULL, NULL);
            Jsi_IncrRefCount(interp, vrv);
            Jsi_Obj *vobj = vrv->d.obj;
            Jsi_ObjInsert(interp, vobj, "value", vv, 0);
            Jsi_DecrRefCount(interp, vv);
            vv = vrv;
            DumpOptionSpec(interp, vobj, specPtr, 0);
        }
        Jsi_ObjInsert(interp, obj, specPtr->name, vv, 0);
        Jsi_DecrRefCount(interp, vv);
        count++;
        i++;
        specPtr++;
    }
    assert(specPtr->type == JSI_OPTION_END);
    Jsi_ValueReplace(interp, ret, rv);
    Jsi_DecrRefCount(interp, rv);
    return JSI_OK;
}

int
Jsi_OptionsProcess(Jsi_Interp *interp, Jsi_OptionSpec *specs,  Jsi_Value *args, void *rec, int flags)
{
    Jsi_OptionSpec *specPtr;
    int count = 0;
    char *record = (char*)rec;
    Jsi_TreeEntry *tPtr;
    Jsi_TreeSearch search;
    Jsi_Obj *to;
    if (!Jsi_OptionsValid(interp, specs))
        return -1;

    if (interp->compat && !(flags&JSI_OPTS_FORCE_STRICT))
        flags |=  JSI_OPTS_IGNORE_EXTRA;
    assert((sizeof(optTypeInfo)/sizeof(optTypeInfo[0]) == (JSI_OPTION_END+1)));
    specs = GetCachedOptionSpecs(interp, specs);
    
    if (args == NULL || args->vt == JSI_VT_NULL) {
        for (specPtr = specs; specPtr->type>JSI_OPTION_NONE && specPtr->type < JSI_OPTION_END && specPtr->name; specPtr++) {
            specPtr->flags &= ~JSI_OPT_IS_SPECIFIED;
            if (SetOption(interp, specPtr, (char*)specPtr->name, record, NULL) != JSI_OK)
                return -1;
        }
        assert(specPtr->type == JSI_OPTION_END);
        return 0;
    }
    if (args->vt != JSI_VT_OBJECT || args->d.obj->ot != JSI_OT_OBJECT || args->d.obj->arr) {
        Jsi_LogError("expected object");
        return -1;
    }
    to = args->d.obj;

    if ((JSI_OPTS_IS_UPDATE&flags)==0) {
        for (specPtr = specs; specPtr->type>JSI_OPTION_NONE && specPtr->type < JSI_OPTION_END && specPtr->name; specPtr++) {
            specPtr->flags &= ~JSI_OPT_IS_SPECIFIED;
        }
        assert(specPtr->type == JSI_OPTION_END);
    }
        
    for (tPtr = Jsi_TreeEntryFirst(to->tree, &search, 0);
        tPtr != NULL; tPtr = Jsi_TreeEntryNext(&search)) {
        
        const char *arg;
        Jsi_Value *optval;
        count++;
        arg =(char*) Jsi_TreeKeyGet(tPtr);
        optval = (Jsi_Value*)Jsi_TreeValueGet(tPtr);

        specPtr = FindOptionSpec(interp, specs, arg, flags);
        if (specPtr == NULL) {
            if (flags&JSI_OPTS_IGNORE_EXTRA)
                continue;
            count = -1;
            goto done;
        }

        if ((JSI_OPT_READ_ONLY&specPtr->flags)) {
            Jsi_LogWarn("Error option is readonly: \"%.40s\"", specPtr->name);
            count = -1;
            goto done;
        }
        if ((JSI_OPTS_IS_UPDATE&flags) && (JSI_OPT_INIT_ONLY&specPtr->flags)) {
            Jsi_LogWarn("Error can not update option: \"%.40s\"", specPtr->name);
            count = -1;
            goto done;
        }

        if (SetOption(interp, specPtr, (char*)arg, record, optval) != JSI_OK) {
            Jsi_LogWarn("Error processing option: \"%.40s\"", specPtr->name);
            count = -1;
            goto done;
        }
 
        specPtr->flags |= JSI_OPT_IS_SPECIFIED;
    }
done:
    Jsi_TreeSearchDone(&search);
    return count;

}

int
Jsi_OptionsConf(Jsi_Interp *interp, Jsi_OptionSpec *specs,  Jsi_Value *val, void *rec, Jsi_Value **ret, int flags)
{
    flags |= JSI_OPTS_IS_UPDATE;
    if (!Jsi_OptionsValid(interp, specs))
        return JSI_ERROR;
    
    if (!val)
        return Jsi_OptionsDump(interp, specs, rec, ret, flags);
    if (val->vt == JSI_VT_NULL)
        return Jsi_OptionsDump(interp, specs, rec, ret, flags|JSI_OPTS_VERBOSE);
    if (Jsi_ValueIsString(interp, val)) {
        const char *cp = Jsi_ValueString(interp, val, NULL);
        if (cp && *cp)
            return Jsi_OptionsGet(interp, specs, rec, cp, ret, flags);
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        Jsi_Value *svalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        jsi_DumpOptionSpecs(interp, sobj, specs);
        Jsi_ValueReplace(interp, ret, svalue);
        return JSI_OK;
    }
    if (val->vt != JSI_VT_OBJECT) {
        Jsi_LogError("expected string, object, or null");
        return JSI_ERROR;
    }
    if (Jsi_OptionsProcess(interp, specs, val, rec, JSI_OPTS_IS_UPDATE) < 0)
        return JSI_ERROR;
    return JSI_OK;
}

static void DumpCustomSpec(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_OptionCustom* spec, void *data)
{
    if (spec->help) {
        if (Jsi_Strchr(spec->help, '\n'))
            Jsi_LogError("%s .help contains newline: %s", spec->name, spec->help);
        Jsi_ObjInsert(interp, nobj, "help", Jsi_ValueNewStringKey(interp, spec->help),0);
    }
    if (spec->info)
        Jsi_ObjInsert(interp, nobj, "info", Jsi_ValueNewStringKey(interp, spec->info),0);
    Jsi_ObjInsert(interp, nobj, "name", Jsi_ValueNewStringKey(interp, spec->name),0);
#if 0
    if (spec == &jsi_OptSwitchEnum || spec == &jsi_OptSwitchEnumNocase || spec == &jsi_OptSwitchBitset) {
        char **lst = data;
        int i = 0;
        Jsi_DString dStr = {};
        if (spec == &jsi_OptSwitchBitset)
            Jsi_DSAppend(&dStr, "Zero or more of: [", NULL);
        else
            Jsi_DSAppend(&dStr, "One of: [", NULL);
        while (lst[i]) {
            Jsi_DSAppend(&dStr, (i?" ,":""), lst[i], NULL);
            i++;
        }
        Jsi_DSAppend(&dStr, "].", NULL);
        Jsi_ObjInsert(interp, nobj, "data", Jsi_ValueNewStringDup(interp, Jsi_DSValue(&dStr)),0);
        Jsi_DSFree(&dStr);
   /*
    else if (spec == Jsi_OptSwitchSubopt) {
    } else {
        //TODO: custom dumper?*/
    }
#endif
}

static void DumpOptionSpec(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_OptionSpec* spec, int addName)
{
    if (addName)
        Jsi_ObjInsert(interp, nobj, "name", Jsi_ValueNewStringKey(interp, spec->name),0);
    if (spec->help) {
        if (Jsi_Strchr(spec->help, '\n'))
            Jsi_LogError("%s .help contains newline: %s", spec->name, spec->help);
        Jsi_ObjInsert(interp, nobj, "help", Jsi_ValueNewStringKey(interp, spec->help),0);
    }
    if (spec->info)
        Jsi_ObjInsert(interp, nobj, "info", Jsi_ValueNewStringKey(interp, spec->info),0);
    Jsi_ObjInsert(interp, nobj, "type", Jsi_ValueNewStringKey(interp, Jsi_OptionTypeStr(spec->type,0)),0);
    Jsi_ObjInsert(interp, nobj, "tname", Jsi_ValueNewStringKey(interp, Jsi_OptionTypeStr(spec->type,1)),0);
    if (spec->init)
        Jsi_ObjInsert(interp, nobj, "init", Jsi_ValueNewStringKey(interp, spec->init),0);
    Jsi_ObjInsert(interp, nobj, "initOnly", Jsi_ValueNewBoolean(interp, (spec->flags & JSI_OPT_INIT_ONLY)!=0), 0);
    Jsi_ObjInsert(interp, nobj, "readOnly", Jsi_ValueNewBoolean(interp, (spec->flags & JSI_OPT_READ_ONLY)!=0), 0);
    Jsi_ObjInsert(interp, nobj, "size", Jsi_ValueNewNumber(interp, (Jsi_Number)spec->size), 0);
    if (spec->flags)
        Jsi_ObjInsert(interp, nobj, "flags", Jsi_ValueNewNumber(interp, (Jsi_Number)spec->flags), 0);
    if (spec->data) {
        if (spec->type == JSI_OPTION_CUSTOM && (spec->custom == Jsi_Opt_SwitchBitset ||
            spec->custom == Jsi_Opt_SwitchEnum)) {
            Jsi_ObjInsert(interp, nobj, "data", Jsi_ValueNewArray(interp, (char**)spec->data, -1), 0);
        } else
            Jsi_ObjInsert(interp, nobj, "data", Jsi_ValueNewNumber(interp, (Jsi_Number)(int)spec->data), 0);
    }
    if (spec->type == JSI_OPTION_CUSTOM && spec->custom) {
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
        Jsi_Value *cvalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        DumpCustomSpec(interp, sobj,  Jsi_OptionCustomBuiltin(spec->custom), spec->data);
        Jsi_ObjInsert(interp, nobj, "customArg", cvalue,0);
    }
}

void jsi_DumpOptionSpecs(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_OptionSpec* spec)
{
    int i = 0;
    while (spec[i].type>JSI_OPTION_NONE && spec[i].type < JSI_OPTION_END) {
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
        Jsi_Value *svalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        DumpOptionSpec(interp, sobj, spec+i, 1);
        Jsi_ObjArrayAdd(interp, nobj, svalue);
        i++;
    }
}

Jsi_DbMultipleBind *Jsi_CDataLookup(Jsi_Interp *interp, const char *name)
{
    Jsi_HashEntry* hPtr;
    hPtr = Jsi_HashEntryFind(interp->optionDataHash, name);
    if (!hPtr)
        return NULL;
    return (Jsi_DbMultipleBind*)Jsi_HashValueGet(hPtr);
}

int Jsi_CDataRegister(Jsi_Interp *interp, const char *name, Jsi_OptionSpec *specs, void *data, int numData, int flags)
{
    int isNew;
    Jsi_DbMultipleBind* opts;
    Jsi_HashEntry* hPtr;
    if (data == NULL) {
        hPtr = Jsi_HashEntryFind(interp->optionDataHash, name);
        if (!hPtr) {
            Jsi_LogError("unknown option-data: %s", name);
            return JSI_ERROR;
        }
        opts = (Jsi_DbMultipleBind*)Jsi_HashValueGet(hPtr);
        if (opts)
            Jsi_Free(opts);
        Jsi_HashEntryDelete(hPtr);
        return JSI_OK;
    }
    if (!Jsi_OptionsValid(interp, specs))
        return JSI_ERROR;
    hPtr = Jsi_HashEntryNew(interp->optionDataHash, name, &isNew);
    if (hPtr == NULL ||
            !(opts = (Jsi_DbMultipleBind*) (isNew ? Jsi_Calloc(2, sizeof(*opts)) : Jsi_HashValueGet(hPtr))))
        return JSI_ERROR;
    opts->opts = specs;
    opts->data = data;
    opts->numData = numData;
    opts->flags = flags;
    Jsi_HashValueSet(hPtr, opts);
    return JSI_OK;
}

static int ValueToVerify(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record )
{
    if (inStr)
        return JSI_ERROR;
    Jsi_Value **s = (Jsi_Value**)((char*)record) + spec->offset;
    Jsi_ValueHandlerProc *vfunc = (Jsi_ValueHandlerProc*)spec->data;
    if (!vfunc) {
        Jsi_LogError("custom value spec did not set data: %s", spec->name);
        return JSI_ERROR;
    }
    if (!inValue) {
        if (*s)
            Jsi_DecrRefCount(interp, *s);
        *s = NULL;
        return JSI_OK;
    }
    if (vfunc(interp, inValue) != JSI_OK)
        return JSI_ERROR;
    *s = inValue;
    if (*s)
        Jsi_IncrRefCount(interp, *s);
    return JSI_OK;
}

static int VerifyToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *dStr, void *record)
{
    if (dStr)
        return JSI_ERROR;
    Jsi_Value **s = (Jsi_Value**)(((char*)record) + spec->offset);
    if (*s)
        Jsi_ValueReplace(interp, outValue, *s);
    return JSI_OK;
}

static void VerifyFree(Jsi_Interp *interp, Jsi_OptionSpec* spec, void *ptr)
{
    Jsi_Value **v = (Jsi_Value**)ptr;
    if (v)
        Jsi_DecrRefCount(interp, *v);
}

static Jsi_OptionCustom jsi_OptSwitchValueVerify = {
    .name="value", .parseProc=ValueToVerify, .formatProc=VerifyToValue, .freeProc=VerifyFree, .help="A value"
};
#else

static Jsi_OptionSpec * GetCachedOptionSpecs(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs)
{
    return (Jsi_OptionSpec *)staticSpecs;
}

#endif


/*
 *----------------------------------------------------------------------
 * Given the configuration specifications and one or more option
 * patterns (terminated by a NULL), indicate if any of the matching
 * configuration options has been reset.
 *
 * Results:
 *      Returns count of each options that has changed, 0 otherwise.
 *
 *----------------------------------------------------------------------
 */
int Jsi_OptionsChanged(Jsi_Interp *interp, Jsi_OptionSpec *spec, const char *pattern, ...)
{
    va_list argList;
    Jsi_OptionSpec *specPtr;
    const char *option = pattern;
    int cnt = 0;
    
    va_start(argList, pattern);
    spec = GetCachedOptionSpecs(interp, spec);
    do  {
        for (specPtr = spec; specPtr->type>JSI_OPTION_NONE && specPtr->type < JSI_OPTION_END; specPtr++) {
            if ((Jsi_GlobMatch(option, specPtr->name, 0)) &&
                    (specPtr->flags & JSI_OPT_IS_SPECIFIED)) {
                cnt++;
            }
        }
        assert(specPtr->type == JSI_OPTION_END);

    } while ((option = va_arg(argList, char *)) != NULL);
    va_end(argList);
    return cnt;
}

Jsi_OptionSpec *
Jsi_OptionsDup(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs)
{
    unsigned int entrySpace = sizeof(Jsi_OptionSpec);
    const Jsi_OptionSpec *staticSpecPtr;
    Jsi_OptionSpec *newSpecs;

    for (staticSpecPtr=staticSpecs; staticSpecPtr->type>JSI_OPTION_NONE && staticSpecPtr->type<JSI_OPTION_END && staticSpecPtr->name;
            staticSpecPtr++) {
        entrySpace += sizeof(Jsi_OptionSpec);
    }

    newSpecs = (Jsi_OptionSpec *) Jsi_Malloc(entrySpace);
    memcpy((void *) newSpecs, (void *) staticSpecs, entrySpace);
    return newSpecs;
}

/* Free data items and reset values back to 0. */
void
Jsi_OptionsFree(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *rec, int flags /*unused*/)
{
    Jsi_OptionSpec *specPtr;
    char *record = (char*)rec;
    for (specPtr = specs; specPtr->type>JSI_OPTION_NONE && specPtr->type < JSI_OPTION_END && specPtr->name; specPtr++) {
        char *ptr = record + specPtr->offset;
        if (specPtr->flags&JSI_OPT_NO_CLEAR)
            continue;
        if ((*(char **)ptr != NULL) && specPtr->custom) {
            Jsi_OptionCustom* cust = Jsi_OptionCustomBuiltin(specPtr->custom);
            if (cust->freeProc != NULL) {
                (*cust->freeProc)(interp, specPtr, *(char **)ptr);
                return;
            }
        }

        switch (specPtr->type) {
#ifndef JSI_LITE_ONLY
        case JSI_OPTION_VALUE:
        case JSI_OPTION_STRING:
        case JSI_OPTION_OBJ:
        case JSI_OPTION_ARRAY:
        case JSI_OPTION_FUNC:
        case JSI_OPTION_VAR:
        {
            Jsi_Value **vpp = (Jsi_Value**)ptr, *vPtr = *vpp;
            if (vPtr && (!(specPtr->flags&JSI_OPT_NO_DUPVALUE)))
                Jsi_DecrRefCount(interp, vPtr);
            *vpp = 0;
            break;
        }
#endif
        case JSI_OPTION_STRKEY:
            *(char**)ptr = 0;
            break;
        case JSI_OPTION_STRBUF:
            *(char*)ptr = 0;
            break;
        case JSI_OPTION_DSTRING:
            Jsi_DSFree((Jsi_DString *)ptr);
            break;
        case JSI_OPTION_TIMESTAMP:
            *(time_t*)ptr = 0;
            break;
        case JSI_OPTION_WIDE:
            *(Jsi_Wide*)ptr = 0;
            break;
        case JSI_OPTION_BYTE:
            *(Jsi_BYTE*)ptr = 0;
            break;
        case JSI_OPTION_WORD:
            *(Jsi_WORD*)ptr = 0;
            break;
        case JSI_OPTION_DWORD:
            *(Jsi_DWORD*)ptr = 0;
            break;
        case JSI_OPTION_QWORD:
            *(Jsi_QWORD*)ptr = 0;
            break;
        case JSI_OPTION_BOOL:
            *(char*)ptr = 0;
            break;
        case JSI_OPTION_INT:
            *(int*)ptr = 0;
            break;
        case JSI_OPTION_DATE:
        case JSI_OPTION_TIME:
        case JSI_OPTION_DATETIME:
        case JSI_OPTION_DOUBLE:
            *(Jsi_Wide*)ptr = 0;
            break;
        case JSI_OPTION_CUSTOM:
            break;
        default:
            if (specPtr->size>0)
                memset(ptr, 0, specPtr->size);
            break;
        }
    }
}

const char *Jsi_OptionTypeStr(Jsi_OptionTypes typ, int tname)
{
    if (typ>=0 && typ<JSI_OPTION_END)
        return (tname?optTypeInfo[typ].type:optTypeInfo[typ].name);
    return NULL;
}

static int ValueToEnum(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record )
{
    int n, *s = (int*)(((char*)record) + spec->offset);
    const char **list = (const char **)spec->data;
    int flags = (spec->flags&JSI_OPT_CUST_NOCASE?JSI_CMP_NOCASE:0);
    if (!list) {
        Jsi_LogError("custom enum spec did not set data: %s", spec->name);
        return JSI_ERROR;
    }
    if (inStr) {
        if (JSI_OK != Jsi_GetIndex(interp, (char*)inStr, list, "enum", flags, &n))
            return JSI_ERROR;
        *s = n;
        return JSI_OK;
    }
#ifndef JSI_LITE_ONLY
    if (JSI_OK != Jsi_ValueGetIndex(interp, inValue, list, "enum", flags, &n))
        return JSI_ERROR;
    *s = n;
#endif
    return JSI_OK;
}

static int EnumToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record)
{
    int i, n, *s = (int*)(((char*)record) + spec->offset);
    const char **list = (const char**)spec->data;
    if (!list) {
        Jsi_LogError("custom enum spec did not set data: %s", spec->name);
        return JSI_ERROR;
    }
    if (outStr) {
        n = *s;
        for (i=0; i<n && list[i]; i++) ; /* Look forward til n */
        if (!list[i]) {
            Jsi_LogWarn("enum has unknown value: %d", *s);
            return JSI_ERROR;
        }
        Jsi_DSAppendLen(outStr, list[i], -1);
        return JSI_OK;
    }
#ifndef JSI_LITE_ONLY
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    int rc = EnumToValue(interp, spec, NULL, &dStr, record);
    if (rc == JSI_OK)
        Jsi_ValueMakeStringKey(interp, outValue, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return rc;
#endif
    return JSI_ERROR;
}

static Jsi_OptionCustom jsi_OptSwitchEnum = {
    .name="enum", .parseProc=ValueToEnum, .formatProc=EnumToValue, .freeProc=0, .help="one value from list"
};


static int ValueToBitset(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record )
{
    int i, argc, n, *s = (int*)(((char*)record) + spec->offset);
    char **argv;
    const char **list = (const char**)spec->data;
    int m = 0, flags = (spec->flags&JSI_OPT_CUST_NOCASE?JSI_CMP_NOCASE:0);
    if (!list) {
        Jsi_LogError("custom enum spec did not set data: %s", spec->name);
        return JSI_ERROR;
    }
    if (inStr) {
        if (!inStr) {
            *s = 0;
            return JSI_OK;
        }
        Jsi_DString sStr;
        Jsi_DSInit(&sStr);
        Jsi_SplitStr(inStr, &argc, &argv, "", &sStr);
        
        for (i=0; i<argc; i++) {
            if (JSI_OK != Jsi_GetIndex(interp, argv[i], list, "enum", flags, &n))
                return JSI_ERROR;
            m |= (1<<n);
        }
        Jsi_DSFree(&sStr);
        *s = m;
        return JSI_OK;
    }
#ifndef JSI_LITE_ONLY
    if (!inValue) {
        *s = 0;
        return 0;
    }
    if (!Jsi_ValueIsArray(interp, inValue)) {
        Jsi_LogError("expected array");
        return JSI_ERROR;
    }
    argc = Jsi_ValueGetLength(interp, inValue);
    for (i=0; i<argc; i++) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, inValue, i);
        if (JSI_OK != Jsi_ValueGetIndex(interp, v, list, "bitset", flags, &n)) {
            return JSI_ERROR;
        }
        m |= (1<<n);
    }
    *s = m;
#endif
    return JSI_OK;
}

static int BitsetToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record)
{
    int i, n, cnt = 0, *s = (int*)(((char*)record) + spec->offset);
    const char **list = (const char**)spec->data;
    if (!list) {
        Jsi_LogError("custom enum spec did not set data: %s", spec->name);
        return JSI_ERROR;
    }
            
    if (outStr) {
        n = *s;
        for (i=0; list[i]; i++) {
            if (!(n & (1<<i)))
                continue;
            if (cnt++)
                Jsi_DSAppendLen(outStr, " ", 1);
            Jsi_DSAppendLen(outStr, list[i], -1);
        }
        return JSI_OK;
    }
#ifndef JSI_LITE_ONLY
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    n = *s;
    for (i=0; list[i]; i++) {
        Jsi_Value *v;
        if (!(n&(1<<i))) continue;
        cnt++;
        v = Jsi_ValueMakeStringKey(interp, NULL, list[i]);
        Jsi_ObjArrayAdd(interp, obj, v);
    }
    Jsi_ObjSetLength(interp, obj, cnt);
    Jsi_ValueMakeArrayObject(interp, outValue, obj);
#endif
    return JSI_OK;
}

static Jsi_OptionCustom jsi_OptSwitchBitset = {
    .name="bitset", .parseProc=ValueToBitset, .formatProc=BitsetToValue, .freeProc=0, .help="An int field accessed a bit at a time"
};

/* Scanning function */
static int ValueToSubopt(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record )
{
#ifndef JSI_LITE_ONLY
    if (inStr)
        return JSI_ERROR;
    int flags = 0;
    char *s = ((char*)record) + spec->offset;
    Jsi_OptionSpec *subspec = (Jsi_OptionSpec *)spec->data;
  
    if (spec == subspec) {
        Jsi_LogError("subspec was recursive");
        return JSI_ERROR;
    }
    if (!subspec) {
        Jsi_LogError("custom suboption spec did not set data: %s", spec->name);
        return JSI_ERROR;
    }
    if (inValue && Jsi_ValueIsNull(interp, inValue) == 0 &&
        (Jsi_ValueIsObjType(interp, inValue, JSI_OT_OBJECT)==0 || inValue->d.obj->isarrlist)) {
        Jsi_LogError("expected object");
        return JSI_ERROR;
    }
    return (Jsi_OptionsProcess(interp, subspec, inValue, s, flags)<0 ? JSI_ERROR : JSI_OK);
#else
    return JSI_ERROR;
#endif
}

/* Printing function. */
static int SuboptToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record)
{
#ifndef JSI_LITE_ONLY
    if (outStr)
        return JSI_ERROR;
    char *s = ((char*)record) + spec->offset;
    int flags = 0;
    Jsi_OptionSpec *subspec = (Jsi_OptionSpec *)spec->data;
    if (spec == subspec) {
        Jsi_LogError("recursive subspec not allowed");
        return JSI_ERROR;
    }
    if (!subspec) {
        Jsi_LogError("custom suboption spec did not set data: %s", spec->name);
        return JSI_ERROR;
    }
    return Jsi_OptionsDump(interp, subspec, s, outValue, flags);
#else
    return JSI_ERROR;
#endif
}

static void SuboptFree(Jsi_Interp *interp, Jsi_OptionSpec* spec, void *ptr)
{
    Jsi_OptionSpec *subspec = (Jsi_OptionSpec *)spec->data;
    Jsi_OptionsFree(interp, subspec, ptr, 0);
}

static Jsi_OptionCustom jsi_OptSwitchSuboption = {
    .name="suboption", .parseProc=ValueToSubopt, .formatProc=SuboptToValue, .freeProc=SuboptFree,
};

static int ValueToBitfield(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record )
{
    Jsi_Wide w;
    int m = 0, flags = spec->flags;
    if (!spec->bitop) {
        Jsi_LogError("custom spec did not set bitop: %s", spec->name);
        return JSI_ERROR;
    }
    if (inStr) {
        if ((spec->flags == JSI_OPT_BITFIELD_BOOL)) {
            static const char *list[] = { "false", "true", NULL };
            if (JSI_OK != Jsi_GetIndex(interp, (char*)inStr, list, "boolean", flags, &m))
                return JSI_ERROR;
            w = m;
        } else if (spec->data) {
            const char **list = (const char **)spec->data;
            if (JSI_OK != Jsi_GetIndex(interp, (char*)inStr, list, "enum", flags, &m))
                return JSI_ERROR;
            w = m;
        } else {
            if (JSI_OK != Jsi_GetInt(interp, inStr, &m, 0))
                return JSI_ERROR;
            w = m;
        }
        if (JSI_OK != spec->bitop(interp, spec, record, &w, 1))
            return JSI_ERROR;
        return JSI_OK;
    }
#ifndef JSI_LITE_ONLY
    Jsi_Bool b;
    if ((spec->flags & JSI_OPT_BITFIELD_BOOL)) {
        if (JSI_OK != Jsi_ValueGetBoolean(interp, inValue, &b))
            return JSI_ERROR;
        w = b;
    } else if (spec->data) {
        const char **list = (const char **)spec->data;
        if (JSI_OK != Jsi_ValueGetIndex(interp, inValue, list, "enum", flags, &m))
            return JSI_ERROR;
        w = m;
    } else {
        Jsi_Number n;
        if (JSI_OK != Jsi_ValueGetNumber(interp, inValue, &n))
            return JSI_ERROR;
        w = (Jsi_Wide)n;
    }
    if (JSI_OK != spec->bitop(interp, spec, record, &w, 1))
        return JSI_ERROR;
#endif
    return JSI_OK;
}

static int BitfieldToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record)
{
    Jsi_Wide w;
    //int flags = spec->flags;
    if (!spec->bitop) {
        Jsi_LogError("custom spec did not set bitop: %s", spec->name);
        return JSI_ERROR;
    }
    if (JSI_OK != spec->bitop(interp, spec, record, &w, 0))
        return JSI_ERROR;
    if (outStr) {
        if ((spec->flags && JSI_OPT_BITFIELD_BOOL)) {
            int b = (int)w;
            if (b == 0 || b == 1)
                Jsi_DSAppend(outStr, b?"true":"false", NULL);
            else
                Jsi_LogError("invalid bool");
        } else if (spec->data) {
            const char **list = (const char **)spec->data;
            int i, n = (int)w;
            for (i=0; i<n && list[i]; i++) ; /* Look forward til n */
            if (!list[i]) {
                Jsi_LogError("enum has unknown value: %d", n);
                return JSI_ERROR;
            }
            Jsi_DSAppend(outStr, list[i], NULL);
        } else {
            if (!Jsi_DSPrintf(outStr, "%lld", w))
                return JSI_ERROR;
        }
        return JSI_OK;
    }
#ifndef JSI_LITE_ONLY
    if ((spec->flags&JSI_OPT_BITFIELD_BOOL)) {
        int b = (int)w;
        if (b == 0 || b == 1)
            Jsi_ValueMakeBool(interp, outValue, b);
        else
            Jsi_LogError("invalid bool");
    } else if (spec->data) {
        const char **list = (const char **)spec->data;
        int i, n = (int)w;
        for (i=0; i<n && list[i]; i++) ; /* Look forward til n */
        if (!list[i]) {
            Jsi_LogError("enum has unknown value: %d", n);
            return JSI_ERROR;
        }
        Jsi_DSAppend(outStr, list[i], NULL);
        Jsi_ValueMakeStringKey(interp, outValue, list[i]);
    } else {
        Jsi_ValueMakeNumber(interp, outValue, (Jsi_Number)w);
    }
    return JSI_OK;
#endif
    return JSI_ERROR;
}

static Jsi_OptionCustom jsi_OptSwitchCBitfield = {
    .name="c_bitfield", .parseProc=ValueToBitfield, .formatProc=BitfieldToValue, .freeProc=0, .help="C Bitfield access"
};

static Jsi_OptionCustom* custOpts[] = { NULL,
    &jsi_OptSwitchEnum, 
    &jsi_OptSwitchBitset,
    &jsi_OptSwitchSuboption, 
    &jsi_OptSwitchCBitfield,
#ifndef JSI_LITE_ONLY
    &jsi_OptSwitchValueVerify
#endif
};

Jsi_OptionCustom* Jsi_OptionCustomBuiltin(Jsi_OptionCustom* cust) {
    if ((int)cust>0 && (int)cust<(int)(sizeof(custOpts)/sizeof(custOpts[0])))
        return custOpts[(int)cust];
    return cust;
}
