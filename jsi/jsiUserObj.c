#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

int Jsi_UserObjUnregister(Jsi_Interp *interp, Jsi_UserObjReg *udreg)
{
    Jsi_HashEntry *hPtr;
    if (interp->deleting) return JSI_ERROR;
    hPtr = Jsi_HashEntryFind(interp->userdataTbl, udreg->name);
    if (hPtr == NULL)
        return JSI_ERROR;
    
    Jsi_HashEntryDelete(hPtr);
    UserObjReg* ptr = (UserObjReg*)Jsi_HashValueGet(hPtr);
    SIGASSERT(ptr, USER_REG);
    Jsi_Free(ptr);
    return JSI_OK;
}

int jsi_UserObjDelete(Jsi_Interp *interp, void *data)
{
    UserObjReg* ptr = (UserObjReg*)data;
    SIGASSERT(ptr, USER_REG);
    Jsi_Hash *tblPtr = ptr->hashPtr;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch search;
    for (hPtr = Jsi_HashEntryFirst(tblPtr, &search);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
        void *dptr;
        Jsi_Obj *obj = (Jsi_Obj*)Jsi_HashValueGet(hPtr);
        SIGASSERT(obj, OBJ);
        assert(obj && obj->ot == JSI_OT_USEROBJ);
        Jsi_UserObj *uobj = obj->d.uobj;
        dptr = uobj->data;
        if (hPtr == uobj->hPtr)
            uobj->hPtr = NULL;
        Jsi_HashEntryDelete(hPtr);
        if (dptr && ptr->reg->freefun)
            ptr->reg->freefun(interp, dptr);
            uobj->data = NULL;
    }
    Jsi_HashDelete(tblPtr);
    Jsi_Free(ptr);
    return JSI_OK;
}

Jsi_Hash* Jsi_UserObjRegister(Jsi_Interp *interp, Jsi_UserObjReg *udreg)
{
    UserObjReg* ptr = (UserObjReg*)Jsi_Calloc(1, sizeof(*ptr));
    Jsi_HashEntry *hPtr;
    SIGINIT(ptr, USER_REG);
    int isNew;
    ptr->reg = udreg;
    ptr->hashPtr = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, NULL);
    //ptr->data = data;
    hPtr = Jsi_HashEntryNew(interp->userdataTbl, udreg->name, &isNew);
    if (hPtr == NULL || !isNew)
        return NULL;
    Jsi_HashValueSet(hPtr, ptr);
    
    //Jsi_InterpSetData(interp, udreg->name, UserObjDelete, ptr);
    return ptr->hashPtr;
}

void jsi_UserObjFree(Jsi_Interp *interp, Jsi_UserObj *uobj)
{
    Jsi_UserObjReg *udr =uobj->reg;
    if (interp != uobj->interp) {
        Jsi_LogError("UDID bad interp");
        return;
    }
    if (uobj->hPtr)
        Jsi_HashEntryDelete(uobj->hPtr);
    if (udr->freefun && uobj->data) {
        udr->freefun(interp, uobj->data);
        uobj->data = NULL;
    }
    MEMCLEAR(uobj);
    Jsi_Free(uobj);
}

static Jsi_UserObj *UserObjNew(Jsi_Interp *interp, Jsi_Hash* id, void *data)
{
    Jsi_UserObj *ud = (Jsi_UserObj*)Jsi_Calloc(1,sizeof(Jsi_UserObj));
    ud->interp = interp;
    SIGINIT(ud,USERDATA);
    ud->id = id;
    ud->data = data;
    return ud;
}

int Jsi_UserObjNew(Jsi_Interp *interp, Jsi_UserObjReg* reg, Jsi_Obj *obj, void *data)
{
    if (obj->ot != JSI_OT_OBJECT) {
        Jsi_LogBug("jsi_userdata_assign to a non raw object");
        return -1;
    }
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->userdataTbl, reg->name);
    if (!hPtr) {
        Jsi_LogBug("no such registered object");
        return -1;
    }
    UserObjReg* ureg = (UserObjReg*)Jsi_HashValueGet(hPtr);
    interp->userObjCnt++;
    Jsi_UserObj * uobj = UserObjNew(interp, ureg->hashPtr, data);
    obj->d.uobj = uobj;
    uobj->reg = reg;
    uobj->ureg = ureg;
    obj->ot = JSI_OT_USEROBJ;
    uobj->idx = ++ureg->idx;
    int isNew;
    uobj->hPtr = Jsi_HashEntryNew(ureg->hashPtr, (void*)uobj->idx, &isNew);
    assert(uobj->hPtr && isNew==1);
    Jsi_HashValueSet(uobj->hPtr, obj);
    return uobj->idx;
}

static void *UserObjGet(Jsi_Interp *interp, Jsi_Obj *obj, Jsi_Hash* id)
{
    Jsi_UserObj *ud = obj->d.uobj;
    if (!ud)
        return NULL;
    if (obj->ot != JSI_OT_USEROBJ || ud->interp != interp) {
        if (ud->interp != interp)
            Jsi_LogWarn("Jsi_Obj not userobj type");
        return NULL;
    }
    if (ud->id != id) {
        Jsi_LogWarn("Get_userdata, id not match");
        return NULL;
    }
    return ud->data;
}

void *Jsi_UserObjGetData(Jsi_Interp *interp, Jsi_Value* value, Jsi_Func *funcPtr)
{
    if (value == NULL || value->vt != JSI_VT_OBJECT || value->d.obj->ot != JSI_OT_USEROBJ)
        return NULL;
    if (!funcPtr)
        return value->d.obj->d.uobj->data;
    Jsi_Obj *obj = Jsi_ValueGetObj(interp, value);
    if (!obj)
        return NULL;
    if (obj->ot != JSI_OT_USEROBJ)
        return NULL;
    void *privData = funcPtr->privData;
    return UserObjGet(interp, obj, (Jsi_Hash*)privData);
}


void *Jsi_UserObjDataFromVar(Jsi_Interp *interp, const char *var) {
    Jsi_Value *vObj = Jsi_NameLookup(interp, var);
    if (!vObj)
        return NULL;
    return Jsi_UserObjGetData(interp, vObj, NULL);
}

int jsi_UserObjIsTrue(Jsi_Interp *interp, Jsi_UserObj *uobj)
{
    Jsi_UserObjReg *udr = uobj->reg;
    if (udr->istrue) {
        return udr->istrue(uobj->data);
    }
    return 1;
}

void jsi_UserObjToName(Jsi_Interp *interp, Jsi_UserObj *uobj, Jsi_DString *dStr)
{
    const char * uname = "userdata";
    char ubuf[50];
    Jsi_UserObjReg *reg = uobj->reg;
    uname = reg->name;
    sprintf(ubuf, "%d", uobj->idx);
    Jsi_DSAppend(dStr, "#", uname, "_", ubuf, NULL);
}

Jsi_Obj *jsi_UserObjFromName(Jsi_Interp *interp, const char *name)
{
    if (*name != '#')
        return NULL;
    const char *cp = strrchr(name, '_');
    if (cp==0 || !*cp)
        return NULL;
    int id = atoi(cp+1);
    Jsi_DString dStr = {};
    Jsi_DSAppendLen(&dStr, name+1, cp-name-1);
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->userdataTbl, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);

    UserObjReg *rdata = (UserObjReg*)Jsi_HashValueGet(hPtr);
    if (!rdata)
        return NULL;
    Jsi_Hash *tPtr = rdata->hashPtr;
    if (tPtr==0)
        return NULL;

    hPtr = Jsi_HashEntryFind(tPtr, (void*)id);
    if (!hPtr)
        return NULL;
    return (Jsi_Obj*)Jsi_HashValueGet(hPtr);
}


int jsi_UserObjDump(Jsi_Interp *interp, const char *argStr, Jsi_Obj *nobj)
{
    char *key;
    int n = 0;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch search;
    Jsi_Hash *tPtr;
    
    if (argStr == NULL) {
        for (hPtr = Jsi_HashEntryFirst(interp->userdataTbl, &search);
            hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
            key = (char*)Jsi_HashKeyGet(hPtr);
            Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringKey(interp, key), n++);
        }
        return JSI_OK;
    }
    hPtr = Jsi_HashEntryFind(interp->userdataTbl, argStr);
    if (hPtr == NULL) {
        Jsi_LogError("no such user object: %s", argStr);
        return JSI_ERROR;
    }
    UserObjReg *rdata = (UserObjReg*)Jsi_HashValueGet(hPtr);
    if (!rdata)
        return JSI_OK;
    tPtr = rdata->hashPtr;
    if (tPtr==0)
        return JSI_OK;
    for (hPtr = Jsi_HashEntryFirst(tPtr, &search);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
        Jsi_Obj *fobj;
        if (!(fobj = (Jsi_Obj*)Jsi_HashValueGet(hPtr)))
            continue;
        assert(fobj->ot == JSI_OT_USEROBJ);
        /* TODO: incr refcount??? */
        Jsi_ObjArraySet(interp, nobj, Jsi_ValueMakeObject(interp, NULL, fobj), n++);
    }

    return JSI_OK;
}

#endif
