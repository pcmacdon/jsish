/* Jsi commands to access C-data.   http://jsish.org */
#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#ifndef JSI_OMIT_CDATA
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/time.h>

#define UdcGet(udf, _this, funcPtr) \
   CDataObj *udf = (typeof(udf))Jsi_UserObjGetData(interp, _this, funcPtr); \
    if (!udf) \
        return Jsi_LogError("CData.%s called with non-CData object", funcPtr->cmdSpec->name);

enum { jsi_CTYP_DYN_MEMORY=(1LL<<32), jsi_CTYP_STRUCT=(1LL<<33), jsi_CTYP_ENUM=(1LL<<34) };

typedef struct {
    JSI_DBDATA_FIELDS  
    Jsi_StructSpec *sl, *keysf;
    Jsi_Map** mapPtr;
    const char *help, *structName, *keyName, *varParam, *name;
    uint flags;
    bool isAlloc;
    Jsi_Map_Type mapType;
    Jsi_Key_Type keyType;
    Jsi_Wide user;
    Jsi_Interp *interp;
    int objId;
    Jsi_Obj *fobj;
} CDataObj;

static Jsi_StructSpec*  jsi_csStructGet(Jsi_Interp *interp, const char *name);

static Jsi_RC     jsi_csStructInit(Jsi_StructSpec* s, uchar* data);
static Jsi_RC jsi_csBitGetSet(Jsi_Interp *interp, void *vrec, Jsi_Wide* valPtr, Jsi_OptionSpec *spec, int idx, bool isSet);

static Jsi_OptionTypedef *jsi_csGetTypeSpec(void* p) { Jsi_OptionTypedef *s = (typeof(s))p; SIGASSERT(s, TYPEDEF); return s; }


/* Traverse hash table and match unique substring. */
Jsi_HashEntry *jsi_csfindInHash(Jsi_Interp *interp, Jsi_Hash * tbl, const char *name)
{
    int len;
    Jsi_HashSearch se;
    Jsi_HashEntry *sentry = 0, *entry = Jsi_HashEntryFind(tbl, name);
    if (entry)
        return entry;
    len = Jsi_Strlen(name);
    entry = Jsi_HashSearchFirst(tbl, &se);
    while (entry) {
        char *ename = (char *) Jsi_HashKeyGet(entry);
        if (!Jsi_Strncmp(name, ename, len)) {
            if (sentry)
                return 0;
            sentry = entry;
        }
        entry = Jsi_HashSearchNext(&se);
    }
    return sentry;
}

/* Traverse enum and match unique substring. */
Jsi_OptionSpec *jsi_csgetEnum(Jsi_Interp *interp, const char *name)
{
    Jsi_HashEntry *entry = jsi_csfindInHash(interp, interp->EnumHash, name);
    return entry ? (Jsi_OptionSpec *) Jsi_HashValueGet(entry) : 0;
}


/************* INITIALIZERS  *********************************************/

/* Init Type hash */
void jsi_csInitType(Jsi_Interp *interp)
{
    if (interp->CTypeHash->numEntries) return;
    bool isNew;
    Jsi_HashEntry *entry;
    const Jsi_OptionTypedef *tl;
    if (!interp->typeInit) {
        int i;
        for (i = JSI_OPTION_BOOL; i!=JSI_OPTION_END; i++) {
            tl = Jsi_OptionTypeInfo((Jsi_OptionId)i);
            entry = Jsi_HashEntryNew(interp->TYPEHash, tl->idName, &isNew);
            if (!isNew)
                Jsi_LogBug("duplicate type: %s", tl->idName);
            Jsi_HashValueSet(entry, (void*)tl);
            if (tl->cName && tl->cName[0])
                Jsi_HashSet(interp->CTypeHash, tl->cName, (void*)tl);
        }
    }
    interp->typeInit = 1;
}

static Jsi_RC jsi_csSetupStruct(Jsi_Interp *interp, Jsi_StructSpec *sl, Jsi_FieldSpec *sf, 
    Jsi_StructSpec* recs, int flen, Jsi_OptionTypedef** stPtr, int arrCnt) {
    bool isNew;
    int i, cnt = 0, boffset = 0;
    Jsi_HashEntry *entry, *hPtr;
    if (!(hPtr=Jsi_HashEntryNew(interp->CTypeHash, sl->name, &isNew)) || !isNew)
        return Jsi_LogError("struct is c-type: %s", sl->name);
    entry = Jsi_HashEntryNew(interp->StructHash, sl->name, &isNew);
    if (!isNew)
        return Jsi_LogError("duplicate struct: %s", sl->name);
    Jsi_FieldSpec *asf = NULL, *osf = sf;
    while (sf && sf->id != JSI_OPTION_END) {
        if (!sf->type)
            sf->type = Jsi_OptionTypeInfo(sf->id);
        if (!sf->type && sf->tname)
            sf->type = Jsi_TypeLookup(interp, sf->tname);
        int isbitset = ((sf->flags&JSI_OPT_BITSET_ENUM)!=0);
        if (sf->type && sf->type->extData && (sf->type->flags&(jsi_CTYP_ENUM|jsi_CTYP_STRUCT))) {
            // A struct sub-field or a bit field mapped to an ENUM.
            Jsi_OptionSpec *es = (typeof(es))sf->type->extData;
            es->value++;
            if ((sf->type->flags&jsi_CTYP_ENUM)) {
                if (sf->bits)
                    return Jsi_LogError("enum of bits unsupported: %s", sl->name); //TODO: get working again...
                sf->custom = (isbitset ? Jsi_Opt_SwitchBitset : Jsi_Opt_SwitchEnum);
                sf->data = (void*)es->data;
                sf->id = JSI_OPTION_CUSTOM;
            }
            else if (sf->type->flags & jsi_CTYP_STRUCT) {
                sf->custom = Jsi_Opt_SwitchSuboption;
                sf->data = es->extData;
                sf->id = JSI_OPTION_CUSTOM;
            }
        }
        if (recs) {
            if (!sf->type)
                return Jsi_LogError("unknown id");
            sf->tname = sf->type->cName;
            sf->size = (isbitset?(int)sizeof(int):sf->type->size);
            if (sf->arrSize)
                sf->size *= sf->arrSize;
            sf->idx = cnt;
            sf->boffset = boffset;
            if (sf->bits) {
                if (sf->bits>=64)
                    return Jsi_LogError("bits too large");
                boffset += sf->bits;
                sf->id = JSI_OPTION_CUSTOM;
                sf->custom=Jsi_Opt_SwitchBitfield;
                sf->init.OPT_BITS=&jsi_csBitGetSet;
            } else {
                sf->offset = (boffset+7)/8;
                boffset += sf->size*8;
            }
        } else {
            boffset += sf->size*8;
        }
        sf->extData = (uchar*)sl;
        sf++, cnt++;
    }
    sl->idx = cnt;
    if (!sl->size) 
        sl->size = (boffset+7)/8;
    if (sl->ssig)
        Jsi_HashSet(interp->SigHash, (void*)(uintptr_t)sl->ssig, sl);
    int extra = 0;
    if (flen)
        extra = sl->size + ((flen+2+arrCnt*2)*sizeof(Jsi_StructSpec));
    Jsi_OptionTypedef *st = (typeof(st))Jsi_Calloc(1, sizeof(*st) + extra);
    SIGINIT(st, TYPEDEF);
    if (!recs) 
        sf = osf;
    else {
        st->extra = (uchar*)(st+1); // Space for struct initializer.
        sf =  (typeof(sf))(st->extra + sl->size);
        memcpy(sf, recs, sizeof(*sf)*(flen+1));
        sl = sf+flen+1;
        if (arrCnt)
            asf = sl+1;
        memcpy(sl, recs+flen+1, sizeof(*sl));
        for (i=0; i<flen; i++) {
            sf[i].extData = (uchar*)sl;
            if (sf[i].id == 0 && sf[i].type)
                sf[i].id = sf[i].type->id;
            if (sf[i].arrSize) {
                asf[0] = sf[i];
                asf[1] = sf[flen];
                asf->arrSize = asf->offset = 0;
                //asf->size = asf->type->size;
                sf[i].id = JSI_OPTION_CUSTOM;
                sf[i].custom=Jsi_Opt_SwitchCArray;
                sf[i].init.OPT_CARRAY = asf;
                asf += 2;
                //sf[i].extData = 
                 //   {.sig=JSI_SIG_OPTS_FIELD, .name=sf[i].name, 
                  //  JSI_OPT_CARRAY_ITEM_(JSI_SIG_OPTS_FIELD,'+otype+', '+name+', sf[i].name, .help=sf[i].help, .flags='+fflags+rest+'),\n'
                   // JSI_OPT_END_(JSI_SIG_OPTS_FIELD,'+name+', .help="Options for array field '+name+'.'+fname+'")\n  };\n\n';
                   // JSI_OPT_CARRAY_(JSI_SIG_OPTS_FIELD,'+name+', '+fname+', "'+fdescr+'", '+fflags+', '+arnam+', '+f.asize+', "'+type+'", '+csinit+'),\n';
            }
        }
    }
    st->extData = (uchar*)sl;
    sl->extData = (uchar*)sf;
    sl->type = st;
    st->cName = sl->name;
    st->idName = "CUSTOM";
    st->id = JSI_OPTION_CUSTOM;
    st->size = sl->size;
    st->flags = jsi_CTYP_DYN_MEMORY|jsi_CTYP_STRUCT;
    Jsi_HashValueSet(entry, sl);
    Jsi_HashValueSet(hPtr, st);
    st->hPtr = hPtr;
    if (stPtr)
        *stPtr = st;
    return JSI_OK;
}

/* Init Struct hash */
static void jsi_csInitStructTables(Jsi_Interp *interp)
{
    Jsi_StructSpec *sf, *sl = interp->statics->structs;
    while (sl  && sl->name) {
        sf = (typeof(sf))sl->data;
        jsi_csSetupStruct(interp, sl, sf, NULL, 0, NULL, 0);
        sl++;
    }
}


/* Init Enum hash */
void jsi_csInitEnum(Jsi_Interp *interp)
{
    bool isNew;
    Jsi_EnumSpec *sl = interp->statics->enums;
    while (sl && sl->name && sl->id != JSI_OPTION_END) {
        Jsi_HashEntry *entry = Jsi_HashEntryNew(interp->EnumHash, sl->name, &isNew);
        if (!isNew)
            Jsi_LogBug("duplicate enum: %s", sl->name);
        assert(isNew);
        Jsi_HashValueSet(entry, sl);
        sl++;
    }
}

/* Find the enum name in all enums */
Jsi_OptionSpec *jsi_csInitEnumItem(Jsi_Interp *interp)
{
    Jsi_HashEntry *entry;
    bool isNew;
    Jsi_EnumSpec *sl = interp->statics->enums;
    while (sl &&  sl->name && sl->id != JSI_OPTION_END) {
        Jsi_OptionSpec *ei = (typeof(ei))sl->data;
        while (ei->name  && ei->id != JSI_OPTION_END) {
            entry = Jsi_HashEntryNew(interp->EnumItemHash, ei->name, &isNew);
            if (!isNew)
                Jsi_LogBug("duplicate enum item: %s", ei->name);
            Jsi_HashValueSet(entry, ei);
            ei++;
        }
        sl++;
    }
    return 0;
}

Jsi_StructSpec *Jsi_CDataStruct(Jsi_Interp *interp, const char *name) {
    return (Jsi_StructSpec *)Jsi_HashGet(interp->StructHash, name, 0);
}

Jsi_StructSpec *jsi_csStructGet(Jsi_Interp *interp, const char *name)
{
    if (!name) return NULL;
    Jsi_StructSpec *sl,*spec = Jsi_CDataStruct(interp, name);
    if (spec) return spec;

    Jsi_CData_Static *CData_Strs = interp->statics;
    while (CData_Strs) {
        sl = CData_Strs->structs;
        while (sl->name) {
            if (!Jsi_Strcmp(name, sl->name))
                return sl;
            sl++;
        }
        CData_Strs = CData_Strs->nextPtr;
    }
    return NULL;
}


Jsi_EnumSpec *jsi_csGetEnum(Jsi_Interp *interp, const char *name) {
    return (Jsi_EnumSpec *)Jsi_HashGet(interp->EnumHash, name, 0);
}
  

/******* INIT ***************************************************/


/* Initialize a struct to default values */
static Jsi_RC jsi_csStructInit(Jsi_StructSpec * sl, uchar * data)
{
    /* Jsi_OptionSpec *sf; */
    assert(sl);
    if (!data) {
        fprintf(stderr, "missing data at %s:%d", __FILE__, __LINE__);
        return JSI_ERROR;
    }
    if (sl->custom)
        memcpy(data, sl->custom, sl->size);
    else if (sl->type && sl->type->extra)
        memcpy(data, sl->type->extra, sl->size);
    else
        memset(data, 0, sl->size);

    if (sl->ssig)
        *(Jsi_Sig *) data = sl->ssig;
    return JSI_OK;
}

/* Scanning function */
static Jsi_RC jsi_csValueToFieldType(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value *inValue, const char *inStr, void *record, Jsi_Wide flags)
{
    if (inStr)
        return JSI_ERROR;
    const Jsi_OptionTypedef *typ, **sp = (typeof(sp))(((uchar*)record) + spec->offset);

    if (!inStr) {
        if (!inValue || Jsi_ValueIsString(interp, inValue)==0)
            return Jsi_LogError("expected string");
        inStr = Jsi_ValueString(interp, inValue, NULL);
    }
    typ = Jsi_TypeLookup(interp, inStr);
    if (!typ)
        return Jsi_LogError("unknown type: %s", inStr);
    *sp = typ;
    return JSI_OK;
}

/* Printing function. */
static Jsi_RC jsi_csFieldTypeToValue(Jsi_Interp *interp, Jsi_OptionSpec* spec, Jsi_Value **outValue, Jsi_DString *outStr, void *record, Jsi_Wide flags)
{
    if (outStr)
        return JSI_ERROR;
    const Jsi_OptionTypedef **sp = (typeof(sp))(((uchar*)record) + spec->offset), *sptr = *sp;
    //const char **s = (const char**)((char*)record + spec->offset);
    if (sptr)
        SIGASSERT(sptr, TYPEDEF);
    if (sptr && sptr->cName)
        Jsi_ValueMakeStringKey(interp, outValue, sptr->cName);
    else
        Jsi_ValueMakeNull(interp, outValue);
    return JSI_OK;
}

Jsi_OptionCustom jsi_OptSwitchFieldType = {
    .name="fieldtype", .parseProc=jsi_csValueToFieldType, .formatProc=jsi_csFieldTypeToValue, .freeProc=NULL,
};


static bool jsi_csBitSetGet(int isSet, uchar *tbuf, int bits, Jsi_UWide *valPtr) {
    union bitfield *bms = (union bitfield *)tbuf;
    Jsi_UWide val = *valPtr;
    union bitfield {
        Jsi_UWide b1:1; Jsi_UWide b2:2; Jsi_UWide b3:3; Jsi_UWide b4:4; Jsi_UWide b5:5; Jsi_UWide b6:6;
        Jsi_UWide b7:7; Jsi_UWide b8:8; Jsi_UWide b9:9; Jsi_UWide b10:10; Jsi_UWide b11:11; Jsi_UWide b12:12;
        Jsi_UWide b13:13; Jsi_UWide b14:14; Jsi_UWide b15:15; Jsi_UWide b16:16; Jsi_UWide b17:17; 
        Jsi_UWide b18:18; Jsi_UWide b19:19; Jsi_UWide b20:20; Jsi_UWide b21:21; Jsi_UWide b22:22;
        Jsi_UWide b23:23; Jsi_UWide b24:24; Jsi_UWide b25:25; Jsi_UWide b26:26; Jsi_UWide b27:27;
        Jsi_UWide b28:28; Jsi_UWide b29:29; Jsi_UWide b30:30; Jsi_UWide b31:31; Jsi_UWide b32:32;
        Jsi_UWide b33:33; Jsi_UWide b34:34; Jsi_UWide b35:35; Jsi_UWide b36:36; Jsi_UWide b37:37;
        Jsi_UWide b38:38; Jsi_UWide b39:39; Jsi_UWide b40:40; Jsi_UWide b41:41; Jsi_UWide b42:42;
        Jsi_UWide b43:43; Jsi_UWide b44:44; Jsi_UWide b45:45; Jsi_UWide b46:46; Jsi_UWide b47:47;
        Jsi_UWide b48:48; Jsi_UWide b49:49; Jsi_UWide b50:50; Jsi_UWide b51:51; Jsi_UWide b52:52;
        Jsi_UWide b53:53; Jsi_UWide b54:54; Jsi_UWide b55:55; Jsi_UWide b56:56; Jsi_UWide b57:57;
        Jsi_UWide b58:58; Jsi_UWide b59:59; Jsi_UWide b60:60; Jsi_UWide b61:61; Jsi_UWide b62:62;
        Jsi_UWide b63:63; Jsi_UWide b64:64;
    };
    if (isSet) {
        switch (bits) {
    #define CBSN(n) \
            case n: bms->b##n = val; return (bms->b##n == val)
           CBSN(1); CBSN(2); CBSN(3); CBSN(4); CBSN(5); CBSN(6); CBSN(7); CBSN(8);
           CBSN(9); CBSN(10); CBSN(11); CBSN(12); CBSN(13); CBSN(14); CBSN(15); CBSN(16);
           CBSN(17); CBSN(18); CBSN(19); CBSN(20); CBSN(21); CBSN(22); CBSN(23); CBSN(24);
           CBSN(25); CBSN(26); CBSN(27); CBSN(28); CBSN(29); CBSN(30); CBSN(31); CBSN(32);
           CBSN(33); CBSN(34); CBSN(35); CBSN(36); CBSN(37); CBSN(38); CBSN(39); CBSN(40);
           CBSN(41); CBSN(42); CBSN(43); CBSN(44); CBSN(45); CBSN(46); CBSN(47); CBSN(48);
           CBSN(49); CBSN(50); CBSN(51); CBSN(52); CBSN(53); CBSN(54); CBSN(55); CBSN(56);
           CBSN(57); CBSN(58); CBSN(59); CBSN(60); CBSN(61); CBSN(62); CBSN(63); CBSN(64);
        }
        assert(0);
    }
    switch (bits) {
#define CBGN(n) \
        case n: val = bms->b##n; break
       CBGN(1); CBGN(2); CBGN(3); CBGN(4); CBGN(5); CBGN(6); CBGN(7); CBGN(8);
       CBGN(9); CBGN(10); CBGN(11); CBGN(12); CBGN(13); CBGN(14); CBGN(15); CBGN(16);
       CBGN(17); CBGN(18); CBGN(19); CBGN(20); CBGN(21); CBGN(22); CBGN(23); CBGN(24);
       CBGN(25); CBGN(26); CBGN(27); CBGN(28); CBGN(29); CBGN(30); CBGN(31); CBGN(32);
       CBGN(33); CBGN(34); CBGN(35); CBGN(36); CBGN(37); CBGN(38); CBGN(39); CBGN(40);
       CBGN(41); CBGN(42); CBGN(43); CBGN(44); CBGN(45); CBGN(46); CBGN(47); CBGN(48);
       CBGN(49); CBGN(50); CBGN(51); CBGN(52); CBGN(53); CBGN(54); CBGN(55); CBGN(56);
       CBGN(57); CBGN(58); CBGN(59); CBGN(60); CBGN(61); CBGN(62); CBGN(63); CBGN(64);
       default: assert(0);
    }
    *valPtr = val;
    return 1;
}

static bool jsi_csSBitSetGet(int isSet, uchar *tbuf, int bits, Jsi_Wide *valPtr) {
    union bitfield *bms = (union bitfield *)tbuf;
    Jsi_Wide val = *valPtr;
    union bitfield {
        Jsi_Wide b1:1; Jsi_Wide b2:2; Jsi_Wide b3:3; Jsi_Wide b4:4; Jsi_Wide b5:5; Jsi_Wide b6:6;
        Jsi_Wide b7:7; Jsi_Wide b8:8; Jsi_Wide b9:9; Jsi_Wide b10:10; Jsi_Wide b11:11; Jsi_Wide b12:12;
        Jsi_Wide b13:13; Jsi_Wide b14:14; Jsi_Wide b15:15; Jsi_Wide b16:16; Jsi_Wide b17:17; 
        Jsi_Wide b18:18; Jsi_Wide b19:19; Jsi_Wide b20:20; Jsi_Wide b21:21; Jsi_Wide b22:22;
        Jsi_Wide b23:23; Jsi_Wide b24:24; Jsi_Wide b25:25; Jsi_Wide b26:26; Jsi_Wide b27:27;
        Jsi_Wide b28:28; Jsi_Wide b29:29; Jsi_Wide b30:30; Jsi_Wide b31:31; Jsi_Wide b32:32;
        Jsi_Wide b33:33; Jsi_Wide b34:34; Jsi_Wide b35:35; Jsi_Wide b36:36; Jsi_Wide b37:37;
        Jsi_Wide b38:38; Jsi_Wide b39:39; Jsi_Wide b40:40; Jsi_Wide b41:41; Jsi_Wide b42:42;
        Jsi_Wide b43:43; Jsi_Wide b44:44; Jsi_Wide b45:45; Jsi_Wide b46:46; Jsi_Wide b47:47;
        Jsi_Wide b48:48; Jsi_Wide b49:49; Jsi_Wide b50:50; Jsi_Wide b51:51; Jsi_Wide b52:52;
        Jsi_Wide b53:53; Jsi_Wide b54:54; Jsi_Wide b55:55; Jsi_Wide b56:56; Jsi_Wide b57:57;
        Jsi_Wide b58:58; Jsi_Wide b59:59; Jsi_Wide b60:60; Jsi_Wide b61:61; Jsi_Wide b62:62;
        Jsi_Wide b63:63; Jsi_Wide b64:64;
    };
    if (isSet) {
        switch (bits) {
           CBSN(1); CBSN(2); CBSN(3); CBSN(4); CBSN(5); CBSN(6); CBSN(7); CBSN(8);
           CBSN(9); CBSN(10); CBSN(11); CBSN(12); CBSN(13); CBSN(14); CBSN(15); CBSN(16);
           CBSN(17); CBSN(18); CBSN(19); CBSN(20); CBSN(21); CBSN(22); CBSN(23); CBSN(24);
           CBSN(25); CBSN(26); CBSN(27); CBSN(28); CBSN(29); CBSN(30); CBSN(31); CBSN(32);
           CBSN(33); CBSN(34); CBSN(35); CBSN(36); CBSN(37); CBSN(38); CBSN(39); CBSN(40);
           CBSN(41); CBSN(42); CBSN(43); CBSN(44); CBSN(45); CBSN(46); CBSN(47); CBSN(48);
           CBSN(49); CBSN(50); CBSN(51); CBSN(52); CBSN(53); CBSN(54); CBSN(55); CBSN(56);
           CBSN(57); CBSN(58); CBSN(59); CBSN(60); CBSN(61); CBSN(62); CBSN(63); CBSN(64);
        }
        assert(0);
    }
    switch (bits) {
       CBGN(1); CBGN(2); CBGN(3); CBGN(4); CBGN(5); CBGN(6); CBGN(7); CBGN(8);
       CBGN(9); CBGN(10); CBGN(11); CBGN(12); CBGN(13); CBGN(14); CBGN(15); CBGN(16);
       CBGN(17); CBGN(18); CBGN(19); CBGN(20); CBGN(21); CBGN(22); CBGN(23); CBGN(24);
       CBGN(25); CBGN(26); CBGN(27); CBGN(28); CBGN(29); CBGN(30); CBGN(31); CBGN(32);
       CBGN(33); CBGN(34); CBGN(35); CBGN(36); CBGN(37); CBGN(38); CBGN(39); CBGN(40);
       CBGN(41); CBGN(42); CBGN(43); CBGN(44); CBGN(45); CBGN(46); CBGN(47); CBGN(48);
       CBGN(49); CBGN(50); CBGN(51); CBGN(52); CBGN(53); CBGN(54); CBGN(55); CBGN(56);
       CBGN(57); CBGN(58); CBGN(59); CBGN(60); CBGN(61); CBGN(62); CBGN(63); CBGN(64);
       default: assert(0);
    }
    *valPtr = val;
    return 1;
}

static Jsi_RC jsi_csSBitGetSet(Jsi_Interp *interp, void *vrec,  Jsi_Wide* vPtr, Jsi_OptionSpec *spec, int idx, bool isSet) {
    Jsi_Wide *valPtr = (typeof(valPtr))vPtr;
    int bits = spec->bits;
    int boffs = spec->boffset;
    if (bits<1 || bits>=64) return JSI_ERROR;
    int ofs = (boffs/8);
    int bo = (boffs%8); // 0 if byte-aligned
    int Bsz = ((bits+bo+7)/8);
    uchar *rec = (uchar*)vrec;
#ifdef __SIZEOF_INT128__
    typedef __int128 stvalType;
#else
    typedef Jsi_Wide stvalType;
#endif
    stvalType tbuf[2] = {};
    uchar sbuf[20], *bptr = (uchar*)tbuf;
    memcpy(tbuf, rec+ofs, Bsz);
    Jsi_Wide mval = *valPtr;
    Jsi_Wide amask = ((1LL<<(bits-1))-1LL);
    stvalType tval = 0, kval = 0, lmask;
    if (bo) { // If not byte aligned, get tval and shift
        bptr = sbuf;
        kval = tval = *(typeof(tval)*)tbuf;
        tval >>= bo;
        if (!isSet) {
            mval = (Jsi_Wide)tval;
            *(Jsi_Wide*)bptr = mval;
        }
    }
        
    if (!isSet) { // Get value.
        if (!jsi_csSBitSetGet(0, bptr, bits, &mval))
            return JSI_ERROR;
        *valPtr = mval;
        return JSI_OK;
    }
    
    if (!jsi_csSBitSetGet(1, bptr, bits, &mval))
        return JSI_ERROR;
    if (bo) {
        tval = (typeof(tval))mval;
        lmask=(amask<<bo);
        kval &= ~lmask;
        tval <<= bo;
        tval = (kval | tval);
        *(typeof(tval)*)tbuf = tval;
    }
    memcpy(rec+ofs, tbuf, Bsz);

    return JSI_OK;    
}

static Jsi_RC jsi_csBitGetSet(Jsi_Interp *interp, void *vrec,  Jsi_Wide* vPtr, Jsi_OptionSpec *spec, int idx, bool isSet) {
    bool us = (spec->tname && spec->tname[0] == 'u');
    if (!us)        
        return jsi_csSBitGetSet(interp, vrec, vPtr, spec, idx, isSet);

    if (*vPtr<0)
        return JSI_ERROR;

    Jsi_UWide *valPtr = (typeof(valPtr))vPtr;
    int bits = spec->bits;
    int boffs = spec->boffset;
    if (bits<1 || bits>=64) return JSI_ERROR;
    int ofs = (boffs/8);
    int bo = (boffs%8); // 0 if byte-aligned
    int Bsz = ((bits+bo+7)/8);
    uchar *rec = (uchar*)vrec;
#ifdef __SIZEOF_INT128__
    typedef unsigned __int128 utvalType;
#else
    typedef Jsi_UWide utvalType;
#endif
    utvalType tbuf[2] = {};
    uchar sbuf[20], *bptr = (uchar*)tbuf;
    memcpy(tbuf, rec+ofs, Bsz);
    Jsi_UWide mval;
    Jsi_UWide amask = ((1LL<<(bits-1))-1LL);
    utvalType tval = 0, kval = 0, lmask;
    if (bo) { // If not byte aligned, get tval and shift
        bptr = sbuf;
        kval = tval = *(typeof(tval)*)tbuf;
        tval >>= bo;
        if (!isSet) {
            mval = (Jsi_UWide)tval;
            *(Jsi_UWide*)bptr = mval;
        }
    } else
         mval = *valPtr;
        
    if (!isSet) { // Get value.
        if (!jsi_csBitSetGet(0, bptr, bits, &mval))
            return JSI_ERROR;
        *valPtr = mval;
        return JSI_OK;
    }
    
    if (!jsi_csBitSetGet(1, bptr, bits, &mval))
        return JSI_ERROR;
    if (bo) {
        tval = (typeof(tval))mval;
        lmask=(amask<<bo);
        kval &= ~lmask;
        tval <<= bo;
        tval = (kval | tval);
        *(typeof(tval)*)tbuf = tval;
    }
    memcpy(rec+ofs, tbuf, Bsz);

    return JSI_OK;    
}


static Jsi_RC jsi_csTypeFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    Jsi_OptionTypedef *type = jsi_csGetTypeSpec(ptr);
    if (type->extData && (type->flags&(jsi_CTYP_ENUM|jsi_CTYP_STRUCT)))
        ((Jsi_OptionSpec*)(type->extData))->value--;
    if (type->flags&jsi_CTYP_DYN_MEMORY)
        Jsi_Free(ptr);
    return JSI_OK;
}

static Jsi_RC jsi_DoneCData(Jsi_Interp *interp)
{
    if (!interp->SigHash) return JSI_OK;
    Jsi_HashDelete(interp->SigHash);
    Jsi_HashDelete(interp->StructHash);
    Jsi_HashDelete(interp->EnumHash);
    Jsi_HashDelete(interp->EnumItemHash);
    Jsi_HashDelete(interp->TYPEHash);
    Jsi_HashDelete(interp->CTypeHash);
    return JSI_OK;
}

Jsi_RC jsi_InitCData(Jsi_Interp *interp, int release)
{
    if (release) return jsi_DoneCData(interp);
#if JSI_USE_STUBS
    if (Jsi_StubsInit(interp, 0) != JSI_OK)
        return JSI_ERROR;
#endif

    interp->SigHash      = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, NULL);
    interp->StructHash   = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    interp->EnumHash     = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    interp->EnumItemHash = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    interp->CTypeHash    = Jsi_HashNew(interp, JSI_KEYS_STRING, jsi_csTypeFree);
    interp->TYPEHash     = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);

    if (Jsi_PkgProvide(interp, "CData", 1, jsi_InitCData) != JSI_OK)
        return JSI_ERROR;
    return JSI_OK;
}

/* Initialize a struct to default values */
Jsi_RC Jsi_CDataStructInit(Jsi_Interp *interp, uchar* data, const char *sname)
{
    Jsi_StructSpec * sl = jsi_csStructGet(interp, sname);
    if (!sl)
        return Jsi_LogError("unknown struct: %s", sname);
    return jsi_csStructInit(sl, data);
}

Jsi_CDataDb *Jsi_CDataLookup(Jsi_Interp *interp, const char *name) {
    CDataObj *cd = (typeof(cd))Jsi_UserObjDataFromVar(interp, name);
    if (!cd)
        return NULL;
    return (Jsi_CDataDb*)cd;
}

Jsi_RC Jsi_CDataRegister(Jsi_Interp *interp, Jsi_CData_Static *statics)
{
    Jsi_RC rc = JSI_OK;
    if (statics) {
        if (interp->statics)
            statics->nextPtr = interp->statics;
        interp->statics = statics;
        jsi_csInitType(interp);
        jsi_csInitStructTables(interp);
        jsi_csInitEnum(interp);
        jsi_csInitEnumItem(interp);
    }
    return rc;
}

/* Traverse types and match unique substring. */
Jsi_OptionTypedef *Jsi_TypeLookup(Jsi_Interp *interp, const char *name)
{
    int isup = 1;
    const char *cp = name;
    while (*cp && isup) {
        if (*cp != '_' && !isdigit(*cp) && !isupper(*cp)) { isup=0; break; }
        cp++;
    }
    Jsi_OptionTypedef *ptr = (typeof(ptr))Jsi_HashGet((isup?interp->TYPEHash:interp->CTypeHash), name, 0);
    if (ptr)
        SIGASSERT(ptr, TYPEDEF);
    return ptr;
}

#else // JSI_OMIT_CDATA

Jsi_CDataDb *Jsi_CDataLookup(Jsi_Interp *interp, const char *name) { return NULL; }
Jsi_RC Jsi_CDataRegister(Jsi_Interp *interp, Jsi_CData_Static *statics) { return JSI_ERROR; }
Jsi_RC Jsi_CDataStructInit(Jsi_Interp *interp, uchar* data, const char *sname) { return JSI_ERROR; }
Jsi_OptionTypedef *Jsi_TypeLookup(Jsi_Interp *interp, const char *name) { return NULL; }

#endif // JSI_OMIT_CDATA
#endif // JSI_LITE_ONLY
