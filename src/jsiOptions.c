#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#define _JSI_OPNM(nam) .sig=JSI_SIG_TYPEDEF, .id=JSI_OPTION_##nam, .idName=#nam
// Note: these need to be in the same order as JSI_OPT_* in jsi.h 
static Jsi_OptionTypedef jsi_OptTypeInfo[] = {
    {.sig=JSI_SIG_TYPEDEF, .id=JSI_OPTION_END,.idName="NONE", },
    {_JSI_OPNM(BOOL),    .cName="bool",     .size=sizeof(bool),       .fmt=PRIu8,  .xfmt=""  PRIu8,    .sfmt=SCNu8, .help="Boolean value: true or false" },
    {_JSI_OPNM(INT8),    .cName="int8_t",   .size=sizeof(int8_t),     .fmt=PRId8,  .xfmt="#" PRIx8,    .sfmt=SCNd8, .help="An 8-bit integer" },
    {_JSI_OPNM(INT16),   .cName="int16_t",  .size=sizeof(int16_t),    .fmt=PRId16, .xfmt="#" PRIx16,   .sfmt=SCNd16, .help="An 16-bit integer" },
    {_JSI_OPNM(INT32),   .cName="int32_t",  .size=sizeof(int32_t),    .fmt=PRId32, .xfmt="#" PRIx32,   .sfmt=SCNd32, .help="An 32-bit integer" },
    {_JSI_OPNM(INT64),   .cName="int64_t",  .size=sizeof(int64_t),    .fmt=PRId64, .xfmt="#" PRIx64,   .sfmt=SCNd64, .help="An 64-bit integer" },
    {_JSI_OPNM(UINT8),   .cName="uint8_t",  .size=sizeof(uint8_t),    .fmt=PRIu8,  .xfmt="#" PRIx8,    .sfmt=SCNu8, .help="Unsigned 8-bit integer" },
    {_JSI_OPNM(UINT16),  .cName="uint16_t", .size=sizeof(uint16_t),   .fmt=PRIu16, .xfmt="#" PRIx16,   .sfmt=SCNu16, .help="Unsigned 16-bit integer" },
    {_JSI_OPNM(UINT32),  .cName="uint32_t", .size=sizeof(uint32_t),   .fmt=PRIu32, .xfmt="#" PRIx32,   .sfmt=SCNu32, .help="Unsigned 32-bit integer" },
    {_JSI_OPNM(UINT64),  .cName="uint64_t", .size=sizeof(uint64_t),   .fmt=PRIu64, .xfmt="#" PRIx64,   .sfmt=SCNu64, .help="Unsigned 64-bit integer" },
    {_JSI_OPNM(FLOAT),   .cName="float",    .size=sizeof(float),      .fmt="g",    .xfmt="#" "g",      .sfmt="g", .help="Floating point" },
    {_JSI_OPNM(DOUBLE),  .cName="double",   .size=sizeof(double),     .fmt="lg",   .xfmt="#" "lg",     .sfmt="lg", .help="Floating point double" },
    {_JSI_OPNM(LDOUBLE),  .cName="ldouble", .size=sizeof(ldouble),    .fmt="Lg",   .xfmt="#" "Lg",     .sfmt="Lg", .help="Floating point long double" },
    {_JSI_OPNM(STRBUF),  .cName="Jsi_Strbuf",.size=0,                 .fmt="s",    .xfmt="s",    .sfmt=0 /*"s"*/, .help="A fixed size string buffer" },
    {_JSI_OPNM(TIME_W),  .cName="time_w",   .size=sizeof(time_w),     .fmt=PRId64, .xfmt="#" PRIx64,   .sfmt=SCNd64, .help="A time value in milliseconds stored as a 64 bit integer" },
    {_JSI_OPNM(TIME_D),  .cName="time_d",   .size=sizeof(time_d),     .fmt="g",    .xfmt="#" "g",      .sfmt="g", .help="A time value in milliseconds stored as a double" },
    {_JSI_OPNM(TIME_T),  .cName="time_t",   .size=sizeof(time_t),     .fmt="ld",   .xfmt="#lx",  .sfmt="ld", .help="A time value stored in a unix time_t" },
    {_JSI_OPNM(SIZE_T),  .cName="size_t",   .size=sizeof(size_t),     .fmt="zd",   .xfmt="#zx",  .sfmt="z", .help="Size unsigned" },
    {_JSI_OPNM(SSIZE_T), .cName="ssize_t",  .size=sizeof(ssize_t),    .fmt="zu",   .xfmt="#zx",  .sfmt="zu", .help="Size integer" },
    {_JSI_OPNM(INTPTR_T),.cName="intptr_t", .size=sizeof(intptr_t),   .fmt="d",    .xfmt="#x",   .sfmt="d", .help="Integer large enough to store pointer" },
    {_JSI_OPNM(UINTPTR_T),.cName="uintptr_t",.size=sizeof(uintptr_t), .fmt="u",    .xfmt="#x",   .sfmt="u", .help="Unsigned large enough to store pointer" },
    {_JSI_OPNM(NUMBER),  .cName="Jsi_Number",.size=sizeof(Jsi_Number),.fmt=JSI_NUMGFMT,.xfmt="#" JSI_NUMGFMT,.sfmt=JSI_NUMGFMT, .help="Double or long double" },
    {_JSI_OPNM(INT),     .cName="int",      .size=sizeof(int),        .fmt="d",    .xfmt="#x",   .sfmt="d", .help="Integer" },
    {_JSI_OPNM(UINT),    .cName="uint",     .size=sizeof(uint),       .fmt="u",    .xfmt="#x",   .sfmt="u", .help="Unsigned" },
    {_JSI_OPNM(LONG),    .cName="long",     .size=sizeof(long),       .fmt="ld",   .xfmt="#lx",  .sfmt="ld", .help="Long" },
    {_JSI_OPNM(ULONG),   .cName="ulong",    .size=sizeof(ulong),      .fmt="lu",   .xfmt="#lx",  .sfmt="lu", .help="Unsigned long" },
    {_JSI_OPNM(SHORT),   .cName="short",    .size=sizeof(short),      .fmt="hd",   .xfmt="#hx",  .sfmt="hd", .help="Short" },
    {_JSI_OPNM(USHORT),  .cName="ushort",   .size=sizeof(ushort),     .fmt="hu",   .xfmt="#hx",  .sfmt="hu", .help="Unsigned short" },
    {_JSI_OPNM(STRING),  .cName="Jsi_Value*",.size=sizeof(Jsi_Value*),.fmt="s",    .xfmt="s", .sfmt=0, .help="A string Value"},
    {_JSI_OPNM(DSTRING), .cName="Jsi_DString",.size=sizeof(Jsi_DString), .fmt=0,    .xfmt=0, .sfmt=0, .help="A dynamic string"},
    {_JSI_OPNM(STRKEY),  .cName="const char*",.size=sizeof(const char *), .fmt=0,    .xfmt=0, .sfmt=0, .help="Const string"},
    {_JSI_OPNM(VALUE),   .cName="Jsi_Value*",.size=sizeof(Jsi_Value*), .fmt=0,    .xfmt=0, .sfmt=0, .help="A Value"},
    {_JSI_OPNM(VAR),     .cName="Jsi_Value*",.size=sizeof(Jsi_Value*), .fmt=0,    .xfmt=0, .sfmt=0, .help="A Var Value"},
    {_JSI_OPNM(OBJ),     .cName="Jsi_Value*",.size=sizeof(Jsi_Value*), .fmt=0,    .xfmt=0, .sfmt=0, .help="An Object Value"},
    {_JSI_OPNM(ARRAY),   .cName="Jsi_Value*",.size=sizeof(Jsi_Value*), .fmt=0,    .xfmt=0, .sfmt=0, .help="An Array Value"},
    {_JSI_OPNM(REGEXP),  .cName="Jsi_Value*",.size=sizeof(Jsi_Value*), .fmt=0,    .xfmt=0, .sfmt=0, .help="A Regex Value"},
    {_JSI_OPNM(FUNC),    .cName="Jsi_Value*",.size=sizeof(Jsi_Value*), .fmt=0,    .xfmt=0, .sfmt=0, .help="A Func Value"}, 
    {_JSI_OPNM(USEROBJ), .cName="Jsi_Value*",.size=sizeof(Jsi_Value*), .fmt=0,    .xfmt=0, .sfmt=0, .help="A User-define Object Value"}, 
    {_JSI_OPNM(CUSTOM),  .cName="", .size=0, .fmt=0,    .xfmt=0, .sfmt=0, .help="A Custom value"},
    {_JSI_OPNM(END)}
};

const Jsi_OptionTypedef* Jsi_OptionsStr2Type(const char *str, bool cName) {
    int typ;
    for (typ=JSI_OPTION_BOOL; typ < JSI_OPTION_END; typ++) {
        const char *snam = (cName?jsi_OptTypeInfo[typ].cName:jsi_OptTypeInfo[typ].idName);
        if (snam && snam[0] && !Jsi_Strcmp(str, snam))
            return jsi_OptTypeInfo+typ;
    }
    return NULL;
}

const Jsi_OptionTypedef* Jsi_OptionTypeInfo(Jsi_OptionId typ) {
    if (typ>=JSI_OPTION_BOOL && typ < JSI_OPTION_END)
        return jsi_OptTypeInfo+typ;
    return NULL;
}

const char *jsi_OptionTypeStr(Jsi_OptionId typ, bool cName)
{
    const Jsi_OptionTypedef* ti = Jsi_OptionTypeInfo(typ);
    if (ti)
        return (cName?ti->cName:ti->idName);
    return NULL;
}

bool Jsi_OptionsValid(Jsi_Interp *interp,  Jsi_OptionSpec* spec)
{
    int i = 0;
    while (spec[i].id>=JSI_OPTION_BOOL && spec[i].id < JSI_OPTION_END) {
        SIGASSERTMASK(spec+i, OPTS, 0xff);
        if (spec[i].help && Jsi_Strchr(spec[i].help, '\n')) {
            if (interp)
                Jsi_LogError("item \"%s\": help contains newline", spec[i].name);
            return 0;
        }
        i++;
    }
    return (i>0);
}

#ifndef JSI_LITE_ONLY

static void jsi_DumpOptionSpec(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_OptionSpec* spec, int addName);

static Jsi_RC jsi_DeleteSpecCacheTable(Jsi_Interp *interp, void *clientData)
{
  Jsi_Hash *tablePtr = (Jsi_Hash *) clientData;
  Jsi_HashEntry *entryPtr;
  Jsi_HashSearch search;

  for (entryPtr = Jsi_HashSearchFirst(tablePtr,&search); entryPtr != NULL;
      entryPtr = Jsi_HashSearchNext(&search)) {

    Jsi_Free(Jsi_HashValueGet(entryPtr));
  }
  Jsi_HashDelete(tablePtr);
  return JSI_OK;
}

static Jsi_OptionSpec * jsi_GetCachedOptionSpecs(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs)
{
    Jsi_OptionSpec *cachedSpecs;
    Jsi_Hash *specCacheTablePtr;
    Jsi_HashEntry *entryPtr;
    bool isNew;

    specCacheTablePtr = (Jsi_Hash*)Jsi_InterpGetData(interp, "jsi:OptionSpec", NULL);
    if (specCacheTablePtr == NULL) {
        specCacheTablePtr = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, 0);
        Jsi_InterpSetData(interp, "jsi:OptionSpec", specCacheTablePtr, jsi_DeleteSpecCacheTable);
    }
    
    entryPtr = Jsi_HashEntryNew(specCacheTablePtr, (char *) staticSpecs, &isNew);
    if (isNew) {
        unsigned int entrySpace = sizeof(Jsi_OptionSpec);
        const Jsi_OptionSpec *staticSpecPtr;

        for (staticSpecPtr=staticSpecs; staticSpecPtr->id>=JSI_OPTION_BOOL && staticSpecPtr->id!=JSI_OPTION_END; staticSpecPtr++)
            entrySpace += sizeof(Jsi_OptionSpec);
        assert(staticSpecPtr->id==JSI_OPTION_END);

        cachedSpecs = (Jsi_OptionSpec *) Jsi_Malloc(entrySpace);
        memcpy((void *) cachedSpecs, (void *) staticSpecs, entrySpace);
        Jsi_HashValueSet(entryPtr, cachedSpecs);

    } else {
        cachedSpecs = (Jsi_OptionSpec *) Jsi_HashValueGet(entryPtr);
    }

    return cachedSpecs;
}

const Jsi_OptionSpec *
Jsi_OptionSpecsCached(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs)
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
    return jsi_GetCachedOptionSpecs(interp, staticSpecs);
#endif
}

/**********************************/
Jsi_OptionSpec *
Jsi_OptionsFind(Jsi_Interp *interp, Jsi_OptionSpec *specs, const char *name, Jsi_Wide flags)
{
    Jsi_OptionSpec *specPtr;
    char c;      /* First character of current argument. */
    Jsi_OptionSpec *matchPtr; /* Matching spec, or NULL. */
    size_t length;
    const char *matStr = NULL;
    
    if (name == NULL) {
        Jsi_LogError("Null name for option");
        return NULL;
    }
    c = name[0];
    length = Jsi_Strlen(name);
    matchPtr = NULL;
    for (specPtr = specs; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END && specPtr->name; specPtr++) {

        if ((specPtr->name[0] != c)
                || (Jsi_Strncmp(specPtr->name, name, length) != 0)) {
            continue;
        }
        if (specPtr->name[length] == 0) {
            return specPtr;   /* Stop on a perfect match. */
        }
        if (matchPtr != NULL) {
            Jsi_LogError("ambiguous option \"%s\" matches both \"%s\" and \"%s\"", name, matStr, specPtr->name);
            return (Jsi_OptionSpec *) NULL;
        }
        matchPtr = specPtr;
        matStr = specPtr->name;
    }

    if (matchPtr == NULL) {
        Jsi_DString dStr = {};
        Jsi_DSAppend(&dStr, "unknown option \"", name, "\" not one of: ", NULL);
    
        for (specPtr = specs; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END && specPtr->name; specPtr++) {
            if (specPtr->name == NULL) {
                break;
            }
            if (name[0] != '?' || _JSICASTINT(specPtr->id) < 0 || specPtr->id >= JSI_OPTION_END) {
                Jsi_DSAppend(&dStr, specPtr->name, " ", NULL);
            } else {

                Jsi_DSAppend(&dStr, "?", specPtr->name, " <", jsi_OptTypeInfo[specPtr->id].idName, ">? ", NULL);
            }
        }
        assert(specPtr->id == JSI_OPTION_END);
        Jsi_LogError("%s", Jsi_DSValue(&dStr));
        Jsi_DSFree(&dStr);
        return (Jsi_OptionSpec *) NULL;
    }
    return matchPtr;
}


Jsi_RC
jsi_SetOption_(Jsi_Interp *interp, Jsi_OptionSpec *specPtr, const char *string /*UNUSED*/, void* rec, Jsi_Value *argValue, Jsi_Wide flags, bool isSafe)
{
    Jsi_Wide wcount = 0;
    bool bn;
    Jsi_Number nv;
    bool isIncr = (flags & JSI_OPTS_INCR);
    const char *expType = NULL;
    char *record = (char*)rec, *ptr = record + specPtr->offset;
    Jsi_OptionCustom* cust = NULL;
    const char *emsg = NULL, *epre = "";

    if (specPtr->id<JSI_OPTION_BOOL || specPtr->id>=JSI_OPTION_END) 
        return Jsi_LogBug("unknown option id \"%d\" for \"%s\"", specPtr->id, specPtr->name);
    if (specPtr->custom  && specPtr->id == JSI_OPTION_CUSTOM) {
        cust = Jsi_OptionCustomBuiltin(specPtr->custom);
        if (cust && cust->parseProc) {
            int lastErrCnt = interp->logErrorCnt;
            Jsi_OptionSpec *oep = interp->parseMsgSpec;
            interp->parseMsgSpec = specPtr;
            Jsi_RC rc = (*cust->parseProc)(interp, specPtr, argValue, NULL, record, flags);
            if (rc != JSI_OK) {
                if (!interp->csc || lastErrCnt == interp->logErrorCnt)
                    Jsi_LogError("invalid value");
                interp->parseMsgSpec = oep;
                return JSI_ERROR;
            }
            interp->parseMsgSpec = oep;
        } else 
            return Jsi_LogBug("missing or bad custom for \"%s\"", specPtr->name);
        goto done;
    }

    switch (specPtr->id) {
    case JSI_OPTION_CUSTOM:
        if (!specPtr->custom) 
            return Jsi_LogBug("missing or custom for \"%s\"", specPtr->name);

    case JSI_OPTION_BOOL: {
        if (!argValue)
            *(char*)ptr = 0;
        else if (!Jsi_ValueIsBoolean(interp, argValue))
            goto bail;
        Jsi_GetBoolFromValue(interp, argValue, &bn);
        *(char*)ptr = bn;
        break;
    }

    case JSI_OPTION_INT:
    case JSI_OPTION_UINT:
    case JSI_OPTION_LONG:
    case JSI_OPTION_INTPTR_T:
    case JSI_OPTION_UINTPTR_T:
    case JSI_OPTION_SIZE_T:
    case JSI_OPTION_SSIZE_T:
    case JSI_OPTION_ULONG:
    case JSI_OPTION_SHORT:
    case JSI_OPTION_USHORT:
    case JSI_OPTION_UINT64:
    case JSI_OPTION_INT64:
    case JSI_OPTION_INT8:
    case JSI_OPTION_UINT8:
    case JSI_OPTION_INT16:
    case JSI_OPTION_UINT16:
    case JSI_OPTION_INT32:
    case JSI_OPTION_UINT32:
        wcount = 0;
        if (argValue) {
            if (!Jsi_ValueIsNumber(interp, argValue))
                goto bail;

            if (Jsi_GetWideFromValue(interp, argValue, &wcount) != JSI_OK) {
                return JSI_ERROR;
            }
            
        }
        switch (specPtr->id) {
#define _JSI_OPTSETTYP(typ, n, ptr) if (isIncr) n += *((typ *)ptr); \
    if (isIncr || ((Jsi_Wide)(typ)(n)) == (n)) interp->cdataIncrVal = *((typ *)ptr) = (typ)(n); else expType = #typ;
            case JSI_OPTION_INT:    _JSI_OPTSETTYP(int, wcount, ptr); break;
            case JSI_OPTION_UINT:   _JSI_OPTSETTYP(uint, wcount, ptr); break;
            case JSI_OPTION_INTPTR_T: _JSI_OPTSETTYP(intptr_t, wcount, ptr); break;
            case JSI_OPTION_UINTPTR_T:_JSI_OPTSETTYP(uintptr_t, wcount, ptr); break;
            case JSI_OPTION_SIZE_T:   _JSI_OPTSETTYP(size_t, wcount, ptr); break;
            case JSI_OPTION_SSIZE_T:  _JSI_OPTSETTYP(ssize_t, wcount, ptr); break;
            case JSI_OPTION_LONG:   _JSI_OPTSETTYP(long, wcount, ptr); break;
            case JSI_OPTION_ULONG:  _JSI_OPTSETTYP(ulong, wcount, ptr); break;
            case JSI_OPTION_SHORT:  _JSI_OPTSETTYP(short, wcount, ptr); break;
            case JSI_OPTION_USHORT: _JSI_OPTSETTYP(ushort, wcount, ptr); break;
            case JSI_OPTION_INT8:   _JSI_OPTSETTYP(int8_t, wcount, ptr); break;
            case JSI_OPTION_UINT8:  _JSI_OPTSETTYP(uint8_t, wcount, ptr) break;
            case JSI_OPTION_INT16:  _JSI_OPTSETTYP(int16_t, wcount, ptr); break;
            case JSI_OPTION_UINT16: _JSI_OPTSETTYP(uint16_t, wcount, ptr); break;
            case JSI_OPTION_INT32:  _JSI_OPTSETTYP(int32_t, wcount, ptr); break;
            case JSI_OPTION_UINT32: _JSI_OPTSETTYP(uint32_t, wcount, ptr); break;
            case JSI_OPTION_INT64:  _JSI_OPTSETTYP(int64_t, wcount, ptr); break;
            case JSI_OPTION_UINT64: _JSI_OPTSETTYP(uint64_t, wcount, ptr); break; // TODO: might loose top sign bit...
            default: break;
        }
        if (expType)
            return Jsi_LogType("not a %s", expType);
        isIncr = 0;
        break;

    case JSI_OPTION_NUMBER:
    case JSI_OPTION_DOUBLE:
    case JSI_OPTION_LDOUBLE:
    case JSI_OPTION_FLOAT:
        nv = 0;
        if (argValue) {
            if (!Jsi_ValueIsNumber(interp, argValue))
                goto bail;
            if (Jsi_GetNumberFromValue(interp, argValue, &nv) != JSI_OK) {
                return JSI_ERROR;
            }
        }
            
        switch (specPtr->id) {
#define _JSI_OPTSETNTYP(typ, n, ptr) if (!argValue) *(typ*)ptr = 0; else { if (isIncr) n += *((typ *)ptr); \
            interp->cdataIncrVal = *((typ *)ptr) = (typ)(n); \
            if (interp->strict && Jsi_NumberIsNaN((Jsi_Number)(*((typ *)ptr)))) return Jsi_LogError("not a number"); }

            case JSI_OPTION_NUMBER: _JSI_OPTSETNTYP(Jsi_Number, nv, ptr); break;
            case JSI_OPTION_LDOUBLE: _JSI_OPTSETNTYP(ldouble, nv, ptr); break;
            case JSI_OPTION_FLOAT: _JSI_OPTSETNTYP(float, nv, ptr); break;
            case JSI_OPTION_DOUBLE: _JSI_OPTSETNTYP(double, nv, ptr); break;
            default: break;
        }
        isIncr = 0;
        break;
    case JSI_OPTION_STRKEY:
    {
        if (argValue == NULL || Jsi_ValueIsNull(interp, argValue))
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
        if (argValue == NULL || Jsi_ValueIsNull(interp, argValue))
            *(char*)ptr = 0;
        else {
            int sLen;
            const char *scp = Jsi_ValueString(interp, argValue, &sLen);
            if (!scp)
                goto bail;
            if (sLen>(int)(specPtr->size-1)) {
                return Jsi_LogError("String too long");
                //sLen = specPtr->size-1;
            }
            memcpy((char*)ptr, scp, sLen);
            ((char*)ptr)[sLen] = 0;
        }
    }
    break;
    
#define _JSI_OPT_CHECKNULL(argValue) if (!argValue || Jsi_ValueIsNull(interp, argValue)) { \
         if (*((Jsi_Value **)ptr)) \
            Jsi_DecrRefCount(interp, *((Jsi_Value **)ptr)); \
        *((Jsi_Value **)ptr) = NULL; \
        break; \
    }

#define _JSI_OPT_ARGSET(argValue, ptr) \
    if (!(specPtr->flags&JSI_OPT_NO_DUPVALUE)) {\
        Jsi_IncrRefCount(interp, argValue); \
        if (*((Jsi_Value **)ptr)) Jsi_DecrRefCount(interp, *((Jsi_Value **)ptr)); \
    }\
    *((Jsi_Value **)ptr) = argValue;
    
    case JSI_OPTION_STRING:
        if (argValue == *((Jsi_Value **)ptr))
            break;
        _JSI_OPT_CHECKNULL(argValue);
        if (!Jsi_ValueIsString(interp, argValue))
            goto bail;
        _JSI_OPT_ARGSET(argValue, ptr);
        break;
        
    case JSI_OPTION_DSTRING:
        Jsi_DSInit((Jsi_DString *)ptr);
        if (argValue && !Jsi_ValueIsNull(interp, argValue))
        {
            int sLen;
            const char *scp = Jsi_ValueString(interp, argValue, &sLen);
            if (!scp)
                goto bail;
            Jsi_DSAppendLen((Jsi_DString *)ptr, scp, sLen);
        }
        break;
    
    case JSI_OPTION_TIME_D: {
       if (argValue)
        {
            Jsi_Number nv = 0;
            if (Jsi_ValueIsNumber(interp, argValue)) {
                Jsi_GetNumberFromValue(interp, argValue, &nv);
                *(double*)ptr = nv;
            } else {
                const char *scp;
                if (Jsi_GetStringFromValue(interp, argValue, &scp) != JSI_OK) {
                    return JSI_ERROR;
                }
                if (JSI_OK != Jsi_DatetimeParse(interp, scp, "", 0, &nv, false))
                    return JSI_ERROR;
                *(double*)ptr = nv;
            }
        } else {
            *(double*)ptr = 0;
        }
        break;
    }
    case JSI_OPTION_TIME_W: {
       if (argValue)
        {
            if (Jsi_ValueIsNumber(interp, argValue)) {
                Jsi_GetNumberFromValue(interp, argValue, (Jsi_Number*)ptr);
            } else {
                const char *scp;
                Jsi_Number num;
                if (Jsi_GetStringFromValue(interp, argValue, &scp) != JSI_OK) {
                    return JSI_ERROR;
                }
                if (JSI_OK != Jsi_DatetimeParse(interp, scp, "", 0, &num, false))
                    return JSI_ERROR;
                *(Jsi_Wide*)ptr = (Jsi_Wide)num;
            }
        } else {
            *(Jsi_Wide*)ptr = 0;
        }
        break;
    }
    case JSI_OPTION_TIME_T: {
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
                if (JSI_OK != Jsi_DatetimeParse(interp, scp, "", 0, &nval, false))
                    return JSI_ERROR;
                *(time_t*)ptr = nval/1000LL;
            }
        } else {
            *(time_t*)ptr = 0;
        }
        break;
    }
    case JSI_OPTION_VAR:
        _JSI_OPT_CHECKNULL(argValue);
        if (argValue->vt != JSI_VT_NULL && argValue->vt != JSI_VT_VARIABLE) 
            goto bail;
        _JSI_OPT_ARGSET(argValue, ptr);
        break;

    case JSI_OPTION_FUNC:
        _JSI_OPT_CHECKNULL(argValue);
        if (argValue->vt != JSI_VT_OBJECT || argValue->d.obj->ot != JSI_OT_FUNCTION) 
            goto bail;
        if (specPtr->data && (interp->typeCheck.run|interp->typeCheck.all))
            if (!jsi_FuncArgCheck(interp, argValue->d.obj->d.fobj->func, (char*)specPtr->data)) 
                return Jsi_LogError("failed setting func pointer for %s", specPtr->name);

        _JSI_OPT_ARGSET(argValue, ptr);
        break;
        
    case JSI_OPTION_USEROBJ:
        _JSI_OPT_CHECKNULL(argValue);
        if (argValue->vt != JSI_VT_OBJECT || argValue->d.obj->ot != JSI_OT_USEROBJ) 
            goto bail;
        if (specPtr->data && Jsi_Strcmp((char*)specPtr->data, argValue->d.obj->d.uobj->reg->name)) 
            return Jsi_LogError("expected id %s for %s",(char*)specPtr->data,  specPtr->name);
        _JSI_OPT_ARGSET(argValue, ptr);
        break;

    case JSI_OPTION_REGEXP:
        _JSI_OPT_CHECKNULL(argValue);
        if (argValue->vt != JSI_VT_OBJECT || argValue->d.obj->ot != JSI_OT_REGEXP)
            goto bail;
        _JSI_OPT_ARGSET(argValue, ptr);
        break;

    case JSI_OPTION_OBJ:
        _JSI_OPT_CHECKNULL(argValue);
        if (argValue->vt != JSI_VT_OBJECT)
            goto bail;

    case JSI_OPTION_VALUE:
         _JSI_OPT_CHECKNULL(argValue);
         _JSI_OPT_ARGSET(argValue, ptr);
        break;
    case JSI_OPTION_ARRAY:
        _JSI_OPT_CHECKNULL(argValue);
        if (argValue->vt != JSI_VT_OBJECT || !argValue->d.obj->isarrlist)
            goto bail;
        _JSI_OPT_ARGSET(argValue, ptr);
        break;
#ifdef __cplusplus
    case JSI_OPTION_END:
#else
    default:
#endif
        Jsi_LogBug("invalid option id: %d", specPtr->id);
    }
done:
    specPtr->flags |= JSI_OPT_IS_SPECIFIED;
    if (isIncr)
        return Jsi_LogError("incr invalid for %s", specPtr->name);
    return JSI_OK;

bail:
    if (!emsg) {
        emsg = jsi_OptTypeInfo[specPtr->id].cName;
        epre = "expected ";
    }
    return Jsi_LogError("%s%s: for %s option \"%.40s\"", epre, emsg, (cust?cust->name:""), specPtr->name);
}

Jsi_RC
jsi_SetOption(Jsi_Interp *interp, Jsi_OptionSpec *specPtr, const char *string /*UNUSED*/, void* rec, Jsi_Value *argValue, Jsi_Wide flags, bool isSafe)
{
    Jsi_Value *oa = interp->lastParseOpt;
    interp->lastParseOpt = argValue;
    Jsi_RC rc = jsi_SetOption_(interp, specPtr, string, rec, argValue, flags, isSafe);
    interp->lastParseOpt = oa;
    return rc;
}

Jsi_RC
Jsi_OptionsSet(Jsi_Interp *interp, Jsi_OptionSpec *specs, void* rec, const char *option, Jsi_Value *valuePtr, Jsi_Wide flags)
{
    char *record = (char*)rec;
    Jsi_OptionSpec *specPtr;
    specs = jsi_GetCachedOptionSpecs(interp, specs);
    const char *cp = NULL, *cb = NULL;
    bool isSafe = interp->isSafe;
    if (option) {
        cp = Jsi_Strchr(option, '.');
        cb = Jsi_Strchr(option, '[');
    }
    if (cp && (!cb || cp<cb) ) {
        Jsi_DString dStr;
        int len = (cp-option);
        Jsi_DSInit(&dStr);
        cp = Jsi_DSAppendLen(&dStr, option, len);
        specPtr = Jsi_OptionsFind(interp, specs, cp, flags);
        Jsi_DSFree(&dStr);
        if (!specPtr || !specPtr->data|| specPtr->id != JSI_OPTION_CUSTOM || specPtr->custom != Jsi_Opt_SwitchSuboption) 
            return Jsi_LogError("unknown or bad sub-config option: %s", option);
        cp = option+len+1;
        return Jsi_OptionsSet(interp, (Jsi_OptionSpec *)(specPtr->data), (void*)(((char*)rec)+specPtr->offset), cp, valuePtr, flags);
    }
    if (cb && cb != option) {
        char *ce = Jsi_Strchr(option, ']');
        Jsi_Wide ul;
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        int len = 0;
        if (ce && ce>cb) {
            len = (ce-cb-1);
            cp = Jsi_DSAppendLen(&dStr, cb+1, len);
        }
        if (len <= 0 || Jsi_GetWide(interp, cp, &ul, 0) != JSI_OK || ul<0) 
            return Jsi_LogError("bad sub-array option: %s", option);
        len = (cb-option);
        Jsi_DSSetLength(&dStr, 0);
        cp = Jsi_DSAppendLen(&dStr, option, len);
        specPtr = Jsi_OptionsFind(interp, specs, cp, flags);
        Jsi_DSFree(&dStr);
        if (!specPtr || !specPtr->init.OPT_CARRAY|| specPtr->id != JSI_OPTION_CUSTOM || specPtr->custom != Jsi_Opt_SwitchCArray) {
bail:
            return Jsi_LogError("unknown or bad array option: %s", option);
        }
        cp = cb+1;
        Jsi_OptionSpec *subSpec = specPtr->init.OPT_CARRAY;
        int isize, size = specPtr->arrSize;
        if (!subSpec || size<=0 || (isize=subSpec->size)<=0)
            goto bail;
        isize = isize/size;
        uchar *s = (((uchar*)rec)+specPtr->offset + isize*ul);
        if (ce[1] != '.' || !subSpec->data) {
            if (Jsi_OptionsSet(interp, subSpec, (void*)s, subSpec->name, valuePtr, flags) != JSI_OK)
                return JSI_ERROR;
        } else {
            if (Jsi_OptionsSet(interp, (Jsi_OptionSpec *)subSpec->data, (void*)s, ce+2, valuePtr, flags) != JSI_OK)
                return JSI_ERROR;
        }
        return JSI_OK;
    }
    specPtr = Jsi_OptionsFind(interp, specs, option, flags);
    if (!specPtr)
        return JSI_ERROR;
    return jsi_SetOption(interp, specPtr, option, record, valuePtr, flags, isSafe);
}

Jsi_RC
jsi_GetOption(Jsi_Interp *interp, Jsi_OptionSpec *specPtr, void* record, const char *option, Jsi_Value **valuePtr, Jsi_Wide flags)
{
    char *ptr;
    
    if (specPtr == NULL) 
        return Jsi_LogError("no such option: %s", option);
    //isNull = ((*string == '\0') && (specPtr->flags & JSI_OPTION_NULL_OK));
    
    ptr = (char *)record + specPtr->offset;
    if (_JSICASTINT(specPtr->id)<0 || specPtr->id>=JSI_OPTION_END) 
        return Jsi_LogError("no such option: %s", option);
    if (specPtr->custom) {
        Jsi_OptionCustom* cust = Jsi_OptionCustomBuiltin(specPtr->custom);
        if (cust->formatProc)
            return (*cust->formatProc) (interp, specPtr, valuePtr, NULL, record, flags);
    }

    switch (specPtr->id) {
    case JSI_OPTION_BOOL:
        Jsi_ValueMakeBool(interp, valuePtr,*(bool*)ptr );
        break;
    case JSI_OPTION_INT:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(int *)ptr));
        break;
    case JSI_OPTION_UINT:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(uint *)ptr));
        break;
    case JSI_OPTION_INT8:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(int8_t *)ptr));
        break;
    case JSI_OPTION_INT16:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(int16_t *)ptr));
        break;
    case JSI_OPTION_INT32:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(int32_t *)ptr));
        break;
    case JSI_OPTION_INT64:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(Jsi_Wide *)ptr));
        break;
    case JSI_OPTION_UINT8:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(uint8_t *)ptr));
        break;
    case JSI_OPTION_UINT16:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(uint16_t *)ptr));
        break;
    case JSI_OPTION_UINT32:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(uint32_t *)ptr));
        break;
    case JSI_OPTION_UINT64:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)(*(uint64_t *)ptr));
        break;
    case JSI_OPTION_FLOAT:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)*(float *)ptr);
        break;
    case JSI_OPTION_DOUBLE:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)*(double *)ptr);
        break;
    case JSI_OPTION_LDOUBLE:
        Jsi_ValueMakeNumber(interp, valuePtr, (Jsi_Number)*(ldouble *)ptr);
        break;
    case JSI_OPTION_NUMBER:
        Jsi_ValueMakeNumber(interp, valuePtr, *(Jsi_Number *)ptr);
        break;
    case JSI_OPTION_SIZE_T:
        Jsi_ValueMakeNumber(interp, valuePtr, *(size_t *)ptr);
        break;
    case JSI_OPTION_SSIZE_T:
        Jsi_ValueMakeNumber(interp, valuePtr, *(ssize_t *)ptr);
        break;
    case JSI_OPTION_INTPTR_T:
        Jsi_ValueMakeNumber(interp, valuePtr, *(intptr_t *)ptr);
        break;
    case JSI_OPTION_UINTPTR_T:
        Jsi_ValueMakeNumber(interp, valuePtr, *(uintptr_t *)ptr);
        break;
    case JSI_OPTION_LONG:
        Jsi_ValueMakeNumber(interp, valuePtr, *(long *)ptr);
        break;
    case JSI_OPTION_ULONG:
        Jsi_ValueMakeNumber(interp, valuePtr, *(ulong *)ptr);
        break;
    case JSI_OPTION_SHORT:
        Jsi_ValueMakeNumber(interp, valuePtr, *(short *)ptr);
        break;
    case JSI_OPTION_USHORT:
        Jsi_ValueMakeNumber(interp, valuePtr, *(ushort *)ptr);
        break;
        
    case JSI_OPTION_DSTRING:
        Jsi_ValueFromDS(interp, (Jsi_DString*)ptr, valuePtr);
        break;
    
    case JSI_OPTION_TIME_W: {
        Jsi_DString dStr = {};
        Jsi_DatetimeFormat(interp, (Jsi_Number)(*(Jsi_Wide*)ptr), "", 0, &dStr);
        Jsi_ValueFromDS(interp, &dStr, valuePtr);
        break;
    }
    case JSI_OPTION_TIME_D: {
        Jsi_DString dStr = {};
        Jsi_DatetimeFormat(interp, (Jsi_Number)(*(double*)ptr), "", 0, &dStr);
        Jsi_ValueFromDS(interp, &dStr, valuePtr);
        break;
    }
    case JSI_OPTION_TIME_T: {
        Jsi_DString dStr = {};
        Jsi_DatetimeFormat(interp, 1000LL* (Jsi_Number)*(time_t*)ptr, "%Y-%m-%d %H:%M:%S", 0, &dStr);
        Jsi_ValueFromDS(interp, &dStr, valuePtr);
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
    case JSI_OPTION_USEROBJ:
    case JSI_OPTION_OBJ:
    case JSI_OPTION_VALUE:
    case JSI_OPTION_REGEXP:
    case JSI_OPTION_ARRAY:
        if (*(Jsi_Value **)ptr)
            Jsi_ValueCopy(interp, *valuePtr, *(Jsi_Value **)ptr);
        else
            Jsi_ValueMakeNull(interp, valuePtr);
        break;

    case JSI_OPTION_CUSTOM:
        break;

#ifdef __cplusplus
    case JSI_OPTION_END:
#else
    default:
#endif
        Jsi_LogBug("invalid option id %d", specPtr->id);
        return JSI_ERROR;
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


Jsi_RC
Jsi_OptionsGet(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *rec, const char *option, Jsi_Value** valuePtr, Jsi_Wide flags)
{
    char *record = (char*)rec;
    Jsi_OptionSpec *specPtr;
    const char *cp = NULL, *cb = NULL;
    if (option) {
        cp = Jsi_Strchr(option, '.');
        cb = Jsi_Strchr(option, '[');
    }
    if (cp && (!cb || cp<cb) ) {
        Jsi_DString dStr;
        int len = (cp-option);
        Jsi_DSInit(&dStr);
        cp = Jsi_DSAppendLen(&dStr, option, len);
        specPtr = Jsi_OptionsFind(interp, specs, cp, flags);
        Jsi_DSFree(&dStr);
        if (!specPtr || !specPtr->data || specPtr->id != JSI_OPTION_CUSTOM || specPtr->custom != Jsi_Opt_SwitchSuboption) 
            return Jsi_LogError("unknown or bad sub-config option: %s", option);
        cp = option+len+1;
        return Jsi_OptionsGet(interp, (Jsi_OptionSpec *)(specPtr->data), (void*)(((char*)rec)+specPtr->offset), cp, valuePtr, flags);
    }
    if (cb && cb != option) {
        char *ce = Jsi_Strchr(option, ']');
        Jsi_Wide ul;
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        int len = 0;
        if (ce && ce>cb) {
            len = (ce-cb-1);
            cp = Jsi_DSAppendLen(&dStr, cb+1, len);
        }
        if (len <= 0 || Jsi_GetWide(interp, cp, &ul, 0) != JSI_OK || ul<0) 
            return Jsi_LogError("bad sub-array option: %s", option);
        len = (cb-option);
        Jsi_DSSetLength(&dStr, 0);
        cp = Jsi_DSAppendLen(&dStr, option, len);
        specPtr = Jsi_OptionsFind(interp, specs, cp, flags);
        Jsi_DSFree(&dStr);
        if (!specPtr || !specPtr->init.OPT_CARRAY|| specPtr->id != JSI_OPTION_CUSTOM || specPtr->custom != Jsi_Opt_SwitchCArray) {
bail:
            return Jsi_LogError("unknown or bad array option: %s", option);
        }
        cp = cb+1;
        Jsi_OptionSpec *subSpec = specPtr->init.OPT_CARRAY;
        int isize, size = specPtr->arrSize;
        if (!subSpec || size<=0 || (isize=subSpec->size)<=0)
            goto bail;
        isize = isize/size;
        uchar *s = (((uchar*)rec)+specPtr->offset + isize*ul);
        if (ce[1] != '.' || !subSpec->data) {
            if (Jsi_OptionsGet(interp, subSpec, (void*)s, subSpec->name, valuePtr, flags) != JSI_OK)
                return JSI_ERROR;
        } else {
            if (Jsi_OptionsGet(interp, (Jsi_OptionSpec *)subSpec->data, (void*)s, ce+2, valuePtr, flags) != JSI_OK)
                return JSI_ERROR;
        }
        return JSI_OK;
    }

    specPtr = Jsi_OptionsFind(interp, specs, option, flags);
    if (specPtr == NULL || jsi_GetOption(interp, specPtr, record, option, valuePtr, flags) != JSI_OK) {
        return JSI_ERROR;
    }

    return JSI_OK;

}

Jsi_RC
Jsi_OptionsDump(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *rec, Jsi_Value **ret, Jsi_Wide flags)
{
    char *record = (char*)rec;
    Jsi_OptionSpec *specPtr = specs;
    int len = 0, i = 0, count = 0;
    if (!Jsi_OptionsValid(interp, specs))
        return Jsi_LogError("invalid options");
    
    while (specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END && specPtr->name) {
        specPtr++;
        len+=2;
    }
    if (!len)
        return JSI_OK;
    Jsi_Value *rv = Jsi_ValueMakeObject(interp, NULL, NULL);
    Jsi_IncrRefCount(interp, rv);
    Jsi_Obj *obj = rv->d.obj;
    specPtr = specs;
    while (specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END && specPtr->name) {
        Jsi_Value  *vv = Jsi_ValueNew1(interp);
        if (jsi_GetOption(interp, specPtr, record, NULL, &vv, flags) != JSI_OK) {
            Jsi_DecrRefCount(interp, vv);
            Jsi_DecrRefCount(interp, rv);
            return JSI_ERROR;
        }
        if (flags&JSI_OPTS_VERBOSE) {
            // dump: id,value,help,info,init
            Jsi_Value *vrv = Jsi_ValueMakeObject(interp, NULL, NULL);
            Jsi_IncrRefCount(interp, vrv);
            Jsi_Obj *vobj = vrv->d.obj;
            Jsi_ObjInsert(interp, vobj, "value", vv, 0);
            Jsi_DecrRefCount(interp, vv);
            vv = vrv;
            jsi_DumpOptionSpec(interp, vobj, specPtr, 0);
        }
        Jsi_ObjInsert(interp, obj, specPtr->name, vv, 0);
        Jsi_DecrRefCount(interp, vv);
        count++;
        i++;
        specPtr++;
    }
    assert(specPtr->id == JSI_OPTION_END);
    Jsi_ValueReplace(interp, ret, rv);
    Jsi_DecrRefCount(interp, rv);
    return JSI_OK;
}

int
Jsi_OptionsProcess(Jsi_Interp *interp, Jsi_OptionSpec *specs,  void *rec, Jsi_Value *args, Jsi_Wide flags)
{
    Jsi_OptionSpec *specPtr;
    int count = 0;
    char *record = (char*)rec;
    Jsi_TreeEntry *tPtr;
    Jsi_TreeSearch search;
    Jsi_Obj *to;
    bool isSafe = interp->isSafe;
    if (!Jsi_OptionsValid(interp, specs))
        return -1;

    if (interp->subOpts.compat && !(flags&JSI_OPTS_FORCE_STRICT))
        flags |=  JSI_OPTS_IGNORE_EXTRA;
    specs = jsi_GetCachedOptionSpecs(interp, specs);
    
    if (args == NULL || args->vt == JSI_VT_NULL) {
        for (specPtr = specs; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END && specPtr->name; specPtr++) {
            specPtr->flags &= ~JSI_OPT_IS_SPECIFIED;
            if (jsi_SetOption(interp, specPtr, (char*)specPtr->name, record, NULL, flags, isSafe) != JSI_OK)
                return -1;
        }
        assert(specPtr->id == JSI_OPTION_END);
        return 0;
    }
    if (args->vt != JSI_VT_OBJECT || args->d.obj->ot != JSI_OT_OBJECT || args->d.obj->arr) {
        Jsi_LogError("expected object");
        return -1;
    }
    to = args->d.obj;
    int reqCnt = 0;
    if ((JSI_OPTS_IS_UPDATE&flags)==0 && !(JSI_OPT_PASS2&flags)) {
        for (specPtr = specs; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END && specPtr->name; specPtr++) {
            specPtr->flags &= ~JSI_OPT_IS_SPECIFIED;
            if (specPtr->flags &  JSI_OPT_REQUIRED)
                reqCnt++;
        }
        assert(specPtr->id == JSI_OPTION_END);
    }
        
    for (tPtr = Jsi_TreeSearchFirst(to->tree, &search, 0, NULL);
        tPtr != NULL; tPtr = Jsi_TreeSearchNext(&search)) {
        
        const char *arg;
        Jsi_Value *optval;
        count++;
        arg =(char*) Jsi_TreeKeyGet(tPtr);
        optval = (Jsi_Value*)Jsi_TreeValueGet(tPtr);

        specPtr = Jsi_OptionsFind(interp, specs, arg, flags);
        if (specPtr == NULL) {
            if (flags&JSI_OPTS_IGNORE_EXTRA)
                continue;
            count = -1;
            goto done;
        }
        if (((JSI_OPT_PASS2&flags) && !(JSI_OPT_PASS2&specPtr->flags))
            || ((JSI_OPT_PASS2&specPtr->flags) && !(JSI_OPT_PASS2&flags)))
            continue;
            
        if ((JSI_OPT_READ_ONLY&specPtr->flags)) {
            Jsi_LogError("Error option is readonly: \"%.40s\"", specPtr->name);
            count = -1;
            goto done;
        }
        if ((JSI_OPTS_IS_UPDATE&flags) && (JSI_OPT_INIT_ONLY&specPtr->flags)) {
            Jsi_LogError("Error can not update option: \"%.40s\"", specPtr->name);
            count = -1;
            goto done;
        }
        if (isSafe && (JSI_OPTS_IS_UPDATE&flags) && (JSI_OPT_LOCKSAFE&specPtr->flags)) {
            Jsi_LogError("Error isSafe disallows updating option: \"%.40s\"", specPtr->name);
            count = -1;
            goto done;
        }

        if (jsi_SetOption(interp, specPtr, (char*)arg, record, optval, flags, isSafe) != JSI_OK) {
            count = -1;
            goto done;
        }
 
        specPtr->flags |= JSI_OPT_IS_SPECIFIED;
    }
    if (reqCnt) {
        for (specPtr = specs; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END && specPtr->name; specPtr++) {
            if (specPtr->flags &  JSI_OPT_REQUIRED &&
                !(specPtr->flags&JSI_OPT_IS_SPECIFIED)) {

                Jsi_LogError("Error required field not specified: \"%.40s\"", specPtr->name);
                count = -1;
            }
        }
    }

done:
    Jsi_TreeSearchDone(&search);
    if (count<0 && !(JSI_OPTS_IS_UPDATE&flags))
        Jsi_OptionsFree(interp, specs, rec, flags);
    return count;
}

Jsi_RC
Jsi_OptionsConf(Jsi_Interp *interp, Jsi_OptionSpec *specs,  void *rec, Jsi_Value *val, Jsi_Value **ret, Jsi_Wide flags)
{
    flags |= JSI_OPTS_IS_UPDATE;
    if (!Jsi_OptionsValid(interp, specs))
        return Jsi_LogError("invalid options");
    
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
    if (val->vt != JSI_VT_OBJECT) 
        return Jsi_LogError("expected string, object, or null");
    if (Jsi_OptionsProcess(interp, specs, rec, val, JSI_OPTS_IS_UPDATE|flags) < 0)
        return JSI_ERROR;
    return JSI_OK;
}

static const char **jsi_OptGetEnumList(Jsi_OptionSpec* spec) {
    const char **list = (const char**)spec->data;
    Jsi_OptionSpec* es=NULL;
    int fflags = (spec->flags);
    if (spec->id != JSI_OPTION_CUSTOM || spec->custom != Jsi_Opt_SwitchEnum)
        return list;
    if (list && (fflags & JSI_OPT_ENUM_SPEC)) {
        es = (typeof(es))list;
        while (es->id != JSI_OPTION_END)
            es++;
        list = es->init.STRKEY;
    }
    return list;
}

static void jsi_DumpCustomSpec(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_OptionSpec* spec)
{
    Jsi_OptionCustom* cspec = Jsi_OptionCustomBuiltin(spec->custom);
    void *data = spec->data;
    if (cspec->help) {
        if (Jsi_Strchr(cspec->help, '\n'))
            Jsi_LogError("%s .help contains newline: %s", cspec->name, cspec->help);
        Jsi_ObjInsert(interp, nobj, "help", Jsi_ValueNewStringKey(interp, cspec->help),0);
    }
    if (cspec->info)
        Jsi_ObjInsert(interp, nobj, "info", Jsi_ValueNewStringKey(interp, cspec->info),0);
    Jsi_ObjInsert(interp, nobj, "name", Jsi_ValueNewStringKey(interp, cspec->name),0);

    if (data && (spec->custom == Jsi_Opt_SwitchEnum || spec->custom == Jsi_Opt_SwitchBitset)) {
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        Jsi_Value *svalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        const char **lst = jsi_OptGetEnumList(spec);
        int i = 0;
        while (lst[i]) {
            Jsi_ObjArrayAdd(interp, sobj, Jsi_ValueNewStringKey(interp, lst[i]));
            i++;
        }
        Jsi_ObjInsert(interp, nobj, (spec->custom == Jsi_Opt_SwitchBitset?"bitSet":"enumList"), svalue, 0);
    } else if (spec->custom == Jsi_Opt_SwitchSuboption) {
        Jsi_OptionSpec* subSpec = (Jsi_OptionSpec*)data;
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        Jsi_Value *svalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        jsi_DumpOptionSpecs(interp, sobj, subSpec);
        Jsi_ObjInsert(interp, nobj, "subSpec", svalue, 0);
    }
}

static void jsi_DumpOptionSpec(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_OptionSpec* spec, int addName)
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
    Jsi_ObjInsert(interp, nobj, "type", Jsi_ValueNewStringKey(interp, jsi_OptionTypeStr(spec->id,0)),0);
    Jsi_ObjInsert(interp, nobj, "cName", Jsi_ValueNewStringKey(interp, jsi_OptionTypeStr(spec->id,1)),0);
    Jsi_ObjInsert(interp, nobj, "initOnly", Jsi_ValueNewBoolean(interp, (spec->flags & JSI_OPT_INIT_ONLY)!=0), 0);
    Jsi_ObjInsert(interp, nobj, "readOnly", Jsi_ValueNewBoolean(interp, (spec->flags & JSI_OPT_READ_ONLY)!=0), 0);
    Jsi_ObjInsert(interp, nobj, "required", Jsi_ValueNewBoolean(interp, (spec->flags & JSI_OPT_REQUIRED)!=0), 0);
    Jsi_ObjInsert(interp, nobj, "noCase", Jsi_ValueNewBoolean(interp, (spec->flags & JSI_OPT_CUST_NOCASE)!=0), 0);
    Jsi_ObjInsert(interp, nobj, "size", Jsi_ValueNewNumber(interp, (Jsi_Number)spec->size), 0);
    if (spec->flags)
        Jsi_ObjInsert(interp, nobj, "flags", Jsi_ValueNewNumber(interp, (Jsi_Number)spec->flags), 0);
    if (spec->data) {
        if (spec->id == JSI_OPTION_FUNC && spec->data) 
            Jsi_ObjInsert(interp, nobj, "args", Jsi_ValueNewStringDup(interp, (char*)spec->data), 0);
        else if (spec->id == JSI_OPTION_CUSTOM && (spec->custom == Jsi_Opt_SwitchBitset ||
            spec->custom == Jsi_Opt_SwitchEnum)) {
            const char **list = jsi_OptGetEnumList(spec);
            if (list)
                Jsi_ObjInsert(interp, nobj, "data", Jsi_ValueNewArray(interp, list, -1), 0);
        }
    }
    if (spec->id == JSI_OPTION_CUSTOM && spec->custom) {
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
        Jsi_Value *cvalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        jsi_DumpCustomSpec(interp, sobj,  spec);
        Jsi_ObjInsert(interp, nobj, "customArg", cvalue,0);
        Jsi_OptionSpec *os = spec;
        while (os->id != JSI_OPTION_END) os++;
        Jsi_ObjInsert(interp, nobj, "customArgHelp", Jsi_ValueNewStringKey(interp, (os->help?os->help:"")), 0);
        
    }
}

void jsi_DumpOptionSpecs(Jsi_Interp *interp, Jsi_Obj *nobj, Jsi_OptionSpec* spec)
{
    int i = 0;
    while (spec[i].id>=JSI_OPTION_BOOL && spec[i].id < JSI_OPTION_END) {
        Jsi_Obj *sobj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
        Jsi_Value *svalue = Jsi_ValueMakeObject(interp, NULL, sobj);
        jsi_DumpOptionSpec(interp, sobj, spec+i, 1);
        Jsi_ObjArrayAdd(interp, nobj, svalue);
        i++;
    }
}

static Jsi_RC jsi_ValueToVerify(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags)
{
    if (inStr)
        return JSI_ERROR;
    Jsi_Value **s = (Jsi_Value**)((char*)record) + spec->offset;
    Jsi_ValueHandlerProc *vfunc = (Jsi_ValueHandlerProc*)spec->data;
    if (!vfunc) 
        return Jsi_LogError("custom value spec did not set data: %s", spec->name);
    if (!inValue) {
        if (*s)
            Jsi_DecrRefCount(interp, *s);
        *s = NULL;
        return JSI_OK;
    }
    if (vfunc(interp, inValue, spec, record) != JSI_OK)
        return JSI_ERROR;
    *s = inValue;
    if (*s)
        Jsi_IncrRefCount(interp, *s);
    return JSI_OK;
}

static Jsi_RC jsi_VerifyToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *dStr, void *record, Jsi_Wide flags)
{
    if (dStr)
        return JSI_ERROR;
    Jsi_Value **s = (Jsi_Value**)(((char*)record) + spec->offset);
    if (*s)
        Jsi_ValueReplace(interp, outValue, *s);
    return JSI_OK;
}

static void jsi_VerifyFree(Jsi_Interp *interp, Jsi_OptionSpec* spec, void *ptr)
{
    Jsi_Value **v = (Jsi_Value**)ptr;
    if (v && *v)
        Jsi_DecrRefCount(interp, *v);
}

static Jsi_OptionCustom jsi_OptSwitchValueVerify = {
    .name="verify", .parseProc=jsi_ValueToVerify, .formatProc=jsi_VerifyToValue, .freeProc=jsi_VerifyFree, .help="Verify that a value id is correct"
};


static Jsi_RC ValueToCArray(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags)
{
    if (inStr)
        return JSI_ERROR;
    uchar *s = (uchar*)((char*)record) + spec->offset;
    Jsi_OptionSpec *subSpec = spec->init.OPT_CARRAY;
    int argc, i, isize, size = spec->arrSize;
    if (!subSpec || size<=0 || (isize=subSpec->size)<=0)
        goto bail;
    isize = isize/size;
    if (!Jsi_ValueIsArray(interp, inValue)) 
        return Jsi_LogError("expected array");
    argc = Jsi_ValueGetLength(interp, inValue);
    if (argc != size) 
        return Jsi_LogError("array length %d was not %d", argc, size);
    for (i = 0; i<size; i++) {
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, inValue, i);
        if (Jsi_OptionsSet(interp, subSpec, (void*)s, subSpec->name, v, 0) != JSI_OK)
            return JSI_ERROR;
        s += isize;
    }
    return JSI_OK;
bail:
    return Jsi_LogError("bad config");
}

static Jsi_RC jsi_CArrayToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *dStr, void *record, Jsi_Wide flags)
{
    Jsi_Obj *obj = NULL;
    if (dStr)
        return JSI_ERROR;
    uchar *s = (uchar*)((char*)record) + spec->offset;
    Jsi_OptionSpec *subSpec = spec->init.OPT_CARRAY;
    int i, cnt, isize, size = spec->arrSize;
    if (!subSpec || size<=0 || (isize=subSpec->size)<=0)
        goto bail;
    isize = isize/size;
#ifndef JSI_LITE_ONLY //TODO: already in lite #ifdef
    obj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    cnt = 0;
    for (i=0; i<size; i++) {
        Jsi_Value *v = Jsi_ValueNew1(interp);
        cnt++;
        Jsi_RC rc = Jsi_OptionsGet(interp, subSpec, s, spec->name, &v, 0);
        Jsi_ObjArrayAdd(interp, obj, v);
        Jsi_DecrRefCount(interp, v);
        if (JSI_OK != rc)
            goto bail;
        s += isize;
    }
    Jsi_ValueMakeArrayObject(interp, outValue, obj);
#endif
    return JSI_OK;
bail:
    if (obj)
        Jsi_ObjFree(interp, obj);
    Jsi_LogError("bad config");
    return JSI_ERROR;
}

static void jsi_CArrayFree(Jsi_Interp *interp, Jsi_OptionSpec* spec, void *ptr)
{
    /*Jsi_OptionSpec *subSpec = spec->init.ini.OPT_CARRAY; // TODO: ???
    if (!subSpec) {
        Jsi_Value **v = (Jsi_Value**)ptr;
        if (v)
            Jsi_DecrRefCount(interp, *v);
    }
    int i, isize, size = spec->asize;
    if ((isize=subSpec->size)<=0)
        return;
    isize = isize/size;
    uchar *s = (uchar*)ptr;
    for (i=0; i<size; i++) {
        Jsi_OptionsFree(interp, subSpec, s, 0);
        s += isize;
    }*/
}

static Jsi_OptionCustom jsi_OptSwitchCArray = {
    .name="array", .parseProc=ValueToCArray, .formatProc=jsi_CArrayToValue, .freeProc=jsi_CArrayFree, .help="Array of OPT types"
};
#else

static Jsi_OptionSpec * jsi_GetCachedOptionSpecs(Jsi_Interp *interp, const Jsi_OptionSpec *staticSpecs)
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
    spec = jsi_GetCachedOptionSpecs(interp, spec);
    do  {
        for (specPtr = spec; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END; specPtr++) {
            if ((Jsi_GlobMatch(option, specPtr->name, 0)) &&
                    (specPtr->flags & JSI_OPT_IS_SPECIFIED)) {
                cnt++;
            }
        }
        assert(specPtr->id == JSI_OPTION_END);

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

    for (staticSpecPtr=staticSpecs; staticSpecPtr->id>=JSI_OPTION_BOOL && staticSpecPtr->id<JSI_OPTION_END && staticSpecPtr->name;
            staticSpecPtr++) {
        entrySpace += sizeof(Jsi_OptionSpec);
    }

    newSpecs = (Jsi_OptionSpec *) Jsi_Malloc(entrySpace);
    memcpy((void *) newSpecs, (void *) staticSpecs, entrySpace);
    return newSpecs;
}

/* Free data items and reset values back to 0. */
void
Jsi_OptionsFree(Jsi_Interp *interp, Jsi_OptionSpec *specs, void *rec, Jsi_Wide flags /*unused*/)
{
    Jsi_OptionSpec *specPtr;
    char *record = (char*)rec;
    for (specPtr = specs; specPtr->id>=JSI_OPTION_BOOL && specPtr->id < JSI_OPTION_END && specPtr->name; specPtr++) {
        char *ptr = record + specPtr->offset;
        if (specPtr->flags&JSI_OPT_NO_CLEAR)
            continue;
        if (specPtr->custom) {
            Jsi_OptionCustom* cust = Jsi_OptionCustomBuiltin(specPtr->custom);
            if (cust->freeProc != NULL) {
                (*cust->freeProc)(interp, specPtr, (char **)ptr);
                continue;
            }
        }

        switch (specPtr->id) {
#ifndef JSI_LITE_ONLY
        case JSI_OPTION_VALUE:
        case JSI_OPTION_STRING:
        case JSI_OPTION_OBJ:
        case JSI_OPTION_REGEXP:
        case JSI_OPTION_ARRAY:
        case JSI_OPTION_FUNC:
        case JSI_OPTION_USEROBJ:
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
        case JSI_OPTION_TIME_T:
            *(time_t*)ptr = 0;
            break;
        case JSI_OPTION_NUMBER:
            *(Jsi_Number*)ptr = 0;
            break;
        case JSI_OPTION_FLOAT:
            *(float*)ptr = 0;
            break;
        case JSI_OPTION_LDOUBLE:
            *(ldouble*)ptr = 0;
            break;
        case JSI_OPTION_DOUBLE:
        case JSI_OPTION_TIME_D:
            *(double*)ptr = 0;
            break;
        case JSI_OPTION_TIME_W:
            *(Jsi_Wide*)ptr = 0;
            break;
        case JSI_OPTION_INT8:  *(int8_t*)ptr = 0; break;
        case JSI_OPTION_INT16: *(int16_t*)ptr = 0; break;
        case JSI_OPTION_INT32: *(int32_t*)ptr = 0; break;
        case JSI_OPTION_INT64: *(int64_t*)ptr = 0; break;
        case JSI_OPTION_UINT8: *(uint8_t*)ptr = 0; break;
        case JSI_OPTION_UINT16:*(uint16_t*)ptr = 0; break;
        case JSI_OPTION_UINT32:*(uint32_t*)ptr = 0; break;
        case JSI_OPTION_UINT64:*(uint64_t*)ptr = 0; break;
        case JSI_OPTION_INT: *(int*)ptr = 0; break;
        case JSI_OPTION_UINT: *(uint*)ptr = 0; break;
        case JSI_OPTION_BOOL: *(bool*)ptr = 0; break;
        case JSI_OPTION_CUSTOM:
            break;
        default:
            if (specPtr->size>0)
                memset(ptr, 0, specPtr->size);
            break;
        }
    }
}

static Jsi_RC jsi_ValueToEnum(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags)
{
    int n = 0;
    char *s = (char*)(((char*)record) + spec->offset);
    const char **list = (const char **)spec->data;
    Jsi_OptionSpec* es=NULL;
    int fflags = (flags|spec->flags);
    int cflags = (fflags&JSI_OPT_CUST_NOCASE?JSI_CMP_NOCASE:0);
    if (fflags&JSI_OPT_ENUM_EXACT)
        cflags |= JSI_CMP_EXACT;
    if (list && (fflags & JSI_OPT_ENUM_SPEC)) {
        es = (typeof(es))list;
        while (es->id != JSI_OPTION_END) es++;
        list = es->init.STRKEY;
        es = (typeof(es))spec->data;
    }
    if (!list) 
        return Jsi_LogError("custom enum spec did not set data: %s", spec->name);
    if (inStr) {
        if (JSI_OK != Jsi_GetIndex(interp, (char*)inStr, list, "enum", cflags, &n))
            return JSI_ERROR;
    } else
#ifndef JSI_LITE_ONLY
    if (JSI_OK != Jsi_ValueGetIndex(interp, inValue, list, "enum", cflags, &n))
        return JSI_ERROR;

#endif
    if (fflags&JSI_OPT_ENUM_UNSIGNED) {
        uint64_t u = (uint64_t)n;
        if (es)
            u = es[n].value;
        switch (spec->size) {
            case 1: *(uint8_t*)s = (uint8_t)u; break;
            case 2: *(uint16_t*)s = (uint16_t)u; break;
            case 4: *(uint32_t*)s = (uint32_t)u; break;
            case 8: *(uint64_t*)s = (uint64_t)u; break;
            default:
                return Jsi_LogError("enum size must be 1, 2, 4, or 8: %s", spec->name);
        }
    } else {
        int64_t m = n;
        if (es)
            m = es[n].value;
        switch (spec->size) {
            case 1: *(int8_t*)s = (int8_t)m; break;
            case 2: *(int16_t*)s = (int16_t)m; break;
            case 4: *(int32_t*)s = (int32_t)m; break;
            case 8: *(int64_t*)s = (int64_t)m; break;
            default: 
                return Jsi_LogError("enum size must be 1, 2, 4, or 8: %s", spec->name);
        }
    }
    
    return JSI_OK;
}

static Jsi_RC jsi_EnumToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record, Jsi_Wide flags)
{
    uint i = 0, j, esiz = 0;
    int64_t n;
    char *s = (char*)(((char*)record) + spec->offset);
    const char **list = (const char**)spec->data;
    Jsi_OptionSpec* es=NULL, *esp = NULL;
    int fflags = (flags|spec->flags);
    int uflag = (fflags&JSI_OPT_ENUM_UNSIGNED);
    if (list && (fflags & JSI_OPT_ENUM_SPEC)) {
        esp = es = (typeof(es))list;
        while (es->id != JSI_OPTION_END) es++;
        list = es->init.STRKEY;
        esiz = es->size;
        es = (typeof(es))spec->data;
    }
    if (!list) 
        return Jsi_LogError("custom enum spec did not set data: %s", spec->name);
    if (outStr) {
        if (uflag) {
            switch (spec->size) {
                case 1: n = *(uint8_t*)s; break;
                case 2: n = *(uint16_t*)s; break;
                case 4: n = *(uint32_t*)s; break;
                case 8: n = *(uint64_t*)s; break;
                default: 
                    return Jsi_LogError("enum size %d must be 1, 2, 4, or 8: %s", spec->size, spec->name);
            }
        } else {
            switch (spec->size) {
                case 1: n = *(int8_t*)s; break;
                case 2: n = *(int16_t*)s; break;
                case 4: n = *(int32_t*)s; break;
                case 8: n = *(int64_t*)s; break;
                default: 
                    return Jsi_LogError("enum size %d must be 1, 2, 4, or 8: %s", spec->size, spec->name);
            }
        }
        if (spec->flags&JSI_OPT_FMT_NUMBER) {
            Jsi_DSPrintf(outStr, "%" PRIu64, (uint64_t)n);
            return JSI_OK;
        }
        
        if (es) {
            for (j=0; j<esiz && list[j]; j++) {
                if (n == esp[j].value) {
                    i = j;
                    break;
                }
                if (j>=esiz)
                    i = esiz;
            }
        } else
            for (i=0; i<n && list[i]; i++) ; /* Look forward til n */
        if (list[i])
            Jsi_DSAppendLen(outStr, list[i], -1);
        else if ((spec->flags&JSI_OPT_COERCE)) {
            Jsi_DSPrintf(outStr, "%" PRIu64, (uint64_t)n);
            return JSI_OK;
        } else
            return Jsi_LogError("enum has unknown value: %d", *s);
        return JSI_OK;
    }
#ifndef JSI_LITE_ONLY
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_RC rc = jsi_EnumToValue(interp, spec, NULL, &dStr, record, flags);
    if (rc == JSI_OK)
        Jsi_ValueMakeStringKey(interp, outValue, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return rc;
#endif
    return JSI_ERROR;
}

static Jsi_OptionCustom jsi_OptSwitchEnum = {
    .name="enum", .parseProc=jsi_ValueToEnum, .formatProc=jsi_EnumToValue, .freeProc=0, .help="one value from list"
};


static Jsi_RC jsi_ValueToBitset(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags)
{
    // TODO: change this to not use byte instead of int...
    int i, argc, n;
    char *s =(((char*)record) + spec->offset);
    char **argv;
    const char *cp, **list = (const char**)spec->data;
    uint64_t m = 0, im = 0;
    int fflags = ((flags|spec->flags)&JSI_OPT_CUST_NOCASE?JSI_CMP_NOCASE:0);
    if (!list) 
        return Jsi_LogError("custom enum spec did not set data: %s", spec->name);
    switch (spec->size) {
        case 1: im = *(uint8_t*)s; break;
        case 2: im = *(uint16_t*)s; break;
        case 4: im = *(uint32_t*)s; break;
        case 8: im = *(uint64_t*)s; break;
        default: 
            return Jsi_LogError("bitset size must be 1, 2, 4, or 8: %s", spec->name);
    }

#ifndef JSI_LITE_ONLY
    if (!inStr && Jsi_ValueIsString(interp, inValue))
        inStr = Jsi_ValueString(interp, inValue, NULL);
#endif
    if (inStr) {
        if (*inStr == '+') {
            inStr++;
            m = im;
        }
        if (*inStr) {
            Jsi_DString sStr;
            Jsi_DSInit(&sStr);
            Jsi_SplitStr(inStr, &argc, &argv, ",", &sStr);
            
            for (i=0; i<argc; i++) {
                int isnot = 0;
                cp = argv[i];
                if (*cp == '!') { isnot = 1; cp++; }
                if (JSI_OK != Jsi_GetIndex(interp, cp, list, "enum", fflags, &n))
                    return JSI_ERROR;
                if (n >= (int)(spec->size*8)) 
                    return Jsi_LogError("list larger than field size: %s", spec->name);
                if (isnot)
                    m &= ~(1<<n);
                else
                    m |= (1<<n);
            }
            Jsi_DSFree(&sStr);
        }
    } else {
#ifndef JSI_LITE_ONLY
        if (!inValue) {
            *s = 0;
            return JSI_OK;
        }
        if (Jsi_ValueIsObjType(interp, inValue, JSI_OT_OBJECT) && !Jsi_ValueIsArray(interp, inValue)) {
            Jsi_TreeEntry *tPtr;
            Jsi_TreeSearch search;
            Jsi_Tree *tp = Jsi_TreeFromValue(interp, inValue);
            
            m = im;
            for (tPtr = (tp?Jsi_TreeSearchFirst(tp, &search, 0, NULL):NULL);
                tPtr != NULL; tPtr = Jsi_TreeSearchNext(&search)) {
                cp =(char*) Jsi_TreeKeyGet(tPtr);
                Jsi_Value *optval = (Jsi_Value*)Jsi_TreeValueGet(tPtr);
                
                if (JSI_OK != Jsi_GetIndex(interp, cp, list, "bitset", fflags, &n)) {
                    Jsi_TreeSearchDone(&search);
                    return JSI_ERROR;
                }
                if (!Jsi_ValueIsBoolean(interp, optval)) {
                    Jsi_TreeSearchDone(&search);
                    return Jsi_LogError("object member is not a bool: %s", cp);
                }
                bool vb;
                Jsi_ValueGetBoolean(interp, optval, &vb);
                if (!vb)
                    m &= ~(1<<n);
                else
                    m |= (1<<n);
            }
            if (tp)
                Jsi_TreeSearchDone(&search);
            *s = m;
            return JSI_OK;
        }
        
        if (!Jsi_ValueIsArray(interp, inValue)) 
            return Jsi_LogError("expected array or object");
        argc = Jsi_ValueGetLength(interp, inValue);
        for (i=0; i<argc; i++) {
            int isnot = 0;
            Jsi_Value *v = Jsi_ValueArrayIndex(interp, inValue, i);
            const char *cp = (v?Jsi_ValueString(interp, v, NULL):"");
            if (!cp) 
                return Jsi_LogError("expected string");
            if (i == 0) {
                if (*cp == '+' && !cp[1]) {
                    m = im;
                    continue;
                }
            }
            if (*cp == '!') { isnot = 1; cp++; }
            if (JSI_OK != Jsi_GetIndex(interp, cp, list, "bitset", fflags, &n))
                return JSI_ERROR;
            if (isnot)
                m &= ~(1<<n);
            else
                m |= (1<<n);
        }
        *s = m;
#endif
    }
    switch (spec->size) {
        case 1: *(uint8_t*)s = (uint8_t)m; break;
        case 2: *(uint16_t*)s = (uint16_t)m; break;
        case 4: *(uint32_t*)s = (uint32_t)m; break;
        case 8: *(uint64_t*)s = (uint64_t)m; break;
    }
    return JSI_OK;
}

static Jsi_RC jsi_BitsetToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record, Jsi_Wide flags)
{
    int i, n, cnt = 0, *s = (int*)(((char*)record) + spec->offset);
    const char **list = (const char**)spec->data;
    if (!list) 
        return Jsi_LogError("custom enum spec did not set data: %s", spec->name);
    if (outStr) {
        n = *s;
        for (i=0; list[i]; i++) {
            if (i >= (int)(spec->size*8)) 
                return Jsi_LogError("list larger than field size: %s", spec->name);
            if (!(n & (1<<i)))
                continue;
            if (cnt++)
                Jsi_DSAppendLen(outStr, ", ", 1);
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
    Jsi_ValueMakeArrayObject(interp, outValue, obj);
#endif
    return JSI_OK;
}

static Jsi_OptionCustom jsi_OptSwitchBitset = {
    .name="bitset", .parseProc=jsi_ValueToBitset, .formatProc=jsi_BitsetToValue, .freeProc=0, .help="An int field accessed a bit at a time"
};

#ifndef JSI_LITE_ONLY
/* Scanning function */
static Jsi_RC jsi_ValueToSubopt(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags)
{
    if (inStr)
        return JSI_ERROR;
    char *s = ((char*)record) + spec->offset;
    Jsi_OptionSpec *subspec = (Jsi_OptionSpec *)spec->data;
  
    if (spec == subspec) 
        return Jsi_LogError("subspec was recursive");
    if (!subspec) 
        return Jsi_LogError("custom suboption spec did not set data: %s", spec->name);
    if (inValue && Jsi_ValueIsNull(interp, inValue) == 0 &&
        (Jsi_ValueIsObjType(interp, inValue, JSI_OT_OBJECT)==0 || inValue->d.obj->isarrlist)) 
        return Jsi_LogError("expected object");
    return (Jsi_OptionsProcess(interp, subspec, s, inValue, flags)<0 ? JSI_ERROR : JSI_OK);
}

/* Printing function. */
static Jsi_RC jsi_SuboptToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record, Jsi_Wide flags)
{
    if (outStr)
        return JSI_ERROR;
    char *s = ((char*)record) + spec->offset;
    Jsi_OptionSpec *subspec = (Jsi_OptionSpec *)spec->data;
    if (spec == subspec) 
        return Jsi_LogError("recursive subspec not allowed");
    if (!subspec) 
        return Jsi_LogError("custom suboption spec did not set data: %s", spec->name);
    return Jsi_OptionsDump(interp, subspec, s, outValue, flags);
}

static void jsi_SuboptFree(Jsi_Interp *interp, Jsi_OptionSpec* spec, void *ptr)
{
    Jsi_OptionSpec *subspec = (Jsi_OptionSpec *)spec->data;
    Jsi_OptionsFree(interp, subspec, ptr, 0);
}

static Jsi_OptionCustom jsi_OptSwitchSuboption = {
    .name="suboption", .parseProc=jsi_ValueToSubopt, .formatProc=jsi_SuboptToValue, .freeProc=jsi_SuboptFree,
};

/* Parent Function: Scanning function */
static Jsi_RC jsi_ValueToParentFunc(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags)
{
    if (inStr)
        return JSI_ERROR;
    Jsi_Value *val;
    Jsi_Value **v = (Jsi_Value **)(((char*)record) + spec->offset);
    const char *s, *subspec = (const char *)spec->data;
    Jsi_Interp *pinterp = interp->parent;
    if (!pinterp)
        return Jsi_LogError("no parent interp");
    if (!subspec) 
        return Jsi_LogError("custom parentfunc spec did not set data: %s", spec->name);
    if (!inValue || Jsi_ValueIsNull(interp, inValue)) {
        if (*v) Jsi_DecrRefCount(pinterp, *v);
        *v = NULL;
        return JSI_OK;
    }
    if (!(s=Jsi_ValueString(interp, inValue, NULL))) {
        return Jsi_LogError("expected string or null");
    }
    val = Jsi_NameLookup(pinterp, s);
    if (!val)
        return Jsi_LogError("value not found in parent: %s", s);
    if (!Jsi_ValueIsFunction(pinterp, val))
        return Jsi_LogError("expected a func value");
    if (spec->data && (interp->strict || pinterp->strict))
        if (!jsi_FuncIsNoop(pinterp, val)
            && !jsi_FuncArgCheck(pinterp, val->d.obj->d.fobj->func, (char*)spec->data)) 
            return Jsi_LogError("failed setting func pointer for %s", spec->name);

    *v = val;
    Jsi_IncrRefCount(pinterp, val);
    return JSI_OK;
}

/* Printing function. */
static Jsi_RC jsi_ParentFuncToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record, Jsi_Wide flags)
{
    if (outStr)
        return JSI_ERROR;
    Jsi_Value **v = (Jsi_Value **)(((char*)record) + spec->offset);
    //const char *s, *subspec = (const char *)spec->data;
    if (!*v) {
        Jsi_ValueMakeNull(interp, outValue);
        return JSI_OK;
    }
    if (!interp->parent)
        return Jsi_LogError("no parent interp");
    if (spec->data) {
        Jsi_DString dStr = {};
        Jsi_DSPrintf(&dStr, "func(%s)", (char*)spec->data);
        Jsi_ValueFromDS(interp, &dStr, outValue);
    }
    return JSI_OK;
}

static void jsi_ParentFuncFree(Jsi_Interp *interp, Jsi_OptionSpec* spec, void *ptr)
{
    Jsi_Value **v = (Jsi_Value **)(((char*)ptr));
    if (!interp->parent) return;
    if (*v)
        Jsi_DecrRefCount(interp->parent, *v);
    *v = NULL;
}

static Jsi_OptionCustom jsi_OptSwitchParentFunc = {
    .name="funcinparent", .parseProc=jsi_ValueToParentFunc, .formatProc=jsi_ParentFuncToValue, .freeProc=jsi_ParentFuncFree,
};

/* Scanning function */
static Jsi_RC jsi_ValueToBitfield(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue,
    const char *inStr, void *record, Jsi_Wide flags)
{
    Jsi_csgset *bsget = spec->init.OPT_BITS;
    Jsi_Interp *d = interp;
    int idx = spec->idx;
    uchar *data = (uchar*)record;
    Jsi_Number num;
    Jsi_OptionSpec* enumSpec = (typeof(enumSpec))spec->data;

    if (!d || !bsget || idx<0) 
        return Jsi_LogBug("invalid bitfield");
    if (enumSpec) {
        struct numStruct { int64_t numVal; } nval = {};
        Jsi_OptionSpec eSpec[] = {
            JSI_OPT(CUSTOM, typeof(nval), numVal, .help=spec->help, .flags=JSI_OPT_ENUM_SPEC,
                .custom=Jsi_Opt_SwitchEnum, .data=(void*)enumSpec, .info=0, .tname=spec->tname, .value=0, .bits=0, .boffset=8*sizeof(int64_t) ),
            JSI_OPT_END(typeof(nval))
        };
        if (JSI_OK != jsi_ValueToEnum(interp, eSpec, inValue, inStr, (void*)&nval, flags))
            return JSI_ERROR;
        num = (Jsi_Number)nval.numVal;
    } else if (inStr) {
        if (Jsi_GetDouble(interp, inStr, &num) != JSI_OK)
            return JSI_ERROR;
    } else {
        if (inValue && !Jsi_ValueIsNumber(interp, inValue)) 
            return JSI_ERROR;
        Jsi_ValueGetNumber(interp, inValue, &num);
    }
    int64_t inum = (int64_t)num;
    return (*bsget)(interp, data, &inum, spec, idx, 1);
}

/* Printing function. */
static Jsi_RC jsi_BitfieldToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue,
    Jsi_DString *outStr, void *record, Jsi_Wide flags)
{
    Jsi_csgset *bsget = spec->init.OPT_BITS;
    Jsi_Interp *d = interp;
    int idx = spec->idx;
    uchar *data = (uchar*)record;
    int64_t inum;
    Jsi_OptionSpec* enumSpec = (typeof(enumSpec))spec->data;

    if (!d || !bsget || idx<0) 
        return Jsi_LogBug("invalid bitfield");
    Jsi_RC rc = (*bsget)(interp, data, &inum, spec, idx, 0);
    if (rc != JSI_OK)
        return JSI_ERROR;

    if (enumSpec) {
        struct numStruct { int64_t numVal; } nval = { inum };
        Jsi_OptionSpec eSpec[] = {
            JSI_OPT(CUSTOM, struct numStruct, numVal, .help=spec->help, .flags=JSI_OPT_ENUM_SPEC, .custom=Jsi_Opt_SwitchEnum,
            .data=(void*)enumSpec, .info=0, .tname=spec->tname, .value=0, .bits=0, .boffset=8*sizeof(int64_t) ), //TODO: extra
            JSI_OPT_END(struct numStruct)
        };
        if (JSI_OK != jsi_EnumToValue(interp, eSpec, outValue, outStr, (void*)&nval, flags))
            return JSI_ERROR;
    } else if (outStr) {
        char obuf[JSI_MAX_NUMBER_STRING];
        snprintf(obuf, sizeof(obuf), "%" PRId64, inum);
        Jsi_DSAppend(outStr, obuf, NULL);
    } else {
        Jsi_Number num = (Jsi_Number)inum;
        Jsi_ValueMakeNumber(interp, outValue, num);
    }
    return JSI_OK;
}

static Jsi_OptionCustom jsi_OptSwitchBitfield = {
    .name="bitfield", .parseProc=jsi_ValueToBitfield, .formatProc=jsi_BitfieldToValue
};

/* Scanning function */
static Jsi_RC jsi_ValueToNull(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags)
{
    return JSI_OK;
}

/* Printing function. */
static Jsi_RC jsi_NullToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record, Jsi_Wide flags)
{
    Jsi_ValueMakeNull(interp, outValue);
    return JSI_OK;
}

static Jsi_OptionCustom jsi_OptSwitchNull = {
    .name="null", .parseProc=jsi_ValueToNull, .formatProc=jsi_NullToValue
};

#endif

typedef struct {
    Jsi_OptionInitVal init;
} jsi_IniValStruct;

static Jsi_OptionCustom* custOpts[] = { NULL,
    &jsi_OptSwitchEnum, 
    &jsi_OptSwitchBitset,
#ifndef JSI_LITE_ONLY
    &jsi_OptSwitchSuboption, 
    &jsi_OptSwitchBitfield,
    &jsi_OptSwitchValueVerify,
    &jsi_OptSwitchCArray,
    &jsi_OptSwitchNull,
    &jsi_OptSwitchParentFunc
#else
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
#endif
};

Jsi_OptionCustom* Jsi_OptionCustomBuiltin(Jsi_OptionCustom* cust) {
    if ((uintptr_t)cust < (uintptr_t)(sizeof(custOpts)/sizeof(custOpts[0])))
        return custOpts[(uintptr_t)cust];
    return cust;
}

Jsi_RC jsi_InitOptions(Jsi_Interp *interp, int release) {
    if (release) return JSI_OK;
    assert((sizeof(jsi_OptTypeInfo)/sizeof(jsi_OptTypeInfo[0])) == (JSI_OPTION_END+1));
    int i;
    for (i=JSI_OPTION_BOOL; i<JSI_OPTION_END; i++)
        assert(jsi_OptTypeInfo[i].id == (Jsi_OptionId)i);
    return JSI_OK;
}
