%{
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#include "jsiCode.c"
#endif

typedef struct ForinVar {
    jsi_Sig sig;
    char *varname;
    OpCodes *local;
    OpCodes *lval;
} ForinVar;

static ForinVar *forinvar_new(jsi_Pstate *pstate, char *varname, OpCodes *local, OpCodes *lval)
{
    ForinVar *r = (ForinVar*)Jsi_Calloc(1,sizeof(*r));
    r->sig = JSI_SIG_FORINVAR;
    r->varname = varname;
    r->local = local;
    r->lval = lval;
    return r;
}

static OpCodes *make_forin(OpCodes *lval, OpCodes *expr, OpCodes *stat, const char *label)
{
    OpCodes *init = codes_join(expr, code_key());
    OpCodes *cond = codes_join3(lval, code_next(),
                                   code_jfalse(stat->code_len + 2));
    OpCodes *stat_jmp = code_jmp(-(cond->code_len + stat->code_len));
    code_reserved_replace(stat, 1, 0, label, 2);
    return codes_join3(codes_join(init, cond), 
                          codes_join(stat, stat_jmp), code_pop(2));
}

typedef struct CaseExprStat {
    jsi_Sig sig;
    OpCodes *expr;
    OpCodes *stat;
    int isdefault;
} CaseExprStat;

static CaseExprStat *exprstat_new(jsi_Pstate *pstate, OpCodes *expr, OpCodes *stat, int isdef)
{
    CaseExprStat *r = (CaseExprStat*)Jsi_Calloc(1,sizeof(*r));
    r->sig = JSI_SIG_CASESTAT;
    r->expr = expr;
    r->stat = stat;
    r->isdefault = isdef;
    return r;
}

typedef struct CaseList {
    jsi_Sig sig;
    CaseExprStat *es;
    int off;
    struct CaseList *tail;
    struct CaseList *next;
} CaseList;

static CaseList *caselist_new(jsi_Pstate *pstate, CaseExprStat *es)
{
    CaseList *a = (CaseList*)Jsi_Calloc(1,sizeof(*a));
    a->sig = JSI_SIG_CASELIST;
    a->es = es;
    a->tail = a;
    return a;
}

static CaseList *caselist_insert(jsi_Pstate *pstate, CaseList *a, CaseExprStat *es)
{
    CaseList *b = (CaseList*)Jsi_Calloc(1,sizeof(*b));
    a->sig = JSI_SIG_CASELIST;
    b->es = es;
    a->tail->next = b;
    a->tail = b;
    return a;
}

static OpCodes *opassign(jsi_Pstate *pstate, jsi_Pline *line, OpCodes *lval, OpCodes *oprand, OpCodes *op)
{
    OpCodes *ret;
    if (((OpCodes *)lval)->lvalue_flag == 1) {
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

%}

%locations          /* location proccess */
%pure-parser        /* re-entence */
%parse-param    {jsi_Pstate *pstate}
%lex-param      {jsi_Pstate *pstate}
%error-verbose
%expect 7 /*6*/          /* if-else shift/reduce
                       lvalue shift/reduce 
                       ',' shift/reduce
                       empty statement '{''}' empty object shift/reduct */

%token STRING
%token IDENTIFIER
%token IF
%token ELSE
%token FOR
%token IN
%token WHILE
%token DO
%token CONTINUE
%token SWITCH
%token CASE
%token DEFAULT
%token BREAK
%token FUNC
%token RETURN
%token LOCAL
%token NEW
%token DELETE
%token TRY
%token CATCH
%token FINALLY
%token THROW
%token WITH
%token UNDEF
%token _TRUE
%token _FALSE
%token _THIS
%token ARGUMENTS
%token FNUMBER
%token REGEXP
%token TYPESTRING
%token TYPENUMBER
%token TYPENULL
%token TYPEOBJECT
%token TYPEBOOLEAN
%token TYPEUSEROBJ
%token TYPEREGEXP
%token TYPEANY


%token TYPEARRAY
%token ELLIPSIS
%token __DEBUG

%left MIN_PRI
%left ','
%left ARGCOMMA                      /* comma in argument list */
%right '=' ADDAS MNSAS MULAS MODAS LSHFAS RSHFAS URSHFAS BANDAS BORAS BXORAS DIVAS
/*           +=    -=    *=    %=   <<=     >>=   >>>=     &=     |=    ^=    /= */
%left '?' ':'
%left OR                            /* || */
%left AND                           /* && */
%left '|'                           /* | */
%left '^'                           /* ^ */
%left '&'                           /* & */
%left EQU NEQ EEQU NNEQ             /* == != === !== */
%left '>' '<' LEQ GEQ INSTANCEOF    /* <= >= instanceof */
%left LSHF RSHF URSHF               /* << >> >>> */
%left '+' '-'
%left '*' '/' '%'
%left NEG '!' INC DEC '~' TYPEOF VOID   /* - ++ -- typeof */
%left NEW                               /* new */
%left '.' '[' '('
%left MAX_PRI
%right IN

%%

file:   { pstate->opcodes = code_nop(); }
    | statements {
        pstate->opcodes = (OpCodes*)$1;
    }
    | statements expr {
        pstate->opcodes = codes_join3((OpCodes*)$1, (OpCodes*)$2, code_ret(pstate, &@1, 1));
    }
    | expr {    /* for json */
        pstate->opcodes = codes_join((OpCodes*)$1, code_ret(pstate, (jsi_Pline *)&@1, 1));
    }
;

statements: statement       { $$ = $1; }
    | statements statement  { $$ = codes_join((OpCodes*)$1, (OpCodes*)$2); }
;

/* Todo, ';' auto gen */
statement: 
    iterstatement       { $$ = $1; }
    | commonstatement    { $$ = $1; }
    | IDENTIFIER ':' commonstatement { $$ = $3; }
;

commonstatement:
    expr ';' { $$ = codes_join((OpCodes*)$1, code_pop(1)); }
    | if_statement  { $$ = $1; }
    | delete_statement  { $$ = $1; }
    | BREAK identifier_opt ';'      { $$ = code_reserved(pstate, (jsi_Pline *)&@2, RES_BREAK, (char*)$2); }
    | CONTINUE identifier_opt ';'   { $$ = code_reserved(pstate, (jsi_Pline *)&@2, RES_CONTINUE, (char*)$2); }
    | RETURN expr ';'   { $$ = codes_join((OpCodes*)$2, code_ret(pstate, (jsi_Pline *)&@2, 1)); }
    | RETURN ';'        { $$ = code_ret(pstate, (jsi_Pline *)&@1, 0); }
    | LOCAL vardecs ';' { $$ = $2; }
    | THROW expr ';'    { $$ = codes_join((OpCodes*)$2, code_throw(pstate, (jsi_Pline *)&@2)); }
    | try_statement     { $$ = $1; }
    | with_statement    { $$ = $1; }
    | ';'                   { $$ = code_nop(); }
    | '{' statements '}'    { $$ = $2; }
    | func_statement        { $$ = $1; }
;

func_statement:
    func_prefix '(' args_opt ')' ':' argtype func_statement_block {
        OpCodes *ret = codes_join4(code_push_index(pstate, (jsi_Pline *)&@1, (char*)$1),
          code_push_func(pstate, &@3, jsi_FuncMake(pstate, (Jsi_ScopeStrs*)$3, (OpCodes*)$7, (jsi_Pline *)&@7, (char*)$1)),
          code_assign(pstate, (jsi_Pline *)&@1, 1), code_pop(1));
        if (pstate->eval_flag) ret = codes_join(code_local(pstate, (jsi_Pline *)&@1, (const char*)$1), ret);
        jsi_PstatePop(pstate);
        $$ = ret;
    }
    | func_prefix '(' args_opt ')' func_statement_block {
        OpCodes *ret = codes_join4(code_push_index(pstate, (jsi_Pline *)&@1, (char*)$1),
          code_push_func(pstate, (jsi_Pline *)&@3, jsi_FuncMake(pstate, (Jsi_ScopeStrs*)$3, (OpCodes*)$5, (jsi_Pline *)&@5, (char*)$1)),
          code_assign(pstate, (jsi_Pline *)&@1, 1), code_pop(1));
        if (pstate->eval_flag) ret = codes_join(code_local(pstate, (jsi_Pline *)&@1, (char*)$1), ret);
        jsi_PstatePop(pstate);
        $$ = ret;
    }
;

func_prefix:
    FUNC IDENTIFIER %prec MAX_PRI {
        if (!pstate->eval_flag) {
            jsi_PstateAddVar(pstate, (const char*)$2);
        }
        $$ = $2;
    }
;

iterstatement:
    for_statement   { $$ = $1; }
    | while_statement   { $$ = $1; }
    | do_statement      { $$ = $1; }
    | switch_statement  { $$ = $1; }
;

identifier_opt: { $$ = NULL; }
    | IDENTIFIER { $$ = $1; }
;

label_opt: { $$ = NULL; }
    | IDENTIFIER ':' {
        $$ = $1;
    }
;

statement_or_empty:
    statement   { $$ = $1; }
    | '{' '}'   { $$ = code_nop(); }
;

with_statement:
    WITH '(' expr ')' statement_or_empty { 
        $$ = codes_join4((OpCodes*)$3, code_with(pstate, (jsi_Pline *)&@3, ((OpCodes *)$5)->code_len + 1), (OpCodes*)$5, code_ewith(pstate, (jsi_Pline *)&@5));
    }
;

switch_statement: 
    label_opt SWITCH '(' expr ')' '{' '}' { $$ = codes_join((OpCodes*)$4, code_pop(1)); }
    | label_opt SWITCH '(' expr ')' '{' cases '}'   {
        CaseList *cl = (CaseList*)$7;
        OpCodes *allstats = codes_new(3);
        CaseList *cldefault = NULL;
        CaseList *head = NULL;
        
        while (cl) {
            cl->off = allstats->code_len;
            allstats = codes_join(allstats, cl->es->stat);

            CaseList *t = cl;
            cl = cl->next;
            
            if (t->es->isdefault) {
                if (cldefault) yyerror((jsi_Pline *)&@8, pstate, "More then one switch default\n");
                cldefault = t;
            } else {
                t->next = head;
                head = t;
            }
        }
        code_reserved_replace(allstats, 0, 1, (const char*)$1, 1);
        
        OpCodes *ophead = code_jmp(allstats->code_len + 1);
        if (cldefault) {
            ophead = codes_join(code_jmp(ophead->code_len + cldefault->off + 1), ophead);
            if (cldefault->es)
                Jsi_Free(cldefault->es);
            Jsi_Free(cldefault);
        }
        while (head) {
            OpCodes *e = codes_join4(code_push_top(), head->es->expr, 
                                        code_equal(), code_jtrue(ophead->code_len + head->off + 1));
            ophead = codes_join(e, ophead);
            CaseList *t = head;
            head = head->next;
            if (t->es)
                Jsi_Free(t->es);
            Jsi_Free(t);
        }
        $$ = codes_join4(codes_join((OpCodes*)$4, code_unref()), ophead, allstats, code_pop(1));
    }
;

cases:
    case            { $$ = caselist_new(pstate,(CaseExprStat*)$1); }
    | cases case    { $$ = caselist_insert(pstate,(CaseList*)$1, (CaseExprStat*)$2); }
;

case:
    CASE expr ':' statements    { $$ = exprstat_new(pstate, (OpCodes*)$2, (OpCodes*)$4, 0); }
    | DEFAULT ':' statements    { $$ = exprstat_new(pstate, NULL, (OpCodes*)$3, 1); }
    | CASE expr ':'    { $$ = exprstat_new(pstate, (OpCodes*)$2, code_nop(), 0); }
;

try_statement:
    TRY func_statement_block CATCH '(' IDENTIFIER ')' func_statement_block {
        OpCodes *catchblock = codes_join3(code_scatch(pstate, (jsi_Pline *)&@5, (const char*)$5), (OpCodes*)$7, code_ecatch(pstate, (jsi_Pline *)&@7));
        OpCodes *finallyblock = codes_join(code_sfinal(pstate, (jsi_Pline *)&@5), code_efinal(pstate, (jsi_Pline *)&@5));
        OpCodes *tryblock = codes_join((OpCodes*)$2, code_etry(pstate, (jsi_Pline *)&@5));
        $$ = codes_join4(code_stry(pstate, &@5, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
    | TRY func_statement_block FINALLY func_statement_block {
        OpCodes *catchblock = codes_join(code_scatch(pstate, (jsi_Pline *)&@1, NULL), code_ecatch(pstate, (jsi_Pline *)&@1));
        OpCodes *finallyblock = codes_join3(code_sfinal(pstate, (jsi_Pline *)&@1), (OpCodes*)$4, code_efinal(pstate, (jsi_Pline *)&@4));
        OpCodes *tryblock = codes_join((OpCodes*)$2, code_etry(pstate, (jsi_Pline *)&@2));
        $$ = codes_join4(code_stry(pstate, (jsi_Pline *)&@1, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
    | TRY func_statement_block CATCH '(' IDENTIFIER ')' func_statement_block 
        FINALLY func_statement_block {
        OpCodes *catchblock = codes_join3(code_scatch(pstate, (jsi_Pline *)&@5, (const char*)$5), (OpCodes*)$7, code_ecatch(pstate, (jsi_Pline *)&@7));
        OpCodes *finallyblock = codes_join3(code_sfinal(pstate, (jsi_Pline *)&@1), (OpCodes*)$9, code_efinal(pstate, (jsi_Pline *)&@1));
        OpCodes *tryblock = codes_join((OpCodes*)$2, code_etry(pstate, (jsi_Pline *)&@2));
        $$ = codes_join4(code_stry(pstate, (jsi_Pline *)&@1, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
;
vardecs:
    vardec                  { $$ = $1; }
    | vardecs ',' vardec    { $$ = codes_join((OpCodes*)$1, (OpCodes*)$3); }
;

vardec:
    IDENTIFIER              {
        OpCodes *ret = codes_join4(code_push_index(pstate, (jsi_Pline *)&@1, (char*)$1),
                            code_push_undef(),
                            code_assign(pstate, (jsi_Pline *)&@1, 1),
                            code_pop(1));
        if (!pstate->eval_flag) jsi_PstateAddVar(pstate, (const char*)$1);
        else ret = codes_join(code_local(pstate, (jsi_Pline *)&@1, (const char*)$1), ret);
        $$ = ret;
    }
    | IDENTIFIER '=' expr   {
        OpCodes *ret = codes_join4(code_push_index(pstate, &@1, (char*)$1),
                            (OpCodes*)$3,
                            code_assign(pstate, (jsi_Pline *)&@1, 1),
                            code_pop(1));
        if (!pstate->eval_flag) jsi_PstateAddVar(pstate, (const char*)$1);
        else ret = codes_join(code_local(pstate, (jsi_Pline *)&@1, (const char*)$1), ret);
        $$ = ret;
    }
;

delete_statement:
    DELETE lvalue ';'           {
        if (((OpCodes *)$2)->lvalue_flag == 2) {
            $$ = codes_join((OpCodes*)$2, code_delete(2));
        } else {
            $$ = codes_join((OpCodes*)$2, code_delete(1));
        }
    }
;

if_statement:
    IF '(' expr ')' statement_or_empty {
        int offset = ((OpCodes *)$5)->code_len;
        $$ = codes_join3((OpCodes*)$3, code_jfalse(offset + 1), (OpCodes*)$5);
    }
    | IF '(' expr ')' statement_or_empty ELSE statement_or_empty {
        int len_block2 = ((OpCodes *)$7)->code_len;
        OpCodes *block1 = codes_join((OpCodes*)$5, code_jmp(len_block2 + 1));
        OpCodes *condi = codes_join((OpCodes*)$3, code_jfalse(block1->code_len + 1));
        $$ = codes_join3(condi, block1, (OpCodes*)$7);
    }
;

for_statement:
    label_opt FOR '(' for_init for_cond ';' expr_opt ')' statement_or_empty {
        OpCodes *init = (OpCodes*)$4;
        OpCodes *cond = (OpCodes*)$5;
        OpCodes *step = ($7 ? codes_join((OpCodes*)$7, code_pop(1)) : code_nop());
        OpCodes *stat = (OpCodes*)$9;
        OpCodes *cont_jmp = code_jfalse(step->code_len + stat->code_len + 2);
        OpCodes *step_jmp = code_jmp(-(cond->code_len + step->code_len + stat->code_len + 1));
        code_reserved_replace(stat, step->code_len + 1, 0, (const char*)$1, 0);
        $$ = codes_join(codes_join3(init, cond, cont_jmp),
                           codes_join3(stat, step, step_jmp));
    }
    | label_opt FOR '(' LOCAL IDENTIFIER IN expr ')' statement_or_empty {
        ForinVar *fv;
        fv = forinvar_new(pstate, (char*)$5, code_local(pstate, (jsi_Pline *)&@5, (const char*)$5), NULL);
        OpCodes *lval;
        if (fv->varname) lval = code_push_index(pstate, (jsi_Pline *)&@1, fv->varname);
        else lval = fv->lval;
        
        OpCodes *ret = make_forin(lval, (OpCodes*)$7, (OpCodes*)$9, (const char*)$1);
        if (fv->varname && fv->local) {
            if (!pstate->eval_flag) {
                jsi_PstateAddVar(pstate,fv->varname);
                jsi_FreeOpcodes(fv->local);
            } else ret = codes_join(fv->local, ret);
        }
        Jsi_Free(fv);
        $$ = ret;
    }
    | label_opt FOR '(' lvalue IN expr ')' statement_or_empty {
        ForinVar *fv;
        if (((OpCodes *)$4)->lvalue_flag == 2) 
            fv = forinvar_new(pstate,NULL, NULL, codes_join((OpCodes*)$4, code_subscript(pstate, (jsi_Pline *)&@4, 0)));
        else fv = forinvar_new(pstate, NULL, NULL, (OpCodes*)$4);
        OpCodes *lval;
        if (fv->varname) lval = code_push_index(pstate, (jsi_Pline *)&@1, fv->varname);
        else lval = fv->lval;
        
        OpCodes *ret = make_forin(lval, (OpCodes*)$6, (OpCodes*)$8, (char*)$1);
        if (fv->varname && fv->local) {
            if (!pstate->eval_flag) {
                jsi_PstateAddVar(pstate,fv->varname);
                jsi_FreeOpcodes(fv->local);
            } else ret = codes_join(fv->local, ret);
        }
        Jsi_Free(fv);
        $$ = ret;
    }
;

/*for_var:
    | LOCAL IDENTIFIER {
        $$ = forinvar_new((OpCodes*)$2, code_local(pstate, (jsi_Pline *)&@2, (OpCodes*)$2), NULL);
    }
    | lvalue {
        if (((OpCodes *)$1)->lvalue_flag == 2) 
            $$ = forinvar_new(NULL, NULL, codes_join((OpCodes*)$1, code_subscript(pstate, (jsi_Pline *)&@1, 0)));
        else $$ = forinvar_new(NULL, NULL, (OpCodes*)$1);
    }
;*/
 
for_init:
    ';'                 { $$ = code_nop(); }
    | expr ';'          { $$ = codes_join((OpCodes*)$1, code_pop(1)); }
    | LOCAL vardecs ';' { $$ = $2; }
;

for_cond:               { $$ = code_push_bool(1); }
    | expr              { $$ = $1; }
;

expr_opt:               { $$ = NULL; }
    | expr              { $$ = $1; }
;

while_statement:
    label_opt WHILE '(' expr ')' statement_or_empty {
        OpCodes *cond = (OpCodes*)$4;
        OpCodes *stat = (OpCodes*)$6;
        code_reserved_replace(stat, 1, 0, (const char*)$1, 0);
        $$ = codes_join4(cond, code_jfalse(stat->code_len + 2), stat,
                           code_jmp(-(stat->code_len + cond->code_len + 1)));
    }
;

do_statement:
    label_opt DO statement_or_empty WHILE '(' expr ')' {
        OpCodes *stat = (OpCodes*)$3;
        OpCodes *cond = (OpCodes*)$6;
        code_reserved_replace(stat, cond->code_len + 1, 0, (const char*)$1, 0);
        $$ = codes_join3(stat, cond,
                            code_jtrue(-(stat->code_len + cond->code_len)));
    }
;

func_expr:
    FUNC '(' args_opt ')' func_statement_block {
        $$ = code_push_func(pstate,  (jsi_Pline *)&@3, jsi_FuncMake(pstate, (Jsi_ScopeStrs*)$3, (OpCodes*)$5, (jsi_Pline *)&@5, NULL));
        jsi_PstatePop(pstate);
    }
    | FUNC '(' args_opt ')' ':' argtype func_statement_block {
        $$ = code_push_func(pstate,  (jsi_Pline *)&@3, jsi_FuncMake(pstate, (Jsi_ScopeStrs*)$3, (OpCodes*)$7, (jsi_Pline *)&@7, NULL));
        jsi_PstatePop(pstate);
    }
    | FUNC IDENTIFIER '(' args_opt ')' func_statement_block {
        $$ = code_push_func(pstate, (jsi_Pline *)&@3, jsi_FuncMake(pstate, (Jsi_ScopeStrs*)$4, (OpCodes*)$6, (jsi_Pline *)&@6, (char*)$2));
        jsi_PstatePop(pstate);
    }
    | FUNC IDENTIFIER '(' args_opt ')' ':' argtype func_statement_block {
        $$ = code_push_func(pstate, (jsi_Pline *)&@3, jsi_FuncMake(pstate, (Jsi_ScopeStrs*)$4, (OpCodes*)$8, (jsi_Pline *)&@8, (char*)$2));
        jsi_PstatePop(pstate);
    }
;

args_opt: { jsi_PstatePush(pstate); $$ = jsi_ScopeStrsNew(pstate); }
    | args {
        $$ = jsi_ArgsOptAdd(pstate, (Jsi_ScopeStrs*)$1);
    }
    | args ',' '.' '.' '.' {
        Jsi_ScopeStrs *s = jsi_ArgsOptAdd(pstate, (Jsi_ScopeStrs*)$1);
        s->varargs = 1;
        $$ = s;
    }
;

typeid:
    TYPESTRING {    $$ = $1; pstate->argType |= JSI_TT_STRING; }
    | TYPENUMBER {  $$ = $1; pstate->argType |= JSI_TT_NUMBER; }
    | TYPEBOOLEAN { $$ = $1; pstate->argType |= JSI_TT_BOOL; }
    | TYPEREGEXP {  $$ = $1; pstate->argType |= JSI_TT_REGEXP; }
    | TYPEOBJECT {  $$ = $1; pstate->argType |= JSI_TT_OBJECT; }
    | TYPEUSEROBJ { $$ = $1; pstate->argType |= JSI_TT_USEROBJ; }
    | TYPEANY {     $$ = $1; pstate->argType |= JSI_TT_ANY; }
    | TYPEARRAY {   $$ = $1; pstate->argType |= JSI_TT_ARRAY; }
    | FUNC {        $$ = $1; pstate->argType |= JSI_TT_FUNCTION; }
    | VOID {        $$ = $1; pstate->argType |= JSI_TT_VOID; }
    
argtype:
    typeid { $$ = $1; }
    | argtype '|' typeid {
        $$ = $1;
    }

argdefault:
    VOID {      $$ = Jsi_ValueNew(pstate->interp); }
    | FNUMBER { $$ = Jsi_ValueNewNumber(pstate->interp, *(Jsi_Number*)$1); }
    | _TRUE {   $$ = Jsi_ValueNewBoolean(pstate->interp, 1); }
    | _FALSE {  $$ = Jsi_ValueNewBoolean(pstate->interp, 0); }
    | TYPENULL {$$ = Jsi_ValueNewNull(pstate->interp); }
    | STRING {  $$ = Jsi_ValueNewStringKey(pstate->interp, (char*)$1); }
    
args:
    IDENTIFIER  { $$ = jsi_argInsert(pstate, NULL, (const char*)$1, NULL); }
    | IDENTIFIER '=' argdefault { $$ = jsi_argInsert(pstate, NULL, (const char*)$1, (Jsi_Value*)$3); }
    | IDENTIFIER ':' argtype { $$ = jsi_argInsert(pstate, NULL, (const char*)$1, NULL);}
    | IDENTIFIER ':' argtype '=' argdefault { $$ = jsi_argInsert(pstate, NULL, (const char*)$1, (Jsi_Value*)$5);}
    | args ',' IDENTIFIER { $$ = jsi_argInsert(pstate, (Jsi_ScopeStrs*)$1, (const char*)$3, NULL); }
    | args ',' IDENTIFIER '=' argdefault { $$ = jsi_argInsert(pstate, (Jsi_ScopeStrs*)$1, (const char*)$3, (Jsi_Value*)$5); }
    | args ',' IDENTIFIER ':' argtype { $$ = jsi_argInsert(pstate, (Jsi_ScopeStrs*)$1, (const char*)$3, NULL);}
    | args ',' IDENTIFIER ':' argtype '=' argdefault { $$ = jsi_argInsert(pstate, (Jsi_ScopeStrs*)$1, (const char*)$3, (Jsi_Value*)$7);}
;

func_statement_block: '{' statements '}'    { $$ = $2; }
    | '{' '}'                               { $$ = code_nop(); }
;

expr:
    value                   { $$ = $1; }
    | func_expr             { $$ = $1; }
    | lvalue                { 
        if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join((OpCodes*)$1, code_subscript(pstate, (jsi_Pline *)&@1, 1)); 
        else $$ = $1;
    }
    | expr ',' expr         { $$ = codes_join3((OpCodes*)$1, code_pop(1), (OpCodes*)$3); }
    | expr '[' expr ']'     { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_subscript(pstate, (jsi_Pline *)&@1, 1)); }
    | expr '.' IDENTIFIER   { $$ = codes_join3((OpCodes*)$1, code_push_string(pstate,(jsi_Pline *)&@3,(const char*)$3), code_subscript(pstate, (jsi_Pline *)&@3, 1)); }
    | '-' expr %prec NEG    { $$ = codes_join((OpCodes*)$2, code_neg()); }
    | '+' expr %prec NEG    { $$ = codes_join((OpCodes*)$2, code_pos()); }
    | '~' expr              { $$ = codes_join((OpCodes*)$2, code_bnot()); }
    | '!' expr              { $$ = codes_join((OpCodes*)$2, code_not()); }
    | VOID expr             { $$ = codes_join3((OpCodes*)$2, code_pop(1), code_push_undef()); }
    | expr '*' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_mul()); }
    | expr '/' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_div()); }
    | expr '%' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_mod()); }
    | expr '+' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_add()); }
    | expr '-' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_sub()); }
    | expr IN expr          { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_in()); }
    | lvalue INC            {
        if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join3((OpCodes*)$1, code_subscript(pstate, (jsi_Pline *)&@1, 0), code_inc(pstate, (jsi_Pline *)&@1, 1));
        else $$ = codes_join((OpCodes*)$1, code_inc(pstate, (jsi_Pline *)&@1, 1));
    }
    | lvalue DEC            { 
        if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join3((OpCodes*)$1, code_subscript(pstate, (jsi_Pline *)&@1, 0), code_dec(pstate, (jsi_Pline *)&@1, 1));
        else $$ = codes_join((OpCodes*)$1, code_dec(pstate, (jsi_Pline *)&@1, 1)); 
    }
    | INC lvalue            {
        if (((OpCodes *)$2)->lvalue_flag == 2) $$ = codes_join3((OpCodes*)$2, code_subscript(pstate, (jsi_Pline *)&@2, 0), code_inc(pstate, (jsi_Pline *)&@2, 0));
        else $$ = codes_join((OpCodes*)$2, code_inc(pstate, (jsi_Pline *)&@2, 0));
    }
    | TYPEOF expr {
        if (((OpCodes *)$2)->lvalue_flag == 2) $$ = codes_join3((OpCodes*)$2, code_subscript(pstate, (jsi_Pline *)&@2, 0), code_typeof(pstate, (jsi_Pline *)&@2, 0));
        else $$ = codes_join((OpCodes*)$2, code_typeof(pstate, (jsi_Pline *)&@2, 0));
    }
    | DEC lvalue            { 
        if (((OpCodes *)$2)->lvalue_flag == 2) $$ = codes_join3((OpCodes*)$2, code_subscript(pstate, (jsi_Pline *)&@2, 0), code_dec(pstate, (jsi_Pline *)&@2, 0));
        else $$ = codes_join((OpCodes*)$2, code_dec(pstate, (jsi_Pline *)&@2, 0));
    }
    | '(' expr ')'          { $$ = $2; }
    | expr AND expr         {
        OpCodes *expr2 = codes_join(code_pop(1), (OpCodes*)$3);
        $$ = codes_join3((OpCodes*)$1, code_jfalse_np(expr2->code_len + 1), expr2);
    }
    | expr OR expr          {
        OpCodes *expr2 = codes_join(code_pop(1), (OpCodes*)$3);
        $$ = codes_join3((OpCodes*)$1, code_jtrue_np(expr2->code_len + 1), expr2);
    }
    | expr '<' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_less()); }
    | expr '>' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_greater()); }
    | expr LEQ expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_lessequ()); }
    | expr GEQ expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_greaterequ()); }
    | expr EQU expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_equal()); }
    | expr NEQ expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_notequal()); }
    | expr EEQU expr        { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_eequ());    }
    | expr NNEQ expr        { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_nneq()); }
    | expr '&' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_band()); }
    | expr '|' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_bor()); }
    | expr '^' expr         { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_bxor()); }
    | expr LSHF expr        { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_shf(0)); }
    | expr RSHF expr        { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_shf(1)); }
    | expr URSHF expr       { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_shf(2)); }
    | lvalue '=' expr       { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_assign(pstate, (jsi_Pline *)&@1, ((OpCodes *)$1)->lvalue_flag)); }
    | lvalue ADDAS expr     { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_add()); }
    | lvalue MNSAS expr     { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_sub()); }
    | lvalue MULAS expr     { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_mul()); }
    | lvalue MODAS expr     { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_mod()); }
    | lvalue LSHFAS expr    { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_shf(0)); }
    | lvalue RSHFAS expr    { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_shf(1)); }
    | lvalue URSHFAS expr   { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_shf(2)); }
    | lvalue BANDAS expr    { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_band()); }
    | lvalue BORAS expr     { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_bor()); }
    | lvalue BXORAS expr    { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_bxor()); }
    | lvalue DIVAS expr     { $$ = opassign(pstate, (jsi_Pline *)&@1, (OpCodes*)$1, (OpCodes*)$3, code_div()); }
    | lvalue INSTANCEOF expr  { $$ = codes_join3((OpCodes*)$1, (OpCodes*)$3, code_instanceof()); }
    | fcall_exprs           { $$ = $1; }
    
    | NEW value             { $$ = codes_join((OpCodes*)$2, code_newfcall(pstate, (jsi_Pline *)&@2, 0, NULL)); }
    | NEW lvalue            { 
        if (((OpCodes *)$2)->lvalue_flag == 2) $$ = codes_join3((OpCodes*)$2, code_subscript(pstate, (jsi_Pline *)&@2, 1), code_newfcall(pstate, (jsi_Pline *)&@2, 0, NULL));
        else $$ = codes_join((OpCodes*)$2, code_newfcall(pstate, (jsi_Pline *)&@2, 0, NULL));}
    | NEW '(' expr ')'      { $$ = codes_join((OpCodes*)$3, code_newfcall(pstate, (jsi_Pline *)&@3,0, NULL)); }
    | NEW func_expr         { $$ = codes_join((OpCodes*)$2, code_newfcall(pstate, (jsi_Pline *)&@2,0, NULL)); }
    | NEW value '(' exprlist_opt ')'        {
        OpCodes *lval = (OpCodes*)$1;
        OpCodes *opl = (OpCodes*)$4;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3((OpCodes*)$2, (opl ? opl : code_nop()), code_newfcall(pstate, (jsi_Pline *)&@2, expr_cnt, lval->lvalue_name));
    }
    | NEW lvalue '(' exprlist_opt ')'       {
        OpCodes *opl = (OpCodes*)$4;
        int expr_cnt = opl ? opl->expr_counter:0;
        OpCodes *lv = NULL;
        if (((OpCodes *)$2)->lvalue_flag == 2) lv = codes_join((OpCodes*)$2, code_subscript(pstate, (jsi_Pline *)&@2, 1));
        else lv = (OpCodes *)$2;
        $$ = codes_join3(lv, (opl ? opl : code_nop()), code_newfcall(pstate, (jsi_Pline *)&@1,expr_cnt, lv?lv->lvalue_name:NULL));
    }
    | NEW '(' expr ')' '(' exprlist_opt ')' { 
        OpCodes *opl = (OpCodes *)$6;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3((OpCodes*)$3, (opl ? opl : code_nop()), code_newfcall(pstate, (jsi_Pline *)&@1,expr_cnt, NULL));
    }
    | NEW func_expr '(' exprlist_opt ')'    {
        OpCodes *opl = (OpCodes*)$4;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3((OpCodes*)$2, (opl ? opl : code_nop()), code_newfcall(pstate, (jsi_Pline *)&@2,expr_cnt, NULL));
    }
    | expr '?' expr ':' expr {
        OpCodes *expr2 = codes_join((OpCodes*)$3, code_jmp(((OpCodes *)$5)->code_len + 1));
        $$ = codes_join4((OpCodes*)$1, code_jfalse(expr2->code_len + 1), expr2, (OpCodes*)$5);
    }
    | __DEBUG '(' expr ')' { $$ = codes_join((OpCodes*)$3, code_debug(pstate,(jsi_Pline *)&@3)); }
;

fcall_exprs:
    expr '.' IDENTIFIER '(' exprlist_opt ')' {
        OpCodes *lval = (OpCodes*)$1;
        OpCodes *lval2 = (OpCodes*)$3;
        OpCodes *ff = codes_join4((OpCodes*)$1, code_push_string(pstate,(jsi_Pline *)&@3, (const char*)$3), code_chthis(1), code_subscript(pstate, (jsi_Pline *)&@1, 1));
        OpCodes *opl = (OpCodes*)$5;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3(ff, (opl ? opl : code_nop()), code_fcall(pstate, (jsi_Pline *)&@3, expr_cnt, lval->lvalue_name, lval2->lvalue_name));
    }
    | expr '[' expr ']' '(' exprlist_opt ')' {
        OpCodes *ff = codes_join4((OpCodes*)$1, (OpCodes*)$3, code_chthis(1), code_subscript(pstate, (jsi_Pline *)&@1, 1));
        OpCodes *opl = (OpCodes*)$6;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3(ff, (opl ? opl : code_nop()), code_fcall(pstate, (jsi_Pline *)&@3, expr_cnt, NULL, NULL));
    }
    | '(' expr ')' '(' exprlist_opt ')' {
        OpCodes *opl = (OpCodes*)$5;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join4((OpCodes*)$2, code_chthis(0), (opl ? opl : code_nop()), code_fcall(pstate, (jsi_Pline *)&@3,expr_cnt, NULL, NULL));
    }
    | lvalue '(' exprlist_opt ')' {
        OpCodes *opl = (OpCodes*)$3;
        int expr_cnt = opl ? opl->expr_counter:0;
        OpCodes *pref;
        OpCodes *lval = (OpCodes*)$1;
        if (lval->lvalue_flag == 2) {
            pref = codes_join3((OpCodes*)$1, code_chthis(1), code_subscript(pstate, (jsi_Pline *)&@1, 1));
            const char *n1 = lval->lvalue_name, *n2 = NULL;
            if (pref->code_len>=2 && pref->codes[0].op == OP_PUSHVAR && pref->codes[1].op == OP_PUSHSTR && !n1) {
                FastVar *fv = (FastVar*)pref->codes[0].data;
                n2 = fv->var.varname;
                n1 = (char*)pref->codes[1].data;
            }
            $$ = codes_join3(pref, (opl ? opl : code_nop()), code_fcall(pstate, (jsi_Pline *)&@1, expr_cnt, n1, n2));
        } else {
            if (lval->lvalue_name && Jsi_Strcmp(lval->lvalue_name, "eval") == 0) {
                $$ = codes_join((opl ? opl : code_nop()), code_eval(pstate, (jsi_Pline *)&@1, expr_cnt, lval));
            } else {
                pref = codes_join((OpCodes*)$1, code_chthis(0));
                $$ = codes_join3(pref, (opl ? opl : code_nop()), code_fcall(pstate, (jsi_Pline *)&@3, expr_cnt, lval->lvalue_name, NULL));
            }
        }
    }
;

lvalue:
    IDENTIFIER              {
        OpCodes *lval = code_push_index(pstate, (jsi_Pline *)&@1, (char*)$1); 
        $$ = lval;
        lval->lvalue_flag = 1; 
        lval->lvalue_name = (char*)$1; 
    }
    | ARGUMENTS             { $$ = code_push_args(); ((OpCodes *)$$)->lvalue_flag = 1; }
    | _THIS                 { $$ = code_push_this(pstate,(jsi_Pline *)&@1); ((OpCodes *)$$)->lvalue_flag = 1; }
    | lvalue '[' expr ']'   {
        if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join3((OpCodes*)$1, code_subscript(pstate, (jsi_Pline *)&@1, 1), (OpCodes *)$3); 
        else $$ = codes_join((OpCodes*)$1, (OpCodes*)$3); 
        ((OpCodes *)$$)->lvalue_flag = 2;
    }
    | lvalue '.' IDENTIFIER {
        if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join3((OpCodes *)$1, code_subscript(pstate, (jsi_Pline *)&@1, 1), code_push_string(pstate,(jsi_Pline *)&@3,(char *)$3)); 
        else $$ = codes_join((OpCodes *)$1, code_push_string(pstate,(jsi_Pline *)&@3,(char *)$3));
        ((OpCodes *)$$)->lvalue_flag = 2;
    }
;

exprlist_opt:   { $$ = NULL; }
    | exprlist  { $$ = $1; }
;

exprlist:
    expr %prec ARGCOMMA { $$ = $1; ((OpCodes *)$$)->expr_counter = 1; }
    | exprlist ',' expr %prec ARGCOMMA { 
        int exprcnt = ((OpCodes *)$1)->expr_counter + 1;
        $$ = codes_join((OpCodes*)$1, (OpCodes*)$3);
        ((OpCodes *)$$)->expr_counter = exprcnt;
    }
;

value: STRING { $$ = code_push_string(pstate,(jsi_Pline *)&@1, (char*)$1); }
    | UNDEF { $$ = code_push_undef(); }
    | _TRUE { $$ = code_push_bool(1); }
    | _FALSE { $$ = code_push_bool(0); }
    | FNUMBER { $$ = code_push_num((Jsi_Number*)$1); }
    | REGEXP { $$ = code_push_regex(pstate, (jsi_Pline *)&@1, (Jsi_Regex*)$1); }
    | object { $$ = $1; }
    | array { $$ = $1; }
;

object:
    '{' items '}'   { $$ = codes_join((OpCodes *)$2, code_object(pstate, (jsi_Pline *)&@2, ((OpCodes *)$2)->expr_counter)); }

;

items:      { $$ = code_nop(); ((OpCodes *)$$)->expr_counter = 0; }
    | item  { $$ = $1; ((OpCodes *)$$)->expr_counter = 1; }
    | items ',' item {
        int cnt = ((OpCodes *)$1)->expr_counter + 1;
        $$ = codes_join((OpCodes*)$1, (OpCodes*)$3);
        ((OpCodes *)$$)->expr_counter = cnt;
    }
;

item:
    IDENTIFIER ':' expr { $$ = codes_join(code_push_string(pstate,(jsi_Pline *)&@1, (const char*)$1), (OpCodes*)$3); }
    | STRING ':' expr   { $$ = codes_join(code_push_string(pstate,(jsi_Pline *)&@1, (const char*)$1), (OpCodes*)$3); }
;

array:
    '[' exprlist ']' { $$ = codes_join((OpCodes*)$2, code_array(pstate, (jsi_Pline *)&@2, ((OpCodes *)$2)->expr_counter)); }
    | '[' ']' { $$ = code_array(pstate, (jsi_Pline *)&@1, 0); }
;


%%

