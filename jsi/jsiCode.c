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

static const char *op_names[OP_LASTOP] = {
    "NOP",
    "PUSHNUM",
    "PUSHSTR",
    "PUSHVAR",
    "PUSHUND",
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
    "DEC",
    "IN",
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

void jsi_FreeOpcodes(OpCodes *ops) {
    int i;
    for (i=0; i<ops->code_len; i++) {
        OpCode *op = ops->codes+i;
        if (op->data && op->alloc)
            Jsi_Free(op->data);
        MEMCLEAR(op);
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

static OpCodes *codes_new(int size)
{
    OpCodes *ops = (OpCodes *)Jsi_Calloc(1, sizeof(*ops));
    jsiOpCodesCnt[0]++;
    jsiOpCodesCnt[2]++;
    ops->codes = (OpCode *)Jsi_Calloc(size, sizeof(OpCode));
    ops->code_size = size;
#ifdef JSI_MEM_DEBUG
    static int idNum = 0;
    ops->hPtr = Jsi_HashSet(jsiMainInterp->codesTbl, ops, ops);
    ops->id = idNum++;
#endif
    return ops;
}

static int codes_insert(OpCodes *c, Eopcode code, void *extra, int doalloc)
{
    if (c->code_size - c->code_len <= 0) {
        c->code_size += 100;
        c->codes = (OpCode *)Jsi_Realloc(c->codes, c->code_size * sizeof(OpCode));
    }
    c->codes[c->code_len].op = code;
    c->codes[c->code_len].data = extra;
    c->codes[c->code_len].alloc = doalloc;
    c->code_len ++;
    return 0;
}

static int codes_insertln(OpCodes *c, Eopcode code, void *extra, jsi_Pstate *pstate, jsi_Pline *line, int doalloc)
{
    if (c->code_size - c->code_len <= 0) {
        c->code_size += 100;
        c->codes = (OpCode *)Jsi_Realloc(c->codes, c->code_size * sizeof(OpCode));
    }
    c->codes[c->code_len].op = code;
    c->codes[c->code_len].data = extra;
    c->codes[c->code_len].line = (code == OP_FCALL ? line->first_line:line->first_line);
    c->codes[c->code_len].fname = jsi_PstateGetFilename(pstate);
    c->codes[c->code_len].alloc = doalloc;
    c->code_len ++;
    return 0;
}


static OpCodes *codes_join(OpCodes *a, OpCodes *b)
{
    OpCodes *ret = codes_new(a->code_len + b->code_len);
    memcpy(ret->codes, a->codes, a->code_len * sizeof(OpCode));
    memcpy(&ret->codes[a->code_len], b->codes, b->code_len * sizeof(OpCode));
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

static OpCodes *codes_join3(OpCodes *a, OpCodes *b, OpCodes *c)
{
    return codes_join(codes_join(a, b), c);
}

static OpCodes *codes_join4(OpCodes *a, OpCodes *b, OpCodes *c, OpCodes *d)
{
    return codes_join(codes_join(a, b), codes_join(c, d));
}

#define NEW_CODES(doalloc,code, extra) do {                 \
        OpCodes *r = codes_new(3);                  \
        codes_insert(r, (code), (void *)(extra), doalloc);   \
        return r;                                   \
    } while(0)

#define NEW_CODESLN(doalloc,code, extra) do {                 \
        OpCodes *r = codes_new(3);                  \
        codes_insertln(r, (code), (void *)(extra), p, line, doalloc);   \
        return r;                                   \
    } while(0)

static OpCodes *code_push_undef() { NEW_CODES(0,OP_PUSHUND, 0); }
static OpCodes *code_push_bool(int v) { NEW_CODES(0,OP_PUSHBOO, v); }
static OpCodes *code_push_num(Jsi_Number *v) { NEW_CODES(1,OP_PUSHNUM, v); }
static OpCodes *code_push_string(jsi_Pstate *p, jsi_Pline *line, const char *str) {
    if (*str == 'c' && !strcmp(str,"callee"))
        p->interp->hasCallee = 1;
    NEW_CODESLN(0,OP_PUSHSTR, str);
}

static OpCodes *code_push_index(jsi_Pstate *p, jsi_Pline *line, char *varname)
{
    FastVar *n = (FastVar*)Jsi_Calloc(1,sizeof(*n)); /* TODO: free when opcodes are freed. */
    n->sig = JSI_SIG_FASTVAR;
    n->ps = p;
    n->context_id = -1;
    n->var.varname = (char*)Jsi_KeyAdd(p->interp, varname);
    Jsi_HashSet(p->fastVarTbl, n, n);
    NEW_CODESLN(1,OP_PUSHVAR, n);
}

static OpCodes *code_push_this(jsi_Pstate *p, jsi_Pline *line) { NEW_CODESLN(0,OP_PUSHTHS, 0); }
static OpCodes *code_push_top() { NEW_CODES(0,OP_PUSHTOP, 0); }
static OpCodes *code_push_top2() { NEW_CODES(0,OP_PUSHTOP2, 0); }
static OpCodes *code_unref() { NEW_CODES(0,OP_UNREF, 0); }
static OpCodes *code_push_args() { NEW_CODES(0,OP_PUSHARG, 0); }
static OpCodes *code_push_func(jsi_Pstate *p, jsi_Pline *line, Jsi_Func *fun) { p->funcDefs++; NEW_CODESLN(0,OP_PUSHFUN, fun); }
static OpCodes *code_push_regex(jsi_Pstate *p, jsi_Pline *line, Jsi_Regex *reg) { NEW_CODESLN(0,OP_PUSHREG, reg); }

static OpCodes *code_local(jsi_Pstate *p, jsi_Pline *line, const char *varname) { NEW_CODESLN(0,OP_LOCAL, varname); }

static OpCodes *code_nop() { NEW_CODES(0,OP_NOP, 0); }
static OpCodes *code_neg() { NEW_CODES(0,OP_NEG, 0); }
static OpCodes *code_pos() { NEW_CODES(0,OP_POS, 0); }
static OpCodes *code_bnot() { NEW_CODES(0,OP_BNOT, 0); }
static OpCodes *code_not() { NEW_CODES(0,OP_NOT, 0); }
static OpCodes *code_mul() { NEW_CODES(0,OP_MUL, 0); }
static OpCodes *code_div() { NEW_CODES(0,OP_DIV, 0); }
static OpCodes *code_mod() { NEW_CODES(0,OP_MOD, 0); }
static OpCodes *code_add() { NEW_CODES(0,OP_ADD, 0); }
static OpCodes *code_sub() { NEW_CODES(0,OP_SUB, 0); }
static OpCodes *code_in() { NEW_CODES(0,OP_IN, 0); }
static OpCodes *code_less() { NEW_CODES(0,OP_LESS, 0); }
static OpCodes *code_greater() { NEW_CODES(0,OP_GREATER, 0); }
static OpCodes *code_lessequ() { NEW_CODES(0,OP_LESSEQU, 0); }
static OpCodes *code_greaterequ() { NEW_CODES(0,OP_GREATEREQU, 0); }
static OpCodes *code_equal() { NEW_CODES(0,OP_EQUAL, 0); } 
static OpCodes *code_notequal() { NEW_CODES(0,OP_NOTEQUAL, 0); }
static OpCodes *code_eequ() { NEW_CODES(0,OP_STRICTEQU, 0); }
static OpCodes *code_nneq() { NEW_CODES(0,OP_STRICTNEQ, 0); }
static OpCodes *code_band() { NEW_CODES(0,OP_BAND, 0); }
static OpCodes *code_bor() { NEW_CODES(0,OP_BOR, 0); }
static OpCodes *code_bxor() { NEW_CODES(0,OP_BXOR, 0); }
static OpCodes *code_shf(int right) { NEW_CODES(0,OP_SHF, right); }
static OpCodes *code_instanceof() { NEW_CODES(0,OP_INSTANCEOF, 0); }
static OpCodes *code_assign(jsi_Pstate *p, jsi_Pline *line, int h) { NEW_CODESLN(0,OP_ASSIGN, h); }
static OpCodes *code_subscript(jsi_Pstate *p, jsi_Pline *line, int right_val) { NEW_CODESLN(0,OP_SUBSCRIPT, right_val); }
static OpCodes *code_inc(jsi_Pstate *p, jsi_Pline *line, int e) { NEW_CODESLN(0,OP_INC, e); }
static OpCodes *code_dec(jsi_Pstate *p, jsi_Pline *line, int e) { NEW_CODESLN(0,OP_DEC, e); }
static OpCodes *code_typeof(jsi_Pstate *p, jsi_Pline *line, int e) { NEW_CODESLN(0,OP_TYPEOF, e); }

static OpCodes *code_fcall(jsi_Pstate *p, jsi_Pline *line, int argc, const char *name, const char *namePre) { jsi_FuncCallCheck(p,line,argc,1, name, namePre);NEW_CODESLN(0,OP_FCALL, argc); }
static OpCodes *code_newfcall(jsi_Pstate *p, jsi_Pline *line, int argc, const char *name) { jsi_FuncCallCheck(p,line,argc,1, name, NULL); NEW_CODESLN(0,OP_NEWFCALL, argc); }
static OpCodes *code_ret(jsi_Pstate *p, jsi_Pline *line, int n) { NEW_CODESLN(0,OP_RET, n); }
static OpCodes *code_delete(int n) { NEW_CODES(0,OP_DELETE, n); }
static OpCodes *code_chthis(int n) { NEW_CODES(0,OP_CHTHIS, n); }
static OpCodes *code_pop(int n) { NEW_CODES(0,OP_POP, n); }
static OpCodes *code_jfalse(int off) { NEW_CODES(0,OP_JFALSE, off); }
static OpCodes *code_jtrue(int off) { NEW_CODES(0,OP_JTRUE, off); }
static OpCodes *code_jfalse_np(int off) { NEW_CODES(0,OP_JFALSE_NP, off); }
static OpCodes *code_jtrue_np(int off) { NEW_CODES(0,OP_JTRUE_NP, off); }
static OpCodes *code_jmp(int off) { NEW_CODES(0,OP_JMP, off); }
static OpCodes *code_object(jsi_Pstate *p, jsi_Pline *line, int c) { NEW_CODESLN(0,OP_OBJECT, c); }
static OpCodes *code_array(jsi_Pstate *p, jsi_Pline *line, int c) { NEW_CODESLN(0,OP_ARRAY, c); }
static OpCodes *code_key() { NEW_CODES(0,OP_KEY, 0); }
static OpCodes *code_next() { NEW_CODES(0,OP_NEXT, 0); }

static OpCodes *code_eval(jsi_Pstate *p, jsi_Pline *line, int argc, OpCodes *c) {
    jsi_FreeOpcodes(c); // Eliminate leak of unused opcodes.
    NEW_CODESLN(0,OP_EVAL, argc);
}

static OpCodes *code_stry(jsi_Pstate *p, jsi_Pline *line, int trylen, int catchlen, int finlen)
{ 
    TryInfo *ti = (TryInfo *)Jsi_Calloc(1,sizeof(*ti));
    ti->trylen = trylen;
    ti->catchlen = catchlen;
    ti->finallen = finlen;
    NEW_CODESLN(1,OP_STRY, ti); 
}
static OpCodes *code_etry(jsi_Pstate *p, jsi_Pline *line) { NEW_CODESLN(0,OP_ETRY, 0); }
static OpCodes *code_scatch(jsi_Pstate *p, jsi_Pline *line, const char *var) { NEW_CODESLN(0,OP_SCATCH, var); }
static OpCodes *code_ecatch(jsi_Pstate *p, jsi_Pline *line) { NEW_CODESLN(0,OP_ECATCH, 0); }
static OpCodes *code_sfinal(jsi_Pstate *p, jsi_Pline *line) { NEW_CODESLN(0,OP_SFINAL, 0); }
static OpCodes *code_efinal(jsi_Pstate *p, jsi_Pline *line) { NEW_CODESLN(0,OP_EFINAL, 0); }
static OpCodes *code_throw(jsi_Pstate *p, jsi_Pline *line) { NEW_CODESLN(0,OP_THROW, 0); }
static OpCodes *code_with(jsi_Pstate *p, jsi_Pline *line, int withlen) { NEW_CODESLN(0,OP_WITH, withlen); }
static OpCodes *code_ewith(jsi_Pstate *p, jsi_Pline *line) { NEW_CODESLN(0,OP_EWITH, 0); }

static OpCodes *code_debug(jsi_Pstate *p, jsi_Pline *line) { NEW_CODESLN(0,OP_DEBUG, 0); }
static OpCodes *code_reserved(jsi_Pstate *p, jsi_Pline *line, int type, char *id)
{
    ReservedInfo *ri = (ReservedInfo*)Jsi_Calloc(1, sizeof(*ri));
    ri->type = type;
    ri->label = id;
    ri->topop = 0;
    NEW_CODESLN(1,OP_RESERVED, ri);
}

static JmpPopInfo *jpinfo_new(int off, int topop)
{
    JmpPopInfo *r = (JmpPopInfo *)Jsi_Calloc(1, sizeof(*r));
    r->off = off;
    r->topop = topop;
    return r;
}

static void code_reserved_replace(OpCodes *ops, int step_len, int break_only,
                           const char *desire_label, int topop)
{
    int i;
    for (i = 0; i < ops->code_len; ++i) {
        if (ops->codes[i].op != OP_RESERVED) continue;
        ReservedInfo *ri = (ReservedInfo *)ops->codes[i].data;

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
                    ops->codes[i].data = (void *)(ops->code_len - i);
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
                ops->codes[i].data = (void *)(step_len + ops->code_len - i);
                ops->codes[i].op = OP_JMP;
                ops->codes[i].alloc = 0;
            }
        }
    }
}

void jsi_code_decode(OpCode *op, int currentip, char *buf, int bsiz)
{
    if (op->op < 0 || op->op >= OP_LASTOP) {
        snprintf(buf, bsiz, "Bad opcode[%d] at %d", op->op, currentip);
    }
    char nbuf[100];
    sprintf(nbuf, "%d#%d", currentip, op->line);
    snprintf(buf, bsiz, "%-8s %s ", nbuf, op_names[op->op]);

    int sl = strlen(buf);
    char *bp = buf + sl;
    bsiz -= sl;
    if (op->op == OP_PUSHBOO || op->op == OP_FCALL || op->op == OP_EVAL ||
        op->op == OP_POP || op->op == OP_ASSIGN ||
        op->op == OP_RET || op->op == OP_NEWFCALL ||
        op->op == OP_DELETE || op->op == OP_CHTHIS ||
        op->op == OP_OBJECT || op->op == OP_ARRAY ||
        op->op == OP_SHF ||
        op->op == OP_INC || op->op == OP_DEC) snprintf(bp, bsiz, "%d", (int)op->data);
    else if (op->op == OP_PUSHNUM) snprintf(bp, bsiz, "%" JSI_NUMGFMT "", *((Jsi_Number *)op->data));
    else if (op->op == OP_PUSHSTR || op->op == OP_LOCAL ||
             op->op == OP_SCATCH) snprintf(bp, bsiz, "\"%s\"", op->data ? (char*)op->data:"(NoCatch)");
    else if (op->op == OP_PUSHVAR) snprintf(bp, bsiz, "var: \"%s\"", ((FastVar *)op->data)->var.varname);
    else if (op->op == OP_PUSHFUN) snprintf(bp, bsiz, "func: 0x%x", (int)op->data);
    else if (op->op == OP_JTRUE || op->op == OP_JFALSE ||
             op->op == OP_JTRUE_NP || op->op == OP_JFALSE_NP ||
             op->op == OP_JMP) snprintf(bp, bsiz, "{%d}\t#%d", (int)op->data, currentip + (int)op->data);
    else if (op->op == OP_JMPPOP) {
        JmpPopInfo *jp = (JmpPopInfo*)op->data;
        snprintf(bp, bsiz, "{%d},%d\t#%d", jp->off, jp->topop, currentip + jp->off);
    }
    else if (op->op == OP_STRY) {
        TryInfo *t = (TryInfo *)op->data;
        snprintf(bp, bsiz, "{try:%d, catch:%d, final:%d}", t->trylen, t->catchlen, t->finallen);
    }
}

/*
void jsi_codes_print(OpCodes *ops)
{
    int i = 0;
    OpCode *opcodes = ops->codes;
    int opcodesi = ops->code_len;
    
    fprintf(stderr, "opcodes count = %d\n", opcodesi);
    
    while(i < opcodesi) {
        jsi_code_decode(&opcodes[i], i);
        i++;
    }
}*/
#endif
#endif
