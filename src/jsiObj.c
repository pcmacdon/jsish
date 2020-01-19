#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

/******************* TREE ACCESS **********************/

Jsi_Value *Jsi_TreeObjGetValue(Jsi_Obj* obj, const char *key, int isstrkey) {
    Jsi_Tree *treePtr = obj->tree;
    
    if (!isstrkey) {
        Jsi_MapEntry *hPtr = Jsi_MapEntryFind(treePtr->opts.interp->strKeyTbl, key);
        if (!hPtr)
            return NULL;
        key = (const char*)Jsi_MapKeyGet(hPtr, 0);
    }
    Jsi_Value *v = (Jsi_Value*)Jsi_TreeGet(treePtr, (void*)key, 0);
    return v;
}

Jsi_TreeEntry *Jsi_TreeObjSetValue(Jsi_Obj *obj, const char *key, Jsi_Value *val, int isstrkey) {
    Jsi_Tree *treePtr = obj->tree;
    bool isNew;
    Jsi_TreeEntry *hPtr;
    Jsi_Interp *interp = treePtr->opts.interp;
    if (!isstrkey) {
        const char *okey = key;
        Jsi_MapEntry *hePtr = Jsi_MapEntryNew(interp->strKeyTbl, key, &isNew);
        key = (const char*)Jsi_MapKeyGet(hePtr, 0);
        if (!key) {
            Jsi_MapEntry *hePtr = Jsi_MapEntryNew(interp->strKeyTbl, okey, &isNew);
            key = (const char*)Jsi_MapKeyGet(hePtr, 0);
        }
    }
    //return Jsi_TreeSet(treePtr, key, val);
    hPtr = Jsi_TreeEntryNew(treePtr, key, &isNew);
    if (!hPtr)
        return NULL;
    if (val)
        SIGASSERT(val,VALUE);
    if (!isNew)
        Jsi_ValueReplace(interp, (Jsi_Value**)&(hPtr->value), val);
    else {
        hPtr->value = val;
        if (val)
            Jsi_IncrRefCount(interp, val);
    }
//    Jsi_Value *oldVal;  /* FYI: This let kitty.breed() work in tests/proto2.js */
//    Assert(val->refCnt>0);
//    if (!isNew) {
//        oldVal = Jsi_TreeValueGet(hPtr);
//        if (oldVal) {
//            Jsi_ValueReset(interp, &oldVal);
//            Jsi_ValueCopy(interp, oldVal, val);
//        }
//    }
//    else
//        hPtr->value = val;

    return hPtr;
}

/*****************************************/

bool Jsi_ObjIsArray(Jsi_Interp *interp, Jsi_Obj *o)  {
    return ((o)->ot == JSI_OT_OBJECT && o->isarrlist);
}

static Jsi_RC ObjListifyCallback(Jsi_Tree *tree, Jsi_TreeEntry *hPtr, void *data)
{
    Jsi_Interp *interp = tree->opts.interp;
    Jsi_Obj *obj = (Jsi_Obj*)data;
    int n;
    if (!hPtr->f.bits.dontenum) {
        char *ep = NULL, *cp = (char*)Jsi_TreeKeyGet(hPtr);
        if (!cp || !isdigit(*cp))
            return JSI_OK;
        n = (int)strtol(cp, &ep, 0);
        if (n<0 || n >= interp->maxArrayList)
            return JSI_OK;
        hPtr->f.bits.isarrlist = 1;
        if (Jsi_ObjArraySizer(interp, obj, n) <= 0) 
            return Jsi_LogError("too long");
        obj->arr[n] = (Jsi_Value*)Jsi_TreeValueGet(hPtr);
       // obj->arrCnt++;
    }
    return JSI_OK;
}

static Jsi_RC ObjListifyArrayCallback(Jsi_Tree *tree, Jsi_TreeEntry *hPtr, void *data)
{
    if (hPtr->f.bits.isarrlist) {
        Jsi_TreeEntryDelete(hPtr);
        tree->opts.interp->delRBCnt++;
        return JSI_ERROR;
    }
    return JSI_OK;
}

void Jsi_ObjListifyArray(Jsi_Interp *interp, Jsi_Obj *obj)
{
    if (!obj->isarrlist) {
        Jsi_LogBug("Can not listify a non-array");
        return;
    }
    if (obj->arr) return;
    Jsi_TreeWalk(obj->tree, ObjListifyCallback, obj, 0);

    do {
        interp->delRBCnt = 0;
        Jsi_TreeWalk(obj->tree, ObjListifyArrayCallback, obj, 0);
    } while (interp->delRBCnt);
}

void Jsi_IterObjFree(Jsi_IterObj *iobj)
{
    if (!iobj->isArrayList) {
        uint i;
        for (i = 0; i < iobj->count; i++) {
            if (iobj->keys[i]) {
                /*Jsi_TreeDecrRef(iobj->keys[i]); TODO: ??? */
            }
        }
        Jsi_Free(iobj->keys);
    }
    Jsi_Free(iobj);
}

Jsi_IterObj *Jsi_IterObjNew(Jsi_Interp *interp, Jsi_IterProc *iterCmd)
{
    Jsi_IterObj *o = (Jsi_IterObj*)Jsi_Calloc(1,sizeof(Jsi_IterObj));
    o->interp = interp;
    o->iterCmd = iterCmd;
    return o;
}

static Jsi_RC DeleteTreeValue(Jsi_Interp *interp, Jsi_TreeEntry *ti, void *p) {
    /* Cleanup tree value. */
    SIGASSERT(ti,TREEENTRY);
    Jsi_Value *v = (Jsi_Value*)p;
    SIGASSERT(v,VALUE);
    Jsi_DecrRefCount(interp, v);
    ti->value = NULL;
    return JSI_OK;
}

int jsi_AllObjOp(Jsi_Interp *interp, Jsi_Obj* obj, int op) {
    if (op==2) {
        Jsi_Obj* o = interp->allObjs;
        while (o) {
            if (o==obj) return 1;
            o = o->next;
        }
        return 0;
    }
    if (op==1) {
        //printf("ADD: %p : %p : %d\n", interp, obj, obj->VD.Idx);
        assert(interp->allObjs!=obj);
        obj->next = interp->allObjs;
        if (interp->allObjs)
            interp->allObjs->prev = obj;
        interp->allObjs = obj;
        return 0;
    }
    if (op==0) {
        //printf("DEL: %p : %p\n", interp, obj);
        if (!obj || !interp->allObjs) return 0;
        if (obj == interp->allObjs)
            interp->allObjs = obj->next;
        if (obj->next)
            obj->next->prev = obj->prev;
        if (obj->prev)  
            obj->prev->next = obj->next; 
        return 0;
    }
    if (op == -1) {
        // TODO: fix cleanup for recursive bug, eg: x=[]; x.push(x);
        // Perhaps use python approach??: http://www.arctrix.com/nas/python/gc/
        while (0 && interp->allObjs) {
            printf("NEED CLEANUP: %p\n", interp->allObjs);
            Jsi_ObjDecrRefCount(interp, interp->allObjs);
        }
        return 0;
    }
#if JSI__MEMDEBUG
    assert(0);
    abort();
#endif
    return 0;
}

Jsi_Obj *jsi_ObjNew_(Jsi_Interp *interp)
{
    Jsi_Obj *obj = (Jsi_Obj*)Jsi_Calloc(1,sizeof(*obj));
    SIGINIT(obj,OBJ);
    jsi_DebugObj(obj,"New", jsi_DebugValueCallIdx(), interp);
    obj->ot = JSI_OT_OBJECT;
    obj->tree = Jsi_TreeNew(interp, JSI_KEYS_STRINGKEY, NULL);
    obj->tree->opts.freeTreeProc = DeleteTreeValue;
    obj->tree->flags.valuesonly = 1;
    obj->__proto__ = interp->Object_prototype;
    interp->dbPtr->objCnt++;
    interp->dbPtr->objAllocCnt++;
   return obj;
}

#ifndef JSI_MEM_DEBUG
Jsi_Obj * Jsi_ObjNew(Jsi_Interp *interp) {
    Jsi_Obj *obj = jsi_ObjNew_(interp);
    jsi_AllObjOp(interp, obj, 1);
    return obj;
}
#else
Jsi_Obj * jsi_ObjNew(Jsi_Interp *interp, const char *fname, int line, const char *func) {
    Jsi_Obj *obj = jsi_ObjNew_(interp);
    jsi_ValueDebugUpdate(interp, obj, objDebugTbl, fname, line, func);
    jsi_AllObjOp(interp, obj, 1);
    return obj;
}

#ifndef JSI_OMIT_STUBS
#undef Jsi_ObjNew
Jsi_Obj *Jsi_ObjNew(Jsi_Interp *interp) {
    Jsi_Obj* obj = jsi_ObjNew_(interp);
#ifdef JSI_MEM_DEBUG
    jsi_ValueDebugUpdate(interp, obj, objDebugTbl, NULL, 0, NULL);
#endif
    jsi_AllObjOp(interp, obj, 1);
    return obj;
}
#define Jsi_ObjNew(interp) jsi_ObjNew(interp, __FILE__, __LINE__,__PRETTY_FUNCTION__)
#endif

#endif

Jsi_Obj *Jsi_ObjNewType(Jsi_Interp *interp, Jsi_otype otype)
{
    Jsi_Obj *obj = Jsi_ObjNew(interp);
    obj->ot = (otype==JSI_OT_ARRAY?JSI_OT_OBJECT:otype);
    switch (otype) {
        case JSI_OT_BOOL:   obj->__proto__ = interp->Boolean_prototype; break;
        case JSI_OT_NUMBER: obj->__proto__ = interp->Number_prototype; break;
        case JSI_OT_STRING: obj->__proto__ = interp->String_prototype; break;
        case JSI_OT_FUNCTION:obj->__proto__ = interp->Function_prototype; break;
        case JSI_OT_REGEXP: obj->__proto__ = interp->RegExp_prototype; break;
        case JSI_OT_OBJECT: obj->__proto__ = interp->Object_prototype; break;
        case JSI_OT_ARRAY:  obj->__proto__ = interp->Array_prototype;
            obj->isarrlist = 1;
            break;
        default: assert(0); break;
    }
    if (interp->protoInit)
        assert(obj->__proto__);
    return obj;
}

void Jsi_ObjFree(Jsi_Interp *interp, Jsi_Obj *obj)
{
    interp->dbPtr->objCnt--;
    //assert(obj->refcnt == 0);
    jsi_AllObjOp(interp, obj, 0);
#ifdef JSI_MEM_DEBUG
    if (interp != obj->VD.interp)
        printf("interp mismatch of objFree: %p!=%p : %p\n", interp, obj->VD.interp, obj);
    jsi_DebugObj(obj,"Free", jsi_DebugValueCallIdx(), interp);
    if (obj->VD.hPtr && !interp->cleanup) {
        Jsi_HashEntryDelete(obj->VD.hPtr);
        obj->VD.hPtr = NULL;
    }
#endif
    /* printf("Free obj: %x\n", (int)obj); */
    switch (obj->ot) {
        case JSI_OT_STRING:
            if (!obj->isstrkey)
                Jsi_Free(obj->d.s.str);
            obj->d.s.str = 0;
            obj->isstrkey = 0;
            break;
        case JSI_OT_FUNCTION:
            jsi_FuncObjFree(obj->d.fobj);
            break;
        case JSI_OT_ITER:
            Jsi_IterObjFree(obj->d.iobj);
            break;
        case JSI_OT_USEROBJ:
            jsi_UserObjFree(interp, obj->d.uobj);
        case JSI_OT_ARRAY:
        case JSI_OT_OBJECT:
            break;
        case JSI_OT_REGEXP:
            if ((obj->d.robj->eflags&JSI_REG_STATIC)==0) {
                regfree(&obj->d.robj->reg);
                Jsi_Free(obj->d.robj);
            }
            break;
        default:
            break;
    }
    if (obj->tree)
        Jsi_TreeDelete(obj->tree);
    if (obj->arr) {
        int i = -1;
        while (++i < (int)obj->arrCnt)
            if (obj->arr[i])
                Jsi_DecrRefCount(interp, obj->arr[i]);
        Jsi_Free(obj->arr);
        obj->arr = NULL;
    }
    obj->tree = NULL;
    if (obj->clearProto)
        Jsi_DecrRefCount(interp, obj->__proto__);
#ifdef JSI_MEM_DEBUG
    memset(obj, 0, (sizeof(*obj)-sizeof(obj->VD)));
#endif
    Jsi_Free(obj);
}


/**************************** ARRAY ******************************/

Jsi_Value *jsi_ObjArrayLookup(Jsi_Interp *interp, Jsi_Obj *obj, const char *key) {
    if (!obj->arr || !key || !isdigit(*key))
        return NULL;
    char *ep = NULL;
    int n = (int)strtol(key, &ep, 0);
    if (n<0 || n >= (int)obj->arrCnt)
        return NULL;
    Jsi_Value *v = obj->arr[n];
    return v;
}

Jsi_RC Jsi_ObjArrayAdd(Jsi_Interp *interp, Jsi_Obj *o, Jsi_Value *v)
{
    if (o->isarrlist == 0)
        return JSI_ERROR;
    if (!o->arr)
        Jsi_ObjListifyArray(interp, o);
    int len = o->arrCnt;
    if (Jsi_ObjArraySizer(interp, o, len+1) <= 0)
        return JSI_ERROR;
    o->arr[len] = v;
    if (v)
        Jsi_IncrRefCount(interp, v);
    assert(o->arrCnt<=o->arrMaxSize);
    return JSI_OK;
}

Jsi_RC Jsi_ObjArraySet(Jsi_Interp *interp, Jsi_Obj *obj, Jsi_Value *value, int arrayindex)
{
    int m, n = arrayindex;
    if (Jsi_ObjArraySizer(interp, obj, n) <= 0)
        return JSI_ERROR;
    if (obj->arr[n] == value)
        return JSI_OK;
    if (obj->arr[n])
        Jsi_DecrRefCount(interp, obj->arr[n]);
    Assert(obj->arrCnt<=obj->arrMaxSize);
    obj->arr[n] = value;
    if (value)
        Jsi_IncrRefCount(interp, value);
    m = Jsi_ObjGetLength(interp, obj);
    if ((n+1) > m)
       Jsi_ObjSetLength(interp, obj, n+1);
    return JSI_OK;
}

// Copying version of above.
Jsi_Value *jsi_ObjArraySetDup(Jsi_Interp *interp, Jsi_Obj *obj, Jsi_Value *value, int n)
{
    if (Jsi_ObjArraySizer(interp, obj, n) <= 0)
        return NULL;
    if (obj->arr[n])
    {
        Jsi_ValueCopy(interp, obj->arr[n], value);
        return obj->arr[n];
    }
    Assert(obj->arrCnt<=obj->arrMaxSize);
    Jsi_Value *v = Jsi_ValueNew1(interp);
    int m;
    Jsi_ValueCopy(interp,v, value);
    obj->arr[n] = v;
    m = Jsi_ObjGetLength(interp, obj);
    if ((n+1) > m)
       Jsi_ObjSetLength(interp, obj, n+1);
    return v;
}

int Jsi_ObjIncrRefCount(Jsi_Interp *interp, Jsi_Obj *obj) {
    jsi_DebugObj(obj,"Incr", jsi_DebugValueCallIdx(), interp);
    SIGASSERT(obj,OBJ);
    Assert(obj->refcnt>=0);
    return ++obj->refcnt;
}

int Jsi_ObjDecrRefCount(Jsi_Interp *interp, Jsi_Obj *obj)  {
   // if (interp->cleanup && !jsi_AllObjOp(interp, obj, 2))
   //    return 0;
    SIGASSERT(obj,OBJ);
    if (obj->refcnt<=0) {
#ifdef JSI_MEM_DEBUG
        fprintf(stderr, "Obj decr with ref %d: VD.Idx=%d\n", obj->refcnt, obj->VD.Idx);
#endif
        return -2;
    }
    jsi_DebugObj(obj,"Decr", jsi_DebugValueCallIdx(), interp);
    int nref;
    if ((nref = --obj->refcnt) <= 0) {
        obj->refcnt = -1;
        Jsi_ObjFree(interp, obj);
    }
    return nref;
}


int Jsi_ObjArraySizer(Jsi_Interp *interp, Jsi_Obj *obj, uint len)
{
    int nsiz = len + 1, mod = ALLOC_MOD_SIZE;
    assert(obj->isarrlist);
    if (mod>1)
        nsiz = nsiz + ((mod-1) - (nsiz + mod - 1)%mod);
    if (nsiz > MAX_ARRAY_LIST) {
        Jsi_LogError("array size too large");
        return 0;
    }
    if (len >= obj->arrMaxSize) {
        int oldsz = (nsiz-obj->arrMaxSize);
        obj->arr = (Jsi_Value**)Jsi_Realloc(obj->arr, nsiz*sizeof(Jsi_Value*));
        memset(obj->arr+obj->arrMaxSize, 0, oldsz*sizeof(Jsi_Value*));
        obj->arrMaxSize = nsiz;
    }
    if (len>obj->arrCnt)
        obj->arrCnt = len;
    return nsiz;
}

Jsi_Obj *Jsi_ObjNewArray(Jsi_Interp *interp, Jsi_Value **items, int count, int copyflag)
{
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    if (count>=0) {
        int i;
        if (Jsi_ObjArraySizer(interp, obj, count) <= 0) {
            Jsi_ObjFree(interp, obj);
            return NULL;
        }
        for (i = 0; i < count; ++i) {
            if (!items[i]) continue;
            if (!copyflag) {
                obj->arr[i] = items[i];
                Jsi_IncrRefCount(interp, items[i]);
            } else {
                obj->arr[i] = Jsi_ValueNew1(interp);
                Jsi_ValueCopy(interp, obj->arr[i], items[i]);
            }
        }
    }
    obj->arrCnt = count;
    assert(obj->arrCnt<=obj->arrMaxSize);
    return obj;
}

/****** END ARRAY ************/

static Jsi_TreeEntry* ObjInsertFromValue(Jsi_Interp *interp, Jsi_Obj *obj, Jsi_Value *keyVal, Jsi_Value *nv)
{
    const char *key = NULL;
    int flags = 0;
    Jsi_DString dStr = {};
    if (keyVal->vt == JSI_VT_STRING) {
        flags = (keyVal->f.bits.isstrkey ? JSI_OM_ISSTRKEY : 0);
        key = keyVal->d.s.str;
    } else if (keyVal->vt == JSI_VT_OBJECT && keyVal->d.obj->ot == JSI_OT_STRING) {
        Jsi_Obj *o = keyVal->d.obj;
        flags = (o->isstrkey ? JSI_OM_ISSTRKEY : 0);
        key = o->d.s.str;
    }
    if (key == NULL)
        key = Jsi_ValueGetDString(interp, keyVal, &dStr, 0);
    return Jsi_ObjInsert(interp, obj, key, nv, flags);
}

Jsi_Obj *Jsi_ObjNewObj(Jsi_Interp *interp, Jsi_Value **items, int count)
{
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
    if (count%2) return obj;
    int i;
    for (i = 0; i < count; i += 2) {
        if (!items[i] || !items[i+1]) continue;
        Jsi_Value *v = Jsi_ValueDup(interp, items[i+1]);
        ObjInsertFromValue(interp, obj, items[i], v);
        Jsi_DecrRefCount(interp, v);
    }
    return obj;
}

void Jsi_ObjSetLength(Jsi_Interp *interp, Jsi_Obj *obj, uint len)
{
    if (obj->isarrlist) {
        assert(len<=obj->arrMaxSize);
        obj->arrCnt = len;
        return;
    }
    Jsi_Value *r = Jsi_TreeObjGetValue(obj,"length", 0);
    if (!r) {
        Jsi_Value *n = Jsi_ValueMakeNumber(interp, NULL, len);
        Jsi_ObjInsert(interp, obj, "length", n, JSI_OM_DONTDEL | JSI_OM_DONTENUM | JSI_OM_READONLY);
    } else {
        Jsi_ValueReset(interp, &r);
        Jsi_ValueMakeNumber(interp, &r, len);
    }
}

int Jsi_ObjGetLength(Jsi_Interp *interp, Jsi_Obj *obj)
{
    if (obj->tree && obj->tree->numEntries) {
        Jsi_Value *r = Jsi_TreeObjGetValue(obj, "length", 0);
        Jsi_Number nr;
        if (r && Jsi_GetNumberFromValue(interp,r, &nr) == JSI_OK) {
            if (Jsi_NumberIsInteger(nr))
                return nr;
        }
    }
    if (obj->arr)
        return obj->arrCnt;

    return 0;
}

Jsi_Value *jsi_ObjValueNew(Jsi_Interp *interp)
{
    return Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNew(interp));
}


/* Set result string into obj. */
void Jsi_ObjFromDS(Jsi_DString *dsPtr, Jsi_Obj *obj) {
    int len = Jsi_DSLength(dsPtr);
    if (obj->ot == JSI_OT_STRING && obj->d.s.str && !obj->isstrkey)
        Jsi_Free(obj->d.s.str);
    if (!(obj->d.s.str = (char*)dsPtr->strA)) {
        obj->d.s.str = (char*)Jsi_Malloc(len+1);
        memcpy(obj->d.s.str, dsPtr->Str, len+1);
    }
    obj->d.s.len = len;
    obj->isBlob = 1;
    dsPtr->strA = NULL;
    dsPtr->Str[0] = 0;
    dsPtr->len = 0;
    dsPtr->spaceAvl = dsPtr->staticSize;
}
#endif
