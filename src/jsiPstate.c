#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

// SCOPESTRS

Jsi_ScopeStrs *jsi_ScopeStrsNew()
{
    Jsi_ScopeStrs *ret = (Jsi_ScopeStrs *)Jsi_Calloc(1, sizeof(*ret));
    return ret;
}

void jsi_ScopeStrsFree(Jsi_Interp *interp, Jsi_ScopeStrs *ss)
{
    if (!ss) return;
    int i;
    for (i=0; i<ss->count; i++)
        if (ss->args[i].defValue)
            Jsi_DecrRefCount(interp, ss->args[i].defValue);
    if (ss->args)
        Jsi_Free(ss->args);
    Jsi_Free(ss);
}

void jsi_ScopeStrsPush(Jsi_Interp *interp, Jsi_ScopeStrs *ss, const char *string, int argType)
{
    if (ss->count >= ss->_size) {
        int osz = ss->_size, isold = (ss->args!=NULL);
        ss->_size += ALLOC_MOD_SIZE;
        if (!isold)
            ss->args = (jsi_ArgValue*)Jsi_Calloc(ss->_size,  sizeof(ss->args[0]));
        else {
            ss->args = (jsi_ArgValue*)Jsi_Realloc(ss->args, (ss->_size) * sizeof(ss->args[0]));
            memset(ss->args+osz, 0, (ss->_size-osz)* sizeof(ss->args[0]));
        }
    }
    ss->args[ss->count].name = (char*)Jsi_KeyAdd(interp, string);
    ss->args[ss->count].type = argType;
    if (argType)
        ss->typeCnt++;
    ss->count++;
}

static Jsi_ScopeStrs *jsi_ScopeStrsDup(jsi_Pstate *ps, Jsi_ScopeStrs *ss)
{
    Jsi_ScopeStrs *n = jsi_ScopeStrsNew();
    int i;
    if (!ss) return n;
    *n = *ss;
    if (!ss->args) return n;
    n->args = (jsi_ArgValue*)Jsi_Calloc(n->count, sizeof(ss->args[0]));
    n->_size = n->count;
    memcpy(n->args, ss->args, (n->count *  sizeof(ss->args[0])));
    for (i = 0; i < ss->count; ++i) {
        if (ss->args[i].defValue)
            Jsi_IncrRefCount(ps->interp, ss->args[i].defValue);
    }
    return n;
}

const char *jsi_ScopeStrsGet(Jsi_ScopeStrs *ss, int i)
{
    if (i < 0 || i >= ss->count)
        return NULL;
    return ss->args[i].name;
}

Jsi_ScopeStrs *jsi_ArgsOptAdd(jsi_Pstate *pstate, Jsi_ScopeStrs *a)
{
    jsi_PstatePush(pstate);
    return a;
}

Jsi_ScopeStrs *jsi_argInsert(jsi_Pstate *pstate, Jsi_ScopeStrs *a, const char *name, Jsi_Value *defValue, jsi_Pline *lPtr, bool prepend)
{
    Jsi_Interp *interp = pstate->interp;
    if (!a)
        a = jsi_ScopeStrsNew();
    pstate->args = a;
    int atyp = pstate->argType;
    if (defValue) {
        int vt = defValue->vt;
        if (vt == JSI_VT_NULL)
            vt = JSI_TT_NULL;
        else if (vt == JSI_VT_UNDEF && defValue->d.num==1)
            vt = JSI_TT_VOID;
        else if (vt == JSI_VT_OBJECT && defValue->d.obj->ot==JSI_OT_STRING)
            vt = JSI_TT_STRING;
        else
            vt = (1<<defValue->vt);
        atyp |= vt;
    }
    jsi_ScopeStrsPush(interp, a, name, atyp);
    pstate->argType = 0;
    a->args[a->count-1].defValue = defValue;
    a->argCnt++;
    if (prepend) {
        jsi_ArgValue t;
        int i, end = a->argCnt-1;
        t = a->args[end];
        for (i=end-1; i>=0; i--)
            a->args[i+1]=a->args[i];
        a->args[0] = t;
    }
    jsi_Pline *opl = interp->parseLine;
    interp->parseLine = lPtr;
    if (defValue) {
        Jsi_IncrRefCount(interp, defValue);
        if (a->firstDef==0)
            a->firstDef = a->argCnt;
            if (atyp)
                jsi_ArgTypeCheck(interp, atyp, defValue, "default value", name, a->argCnt, NULL, 1);
    } else {
        if (a->firstDef && (interp->typeCheck.run || interp->typeCheck.all) )
            Jsi_LogWarn("expected default value in argument list: \"%s\"", name);
    }
    interp->parseLine = opl;
    return a;
}


// PSTATE 

void jsi_PstatePush(jsi_Pstate *ps)
{
    Jsi_Interp *interp = ps->interp;
    if (interp->cur_scope >= (int)(JSI_MAX_SCOPE - 1)) {
        Jsi_LogBug("Scope chain too short");
        return;
    }
    interp->cur_scope++;
}

void jsi_PstatePop(jsi_Pstate *ps)
{
    Jsi_Interp *interp = ps->interp;
    if (interp->cur_scope <= 0) {
        Jsi_LogBug("No more scope to pop");
        return;
    }
    jsi_ScopeStrsFree(interp, interp->scopes[interp->cur_scope]);
    interp->scopes[interp->cur_scope] = NULL;
    interp->cur_scope--;
}

void jsi_PstateAddVar(jsi_Pstate *ps, jsi_Pline *line, const char *str)
{
    Jsi_Interp *interp = ps->interp;
    int i;
    if (interp->scopes[interp->cur_scope] == NULL)
        interp->scopes[interp->cur_scope] = (Jsi_ScopeStrs *)jsi_ScopeStrsNew();
    
    for (i = 0; i < interp->scopes[interp->cur_scope]->count; ++i) {
        if (Jsi_Strcmp(str, interp->scopes[interp->cur_scope]->args[i].name) == 0) {
            Jsi_Interp *interp = ps->interp;
            if (interp && interp->strict) {
                interp->parseLine = line;
                Jsi_LogWarn("duplicate var: %s", str);
                interp->parseLine = NULL;
            }
            return;
        }
    }
    jsi_ScopeStrsPush(ps->interp, interp->scopes[interp->cur_scope], str, JSI_VT_UNDEF);
}

Jsi_ScopeStrs *jsi_ScopeGetVarlist(jsi_Pstate *ps)
{
    Jsi_Interp *interp = ps->interp;
    return jsi_ScopeStrsDup(ps, interp->scopes[interp->cur_scope]);
}

#if 0
static int fastVarFree(Jsi_Interp *interp, void *ptr) {
    FastVar *fv = ptr;
    Jsi_Value *v = fv->var.lval;
    if (v) {
        //printf("FV FREE: %p (%d/%d)\n", fv, v->refCnt, v->vt == JSI_VT_OBJECT?v->d.obj->refcnt:-99);
        //Jsi_DecrRefCount(interp, v);
    }
    return JSI_OK;
}
#endif


static Jsi_RC jsi_StringFree(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *d) {
    Jsi_String *s = (Jsi_String *)d;
    if (s->flags&1)
        Jsi_Free(s->str);
    Jsi_Free(s);
    return JSI_OK;
}

jsi_Pstate *jsi_PstateNew(Jsi_Interp *interp)
{
    jsi_Pstate *ps = (jsi_Pstate *)Jsi_Calloc(1,sizeof(*ps));
    SIGINIT(ps,PARSER);
    ps->lexer = (jsi_Lexer*)Jsi_Calloc(1,sizeof(*ps->lexer));
    ps->lexer->pstate = ps;
    ps->interp = interp;
    ps->fastVarTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, NULL /*fastVarFree*/);
    ps->strTbl = Jsi_HashNew(interp, JSI_KEYS_ONEWORD, jsi_StringFree);
    return ps;
}

const char *jsi_PstateGetFilename(jsi_Pstate *ps)
{
    Jsi_Interp *interp = ps->interp;
    return interp->curFile;
}

void jsi_PstateClear(jsi_Pstate *ps)
{
    jsi_Lexer* l = ps->lexer;
    if (l->ltype == LT_FILE)
    {
        if (l->d.fp)
            Jsi_Close(ps->interp, l->d.fp);
        l->d.fp = NULL;
    }
    if (l->ltype == LT_STRING)
    {
        l->d.str = NULL;
    }
    l->ltype = LT_NONE;
    l->last_token = 0;
    l->cur_line = 1;
    l->cur_char = 0;
    l->cur = 0;
    ps->err_count = 0;
}

int jsi_PstateSetFile(jsi_Pstate *ps, Jsi_Channel fp, int skipbang)
{
    jsi_Lexer *l = ps->lexer;
    jsi_PstateClear(ps);
    l->ltype = LT_FILE;
    l->d.fp = fp;
    Jsi_Rewind(ps->interp, fp);
    if (skipbang) {
        char buf[JSI_BUFSIZ];
        if (Jsi_Gets(ps->interp, fp, buf, sizeof(buf)) && (buf[0] != '#' || buf[1] != '!')) {
            Jsi_Rewind(ps->interp, fp);
        }
    }
            
    return JSI_OK;
}


int jsi_PstateSetString(jsi_Pstate *ps, const char *str)
{
    Jsi_Interp *interp = ps->interp;
    jsi_Lexer *l = ps->lexer;
    jsi_PstateClear(ps);
    l->ltype = LT_STRING;
    Jsi_HashEntry *hPtr = Jsi_HashEntryNew(interp->codeTbl, (void*)str, NULL);
    assert(hPtr);
    l->d.str = (char*)Jsi_HashKeyGet(hPtr);
    return JSI_OK;
}

void jsi_PstateFree(jsi_Pstate *ps)
{
    /* TODO: when do we free opcodes */
    jsi_PstateClear(ps);
    Jsi_Free(ps->lexer);
    if (ps->opcodes)
        jsi_FreeOpcodes(ps->opcodes);
    if (ps->hPtr)
        Jsi_HashEntryDelete(ps->hPtr);
    if (ps->argsTbl)
        Jsi_HashDelete(ps->argsTbl);
    if (ps->fastVarTbl)
        Jsi_HashDelete(ps->fastVarTbl);
    if (ps->strTbl)
        Jsi_HashDelete(ps->strTbl);
    if (ps->last_exception)
        Jsi_DecrRefCount(ps->interp, ps->last_exception);
    _JSI_MEMCLEAR(ps);
    Jsi_Free(ps);
}

#endif
