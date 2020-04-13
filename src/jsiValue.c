#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#define bits_set(who, mask)     ((who) |= (mask))
#define bits_unset(who, mask)   ((who) &= (~(mask)))
#define bits_get(who, mask)     ((who) & (mask))

#if  JSI__MEMDEBUG
void jsi_VALCHK(Jsi_Value *val) {
    SIGASSERTV(val,VALUE);
    assert(val->vt <= JSI_VT__MAX);
    if (val->vt == JSI_VT_OBJECT)
        OBJCHK(val->d.obj);
}

void jsi_OBJCHK(Jsi_Obj *obj) {
    SIGASSERTV(obj,OBJ);
    assert(obj->ot <= JSI_OT__MAX);
}
#endif

/*********************************************/

bool Jsi_IsShared(Jsi_Interp* interp, Jsi_Value *v)
{
    SIGASSERT(v,VALUE);
    return (v->refCnt > 1);
}

int Jsi_IncrRefCount(Jsi_Interp* interp, Jsi_Value *v)
{
    SIGASSERT(v,VALUE);
    assert(v->refCnt>=0);
    jsi_DebugValue(v,"Incr", jsi_DebugValueCallIdx(), interp);
    return ++(v->refCnt);
}

int Jsi_DecrRefCount(Jsi_Interp* interp, Jsi_Value *v) {
    SIGASSERT(v,VALUE);
    if (v->refCnt<=0) {
#ifdef JSI_MEM_DEBUG
        fprintf(stderr, "Value decr with ref %d: VD.Idx=%d\n", v->refCnt, v->VD.Idx);
#endif
        return -2;
    }
    int ref;
    jsi_DebugValue(v,"Decr", jsi_DebugValueCallIdx(), interp);
    if ((ref = --(v->refCnt)) <= 0) {
        v->refCnt = -1;
        Jsi_ValueFree(interp, v);
    }
    return ref;
}

static Jsi_Value *ValueNew(Jsi_Interp *interp)
{
    interp->dbPtr->valueCnt++;
    interp->dbPtr->valueAllocCnt++;
    Jsi_Value *v = (Jsi_Value *)Jsi_Calloc(1,sizeof(*v));
    SIGINIT(v,VALUE)
    v->vt = JSI_VT_UNDEF;
    jsi_DebugValue(v,"New", jsi_DebugValueCallIdx(), interp);
    return v;
}

static Jsi_Value *ValueNew1(Jsi_Interp *interp)
{
    Jsi_Value *v = ValueNew(interp);
    Jsi_IncrRefCount(interp, v);
    return v;
}

static Jsi_Value *ValueDup(Jsi_Interp *interp, Jsi_Value *v)
{
    Jsi_Value *r = ValueNew1(interp);
    Jsi_ValueCopy(interp,r, v);
#ifdef JSI_MEM_DEBUG
    r->VD.label2 = "ValueDup";
#endif
    return r;
}
#ifndef JSI_MEM_DEBUG
Jsi_Value* Jsi_ValueNew(Jsi_Interp *interp) {
    return ValueNew(interp);
}
Jsi_Value* Jsi_ValueNew1(Jsi_Interp *interp) {
    return ValueNew1(interp);
}
Jsi_Value *Jsi_ValueDup(Jsi_Interp *interp, Jsi_Value *v) {
    return ValueDup(interp, v);
}
#else
static uint jsi_memDebugBreakIdx = 0;  // Debug memory by setting this, and adding BP on following func.
void jsi_memDebugBreak() {
}

// Debugging functions: set breakpoint with "cond B v == 0xNNN"
void jsi_DebugValue(Jsi_Value* v, const char *reason, uint cidx, Jsi_Interp *interp)
{
    if (jsi_memDebugBreakIdx && jsi_memDebugBreakIdx == v->VD.Idx)
        jsi_memDebugBreak();
    return;
}
void jsi_DebugObj(Jsi_Obj* o, const char *reason, uint cidx, Jsi_Interp *interp)
{
    if (jsi_memDebugBreakIdx && jsi_memDebugBreakIdx == o->VD.Idx)
        jsi_memDebugBreak();
    return;
}


void jsi_ValueDebugUpdate_(Jsi_Interp *interp, jsi_ValueDebug *vd, void *v, Jsi_Hash* tbl, const char *fname, int line, const char *func)
{
    vd->fname = fname;
    vd->line = line;
    vd->func = func;
    if (!vd->Idx)
        vd->Idx = interp->dbPtr->memDebugCallIdx;
    vd->hPtr = Jsi_HashSet(tbl, v, 0);
    vd->ip = interp->curIp;
    if (vd->ip) {
        vd->ipLine = vd->ip->Line;
        vd->ipOp = vd->ip->op;
        vd->ipFname = vd->ip->fname;
    }
    vd->interp = interp;
    if (jsi_memDebugBreakIdx && jsi_memDebugBreakIdx == vd->Idx)
        jsi_memDebugBreak();
}

void jsi_ValueDebugLabel_(jsi_ValueDebug *vd, const char *l1, const char *l2)
{
    if (l1)
        vd->label = l1;
    if (l2)
        vd->label2 = l2;
}


Jsi_Value * jsi_ValueNew(Jsi_Interp *interp, const char *fname, int line, const char *func) {
    Jsi_Value *v = ValueNew(interp);
    jsi_ValueDebugUpdate(interp, v, valueDebugTbl, fname, line, func);
    return v;
}

Jsi_Value * jsi_ValueNew1(Jsi_Interp *interp, const char *fname, int line, const char *func) {
    Jsi_Value *v = ValueNew1(interp);
    jsi_ValueDebugUpdate(interp, v, valueDebugTbl, fname, line, func);
    return v;
}
Jsi_Value * jsi_ValueDup(Jsi_Interp *interp, Jsi_Value *ov, const char *fname, int line, const char *func) {
    Jsi_Value *v = ValueDup(interp, ov);
    jsi_ValueDebugUpdate(interp, v, valueDebugTbl, fname, line, func);
    return v;
}

#ifndef JSI_OMIT_STUBS
#undef Jsi_ValueNew
#undef Jsi_ValueNew1
Jsi_Value *Jsi_ValueNew(Jsi_Interp *interp) { return ValueNew(interp); }
Jsi_Value *Jsi_ValueNew1(Jsi_Interp *interp) { return ValueNew1(interp); }
#define Jsi_ValueNew(interp) jsi_ValueNew(interp, __FILE__, __LINE__,__PRETTY_FUNCTION__)
#define Jsi_ValueNew1(interp) jsi_ValueNew1(interp, __FILE__, __LINE__,__PRETTY_FUNCTION__)
#endif

#endif

Jsi_Hash *strDebug = NULL;

static void ValueFree(Jsi_Interp *interp, Jsi_Value* v)
{
    SIGASSERTV(v,VALUE);
    //printf("FREE: %d\n", interp->valueCnt);
    switch (v->vt) {
        case JSI_VT_OBJECT:
            Jsi_ObjDecrRefCount(interp, v->d.obj);
            break;
        case JSI_VT_VARIABLE:
            assert(v->d.lval != v);
            Jsi_DecrRefCount(interp, v->d.lval);
            break;
        case JSI_VT_STRING:
            if (v->d.s.str && !v->f.bits.isstrkey) {
                Jsi_Free(v->d.s.str);
                /*Jsi_HashEntry *hPtr;
                if ((hPtr = Jsi_HashEntryFind(strDebug, v->d.s.str)))
                    Jsi_HashEntryDelete(hPtr);*/
            }
            break;
        default:
            break;
    }
    v->vt = JSI_VT_UNDEF;
}

void Jsi_ValueFree(Jsi_Interp *interp, Jsi_Value* v)
{
    interp->dbPtr->valueCnt--;
    jsi_DebugValue(v, "Free", jsi_DebugValueCallIdx(), interp);
    ValueFree(interp, v);
#ifdef JSI_MEM_DEBUG
    //if (v->VD.interp != interp)  //TODO: InterpAliasCmd leaking Values.
     //   fprintf(stderr, "cross interp delete: %p\n", v);
    if (v->VD.hPtr && !interp->cleanup) {
        if (!Jsi_HashEntryDelete(v->VD.hPtr))
            fprintf(stderr, "Value not in hash\n");
    }
    memset(v, 0, (sizeof(*v)-sizeof(v->VD)));
#endif
    Jsi_Free(v);
}

/* Reset a value back to undefined, releasing string/obj if necessary. */
void Jsi_ValueReset(Jsi_Interp *interp, Jsi_Value **vPtr) {
    Jsi_Value *v = *vPtr;
    SIGASSERTV(v,VALUE);
    Assert(v->vt <= JSI_VT__MAX);
    jsi_DebugValue(v, "Reset", jsi_DebugValueCallIdx(), interp);
    Assert(v->refCnt>=0);
    v->f.bits.lookupfailed = 0; // TODO: rework lookup-fail mechanism.
    if (v->vt == JSI_VT_UNDEF)
        return;
    ValueFree(interp, v);
    v->f.flag = 0;
}

// Assign value ptrs (to=from). Decr old to, and Incr from ref count.
void Jsi_ValueReplace(Jsi_Interp *interp, Jsi_Value **to, Jsi_Value *from )  {
    VALCHK(from);
    if( *to == from) return;
    if (*to)
        Jsi_DecrRefCount(interp, *to);
    *to = from;
    if (from)
        Jsi_IncrRefCount(interp, from);
}


static void jsi_ValueCopyMove(Jsi_Interp *interp, Jsi_Value *to, Jsi_Value *from, int isCopy )  {
    if (!from) {
        Jsi_ValueMakeUndef(interp, &to);
        return;
    }
    VALCHK(from);
    if( to == from) return;
    int ocnt = to->refCnt;
    Jsi_Value *ovt = NULL;
    assert(ocnt>0);
    int toVt = to->vt;
    if (toVt == JSI_VT_VARIABLE) {
        ovt = to->d.lval;
        Jsi_IncrRefCount(interp, ovt);
    }
    Jsi_ValueMakeUndef(interp, &to);
#ifdef JSI_MEM_DEBUG
    memcpy(to, from, sizeof(*to)-sizeof(to->VD));
    to->VD.label3 = from->VD.func;
#else
    *to = *from;
#endif
    if (isCopy) {
        if (to->refCnt) {
            switch (to->vt) {
                case JSI_VT_STRING:
                    if (!to->f.bits.isstrkey) {
                        to->d.s.str = Jsi_StrdupLen(to->d.s.str, to->d.s.len);
                    }
                    break;
                case JSI_VT_OBJECT:
                    Jsi_ObjIncrRefCount(interp,to->d.obj);
                    break;
                case JSI_VT_VARIABLE:
                    Jsi_IncrRefCount(interp,to->d.lval);
                    break;
                default:
                    break;
            }
        }
        to->refCnt = ocnt;
        if (ovt)
            Jsi_DecrRefCount(interp, ovt);
    } else {
        to->refCnt = ocnt;
        if (ovt)
            Jsi_DecrRefCount(interp, ovt);
        ocnt = from->refCnt;
#ifdef JSI_MEM_DEBUG
        memset(from, 0, sizeof(*to)-sizeof(to->VD));
#else
        memset(from, 0, sizeof(*to));
#endif
        SIGINIT(from, VALUE);
        from->refCnt = ocnt;
    }
}

void Jsi_ValueMove(Jsi_Interp *interp, Jsi_Value *to, Jsi_Value *from )  {
    return jsi_ValueCopyMove(interp, to, from, 0);
}

void Jsi_ValueCopy(Jsi_Interp *interp, Jsi_Value *to, Jsi_Value *from ) {
    return jsi_ValueCopyMove(interp, to, from, 1);
}

void Jsi_ValueDup2(Jsi_Interp *interp, Jsi_Value **to, Jsi_Value *from )
{
    if (!*to)
        *to = Jsi_ValueNew1(interp);
#ifdef JSI_MEM_DEBUG
    (*to)->VD.label3 = "ValueDup2";
#endif
    Jsi_ValueCopy(interp, *to, from);
    (*to)->f.bits.readonly = 0;
}

Jsi_Value *Jsi_ValueDupJSON(Jsi_Interp *interp, Jsi_Value *val)
{
    Jsi_DString pStr;
    Jsi_DSInit(&pStr);
    Jsi_ValueGetDString(interp, val, &pStr, JSI_OUTPUT_JSON);
    Jsi_Value *valPtr = NULL;
    if (Jsi_JSONParse(interp, Jsi_DSValue(&pStr), &valPtr, 0) != JSI_OK)
        Jsi_LogBug("bad json parse");
    Jsi_DSFree(&pStr);
    return valPtr;
}

#if 0
void jsi_AllValueOp(Jsi_Interp *interp, Jsi_Value* val, int op) {
    if (op==1) {
        //printf("ADD: %p : %p : %d\n", interp, val, val->VD.Idx);
        assert(interp->allValues!=val);
        val->next = interp->allValues;
        if (interp->allValues)
            interp->allValues->prev = val;
        interp->allValues = val;
        return;
    }
    if (op==0) {
        //printf("DEL: %p : %p\n", interp, val);
        if (!val || !interp->allValues) return;
        if (val == interp->allValues)
            interp->allValues = val->next;
        if (val->next)
            val->next->prev = val->prev;
        if (val->prev)  
            val->prev->next = val->next; 
        return;
    }
    if (op == -1) {
        while (interp->allValues) {
            printf("NEED CLEANUP: %p\n", interp->allValues);
            Jsi_ValueFree(interp, interp->allValues);
        }
        return;
    }
#if JSI__MEMDEBUG
    assert(0);
    abort();
#endif
}
#endif

Jsi_Value *Jsi_ValueObjLookup(Jsi_Interp *interp, Jsi_Value *target, const char *key, int isstrkey)
{
    Jsi_Obj *obj;
    Jsi_Value *v = NULL;
    if (interp->subOpts.noproto && key) {
        if (key[0] == 'p' && Jsi_Strcmp(key, "prototype")==0) {
            Jsi_LogError("inheritance is disabled in interp");
            return NULL;
        }
    }
    if (target->vt != JSI_VT_OBJECT) {
        if (interp->strict)
            Jsi_LogWarn("Target is not object: %d", target->vt);
        return NULL;
    }
    obj = target->d.obj;
    
#if (defined(JSI_HAS___PROTO__) && JSI_HAS___PROTO__==2)
    if (*key == '_' && Jsi_Strcmp(key, "__proto__")==0 && interp->noproto==0)
        return obj->__proto__;
#endif

    if (*key == 't' && Jsi_Strcmp(key, "this")==0)
        return interp->framePtr->inthis;
    if (obj->arr)
        v = jsi_ObjArrayLookup(interp, obj, key);
    if (!v)
        v= Jsi_TreeObjGetValue(obj, key, isstrkey);
    return v;  /* TODO: redo with copy */
}

Jsi_Value *Jsi_ValueArrayIndex(Jsi_Interp *interp, Jsi_Value *args, int index)
{
    Jsi_Obj *obj = args->d.obj;
    Jsi_Value *v;
    assert(args->vt == JSI_VT_OBJECT);
    if (obj->isarrlist && obj->arr)
        return ((index < 0 || (uint)index >= obj->arrCnt) ? NULL : obj->arr[index]);
    char unibuf[JSI_MAX_NUMBER_STRING];
    Jsi_NumberItoA10(index, unibuf, sizeof(unibuf));
    v = Jsi_TreeObjGetValue(args->d.obj, unibuf, 0);
    return v;
}

/**************************************************/

Jsi_RC Jsi_ValueGetBoolean(Jsi_Interp *interp, Jsi_Value *pv, bool *val)
{
    if (!pv) return JSI_ERROR;
    if (pv->vt == JSI_VT_BOOL)
        *val = pv->d.val;
    else if (pv->vt == JSI_VT_OBJECT && pv->d.obj->ot == JSI_OT_BOOL)
        *val = pv->d.obj->d.val;
    else 
        return JSI_ERROR;
    return JSI_OK;
}

bool Jsi_ValueIsArray(Jsi_Interp *interp, Jsi_Value *v)  {
    return (v->vt == JSI_VT_OBJECT && v->d.obj->ot == JSI_OT_OBJECT && v->d.obj->isarrlist);
}

bool Jsi_ValueIsBoolean(Jsi_Interp *interp, Jsi_Value *pv)
{
    return (pv->vt == JSI_VT_BOOL || (pv->vt == JSI_VT_OBJECT && pv->d.obj->ot == JSI_OT_BOOL));
}

bool Jsi_ValueIsNull(Jsi_Interp *interp, Jsi_Value *pv)
{
    return (pv->vt == JSI_VT_NULL);
}

bool Jsi_ValueIsUndef(Jsi_Interp *interp, Jsi_Value *pv)
{
    return (pv->vt == JSI_VT_UNDEF);
}

Jsi_RC Jsi_ValueGetNumber(Jsi_Interp *interp, Jsi_Value *pv, Jsi_Number *val)
{
    if (!pv) return JSI_ERROR;
    if (pv->vt == JSI_VT_NUMBER)
        *val = pv->d.num;
    else if (pv->vt == JSI_VT_OBJECT && pv->d.obj->ot == JSI_OT_NUMBER)
        *val = pv->d.obj->d.num;
    else 
        return JSI_ERROR;
    return JSI_OK;
}
bool Jsi_ValueIsNumber(Jsi_Interp *interp, Jsi_Value *pv)
{
    return (pv->vt == JSI_VT_NUMBER || (pv->vt == JSI_VT_OBJECT && pv->d.obj->ot == JSI_OT_NUMBER));
}

bool Jsi_ValueIsStringKey(Jsi_Interp* interp, Jsi_Value *key)
{
    if (key->vt == JSI_VT_STRING && key->f.bits.isstrkey)
        return 1;
    if (key->vt == JSI_VT_OBJECT && key->d.obj->ot == JSI_OT_STRING && key->d.obj->isstrkey)
        return 1;
    return 0;
}

bool Jsi_ValueIsString(Jsi_Interp *interp, Jsi_Value *pv)
{
    return (pv->vt == JSI_VT_STRING || (pv->vt == JSI_VT_OBJECT && pv->d.obj->ot == JSI_OT_STRING));
}

bool Jsi_ValueIsFunction(Jsi_Interp *interp, Jsi_Value *v)
{
    int rc = (v!=NULL && v->vt == JSI_VT_OBJECT && v->d.obj->ot == JSI_OT_FUNCTION);
    if (!rc) return rc;
    if (interp == v->d.obj->d.fobj->interp)
        return 1;
    fprintf(stderr, "OOPS: function in wrong interp %s: %s\n", 
        interp->parent?"(string came in from parent interp?)":"",
        v->d.obj->d.fobj->func->name);
    return 0;
}

bool Jsi_ValueIsType(Jsi_Interp *interp, Jsi_Value *pv, Jsi_vtype vtype) {
    if (!pv) return 0;
    return pv->vt == vtype;
}

Jsi_vtype Jsi_ValueTypeGet(Jsi_Value *pv) { return pv->vt; }


bool Jsi_ValueIsObjType(Jsi_Interp *interp, Jsi_Value *v, Jsi_otype otype)
{
    if (v == NULL || v->vt != JSI_VT_OBJECT)
        return 0;
    if (otype != JSI_OT_ARRAY)
        return (v->d.obj->ot == otype);
    if (v->d.obj->ot != JSI_OT_OBJECT || !v->d.obj->isarrlist)
        return 0;
    return 1;
}

char* Jsi_NumberToString(Jsi_Interp *interp, Jsi_Number d, char *buf, int bsiz)
{
     if (Jsi_NumberIsInteger(d)) {
        Jsi_NumberItoA10((Jsi_Wide)d, buf, bsiz);
    } else if (Jsi_NumberIsNormal(d)) {
        Jsi_NumberDtoA(interp, d, buf, bsiz, 0);
    } else if (Jsi_NumberIsNaN(d)) {
        Jsi_Strcpy(buf, "NaN");
    } else {
        int s = Jsi_NumberIsInfinity(d);
        if (s > 0) Jsi_Strcpy(buf,  "Infinity");
        else if (s < 0) Jsi_Strcpy(buf, "-Infinity");
        else {
            buf[0] = 0;
        }
    }
    return buf;
}

/* Return the string value.  Coerce value to a string type. */
const char* Jsi_ValueToString(Jsi_Interp *interp, Jsi_Value *v, int *lenPtr)
{
    Jsi_Number d;
    const char *ntxt = "undefined";
    int kflag = 1;
    int isKey = 0;
    char *key = NULL;
    if (!v)
        goto done;
    if (lenPtr) *lenPtr = 0;
    char unibuf[JSI_MAX_NUMBER_STRING*2];
    switch(v->vt) {
        case JSI_VT_STRING:
            ntxt = v->d.s.str;
            goto done;
        case JSI_VT_UNDEF:
            break;
        case JSI_VT_BOOL:
            ntxt = v->d.val ? "true":"false";
            break;
        case JSI_VT_NULL:
            ntxt = "null";
            break;
        case JSI_VT_NUMBER: {
            d = v->d.num;
fmtnum:
            if (Jsi_NumberIsInteger(d)) {
                Jsi_NumberItoA10((Jsi_Wide)d, unibuf, sizeof(unibuf));
                kflag = 0;
                ntxt = unibuf;
            } else if (Jsi_NumberIsNormal(d)) {
                Jsi_NumberDtoA(interp, d, unibuf, sizeof(unibuf), 0);
                kflag = 0;
                ntxt = unibuf;
            } else if (Jsi_NumberIsNaN(v->d.num)) {
                ntxt = "NaN";
            } else {
                int s = Jsi_NumberIsInfinity(d);
                if (s > 0) ntxt = "Infinity";
                else if (s < 0) ntxt = "-Infinity";
                else Jsi_LogBug("Ieee function got problem");
            }
            break;
        }
        case JSI_VT_OBJECT: {
            Jsi_Obj *obj = v->d.obj;
            switch(obj->ot) {
                case JSI_OT_STRING:
                    ntxt = obj->d.s.str;
                    goto done;
                case JSI_OT_BOOL:
                    ntxt = obj->d.val ? "true":"false";
                    break;
                case JSI_OT_NUMBER:
                    d = obj->d.num;
                    goto fmtnum;
                    break;
                default:
                    ntxt = "[object Object]";
                    break;
            }
            break;
        }
        default:
            Jsi_LogBug("Convert a unknown type: 0x%x to string", v->vt);
            break;
    }
    Jsi_ValueReset(interp, &v);
    if (!kflag) {
        Jsi_ValueMakeStringDup(interp, &v, ntxt);
        return Jsi_ValueString(interp, v, lenPtr);
    }
    
    key = jsi_KeyFind(interp, ntxt, 0, &isKey);
    if (key)
        Jsi_ValueMakeStringKey(interp, &v, key);
    else
        Jsi_ValueMakeString(interp, &v, ntxt);
    ntxt = v->d.s.str;
    
done:
    if (lenPtr) *lenPtr = Jsi_Strlen(ntxt);
    return ntxt;
}

Jsi_Number Jsi_ValueToNumberInt(Jsi_Interp *interp, Jsi_Value *v, int isInt)
{
    char *endPtr = NULL, *sptr;
    Jsi_Number a = 0;
    switch(v->vt) {
        case JSI_VT_BOOL:
            a = (Jsi_Number)(v->d.val ? 1.0: 0);
            break;
        case JSI_VT_NULL:
            a = 0;
            break;
        case JSI_VT_OBJECT: {
            Jsi_Obj *obj = v->d.obj;
            switch(obj->ot) {
                case JSI_OT_BOOL:
                    a = (Jsi_Number)(obj->d.val ? 1.0: 0);
                    break;
                case JSI_OT_NUMBER:
                    a = obj->d.num;
                    break;
                case JSI_OT_STRING:
                    sptr = obj->d.s.str;
                    goto donum;
                    break;
                default:
                    a = 0;
                break;
            }
            break;
        }
        case JSI_VT_UNDEF:
            a = Jsi_NumberNaN();
            break;
        case JSI_VT_NUMBER:
            a = v->d.num;
            break;
        case JSI_VT_STRING:
            sptr = v->d.s.str;
donum:
            if (!isInt) {
                a = strtod(sptr, &endPtr);
                if (endPtr && *endPtr) {
                    a = interp->NaNValue->d.num;
                }
            } else {
                a = (Jsi_Number)strtol(sptr, &endPtr, 0);
                if (!isdigit(*sptr))
                    a = interp->NaNValue->d.num;
            }
            break;
        default:
            Jsi_LogBug("Convert a unknown type: 0x%x to number", v->vt);
            break;
    }
    if (isInt && Jsi_NumberIsNormal(a))
        a = (Jsi_Number)((int64_t)(a));
    return a;
}

Jsi_RC Jsi_ValueToNumber(Jsi_Interp *interp, Jsi_Value *v)
{
    if (v->vt == JSI_VT_NUMBER) return JSI_OK;
    Jsi_Number a = Jsi_ValueToNumberInt(interp, v, 0);
    Jsi_ValueReset(interp, &v);
    Jsi_ValueMakeNumber(interp, &v, a);
    return JSI_OK;
}

Jsi_RC Jsi_ValueToBool(Jsi_Interp *interp, Jsi_Value *v)
{
    Jsi_RC rc = JSI_OK;
    bool a = 0;
    switch(v->vt) {
        case JSI_VT_BOOL:
            a = v->d.val;
            break;
        case JSI_VT_NULL:
            break;
        case JSI_VT_UNDEF:
            break;
        case JSI_VT_NUMBER:
            a = (v->d.num ? 1: 0);
            break;
        case JSI_VT_STRING:     /* TODO: NaN, and accept true/false string? */
            a = atoi(v->d.s.str);
            a = (a ? 1 : 0);
            break;
        case JSI_VT_OBJECT: {
            Jsi_Obj *obj = v->d.obj;
            switch(obj->ot) {
                case JSI_OT_BOOL:
                    a = (obj->d.val ? 1.0: 0);
                    break;
                case JSI_OT_NUMBER:
                    a = obj->d.num;
                    a = (a ? 1 : 0);
                    break;
                case JSI_OT_STRING:
                    a = atoi(obj->d.s.str);
                    a = (a ? 1 : 0);
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            Jsi_LogBug("Convert a unknown type: 0x%x to number", v->vt);
            return JSI_ERROR;
    }
    Jsi_ValueReset(interp,&v);
    Jsi_ValueMakeBool(interp, &v, a);
    return rc;
}

int jsi_ValueToOInt32(Jsi_Interp *interp, Jsi_Value *v)
{
    Jsi_Number a = Jsi_ValueToNumberInt(interp, v, 1);
    Jsi_ValueReset(interp,&v);
    Jsi_ValueMakeNumber(interp, &v, a);
    return (int)a;
}

Jsi_RC Jsi_ValueToObject(Jsi_Interp *interp, Jsi_Value *v)
{
    Jsi_RC rc = JSI_OK;
    if (v->vt == JSI_VT_OBJECT) return rc;
    Jsi_Obj *o = Jsi_ObjNew(interp);
    switch(v->vt) {
        case JSI_VT_UNDEF:
        case JSI_VT_NULL:
            if (interp->strict) {
                Jsi_LogError("converting a undefined/null value to object");
                rc = JSI_ERROR;
            }
            o->d.num = 0;
            o->ot = JSI_OT_NUMBER;
            o->__proto__ = interp->Number_prototype;
            break;
        case JSI_VT_BOOL: {
            o->d.val = v->d.val;
            o->ot = JSI_OT_BOOL;
            o->__proto__ = interp->Boolean_prototype;
            break;
        }
        case JSI_VT_NUMBER: {
            o->d.num = v->d.num;
            o->ot = JSI_OT_NUMBER;
            o->__proto__ = interp->Number_prototype;
            break;
        }
        case JSI_VT_STRING: {
            o->d.s = v->d.s;
            if (!v->f.bits.isstrkey)
                o->d.s.str = (char*)Jsi_KeyAdd(interp, v->d.s.str);
            o->isstrkey = 1;
            o->ot = JSI_OT_STRING;
            o->__proto__ = interp->String_prototype;
            break;
        }
        default:
            Jsi_LogBug("toobject, not suppose to reach here");
    }
    Jsi_ValueReset(interp,&v);
    Jsi_ValueMakeObject(interp, &v, o);
    return rc;
}

/* also toBoolean here, in ecma */
bool Jsi_ValueIsTrue(Jsi_Interp *interp, Jsi_Value *v)
{
    switch(v->vt) {
        case JSI_VT_UNDEF:
        case JSI_VT_NULL:   return 0;
        case JSI_VT_BOOL:   return v->d.val ? 1:0;
        case JSI_VT_NUMBER: 
            if (v->d.num == 0.0 || Jsi_NumberIsNaN(v->d.num)) return 0;
            return 1;
        case JSI_VT_STRING: return (Jsi_ValueStrlen(v)!=0);
        case JSI_VT_OBJECT: {
            Jsi_Obj *o = v->d.obj;
            if (o->ot == JSI_OT_STRING)
                return (Jsi_ValueStrlen(v)!=0);
            if (o->ot == JSI_OT_NUMBER)
                return (o->d.num != 0);
            if (o->ot == JSI_OT_BOOL)
                return (o->d.val != 0);
            if (o->ot == JSI_OT_USEROBJ && o->d.uobj->interp == interp) {
                return jsi_UserObjIsTrue(interp, o->d.uobj);
            }
            return 1;
        }
        default: Jsi_LogBug("TOP is type incorrect: %d", v->vt);
    }
    return 0;
}

bool Jsi_ValueIsFalse(Jsi_Interp *interp, Jsi_Value *v)
{
    if (v->vt == JSI_VT_BOOL)  return v->d.val ? 0:1;
    return 0;
}

bool Jsi_ValueIsEqual(Jsi_Interp* interp, Jsi_Value* v1, Jsi_Value* v2)
{
    int eq = 0;
    if (v1->vt == JSI_VT_OBJECT && v2->vt == JSI_VT_OBJECT && v1->d.obj == v2->d.obj)
        eq = 1;
    else if (Jsi_ValueIsNull(interp, v1) && Jsi_ValueIsNull(interp, v2))
        eq = 1;
    else if (Jsi_ValueIsUndef(interp, v1) && Jsi_ValueIsUndef(interp, v2))
        eq = 1;
    else if (Jsi_ValueIsBoolean(interp, v1) && Jsi_ValueIsBoolean(interp, v2)) {
        bool b1, b2;
        eq = (Jsi_GetBoolFromValue(interp, v1, &b1) == JSI_OK
            && Jsi_GetBoolFromValue(interp, v2, &b2) == JSI_OK
            && b1 == b2);
    } else if (Jsi_ValueIsNumber(interp, v1) && Jsi_ValueIsNumber(interp, v2)) {
        Jsi_Number n1, n2;
        eq = (Jsi_GetNumberFromValue(interp, v1, &n1) == JSI_OK
            && Jsi_GetNumberFromValue(interp, v2, &n2) == JSI_OK
            && n1 == n2);
    } else if (Jsi_ValueIsString(interp, v1) && Jsi_ValueIsString(interp, v2)) {
        const char *s1, *s2;
        int l1, l2;
        eq = (((s1=Jsi_ValueString(interp, v1, &l1)) && ((s2=Jsi_ValueString(interp, v2, &l2)))
            && l1 == l2 && Jsi_Strcmp(s1, s2)==0));
    }
    return eq;
}

void jsi_ValueToPrimitive(Jsi_Interp *interp, Jsi_Value **vPtr)
{
    Jsi_Value *v = *vPtr;
    if (v->vt != JSI_VT_OBJECT)
        return;
    DECL_VALINIT(res);
    Jsi_Value *rPtr = &res;
    Jsi_Obj *obj = v->d.obj;
    //rPtr = v;
    switch(obj->ot) {
        case JSI_OT_BOOL:
            Jsi_ValueMakeBool(interp,&rPtr, obj->d.val);
            break;
        case JSI_OT_NUMBER:
            Jsi_ValueMakeNumber(interp,&rPtr, obj->d.num);
            break;
        case JSI_OT_STRING:
            if (obj->isstrkey) {
                res.d.s = obj->d.s;
                res.f.bits.isstrkey = 1;
                obj->d.s.str = NULL;
            } else {
                if (obj->refcnt==1) {
                    Jsi_ValueMakeString(interp, &rPtr, obj->d.s.str);
                    res.d.s = obj->d.s;
                    obj->d.s.str = NULL;
                } else if (obj->d.s.len >= 0) 
                {
                    Assert(obj->refcnt>=1);
                    obj->refcnt--;
                    int bytes = obj->d.s.len;
                    jsi_ValueMakeBlobDup(interp, &rPtr, (uchar*)obj->d.s.str, bytes);
                } else
                    Jsi_ValueMakeStringDup(interp, &rPtr, obj->d.s.str);
            }
            break;
        case JSI_OT_FUNCTION: {
            Jsi_DString dStr;
            Jsi_DSInit(&dStr);
            Jsi_FuncObjToString(interp, obj->d.fobj->func, &dStr, 3);
            Jsi_ValueFromDS(interp, &dStr, &rPtr);
            break;
        }
        case JSI_OT_USEROBJ: {
            Jsi_DString dStr;
            Jsi_DSInit(&dStr);
            jsi_UserObjToName(interp, obj->d.uobj, &dStr);
            Jsi_ValueFromDS(interp, &dStr, &rPtr);
            break;
        }
        default:
            Jsi_ValueMakeStringKey(interp,&rPtr, "[object Object]");
            break;
    }
    Jsi_ValueReset(interp, vPtr);
    res.refCnt = v->refCnt;
#ifdef JSI_MEM_DEBUG
    memcpy(v, &res, sizeof(res)-sizeof(res.VD));
#else
    *v = res;
#endif
}

static void jsi_ValueToPrimitiveRes(Jsi_Interp *interp, Jsi_Value *v, Jsi_Value *rPtr)
{
    if (v->vt != JSI_VT_OBJECT) {
#ifdef JSI_MEM_DEBUG
    memcpy(rPtr, v, sizeof(*v)-sizeof(v->VD));
#else
    *rPtr = *v; //TODO: usde only by ValueCompare, so refCnt doesn't matter?
#endif
        return;
    }
    Jsi_Obj *obj = v->d.obj;
    switch(obj->ot) {
        case JSI_OT_BOOL:
            Jsi_ValueMakeBool(interp, &rPtr, obj->d.val);
            break;
        case JSI_OT_NUMBER:
            Jsi_ValueMakeNumber(interp, &rPtr, obj->d.num);
            break;
        case JSI_OT_STRING:
            rPtr->vt = JSI_VT_STRING;
            rPtr->d.s = obj->d.s;
            rPtr->f.bits.isstrkey = 1;
            break;
        default:
            break;
    }
}

int Jsi_ValueCmp(Jsi_Interp *interp, Jsi_Value *v1, Jsi_Value* v2, int flags)
{
    DECL_VALINIT(res1);
    DECL_VALINIT(res2);
    int r = 1;
    int nocase = (flags&JSI_SORT_NOCASE), dict = ((flags & JSI_SORT_DICT));
    if (v1 == v2)
        return 1;
    if (v1->vt != v2->vt) {
        jsi_ValueToPrimitiveRes(interp, v1, &res1);
        jsi_ValueToPrimitiveRes(interp, v2, &res2);
        v1 = &res1;
        v2 = &res2;
    }
    if (v1->vt != v2->vt) {
        if ((flags&JSI_CMP_EXACT))
            return 1;
        if ((v1->vt == JSI_VT_UNDEF || v1->vt == JSI_VT_NULL) && 
            (v2->vt == JSI_VT_UNDEF || v2->vt == JSI_VT_NULL)) {
            r = 0;
        } else {
            Jsi_Number n1, n2;
            n1 = Jsi_ValueToNumberInt(interp, v1, 0);
            n2 = Jsi_ValueToNumberInt(interp, v2, 0);
            r = (n2 - n1);
        }
    } else {
        switch (v1->vt) {
            case JSI_VT_NUMBER:
                if (v2->d.num == v1->d.num) return 0;
                r = (v2->d.num < v1->d.num ? -1 : 1);
                break;
            case JSI_VT_BOOL:
                r = (v2->d.val - v1->d.val);
                break;
            case JSI_VT_STRING:
                r = (Jsi_StrcmpDict(v2->d.s.str, v1->d.s.str, nocase, dict));
                break;
            case JSI_VT_OBJECT:
                /* TODO: refer to objects joined to each other */
                if (v2->vt != JSI_VT_OBJECT)
                    r = 1;
                else if (v1->d.obj->ot == JSI_OT_STRING && v2->d.obj->ot == JSI_OT_STRING)
                    r = (Jsi_StrcmpDict(v2->d.obj->d.s.str, v1->d.obj->d.s.str, nocase, dict));
                else
                    r = (v2->d.obj - v1->d.obj);
                break;
            case JSI_VT_UNDEF:
            case JSI_VT_NULL:
                r = 0;
                break;
            default:
                Jsi_LogBug("Unexpected value type");
        }
    }
    return r;
}

/**
 * @brief Split a string.
 * @param interp 
 * @param str - input string to split
 * @param split - to split on
 * @returns an array of string values
 * 
 * 
 */
Jsi_Value *Jsi_StringSplit(Jsi_Interp *interp, const char *str, const char *spliton)
{
    char **argv; int argc;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_SplitStr(str, &argc, &argv, spliton, &dStr);
    Jsi_Value *nret = Jsi_ValueNewArray(interp, NULL, 0);
    Jsi_Obj *obj = nret->d.obj;
    int i;
    for (i = 0; i < argc; ++i) {
        Jsi_ObjArraySet(interp, obj, Jsi_ValueNewStringDup(interp, argv[i]), i);
    }
    Jsi_ObjSetLength(interp, obj, argc);
    Jsi_ValueMakeArrayObject(interp, &nret, obj);
    Jsi_DSFree(&dStr);
    return nret;
}

void jsi_ValueObjSet(Jsi_Interp *interp, Jsi_Value *target, const char *key, Jsi_Value *value, int flags, int isstrkey)
{
    Jsi_TreeEntry *hPtr;
    if (target->vt != JSI_VT_OBJECT) {
        if (interp->strict)
            Jsi_LogWarn("Target is not object: %d", target->vt);
        return;
    }
    hPtr = Jsi_ObjInsert(interp, target->d.obj, key, value, (isstrkey?JSI_OM_ISSTRKEY:0));
    if (!hPtr)
        return;
    hPtr->f.flags |= (flags&JSI_TREE_USERFLAG_MASK);
}

Jsi_Value *jsi_ValueObjKeyAssign(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *keyval, Jsi_Value *value, int flag)
{
    int arrayindex = -1;

    if (keyval->vt == JSI_VT_NUMBER && Jsi_NumberIsInteger(keyval->d.num) && keyval->d.num >= 0) {
        arrayindex = (int)keyval->d.num;
    }
    /* TODO: array["1"] also extern the length of array */
    
    if (arrayindex >= 0 && (uint)arrayindex < interp->maxArrayList &&
        target->vt == JSI_VT_OBJECT && target->d.obj->arr) {
        return jsi_ObjArraySetDup(interp, target->d.obj, value, arrayindex);
    }
    const char *kstr = Jsi_ValueToString(interp, keyval, NULL);
    
#if (defined(JSI_HAS___PROTO__) && JSI_HAS___PROTO__==2)
    if (Jsi_Strcmp(kstr, "__proto__")==0) {
        Jsi_Obj *obj = target->d.obj;
        obj->__proto__ = Jsi_ValueDup(interp, value);
        //obj->clearProto = 1;
        return obj->__proto__;
    }
#endif
    Jsi_Value *v = Jsi_ValueNew1(interp);
    if (value)
        Jsi_ValueCopy(interp, v, value);

    jsi_ValueObjSet(interp, target, kstr, v, flag, (Jsi_ValueIsStringKey(interp, keyval)? JSI_OM_ISSTRKEY:0));
    Jsi_DecrRefCount(interp, v);
    return v;
}

static Jsi_Value *jsi_ValueLookupBase(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *key, Jsi_Value **ret)
{
    if (!target)
        return NULL;
    if (target->vt != JSI_VT_OBJECT) {
        Jsi_LogError("subscript operand is not object");
        return NULL;
    }
    const char *keyStr = Jsi_ValueToString(interp, key, NULL);
    if (!keyStr)
        return NULL;
    bool isStrKey = (key->vt == JSI_VT_STRING && key->f.bits.isstrkey);
    Jsi_Value *v = Jsi_ValueObjLookup(interp, target, (char*)keyStr, isStrKey);
    if (v)
        return v;
    if (target->d.obj->__proto__)
        return jsi_ValueLookupBase(interp, target->d.obj->__proto__, key, ret);
    return NULL;
}


Jsi_Value* jsi_ValueSubscript(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *key, Jsi_Value **ret)
{
    int len;
    Jsi_ValueReset(interp, ret);
    Jsi_Value *v = jsi_ValueLookupBase(interp, target, key, ret);
    if (v)
        return v;
    const char *keyStr = Jsi_ValueString(interp, key, NULL);
    if (!keyStr)
        return NULL;
    // Special cases such as "length", "constructor", etc...
    if (Jsi_Strcmp(keyStr,"length")==0) {
        if (Jsi_ValueIsString(interp, target)) {
            len = Jsi_ValueStrlen(target);
        } else if (target->vt == JSI_VT_OBJECT && target->d.obj->isarrlist) {
            len = target->d.obj->arrCnt;
        } else if (target->vt == JSI_VT_OBJECT && target->d.obj->ot == JSI_OT_FUNCTION) {
            Jsi_Func *fo = target->d.obj->d.fobj->func;
            if (fo->type == FC_NORMAL)
                len = fo->argnames->count;
            else
                len = fo->cmdSpec->maxArgs, len = (len>=0?len:fo->cmdSpec->minArgs);
        } else if (target->vt == JSI_VT_OBJECT && target->d.obj->tree) {
            len = target->d.obj->tree->numEntries;
        } else {
            return NULL;
        }
        (*ret)->vt = JSI_VT_NUMBER;
        (*ret)->d.num = (Jsi_Number)len;
        return *ret;
    }

    if (target->vt == JSI_VT_OBJECT && (interp->subOpts.noproto==0 && Jsi_Strcmp(keyStr,"constructor")==0)) {
        const char *cp;
        Jsi_Obj *o = target->d.obj->constructor;
        if (o) {
            if (o->ot == JSI_OT_FUNCTION) {
                Jsi_Value *proto = Jsi_TreeObjGetValue(o, "prototype", 0);
                if (proto && proto->vt == JSI_VT_OBJECT && proto->d.obj->constructor) {
                    o = proto->d.obj->constructor;
                }
            }
        } else {
            switch(target->d.obj->ot) {
                case JSI_OT_NUMBER:
                    cp = "Number";
                    break;
                case JSI_OT_BOOL:
                    cp = "Boolean";
                    break;
                case JSI_OT_STRING:
                    cp = "String";
                    break;
                case JSI_OT_REGEXP:
                    cp = "RegExp";
                    break;
                case JSI_OT_OBJECT:
                    if (target->d.obj->isarrlist) {
                        cp = "Array";
                        break;
                    }
                    cp = "Object";
                    break;
                default:
                    Jsi_ValueMakeUndef(interp, ret);
                    return *ret;
            }
            v = Jsi_ValueObjLookup(interp, interp->csc, cp, 0);
            if (v==NULL || v->vt != JSI_VT_OBJECT)
                return NULL;
            o = target->d.obj->constructor = v->d.obj;
        }
        Jsi_ValueMakeObject(interp, ret, o);
        return *ret;
    }

    if (target->vt == JSI_VT_OBJECT && target->d.obj->ot == JSI_OT_FUNCTION) {
        /* Looking up something like "String.substr" */
        Jsi_Func* func = target->d.obj->d.fobj->func;
        if (func->type == FC_BUILDIN) {
            if (func->f.bits.iscons && func->name) {
                Jsi_Value *v = Jsi_ValueObjLookup(interp, interp->csc, (char*)func->name, 0);
                if (!v) {
                } else {
                    bool ooo = interp->subOpts.noproto;
                    interp->subOpts.noproto = 0;
                    v = Jsi_ValueObjLookup(interp, v, "prototype", 0);
                    interp->subOpts.noproto = ooo;
                    
                    if (v && ((v = Jsi_ValueObjLookup(interp, v, (char*)keyStr, 0)))) {
                        if (v->vt == JSI_VT_OBJECT && v->d.obj->ot == JSI_OT_FUNCTION && Jsi_Strcmp(func->name,"Interp")) {
                            Jsi_Func* sfunc = v->d.obj->d.fobj->func;
                            /* Handle "Math.pow(2,3)", "String.fromCharCode(0x21)", ... */
                            sfunc->callflags.bits.addargs = 1;
                        }
                        return v;
                    }
                }
            }
            if (Jsi_ValueIsString(interp, key)) {
                char *kstr = Jsi_ValueString(interp, key, NULL);
                if (!Jsi_Strcmp(kstr,"call") || !Jsi_Strcmp(kstr,"apply") || !Jsi_Strcmp(kstr,"bind")) {
                    char fbuf[JSI_MAX_NUMBER_STRING];
                    snprintf(fbuf, sizeof(fbuf), "Function.%s", kstr);
                    Jsi_Value *vv = Jsi_NameLookup(interp, fbuf);
                    if (vv)
                        return vv;
                }
            }
        }
    }
    return NULL;
}

bool Jsi_ValueKeyPresent(Jsi_Interp *interp, Jsi_Value *target, const char *key, int isstrkey)
{
    SIGASSERT(interp,INTERP);
    //SIGASSERT(target,VALUE);
    if (Jsi_TreeObjGetValue(target->d.obj, key, isstrkey))
        return 1;
    if (target->d.obj->__proto__ == NULL || target->d.obj->__proto__ == target)
        return 0;
    return Jsi_ValueKeyPresent(interp, target->d.obj->__proto__, key, isstrkey);
}

void jsi_ValueObjGetKeys(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *ret, bool isof)
{
    Jsi_IterObj *io = Jsi_IterObjNew(interp, NULL);
    Jsi_Obj *to = target->d.obj;
    
    if (target->vt != JSI_VT_UNDEF && target->vt != JSI_VT_NULL) {

        if (target->vt == JSI_VT_OBJECT && to->arr) {
            io->isArrayList = 1;
            io->count = to->arrCnt;
        } else {
            if (isof &&interp->strict)
                Jsi_LogWarn("non-array in 'for...of'");
            Jsi_IterGetKeys(interp, target, io, 0);
        }
    }
    io->obj = to;
    io->isof = isof;
    Jsi_Obj *r = Jsi_ObjNew(interp);
    r->ot = JSI_OT_ITER;
    r->d.iobj = io;
    Jsi_ValueMakeObject(interp, &ret, r);
}

Jsi_RC Jsi_ValueGetKeys(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *ret)
{
    uint i, n = 0;
    Jsi_IterObj *io;
    if (target->vt != JSI_VT_OBJECT)
        return JSI_ERROR;
    Jsi_Obj *to = target->d.obj;
    Jsi_Obj *r = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    Jsi_ValueMakeArrayObject(interp, &ret, r);
    if (to->arr) {
        for (i=0; i<to->arrCnt; i++)
            if (to->arr[i]) n++;
        if (Jsi_ObjArraySizer(interp, r, n) <= 0) {
            Jsi_LogError("too long");
            Jsi_ValueMakeUndef(interp, &ret);
            return JSI_ERROR;
        }
        for (i=0, n=0; i<to->arrCnt; i++) {
            if (to->arr[i]) {
                r->arr[n] = Jsi_ValueNewNumber(interp, (Jsi_Number)i);
                Jsi_IncrRefCount(interp, r->arr[n]);
                n++;
            }
        }
        r->arrCnt = n;
        return JSI_OK;
    }
    io = Jsi_IterObjNew(interp, NULL);
    Jsi_IterGetKeys(interp, target, io, 0);
    if (Jsi_ObjArraySizer(interp, r, io->count) <= 0) {
        Jsi_LogError("too long");
        Jsi_ValueMakeUndef(interp, &ret);
        return JSI_ERROR;
    }
    for (i=0; i<io->count; i++) {
        r->arr[i] = (io->keys[i] ? Jsi_ValueNewStringKey(interp, io->keys[i]) : NULL);
        Jsi_IncrRefCount(interp, r->arr[i]);
    }
    io->count = 0;
    r->arrCnt = i;
    Jsi_IterObjFree(io);
    return JSI_OK;
}

jsi_ScopeChain *jsi_ScopeChainNew(Jsi_Interp *interp, int cnt)
{
    jsi_ScopeChain *r = (jsi_ScopeChain *)Jsi_Calloc(1, sizeof(*r));
    r->interp = interp;
    SIGINIT(r,SCOPE);
    r->chains = (Jsi_Value **)Jsi_Calloc(cnt, sizeof(r->chains[0]));
    r->chains_cnt = cnt;
    return r;
}

Jsi_Value *jsi_ScopeChainObjLookupUni(jsi_ScopeChain *sc, char *key)
{
    int i;
    Jsi_Value *ret;
    for (i = sc->chains_cnt - 1; i >= 0; --i) {
        if ((ret = Jsi_ValueObjLookup(sc->interp, sc->chains[i], key, 0))) {
            return ret;
        }
    }
    return NULL;
}

jsi_ScopeChain *jsi_ScopeChainDupNext(Jsi_Interp *interp, jsi_ScopeChain *sc, Jsi_Value *next)
{
    if (!sc) {
        jsi_ScopeChain *nr = jsi_ScopeChainNew(interp, 1);
        nr->chains[0] = next;
        Jsi_IncrRefCount(interp, next);
        nr->chains_cnt = 1;
        return nr;
    }
    jsi_ScopeChain *r = jsi_ScopeChainNew(interp, sc->chains_cnt + 1);
    int i;
    for (i = 0; i < sc->chains_cnt; ++i) {
        r->chains[i] = sc->chains[i];
        Jsi_IncrRefCount(interp, sc->chains[i]);
    }
    r->chains[i] =  next;
    Jsi_IncrRefCount(interp, next);
    r->chains_cnt = i + 1;
    return r;
}

void jsi_ScopeChainFree(Jsi_Interp *interp, jsi_ScopeChain *sc)
{
    int i;
    for (i = 0; i < sc->chains_cnt; ++i) {
        Jsi_DecrRefCount(interp, sc->chains[i]);
    }
    Jsi_Free(sc->chains);
    _JSI_MEMCLEAR(sc);
    Jsi_Free(sc);
}

int Jsi_ValueGetLength(Jsi_Interp *interp, Jsi_Value *v) {
    if (Jsi_ValueIsArray(interp, v))
        return v->d.obj->arrCnt;
    Jsi_LogWarn("expected array");
    return 0;
}

char *Jsi_ValueArrayIndexToStr(Jsi_Interp *interp, Jsi_Value *args, int index, int *lenPtr)
{
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, index);
    if (!arg)
        return NULL;
    char *res = Jsi_ValueString(interp, arg, lenPtr);
    if (res)
        return res;
    res = (char*)Jsi_ValueToString(interp, arg, NULL);
    if (res && lenPtr)
        *lenPtr = Jsi_Strlen(res);
    return res;
}

Jsi_RC Jsi_ValueInsert(Jsi_Interp *interp, Jsi_Value *target, const char *key, Jsi_Value *val, int flags)
{
    if (target == NULL)
        target = interp->csc;
    if (target->vt != JSI_VT_OBJECT) {
        if (interp->strict)
            Jsi_LogWarn("Target is not object");
        return JSI_ERROR;
    }
    target->f.flag |= flags;
    if (Jsi_ObjInsert(interp, target->d.obj, key, val, flags))
        return JSI_OK;
    return JSI_ERROR;
}

Jsi_RC Jsi_ValueInsertArray(Jsi_Interp *interp, Jsi_Value *target, int key, Jsi_Value *val, int flags)
{
    if (target->vt != JSI_VT_OBJECT) {
        if (interp->strict)
            Jsi_LogWarn("Target is not object");
        return JSI_ERROR;
    }
    Jsi_Obj *obj = target->d.obj;
    
    if (obj->isarrlist) {
        if (key >= 0 && (uint)key < interp->maxArrayList) {
            Jsi_ObjArraySet(interp, obj, val, key);
            return JSI_OK;
        }
        return JSI_ERROR;
    }
    char unibuf[JSI_MAX_NUMBER_STRING];
    Jsi_NumberItoA10(key, unibuf, sizeof(unibuf));
    Jsi_ObjInsert(interp, obj, unibuf, val, flags);
    return JSI_OK;
}

/* OBJ INTERFACE TO BTREE */

static void IterObjInsertKey(Jsi_IterObj *io, const char *key)
{
    assert(!io->isArrayList);
    if (io->depth) {
        uint i;
        for (i=0; i<io->count; i++) {
            if (!Jsi_Strcmp(key, io->keys[i]))
                return;
        }
    }

    if (io->count >= io->size) {
        io->size += 10;
        io->keys = (const char**)Jsi_Realloc(io->keys, io->size * sizeof(io->keys[0]));
    }
    io->keys[io->count] = key;
    io->count++;
}
static void IterObjInsert(Jsi_IterObj *io, Jsi_TreeEntry *hPtr)
{
    IterObjInsertKey(io, (const char*)Jsi_TreeKeyGet(hPtr));
}

Jsi_TreeEntry * Jsi_ObjInsert(Jsi_Interp *interp, Jsi_Obj *obj, const char *key, Jsi_Value *val, int flags)
{
    Jsi_TreeEntry *hPtr;
    SIGASSERT(val, VALUE);
    /*if (val)
        Jsi_IncrRefCount(interp, val);*/
    hPtr = Jsi_TreeObjSetValue(obj, key, val, (flags&JSI_OM_ISSTRKEY));
    if ((flags&JSI_OM_DONTDEL))
        val->f.bits.dontdel = hPtr->f.bits.dontdel = 1;
    if ((flags&JSI_OM_READONLY))
        val->f.bits.readonly =hPtr->f.bits.readonly = 1;
    if ((flags&JSI_OM_DONTENUM))
        val->f.bits.dontenum =hPtr->f.bits.dontenum = 1;
    return hPtr;
}

static Jsi_RC IterGetKeysCallback(Jsi_Tree* tree, Jsi_TreeEntry *hPtr, void *data)
{
    Jsi_IterObj *io = (Jsi_IterObj *)data;
    if (!hPtr->f.bits.dontenum) {
        IterObjInsert(io, hPtr);
    }
    return JSI_OK;
}

void Jsi_IterGetKeys(Jsi_Interp *interp, Jsi_Value *target, Jsi_IterObj *iterobj, int depth)
{
    if (!target) return;
    if (target->vt != JSI_VT_OBJECT) {
        if (interp->strict)
            Jsi_LogWarn("operand is not a object");
        return;
    }
    Jsi_Obj *to = target->d.obj;
    Jsi_CmdSpec *cs = NULL;
    if (to->ot == JSI_OT_USEROBJ) {
        Jsi_UserObj *uobj = to->d.uobj;
        cs = uobj->reg->spec;
    } else if (to->ot == JSI_OT_FUNCTION) {
        Jsi_FuncObj *fobj = to->d.fobj;
        if (fobj->func->type == FC_BUILDIN)
            cs = fobj->func->cmdSpec;
    }
    if (cs) {
        while (cs->name) {
            IterObjInsertKey(iterobj, cs->name);
            cs++;
        }
        return;
    }
    iterobj->depth = depth;
    Jsi_TreeWalk(target->d.obj->tree, IterGetKeysCallback, iterobj, 0);
    if (target->d.obj->__proto__ && target != target->d.obj->__proto__)
        Jsi_IterGetKeys(interp, target->d.obj->__proto__, iterobj, depth+1);
    iterobj->depth = depth;
}

Jsi_Value* Jsi_ValueMakeDStringObject(Jsi_Interp *interp, Jsi_Value **vPtr, Jsi_DString *dsPtr)  {
    Jsi_Value *v = (vPtr?*vPtr:NULL);
    Jsi_Obj *obj;
    if (!v)
        v = Jsi_ValueNew(interp);
    else {
        assert(v->vt <= JSI_VT__MAX);
        if (v->vt == JSI_VT_OBJECT && v->d.obj->ot == JSI_OT_STRING
            && v->d.obj->refcnt == 1
        ) {
            Jsi_ObjFromDS(dsPtr, v->d.obj);
            return v;
        }
        Jsi_ValueReset(interp, &v);
    }
    obj = Jsi_ObjNewType(interp, JSI_OT_STRING);
    Jsi_ObjFromDS(dsPtr, obj);
    Jsi_ValueMakeObject(interp, &v, obj);
    return v;
}

Jsi_Value* Jsi_ValueMakeObject(Jsi_Interp *interp, Jsi_Value **vPtr, Jsi_Obj *o)  {
    Jsi_Value *v = (vPtr?*vPtr:NULL);
    if (v && v->vt == JSI_VT_OBJECT && o == v->d.obj)
        return v;
    if (v)
        Jsi_ValueReset(interp, vPtr);
    else
        v = Jsi_ValueNew(interp);
    //Jsi_IncrRefCount(interp, v);
    if (!o)
        o = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
    v->vt = JSI_VT_OBJECT;
    v->d.obj = o;
    Jsi_ObjIncrRefCount(interp,v->d.obj);
    return v;
}

Jsi_Value* Jsi_ValueMakeArrayObject(Jsi_Interp *interp, Jsi_Value **vPtr, Jsi_Obj *o)  {
    Jsi_Value *v = (vPtr?*vPtr:NULL);
    if (!o)
        o = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    if (!v)
        v = Jsi_ValueNew(interp);
    else {
       if (v->vt == JSI_VT_OBJECT && o == v->d.obj) {
            if (!o->isarrlist) {
                if (o->tree)
                    Jsi_TreeDelete( o->tree);
                o->tree = NULL;
                o->__proto__ = interp->Array_prototype;
                o->isarrlist = 1;
            }
            return v;
        }
        Jsi_ValueReset(interp, vPtr);
    }
    v->vt = JSI_VT_OBJECT;
    v->d.obj = o;
    o->ot = JSI_OT_OBJECT;
    o->__proto__ = interp->Array_prototype;
    o->isarrlist = 1;
    Jsi_ObjArraySizer(interp, o, 0);
    Jsi_ObjIncrRefCount(interp,v->d.obj);
    return v;
}

Jsi_Value* Jsi_ValueMakeNumber(Jsi_Interp *interp, Jsi_Value **vPtr, Jsi_Number n) {
    Jsi_Value *v = (vPtr?*vPtr:NULL);
    if (!v)
        v = Jsi_ValueNew(interp);
    else
        Jsi_ValueReset(interp, vPtr);
    v->vt = JSI_VT_NUMBER;
    v->d.num = n;
    return v;
}

Jsi_Value* Jsi_ValueMakeBool(Jsi_Interp *interp, Jsi_Value **vPtr, int b) {
    Jsi_Value *v = (vPtr?*vPtr:NULL);
    if (!v)
        v = Jsi_ValueNew(interp);
    else
        Jsi_ValueReset(interp, vPtr);
    v->vt = JSI_VT_BOOL;
    v->d.val = b;
    return v;
}

Jsi_Value* Jsi_ValueMakeBlob(Jsi_Interp *interp, Jsi_Value **vPtr, unsigned char *s, int len) {
    Jsi_Value *v = (vPtr?*vPtr:NULL);
    if (!v)
        v = Jsi_ValueNew(interp);
    else
        Jsi_ValueReset(interp, vPtr);
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_STRING);
    Jsi_ValueMakeObject(interp, &v, obj);
    obj->d.s.str = (char*)s;
    obj->d.s.len = len;
    obj->isBlob = 1;
    return v;
}
Jsi_Value* jsi_ValueMakeBlobDup(Jsi_Interp *interp, Jsi_Value **ret, unsigned char *s, int len) {
    if (len<0) len = Jsi_Strlen((char*)s);
    uchar *dp = (uchar*)Jsi_Malloc(len+1);
    memcpy(dp, s, len);
    dp[len] = 0;
    return Jsi_ValueMakeBlob(interp, ret, dp, len);
}


Jsi_Value* Jsi_ValueMakeString(Jsi_Interp *interp, Jsi_Value **vPtr, const char *s) {
    return Jsi_ValueMakeBlob(interp, vPtr, (unsigned char *)s, Jsi_Strlen(s));
}

Jsi_Value* Jsi_ValueMakeStringKey(Jsi_Interp *interp, Jsi_Value **vPtr, const char *s) {
    Jsi_Value *v = (vPtr?*vPtr:NULL);
    if (!v)
        v = Jsi_ValueNew(interp);
    else
        Jsi_ValueReset(interp, vPtr);
    v->vt = JSI_VT_STRING;
    v->d.s.str = (char*)Jsi_KeyAdd(interp,s);
    v->d.s.len = Jsi_Strlen(s);
    v->f.bits.isstrkey = 1;
    return v;
}

Jsi_Value* Jsi_ValueMakeNull(Jsi_Interp *interp, Jsi_Value **vPtr) {
    Jsi_Value *v = (vPtr?*vPtr:NULL);
    if (!v)
        v = Jsi_ValueNew(interp);
    else
        Jsi_ValueReset(interp, vPtr);
    v->vt = JSI_VT_NULL;
    return v;
}

Jsi_Value* Jsi_ValueMakeUndef(Jsi_Interp *interp, Jsi_Value **vPtr) {
    Jsi_Value *v = (vPtr?*vPtr:NULL);
    if (!v)
        v = Jsi_ValueNew(interp);
    else {
        if (v->vt == JSI_VT_UNDEF) return v;
        Jsi_ValueReset(interp, vPtr);
    }
    return v;
}

Jsi_Value* Jsi_ValueNewNumber(Jsi_Interp *interp, Jsi_Number n) {
    Jsi_Value *v = Jsi_ValueNew(interp);
    v->vt = JSI_VT_NUMBER;
    v->d.num = n;
    return v;
}

Jsi_Value* Jsi_ValueNewObj(Jsi_Interp *interp, Jsi_Obj *o) {
    Jsi_Value *v = Jsi_ValueNew(interp);
    Jsi_ValueMakeObject(interp, &v, o);
    return v;
}

Jsi_Value* Jsi_ValueNewString(Jsi_Interp *interp, const char *s, int len) {
    assert(s);
    Jsi_Value *v = Jsi_ValueNew(interp);
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_STRING);
    Jsi_ValueMakeObject(interp, &v, obj);
    obj->d.s.str = (char*)s;
    obj->d.s.len = (len<0?Jsi_Strlen(s):(uint)len);
    return v;
}

Jsi_Value* Jsi_ValueNewStringDup(Jsi_Interp *interp, const char *s) {
    return Jsi_ValueNewString(interp, Jsi_Strdup(s), -1);
}

Jsi_Value* Jsi_ValueNewStringKey(Jsi_Interp *interp, const char *s) {
    Jsi_Value *v = Jsi_ValueNew(interp);
    v->vt = JSI_VT_STRING;
    v->d.s.str = (char*)Jsi_KeyAdd(interp,s);
    v->d.s.len = Jsi_Strlen(s);
    v->f.bits.isstrkey = 1;
    return v;
}


Jsi_Value* Jsi_ValueNewStringConst(Jsi_Interp *interp, const char *s, int len) {
    Jsi_Value *v = Jsi_ValueNew(interp);
    v->vt = JSI_VT_STRING;
    v->d.s.str = (char*)s;
    v->d.s.len = (len<0?Jsi_Strlen(s):(uint)len);
    v->f.bits.isstrkey = 1;
    return v;
}

Jsi_Value* Jsi_ValueNewBlob(Jsi_Interp *interp, unsigned char *s, uint len) {
    Jsi_Value *v = Jsi_ValueNew(interp);
    Jsi_Obj *o = Jsi_ObjNewType(interp, JSI_OT_STRING);
    Jsi_ValueMakeObject(interp, &v, o);
    o->d.s.str = (char*)Jsi_Malloc(len+1);
    memcpy(o->d.s.str, (char*)s, len);
    o->d.s.str[len] = 0;
    o->d.s.len = len;
    o->isBlob = 1;
    return v;
}

Jsi_Value* Jsi_ValueNewBoolean(Jsi_Interp *interp, int bval) {
    Jsi_Value *v = Jsi_ValueNew(interp);
    v->vt = JSI_VT_BOOL;
    v->d.val = bval;
    return v;
}

Jsi_Value* Jsi_ValueNewNull(Jsi_Interp *interp) {
    Jsi_Value *v = Jsi_ValueNew(interp);
    v->vt = JSI_VT_NULL;
    return v;
}

Jsi_Value *Jsi_ValueNewArray(Jsi_Interp *interp, const char **items, int count)
{
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    int i = 0;
    if (count<0) {
        count = 0;
        while (items[count])
            count++;
    }
    if (Jsi_ObjArraySizer(interp, obj, count) <= 0) {
        Jsi_ObjFree(interp, obj);
        return NULL;
    }
    for (i = 0; i < count; ++i) {
        obj->arr[i] = Jsi_ValueNewStringDup(interp, items[i]);
        Jsi_IncrRefCount(interp, obj->arr[i]);
    }
    obj->arrCnt = count;
    assert(obj->arrCnt<=obj->arrMaxSize);
    return Jsi_ValueMakeArrayObject(interp, NULL, obj);
}

Jsi_Obj *Jsi_ValueGetObj(Jsi_Interp *interp, Jsi_Value* v)
{
    if (v->vt == JSI_VT_OBJECT) {
        return v->d.obj;
    }
    return NULL;
}

int Jsi_ValueStrlen(Jsi_Value* v) {
    //if (v->vt == JSI_VT_OBJECT && v->d.obj->ot == JSI_OT_STRING && v->d.obj->isBlob)
    //    return v->d.obj->d.s.len;
    Jsi_String *s = jsi_ValueString(v);
    if (s == 0 || s->str == 0)
        return 0;
#if JSI__UTF8
    return (int)Jsi_NumUtfChars(s->str, s->len);
#else
    if (s->len>=0) return s->len;
    return (int)Jsi_NumUtfChars(s->str, s->len);
#endif
}

char *Jsi_ValueString(Jsi_Interp *interp, Jsi_Value* v, int *lenPtr)
{
    if (!v) return NULL;
    Jsi_String *s = jsi_ValueString(v);
    if (s) {
        if (lenPtr)
            *lenPtr = (s->len<0 ? (int)Jsi_Strlen(s->str) : s->len);
        return s->str;
    }
    if (lenPtr)
        *lenPtr = 0;
    return NULL;
}

unsigned char *Jsi_ValueBlob(Jsi_Interp *interp, Jsi_Value* v, int *lenPtr)
{
    return (unsigned char*)Jsi_ValueString(interp, v, lenPtr);
}

char* Jsi_ValueGetStringLen(Jsi_Interp *interp, Jsi_Value *pv, int *lenPtr)
{
    if (!pv)
        return NULL;
    Jsi_String *s = jsi_ValueString(pv);
    if (!s)
        return NULL;
    if (lenPtr)
        *lenPtr = (s->len<0 ? (int)Jsi_Strlen(s->str) : s->len);
    return s->str;
}

int Jsi_ValueInstanceOf( Jsi_Interp *interp, Jsi_Value* v1, Jsi_Value* v2)
{
    Jsi_Value *proto, *sproto;
    if (v1->vt != JSI_VT_OBJECT || v2->vt != JSI_VT_OBJECT  || v2->d.obj->ot != JSI_OT_FUNCTION)
        return 0;
    proto = Jsi_ValueObjLookup(interp, v2, "prototype", 0);
    if (!proto)
        return 0;
    sproto = v1->d.obj->__proto__ ;
    while (sproto) {
        if (sproto == proto)
            return 1;
        if (sproto->vt != JSI_VT_OBJECT)
            return 0;
        sproto = sproto->d.obj->__proto__;
    }
    return 0;
}


Jsi_RC jsi_InitValue(Jsi_Interp *interp, int release)
{
    return JSI_OK;
}

void  Jsi_ValueFromDS(Jsi_Interp *interp, Jsi_DString *dsPtr, Jsi_Value **ret)
{
    char *cp = NULL;
    int len = Jsi_DSLength(dsPtr);
    if (len && !(cp=(char*)dsPtr->strA)) 
        cp = Jsi_StrdupLen(dsPtr->Str, len);
    dsPtr->strA = NULL;
    dsPtr->Str[0] = 0;
    dsPtr->len = 0;
    dsPtr->spaceAvl = dsPtr->staticSize;
    if (!cp)
        Jsi_ValueMakeStringDup(interp, ret, "");
    else
        Jsi_ValueMakeBlob(interp, ret, (uchar*)cp, len);
}

#endif
