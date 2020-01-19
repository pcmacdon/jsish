#ifndef JSI_LITE_ONLY
#ifndef _JSI_CODE_C_
#define _JSI_CODE_C_
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

/* replace continue/break(coded as OP_RESERVED) jmp
 * |------------------| \
 * |                  | \\ where 'continue' jmp (jmp to step code)
 * |       ops        |  / 
 * |                  | / \
 * |------------------|    \ where 'break' jmp (jmp after step code)
 * |                  |    /
 * |       step       |   /
 * |                  |  /
 * |------------------| /
 * 1. break_only used only in switch
 * 2. desire_label, only replace if current iter statement has the same label with opcode
 * 3. topop, if not replace in current iter statment, make sure when jmp out of this loop/switch
 *    corrent stack elems poped(for in always has 2 elem, while switch has 1)
 */

static const char *jsi_op_names[OP_LASTOP] = {
    "NOP",
    "PUSHNUM",
    "PUSHSTR",
    "PUSHVSTR",
    "PUSHVAR",
    "PUSHUND",
    "PUSHNULL",
    "PUSHBOO",
    "PUSHFUN",
    "PUSHREG",
    "PUSHARG",
    "PUSHTHS",
    "PUSHTOP",
    "PUSHTOP2",
    "UNREF",
    "POP",
    "LOCAL",
    "NEG",
    "POS",
    "NOT",
    "BNOT",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "LESS",
    "GREATER",
    "LESSEQU",
    "GREATEREQU",
    "EQUAL",
    "NOTEQUAL",
    "STRICTEQU",
    "STRICTNEQ",
    "BAND",
    "BOR",
    "BXOR",
    "SHF",
    "INSTANCEOF",
    "ASSIGN",
    "SUBSCRIPT",
    "INC",
    "TYPEOF",
    "IN",
    "DEC",
    "KEY",
    "NEXT",
    "JTRUE",
    "JFALSE",
    "JTRUE_NP",
    "JFALSE_NP",
    "JMP",
    "JMPPOP",
    "FCALL",
    "NEWFCALL",
    "RET",
    "DELETE",
    "CHTHIS",
    "OBJECT",
    "ARRAY",
    "EVAL",
    "STRY",
    "ETRY",
    "SCATCH",
    "ECATCH",
    "SFINAL",
    "EFINAL",
    "THROW",
    "WITH",
    "EWITH",
    "RESERVED",
    "DEBUG"
};

static int jsiOpCodesCnt[3] = {0,0,0};

void jsi_FreeOpcodes(Jsi_OpCodes *ops) {
    int i;
    if (!ops) return;
    for (i=0; i<ops->code_len; i++) {
        jsi_OpCode *op = ops->codes+i;
        if (op->data && op->alloc)
            Jsi_Free(op->data);
        _JSI_MEMCLEAR(op);
    }
    jsiOpCodesCnt[1]++;
    jsiOpCodesCnt[2]--;
#ifdef JSI_MEM_DEBUG
    if (ops->hPtr)
        Jsi_HashEntryDelete(ops->hPtr);
#endif
    Jsi_Free(ops->codes);
    Jsi_Free(ops);
}

static Jsi_OpCodes *codes_new(int size)
{
    Jsi_OpCodes *ops = (Jsi_OpCodes *)Jsi_Calloc(1, sizeof(*ops));
    jsiOpCodesCnt[0]++;
    jsiOpCodesCnt[2]++;
    ops->codes = (jsi_OpCode *)Jsi_Calloc(size, sizeof(jsi_OpCode));
    ops->code_size = size;
#ifdef JSI_MEM_DEBUG
    static int idNum = 0;
    ops->hPtr = Jsi_HashSet(jsiIntData.mainInterp->codesTbl, ops, ops);
    ops->id = idNum++;
#endif
    return ops;
}

static int codes_insert(Jsi_OpCodes *c, jsi_Eopcode code, void *extra, int doalloc)
{
    if (c->code_size - c->code_len <= 0) {
        c->code_size += 100;
        c->codes = (jsi_OpCode *)Jsi_Realloc(c->codes, c->code_size * sizeof(jsi_OpCode));
    }
    c->codes[c->code_len].op = code;
    c->codes[c->code_len].data = extra;
    c->codes[c->code_len].alloc = doalloc;
    c->code_len ++;
    return 0;
}

static int codes_insertln(Jsi_OpCodes *c, jsi_Eopcode code, void *extra, jsi_Pstate *pstate, jsi_Pline *line, int doalloc)
{
    if (c->code_size - c->code_len <= 0) {
        c->code_size += 100;
        c->codes = (jsi_OpCode *)Jsi_Realloc(c->codes, c->code_size * sizeof(jsi_OpCode));
    }
    c->codes[c->code_len].op = code;
    c->codes[c->code_len].data = extra;
    c->codes[c->code_len].Line = line->first_line;
    c->codes[c->code_len].Lofs = line->first_column;
    c->codes[c->code_len].fname = jsi_PstateGetFilename(pstate);
    c->codes[c->code_len].alloc = doalloc;
    c->code_len ++;
    return 0;
}


static Jsi_OpCodes *codes_join(Jsi_OpCodes *a, Jsi_OpCodes *b)
{
    Jsi_OpCodes *ret = codes_new(a->code_len + b->code_len);
    memcpy(ret->codes, a->codes, a->code_len * sizeof(jsi_OpCode));
    memcpy(&ret->codes[a->code_len], b->codes, b->code_len * sizeof(jsi_OpCode));
    ret->code_size = a->code_len + b->code_len;
    ret->code_len = ret->code_size;
    ret->expr_counter = a->expr_counter + b->expr_counter;
#if 0
    a->code_len=0;
    jsi_FreeOpcodes(a);
    b->code_len=0;
    jsi_FreeOpcodes(b);
#else
    Jsi_Free(a->codes);
    Jsi_Free(b->codes);
#ifdef JSI_MEM_DEBUG
    if (a->hPtr)
        Jsi_HashEntryDelete(a->hPtr);
    if (b->hPtr)
        Jsi_HashEntryDelete(b->hPtr);
#endif
    Jsi_Free(a);
    Jsi_Free(b);
#endif
    jsiOpCodesCnt[1]++;
    jsiOpCodesCnt[2]-=2;
    return ret;
}

static Jsi_OpCodes *codes_join3(Jsi_OpCodes *a, Jsi_OpCodes *b, Jsi_OpCodes *c)
{
    return codes_join(codes_join(a, b), c);
}

static Jsi_OpCodes *codes_join4(Jsi_OpCodes *a, Jsi_OpCodes *b, Jsi_OpCodes *c, Jsi_OpCodes *d)
{
    return codes_join(codes_join(a, b), codes_join(c, d));
}

#define JSI_NEW_CODES(doalloc,code, extra) do {                 \
        Jsi_OpCodes *r = codes_new(3);                  \
        codes_insert(r, (code), (void *)(uintptr_t)(extra), doalloc);   \
        return r;                                   \
    } while(0)

#define JSI_NEW_CODESLN(doalloc,code, extra) do {                 \
        Jsi_OpCodes *r = codes_new(3);                  \
        codes_insertln(r, (code), (void *)(uintptr_t)(extra), p, line, doalloc);   \
        return r;                                   \
    } while(0)

static Jsi_OpCodes *code_push_undef() { JSI_NEW_CODES(0,OP_PUSHUND, 0); }
static Jsi_OpCodes *code_push_null() { JSI_NEW_CODES(0,OP_PUSHNULL, 0); }
static Jsi_OpCodes *code_push_bool(int v) { JSI_NEW_CODES(0,OP_PUSHBOO, v); }
static Jsi_OpCodes *code_push_num(Jsi_Number *v) { JSI_NEW_CODES(1,OP_PUSHNUM, v); }
static Jsi_OpCodes *code_push_string(jsi_Pstate *p, jsi_Pline *line, const char *str) {
    if (*str == 'c' && !Jsi_Strcmp(str,"callee"))
        p->interp->hasCallee = 1;
    JSI_NEW_CODESLN(0,OP_PUSHSTR, str);
}

static Jsi_OpCodes *code_push_vstring(jsi_Pstate *p, jsi_Pline *line, Jsi_String *s) {
    JSI_NEW_CODESLN(0,OP_PUSHVSTR, s);
}

static Jsi_OpCodes *code_push_index(jsi_Pstate *p, jsi_Pline *line, const char *varname, int local)
{
    jsi_FastVar *n = (typeof(n))Jsi_Calloc(1, sizeof(*n));
    n->sig = JSI_SIG_FASTVAR;
    n->ps = p;
    n->context_id = -1;
    n->local = local;
    n->varname = (char*)Jsi_KeyAdd(p->interp, varname);
    Jsi_HashSet(p->fastVarTbl, n, n);
    JSI_NEW_CODESLN(1,OP_PUSHVAR, n);
}

static Jsi_OpCodes *code_push_this(jsi_Pstate *p, jsi_Pline *line) { JSI_NEW_CODESLN(0,OP_PUSHTHS, 0); }
static Jsi_OpCodes *code_push_top() { JSI_NEW_CODES(0,OP_PUSHTOP, 0); }
static Jsi_OpCodes *code_push_top2() { JSI_NEW_CODES(0,OP_PUSHTOP2, 0); }
static Jsi_OpCodes *code_unref() { JSI_NEW_CODES(0,OP_UNREF, 0); }
static Jsi_OpCodes *code_push_args() { JSI_NEW_CODES(0,OP_PUSHARG, 0); }
static Jsi_OpCodes *code_push_func_sub(jsi_Pstate *p, jsi_Pline *line, Jsi_Func *fun) { p->funcDefs++; JSI_NEW_CODESLN(0,OP_PUSHFUN, fun); }
static Jsi_OpCodes *code_push_func(jsi_Pstate *p, jsi_Pline *line, Jsi_Func *fun) {
    Jsi_OpCodes* codes = code_push_func_sub(p, line, fun);
    if (codes && fun && fun->name)
        codes->codes[0].local = 1;
    return codes;
}
static Jsi_OpCodes *code_push_regex(jsi_Pstate *p, jsi_Pline *line, Jsi_Regex *reg) { JSI_NEW_CODESLN(0,OP_PUSHREG, reg); }

static Jsi_OpCodes *code_local(jsi_Pstate *p, jsi_Pline *line, const char *varname) { JSI_NEW_CODESLN(0,OP_LOCAL, varname); }

static Jsi_OpCodes *code_nop() { JSI_NEW_CODES(0,OP_NOP, 0); }
static Jsi_OpCodes *code_neg() { JSI_NEW_CODES(0,OP_NEG, 0); }
static Jsi_OpCodes *code_pos() { JSI_NEW_CODES(0,OP_POS, 0); }
static Jsi_OpCodes *code_bnot() { JSI_NEW_CODES(0,OP_BNOT, 0); }
static Jsi_OpCodes *code_not() { JSI_NEW_CODES(0,OP_NOT, 0); }
static Jsi_OpCodes *code_mul() { JSI_NEW_CODES(0,OP_MUL, 0); }
static Jsi_OpCodes *code_div() { JSI_NEW_CODES(0,OP_DIV, 0); }
static Jsi_OpCodes *code_mod() { JSI_NEW_CODES(0,OP_MOD, 0); }
static Jsi_OpCodes *code_add() { JSI_NEW_CODES(0,OP_ADD, 0); }
static Jsi_OpCodes *code_sub() { JSI_NEW_CODES(0,OP_SUB, 0); }
static Jsi_OpCodes *code_in() { JSI_NEW_CODES(0,OP_IN, 0); }
static Jsi_OpCodes *code_less() { JSI_NEW_CODES(0,OP_LESS, 0); }
static Jsi_OpCodes *code_greater() { JSI_NEW_CODES(0,OP_GREATER, 0); }
static Jsi_OpCodes *code_lessequ() { JSI_NEW_CODES(0,OP_LESSEQU, 0); }
static Jsi_OpCodes *code_greaterequ() { JSI_NEW_CODES(0,OP_GREATEREQU, 0); }
static Jsi_OpCodes *code_equal() { JSI_NEW_CODES(0,OP_EQUAL, 0); } 
static Jsi_OpCodes *code_notequal() { JSI_NEW_CODES(0,OP_NOTEQUAL, 0); }
static Jsi_OpCodes *code_eequ() { JSI_NEW_CODES(0,OP_STRICTEQU, 0); }
static Jsi_OpCodes *code_nneq() { JSI_NEW_CODES(0,OP_STRICTNEQ, 0); }
static Jsi_OpCodes *code_band() { JSI_NEW_CODES(0,OP_BAND, 0); }
static Jsi_OpCodes *code_bor() { JSI_NEW_CODES(0,OP_BOR, 0); }
static Jsi_OpCodes *code_bxor() { JSI_NEW_CODES(0,OP_BXOR, 0); }
static Jsi_OpCodes *code_shf(int right) { JSI_NEW_CODES(0,OP_SHF, right); }
static Jsi_OpCodes *code_instanceof() { JSI_NEW_CODES(0,OP_INSTANCEOF, 0); }
static Jsi_OpCodes *code_assign(jsi_Pstate *p, jsi_Pline *line, int h) { JSI_NEW_CODESLN(0,OP_ASSIGN, h); }
static Jsi_OpCodes *code_subscript(jsi_Pstate *p, jsi_Pline *line, int right_val) { JSI_NEW_CODESLN(0,OP_SUBSCRIPT, right_val); }
static Jsi_OpCodes *code_inc(jsi_Pstate *p, jsi_Pline *line, int e) { JSI_NEW_CODESLN(0,OP_INC, e); }
static Jsi_OpCodes *code_dec(jsi_Pstate *p, jsi_Pline *line, int e) { JSI_NEW_CODESLN(0,OP_DEC, e); }
static Jsi_OpCodes *code_typeof(jsi_Pstate *p, jsi_Pline *line, int e) { JSI_NEW_CODESLN(0,OP_TYPEOF, e); }

static Jsi_OpCodes *code__fcall(jsi_Pstate *p, jsi_Pline *line, int argc, const char *name, const char *namePre, Jsi_OpCodes *argCodes) {
    jsi_FuncCallCheck(p,line,argc,1, name, namePre, argCodes);JSI_NEW_CODESLN(0,OP_FCALL, argc);
}
static Jsi_OpCodes *code_fcall(jsi_Pstate *p, jsi_Pline *line, int argc, const char *name, const char *namePre, Jsi_OpCodes *argCodes, Jsi_OpCodes* pref) {
    Jsi_OpCodes *codes = code__fcall(p, line, argc, name, namePre, argCodes);
    int i;
    if (!name || !codes || !pref)
        return codes;
    jsi_OpLogFlags logflag = jsi_Oplf_none;
    if (name[0] == 'a' && !Jsi_Strcmp(name, "assert"))
        logflag = jsi_Oplf_assert;
    else if (name[0] == 'L' && name[1] == 'o') {
        if (!Jsi_Strcmp(name, "LogDebug"))
            logflag = jsi_Oplf_debug;
        else if (!Jsi_Strcmp(name, "LogTrace"))
            logflag = jsi_Oplf_trace;
        else if (!Jsi_Strcmp(name, "LogTest"))
            logflag = jsi_Oplf_test;
    }
    if (logflag) {
        codes->codes[0].logflag = logflag;
        if (argCodes)
            for (i=0; i<argCodes->code_len; i++)
                argCodes->codes[i].logflag = logflag;
        for (i=0; i<pref->code_len; i++)
            pref->codes[i].logflag = logflag;
    }
    return codes;
}
static Jsi_OpCodes *code_newfcall(jsi_Pstate *p, jsi_Pline *line, int argc, const char *name, Jsi_OpCodes *argCodes) {
    jsi_FuncCallCheck(p,line,argc,1, name, NULL, argCodes); JSI_NEW_CODESLN(0,OP_NEWFCALL, argc);
}
static Jsi_OpCodes *code_ret(jsi_Pstate *p, jsi_Pline *line, int n) { JSI_NEW_CODESLN(0,OP_RET, n); }
static Jsi_OpCodes *code_delete(int n) { JSI_NEW_CODES(0,OP_DELETE, n); }
static Jsi_OpCodes *code_chthis(jsi_Pstate *p, jsi_Pline *line, int n) { JSI_NEW_CODESLN(0,OP_CHTHIS, n); }
static Jsi_OpCodes *code_pop(int n) { JSI_NEW_CODES(0,OP_POP, n); }
static Jsi_OpCodes *code_jfalse(int off) { JSI_NEW_CODES(0,OP_JFALSE, off); }
static Jsi_OpCodes *code_jtrue(int off) { JSI_NEW_CODES(0,OP_JTRUE, off); }
static Jsi_OpCodes *code_jfalse_np(int off) { JSI_NEW_CODES(0,OP_JFALSE_NP, off); }
static Jsi_OpCodes *code_jtrue_np(int off) { JSI_NEW_CODES(0,OP_JTRUE_NP, off); }
static Jsi_OpCodes *code_jmp(int off) { JSI_NEW_CODES(0,OP_JMP, off); }
static Jsi_OpCodes *code_object(jsi_Pstate *p, jsi_Pline *line, int c) { JSI_NEW_CODESLN(0,OP_OBJECT, c); }
static Jsi_OpCodes *code_array(jsi_Pstate *p, jsi_Pline *line, int c) { JSI_NEW_CODESLN(0,OP_ARRAY, c); }
static Jsi_OpCodes *code_key() { JSI_NEW_CODES(0,OP_KEY, 0); }
static Jsi_OpCodes *code_next() { JSI_NEW_CODES(0,OP_NEXT, 0); }

static Jsi_OpCodes *code_eval(jsi_Pstate *p, jsi_Pline *line, int argc, Jsi_OpCodes *c) {
    jsi_FreeOpcodes(c); // Eliminate leak of unused opcodes.
    JSI_NEW_CODESLN(0,OP_EVAL, argc);
}

static Jsi_OpCodes *code_stry(jsi_Pstate *p, jsi_Pline *line, int trylen, int catchlen, int finlen)
{ 
    jsi_TryInfo *ti = (jsi_TryInfo *)Jsi_Calloc(1,sizeof(*ti));
    ti->trylen = trylen;
    ti->catchlen = catchlen;
    ti->finallen = finlen;
    JSI_NEW_CODESLN(1,OP_STRY, ti); 
}
static Jsi_OpCodes *code_etry(jsi_Pstate *p, jsi_Pline *line) { JSI_NEW_CODESLN(0,OP_ETRY, 0); }
static Jsi_OpCodes *code_scatch(jsi_Pstate *p, jsi_Pline *line, const char *var) { JSI_NEW_CODESLN(0,OP_SCATCH, var); }
static Jsi_OpCodes *code_ecatch(jsi_Pstate *p, jsi_Pline *line) { JSI_NEW_CODESLN(0,OP_ECATCH, 0); }
static Jsi_OpCodes *code_sfinal(jsi_Pstate *p, jsi_Pline *line) { JSI_NEW_CODESLN(0,OP_SFINAL, 0); }
static Jsi_OpCodes *code_efinal(jsi_Pstate *p, jsi_Pline *line) { JSI_NEW_CODESLN(0,OP_EFINAL, 0); }
static Jsi_OpCodes *code_throw(jsi_Pstate *p, jsi_Pline *line) { JSI_NEW_CODESLN(0,OP_THROW, 0); }
static Jsi_OpCodes *code_with(jsi_Pstate *p, jsi_Pline *line, int withlen) { JSI_NEW_CODESLN(0,OP_WITH, withlen); }
static Jsi_OpCodes *code_ewith(jsi_Pstate *p, jsi_Pline *line) { JSI_NEW_CODESLN(0,OP_EWITH, 0); }

static Jsi_OpCodes *code_debug(jsi_Pstate *p, jsi_Pline *line) { JSI_NEW_CODESLN(0,OP_DEBUG, 0); }
static Jsi_OpCodes *code_reserved(jsi_Pstate *p, jsi_Pline *line, int type, const char *id)
{
    jsi_ReservedInfo *ri = (jsi_ReservedInfo*)Jsi_Calloc(1, sizeof(*ri));
    ri->type = type;
    ri->label = id;
    ri->topop = 0;
    JSI_NEW_CODESLN(1,OP_RESERVED, ri);
}

static jsi_JmpPopInfo *jpinfo_new(int off, int topop)
{
    jsi_JmpPopInfo *r = (jsi_JmpPopInfo *)Jsi_Calloc(1, sizeof(*r));
    r->off = off;
    r->topop = topop;
    return r;
}

static void code_reserved_replace(Jsi_OpCodes *ops, int step_len, int break_only,
                           const char *desire_label, int topop)
{
    int i;
    for (i = 0; i < ops->code_len; ++i) {
        if (ops->codes[i].op != OP_RESERVED) continue;
        jsi_ReservedInfo *ri = (jsi_ReservedInfo *)ops->codes[i].data;

        if (ri->label) {
            if (!desire_label || Jsi_Strcmp(ri->label, desire_label) != 0) {
                ri->topop += topop;
                continue;
            }
        }
        
        if (ri->type == RES_CONTINUE) {
            if (break_only) {
                ri->topop += topop;
                continue;
            } else {
                int topop = ri->topop;
                Jsi_Free(ri);       /* kill reserved Warn, replace with other opcode */
 /*               if (ops->codes[i].data && ops->codes[i].alloc) //TODO: memory leak?
                    Jsi_Free(ops->codes[i].data);*/
                if (topop) {
                    ops->codes[i].data = jpinfo_new(ops->code_len - i, topop);
                    ops->codes[i].op = OP_JMPPOP;
                    ops->codes[i].alloc = 1;
                } else {
                    ops->codes[i].data = (void *)(uintptr_t)(ops->code_len - i);
                    ops->codes[i].op = OP_JMP;
                    ops->codes[i].alloc = 0;
                }
            }
        } else if (ri->type == RES_BREAK) {
            int topop = ri->topop;
            Jsi_Free(ri);
/*           if (ops->codes[i].data && ops->codes[i].alloc)
                Jsi_Free(ops->codes[i].data); */
            if (topop) {
                ops->codes[i].data = jpinfo_new(step_len + ops->code_len - i, topop);
                ops->codes[i].op = OP_JMPPOP;
                ops->codes[i].alloc = 1;
            } else {
                ops->codes[i].data = (void *)(uintptr_t)(step_len + ops->code_len - i);
                ops->codes[i].op = OP_JMP;
                ops->codes[i].alloc = 0;
            }
        }
    }
}

const char* jsi_opcode_string(uint opCode)
{
    if (opCode >= (sizeof(jsi_op_names)/sizeof(jsi_op_names[0])))
        return "NULL";
    return jsi_op_names[opCode];
}

void jsi_code_decode(Jsi_Interp *interp, jsi_OpCode *op, int currentip, char *buf, int bsiz)
{
    if (_JSICASTINT(op->op) < 0 || op->op >= OP_LASTOP) {
        snprintf(buf, bsiz, "Bad opcode[%d] at %d", op->op, currentip);
    }
    char nbuf[100];
    snprintf(nbuf, sizeof(nbuf), "%d#%d", currentip, op->Line);
    snprintf(buf, bsiz, "%-8s %s ", nbuf, jsi_op_names[op->op]);

    int sl = Jsi_Strlen(buf);
    char *bp = buf + sl;
    bsiz -= sl;
    if (op->op == OP_PUSHBOO || op->op == OP_FCALL || op->op == OP_EVAL ||
        op->op == OP_POP || op->op == OP_ASSIGN ||
        op->op == OP_RET || op->op == OP_NEWFCALL ||
        op->op == OP_DELETE || op->op == OP_CHTHIS ||
        op->op == OP_OBJECT || op->op == OP_ARRAY ||
        op->op == OP_SHF ||
        op->op == OP_INC || op->op == OP_DEC) snprintf(bp, bsiz, "%" PRId64, (Jsi_Wide)(uintptr_t)op->data);
    else if (op->op == OP_PUSHNUM) Jsi_NumberDtoA(interp, *((Jsi_Number *)op->data), bp, bsiz, 0);
    else if (op->op == OP_PUSHVSTR) {
        Jsi_String *ss = (Jsi_String*)op->data;
        snprintf(bp, bsiz, "\"%s\"", ss->str);
    } else if (op->op == OP_PUSHSTR || op->op == OP_LOCAL ||
             op->op == OP_SCATCH) snprintf(bp, bsiz, "\"%s\"", op->data ? (char*)op->data:"(NoCatch)");
    else if (op->op == OP_PUSHVAR) snprintf(bp, bsiz, "var: \"%s\"", ((jsi_FastVar *)op->data)->varname);
    else if (op->op == OP_PUSHFUN) snprintf(bp, bsiz, "func: 0x%" PRIx64, (Jsi_Wide)(uintptr_t)op->data);
    else if (op->op == OP_JTRUE || op->op == OP_JFALSE ||
             op->op == OP_JTRUE_NP || op->op == OP_JFALSE_NP ||
             op->op == OP_JMP) snprintf(bp, bsiz, "{%" PRIu64 "}\t#%" PRIu64 "", (Jsi_Wide)(uintptr_t)op->data, (Jsi_Wide)((uintptr_t)currentip + (uintptr_t)op->data));
    else if (op->op == OP_JMPPOP) {
        jsi_JmpPopInfo *jp = (jsi_JmpPopInfo*)op->data;
        snprintf(bp, bsiz, "{%d},%d\t#%d", jp->off, jp->topop, currentip + jp->off);
    }
    else if (op->op == OP_STRY) {
        jsi_TryInfo *t = (jsi_TryInfo *)op->data;
        snprintf(bp, bsiz, "{try:%d, catch:%d, final:%d}", t->trylen, t->catchlen, t->finallen);
    }
}

void jsi_mark_local(Jsi_OpCodes *ops) // Mark variables as declared with "var"
{
    return;
    int i = 0;
    if (ops == NULL || ops->codes == NULL)
        return;
    while (i < ops->code_len) {
        if (ops->codes[i].op == OP_PUSHVAR)
            ops->codes[i].local = 1;
        i++;
    }
}

static jsi_ForinVar *forinvar_new(jsi_Pstate *pstate, const char *varname, Jsi_OpCodes *local, Jsi_OpCodes *lval)
{
    jsi_ForinVar *r = (jsi_ForinVar*)Jsi_Calloc(1,sizeof(*r));
    r->sig = JSI_SIG_FORINVAR;
    r->varname = varname;
    r->local = local;
    r->lval = lval;
    return r;
}

static Jsi_OpCodes *make_forin(Jsi_OpCodes *lval, jsi_Pline *line, Jsi_OpCodes *expr, Jsi_OpCodes *stat, const char *label, int isof)
{
    Jsi_OpCodes *keycode = code_key();
    keycode->codes[0].isof = isof;
    keycode->codes[0].Line = line->first_line;
    Jsi_OpCodes *init = codes_join(expr, keycode);
    Jsi_OpCodes *cond = codes_join3(lval, code_next(),
                                   code_jfalse(stat->code_len + 2));
    Jsi_OpCodes *stat_jmp = code_jmp(-(cond->code_len + stat->code_len));
    code_reserved_replace(stat, 1, 0, label, 2);
    return codes_join3(codes_join(init, cond), 
                          codes_join(stat, stat_jmp), code_pop(2));
}

static jsi_CaseExprStat *exprstat_new(jsi_Pstate *pstate, Jsi_OpCodes *expr, Jsi_OpCodes *stat, int isdef)
{
    jsi_CaseExprStat *r = (jsi_CaseExprStat*)Jsi_Calloc(1,sizeof(*r));
    r->sig = JSI_SIG_CASESTAT;
    r->expr = expr;
    r->stat = stat;
    r->isdefault = isdef;
    return r;
}

static jsi_CaseList *caselist_new(jsi_Pstate *pstate, jsi_CaseExprStat *es)
{
    jsi_CaseList *a = (jsi_CaseList*)Jsi_Calloc(1,sizeof(*a));
    a->sig = JSI_SIG_CASELIST;
    a->es = es;
    a->tail = a;
    return a;
}

static jsi_CaseList *caselist_insert(jsi_Pstate *pstate, jsi_CaseList *a, jsi_CaseExprStat *es)
{
    jsi_CaseList *b = (jsi_CaseList*)Jsi_Calloc(1,sizeof(*b));
    a->sig = JSI_SIG_CASELIST;
    b->es = es;
    a->tail->next = b;
    a->tail = b;
    return a;
}

static void caselist_free(jsi_CaseList *c)
{
    jsi_CaseList *a = c;
    while (a) {
        a = c->next;
        if (c->es) Jsi_Free(c->es);
        Jsi_Free(c);
        c = a;
    }
}

static Jsi_OpCodes *opassign(jsi_Pstate *pstate, jsi_Pline *line, Jsi_OpCodes *lval, Jsi_OpCodes *oprand, Jsi_OpCodes *op)
{
    Jsi_OpCodes *ret;
    if ((lval)->lvalue_flag == 1) {
        ret = codes_join3(lval, 
                             codes_join3(code_push_top(), oprand, op),
                             code_assign(pstate, line, 1));
    } else {
        ret = codes_join3(lval,
                             codes_join4(code_push_top2(), code_subscript(pstate, line, 1), oprand, op),
                             code_assign(pstate, line, 2));
    }
    return ret;
}

#endif
#endif
