#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#if JSI__MUSL==1 || defined(__FreeBSD__)
#define NO_QSORT_R 1
#endif

static uint jsi_SizeOfArray(Jsi_Interp *interp, Jsi_Obj *obj) {
    if (!obj || !obj->arr)
        return 0;
    return obj->arrCnt;
}

static Jsi_RC jsi_ArrayPushCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Obj *obj;
    
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    obj = _this->d.obj;
    
    int argc = Jsi_ValueGetLength(interp, args);
    int curlen = jsi_SizeOfArray(interp, obj);    
    int i;
    for (i = 0; i < argc; ++i) {
        Jsi_Value *ov = Jsi_ValueArrayIndex(interp, args, i);
        if (!ov) { Jsi_LogBug("Arguments Error"); ov = Jsi_ValueNew(interp); }
        Jsi_ValueInsertArray(interp, _this, curlen + i, ov, 0);
    }
    
    Jsi_ValueMakeNumber(interp, ret, jsi_SizeOfArray(interp, obj));
    return JSI_OK;
}

static Jsi_RC jsi_ArrayPopCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_Value *v;
    Jsi_Obj *obj;
    obj = _this->d.obj;
    int i = jsi_SizeOfArray(interp, obj) - 1;

    if (i < 0) {
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_OK;
    }
    
    if (obj->arr) {
        if ((v = obj->arr[i])) {
            obj->arr[i] = NULL;
            obj->arrCnt--;
        }
    } else {
        v = Jsi_ValueArrayIndex(interp, _this, i);
    }
    if (v) {
        Jsi_DecrRefCount(interp, *ret);
        *ret = v;
    }
    Jsi_ObjSetLength(interp, obj, i);
    return JSI_OK;
}


static Jsi_RC jsi_ArrayJoinCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");
    const char *jstr = "";
    int argc, curlen;
    Jsi_DString dStr = {};

    curlen = jsi_SizeOfArray(interp, _this->d.obj);
    if (curlen == 0) {
        goto bail;
    }

    if (Jsi_ValueGetLength(interp, args) >= 1) {
        Jsi_Value *sc = Jsi_ValueArrayIndex(interp, args, 0);
        if (sc != NULL)
            jstr = Jsi_ValueToString(interp, sc, NULL);
    }
    
    if (0 == (argc=jsi_SizeOfArray(interp, _this->d.obj))) {
        goto bail;
    }
    int i;
    for (i = 0; i < argc; ++i) {
        const char *cp;
        Jsi_Value *ov = Jsi_ValueArrayIndex(interp, _this, i);
        if (!ov) {
            /* TODO: are NULL args ok? */ 
            continue;
            cp = "";
        } else
            cp = Jsi_ValueToString(interp, ov, NULL);
        if (i && jstr[0])
            Jsi_DSAppend(&dStr, jstr, NULL);
        Jsi_DSAppend(&dStr, cp, NULL);
    }
    
    Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return JSI_OK;
bail:
    Jsi_ValueMakeStringDup(interp, ret, "");
    return JSI_OK;        
}


Jsi_Value* Jsi_ValueArrayConcat(Jsi_Interp *interp, Jsi_Value *arg1, Jsi_Value *arg2) {
    Jsi_Value *va;
    Jsi_Obj *obj;
    if (arg1->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, arg1->d.obj)) {
        return NULL;
    }
    if (arg2->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, arg2->d.obj)) {
        return NULL;
    }
    int len1 = arg1->d.obj->arrCnt;
    int len2 = arg2->d.obj->arrCnt;
    Jsi_Obj *nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    Jsi_ObjArraySizer(interp, nobj, len1+len2);

    int i, j = 0;
    obj = arg1->d.obj;
    for (i = 0; i<len1; i++, j++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[j] = NULL;
        Jsi_ValueDup2(interp, nobj->arr+j, obj->arr[i]);
    }
    obj = arg2->d.obj;
    for (i = 0; i<len2; i++, j++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[j] = NULL;
        Jsi_ValueDup2(interp, nobj->arr+j, obj->arr[i]);
    }
    Jsi_ObjSetLength(interp, nobj, len1+len2);
    va = Jsi_ValueMakeArrayObject(interp, NULL, nobj);
    return va;
}

Jsi_RC Jsi_ValueArrayPush(Jsi_Interp *interp, Jsi_Value *arg1, Jsi_Value *arg2) {
    Jsi_Obj *obj;
    if (arg1->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, arg1->d.obj))
        return JSI_ERROR;
    if (!arg2)
        return JSI_ERROR;
    int len1 = arg1->d.obj->arrCnt;
    obj = arg1->d.obj;
    Jsi_ObjArraySizer(interp, obj, len1);
    obj->arr[len1] = arg2;
    Jsi_IncrRefCount(interp, arg2);
    obj->arrCnt++;
    return JSI_OK;
}


Jsi_Value *Jsi_ValueArrayPop(Jsi_Interp *interp, Jsi_Value *v)
{
    if (v->vt != JSI_VT_OBJECT) {
        Jsi_LogBug("Jsi_ValueArrayPop, target is not object");
        return NULL;
    }
    Jsi_Obj *o = v->d.obj;
    if (!o->isarrlist) {
        Jsi_LogBug("Jsi_ValueArrayPop, target is not array");
        return NULL;
    }
    if (o->arrCnt<=0)
        return NULL;
    int idx = o->arrCnt-1;
    if (!o->arr[idx])
        return NULL;
    Jsi_DecrRefCount(interp, o->arr[idx]);
    Jsi_Value *ret = o->arr[idx];
    o->arr[idx] = NULL;
    o->arrCnt--;
    return ret;
}


Jsi_Value *Jsi_ValueArrayUnshift(Jsi_Interp *interp, Jsi_Value *v)
{
    if (v->vt != JSI_VT_OBJECT) {
        Jsi_LogBug("Jsi_ValueArrayUnshift, target is not object");
        return NULL;
    }
    Jsi_Obj *o = v->d.obj;
    if (!o->isarrlist) {
        Jsi_LogBug("Jsi_ValueArrayUnshift, target is not array");
        return NULL;
    }
    if (o->arrCnt<=0)
        return NULL;
    if (!o->arr[0])
        return NULL;
    Jsi_DecrRefCount(interp, o->arr[0]);
    Jsi_Value *ret = o->arr[0];
    o->arr[0] = NULL;
    o->arrCnt--;
    return ret;
}

/* delete array[0], array[1]->array[0] */
void Jsi_ValueArrayShift(Jsi_Interp *interp, Jsi_Value *v)
{
    if (v->vt != JSI_VT_OBJECT) {
        Jsi_LogBug("Jsi_ValueArrayShift, target is not object");
        return;
    }
    Jsi_Obj *o = v->d.obj;
    if (o->isarrlist) {
        uint i;
        if (!o->arrCnt)
            return;
        if (o->arr[0])
            Jsi_DecrRefCount(interp, o->arr[0]);
        for (i=1; i<o->arrCnt; i++) {
            o->arr[i-1] = o->arr[i];
        }
        o->arr[o->arrCnt--] = NULL;
        return;
    }
    
    int len = jsi_SizeOfArray(interp, v->d.obj);
    if (len <= 0) return;
    
    Jsi_Value *v0 = Jsi_ValueArrayIndex(interp, v, 0);
    if (!v0) return;
    
    Jsi_ValueReset(interp, &v0);
    
    int i;
    Jsi_Value *last = v0;
    for (i = 1; i < len; ++i) {
        Jsi_Value *t = Jsi_ValueArrayIndex(interp, v, i);
        if (!t) return;
        Jsi_ValueCopy(interp, last, t);
        Jsi_ValueReset(interp, &t);
        last = t;
    }
    Jsi_ObjSetLength(interp, v->d.obj, len - 1);
}

static Jsi_RC jsi_ArrayFlatSub(Jsi_Interp *interp, Jsi_Obj* nobj, Jsi_Value *arr, int depth) {
    
    int i, n = 0, len = jsi_SizeOfArray(interp, arr->d.obj);
    if (len <= 0) return JSI_OK;
    Jsi_RC rc = JSI_OK;
    int clen = jsi_SizeOfArray(interp, nobj);
    for (i = 0; i < len && rc == JSI_OK; i++) {
        Jsi_Value *t = Jsi_ValueArrayIndex(interp, arr, i);
        if (t && depth>0 && Jsi_ValueIsArray(interp, t))
            rc = jsi_ArrayFlatSub(interp, nobj, t , depth-1);
        else if (!Jsi_ValueIsUndef(interp, t))
            Jsi_ObjArrayAdd(interp, nobj, t);
        if ((++n + clen)>interp->maxArrayList)
            return Jsi_LogError("array size exceeded");
    }
    return rc;
}

static Jsi_RC jsi_ArrayFlatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");

    Jsi_Number ndepth = 1;
    Jsi_Obj *nobj;
    Jsi_Value *depth = Jsi_ValueArrayIndex(interp, args, 0);
    if (depth && Jsi_GetNumberFromValue(interp,depth, &ndepth) != JSI_OK)
        return JSI_ERROR;
    
    if (ndepth < 0 || ndepth>1000)
        return Jsi_LogError("bad depth: %d", (int)ndepth);

    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    Jsi_ValueMakeArrayObject(interp, ret, nobj );
    if (ndepth>0)
        return jsi_ArrayFlatSub(interp, nobj, _this, ndepth);
    return JSI_OK;
}

static Jsi_RC jsi_ArrayConcatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");
    Jsi_RC rc = JSI_OK;
    int curlen, argc, nsiz;
    Jsi_Obj *obj, *nobj;
    Jsi_Value *va;

    obj = _this->d.obj;
    
    argc = Jsi_ValueGetLength(interp, args);
    curlen = jsi_SizeOfArray(interp, obj);
    Jsi_ObjListifyArray(interp, obj);
   
    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    nsiz = obj->arrMaxSize;
    if (nsiz<=0) nsiz = 100;
    if (Jsi_ObjArraySizer(interp, nobj, nsiz+1) <= 0) {
        rc = JSI_ERROR;
        Jsi_LogError("index too large: %d", nsiz+1);
        goto bail;
    }

    int i, j, m;
    for (i = 0; i<curlen; i++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[i] = NULL;
        Jsi_ValueDup2(interp, nobj->arr+i, obj->arr[i]);
    }
    m = i;
    for (i = 0; i < argc; i++) {
         va = Jsi_ValueArrayIndex(interp, args, i);
         if (va->vt == JSI_VT_OBJECT && Jsi_ObjIsArray(interp, va->d.obj)) {
            int margc = Jsi_ValueGetLength(interp, va);
            Jsi_Obj *mobj = va->d.obj;
            Jsi_ObjListifyArray(interp, mobj);
            if (Jsi_ObjArraySizer(interp, nobj, curlen += margc) <= 0) {
                rc = JSI_ERROR;
                Jsi_LogError("index too large: %d", curlen);
                goto bail;
            }
            for (j = 0; j<margc; j++, m++)
            {
                if (!mobj->arr[j]) continue;
                nobj->arr[m] = NULL;
                Jsi_ValueDup2(interp, nobj->arr+m, mobj->arr[j]);
            }
        } else {
            if (Jsi_ObjArraySizer(interp, nobj, ++curlen) <= 0) {
                rc = JSI_ERROR;
                Jsi_LogError("index too large: %d", curlen);
                goto bail;
            }
            nobj->arr[m] = NULL;
            Jsi_ValueDup2(interp, nobj->arr+m++, va);
       }
    }
    Jsi_ObjSetLength(interp, nobj, curlen);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    return JSI_OK;
        
bail:
    Jsi_ValueMakeNull(interp, ret);
    return rc;
}

static Jsi_RC jsi_ArrayMapCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");
    Jsi_RC rc = JSI_OK;
    int curlen, nsiz, i, maa = 0;
    Jsi_Obj *obj, *nobj;
    Jsi_Value *func, *vpargs, *nthis = NULL, *sthis;
    Jsi_Func *fptr = NULL;

    func = Jsi_ValueArrayIndex(interp, args, 0);
    if (!Jsi_ValueIsFunction(interp, func)) 
        return Jsi_LogError("expected function");
    sthis = Jsi_ValueArrayIndex(interp, args, 1);
    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);
    obj = _this->d.obj;
    curlen = jsi_SizeOfArray(interp, obj);    
    Jsi_ObjListifyArray(interp, obj);
    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    nsiz = obj->arrCnt;
    if (nsiz<=0) nsiz = 1;
    if (Jsi_ObjArraySizer(interp, nobj, nsiz) <= 0) {
        Jsi_LogError("index too large: %d", nsiz);
        rc = JSI_ERROR;
        goto bail;
    }
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    Jsi_Value *vobjs[3];

    fptr = func->d.obj->d.fobj->func;
    maa = (fptr->argnames?fptr->argnames->argCnt:0);
    if (maa>3)
        maa = 3;
    for (i = 0; i < curlen; i++) {
        if (!obj->arr[i]) continue;
        vobjs[0] = obj->arr[i];
        vobjs[1] = (maa>1?Jsi_ValueNewNumber(interp, i):NULL);
        vobjs[2] = _this;
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vobjs, maa, 0));
        Jsi_IncrRefCount(interp, vpargs);
        nobj->arr[i] = Jsi_ValueNew1(interp);
        rc = Jsi_FunctionInvoke(interp, func, vpargs, nobj->arr+i, sthis);
        Jsi_DecrRefCount(interp, vpargs);
        if( JSI_OK!=rc ) {
            goto bail;
        }
    }
    Jsi_ObjSetLength(interp, nobj, curlen);
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    return JSI_OK;
        
bail:
    Jsi_ValueMakeNull(interp, ret);
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    return rc;
}

static Jsi_RC jsi_ArrayFilterCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");
    Jsi_RC rc = JSI_OK;
    int curlen, nsiz, i, fval, n = 0, maa = 0;
    Jsi_Obj *obj, *nobj;
    Jsi_Value *func, *vpargs, *nthis = NULL, *sthis, *nrPtr = NULL;
    Jsi_Func *fptr = NULL;

    func = Jsi_ValueArrayIndex(interp, args, 0);
    if (!Jsi_ValueIsFunction(interp, func)) 
        return Jsi_LogError("expected function");
    sthis = Jsi_ValueArrayIndex(interp, args, 1);
    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);
    obj = _this->d.obj;
    curlen = jsi_SizeOfArray(interp, obj);    
    Jsi_ObjListifyArray(interp, obj);
    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    nsiz = obj->arrCnt;
    if (nsiz<=0) nsiz = 1;
    if (Jsi_ObjArraySizer(interp, nobj, nsiz) <= 0) {
        Jsi_LogError("index too large: %d", nsiz);
        rc = JSI_ERROR;
        goto bail;
    }
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    nrPtr = Jsi_ValueNew1(interp);
    Jsi_Value *vobjs[4];

    fptr = func->d.obj->d.fobj->func;
    maa = (fptr->argnames?fptr->argnames->argCnt:0);
    if (maa>3)
        maa = 3;
    for (i = 0; i < curlen; i++) {
        if (!obj->arr[i]) continue;
        vobjs[0] = obj->arr[i];
        vobjs[1] = (maa>1?Jsi_ValueNewNumber(interp, i):NULL);
        vobjs[2] = _this;
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vobjs, maa, 0));
        Jsi_IncrRefCount(interp, vpargs);
        rc = Jsi_FunctionInvoke(interp, func, vpargs, &nrPtr, sthis);
        Jsi_DecrRefCount(interp, vpargs);
        fval = Jsi_ValueIsTrue(interp, nrPtr);
        Jsi_ValueMakeUndef(interp, &nrPtr);
        if( JSI_OK!=rc ) {
            goto bail;
        }
        if (fval) {
            nobj->arr[n++] = obj->arr[i];
            Jsi_IncrRefCount(interp, obj->arr[i]);
        }
    }
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    Jsi_DecrRefCount(interp, nrPtr);
    Jsi_ObjSetLength(interp, nobj, n);
    return JSI_OK;
        
bail:
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    if (nrPtr)
        Jsi_DecrRefCount(interp, nrPtr);
    Jsi_ValueMakeNull(interp, ret);
    return rc;
}

static Jsi_RC jsi_ArrayReverseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) 
        return Jsi_LogError("expected array");
    int i, n, m;
    Jsi_Obj *obj;
    Jsi_Value *tval, *nthis = NULL, *sthis = Jsi_ValueArrayIndex(interp, args, 1);

    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);
    obj = _this->d.obj;
    Jsi_ObjListifyArray(interp, obj);
    m = obj->arrCnt/2;
    for (i = 0, n=obj->arrCnt-1; i < m; i++, n--) {
        tval = obj->arr[i];
        obj->arr[i] = obj->arr[n];
        obj->arr[n] = tval;
    }
    Jsi_ValueDup2(interp, ret, _this);
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    return JSI_OK;
}

static Jsi_RC jsi_ArrayForeachCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) 
{
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) 
        return Jsi_LogError("expected array object");
    Jsi_Obj *obj;
    uint i;
    Jsi_Value *func, *vpargs;

    func = Jsi_ValueArrayIndex(interp, args, 0);
    if (!Jsi_ValueIsFunction(interp, func)) 
        return Jsi_LogError("expected function");
    Jsi_Value *sthis = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *nthis = NULL;
    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);

    obj = _this->d.obj;
    Jsi_ObjListifyArray(interp, obj);
    Jsi_RC rc = JSI_OK;
    
    Jsi_Value *vobjs[3];
    Jsi_Func *fptr = func->d.obj->d.fobj->func;
    int maa = (fptr->argnames?fptr->argnames->argCnt:0);
    if (maa>3)
        maa = 3;
    for (i = 0; i < obj->arrCnt && rc == JSI_OK; i++) {
        if (!obj->arr[i]) continue;
        vobjs[0] = obj->arr[i];
        vobjs[1] = (maa>1?Jsi_ValueNewNumber(interp, i):NULL);
        vobjs[2] = _this;
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vobjs, maa, 0));
        Jsi_IncrRefCount(interp, vpargs);
        rc = Jsi_FunctionInvoke(interp, func, vpargs, ret, sthis);
        Jsi_DecrRefCount(interp, vpargs);
    }
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    return rc;
}

static Jsi_RC jsi_ArrayFindSubCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr, int op) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) 
        return Jsi_LogError("expected array");
    Jsi_Obj *obj;
    uint i;
    Jsi_RC rc = JSI_OK;
    Jsi_Value *func, *vpargs, *sthis = Jsi_ValueArrayIndex(interp, args, 1);

    func = Jsi_ValueArrayIndex(interp, args, 0);
    if (!Jsi_ValueIsFunction(interp, func)) 
        return Jsi_LogError("expected function");
    Jsi_Value *nthis = NULL;
    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);

    obj = _this->d.obj;
    Jsi_ObjListifyArray(interp, obj);
    int fval = 0;
    Jsi_Value *nrPtr = Jsi_ValueNew1(interp);
    Jsi_Value *vobjs[3];
    Jsi_Func *fptr = func->d.obj->d.fobj->func;
    int maa = (fptr->argnames?fptr->argnames->argCnt:0);
    if (maa>3)
        maa = 3;
    for (i = 0; i < obj->arrCnt && rc == JSI_OK; i++) {
        if (!obj->arr[i]) continue;
        vobjs[0] = obj->arr[i];
        vobjs[1] = (maa>1?Jsi_ValueNewNumber(interp, i):NULL);
        vobjs[2] = _this;
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vobjs, maa, 0));
        Jsi_IncrRefCount(interp, vpargs);
        rc = Jsi_FunctionInvoke(interp, func, vpargs, &nrPtr, sthis);
        Jsi_DecrRefCount(interp, vpargs);
        if (rc != JSI_OK)
            break;
        fval = Jsi_ValueIsTrue(interp, nrPtr);
        Jsi_ValueMakeUndef(interp, &nrPtr);
        if (op == 3) {
            if (!fval) break;
        } else if (fval)
            break;
    }
    if (rc == JSI_OK) {
        if (op == 1 && fval) // Find
            Jsi_ValueCopy(interp, *ret, obj->arr[i]); 
        else if (op == 2 || op == 3) // Some/Every
            Jsi_ValueMakeBool(interp, ret, fval);
        else if (op == 4)
            Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)(fval?(int)i:-1));
    }
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    Jsi_DecrRefCount(interp, nrPtr);
    return rc;

}

static Jsi_RC jsi_ArrayReduceSubCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr, int op) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) 
        return Jsi_LogError("expected array");
    Jsi_RC rc = JSI_OK;
    int i;
    Jsi_Obj *obj;
    Jsi_Value *func, *vpargs, *ini = Jsi_ValueArrayIndex(interp, args, 1);

    func = Jsi_ValueArrayIndex(interp, args, 0);
    if (!Jsi_ValueIsFunction(interp, func)) 
        return Jsi_LogError("expected function");

    Jsi_Value *nrPtr = Jsi_ValueNew1(interp);
    obj = _this->d.obj;
    Jsi_ObjListifyArray(interp, obj);
    Jsi_Value *vobjs[4];
    int n, rev = (op==2);
    Jsi_Func *fptr = func->d.obj->d.fobj->func;
    int maa = (fptr->argnames?fptr->argnames->argCnt:0);
    if (maa>4)
        maa = 4;

    for (n = 0, i = (rev?obj->arrCnt-1:0); (rev?i>=0:i < (int)obj->arrCnt) && rc == JSI_OK; n++, i = (rev?i-1:i+1)) {
        if (!obj->arr[i]) continue;
        if (n==0 && !ini) {
            ini = obj->arr[i];
            continue;
        }
            
        vobjs[0] = ini;
        vobjs[1] = obj->arr[i];
        vobjs[2] = (maa>2?Jsi_ValueNewNumber(interp, i):NULL);
        vobjs[3] = _this;
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vobjs, maa, 0));
        Jsi_IncrRefCount(interp, vpargs);
        rc = Jsi_FunctionInvoke(interp, func, vpargs, &nrPtr, NULL);
        Jsi_DecrRefCount(interp, vpargs);
        if (rc != JSI_OK)
            break;
        ini = nrPtr;
    }
    if (rc == JSI_OK && ini)
        Jsi_ValueCopy(interp, *ret, ini); 
    Jsi_DecrRefCount(interp, nrPtr);
    return rc;

}

static Jsi_RC jsi_ArrayFindCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_ArrayFindSubCmd(interp, args, _this, ret, funcPtr, 1);
}
static Jsi_RC jsi_ArraySomeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_ArrayFindSubCmd(interp, args, _this, ret, funcPtr, 2);
}
static Jsi_RC jsi_ArrayEveryCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_ArrayFindSubCmd(interp, args, _this, ret, funcPtr, 3);
}
static Jsi_RC jsi_ArrayFindIndexCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_ArrayFindSubCmd(interp, args, _this, ret, funcPtr, 4);
}
static Jsi_RC jsi_ArrayReduceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_ArrayReduceSubCmd(interp, args, _this, ret, funcPtr, 1);
}
static Jsi_RC jsi_ArrayReduceRightCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_ArrayReduceSubCmd(interp, args, _this, ret, funcPtr, 2);
}
static Jsi_RC jsi_ArrayIsArrayCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    bool b = 0;
    Jsi_Value *sthis = _this;
    if (_this->vt == JSI_VT_OBJECT && _this->d.obj->ot == JSI_OT_FUNCTION &&
       _this->d.obj->__proto__ == interp->Array_prototype->d.obj->__proto__ )
        sthis = Jsi_ValueArrayIndex(interp, args, 0); 
    if (sthis && sthis->vt == JSI_VT_OBJECT && Jsi_ObjIsArray(interp, sthis->d.obj))
        b = 1;
    Jsi_ValueMakeBool(interp, ret, b);
    return JSI_OK;
}

static Jsi_RC jsi_ArrayIndexSubCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr, int op) {
    int istart = 0, n, i = 0, dir=1, idx=-1;
    Jsi_Value *seq = Jsi_ValueArrayIndex(interp, args, 0),
        *start = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Obj *obj = _this->d.obj;
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");

    if (!seq) {
        goto bail;
    }
    
    n = jsi_SizeOfArray(interp, obj);    
    if (n == 0) {
        goto bail;
    }
    Jsi_Number nstart;
    if (op == 2) {
        istart = n-1;
    }
    if (start && Jsi_GetNumberFromValue(interp,start, &nstart)==JSI_OK) {
        istart = (int)nstart;
        if (istart > n)
            goto bail;
        if (istart < 0)
            istart = (n+istart);
        if (istart<0)
            goto bail;
    }
    if (op == 2) {
        istart = n-1;
        dir = -1;
    }
    Jsi_ObjListifyArray(interp, obj);
    for (i = istart; ; i+=dir)
    {
        if ((dir>0 && i>=n) || (dir<0 && i<0) || i>=(int)obj->arrCnt)
            break;
        if (obj->arr[i] && Jsi_ValueCmp(interp, obj->arr[i], seq, JSI_CMP_EXACT)==0) {
            idx = i;
            break;
        }
    }
bail:
    if (op == 3)
        Jsi_ValueMakeBool(interp, ret, (idx!=-1));
    else
        Jsi_ValueMakeNumber(interp, ret, idx);
    return JSI_OK;
}

static Jsi_RC jsi_ArrayIndexOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_ArrayIndexSubCmd(interp, args, _this, ret, funcPtr, 1);
}
static Jsi_RC jsi_ArrayLastindexOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_ArrayIndexSubCmd(interp, args, _this, ret, funcPtr, 2);
}
static Jsi_RC jsi_ArrayIncludesCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    return jsi_ArrayIndexSubCmd(interp, args, _this, ret, funcPtr, 3);
}

static Jsi_RC jsi_ArraySizeOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");
    int i = jsi_SizeOfArray(interp, _this->d.obj);
    Jsi_ValueMakeNumber(interp, ret, i);
    return JSI_OK;
}

static Jsi_RC jsi_ArrayShiftCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");
    Jsi_Value *v;
    Jsi_Obj *obj = _this->d.obj;
    Jsi_ObjListifyArray(interp, obj);
    uint n = jsi_SizeOfArray(interp, obj);
    if (n<=0) {
        Jsi_ValueMakeUndef(interp, ret);
    } else {
        n--;
        v = obj->arr[0];
        memmove(obj->arr, obj->arr+1, n*sizeof(Jsi_Value*));
        obj->arr[n] = NULL;
        Jsi_ValueDup2(interp, ret, v);
        Jsi_DecrRefCount(interp, v);
        Jsi_ObjSetLength(interp, obj, n);
    }
    return JSI_OK;
}

static Jsi_RC jsi_ArrayUnshiftCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");

    Jsi_Obj *obj = _this->d.obj;
    int argc = Jsi_ValueGetLength(interp, args);
    int curlen = jsi_SizeOfArray(interp, obj);
    if (argc <= 0) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_ObjListifyArray(interp, obj);
    if (Jsi_ObjArraySizer(interp, obj, curlen+argc)<=0) 
        return Jsi_LogError("too long");
    memmove(obj->arr+argc, obj->arr, (curlen)*sizeof(Jsi_Value*));
    obj->arrCnt += argc;
    int i;
    for (i = 0; i < argc; ++i) {
        Jsi_Value *ov = Jsi_ValueArrayIndex(interp, args, i);
        obj->arr[i] = NULL;
        if (!ov) { Jsi_LogBug("Arguments Error"); continue; }
        obj->arr[i] = ov;
        Jsi_IncrRefCount(interp, ov);
    }
    Jsi_ObjSetLength(interp, obj, curlen+argc);
    
    Jsi_ValueMakeNumber(interp, ret, jsi_SizeOfArray(interp, obj));
    return JSI_OK;
}

static Jsi_RC jsi_ArrayFillCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) 
        return Jsi_LogError("expected array object");
    Jsi_RC rc = JSI_OK;
    int istart = 0, iend, n, nsiz;
    Jsi_Number nstart = 0, nend = 0; // TODO: merge with code in ArraySliceCmd.
    Jsi_Value *value = Jsi_ValueArrayIndex(interp, args, 0),
        *start = Jsi_ValueArrayIndex(interp, args, 1),
        *end = Jsi_ValueArrayIndex(interp, args, 2);
    Jsi_Obj *obj = _this->d.obj;
    n = jsi_SizeOfArray(interp, obj);

    if (start && Jsi_GetNumberFromValue(interp, start, &nstart) == JSI_OK) {
        istart = (int)nstart;
        if (istart > n)
            goto bail;
        if (istart < 0)
            istart = (n+istart);
        if (istart<0)
            goto bail;
    }
      
    if (n == 0) {
        goto bail;
    }
    iend = n-1;
    if (end && Jsi_GetNumberFromValue(interp,end, &nend) == JSI_OK) {
        iend = (int) nend;
        if (iend >= n)
            iend = n;
        if (iend < 0)
            iend = (n+iend);
        if (iend<0)
            goto bail;
    }
    nsiz = iend-istart+1;
    if (nsiz<=0)
        goto bail;

    int i;
    for (i = istart; i <= iend; i++)
    {
        if (obj->arr[i])
            Jsi_ValueCopy(interp, obj->arr[i], value);
        else
            obj->arr[i] = Jsi_ValueDup(interp, value);
    }
bail:
    if (_this != *ret) {
        Jsi_ValueMove(interp, *ret, _this);
        /*if (*ret)
            Jsi_DecrRefCount(interp, *ret);
        *ret = _this;
        Jsi_IncrRefCount(interp, *ret);*/
    }
    return rc;
}


static Jsi_RC jsi_ArraySliceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
   if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");

    Jsi_RC rc = JSI_OK;
    int istart = 0, iend, n, nsiz;
    Jsi_Number nstart;
    Jsi_Obj *nobj, *obj;
    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, 0),
        *end = Jsi_ValueArrayIndex(interp, args, 1);
    if (!start) {
        goto bail;
    }
    obj = _this->d.obj;
    n = jsi_SizeOfArray(interp, obj);
    if (Jsi_GetNumberFromValue(interp,start, &nstart) == JSI_OK) {
        istart = (int)nstart;
        if (istart > n)
            goto done;
        if (istart < 0)
            istart = (n+istart);
        if (istart<0)
            goto bail;
    }
      
    if (n == 0) {
done:
        Jsi_ValueMakeArrayObject(interp, ret, Jsi_ObjNewType(interp, JSI_OT_ARRAY));
        return JSI_OK;
    }
    Jsi_Number nend;
    iend = n-1;
    if (end && Jsi_GetNumberFromValue(interp,end, &nend) == JSI_OK) {
        iend = (int) nend;
        if (iend >= n)
            iend = n;
        if (iend < 0)
            iend = (n+iend);
        if (iend<0)
            goto bail;
    }
    nsiz = iend-istart+1;
    if (nsiz<=0)
        goto done;
    Jsi_ObjListifyArray(interp, obj);
    
    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);

    if (Jsi_ObjArraySizer(interp, nobj, nsiz) <= 0) {
        rc = Jsi_LogError("index too large: %d", nsiz);
        goto bail;
    }

    int i, m;
    for (m = 0, i = istart; i <= iend; i++, m++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[m] = NULL;
        Jsi_ValueDup2(interp, nobj->arr+m, obj->arr[i]);
    }
    Jsi_ObjSetLength(interp, nobj, nsiz);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    return JSI_OK;
    
bail:
    Jsi_ValueMakeNull(interp, ret);
    return rc;
}

typedef struct {
    Jsi_Interp *interp;
    int flags;
    int mode;
    bool unique;
    Jsi_Value *compare;
    int errCnt;
} SortInfo;

static const char *sortArrayStrs[] = {"default", "desc", "dict", "nocase", 0};

static Jsi_OptionSpec jsi_ArraySortOptions[] = {
    JSI_OPT(CUSTOM, SortInfo, mode,     .help="Mode to sort by", .flags=0, .custom=Jsi_Opt_SwitchEnum,  .data=sortArrayStrs),
    JSI_OPT(FUNC,   SortInfo, compare,  .help="Function to do comparison", .flags=0, .custom=0, .data=(void*)"val1,val2"),
    JSI_OPT(BOOL,   SortInfo, unique,   .help="Eliminate duplicate items"),
    JSI_OPT_END(SortInfo)
};

#ifdef NO_QSORT_R

SortInfo *curSortInfo = NULL;

static int SortSubCmd(const void *p1, const void *p2) {
    SortInfo *si = curSortInfo;
#else
#ifdef __WIN32
static int SortSubCmd(void *thunk, const void *p1, const void *p2)
#else
static int SortSubCmd(const void *p1, const void *p2, void *thunk)
#endif
{
    SortInfo *si = (SortInfo *)thunk;
#endif
    Jsi_Interp *interp = si->interp;
    int sortFlags = si->flags;

    if (interp == NULL || interp->deleting)
        return 0;
    Jsi_Value *v1 = *(Jsi_Value**)p1, *v2 = *(Jsi_Value**)p2;
    int rc = 0;
    if (v1 != NULL && v2 != NULL) {
        VALCHK(v1);
        VALCHK(v2);
        if (!si->compare)
            rc = Jsi_ValueCmp(interp, v1, v2, sortFlags);
        else {
            Jsi_Value *vv[2] = {v1, v2};
            Jsi_Value *retP = Jsi_ValueNew1(interp);
            Jsi_Value *vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vv, 2, 0));
            Jsi_IncrRefCount(interp, vpargs);
            rc = Jsi_FunctionInvoke(interp, si->compare, vpargs, &retP, NULL);
            Jsi_DecrRefCount(interp, vpargs);
            if (rc == JSI_OK) {
                Jsi_Number d = 0;
                if (Jsi_ValueGetNumber(interp, retP, &d) == JSI_OK)
                    rc = -(int)d;
                else {
                    if (!si->errCnt)
                        Jsi_LogWarn("invalid function return");
                    si->errCnt++;
                }
            }
            Jsi_DecrRefCount(interp, retP);
        }
    } else {
        if (v1 == v2) 
            rc = 0;
        else if (v1 == NULL)
            rc = 1;
        else
            rc = -1;
    }
    if ((sortFlags&JSI_SORT_DESCEND))
        return rc;
    return -rc;
}

Jsi_RC Jsi_ValueArraySort(Jsi_Interp *interp, Jsi_Value *val, int flags)
{
    if (val->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, val->d.obj)) {
        return JSI_ERROR;
    }
    Jsi_Obj *obj = val->d.obj;
    Jsi_ObjListifyArray(interp, obj);
    if (obj->arrCnt <= 0) {
        return JSI_OK;
    }
#ifdef __WIN32
#define qsort_r qsort_s
#endif

    SortInfo si = {};
    si.interp = interp;
    si.flags = flags;
#ifdef NO_QSORT_R
    curSortInfo = &si;
    qsort(obj->arr, obj->arrCnt, sizeof(Jsi_Value*), SortSubCmd);
    curSortInfo = NULL;
#else
    qsort_r(obj->arr, obj->arrCnt, sizeof(Jsi_Value*), SortSubCmd, &si);
#endif
    return JSI_OK;
}

static Jsi_RC jsi_ArraySortCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");

    int flags = 0, i, curlen, hasopt = 0;
    Jsi_Value *v, *arg = NULL;
    SortInfo si = {};
    si.interp = interp;


    Jsi_Obj *obj = _this->d.obj;
    curlen = obj->arrCnt;

    if (curlen <= 1) {
        goto done;
    }
    
    arg = Jsi_ValueArrayIndex(interp, args, 0);
    if (arg) {
        if (Jsi_ValueIsObjType(interp, arg, JSI_OT_OBJECT)) {
            if (Jsi_OptionsProcess(interp, jsi_ArraySortOptions, &si, arg, 0) < 0)
                return JSI_ERROR;
            hasopt = 1;
            switch (si.mode) {
                case 1: flags |= JSI_SORT_DESCEND; break;
                case 2: flags |= JSI_SORT_DICT; break;
                case 3: flags |= JSI_SORT_NOCASE; break;
            }
        } else if (Jsi_ValueIsObjType(interp, arg, JSI_OT_FUNCTION))
            si.compare = arg;
        else 
            return Jsi_LogError("expected object or function");
    }
    si.flags = flags;
    Jsi_ObjListifyArray(interp, obj);
#ifdef NO_QSORT_R
    /* TODO: mutex. */
    curSortInfo = &si;
    qsort(obj->arr, curlen, sizeof(Jsi_Value*), SortSubCmd);
#else
    qsort_r(obj->arr, curlen, sizeof(Jsi_Value*), SortSubCmd, &si);
#endif

    if (interp->deleting) {
#ifdef NO_QSORT_R
        curSortInfo = NULL;
#endif
        return JSI_ERROR;
    }
    if (si.unique) {
        int n, diff = 1, dupCnt=0;
        for (n=0, i=1; i<(int)obj->arrCnt; i++) {
            if (obj->arr[n] == obj->arr[i])
                diff = 1;
            else
#ifdef NO_QSORT_R
                diff = SortSubCmd(&obj->arr[n], &obj->arr[i]);
#else
#ifdef __WIN32
                diff = SortSubCmd(&si, &obj->arr[n], &obj->arr[i]);
#else
                diff = SortSubCmd(&obj->arr[n], &obj->arr[i], &si);
#endif
#endif
            if (diff) {
                n++;
                if (n!=i)
                    obj->arr[n] = obj->arr[i];
            } else {
                dupCnt++;
                if (obj->arr[i])
                    Jsi_DecrRefCount(interp, obj->arr[i]);
                obj->arr[i] = 0;
            }
        }
        obj->arrCnt -= dupCnt;
    }
#ifdef NO_QSORT_R
    curSortInfo = NULL;
#endif
    if (hasopt)
        Jsi_OptionsFree(interp, jsi_ArraySortOptions, &si, 0);
done:
    v = Jsi_ValueMakeObject(interp, NULL, obj);
    Jsi_ValueReplace(interp, ret, v);
    return JSI_OK;
    
    Jsi_ValueMakeNull(interp, ret);
    return JSI_OK;
}

static Jsi_RC jsi_ArraySpliceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        return Jsi_LogError("expected array object");
    int newlen, argc, istart, n, rhowmany, ilen, curlen;
    Jsi_Value *va, *start, *howmany;
    Jsi_Obj *nobj, *obj = _this->d.obj;
    
    start = Jsi_ValueArrayIndex(interp, args, 0);
    howmany = Jsi_ValueArrayIndex(interp, args, 1);
    argc = Jsi_ValueGetLength(interp, args);
    istart = 0;
    ilen = (argc>=2 ? argc - 2 : 0);
    n = jsi_SizeOfArray(interp, obj);
    curlen = n;
    
    if (!start) {
        goto bail2;
    }

    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    Jsi_ObjSetLength(interp, nobj, 0);
    
    /* Determine start index. */
    Jsi_Number nstart;
    if (Jsi_GetNumberFromValue(interp, start, &nstart) == JSI_OK) {
        istart = (int)nstart;
        if (istart > n)
            goto bail;
        if (istart < 0)
            istart = (n+istart);
        if (istart<0)
            istart=0;
    }
      
    Jsi_Number nhow;
    rhowmany = n-istart;
    if (howmany && Jsi_GetNumberFromValue(interp, howmany, &nhow) == JSI_OK) {
        rhowmany = (int)nhow;
        if (rhowmany >= (n-istart))
            rhowmany = n-istart;
        if (rhowmany < 0)
            rhowmany = (n-istart);
        if (rhowmany<0)
            goto bail;
    }
    
    if (curlen < 0) {
        Jsi_ObjSetLength(interp, obj, curlen=0);
    }
    Jsi_ObjListifyArray(interp, obj);
   
    Jsi_ObjArraySizer(interp, nobj, rhowmany);

    /* Move elements to return object. */
    int i, j, m;
    for (m=0, j = 0, i = istart; m<rhowmany && m<curlen; m++, i++, j++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[m] = obj->arr[i];
        obj->arr[i] = NULL;
    }
    Jsi_ObjSetLength(interp, nobj, m);
    
    /* Shift remaining down. */
    for (; rhowmany && i<curlen; i++)
    {
        obj->arr[i-rhowmany] = obj->arr[i];
        obj->arr[i] = NULL;
    }
    curlen -= j;
    /* Add elements. */
    newlen = curlen + argc - (argc>=2?2:1);
    if (Jsi_ObjArraySizer(interp, obj, newlen+3) <= 0) {
        Jsi_LogError("too long");
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_ERROR;
    }
    if (ilen>0) {
        for (i = curlen-1; i>=istart; i--) {
            obj->arr[i+ilen] = obj->arr[i];
            obj->arr[i] = NULL;
        }
        for (m=istart, i = 2; i<argc; m++,i++) {
            va = Jsi_ValueArrayIndex(interp, args, i);
            if (!va) continue;
            obj->arr[m] = NULL;
            Jsi_ValueDup2(interp, obj->arr+m, va);
        }
    }
    Jsi_ObjSetLength(interp, obj, newlen);
bail:    
    return JSI_OK;
     
            
bail2:
    Jsi_ValueMakeNull(interp, ret);
    return JSI_OK;
}

static Jsi_RC jsi_ArrayConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int argc = Jsi_ValueGetLength(interp, args), iscons = Jsi_FunctionIsConstructor(funcPtr);
    Jsi_Value *target;
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    
    if (iscons) {
        target = _this;
        Jsi_ValueMakeArrayObject(interp, &_this, Jsi_ObjNewArray(interp, NULL, 0, 0));
    } else {
        Jsi_Obj *o = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        o->__proto__ = interp->Array_prototype;
        Jsi_ValueMakeObject(interp, ret, o);
        target = *ret;
    }

    if (argc == 1 && v && Jsi_ValueIsNumber(interp, v)) {
        Jsi_Number nv;
        Jsi_GetNumberFromValue(interp,v, &nv);
        int len = (int)nv;
        if (!Jsi_NumberIsInteger(v->d.num) || len < 0) 
            return Jsi_LogError("Invalid array length");
        target->d.obj->isarrlist = 1;
        if (Jsi_ObjArraySizer(interp, target->d.obj, len) <= 0)
            return JSI_ERROR;
    } else {
    
        int i;
        target->d.obj->isarrlist = 1;
        if (Jsi_ObjArraySizer(interp, target->d.obj, 0) <= 0)
                return JSI_ERROR;
        
        for (i = 0; i < argc; ++i) {
            Jsi_Value *argv = Jsi_ValueArrayIndex(interp, args, i);   ;
            Jsi_ValueInsertArray(interp, _this, i, argv, 0);
        }
    }
    if (iscons)
        Jsi_ValueDup2(interp, ret, target);
    return JSI_OK;
}
            
static Jsi_CmdSpec arrayCmds[] = {
    { "Array",      jsi_ArrayConstructor,   0,-1, "...", .help="jsi_Array constructor", .retType=(uint)JSI_TT_ARRAY, .flags=JSI_CMD_IS_CONSTRUCTOR },
    { "concat",     jsi_ArrayConcatCmd,     0,-1, "...", .help="Return array with args appended", .retType=(uint)JSI_TT_ARRAY },
    { "every",      jsi_ArrayEveryCmd,      1, 1, "callback:function", .help="Returns true if every value in array satisfies the test", .retType=(uint)JSI_TT_ANY },
    { "fill",       jsi_ArrayFillCmd,       1, 3, "value:any, start:number=0, end:number=-1", .help="Fill an array with values", .retType=(uint)JSI_TT_ARRAY },
    { "filter",     jsi_ArrayFilterCmd,     1, 2, "callback:function, this:object=void", .help="Return a filtered array", .retType=(uint)JSI_TT_ARRAY },
    { "find",       jsi_ArrayFindCmd,       1, 1, "callback:function", .help="Returns the value of the first element in the array that satisfies the test", .retType=(uint)JSI_TT_ANY },
    { "findIndex",  jsi_ArrayFindIndexCmd,  1, 1, "callback:function", .help="Returns the index of the first element in the array that satisfies the test", .retType=(uint)JSI_TT_ANY },
    { "flat",       jsi_ArrayFlatCmd,       0, 1, "depth:number=1", .help="Flatten an arra", .retType=(uint)JSI_TT_ARRAY },
    { "forEach",    jsi_ArrayForeachCmd,    1, 2, "callback:function, this:object=void", .help="Invoke function with each item in object", .retType=(uint)JSI_TT_VOID },
    { "includes",   jsi_ArrayIncludesCmd,   1, 1, "val:any", .help="Returns true if array contains value", .retType=(uint)JSI_TT_ANY },
    { "indexOf",    jsi_ArrayIndexOfCmd,    1, 2, "str:any, startIdx:number=0", .help="Return index of first occurrance in array", .retType=(uint)JSI_TT_NUMBER },
    { "isArray",    jsi_ArrayIsArrayCmd,    0, 0, "", .help="True if val array", .retType=(uint)JSI_TT_BOOLEAN },
    { "join",       jsi_ArrayJoinCmd,       0, 1, "sep:string=''", .help="Return elements joined by char", .retType=(uint)JSI_TT_STRING },
    { "lastIndexOf",jsi_ArrayLastindexOfCmd,1, 2, "val:any, start:number=0", .help="Return index of last occurence in array", .retType=(uint)JSI_TT_NUMBER },
    { "map",        jsi_ArrayMapCmd,        1, 2, "callback:function, this:object=void", .help="Creates a new array with the results of calling a provided function on every element in this array", .retType=(uint)JSI_TT_ARRAY },
    { "pop",        jsi_ArrayPopCmd,        0, 0, "", .help="Remove and return last element of array", .retType=(uint)JSI_TT_ANY },
    { "push",       jsi_ArrayPushCmd,       1,-1, "val:any, ...", .help="Push one or more elements onto array and return size", .retType=(uint)JSI_TT_NUMBER },
    { "reduce",     jsi_ArrayReduceCmd,     1, 2, "callback:function, initial:any", .help="Return a reduced array", .retType=(uint)JSI_TT_ANY },
    { "reduceRight",jsi_ArrayReduceRightCmd,1, 2, "callback:function, initial:any", .help="Return a reduced array", .retType=(uint)JSI_TT_ANY },
    { "shift",      jsi_ArrayShiftCmd,      0, 0, "", .help="Remove first element and shift downwards", .retType=(uint)JSI_TT_ANY },
    { "sizeOf",     jsi_ArraySizeOfCmd,     0, 0, "", .help="Return size of array", .retType=(uint)JSI_TT_NUMBER },
    { "slice",      jsi_ArraySliceCmd,      1, 2, "start:number, end:number=void", .help="Return sub-array", .retType=(uint)JSI_TT_ARRAY },
    { "some",       jsi_ArraySomeCmd,       1, 2, "callback:function, this:object=void", .help="Return true if function returns true some element", .retType=(uint)JSI_TT_BOOLEAN },
    { "sort",       jsi_ArraySortCmd,       0, 1, "options:function|object=void", .help="Sort an array", .retType=(uint)JSI_TT_ARRAY, .flags=0, .info=0, .opts=jsi_ArraySortOptions },
    { "splice",     jsi_ArraySpliceCmd,     1,-1, "start:number, howmany:number=void, ...", .help="Change the content of an array, adding new elements while removing old elements", .retType=(uint)JSI_TT_ARRAY },
    { "reverse",    jsi_ArrayReverseCmd,    0, 0, "", .help="Reverse order of all elements in an array", .retType=(uint)JSI_TT_ARRAY },
    { "unshift",    jsi_ArrayUnshiftCmd,    0,-1, "...", .help="Add new elements to start of array and return size", .retType=(uint)JSI_TT_NUMBER },
    { NULL, 0,0,0,0, .help="Provide access to array objects" }
};

Jsi_RC jsi_InitArray(Jsi_Interp *interp, int release)
{
    if (release) return JSI_OK;
    interp->Array_prototype = Jsi_CommandCreateSpecs(interp, "Array", arrayCmds, NULL, JSI_CMDSPEC_ISOBJ);
    return JSI_OK;
}

#endif
