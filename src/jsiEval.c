/* The interpreter evaluation engine for jsi. */
#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#include <math.h>

#define _jsi_STACK (interp->Stack)
#define _jsi_STACKIDX(s) interp->Stack[s]
#define _jsi_TOP (interp->Stack[interp->framePtr->Sp-1])
#define _jsi_TOQ (interp->Stack[interp->framePtr->Sp-2])
#define _jsi_THIS (interp->Obj_this)
#define _jsi_THISIDX(s) interp->Obj_this[s]

static Jsi_RC jsi_EvalLogErr(Jsi_Interp *interp, const char *str) { Jsi_LogMsg(interp, JSI_ERROR, "%s", str); return JSI_ERROR; }

#define _jsi_StrictChk(v) ((strict==0 || !Jsi_NumberIsNaN(v->d.num)) ? JSI_OK : jsi_EvalLogErr(interp, "value is NaN"))
#define _jsi_StrictChk2(v1,v2)  ((strict==0  || (Jsi_NumberIsNaN(v1->d.num)==0 && Jsi_NumberIsNaN(v2->d.num)==0))  ? JSI_OK : jsi_EvalLogErr(interp, "value is NaN"))
#define _jsi_StrictUChk(v) ((strict==0 || v->vt != JSI_VT_UNDEF) ? JSI_OK : jsi_EvalLogErr(interp, "value is undefined"))
#define _jsi_StrictUChk2(v1,v2)  ((strict==0  || (v1->vt != JSI_VT_UNDEF && v2->vt != JSI_VT_UNDEF))  ? JSI_OK : jsi_EvalLogErr(interp, "value is undefined"))
#define _jsi_StrictUChk3(v1,v2)  ((strict==0  || (v1->vt != JSI_VT_UNDEF || v2->vt == JSI_VT_UNDEF))  ? JSI_OK : jsi_EvalLogErr(interp, "lhs value undefined in ===/!==") )

static jsi_Pstate* jsiNewParser(Jsi_Interp* interp, const char *codeStr, Jsi_Channel fp, int iseval)
{
    bool isNew;
    Jsi_HashEntry *hPtr = NULL;
    hPtr = Jsi_HashEntryNew(interp->codeTbl, (void*)codeStr, &isNew);
    if (!hPtr) return NULL;
    jsi_Pstate *ps, *topPs;

    if (isNew==0 && ((ps = (jsi_Pstate *)Jsi_HashValueGet(hPtr)))) {
        interp->codeCacheHit++;
        return ps;
    }
    ps = jsi_PstateNew(interp);
    ps->eval_flag = iseval;
    if (codeStr)
        jsi_PstateSetString(ps, codeStr);
    else
        jsi_PstateSetFile(ps, fp, 1);
        
    interp->inParse++;
    topPs = interp->parsePs;
    interp->parsePs = ps;
    yyparse(ps);
    interp->parsePs = topPs;
    interp->inParse--;
    
    if (ps->err_count) {
        Jsi_HashEntryDelete(hPtr);
        jsi_PstateFree(ps);
        return NULL;
    }
    if (isNew) {
        Jsi_HashValueSet(hPtr, ps);
        ps->hPtr = hPtr;
    }
    return ps;
}

/* eval here is diff from Jsi_CmdProc, current scope Jsi_LogWarn should be past to eval */
/* make evaling script execute in the same context */
static Jsi_RC jsiEvalOp(Jsi_Interp* interp, jsi_Pstate *ps, char *program,
                       jsi_ScopeChain *scope, Jsi_Value *currentScope, Jsi_Value *_this, Jsi_Value **ret)
{
    Jsi_RC r = JSI_OK;
    jsi_Pstate *newps = jsiNewParser(interp, program, NULL, 1);
    if (newps) {
        int oef = newps->eval_flag;
        newps->eval_flag = 1;
        interp->ps = newps;
        r = jsi_evalcode(newps, NULL, newps->opcodes, scope, currentScope, _this, ret);
        if (r) {
            Jsi_ValueDup2(interp, &ps->last_exception, newps->last_exception);
        }
        newps->eval_flag = oef;
        interp->ps = ps;
    } else  {
        //Jsi_ValueMakeStringKey(interp, &ps->last_exception, "Syntax Error");
        r = JSI_ERROR;
    }
    return r;
}
                     
static Jsi_Value** jsi_ValuesAlloc(Jsi_Interp *interp, int cnt, Jsi_Value**old, int oldsz) {
    int i;
    Jsi_Value **v = (Jsi_Value **)Jsi_Realloc(old, cnt* sizeof(Jsi_Value*));
    for (i=oldsz; i<cnt; i++)
        v[i] = NULL;
    return v;
}

static void jsiSetupStack(Jsi_Interp *interp)
{
    int oldsz = interp->maxStack;
    if (interp->maxStack)
        interp->maxStack += STACK_INCR_SIZE;
    else
        interp->maxStack = STACK_INIT_SIZE;
    _jsi_STACK = jsi_ValuesAlloc(interp, interp->maxStack, _jsi_STACK, oldsz);
    _jsi_THIS = jsi_ValuesAlloc(interp, interp->maxStack, _jsi_THIS, oldsz); //TODO:!!! use interp->framePtr for this.
}

static void jsiPush(Jsi_Interp* interp, int n) {
    int i = 0;
    do {
        if (!_jsi_STACKIDX(interp->framePtr->Sp))
            _jsi_STACKIDX(interp->framePtr->Sp) = Jsi_ValueNew1(interp);
        if (!_jsi_THISIDX(interp->framePtr->Sp))
            _jsi_THISIDX(interp->framePtr->Sp) = Jsi_ValueNew1(interp);
        if (i++ >= n) break;
        interp->framePtr->Sp++;
    } while (1);
}

/* Before setting a value in the _jsi_STACK/obj, unlink any reference to it. */

static void jsiClearStack(Jsi_Interp *interp, int ofs) {
    Jsi_Value **vPtr = &_jsi_STACKIDX(interp->framePtr->Sp-ofs), *v = *vPtr;
    if (!v) return;
#ifndef XX_NEWSTACK
    Jsi_ValueReset(interp, vPtr);
#else
    if (v->refCnt<=1)
        Jsi_ValueReset(interp, vPtr);
    else {
        Jsi_DecrRefCount(interp, v);
        _jsi_STACKIDX(interp->framePtr->Sp-ofs) = Jsi_ValueNew1(interp);
    }
#endif
}

static void jsiClearThis(Jsi_Interp *interp, int ofs) {
    Jsi_Value **vPtr = &_jsi_THISIDX(ofs), *v = *vPtr;
    if (!v) return;
#ifndef XX_NEWSTACK
    Jsi_ValueReset(interp, vPtr);
#else
    if (v->refCnt<=1)
        Jsi_ValueReset(interp, vPtr);
    else {
        Jsi_DecrRefCount(interp, v);
        _jsi_THISIDX(ofs) = Jsi_ValueNew1(interp);
    }
#endif
}


static Jsi_RC inline jsi_ValueAssign(Jsi_Interp *interp, Jsi_Value *dst, Jsi_Value* src, int lop)
{
    Jsi_Value *v;
    if (dst->vt != JSI_VT_VARIABLE) {
        if (interp->strict) 
            return Jsi_LogError("operand not a left value");
    } else {
        v = dst->d.lval;
        SIGASSERT(v, VALUE);
        int strict = interp->strict;
        if (strict && lop == OP_PUSHFUN && interp->curIp[-1].local)
            dst->f.bits.local = 1;
        if (strict && dst->f.bits.local==0) {
            const char *varname = "";
            if (v->f.bits.lookupfailed)
                varname = v->d.lookupFail;
            Jsi_RC rc = Jsi_LogType("function created global: \"%s\"", varname);
            dst->f.bits.local=1;
            if (rc != JSI_OK)
                return rc;
        }
        if (v == src)
            return JSI_OK;
        if (v->f.bits.readonly) {
            if (interp->strict) 
                return Jsi_LogError("assign to readonly variable");
            return JSI_OK;
        }
        if (Jsi_ValueIsFunction(interp, src))
            Jsi_ValueMove(interp,v, src);
        else
            Jsi_ValueCopy(interp,v, src);
        SIGASSERT(v, VALUE);
        v->f.bits.readonly = 0;
#ifdef JSI_MEM_DEBUG
    if (!v->VD.label2)
        v->VD.label2 = "ValueAssign";
#endif
    }
    return JSI_OK;
}

/* pop n values from _jsi_STACK */
static void jsiPop(Jsi_Interp* interp, int n) {
    int t = n;
    while (t > 0) {
        Assert((interp->framePtr->Sp-t)>=0);
/*        Jsi_Value *v = _jsi_STACKIDX(interp->framePtr->Sp-t);
         if (v->refCnt>1) puts("OO");*/
        jsiClearStack(interp,t);
        --t;
    }
    interp->framePtr->Sp -= n;
}

/* Convert preceding _jsi_STACK variable(s) into value(s). */
static void jsiVarDeref(Jsi_Interp* interp, int n) {
    while(interp->framePtr->Sp<n) // Assert and Log may map-out Ops.
        jsiPush(interp, 1);
    int i;
    for (i=1; i<=n; i++) {
        Jsi_Value *vb = _jsi_STACKIDX(interp->framePtr->Sp - i);
        if (vb->vt == JSI_VT_VARIABLE) {
            SIGASSERTV(vb->d.lval, VALUE);
            Jsi_ValueCopy(interp, vb, vb->d.lval);
        }
    }
}

#define common_math_opr(opr) {                      \
    jsiVarDeref(interp,2);                                     \
    Jsi_ValueToNumber(interp, _jsi_TOP);     \
    Jsi_ValueToNumber(interp, _jsi_TOQ);     \
    rc = _jsi_StrictChk2(_jsi_TOP, _jsi_TOQ); \
    _jsi_TOQ->d.num = _jsi_TOQ->d.num opr _jsi_TOP->d.num;            \
    jsiPop(interp, 1);                                          \
}

#define common_bitwise_opr(opr) {                       \
    Jsi_UWide a, b;                                       \
    jsiVarDeref(interp,2);                                     \
    Jsi_ValueToNumber(interp, _jsi_TOP);     \
    Jsi_ValueToNumber(interp, _jsi_TOQ);     \
    rc = _jsi_StrictChk2(_jsi_TOP, _jsi_TOQ); \
    a = _jsi_TOQ->d.num; b = _jsi_TOP->d.num;                   \
    _jsi_TOQ->d.num = (Jsi_Number)(a opr b);                  \
    jsiPop(interp, 1);                                          \
}

static Jsi_RC jsi_logic_less(Jsi_Interp* interp, int i1, int i2) {
    Jsi_Value *v, *v1 = _jsi_STACK[interp->framePtr->Sp-i1], *v2 = _jsi_STACK[interp->framePtr->Sp-i2], *res = _jsi_TOQ;
    int val = 0, l1 = 0, l2 = 0; 
    bool strict = interp->strict;
    Jsi_RC rc = JSI_OK;
    rc = _jsi_StrictUChk2(v1, v2);
    if (rc != JSI_OK)
        return JSI_ERROR;
    char *s1 = Jsi_ValueString(interp, v1, &l1);
    char *s2 = Jsi_ValueString(interp, v2, &l2);
    Jsi_Number n1, n2;

    if (s1 || s2) {
        char *str;
        if (!(s1 && s2)) {
            v = (s1 ? v2 : v1);
            jsi_ValueToPrimitive(interp, &v);
            Jsi_ValueToString(interp, v, NULL);
            str = Jsi_ValueString(interp, v, (s1?&l2:&l1));
            if (s1) s2 = str; else s1 = str;
        }
        Assert(l1>=0 && l1<=JSI_MAX_ALLOC_BUF);
        Assert(l2>=0 && l2<=JSI_MAX_ALLOC_BUF);
        //int mlen = (l1>l2?l1:l2);
        val = Jsi_Strcmp(s1, s2);
  
        if (val > 0) val = 0;
        else if (val < 0) val = 1;
        else val = (l1 < l2);
        jsiClearStack(interp,2);
        Jsi_ValueMakeBool(interp, &res, val);
    } else {
        Jsi_ValueToNumber(interp, v1);
        Jsi_ValueToNumber(interp, v2);
        rc = _jsi_StrictChk2(v1,v2);
        if (rc != JSI_OK)
            return JSI_ERROR;
        n1 = v1->d.num; n2 = v2->d.num;
        if (Jsi_NumberIsNaN(n1) || Jsi_NumberIsNaN(n2)) {
            jsiClearStack(interp,2);
            Jsi_ValueMakeUndef(interp, &res);
        } else {
            val = (n1 < n2);
            jsiClearStack(interp,2);
            Jsi_ValueMakeBool(interp, &res, val);
        }
    }
    return JSI_OK;
}

static const char *jsi_evalprint(Jsi_Value *v)
{
    static char buf[JSI_MAX_NUMBER_STRING];
    if (!v)
        return "nil";
    if (v->vt == JSI_VT_NUMBER) {
        snprintf(buf, sizeof(buf), "NUM:%" JSI_NUMGFMT " ", v->d.num);
    } else if (v->vt == JSI_VT_BOOL) {
        snprintf(buf, sizeof(buf), "BOO:%d", v->d.val);
    } else if (v->vt == JSI_VT_STRING) {
        snprintf(buf, sizeof(buf), "STR:'%s'", v->d.s.str);
    } else if (v->vt == JSI_VT_VARIABLE) {
        snprintf(buf, sizeof(buf), "VAR:%p", v->d.lval);
    } else if (v->vt == JSI_VT_NULL) {
        snprintf(buf, sizeof(buf), "NULL");
    } else if (v->vt == JSI_VT_OBJECT) {
        snprintf(buf, sizeof(buf), "OBJ:%p", v->d.obj);
    } else if (v->vt == JSI_VT_UNDEF) {
        snprintf(buf, sizeof(buf), "UNDEFINED");
    }
    return buf;
}
/* destroy top of trylist */
#define pop_try(head) jsi_pop_try(interp, &head)
static void jsi_pop_try(Jsi_Interp* interp, jsi_TryList **head)
{
    interp->framePtr->tryDepth--;
    jsi_TryList *t = (*head)->next;
    Jsi_Free(*head);
    *head = t;
    interp->tryList = t;
}

static void jsi_push_try(Jsi_Interp* interp, jsi_TryList **head, jsi_TryList *n)
{
    interp->tryList = n;
    interp->framePtr->tryDepth++;
    n->next = *head;
    *head = n;
}

/* restore scope chain */
#define JSI_RESTORE_SCOPE() jsi_restore_scope(interp, ps, trylist, \
    &scope, &currentScope, &context_id)
static void jsi_restore_scope(Jsi_Interp* interp, jsi_Pstate *ps, jsi_TryList* trylist,
  jsi_ScopeChain **scope, Jsi_Value **currentScope, int *context_id) {

/* JSI_RESTORE_SCOPE(scope_save, curscope_save)*/
    if (*scope != (trylist->scope_save)) {
        jsi_ScopeChainFree(interp, *scope);
        *scope = (trylist->scope_save);
        interp->framePtr->ingsc = *scope;
    }
    if (*currentScope != (trylist->curscope_save)) {
        Jsi_DecrRefCount(interp, *currentScope);
        *currentScope = (trylist->curscope_save); 
        interp->framePtr->incsc = *currentScope;
    }
    *context_id = ps->_context_id++; 
}

#define JSI_DO_THROW(nam) if (jsi_do_throw(interp, ps, &ip, &trylist,&scope, &currentScope, &context_id, (interp->framePtr->Sp?_jsi_TOP:NULL), nam) != JSI_OK) { rc = JSI_ERROR; break; }

static int jsi_do_throw(Jsi_Interp *interp, jsi_Pstate *ps, jsi_OpCode **ipp, jsi_TryList **tlp,
     jsi_ScopeChain **scope, Jsi_Value **currentScope, int *context_id, Jsi_Value *top, const char *nam) {
    if (Jsi_InterpGone(interp))
        return JSI_ERROR;
    jsi_TryList *trylist = *tlp;
    while (1) {
        if (trylist == NULL) {
            const char *str = (top?Jsi_ValueString(interp, top, NULL):"");
            if (str) {
                if (!Jsi_Strcmp(nam, "help"))
                    Jsi_LogError("...%s", str);
                else
                    Jsi_LogError("%s: %s", nam, str);
            }
            return JSI_ERROR;
        }
        if (trylist->type == jsi_TL_TRY) {
            int n = interp->framePtr->Sp - trylist->d.td.tsp;
            jsiPop(interp, n);
            if (*ipp >= trylist->d.td.tstart && *ipp < trylist->d.td.tend) {
                *ipp = trylist->d.td.cstart - 1;
                break;
            } else if (*ipp >= trylist->d.td.cstart && *ipp < trylist->d.td.cend) {
                trylist->d.td.last_op = jsi_LOP_THROW;
                *ipp = trylist->d.td.fstart - 1;
                break;
            } else if (*ipp >= trylist->d.td.fstart && *ipp < trylist->d.td.fend) {
                jsi_pop_try(interp, tlp);
                trylist = *tlp;
            } else Jsi_LogBug("Throw within a try, but not in its scope?");
        } else {
            jsi_restore_scope(interp, ps, trylist, scope, currentScope, context_id);
            jsi_pop_try(interp, tlp);
            trylist = *tlp;
        }
    }
    return JSI_OK;
}

static jsi_TryList *jsi_trylist_new(jsi_try_op_type t, jsi_ScopeChain *scope_save, Jsi_Value *curscope_save)
{
    jsi_TryList *n = (jsi_TryList *)Jsi_Calloc(1,sizeof(*n));
    
    n->type = t;
    n->curscope_save = curscope_save;
    /*Jsi_IncrRefCount(interp, curscope_save);*/
    n->scope_save = scope_save;
    
    return n;
}

static void jsiDumpInstr(Jsi_Interp *interp, jsi_Pstate *ps, Jsi_Value *_this,
    jsi_TryList *trylist, jsi_OpCode *ip, Jsi_OpCodes *opcodes)
{
    int i;
    char buf[JSI_MAX_NUMBER_STRING*2];
    jsi_code_decode(interp, ip, ip - opcodes->codes, buf, sizeof(buf));
    Jsi_Printf(interp, jsi_Stderr, "%p: %-30.200s : THIS=%s, STACK=[", ip, buf, jsi_evalprint(_this));
    for (i = 0; i < interp->framePtr->Sp; ++i) {
        Jsi_Printf(interp, jsi_Stderr, "%s%s", (i>0?", ":""), jsi_evalprint(_jsi_STACKIDX(i)));
    }
    Jsi_Printf(interp, jsi_Stderr, "]");
    if (ip->fname) {
        const char *fn = ip->fname,  *cp = Jsi_Strrchr(fn, '/');
        if (cp) fn = cp+1;
        Jsi_Printf(interp, jsi_Stderr, ", %s:%d", fn, ip->Line);
    }
    Jsi_Printf(interp, jsi_Stderr, "\n");
    jsi_TryList *tlt = trylist;
    for (i = 0; tlt; tlt = tlt->next) i++;
    if (ps->last_exception)
        Jsi_Printf(interp, jsi_Stderr, "TL: %d, excpt: %s\n", i, jsi_evalprint(ps->last_exception));
}

static int jsi_cmpstringp(const void *p1, const void *p2)
{
   return Jsi_Strcmp(* (char * const *) p1, * (char * const *) p2);
}

void jsi_SortDString(Jsi_Interp *interp, Jsi_DString *dStr, const char *sep) {
    int argc, i;
    char **argv;
    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    Jsi_SplitStr(Jsi_DSValue(dStr), &argc, &argv, sep, &sStr);
    qsort(argv, argc, sizeof(char*), jsi_cmpstringp);
    Jsi_DSSetLength(dStr, 0);
    for (i=0; i<argc; i++)
        Jsi_DSAppend(dStr, (i?" ":""), argv[i], NULL);
    Jsi_DSFree(&sStr);
}

static void jsi_ValueObjDelete(Jsi_Interp *interp, Jsi_Value *target, Jsi_Value *key, int force)
{
    if (target->vt != JSI_VT_OBJECT) return;
    const char *kstr = Jsi_ValueToString(interp, key, NULL);
    Jsi_TreeEntry *hPtr;
    if (!Jsi_ValueIsStringKey(interp, key)) {
        Jsi_MapEntry *hePtr = Jsi_MapEntryFind(target->d.obj->tree->opts.interp->strKeyTbl, kstr);
        if (hePtr)
            kstr = (char*)Jsi_MapKeyGet(hePtr, 0);
    }
    hPtr = Jsi_TreeEntryFind(target->d.obj->tree, kstr);
    if (hPtr == NULL || (hPtr->f.bits.dontdel && !force))
        return;
    Jsi_TreeEntryDelete(hPtr);
}

static void jsiObjGetNames(Jsi_Interp *interp, Jsi_Obj *obj, Jsi_DString* dStr, int flags) {
    Jsi_TreeEntry *hPtr;
    Jsi_TreeSearch srch;
    Jsi_Value *v;
    int m = 0;
    Jsi_DSInit(dStr);
    if (obj->isarrlist)
        obj = interp->Array_prototype->d.obj;
    else if (!obj->tree->numEntries && obj->__proto__)
        obj = obj->__proto__->d.obj;
    for (hPtr=Jsi_TreeSearchFirst(obj->tree, &srch,  JSI_TREE_ORDER_IN, NULL); hPtr; hPtr=Jsi_TreeSearchNext(&srch)) {
        v = (Jsi_Value*)Jsi_TreeValueGet(hPtr);
        if (!v) continue;
        if ((flags&JSI_NAME_FUNCTIONS) && !Jsi_ValueIsFunction(interp,v)) {
            continue;
        }
        if ((flags&JSI_NAME_DATA) && Jsi_ValueIsFunction(interp,v)) {
            continue;
        }

        Jsi_DSAppend(dStr, (m++?" ":""), Jsi_TreeKeyGet(hPtr), NULL);
    }
    Jsi_TreeSearchDone(&srch);
}

static void jsi_DumpFunctions(Jsi_Interp *interp, const char *spnam) {
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_MapEntry *hPtr;
    Jsi_MapSearch search;
    Jsi_CmdSpecItem *csi = NULL;
    Jsi_CmdSpec *cs;
    Jsi_Value *lsf = interp->lastSubscriptFail;
    Jsi_Obj *lso = ((lsf && lsf->vt == JSI_VT_OBJECT)?lsf->d.obj:0);
    const char *varname = NULL;
    int m = 0;
    
    if (lso) {
        spnam = interp->lastSubscriptFailStr;
        if (!spnam) spnam = interp->lastPushStr;
        if (!spnam) spnam = "";
        if (lso->ot == JSI_OT_USEROBJ && lso->d.uobj->reg && lso->d.uobj->interp == interp) {
            cs = lso->d.uobj->reg->spec;
            if (cs)
                goto dumpspec;
        } else if (lso->ot == JSI_OT_FUNCTION) {
            cs = lso->d.fobj->func->cmdSpec;
            if (cs)
                goto dumpspec;
        } else if (lso->ot == JSI_OT_OBJECT) {
            jsiObjGetNames(interp, lso, &dStr, JSI_NAME_FUNCTIONS);
            Jsi_LogError("'%s', functions are: %s.",
                spnam, Jsi_DSValue(&dStr));
            Jsi_DSFree(&dStr);
            return;
        } else {
            const char *sustr = NULL;
            switch (lso->ot) {
                case JSI_OT_STRING: sustr = "String"; break;
                case JSI_OT_NUMBER: sustr = "Number"; break;
                case JSI_OT_BOOL: sustr = "Boolean"; break;
                default: break;
            }
            if (sustr) {
                hPtr = Jsi_MapEntryFind(interp->cmdSpecTbl, sustr);
                csi = (Jsi_CmdSpecItem*)Jsi_MapValueGet(hPtr);
                cs = csi->spec;
                if (!spnam[0])
                    spnam = sustr;
                goto dumpspec;
            }
        }
    }
    if (!spnam) spnam = "";
    if (!*spnam) {
        for (hPtr = Jsi_MapSearchFirst(interp->cmdSpecTbl, &search, 0);
            hPtr; hPtr = Jsi_MapSearchNext(&search)) {
            csi = (Jsi_CmdSpecItem*)Jsi_MapValueGet(hPtr);
            if (csi->name && csi->name[0])
                Jsi_DSAppend(&dStr, (m++?" ":""), csi->name, NULL);
        }
        Jsi_MapSearchDone(&search);
    }
    
    varname = spnam;
    if ((hPtr = Jsi_MapEntryFind(interp->cmdSpecTbl, spnam))) {
        csi = (Jsi_CmdSpecItem*)Jsi_MapValueGet(hPtr);
        while (csi) {
            int n;
            cs = csi->spec;
dumpspec:
            n = 0;
            while (cs->name) {
                if (n != 0 || !(cs->flags & JSI_CMD_IS_CONSTRUCTOR)) {
                    if (!*cs->name) continue;
                    Jsi_DSAppend(&dStr, (m?" ":""), cs->name, NULL);
                    n++; m++;
                }
                cs++;
            }
            csi = (csi?csi->next:NULL);
        }
        jsi_SortDString(interp, &dStr, " ");
        if (varname)
            spnam = varname;
        else if (interp->lastPushStr && !spnam[0])
            spnam = interp->lastPushStr;
        Jsi_LogError("'%s' sub-commands are: %s.",
            spnam, Jsi_DSValue(&dStr));
        Jsi_DSFree(&dStr);
    } else {
        Jsi_LogError("can not execute expression: '%s' not a function",
            varname ? varname : "");
    }
}

/* Attempt to dynamically load function XX by doing an eval of Jsi_Auto.XX */
/* TODO: prevent infinite loop/recursion. */
Jsi_Value *jsi_LoadFunction(Jsi_Interp *interp, const char *str, Jsi_Value *tret) {
    Jsi_DString dStr = {};
    Jsi_Value *v;
    int i;
    const char *curFile = interp->curFile;
    interp->curFile = "<jsiLoadFunction>";
    for (i=0; i<2; i++) {
        Jsi_DSAppend(&dStr, "Jsi_Auto.", str, NULL);
        Jsi_VarLookup(interp, Jsi_DSValue(&dStr));
        v = Jsi_NameLookup(interp, Jsi_DSValue(&dStr));
        if (v)
            jsi_ValueDebugLabel(v, "jsiLoadFunction","f1");
        Jsi_DSFree(&dStr);
        if (v) {
            const char *cp = Jsi_ValueGetDString(interp, v, &dStr, 0);
            v = NULL;
            if (Jsi_EvalString(interp, cp, 0) == JSI_OK) {
                v = Jsi_NameLookup(interp, str);
                if (v)
                    jsi_ValueDebugLabel(v, "jsiLoadFunction","f2");
            }
            Jsi_DSFree(&dStr);
            if (v) {
                tret = v;
                break;
            }
        }
        if (interp->autoLoaded++ || i>0)
            break;
        /*  Index not in memory, so try loading Jsi_Auto from the autoload.jsi file. */
        if (interp->autoFiles == NULL)
            return tret;
        Jsi_Value **ifs = &interp->autoFiles;
        int i, ifn = 1;
        if (Jsi_ValueIsArray(interp, interp->autoFiles)) {
            ifs = interp->autoFiles->d.obj->arr;
            ifn = interp->autoFiles->d.obj->arrCnt;
        }
        for (i=0; i<ifn; i++) {  
            if (Jsi_EvalFile(interp, ifs[i], 0) != JSI_OK)
                break;
            interp->autoLoaded++;
        }
    }
    interp->curFile = curFile;
    return tret;
}

void jsi_TraceFuncCall(Jsi_Interp *interp, Jsi_Func *fstatic, jsi_OpCode *iPtr,
    Jsi_Value *_this, Jsi_Value* args, Jsi_Value *ret, int tc)
{
    jsi_OpCode *ip = (iPtr ? iPtr : interp->curIp);
    if (!ip)
        return;
    const char *ff, *fname = ip->fname?ip->fname:"";
    if ((tc&jsi_callTraceFullPath)==0 && ((ff=Jsi_Strrchr(fname,'/'))))
        fname = ff+1;
    if (interp->traceHook)
        (*interp->traceHook)(interp, fstatic->name, ip->fname, ip->Line, fstatic->cmdSpec, _this, args, ret);
    else {
        const char *fp = ((tc&jsi_callTraceNoParent)?NULL:fstatic->parentName);
        if (fp && !*fp)
            fp = NULL;
        Jsi_DString aStr;
        Jsi_DSInit(&aStr);
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        Jsi_DString pStr;
        Jsi_DSInit(&pStr);
        Jsi_DString *sPtr = NULL;
        int plen = 0;
        if (ret) {
            sPtr = &dStr;
            Jsi_DSAppend(sPtr, " <-- ", NULL);
            plen = Jsi_DSLength(sPtr);
            Jsi_ValueGetDString(interp, ret, sPtr, 0);
        } else if ((tc&jsi_callTraceArgs)) {
            sPtr = &aStr;
            Jsi_ValueGetDString(interp, args, sPtr, JSI_OUTPUT_JSON);
        }
        if (sPtr) {
            if (!(tc&jsi_callTraceNoTrunc)) {
                const char *cp0 = Jsi_DSValue(sPtr), *cp1 = Jsi_Strchr(cp0, '\n');
                int nlen = 0, clen = Jsi_DSLength(sPtr);
                if (cp1) {
                    nlen = (cp1-cp0);
                    if (nlen>60) nlen = 60;
                }  else if (clen>60)
                    nlen = 60;
                else nlen = clen;
                if (nlen != clen && clen>plen) {
                    Jsi_DSSetLength(sPtr, nlen);
                    Jsi_DSAppend(sPtr, "...", NULL);
                }
            }
        }
        if (interp->parent && interp->debugOpts.traceCallback) {
            Jsi_DString jStr={}, kStr={}, lStr={};
            Jsi_DSPrintf(&kStr, "[\"%s%s%s\", %s, %s, \"%s\", %d, %d ]",
                (fp?fp:""), (fp?".":""), fstatic->name, 
                (ret?"null":Jsi_JSONQuote(interp, Jsi_DSValue(&aStr),-1, &jStr)),
                (ret?Jsi_JSONQuote(interp, Jsi_DSValue(&dStr),-1, &lStr):"null"),
                 fname, ip->Line, ip->Lofs);
            if (Jsi_FunctionInvokeJSON(interp->parent, interp->debugOpts.traceCallback, Jsi_DSValue(&kStr), NULL) != JSI_OK)
                Jsi_DSPrintf(&pStr, "failed trace call\n");
            Jsi_DSFree(&jStr);
            Jsi_DSFree(&kStr);
            Jsi_DSFree(&lStr);
        } else if ((tc&jsi_callTraceBefore))
            Jsi_DSPrintf(&pStr, "%s:%d %*s#%d: %c %s%s%s(%s) %s\n",
                fname, ip->Line,
                (interp->level-1)*2, "", interp->level,
                (ret?'<':'>'), (fp?fp:""), (fp?".":""), fstatic->name, Jsi_DSValue(&aStr), Jsi_DSValue(&dStr));
        else {
            if (!interp->curIp || !interp->logOpts.before) {
                Jsi_DSPrintf(&pStr, "%*s#%d: %c %s%s%s(%s) in %s:%d%s\n", (interp->level-1)*2, "", interp->level,
                    (ret?'<':'>'), (fp?fp:""), (fp?".":""), fstatic->name, Jsi_DSValue(&aStr),
                fname, ip->Line, Jsi_DSValue(&dStr));
            } else {
                int quote = 0;
                jsi_SysPutsCmdPrefix(interp, &interp->logOpts, &pStr, &quote, NULL);
                Jsi_DSPrintf(&pStr, "%*s#%d: %c %s%s%s(%s) \n", (interp->level-1)*2, "", interp->level,
                    (ret?'<':'>'), (fp?fp:""), (fp?".":""), fstatic->name, Jsi_DSValue(&aStr));
                Jsi_DSAppend(&pStr, Jsi_DSValue(&dStr), NULL);
            }
        }
        if (Jsi_DSLength(&pStr))
            Jsi_Puts(interp, jsi_Stderr, Jsi_DSValue(&pStr), -1);
        Jsi_DSFree(&pStr);
        Jsi_DSFree(&dStr);
        Jsi_DSFree(&aStr);
    }
}

Jsi_RC jsi_FunctionSubCall(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Value *tocall, int discard)
{
    Jsi_RC rc = JSI_OK;
    const char *oldCurFunc = interp->curFunction, *spnam = "";
    jsi_OpCode *ip = interp->curIp;
    int adds, as_constructor = (ip->op == OP_NEWFCALL);
    double timStart = 0;
    int docall;
    int calltrc = 0, profile = interp->profile, coverage = interp->coverage;
    jsi_PkgInfo *pkg;
    int tc;
    
    //char *lpv = interp->lastPushStr;
    if (tocall->vt == JSI_VT_UNDEF && tocall->f.bits.lookupfailed && tocall->d.lookupFail && !interp->noAutoLoad) {
        spnam = tocall->d.lookupFail;
        tocall->f.bits.lookupfailed = 0;
        tocall = jsi_LoadFunction(interp, spnam, tocall);
        interp->lastPushStr = (char*)spnam;
        interp->curIp = ip;
    }
    if (!Jsi_ValueIsFunction(interp, tocall)) {
       // if (tocall->f.bits.subscriptfailed && tocall->d.lookupFail)
       //     spnam = tocall->d.lookupFail;
        jsi_DumpFunctions(interp, spnam);
        rc = JSI_ERROR;
        goto empty_func;
    }

    if (tocall->d.obj->d.fobj==NULL || tocall->d.obj->d.fobj->func==NULL) {   /* empty function */
empty_func:
        //jsiPop(interp, stackargc);
        //jsiClearStack(interp,1);
        //Jsi_ValueMakeUndef(interp, &_jsi_TOP);
        interp->curFunction = oldCurFunc;
        Jsi_DecrRefCount(interp, _this);
        if (rc==JSI_OK)
            rc = JSI_CONTINUE;
        return rc;
        //goto done;
    }
    
    Jsi_FuncObj *fobj = tocall->d.obj->d.fobj;
    Jsi_Func *funcPtr = fobj->func;
    if (funcPtr->callback == jsi_NoOpCmd || tocall->d.obj->isNoOp) {
        jsi_NoOpCmd(interp, NULL, NULL, NULL, NULL);
        goto empty_func;
    }
    if (!interp->asserts && funcPtr->callback == jsi_AssertCmd)
        goto empty_func;
    const char *onam = funcPtr->name;
//        if (!onam) // Override blank name with last index.
//            funcPtr->name = lpv;
    if (funcPtr->name && funcPtr->name[0] && funcPtr->type == FC_NORMAL)
        interp->curFunction = funcPtr->name;
    adds = funcPtr->callflags.bits.addargs;
    Jsi_CmdSpec *cs  = funcPtr->cmdSpec;
    if (adds && (cs->flags&JSI_CMDSPEC_NONTHIS))
        adds = 0;

    Jsi_Func *pprevActive = interp->prevActiveFunc, *prevActive = interp->prevActiveFunc = interp->activeFunc;
    interp->activeFunc = funcPtr;

    rc = jsi_SharedArgs(interp, args, funcPtr, 1); /* make arg vars to share arguments */
    if (rc != JSI_OK)
        goto bail;
    funcPtr->callflags.bits.addargs = 0;
    jsi_InitLocalVar(interp, args, funcPtr);
    jsi_SetCallee(interp, args, tocall);
    
    pkg = funcPtr->pkg;
    tc = interp->traceCall;
    if (pkg) {
        tc |= pkg->popts.modConf.traceCall;
        profile |= pkg->popts.modConf.profile;
        coverage |= pkg->popts.modConf.coverage;
    }

    if (as_constructor) {                       /* new Constructor */
        Jsi_Obj *newobj = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
        Jsi_Value *proto = NULL;
        if (!interp->subOpts.noproto)
            proto = Jsi_ValueObjLookup(interp, tocall, "prototype", 0);
        if (proto && proto->vt == JSI_VT_OBJECT) {
            newobj->__proto__ = proto;
            newobj->clearProto = 1;
            Jsi_IncrRefCount(interp, proto);
        }
        Jsi_ValueReset(interp, &_this);
        Jsi_ValueMakeObject(interp, &_this, newobj);            
        /* TODO: constructor specifics??? */
        calltrc = (tc&jsi_callTraceNew);
    }
    if (funcPtr->type == FC_NORMAL)
        calltrc = (tc&jsi_callTraceFuncs);
    else
        calltrc = (tc&jsi_callTraceCmds);
    if (calltrc && funcPtr->name)
        jsi_TraceFuncCall(interp, funcPtr, ip, _this, args, 0, tc);

    //Jsi_Value *spretPtr = *ret;
 
    interp->activeFunc = funcPtr;
    docall = (rc==JSI_OK);
    if (profile || coverage) {
        interp->profileCnt++;
        timStart = jsi_GetTimestamp();
    }
    if (funcPtr->type == FC_NORMAL) {
        if (docall) {
            rc = jsi_evalcode(interp->ps, funcPtr, funcPtr->opcodes, tocall->d.obj->d.fobj->scope, 
                args, _this, ret);
        }
        interp->funcCallCnt++;
    } else if (!funcPtr->callback) {
        Jsi_LogError("can not call:\"%s()\"", funcPtr->name);
    } else {
        int oldcf = funcPtr->callflags.i;
        funcPtr->callflags.bits.iscons = (as_constructor?JSI_CALL_CONSTRUCTOR:0);
        if (funcPtr->f.bits.hasattr)
        {
#define SPTR(s) (s?s:"")
            if ((funcPtr->f.bits.isobj) && _this->vt != JSI_VT_OBJECT) {
                rc = JSI_ERROR;
                docall = 0;
                Jsi_LogError("'this' is not object: \"%s()\"", funcPtr->name);
            } else if ((!(funcPtr->f.bits.iscons)) && as_constructor) {
                rc = JSI_ERROR;
                docall = 0;
                Jsi_LogError("can not call as constructor: \"%s()\"", funcPtr->name);
            } else {
                int aCnt = Jsi_ValueGetLength(interp, args);
                if (aCnt<(cs->minArgs+adds)) {
                    Jsi_LogError("missing args, expected \"%s(%s)\" ", cs->name, SPTR(cs->argStr));
                    rc = JSI_ERROR;
                    docall = 0;
                } else if (cs->maxArgs>=0 && (aCnt>cs->maxArgs+adds)) {
                    Jsi_LogError("extra args, expected \"%s(%s)\" ", cs->name, SPTR(cs->argStr));
                    rc = JSI_ERROR;
                    docall = 0;
                }
            }
        }
        if (docall) {
            funcPtr->fobj = fobj; // Backlink for bind.
            funcPtr->callflags.bits.isdiscard = discard;
            rc = funcPtr->callback(interp, args, 
                _this, ret, funcPtr);
            interp->cmdCallCnt++;
        }
        funcPtr->callflags.i = oldcf;
    }
    if (profile || coverage) {
        double timEnd = jsi_GetTimestamp(), timUsed = (timEnd - timStart);;
        assert(timUsed>=0);
        funcPtr->allTime += timUsed;
        if (interp->framePtr->evalFuncPtr)
            interp->framePtr->evalFuncPtr->subTime += timUsed;
        else
            interp->subTime += timUsed;
    }
    if (calltrc && (tc&jsi_callTraceReturn) && funcPtr->name)
        jsi_TraceFuncCall(interp, funcPtr, ip, _this, NULL, *ret, tc);
    if (!onam)
        funcPtr->name = NULL;
    if (docall) {
        funcPtr->callCnt++;
        if (rc == JSI_OK && !as_constructor && funcPtr->retType && (interp->typeCheck.all || interp->typeCheck.run))
            rc = jsi_ArgTypeCheck(interp, funcPtr->retType, *ret, "returned from", funcPtr->name, 0, funcPtr, 0);
    }
    interp->prevActiveFunc = pprevActive;
    interp->activeFunc = prevActive;

    if (as_constructor) {
        if (_this->vt == JSI_VT_OBJECT)
            _this->d.obj->constructor = tocall->d.obj;
        if ((*ret)->vt != JSI_VT_OBJECT) {
            Jsi_ValueReset(interp, ret);
            Jsi_ValueCopy(interp, *ret, _this);
        }
    }

bail:
    jsi_SharedArgs(interp, args, funcPtr, 0); /* make arg vars to shared arguments */
    Jsi_DecrRefCount(interp, _this);
    interp->curFunction = oldCurFunc;

    return rc;
}

Jsi_RC jsiEvalFunction(jsi_Pstate *ps, jsi_OpCode *ip, int discard) {
    Jsi_Interp *interp = ps->interp;
    int stackargc = (int)(uintptr_t)ip->data;
    jsiVarDeref(interp, stackargc + 1);
    int tocall_index = interp->framePtr->Sp - stackargc - 1;
    Jsi_Value *_this = _jsi_THISIDX(tocall_index),
        *tocall = _jsi_STACKIDX(tocall_index), **spargs = _jsi_STACK+(interp->framePtr->Sp - stackargc),   
        *spretPtr = Jsi_ValueNew1(interp), *spretPtrOld = spretPtr,
        *args = Jsi_ValueNewArrayObj(interp, spargs, stackargc, 1);
        
    Jsi_IncrRefCount(interp, args);
    if (_this->vt != JSI_VT_OBJECT)
        _this = Jsi_ValueDup(interp, interp->Top_object);
    else {
        _this = Jsi_ValueDup(interp, _this);
        jsiClearThis(interp, tocall_index);
    }   
    Jsi_RC rc = jsi_FunctionSubCall(interp, args, _this, &spretPtr, tocall, discard);
    
    jsiPop(interp, stackargc);
    jsiClearStack(interp,1);
    if (rc == JSI_CONTINUE) {
        Jsi_ValueMakeUndef(interp, &_jsi_TOP);
        rc = JSI_OK;
    }
    if (spretPtr == spretPtrOld) {
        Jsi_ValueMove(interp, _jsi_TOP, spretPtr);
        Jsi_DecrRefCount(interp, spretPtr);
    } else {
        /*  returning a (non-copied) value reference */
        Jsi_DecrRefCount(interp, _jsi_TOP);
        _jsi_TOP = spretPtr;
    }
    Jsi_DecrRefCount(interp, args);
    return rc;
}

static Jsi_RC jsi_PushVar(jsi_Pstate *ps, jsi_OpCode *ip, jsi_ScopeChain *scope, Jsi_Value *currentScope, int context_id) {
    Jsi_Interp *interp = ps->interp;
    jsi_FastVar *fvar = (typeof(fvar))ip->data;
    SIGASSERT(fvar,FASTVAR);
    Jsi_Value **dvPtr = &_jsi_STACKIDX(interp->framePtr->Sp), *dv = *dvPtr, *v = NULL;
    if (fvar->context_id == context_id && fvar->ps == ps) {
        v = fvar->lval;
    } else {
        char *varname = fvar->varname;
        v = Jsi_ValueObjLookup(interp, currentScope, varname, 1);
        if (v) {
            fvar->local = 1;
            if (v->vt == JSI_VT_UNDEF) {
                v->d.lookupFail = varname;
                v->f.bits.lookupfailed = 1;
            }
        } else {
            v = jsi_ScopeChainObjLookupUni(scope, varname);
            if (v) 
                fvar->local = 1;
            else {
                /* add to global scope.  TODO: do not define if a right_val??? */
                Jsi_Value *global_scope = scope->chains_cnt > 0 ? scope->chains[0]:currentScope;
                Jsi_Value key = VALINIT, *kPtr = &key; // Note: a string key so no reset needed.
                Jsi_ValueMakeStringKey(interp, &kPtr, varname);
                v = jsi_ValueObjKeyAssign(interp, global_scope, &key, NULL, JSI_OM_DONTENUM);
                if (v->vt == JSI_VT_UNDEF) {
                    v->d.lookupFail = varname;
                    v->f.bits.lookupfailed = 1;
                }
                jsi_ValueDebugLabel(v, "var", varname);
                bool isNew;
                Jsi_HashEntry *hPtr = Jsi_HashEntryNew(interp->varTbl, varname, &isNew);
                if (hPtr && isNew)
                    Jsi_HashValueSet(hPtr, 0);
            }
        }
        
        Jsi_IncrRefCount(interp, v);

    }
    if (dv != v && (dv->vt != JSI_VT_VARIABLE || dv->d.lval != v)) {
        Jsi_ValueReset(interp, dvPtr);
        dv->vt = JSI_VT_VARIABLE;
        SIGASSERT(v, VALUE);
        dv->d.lval = v;
        dv->f.bits.local = (fvar->local);
    }
    SIGASSERT(v, VALUE);
    jsiPush(interp,1);
    return JSI_OK;
}

static void jsi_PushFunc(jsi_Pstate *ps, jsi_OpCode *ip, jsi_ScopeChain *scope, Jsi_Value *currentScope) {
    /* TODO: now that we're caching ps, may need to reference function ps for context_id??? */
    Jsi_Interp *interp = ps->interp;
    Jsi_FuncObj *fo = jsi_FuncObjNew(interp, (Jsi_Func *)ip->data);
    fo->scope = jsi_ScopeChainDupNext(interp, scope, currentScope);
    Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_FUNCTION);
    obj->d.fobj = fo;
    
    Jsi_Value *v = _jsi_STACKIDX(interp->framePtr->Sp), *fun_prototype = jsi_ObjValueNew(interp);
    fun_prototype->d.obj->__proto__ = interp->Object_prototype;                
    Jsi_ValueMakeObject(interp, &v, obj);
    Jsi_ValueInsert(interp, v, "prototype", fun_prototype, JSI_OM_DONTDEL|JSI_OM_DONTENUM);
    /* TODO: make own prototype and prototype.constructor */
    
    bool isNew;
    Jsi_HashEntry *hPtr;  Jsi_Value *vv;
    if (interp->framePtr->Sp == 1 && (vv=_jsi_STACKIDX(0))->vt == JSI_VT_VARIABLE) {
        const char *varname = NULL;
        vv = vv->d.lval;
        if (vv && vv->f.bits.lookupfailed && vv->d.lookupFail) {
            varname = vv->d.lookupFail;
            vv->f.bits.lookupfailed = 0;
        }
        if (varname) {
            if (!fo->func->name)
                fo->func->name = varname;
            hPtr = Jsi_HashEntryNew(interp->varTbl, varname, &isNew);
            if (hPtr)
                Jsi_HashValueSet(hPtr, obj);
        }
    }
    hPtr = Jsi_HashEntryNew(interp->funcObjTbl, fo, &isNew);
    if (hPtr && isNew) {
        Jsi_ObjIncrRefCount(interp, obj);
        Jsi_HashValueSet(hPtr, obj);
    }
    jsiPush(interp,1);
}

static Jsi_RC jsi_evalSubscript(Jsi_Interp *interp, Jsi_Value *src, Jsi_Value *idx, jsi_OpCode *ip,  jsi_OpCode *end,
    Jsi_Value *currentScope)
{
    Jsi_RC rc = JSI_OK;
    jsiVarDeref(interp,2);
    int isnull;
    if ((isnull=Jsi_ValueIsNull(interp, src)) || Jsi_ValueIsUndef(interp, src)) {
        Jsi_LogError("invalid subscript of %s", (isnull?"null":"undefined"));
        jsiPop(interp, 1);
        return JSI_ERROR;
    }
    Jsi_String *str = jsi_ValueString(src);
    if (str && Jsi_ValueIsNumber(interp, idx)) {
        int bLen, cLen;
        char bbuf[10], *cp = Jsi_ValueString(interp, src, &bLen);
        int n = (int)idx->d.num;
        cLen = bLen;
#if JSI__UTF8
        if (str->flags&JSI_IS_UTF || !(str->flags&JSI_UTF_CHECKED)) {
            cLen = Jsi_NumUtfChars(cp, bLen);
            str->flags |= JSI_UTF_CHECKED;
            if (cLen != bLen)
                str->flags |= JSI_IS_UTF;
        }
#endif
        if (n<0 || n>=cLen) {
            Jsi_ValueMakeUndef(interp, &src);
        } else {
            if (cLen != bLen)
                Jsi_UtfGetIndex(cp, n, bbuf);
            else {
                bbuf[0] = cp[n];
                bbuf[1] = 0;
            }
            Jsi_ValueMakeStringDup(interp, &src, bbuf);
        }
        jsiPop(interp, 1);
        return rc;
    }
    Jsi_ValueToObject(interp, src);
    if (interp->hasCallee && (src->d.obj == currentScope->d.obj || (interp->framePtr->arguments && src->d.obj == interp->framePtr->arguments->d.obj))) {
        if (idx->vt == JSI_VT_STRING && Jsi_Strcmp(idx->d.s.str, "callee") == 0) {
            jsiClearStack(interp,1);
            Jsi_ValueMakeStringKey(interp, &idx, "\1callee\1");
        }
    }
    int bsc = Jsi_ValueIsObjType(interp, src, JSI_OT_NUMBER); // Previous bad subscript.
    if (bsc == 0 && interp->lastSubscriptFail && interp->lastSubscriptFail->vt != JSI_VT_UNDEF)
        Jsi_ValueReset(interp, &interp->lastSubscriptFail);

    if (src->vt != JSI_VT_UNDEF) {
        int right_val = (uintptr_t)ip->data;
        Jsi_Value res = VALINIT, 
            *resPtr = &res,
            *vp = jsi_ValueSubscript(interp, src, idx, &resPtr);
        if (!vp && bsc == 0) {
            /* eg. so we can list available commands for  "db.xx()" */
            if (idx->vt == JSI_VT_STRING)
                interp->lastSubscriptFailStr = idx->d.s.str;
            Jsi_ValueDup2(interp, &interp->lastSubscriptFail, src);
        }
        if (vp)
            Jsi_IncrRefCount(interp, vp);
        jsiClearStack(interp,2);
        if (!vp)
            Jsi_ValueMakeUndef(interp, &src);
        else {
            //printf("IDX(%p): %s\n", idx, Jsi_ValueString(interp, idx, NULL));
            if (right_val || vp->f.bits.readonly) {
                if (vp == resPtr && (res.vt == JSI_VT_OBJECT || res.vt == JSI_VT_STRING))  // TODO:*** Undo using ValueCopy. ***
                    Jsi_ValueMove(interp, src, vp);
                else
                    Jsi_ValueCopy(interp, src, vp);
            } else {
                assert(vp != resPtr);
                res.vt = JSI_VT_VARIABLE;
                res.d.lval = vp;
                Jsi_ValueCopy(interp, src, resPtr);
            }
            Jsi_DecrRefCount(interp, vp);
        }
    }
    jsiPop(interp, 1);
    return rc;
}

void jsi_DebuggerStmt(void) {
    // Called for "debugger" statement.
}

static Jsi_RC jsi_ValueAssignCheck(Jsi_Interp *interp, Jsi_Value *val, int lop) {
    if (lop == OP_FCALL || lop == OP_NEWFCALL)
        return JSI_OK;
    if (val->f.bits.lookupfailed && val->d.lookupFail)
        return Jsi_LogType("assign from undef var: %s", val->d.lookupFail);
    return JSI_OK;
}

Jsi_RC jsi_evalcode_sub(jsi_Pstate *ps, Jsi_OpCodes *opcodes, 
     jsi_ScopeChain *scope, Jsi_Value *currentScope,
     Jsi_Value *_this, Jsi_Value *vret)
{
    Jsi_Interp* interp = ps->interp;
    jsi_OpCode *ip = &opcodes->codes[0];
    Jsi_RC rc = JSI_OK;
    int curLine = 0;
    int context_id = ps->_context_id++, lop = -1;
    jsi_OpCode *end = &opcodes->codes[opcodes->code_len];
    jsi_TryList  *trylist = NULL;
    bool strict = interp->strict;
    const char *curFile = NULL;
    
    if (currentScope->vt != JSI_VT_OBJECT) {
        Jsi_LogBug("Eval: current scope is not a object");
        return JSI_ERROR;
    }
    if (interp->maxDepth>0 && interp->level > interp->maxDepth)
        rc = Jsi_LogError("Exceeded call depth: %d", interp->level);
    
    while(ip < end && rc == JSI_OK) {
        int plop = ip->op;

        if (ip->logflag) { // Mask out LogDebug, etc if not enabled.
            interp->curIp = ip;
            switch (ip->logflag) {
                case jsi_Oplf_assert:
                    if (!interp->asserts) {
                        ip++;
                        if (ip->logflag != jsi_Oplf_assert && (ip->op == OP_POP || ip->op == OP_RET))
                            ip++;
                        continue;
                    }
                    break;
                case jsi_Oplf_debug:
                    if (!interp->logOpts.Debug && !(interp->framePtr->logflag &(1<<jsi_Oplf_debug))) {
                        ip++;
                        if (ip->logflag != jsi_Oplf_debug && (ip->op == OP_POP || ip->op == OP_RET))
                            ip++;
                        continue;
                    }
                    break;
                case jsi_Oplf_test:
                    if (!interp->logOpts.Test && !(interp->framePtr->logflag &(1<<jsi_Oplf_test))) {
                        ip++;
                        if (ip->logflag != jsi_Oplf_test && (ip->op == OP_POP || ip->op == OP_RET))
                            ip++;
                        continue;
                    }
                    break;
                case jsi_Oplf_trace:
                    if (!interp->logOpts.Trace && !(interp->framePtr->logflag &(1<<jsi_Oplf_trace))) {
                        ip++;
                        if (ip->logflag != jsi_Oplf_trace && (ip->op == OP_POP || ip->op == OP_RET))
                            ip++;
                        continue;
                    }
                    break;
                default:
                    break;
            }
        }
        if (interp->interrupted) {
            if (!interp->framePtr->tryDepth) {
                Jsi_LogError("program interrupted: function=%s", interp->framePtr->funcName);
                interp->interrupted = 0;
            } else {
                interp->interrupted++;
            }
            rc = JSI_ERROR;
            break;
        }
        if (interp->exited) {
            rc = JSI_ERROR;
            break;
        }
        interp->opCnt++;
        if (interp->maxOpCnt && interp->opCnt > interp->maxOpCnt) {
            puts("EXEC CAP EXCEED");
            interp->maxOpCnt += 1000;
            rc = Jsi_LogError("Exceeded execution cap: %d", interp->opCnt);
            interp->exited = 1;
            interp->exitCode = 99;
            break;
        }
        if (interp->traceOp) {
            jsiDumpInstr(interp, ps, _this, trylist, ip, opcodes);
        }
        if (interp->parent && interp->busyCallback && (interp->opCnt%(interp->busyInterval<=0?100000:interp->busyInterval))==0) {
            // Handle parent interp events.
            if (jsi_FuncIsNoop(interp, interp->busyCallback))
                Jsi_EventProcess(interp->parent, -1);
            else {
                Jsi_DString nStr;
                Jsi_DSInit(&nStr);
                Jsi_DSPrintf(&nStr, "[\"#Interp_%d\", %d]", interp->objId, interp->opCnt);//TODO: use actual time interval rather than opCnt.
                if (Jsi_FunctionInvokeJSON(interp->parent, interp->busyCallback, Jsi_DSValue(&nStr), NULL) != JSI_OK)
                    rc = JSI_ERROR;
                Jsi_DSFree(&nStr);
            }
        }
        ip->hit=1;
#ifndef USE_STATIC_STACK
        if ((interp->maxStack-interp->framePtr->Sp)<STACK_MIN_PAD)
            jsiSetupStack(interp);
#endif
        jsiPush(interp,0);
        interp->curIp = ip;
        // Carry forward line/file info from previous OPs.
        if (!ip->Line)
            ip->Line = curLine;
        else
            curLine = ip->Line;
        if (!ip->fname)
            ip->fname = curFile;
        else
            curFile = ip->fname;
        if (interp->debugOpts.hook) {
            interp->framePtr->fileName = curFile;
            interp->framePtr->line = curLine;
            if ((rc = (*interp->debugOpts.hook)(interp, curFile, curLine, interp->framePtr->level, interp->curFunction, jsi_opcode_string(ip->op), ip, NULL)) != JSI_OK)
                break;
        }

        switch(ip->op) {
            case OP_NOP:
            case OP_LASTOP:
                break;
            case OP_PUSHUND:
                Jsi_ValueMakeUndef(interp, &_jsi_STACKIDX(interp->framePtr->Sp));
                jsiPush(interp,1);
                break;
            case OP_PUSHNULL:
                Jsi_ValueMakeNull(interp, &_jsi_STACKIDX(interp->framePtr->Sp));
                jsiPush(interp,1);
                break;
            case OP_PUSHBOO:
                Jsi_ValueMakeBool(interp, &_jsi_STACKIDX(interp->framePtr->Sp), (uintptr_t)ip->data);
                jsiPush(interp,1);
                break;
            case OP_PUSHNUM:
                Jsi_ValueMakeNumber(interp, &_jsi_STACKIDX(interp->framePtr->Sp), (*((Jsi_Number *)ip->data)));
                jsiPush(interp,1);
                break;
            case OP_PUSHSTR: {
                Jsi_Value **v = &_jsi_STACKIDX(interp->framePtr->Sp);
                Jsi_ValueMakeStringKey(interp, v, (char*)ip->data);
                interp->lastPushStr = Jsi_ValueString(interp, *v, NULL);
                jsiPush(interp,1);
                break;
            }
            case OP_PUSHVSTR: {
                Jsi_String *s = (Jsi_String *)ip->data;
                Jsi_Value **v = &_jsi_STACKIDX(interp->framePtr->Sp);
                if (s->flags&1)
                    jsi_ValueMakeBlobDup(interp,v, (uchar*)s->str, s->len);
                else {
                    Jsi_ValueMakeStringKey(interp, v, s->str);
                    interp->lastPushStr = s->str;
                }
                jsiPush(interp,1);
                break;
            }
            case OP_PUSHVAR: {
                rc = jsi_PushVar(ps, ip, scope, currentScope, context_id);      
                break;
            }
            case OP_PUSHFUN: {
                jsi_PushFunc(ps, ip, scope, currentScope);
                break;
            }
            case OP_NEWFCALL:
                if (interp->maxUserObjs && interp->userObjCnt > interp->maxUserObjs) {
                    rc = Jsi_LogError("Max 'new' count exceeded");
                    break;
                }
            case OP_FCALL: {
                /* TODO: need reliable way to capture func string name to handle unknown functions.*/
                int discard = ((ip+1)<end && ip[1].op == OP_POP);
                switch (jsiEvalFunction(ps, ip, discard)) {        /* throw an execption */
                    case JSI_OK: break;
                    case JSI_BREAK:
                        if (interp->framePtr->tryDepth<=0)
                            interp->isHelp = 1;
                        JSI_DO_THROW("help");
                        break;
                    default:  
                        JSI_DO_THROW("fcall");
                }
                strict = interp->strict;
                /* TODO: new Function return a function without scopechain, add here */
                break;
            }
            case OP_SUBSCRIPT: {
                rc = jsi_evalSubscript(interp, _jsi_TOQ, _jsi_TOP, ip, end, currentScope);
                break;
            }
            case OP_ASSIGN: {
                Jsi_Value *sval = _jsi_TOP, *dval = _jsi_TOQ;
                bool globThis = (sval->vt == JSI_VT_OBJECT && sval->d.obj == interp->csc->d.obj);
                if ((uintptr_t)ip->data == 1) {
                    jsiVarDeref(interp,1);
                    rc = jsi_ValueAssign(interp, dval, sval, lop);                    
                    if (strict && sval->vt == JSI_VT_UNDEF)
                        rc = jsi_ValueAssignCheck(interp, sval, lop);
                    jsiPop(interp,1);
                } else {
                    jsiVarDeref(interp, 3);
                    Jsi_Value *v3 = _jsi_STACKIDX(interp->framePtr->Sp-3);
                    if (v3->vt == JSI_VT_OBJECT) {
                        if (strict && sval->vt == JSI_VT_UNDEF)
                            rc = jsi_ValueAssignCheck(interp, sval, lop);
                        jsi_ValueObjKeyAssign(interp, v3, dval, sval, 0);
                        jsi_ValueDebugLabel(sval, "assign", NULL);
                    } else if (strict)
                        rc = Jsi_LogError("assign to a non-exist object");
                    jsiClearStack(interp,3);
                    Jsi_ValueCopy(interp,v3, sval);
                    dval = v3;
                    jsiPop(interp, 2);
                }
                if (interp->framePtr->level<=1 && globThis && rc == JSI_OK && dval && dval->vt == JSI_VT_VARIABLE) {
                    dval = dval->d.lval;
                    //printf("GLOBAL THIS: %p\n", dval);
                    Jsi_HashSet(interp->genValueTbl, dval, dval);
                }
                break;
            }
            case OP_PUSHREG: {
                Jsi_Obj *obj = Jsi_ObjNewType(interp, JSI_OT_REGEXP);
                obj->d.robj = (Jsi_Regex *)ip->data;
                Jsi_ValueMakeObject(interp, &_jsi_STACKIDX(interp->framePtr->Sp), obj);
                jsiPush(interp,1);
                break;
            }
            case OP_PUSHARG:
                //Jsi_ValueCopy(interp,_jsi_STACKIDX(interp->framePtr->Sp), currentScope);
                
                if (!interp->framePtr->arguments) {
                    interp->framePtr->arguments = Jsi_ValueNewObj(interp,
                        Jsi_ObjNewArray(interp, currentScope->d.obj->arr, currentScope->d.obj->arrCnt, 0));
                    Jsi_IncrRefCount(interp, interp->framePtr->arguments);
                    if (interp->hasCallee) {
                        Jsi_Value *callee = Jsi_ValueObjLookup(interp, currentScope, "\1callee\1", 0);
                        if (callee)
                            Jsi_ValueInsert(interp, interp->framePtr->arguments, "\1callee\1", callee, JSI_OM_DONTENUM);
                    }
                    // interp->framePtr->arguments->d.obj->__proto__ = interp->Object_prototype; // ecma
                }
                Jsi_ValueCopy(interp,_jsi_STACKIDX(interp->framePtr->Sp), interp->framePtr->arguments);
                jsiPush(interp,1);
                break;
            case OP_PUSHTHS: { //TODO: Value copy can cause memory leak!
                Jsi_Value *tval = _jsi_STACKIDX(interp->framePtr->Sp);
                Jsi_ValueCopy(interp, tval, _this);
                /*if (interp->csc == _this)
                    Jsi_ValueDup2(interp, &tval, _this);
                else
                    Jsi_ValueCopy(interp, tval, _this);*/
                jsiPush(interp,1);
                break;
            }
            case OP_PUSHTOP:
                Jsi_ValueCopy(interp,_jsi_STACKIDX(interp->framePtr->Sp), _jsi_TOP);
                jsiPush(interp,1);
                break;
            case OP_UNREF:
                jsiVarDeref(interp,1);
                break;
            case OP_PUSHTOP2: {
                Jsi_Value *vp1 = _jsi_STACKIDX(interp->framePtr->Sp);
                Jsi_Value *vp2 = _jsi_STACKIDX(interp->framePtr->Sp+1);
                if (!vp1 || !vp2)
                    rc = Jsi_LogError("Invalid lookup/push");
                else {
                    Jsi_ValueCopy(interp, vp1, _jsi_TOQ);
                    Jsi_ValueCopy(interp, vp2, _jsi_TOP);
                    jsiPush(interp, 2);
                }
                break;
            }
            case OP_CHTHIS: {
                if (ip->data) {
                    int t = interp->framePtr->Sp - 2;
                    Assert(t>=0);
                    Jsi_Value *v = _jsi_THISIDX(t);
                    jsiClearThis(interp, t);
                    Jsi_ValueCopy(interp, v, _jsi_TOQ);
                    if (v->vt == JSI_VT_VARIABLE) {
                        Jsi_ValueCopy(interp, v, v->d.lval);
                    }
                    Jsi_ValueToObject(interp, v);
                }
                break;
            }
            case OP_LOCAL: {
                Jsi_Value key = VALINIT, *kPtr = &key; // Note we use a string key so no reset needed.
                Jsi_ValueMakeStringKey(interp, &kPtr, (char*)ip->data);
                jsi_ValueObjKeyAssign(interp, currentScope, kPtr, NULL, JSI_OM_DONTENUM);
                context_id = ps->_context_id++;
                break;
            }
            case OP_POP: {
                Jsi_Value *tval = _jsi_TOP;
                if ((interp->evalFlags&JSI_EVAL_RETURN) && (ip+1) >= end && 
                (Jsi_ValueIsObjType(interp, tval, JSI_OT_ITER)==0 &&
                Jsi_ValueIsObjType(interp, tval, JSI_OT_FUNCTION)==0)) {
                    /* Interactive and last instruction is a pop: save result. */
                    Jsi_ValueMove(interp, vret, tval); /*TODO***: correct ***/
                    tval->vt = JSI_VT_UNDEF;
                }
                jsiPop(interp, (uintptr_t)ip->data);
                break;
            }
            case OP_NEG:
                jsiVarDeref(interp,1);
                Jsi_ValueToNumber(interp, _jsi_TOP);
                rc = _jsi_StrictChk(_jsi_TOP);
                _jsi_TOP->d.num = -(_jsi_TOP->d.num);
                break;
            case OP_POS:
                jsiVarDeref(interp,1);
                Jsi_ValueToNumber(interp, _jsi_TOP);
                rc = _jsi_StrictChk(_jsi_TOP);
                break;
            case OP_NOT: {
                int val = 0;
                jsiVarDeref(interp,1);
                
                val = Jsi_ValueIsTrue(interp, _jsi_TOP);
                
                jsiClearStack(interp,1);
                Jsi_ValueMakeBool(interp, &_jsi_TOP, !val);
                break;
            }
            case OP_BNOT: {
                jsiVarDeref(interp,1);
                jsi_ValueToOInt32(interp, _jsi_TOP);
                rc = _jsi_StrictChk(_jsi_TOP);
                _jsi_TOP->d.num = (Jsi_Number)(~((int)_jsi_TOP->d.num));
                break;
            }
            case OP_ADD: {
                jsiVarDeref(interp,2);
                Jsi_Value *v, *v1 = _jsi_TOP, *v2 = _jsi_TOQ;
                int l1, l2;
                if (strict)
                    if (Jsi_ValueIsUndef(interp, v1) || Jsi_ValueIsUndef(interp, v2)) {
                        rc = Jsi_LogError("operand value to + is undefined");
                        break;
                    }
                char *s1 = Jsi_ValueString(interp, v1, &l1);
                char *s2 = Jsi_ValueString(interp, v2, &l2);
                if (s1 || s2) {
                    char *str;
                    if (!(s1 && s2)) {
                        v = (s1 ? v2 : v1);
                        jsi_ValueToPrimitive(interp, &v);
                        Jsi_ValueToString(interp, v, NULL);
                        str = Jsi_ValueString(interp, v, (s1?&l2:&l1));
                        if (s1) s2 = str; else s1 = str;
                    }
                    Assert(l1>=0 && l1<=JSI_MAX_ALLOC_BUF);
                    Assert(l2>=0 && l2<=JSI_MAX_ALLOC_BUF);
                    str = (char*)Jsi_Malloc(l1+l2+1);
                    memcpy(str, s2, l2);
                    memcpy(str+l2, s1, l1);
                    str[l1+l2] = 0;
                    jsiClearStack(interp,2);
                    Jsi_ValueMakeBlob(interp, &v2, (uchar*)str, l1+l2);
                } else {
                    Jsi_ValueToNumber(interp, v1);
                    Jsi_ValueToNumber(interp, v2);
                    rc = _jsi_StrictChk2(v1, v2);
                    Jsi_Number n = v1->d.num + v2->d.num;
                    jsiClearStack(interp,2);
                    Jsi_ValueMakeNumber(interp, &v2, n);
                }
                jsiPop(interp,1);
                break;
            }
            case OP_IN: {
                Jsi_Value *v, *vl;
                const char *cp = NULL;
                Jsi_Number nval;
                jsiVarDeref(interp,2);
                vl = _jsi_TOQ;
                v = _jsi_TOP;
                if (Jsi_ValueIsString(interp,vl))
                    cp = Jsi_ValueGetStringLen(interp, vl, NULL);
                else if (Jsi_ValueIsNumber(interp,vl))
                    Jsi_ValueGetNumber(interp, vl, &nval);
                else {
                    if (strict)
                        Jsi_LogWarn("expected string or number before IN");
                    Jsi_ValueMakeBool(interp, &_jsi_TOQ, 0);
                    jsiPop(interp,1);
                    break;
                }
                
                if (v->vt == JSI_VT_VARIABLE) {
                    v = v->d.lval;
                    SIGASSERT(v, VALUE);
                }
                if (v->vt != JSI_VT_OBJECT || v->d.obj->ot != JSI_OT_OBJECT) {
                    if (strict)
                        Jsi_LogWarn("expected object after IN");
                    Jsi_ValueMakeBool(interp, &_jsi_TOQ, 0);
                    jsiPop(interp,1);
                    break;
                }
                int bval = 0;
                char nbuf[JSI_MAX_NUMBER_STRING];
                Jsi_Value *vv;
                Jsi_Obj *obj = v->d.obj;
                if (!cp) {
                    snprintf(nbuf, sizeof(nbuf), "%d", (int)nval);
                    cp = nbuf;
                }
                if (obj->arr) {
                    vv = jsi_ObjArrayLookup(interp, obj, (char*)cp);
                } else {
                    vv = Jsi_TreeObjGetValue(obj, (char*)cp, 1);
                }
                bval = (vv != 0);
                Jsi_ValueMakeBool(interp, &_jsi_TOQ, bval);
                jsiPop(interp,1);
                break;
            }
            case OP_SUB: 
                common_math_opr(-); break;
            case OP_MUL:
                common_math_opr(*); break;
            case OP_DIV:
                common_math_opr(/); break;
            case OP_MOD: {
                jsiVarDeref(interp,2);
                if (!Jsi_ValueIsType(interp,_jsi_TOP, JSI_VT_NUMBER))
                    Jsi_ValueToNumber(interp, _jsi_TOP);
                if (!Jsi_ValueIsType(interp,_jsi_TOQ, JSI_VT_NUMBER))
                    Jsi_ValueToNumber(interp, _jsi_TOQ);
                rc = _jsi_StrictChk2(_jsi_TOP,_jsi_TOQ);
                if (rc == JSI_OK)
                    _jsi_TOQ->d.num = fmod(_jsi_TOQ->d.num, _jsi_TOP->d.num);
                jsiPop(interp,1);
                break;
            }
            case OP_LESS:
                jsiVarDeref(interp,2);
                rc = jsi_logic_less(interp,2,1);
                jsiPop(interp,1);
                break;
            case OP_GREATER:
                jsiVarDeref(interp,2);
                rc = jsi_logic_less(interp,1,2);
                jsiPop(interp,1);
                break;
            case OP_LESSEQU:
                jsiVarDeref(interp,2);
                rc = jsi_logic_less(interp,1,2);
                if (rc == JSI_OK)
                    _jsi_TOQ->d.val = !_jsi_TOQ->d.val;
                jsiPop(interp,1);
                break;
            case OP_GREATEREQU:
                jsiVarDeref(interp,2);
                rc = jsi_logic_less(interp,2,1);
                if (rc == JSI_OK)
                    _jsi_TOQ->d.val = !_jsi_TOQ->d.val;
                jsiPop(interp,1);
                break;
            case OP_EQUAL:
            case OP_NOTEQUAL: {
                jsiVarDeref(interp,2);
                int r = Jsi_ValueCmp(interp, _jsi_TOP, _jsi_TOQ, 0);
                r = (ip->op == OP_EQUAL ? !r : r);
                jsiClearStack(interp,2);
                Jsi_ValueMakeBool(interp, &_jsi_TOQ, r);
                jsiPop(interp,1);
                break;
            }
            case OP_STRICTEQU:
            case OP_STRICTNEQ: {
                int r = 0;
                jsiVarDeref(interp,2);
                rc = _jsi_StrictUChk3(_jsi_TOQ, _jsi_TOP);
                r = !Jsi_ValueIsEqual(interp, _jsi_TOP, _jsi_TOQ);
                r = (ip->op == OP_STRICTEQU ? !r : r);
                jsiClearStack(interp,2);
                Jsi_ValueMakeBool(interp, &_jsi_TOQ, r);
                jsiPop(interp,1);
                break;
            }
            case OP_BAND: 
                common_bitwise_opr(&); break;
            case OP_BOR:
                common_bitwise_opr(|); break;
            case OP_BXOR:
                common_bitwise_opr(^); break;
            case OP_SHF: {
                jsiVarDeref(interp,2);
                jsi_ValueToOInt32(interp, _jsi_TOQ);
                jsi_ValueToOInt32(interp, _jsi_TOP);
                int t1 = (int)_jsi_TOQ->d.num;
                int t2 = ((unsigned int)_jsi_TOP->d.num) & 0x1f;
                if (ip->data) {                 /* shift right */
                    if ((uintptr_t)ip->data == 2) {   /* unsigned shift */
                        unsigned int t3 = (unsigned int)t1;
                        t3 >>= t2;
                        Jsi_ValueMakeNumber(interp, &_jsi_TOQ, t3);
                    } else {
                        t1 >>= t2;
                        Jsi_ValueMakeNumber(interp, &_jsi_TOQ, t1);
                    }
                } else {
                    t1 <<= t2;
                    Jsi_ValueMakeNumber(interp, &_jsi_TOQ, t1);
                }
                jsiPop(interp,1);
                break;
            }
            case OP_KEY: {
                jsiVarDeref(interp,1);
                if (ip->isof && !Jsi_ValueIsArray(interp, _jsi_TOP)) {
                    rc = Jsi_LogError("operand not an array");
                    break;
                }
                if (_jsi_TOP->vt != JSI_VT_UNDEF && _jsi_TOP->vt != JSI_VT_NULL)
                    Jsi_ValueToObject(interp, _jsi_TOP);
                Jsi_Value *spret = Jsi_ValueNew1(interp);
                jsi_ValueObjGetKeys(interp, _jsi_TOP, spret, ip->isof);
                Jsi_ValueReplace(interp, _jsi_STACK+interp->framePtr->Sp, spret);  
                Jsi_DecrRefCount(interp, spret);  
                jsiPush(interp,1);
                break;
            }
            case OP_NEXT: {
                Jsi_Value *toq = _jsi_TOQ, *top = _jsi_TOP;
                if (toq->vt != JSI_VT_OBJECT || toq->d.obj->ot != JSI_OT_ITER) Jsi_LogBug("next: toq not a iter\n");
                if (top->vt != JSI_VT_VARIABLE) {
                    rc = Jsi_LogError ("invalid for/in left hand-side");
                    break;
                }
                if (strict && top->f.bits.local==0) {
                    const char *varname = "";
                    Jsi_Value *v = top->d.lval;
                    if (v->f.bits.lookupfailed)
                        varname = v->d.lookupFail;

                    rc = Jsi_LogError("function created global: \"%s\"", varname);
                    break;
                }
                
                Jsi_IterObj *io = toq->d.obj->d.iobj;
                if (io->iterCmd) {
                    io->iterCmd(io, top, _jsi_STACKIDX(interp->framePtr->Sp-3), io->iter++);
                } else {
                    while (io->iter < io->count) {
                        if (!io->isArrayList) {
                            if (Jsi_ValueKeyPresent(interp, _jsi_STACKIDX(interp->framePtr->Sp-3), io->keys[io->iter],1)) 
                                break;
                        } else {
                            while (io->cur < io->obj->arrCnt) {
                                if (io->obj->arr[io->cur]) break;
                                io->cur++;
                            }
                            if (io->cur >= io->obj->arrCnt) {
                                /* TODO: Is this really a bug??? */
                                /* Jsi_LogBug("NOT FOUND LIST ARRAY");*/
                                io->iter = io->count;
                                break;
                            } else if (io->obj->arr[io->cur]) {
                                io->cur++;
                                break;
                            }
                        }
                        io->iter++;
                    }
                    if (io->iter >= io->count) {
                        jsiClearStack(interp,1);
                        Jsi_ValueMakeNumber(interp, &_jsi_TOP, 0);
                    } else {
                        Jsi_Value **vPtr = &_jsi_TOP->d.lval, *v = *vPtr;
                        SIGASSERT(v, VALUE);
                        Jsi_ValueReset(interp, vPtr);
                        if (io->isArrayList) {
                            if (!io->isof)
                                Jsi_ValueMakeNumber(interp, &v, io->cur-1);
                            else if (!io->obj->arr[io->cur-1])
                                Jsi_ValueMakeNull(interp, &v);
                            else
                                Jsi_ValueCopy(interp, v, io->obj->arr[io->cur-1]);
                        } else
                            Jsi_ValueMakeStringKey(interp, &v, io->keys[io->iter]);
                        io->iter++;
                        
                        jsiClearStack(interp,1);
                        Jsi_ValueMakeNumber(interp, &_jsi_TOP, 1);
                    }
                    break;
                }
            }
            case OP_INC:
            case OP_DEC: {
                int inc = ip->op == OP_INC ? 1 : -1;
                
                if (_jsi_TOP->vt != JSI_VT_VARIABLE) {
                    rc = Jsi_LogError("operand not left value");
                    break;
                }
                Jsi_Value *v = _jsi_TOP->d.lval;
                SIGASSERT(v, VALUE);
                Jsi_ValueToNumber(interp, v);
                rc = _jsi_StrictChk(v);
                v->d.num += inc;
                    
                jsiVarDeref(interp,1);
                if (ip->data) {
                    _jsi_TOP->d.num -= inc;
                }
                break;
            }
            case OP_TYPEOF: {
                const char *typ;
                Jsi_Value *v = _jsi_TOP;
                if (v->vt == JSI_VT_VARIABLE) {
                    v = v->d.lval;
                    SIGASSERT(v, VALUE);
                }
                typ = Jsi_ValueTypeStr(interp, v);
                jsiVarDeref(interp,1);
                Jsi_ValueMakeStringKey(interp, &_jsi_TOP, (char*)typ);
                break;
            }
            case OP_INSTANCEOF: {

                jsiVarDeref(interp,2);
                int bval = Jsi_ValueInstanceOf(interp, _jsi_TOQ, _jsi_TOP);
                jsiPop(interp,1);
                Jsi_ValueMakeBool(interp, &_jsi_TOP, bval);
                break;
            }
            case OP_JTRUE:
            case OP_JFALSE: 
            case OP_JTRUE_NP:
            case OP_JFALSE_NP: {
                jsiVarDeref(interp,1);
                int off = (uintptr_t)ip->data - 1; 
                int r = Jsi_ValueIsTrue(interp, _jsi_TOP);
                
                if (ip->op == OP_JTRUE || ip->op == OP_JFALSE) jsiPop(interp,1);
                ip += ((ip->op == OP_JTRUE || ip->op == OP_JTRUE_NP) ^ r) ? 0 : off;
                break;
            }
            case OP_JMPPOP: 
                jsiPop(interp, ((jsi_JmpPopInfo *)ip->data)->topop);
            case OP_JMP: {
                int off = (ip->op == OP_JMP ? (uintptr_t)ip->data - 1
                            : (uintptr_t)((jsi_JmpPopInfo *)ip->data)->off - 1);

                while (1) {
                    if (trylist == NULL) break;
                    jsi_OpCode *tojmp = ip + off;

                    /* jmp out of a try block, should execute the finally block */
                    /* while jmp out a 'with' block, restore the scope */

                    if (trylist->type == jsi_TL_TRY) { 
                        if (tojmp >= trylist->d.td.tstart && tojmp < trylist->d.td.fend) break;
                        
                        if (ip >= trylist->d.td.tstart && ip < trylist->d.td.cend) {
                            trylist->d.td.last_op = jsi_LOP_JMP;
                            trylist->d.td.ld.tojmp = tojmp;
                            
                            ip = trylist->d.td.fstart - 1;
                            off = 0;
                            break;
                        } else if (ip >= trylist->d.td.fstart && ip < trylist->d.td.fend) {
                            pop_try(trylist);
                        } else Jsi_LogBug("jmp within a try, but not in its scope?");
                    } else {
                        /* with block */
                        
                        if (tojmp >= trylist->d.wd.wstart && tojmp < trylist->d.wd.wend) break;
                        
                        JSI_RESTORE_SCOPE();
                        pop_try(trylist);
                    }
                }
                
                ip += off;
                break;
            }
            case OP_EVAL: {
                int stackargc = (uintptr_t)ip->data;
                jsiVarDeref(interp, stackargc);

                int r = 0;
                Jsi_Value *spPtr = Jsi_ValueNew1(interp);
                if (stackargc > 0) {
                    if (_jsi_STACKIDX(interp->framePtr->Sp - stackargc)->vt == JSI_VT_UNDEF) {
                        Jsi_LogError("undefined value to eval()");
                        goto undef_eval;
                    }
                    int plen;
                    char *pro = Jsi_ValueString(interp, _jsi_STACKIDX(interp->framePtr->Sp - stackargc), &plen);
                    if (pro) {
                        pro = Jsi_StrdupLen(pro, plen);
                        r = jsiEvalOp(interp, ps, pro, scope, currentScope, _this, &spPtr);
                        Jsi_Free(pro);
                    } else {
                        Jsi_ValueCopy(interp, spPtr, _jsi_STACKIDX(interp->framePtr->Sp - stackargc));
                    }
                }
undef_eval:
                jsiPop(interp, stackargc);
                Jsi_ValueCopy(interp, _jsi_STACK[interp->framePtr->Sp], spPtr); /*TODO: is this correct?*/
                Jsi_DecrRefCount(interp, spPtr);
                jsiPush(interp,1);

                if (r) {
                    JSI_DO_THROW("eval");
                }
                break;
            }
            case OP_RET: {
                if (interp->framePtr->Sp>=1 && ip->data) {
                    jsiVarDeref(interp,1);
                    Jsi_ValueMove(interp, vret, _jsi_TOP);
                }
                jsiPop(interp, (uintptr_t)ip->data);
                interp->didReturn = 1;
                if (trylist) {
                    while (trylist) {
                        if (trylist->type == jsi_TL_TRY && trylist->inCatch)
                            JSI_RESTORE_SCOPE();
                        pop_try(trylist);
                    }
                    goto done;
                }
                ip = end;
                break;
            }
            case OP_DELETE: {
                int count = (uintptr_t)ip->data;
                if (count == 1) { // Non-standard.
                    if (_jsi_TOP->vt != JSI_VT_VARIABLE)
                        rc = Jsi_LogError("delete a right value");
                    else {
                        Jsi_Value **vPtr = &_jsi_TOP->d.lval, *v = *vPtr;
                        SIGASSERT(v, VALUE);
                        if (v->f.bits.dontdel) {
                            if (strict) rc = Jsi_LogWarn("delete not allowed");
                        } else if (v != currentScope) {
                            Jsi_ValueReset(interp,vPtr);     /* not allow to delete arguments */
                        }
                        else if (strict)
                            Jsi_LogWarn("Delete arguments");
                    }
                    jsiPop(interp,1);
                } else if (count == 2) {
                    jsiVarDeref(interp,2);
                    assert(interp->framePtr->Sp>=2);
                    if (strict) {
                        if (_jsi_TOQ->vt != JSI_VT_OBJECT) Jsi_LogWarn("delete non-object key, ignore");
                        if (_jsi_TOQ->d.obj == currentScope->d.obj) Jsi_LogWarn("Delete arguments");
                    }
                    jsi_ValueObjDelete(interp, _jsi_TOQ, _jsi_TOP, 0);
                    
                    jsiPop(interp,2);
                } else Jsi_LogBug("delete");
                break;
            }
            case OP_OBJECT: {
                int itemcount = (uintptr_t)ip->data;
                Assert(itemcount>=0);
                jsiVarDeref(interp, itemcount * 2);
                Jsi_Obj *obj = Jsi_ObjNewObj(interp, _jsi_STACK+(interp->framePtr->Sp-itemcount*2), itemcount*2);
                jsiPop(interp, itemcount * 2 - 1);       /* one left */
                jsiClearStack(interp,1);
                Jsi_ValueMakeObject(interp, &_jsi_TOP, obj);
                break;
            }
            case OP_ARRAY: {
                int itemcount = (uintptr_t)ip->data;
                Assert(itemcount>=0);
                jsiVarDeref(interp, itemcount);
                Jsi_Obj *obj = Jsi_ObjNewArray(interp, _jsi_STACK+(interp->framePtr->Sp-itemcount), itemcount, 1);
                jsiPop(interp, itemcount - 1);
                jsiClearStack(interp,1);
                Jsi_ValueMakeObject(interp, &_jsi_TOP, obj);
                break;
            }
            case OP_STRY: {
                jsi_TryInfo *ti = (jsi_TryInfo *)ip->data;
                jsi_TryList *n = jsi_trylist_new(jsi_TL_TRY, scope, currentScope);
                
                n->d.td.tstart = ip;                            /* make every thing pointed to right pos */
                n->d.td.tend = n->d.td.tstart + ti->trylen;
                n->d.td.cstart = n->d.td.tend + 1;
                n->d.td.cend = n->d.td.tend + ti->catchlen;
                n->d.td.fstart = n->d.td.cend + 1;
                n->d.td.fend = n->d.td.cend + ti->finallen;
                n->d.td.tsp = interp->framePtr->Sp;
                n->inCatch=0;
                n->inFinal=0;

                jsi_push_try(interp, &trylist, n);
                break;
            }
            case OP_ETRY: {             /* means nothing happen go to final */
                if (trylist == NULL || trylist->type != jsi_TL_TRY)
                    Jsi_LogBug("Unexpected ETRY opcode??");

                ip = trylist->d.td.fstart - 1;
                break;
            }
            case OP_SCATCH: {
                if (trylist == NULL || trylist->type != jsi_TL_TRY) 
                    Jsi_LogBug("Unexpected SCATCH opcode??");

                if (!ip->data) {
                    JSI_DO_THROW("catch");
                } else {
                    trylist->inCatch=1;
                    /* new scope and make var */
                    scope = jsi_ScopeChainDupNext(interp, scope, currentScope);
                    currentScope = jsi_ObjValueNew(interp);
                    interp->framePtr->ingsc = scope;  //TODO: changing frame
                    interp->framePtr->incsc = currentScope;
                    Jsi_IncrRefCount(interp, currentScope);
                    Jsi_Value *excpt = Jsi_ValueNew1(interp);
                    if (ps->last_exception && ps->last_exception->vt != JSI_VT_UNDEF) {
                        //TODO: fix test262 crash in freeValueTbl@jsiInterp.c:565 for last_exception which is
                        // freed in jsi_PstateFree@jsiPstate.c:251. Is this code the problem?
                        Jsi_Value *ple = ps->last_exception;
                        Jsi_ValueCopy(interp, excpt, ple);
                        Jsi_ValueReset(interp, &ps->last_exception);
                    } else if (interp->errMsgBuf[0]) {
                        Jsi_ValueMakeStringDup(interp, &excpt, interp->errMsgBuf);
                        interp->errMsgBuf[0] = 0;
                    }
                    Jsi_ValueInsert(interp, currentScope, (char*)ip->data, excpt, JSI_OM_DONTENUM);
                    Jsi_DecrRefCount(interp, excpt);
                    context_id = ps->_context_id++;
                }
                break;
            }
            case OP_ECATCH: {
                if (trylist == NULL || trylist->type != jsi_TL_TRY)
                    Jsi_LogBug("Unexpected ECATCH opcode??");

                trylist->inCatch=0;
                ip = trylist->d.td.fstart - 1;
                break;
            }
            case OP_SFINAL: {
                if (trylist == NULL || trylist->type != jsi_TL_TRY)
                    Jsi_LogBug("Unexpected SFINAL opcode??");

                /* restore scatch scope chain */
                trylist->inFinal = 1;
                JSI_RESTORE_SCOPE();
                break;
            }
            case OP_EFINAL: {
                if (trylist == NULL || trylist->type != jsi_TL_TRY)
                    Jsi_LogBug("Unexpected EFINAL opcode??");

                trylist->inFinal = 0;
                int last_op = trylist->d.td.last_op;
                jsi_OpCode *tojmp = (last_op == jsi_LOP_JMP ? trylist->d.td.ld.tojmp : 0);
                
                pop_try(trylist);

                if (last_op == jsi_LOP_THROW) {
                    JSI_DO_THROW("finally");
                } else if (last_op == jsi_LOP_JMP) {
                    while (1) {
                        if (trylist == NULL) {
                            ip = tojmp;
                            break;
                        }
                        /* same as jmp opcode, see above */
                        if (trylist->type == jsi_TL_TRY) {
                            if (tojmp >= trylist->d.td.tstart && tojmp < trylist->d.td.fend) {
                                ip = tojmp;
                                break;
                            }
                            
                            if (ip >= trylist->d.td.tstart && ip < trylist->d.td.cend) {
                                trylist->d.td.last_op = jsi_LOP_JMP;
                                trylist->d.td.ld.tojmp = tojmp;
                                
                                ip = trylist->d.td.fstart - 1;
                                break;
                            } else if (ip >= trylist->d.td.fstart && ip < trylist->d.td.fend) {
                                pop_try(trylist);
                            } else Jsi_LogBug("jmp within a try, but not in its scope?");
                        } else {        /* 'with' block */
                            if (tojmp >= trylist->d.wd.wstart && tojmp < trylist->d.wd.wend) {
                                ip = tojmp;
                                break;
                            }
                            JSI_RESTORE_SCOPE();
                            pop_try(trylist);
                        }
                    }
                }
                break;
            }
            case OP_THROW: {
                jsiVarDeref(interp,1);
                Jsi_ValueDup2(interp,&ps->last_exception, _jsi_TOP);
                interp->didReturn = 1; /* TODO: could possibly hide _jsi_STACK problem */
                JSI_DO_THROW("throw");
                break;
            }
            case OP_WITH: {
                static int warnwith = 1;
                if (strict && warnwith && interp->typeCheck.nowith) {
                    warnwith = 0;
                    rc = Jsi_LogError("use of with is illegal due to \"use nowith\"");
                    break;
                }
                jsiVarDeref(interp,1);
                Jsi_ValueToObject(interp, _jsi_TOP);
                
                jsi_TryList *n = jsi_trylist_new(jsi_TL_WITH, scope, currentScope);
                
                n->d.wd.wstart = ip;
                n->d.wd.wend = n->d.wd.wstart + (uintptr_t)ip->data;

                jsi_push_try(interp, &trylist, n);
                interp->framePtr->withDepth++;
                
                /* make expr to top of scope chain */
                scope = jsi_ScopeChainDupNext(interp, scope, currentScope);
                currentScope = Jsi_ValueNew1(interp);
                interp->framePtr->ingsc = scope;
                interp->framePtr->incsc = currentScope;
                Jsi_ValueCopy(interp, currentScope, _jsi_TOP);
                jsiPop(interp,1);
                
                context_id = ps->_context_id++;
                break;
            }
            case OP_EWITH: {
                if (trylist == NULL || trylist->type != jsi_TL_WITH)
                    Jsi_LogBug("Unexpected EWITH opcode??");

                JSI_RESTORE_SCOPE();
                
                pop_try(trylist);
                interp->framePtr->withDepth--;
                break;
            }
            case OP_DEBUG: {
                jsi_DebuggerStmt();
                jsiPush(interp,1);
                break;
            }
            case OP_RESERVED: {
                jsi_ReservedInfo *ri = (jsi_ReservedInfo *)ip->data;
                const char *cmd = ri->type == RES_CONTINUE ? "continue" : "break";
                /* TODO: continue/break out of labeled scope: see tests/prob/break.jsi. */
                if (ri->label) {
                    Jsi_LogError("%s: label(%s) not found", cmd, ri->label);
                } else {
                    Jsi_LogError("%s must be inside loop(or switch)", cmd);
                }
                rc = JSI_ERROR;
                break;
            }
#ifndef __cplusplus
            default:
                Jsi_LogBug("invalid op ceod: %d", ip->op);
#endif
        }
        lop = plop;
        ip++;
    }
done:
    while (trylist) {
        JSI_RESTORE_SCOPE();
        pop_try(trylist);
    }
    return rc;
}

// Bottom-most eval() routine creates stack frame.
Jsi_RC jsi_evalcode(jsi_Pstate *ps, Jsi_Func *func, Jsi_OpCodes *opcodes, 
         jsi_ScopeChain *scope, Jsi_Value *fargs,
         Jsi_Value *_this,
         Jsi_Value **vret)
{
    Jsi_Interp *interp = ps->interp;
    if (interp->exited)
        return JSI_ERROR;
    Jsi_RC rc;
    jsi_Frame frame = *interp->framePtr;
    frame.parent = interp->framePtr;
    interp->framePtr = &frame;
    frame.parent->child = interp->framePtr = &frame;
    frame.ps = ps;
    frame.ingsc = scope;
    frame.incsc = fargs;
    frame.inthis = _this;
    frame.opcodes = opcodes;
    frame.fileName = ((func && func->script)?func->script:interp->curFile);
    frame.funcName = interp->curFunction;
    frame.dirName = interp->curDir;
    if (frame.fileName && frame.fileName == frame.parent->fileName)
        frame.logflag = frame.parent->logflag;
    else
        frame.logflag = 0;
    frame.level = frame.parent->level+1;
    frame.evalFuncPtr = func;
    frame.arguments = NULL;
   // if (func && func->strict)
    //    frame.strict = 1;
    if (interp->curIp)
        frame.parent->line = interp->curIp->Line;
    frame.ip = interp->curIp;
    interp->refCount++;
    interp->level++;
    Jsi_IncrRefCount(interp, fargs);
    rc = jsi_evalcode_sub(ps, opcodes, scope, fargs, _this, *vret);
    Jsi_DecrRefCount(interp, fargs);
    if (interp->didReturn == 0 && !interp->exited && rc == JSI_OK) {
        if ((interp->evalFlags&JSI_EVAL_RETURN)==0)
            Jsi_ValueMakeUndef(interp, vret);
        /*if (interp->framePtr->Sp != oldSp) //TODO: at some point after memory refs???
            Jsi_LogBug("Stack not balance after execute script");*/
    }
    if (frame.arguments)
        Jsi_DecrRefCount(interp, frame.arguments);
    interp->didReturn = 0;
    interp->refCount--;
    interp->level--;
    interp->framePtr = frame.parent;
    interp->framePtr->child = NULL;
    interp->curIp = frame.ip;
    if (interp->exited)
        rc = JSI_ERROR;
    return rc;
}

Jsi_RC jsi_JsPreprocessLine(Jsi_Interp* interp, char *buf, size_t bsiz, uint ilen, int jOpts[4], int lineNo) {
    if (buf[0]==';' && buf[1] && buf[2]) {
        // Wrap ";XXX;" in a puts("XXX ==> ", XXX)
        if (!jOpts[0]) {
            if (!Jsi_Strcmp(buf, "=!EXPECTSTART!=\n") || !Jsi_Strcmp(buf, "=!INPUTSTART!=\n") ) {
                return JSI_OK;
            }
        } else {
            if (!Jsi_Strcmp(buf, "=!EXPECTEND!=\n") || !Jsi_Strcmp(buf, "=!INPUTEND!=\n")) {
                jOpts[0] = 0;
                return JSI_OK;
            }
        }
        if (buf[ilen-1]=='\n' && buf[ilen-2]==';' && (2*ilen+12)<bsiz) {
            if (Jsi_Strchr(buf, '`')) {
                return Jsi_LogError("back-tick is illegal in unitTest on line %d: %s", lineNo, buf);
            }
            char ubuf[bsiz], *ucp = ubuf;
            buf[ilen-=2] = 0;
            Jsi_Strcpy(ubuf, buf+1);
            while (*ucp && isspace(*ucp)) ucp++;
            if (ilen>2 && ucp[0]=='\'' && ubuf[ilen-2]=='\'')
                snprintf(buf, bsiz, "puts(`%s`);\n", ucp); //output a 'Comment'
                
            else if (interp->debugOpts.testFmtCallback) {
                Jsi_DString kStr={};
                Jsi_Value *vrc = Jsi_ValueNew1(interp);
                Jsi_DSPrintf(&kStr, "[\"%s\", %d ]", ucp, lineNo);
                Jsi_RC rcs = Jsi_FunctionInvokeJSON(interp->parent, interp->debugOpts.testFmtCallback, Jsi_DSValue(&kStr), &vrc);
                if (rcs == JSI_OK) {
                    const char *cps = Jsi_ValueString(interp, vrc, NULL);
                    if (!cps)
                        rcs = JSI_ERROR;
                    else
                        snprintf(buf, bsiz, "%s", cps);
                }
                Jsi_DecrRefCount(interp, vrc);
                Jsi_DSFree(&kStr);
                if (rcs != JSI_OK)
                    return Jsi_LogError("failure in debugOpts.testFmtCallback");
            } else if (ilen>3 && ubuf[0]=='/' && ubuf[0]=='/') {
                char *ecp = ubuf+2;
                while (*ecp && isspace(*ecp)) ecp++;
                snprintf(buf, bsiz, "printf(`%%s ==>`, \"%s\"); try { %s; puts('\\nFAIL!\\n'); } "
                    "catch(err) { puts('\\nPASS!: err =',err); }\n", ecp, ecp);
            } else {
                snprintf(buf, bsiz, "printf(`%%s ==> `,`%s`),puts(%s);\n", ucp, ucp);
            }
        }
    }
    return JSI_OK;
}

Jsi_RC jsi_JsPreprocessLineCB(Jsi_Interp* interp, char *buf, size_t bsiz, uint ilen, int jOpts[4], int lineNo) {
    const char *jpp = interp->jsppChars;
    if (!jpp[0] || !jpp[1])
        return JSI_OK;
    if (buf[0] && jpp[0] == buf[0] && ilen>2 && buf[ilen-2]==jpp[1]) {
        Jsi_DString dStr = {};
        buf[ilen-2] = 0; // Remove last char and newline.
        Jsi_Value *inStr = Jsi_ValueNewStringDup(interp, buf+1);
        Jsi_IncrRefCount(interp, inStr);
        Jsi_RC rc = Jsi_FunctionInvokeString(interp, interp->jsppCallback, inStr, &dStr);
        if (Jsi_InterpGone(interp))
            return JSI_ERROR;
        if (rc != JSI_OK) {
            Jsi_DSFree(&dStr);
            Jsi_DecrRefCount(interp, inStr);
            return JSI_ERROR;
        }
        Jsi_DecrRefCount(interp, inStr);
        Jsi_DSAppendLen(&dStr, "\n", 1);
        Jsi_Strncpy(buf, Jsi_DSValue(&dStr), bsiz);
        buf[bsiz-1] = 0;
        Jsi_DSFree(&dStr);
    }
    return JSI_OK;
}

Jsi_RC jsi_evalStrFile(Jsi_Interp* interp, Jsi_Value *path, const char *str, int flags, int level)
{
    Jsi_Channel tinput = NULL, input = Jsi_GetStdChannel(interp, 0);
    Jsi_Value *npath = path;
    Jsi_RC rc = JSI_ERROR;
    const char *ustr = NULL, *ostr = str;
    if (Jsi_MutexLock(interp, interp->Mutex) != JSI_OK)
        return rc;
    int oldSp, uskip = 0, fncOfs = 0, fnLen;
    int oldef = interp->evalFlags;
    jsi_Pstate *oldps = interp->ps;
    const char *oldFile = interp->curFile;
    char *origFile = Jsi_ValueString(interp, path, &fnLen);
    const char *fname = origFile;
    char *oldDir = interp->curDir, *cp;
    char dirBuf[PATH_MAX];
    jsi_Pstate *ps = NULL;
    jsi_FileInfo *fi = NULL;
    int exists = (flags&JSI_EVAL_EXISTS);
    int ignore = (flags&JSI_EVAL_ERRIGNORE);
    if (flags & JSI_EVAL_GLOBAL)
        level = 1;
    int oisi = interp->isMain;
    if (flags & JSI_EVAL_ISMAIN)
        interp->isMain = 1;
    
    oldSp = interp->framePtr->Sp;
    dirBuf[0] = 0;
    Jsi_DString dStr, fStr;
    Jsi_DSInit(&dStr);
    Jsi_DSInit(&fStr);

    if (str == NULL) {
        if (fname != NULL) {
            if (fnLen>2 && fname[fnLen-1]=='/') {
                Jsi_DSAppendLen(&fStr, fname, fnLen-1);
                const char *fcp = Jsi_DSValue(&fStr), *fce = strrchr(fcp,'/');
                if (fce) {
                    fname = Jsi_DSAppend(&fStr, fce, ".jsi", NULL);
                    npath = Jsi_ValueNewStringDup(interp, fname);
                    Jsi_IncrRefCount(interp, npath);
                }
            }
            if (!Jsi_Strcmp(fname,"-"))
                input = Jsi_GetStdChannel(interp, 0);
            else {
                /* Use translated FileName. */
                if (interp->curDir && fname[0] != '/' && fname[0] != '~') {
                    char dirBuf2[PATH_MAX], *np;
                    snprintf(dirBuf, sizeof(dirBuf), "%s/%s", interp->curDir, fname);
                    if ((np=Jsi_FileRealpathStr(interp, dirBuf, dirBuf2)) == NULL) {
                        Jsi_LogError("Can not open '%s'", fname);
                        goto bail;
                    }
                    if (npath != path)
                        Jsi_IncrRefCount(interp, npath);
                    npath = Jsi_ValueNewStringDup(interp, np);
                    Jsi_IncrRefCount(interp, npath);
                    fname = Jsi_ValueString(interp, npath, NULL);
                }
                if (flags&JSI_EVAL_ARGV0) {
                    if (interp->argv0)
                        Jsi_DecrRefCount(interp, interp->argv0);
                    interp->argv0 = Jsi_ValueNewStringDup(interp, fname);
                    Jsi_IncrRefCount(interp, interp->argv0);
                }
                
                bool osafe = interp->isSafe;
                if (interp->startSafe  && flags&JSI_EVAL_ARGV0) {
                    if (interp->safeReadDirs || interp->safeMode == jsi_safe_None)
                        interp->isSafe = 0;
                    else {
                        char vds[PATH_MAX], *cp;
                        const char *vda[2] = {vds};
                        Jsi_Strncpy(vds, Jsi_ValueString(interp, npath, NULL), sizeof(vds)-1);
                        vds[sizeof(vds)-1] = 0;
                        cp = Jsi_Strrchr(vds, '/');
                        if (cp)
                            cp[1] = 0;
                        Jsi_DString pStr = {};
                        vda[1] = Jsi_GetCwd(interp, &pStr);
                        interp->safeReadDirs = Jsi_ValueNewArray(interp, vda, 2);
                        Jsi_IncrRefCount(interp, interp->safeReadDirs);
                        if (interp->safeMode == jsi_safe_WriteRead || interp->safeMode == jsi_safe_Lockdown) {
                            if (!interp->safeWriteDirs) {
                                interp->safeWriteDirs = interp->safeReadDirs;
                                Jsi_IncrRefCount(interp, interp->safeWriteDirs);
                            }
                        } else if (interp->safeMode == jsi_safe_Write) {
                            interp->safeWriteDirs = Jsi_ValueNewArray(interp, vda+1, 1);
                            Jsi_IncrRefCount(interp, interp->safeWriteDirs);
                        }
                        Jsi_DSFree(&pStr);
                    }
                }
                tinput = input = Jsi_Open(interp, npath, (exists?"-r":"r"));
                interp->isSafe = osafe;
                if (!input) {
                    if (exists)
                        rc = JSI_OK;
                    //Jsi_LogError("Can not open '%s'", fname);
                    goto bail;
                }
            }
            cp = Jsi_Strrchr(fname, '.');
            if (cp && !Jsi_Strcmp(cp, ".jsi") && interp->isMain) {
                interp->typeCheck.parse = interp->typeCheck.run = interp->typeCheck.all = 1;
                interp->strict = 1;
            }
            bool isNew;
            Jsi_HashEntry *hPtr;
            hPtr = Jsi_HashEntryNew(interp->fileTbl, fname, &isNew);
            if (isNew == 0 && hPtr) {
                if ((flags & JSI_EVAL_ONCE)) {
                    rc = JSI_OK;
                    goto bail;
                }
                fi = (jsi_FileInfo *)Jsi_HashValueGet(hPtr);
                if (!fi) goto bail;
                interp->curFile = fi->fileName;
                interp->curDir = fi->dirName;
                
            } else {
                fi = (jsi_FileInfo *)Jsi_Calloc(1,sizeof(*fi));
                if (!fi) goto bail;
                Jsi_HashValueSet(hPtr, fi);
                fi->origFile = (char*)Jsi_KeyAdd(interp, origFile);
                interp->curFile = fi->fileName = (char*)Jsi_KeyAdd(interp, fname);
                char *dfname = Jsi_Strdup(fname);
                if ((cp = Jsi_Strrchr(dfname,'/')))
                    *cp = 0;
                interp->curDir = fi->dirName = (char*)Jsi_KeyAdd(interp, dfname);
                Jsi_Free(dfname);
            }
            if (!input->fname)
                input->fname = interp->curFile;

            int cnt = 0, noncmt = 0, jppOpts[4]={};
            uint ilen;
            char buf[JSI_BUFSIZ*2];
            const char *jpp;
            while (cnt<MAX_LOOP_COUNT) {
                if (!Jsi_Gets(interp, input, buf, sizeof(buf)))
                    break;
                if (++cnt==1 && (!(flags&JSI_EVAL_NOSKIPBANG)) && (buf[0] == '#' && buf[1] == '!')) {
                    Jsi_DSAppend(&dStr, "\n", NULL);
                    uskip=1;
                    continue;
                }
                if (!noncmt) {
                    int bi;
                    if (!buf[0] || (buf[0] == '/' && buf[1] == '/'))
                        goto cont;
                    for (bi=0; buf[bi]; bi++) if (!isspace(buf[bi])) break;
                    if (!buf[bi])
                        goto cont;
                }
                if (!noncmt++)
                    fncOfs = Jsi_DSLength(&dStr)-uskip;
                jpp = interp->jsppChars;
                if (jpp || interp->unitTest)
                    ilen = Jsi_Strlen(buf);
                if (interp->unitTest && buf[0]==';' && buf[1] && buf[2]) {
                    if (interp->unitTest&1 && jsi_JsPreprocessLine(interp, buf, sizeof(buf), ilen, jppOpts, cnt) != JSI_OK)
                        goto bail;
                } else if (interp->jsppCallback && interp->jsppChars) {
                    if (jsi_JsPreprocessLineCB(interp, buf, sizeof(buf), ilen, jppOpts, cnt) != JSI_OK)
                        goto bail;
                }
cont:
                Jsi_DSAppend(&dStr, buf,  NULL);
            }
            if (cnt>=MAX_LOOP_COUNT)
                Jsi_LogError("source file too large");
            str = Jsi_DSValue(&dStr);

        }
        if (interp->curDir && (flags&JSI_EVAL_AUTOINDEX))
            Jsi_AddAutoFiles(interp, interp->curDir);
    }
    ustr = str + fncOfs;
    // See if "use XXX" is on first non // or empty line (or second if there is a #! on first line)
    if (ustr && *ustr && !Jsi_Strncmp(ustr+uskip, "\"use ", 5)) {
        ustr += 5+uskip;
        const char *cpe = ustr;
        while (*cpe && *cpe != '\"' && *cpe != '\n' && (isalpha(*cpe) || *cpe ==',' || *cpe =='!')) cpe++;
        if (*cpe == '\"') {
            Jsi_DString cStr;
            Jsi_DSInit(&cStr);
            cpe = Jsi_DSAppendLen(&cStr, ustr, (cpe-ustr));
            rc = jsi_ParseTypeCheckStr(interp, cpe);
            Jsi_DSFree(&cStr);
            if (rc != JSI_OK)
                goto bail;
        }
    }

    /* TODO: cleanup interp->framePtr->Sp stuff. */
    oldSp = interp->framePtr->Sp;
    // Evaluate code.
    rc = JSI_OK;
    ps = jsiNewParser(interp, str, input, 0);
    interp->evalFlags = flags;
    if (!ps)
        rc = JSI_ERROR;
    else {
        Jsi_ValueMakeUndef(interp, &interp->retValue);
        interp->ps = ps;
        Jsi_Value *retValue = interp->retValue;
        if (!interp->strict)
            interp->strict = (jsi_GetDirective(interp, ps->opcodes, "use strict")!=NULL);
        const char *curFile = interp->curFile;

        if (level <= 0)
            rc = jsi_evalcode(ps, NULL, ps->opcodes, interp->gsc, interp->csc, interp->csc, &retValue);
        else {
            jsi_Frame *fptr = interp->framePtr;
            while (fptr && fptr->level > level)
                fptr = fptr->parent;
            if (!fptr)
                rc = JSI_ERROR;
            else
                rc = jsi_evalcode(ps, NULL, ps->opcodes, fptr->ingsc, fptr->incsc, fptr->inthis, &retValue);
        }
        interp->curFile = curFile;
        if (rc != JSI_OK)
            rc = (ignore?JSI_OK:JSI_ERROR);
        else if (ps->last_exception || oldps->last_exception)
            Jsi_ValueDup2(interp, &oldps->last_exception, ps->last_exception); //TODO: dup even if null?
        interp->ps = oldps;
        interp->evalFlags = oldef;
        if (rc == JSI_OK && ps && !ostr)
            fi->str = ps->lexer->d.str;
    }
    
bail:
    interp->curFile = oldFile;
    interp->curDir = oldDir;
    interp->framePtr->Sp = oldSp;
    interp->isMain = oisi;
    if (path != npath)
        Jsi_DecrRefCount(interp, npath);
    Jsi_DSFree(&dStr);
    Jsi_DSFree(&fStr);
    if (tinput)
        Jsi_Close(interp, tinput);
    Jsi_MutexUnlock(interp, interp->Mutex);
    if (interp->exited && interp->level <= 0)
    {
        rc = JSI_EXIT;
        if (!interp->parent)
            Jsi_InterpDelete(interp);
    }

    return rc;
}

Jsi_RC Jsi_EvalFile(Jsi_Interp* interp, Jsi_Value *fname, int flags)
{
    int isnull;
    if ((isnull=Jsi_ValueIsNull(interp, fname)) || Jsi_ValueIsUndef(interp, fname)) 
        return Jsi_LogError("invalid file eval %s", (isnull?"null":"undefined"));
    int lev = interp->framePtr->level;
    return jsi_evalStrFile(interp, fname, NULL, flags, lev);
}

Jsi_RC Jsi_EvalString(Jsi_Interp* interp, const char *str, int flags)
{
    int lev = interp->framePtr->level;
    return jsi_evalStrFile(interp, NULL, (char*)str, flags, lev);
}

#undef _jsi_THIS
#undef _jsi_STACK
#undef _jsi_STACKIDX
#undef _jsi_THISIDX
#undef _jsi_TOP
#undef _jsi_TOQ

#endif
