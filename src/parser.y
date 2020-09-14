%{
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#include "jsiCode.c"
#endif
%}

%locations          /* location proccess */
%pure-parser        /* re-entence */
%parse-param    {struct jsi_Pstate *pstate}
%lex-param      {struct jsi_Pstate *pstate}
%error-verbose
%expect 9 /*6*/          /* if-else shift/reduce
                       lvalue shift/reduce 
                       ',' shift/reduce
                       empty statement '{''}' empty object shift/reduct */

%union {
    Jsi_OpCodes *opcodes;
    Jsi_Value *value;
    const char *sstr;
    Jsi_String *vstr;
    Jsi_Regex* regex;
    Jsi_Number *num;
    Jsi_ScopeStrs *scopes;
    int inum;
    struct jsi_CaseExprStat* caseitem;
    struct jsi_CaseList* caselist;
}


%destructor { Jsi_ValueFree(pstate->interp, $$); } <value>
%destructor { Jsi_Free($$); } <num>
%destructor { jsi_ScopeStrsFree(pstate->interp, $$); } <scopes>
%destructor { jsi_FreeOpcodes($$); } <opcodes>
%destructor { Jsi_Free($$); } <caseitem>
%destructor { caselist_free($$);} <caselist>
//%destructor { Jsi_RegExpFree($$); } <regex>
/*
%destructor { } <inum>
%destructor { } <str>
*/

%type <opcodes> array commonstatement delete_statement do_statement expr expr_opt exprlist exprlist_opt itemident itemfunc
%type <opcodes> fcall_exprs for_cond for_init for_statement func_expr func_statement func_statement_block if_statement item items iterstatement lvalue
%type <opcodes> object objectident statement statements statement_or_empty switch_statement try_statement value vardec vardecs while_statement with_statement
%type <scopes> args args_opt argsa arrowargs
%type <inum> typeid inof rettype argtype localvar
%type <sstr> identifier_opt label_opt func_prefix
%type <vstr> strlit
%type <caseitem> case
%type <caselist> cases
%type <value> argdefault

%token <sstr> IDENTIFIER
%token <vstr> STRING
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
%token LOCALCONST
%token LOCALLET
%token OF
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
%token <num> FNUMBER
%token <regex> REGEXP
%token TYPESTRING
%token TYPENUMBER
%token TYPENULL
%token TYPEOBJECT
%token TYPEBOOLEAN
%token TYPEUSEROBJ
%token TYPEITEROBJ
%token TYPEREGEXP
%token TYPEANY
%token TYPEARRAY
%token ELLIPSIS
%token EXPORT
%token ARROW
%token __DEBUG

%left MIN_PRI
%left ','
%left ARGCOMMA                      /* comma in argument list */
%right '=' ADDAS MNSAS MULAS MODAS LSHFAS RSHFAS URSHFAS BANDAS BORAS BXORAS DIVAS
/*           +=    -=    *=    %=   <<=     >>=   >>>=     &=     |=    ^=    /= */
%right '?' ':'
%left OR                            /* || */
%left AND                           /* && */
%left '|'                           /* | */
%left '^'                           /* ^ */
%left '&'                           /* & */
%left EQU NEQ EEQU NNEQ             /* == != === !== */
%left '>' '<' LEQ GEQ INSTANCEOF IN /* <= >= instanceof in */
%left LSHF RSHF URSHF               /* << >> >>> */
%left '+' '-'
%left '*' '/' '%'
%right NEG '!' INC DEC '~' TYPEOF VOID   /* - ++ -- typeof */
%right NEW                               /* new */
%left '.' '[' '('
%left MAX_PRI

%%

file:   { pstate->opcodes = code_nop(); }
    | statements {
        pstate->opcodes = $1;
    }
    | statements expr {
        pstate->opcodes = codes_join3($1, $2, code_ret(pstate, &@1, 1));
    }
    | expr {    /* for json */
        pstate->opcodes = codes_join($1, code_ret(pstate, &@1, 1));
    }
;

statements: statement       { $$ = $1; }
    | statements statement  { $$ = codes_join($1, $2); }
;

/* Todo, ';' auto gen */
statement: 
    iterstatement       { $$ = $1; }
    | commonstatement    { $$ = $1; }
    | IDENTIFIER ':' commonstatement { $$ = $3; }
;

localvar:
    LOCAL { $$ = LOCAL; }
    | LOCALLET { $$ = LOCAL; }
    | LOCALCONST { $$ = LOCAL; }
;
    
objectident:
    object { $$ = $1; }
/*    | IDENTIFIER {
        Jsi_OpCodes *lval = code_push_index(pstate, &@1, $1, 0); 
        $$ = lval;
        lval->lvalue_flag = 1; 
        lval->lvalue_name = $1; 
    }*/
    | '*' {
        $$ = code_push_null();
    }
    | '+' {
        $$ = code_push_undef();
    }
;

commonstatement:
    expr ';' { $$ = codes_join($1, code_pop(1)); }
    | if_statement  { $$ = $1; }
    | delete_statement  { $$ = $1; }
    | BREAK identifier_opt ';'      { $$ = code_reserved(pstate, &@2, RES_BREAK, $2); }
    | CONTINUE identifier_opt ';'   { $$ = code_reserved(pstate, &@2, RES_CONTINUE, $2); }
    | RETURN expr ';'   { $$ = codes_join($2, code_ret(pstate, &@2, 1)); }
    | RETURN ';'        { $$ = code_ret(pstate, &@1, 0); }
    | localvar vardecs ';' {
        jsi_mark_local($2);
        $$ = $2;
    }
    | THROW expr ';'    { $$ = codes_join($2, code_throw(pstate, &@2)); }
    | try_statement     { $$ = $1; }
    | with_statement    { $$ = $1; }
    | ';'                   { $$ = code_nop(); }
    | '{' statements '}'    { $$ = $2; }
    | func_statement        { $$ = $1; }
    | EXPORT DEFAULT objectident { $$ = codes_join($3, code_export(pstate, &@3, 1)); }
;

func_statement:
    func_prefix '(' args_opt ')' ':' rettype func_statement_block {
        $3->retType = $6;
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &@1, $1, 0),
          code_push_func(pstate, &@3, jsi_FuncMake(pstate, $3, $7, &@1, $1, 0)),
          code_assign(pstate, &@1, 1), code_pop(1));
        if (pstate->eval_flag) ret = codes_join(code_local(pstate, &@1, $1), ret);
        jsi_PstatePop(pstate);
        $$ = ret;
    }
    | func_prefix '(' args_opt ')' func_statement_block {
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &@1, $1, 0),
          code_push_func(pstate, &@3, jsi_FuncMake(pstate, $3, $5, &@1, $1, 0)),
          code_assign(pstate, &@1, 1), code_pop(1));
        if (pstate->eval_flag) ret = codes_join(code_local(pstate, &@1, $1), ret);
        jsi_PstatePop(pstate);
        $$ = ret;
    }
;

func_prefix:
    FUNC IDENTIFIER %prec MAX_PRI {
        if (!pstate->eval_flag) {
            jsi_PstateAddVar(pstate, &@2, $2);
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
        $$ = codes_join4($3, code_with(pstate, &@3, ($5)->code_len + 1), $5, code_ewith(pstate, &@5));
    }
;

switch_statement: 
    label_opt SWITCH '(' expr ')' '{' '}' { $$ = codes_join($4, code_pop(1)); }
    | label_opt SWITCH '(' expr ')' '{' cases '}'   {
        jsi_CaseList *cl = $7;
        Jsi_OpCodes *allstats = codes_new(3);
        jsi_CaseList *cldefault = NULL;
        jsi_CaseList *head = NULL;
        
        while (cl) {
            cl->off = allstats->code_len;
            allstats = codes_join(allstats, cl->es->stat);

            jsi_CaseList *t = cl;
            cl = cl->next;
            
            if (t->es->isdefault) {
                if (cldefault) {
                    if (cldefault->es) Jsi_Free(cldefault->es);
                    Jsi_Free(cldefault);
                    yyerror(&@8, pstate, "switch with more then one default\n");
                }
                cldefault = t;
            } else {
                t->next = head;
                head = t;
            }
        }
        code_reserved_replace(allstats, 0, 1, $1, 1);
        
        Jsi_OpCodes *ophead = code_jmp(allstats->code_len + 1);
        if (cldefault) {
            ophead = codes_join(code_jmp(ophead->code_len + cldefault->off + 1), ophead);
            if (cldefault->es)
                Jsi_Free(cldefault->es);
            Jsi_Free(cldefault);
        }
        while (head) {
            Jsi_OpCodes *e = codes_join4(code_push_top(), head->es->expr, 
                                        code_eequ(), code_jtrue(ophead->code_len + head->off + 1));
            ophead = codes_join(e, ophead);
            jsi_CaseList *t = head;
            head = head->next;
            if (t->es)
                Jsi_Free(t->es);
            Jsi_Free(t);
        }
        jsi_OpCode *oc = ophead->codes;
        int i;
        for (i=0; i<ophead->code_len; i++)
            oc[i].nodebug = 1;
        $$ = codes_join4(codes_join($4, code_unref()), ophead, allstats, code_pop(1));
    }
;

cases:
    case            { $$ = caselist_new(pstate, $1); }
    | cases case    { $$ = caselist_insert(pstate, $1, $2); }
;

case:
    CASE expr ':' statements    { $$ = exprstat_new(pstate, $2, $4, 0); }
    | DEFAULT ':' statements    { $$ = exprstat_new(pstate, NULL, $3, 1); }
    | DEFAULT ':'      { $$ = exprstat_new(pstate, NULL, code_nop(), 1); }
    | CASE expr ':'    { $$ = exprstat_new(pstate, $2, code_nop(), 0); }
;

try_statement:
    TRY func_statement_block CATCH '(' IDENTIFIER ')' func_statement_block {
        Jsi_OpCodes *catchblock = codes_join3(code_scatch(pstate, &@5, $5), $7, code_ecatch(pstate, &@7));
        Jsi_OpCodes *finallyblock = codes_join(code_sfinal(pstate, &@5), code_efinal(pstate, &@5));
        Jsi_OpCodes *tryblock = codes_join($2, code_etry(pstate, &@2));
        $$ = codes_join4(code_stry(pstate, &@1, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
    | TRY func_statement_block FINALLY func_statement_block {
        Jsi_OpCodes *catchblock = codes_join(code_scatch(pstate, &@1, NULL), code_ecatch(pstate, &@1));
        Jsi_OpCodes *finallyblock = codes_join3(code_sfinal(pstate, &@1), $4, code_efinal(pstate, &@4));
        Jsi_OpCodes *tryblock = codes_join($2, code_etry(pstate, &@2));
        $$ = codes_join4(code_stry(pstate, &@1, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
    | TRY func_statement_block CATCH '(' IDENTIFIER ')' func_statement_block 
        FINALLY func_statement_block {
        Jsi_OpCodes *catchblock = codes_join3(code_scatch(pstate, &@5, $5), $7, code_ecatch(pstate, &@7));
        Jsi_OpCodes *finallyblock = codes_join3(code_sfinal(pstate, &@1), $9, code_efinal(pstate, &@1));
        Jsi_OpCodes *tryblock = codes_join($2, code_etry(pstate, &@2));
        $$ = codes_join4(code_stry(pstate, &@1, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
;
vardecs:
    vardec                  { $$ = $1; }
    | vardecs ',' vardec    { $$ = codes_join($1, $3); }
;

vardec:
    IDENTIFIER              {
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &@1, $1, 1),
                            code_push_undef(),
                            code_assign(pstate, &@1, 1),
                            code_pop(1));
        if (!pstate->eval_flag) jsi_PstateAddVar(pstate, &@1, $1);
        else ret = codes_join(code_local(pstate, &@1, $1), ret);
        $$ = ret;
    }
    | IDENTIFIER '=' expr   {
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &@1, $1, 1),
                            $3,
                            code_assign(pstate, &@1, 1),
                            code_pop(1));
        if (!pstate->eval_flag) jsi_PstateAddVar(pstate, &@1, $1);
        else ret = codes_join(code_local(pstate, &@1, $1), ret);
        $$ = ret;
    }
;

delete_statement:
    DELETE lvalue ';'           {
        if (($2)->lvalue_flag&2) {
            $$ = codes_join($2, code_delete(2));
        } else {
            $$ = codes_join($2, code_delete(1));
        }
    }
;

if_statement:
    IF '(' expr ')' statement_or_empty {
        int offset = ($5)->code_len;
        $$ = codes_join3($3, code_jfalse(offset + 1), $5);
    }
    | IF '(' expr ')' statement_or_empty ELSE statement_or_empty {
        int len_block2 = ($7)->code_len;
        Jsi_OpCodes *block1 = codes_join($5, code_jmp(len_block2 + 1));
        Jsi_OpCodes *condi = codes_join($3, code_jfalse(block1->code_len + 1));
        $$ = codes_join3(condi, block1, $7);
    }
;

inof:
    IN      { $$ = 0; }
    | OF    { $$ = 1; }
;
    
for_statement:
    label_opt FOR '(' for_init for_cond ';' expr_opt ')' statement_or_empty {
        Jsi_OpCodes *init = $4;
        Jsi_OpCodes *cond = $5;
        Jsi_OpCodes *step = ($7 ? codes_join($7, code_pop(1)) : code_nop());
        Jsi_OpCodes *stat = $9;
        Jsi_OpCodes *cont_jmp = code_jfalse(step->code_len + stat->code_len + 2);
        Jsi_OpCodes *step_jmp = code_jmp(-(cond->code_len + step->code_len + stat->code_len + 1));
        code_reserved_replace(stat, step->code_len + 1, 0, $1, 0);
        $$ = codes_join(codes_join3(init, cond, cont_jmp),
                           codes_join3(stat, step, step_jmp));
    }
    | label_opt FOR '(' localvar IDENTIFIER inof expr ')' statement_or_empty {
        jsi_ForinVar *fv;
        int inof = $6;
        Jsi_OpCodes *loc = code_local(pstate, &@5, $5);
        jsi_mark_local(loc);
        fv = forinvar_new(pstate, $5, loc, NULL);
        Jsi_OpCodes *lval;
        if (fv->varname) lval = code_push_index(pstate, &@2, fv->varname, 1);
        else lval = fv->lval;
        
        Jsi_OpCodes *ret = make_forin(lval, &@2, $7, $9, $1, (inof!=0));
        if (fv->varname && fv->local) {
            if (!pstate->eval_flag) {
                jsi_PstateAddVar(pstate, &@5, fv->varname);
                jsi_FreeOpcodes(fv->local);
            } else ret = codes_join(fv->local, ret);
        }
        Jsi_Free(fv);
        $$ = ret;
    }
    | label_opt FOR '(' lvalue inof expr ')' statement_or_empty {
        jsi_ForinVar *fv;
        int inof = $5;
        if (($4)->lvalue_flag&2) 
            fv = forinvar_new(pstate, NULL, NULL, codes_join($4, code_subscript(pstate, &@4, 0)));
        else fv = forinvar_new(pstate, NULL, NULL, $4);
        Jsi_OpCodes *lval;
        if (fv->varname) lval = code_push_index(pstate, &@1, fv->varname, 0);
        else lval = fv->lval;
        
        Jsi_OpCodes *ret = make_forin(lval, &@2, $6, $8, $1, (inof!=0));
        if (fv->varname && fv->local) {
            if (!pstate->eval_flag) {
                jsi_PstateAddVar(pstate, &@4, fv->varname);
                jsi_FreeOpcodes(fv->local);
            } else ret = codes_join(fv->local, ret);
        }
        Jsi_Free(fv);
        $$ = ret;
    }
;

 
for_init:
    ';'                 { $$ = code_nop(); }
    | expr ';'          { $$ = codes_join($1, code_pop(1)); }
    | localvar vardecs ';' {
        jsi_mark_local($2);
        $$ = $2;
    }
;

for_cond:               { $$ = code_push_bool(1); }
    | expr              { $$ = $1; }
;

expr_opt:               { $$ = NULL; }
    | expr              { $$ = $1; }
;

while_statement:
    label_opt WHILE '(' expr ')' statement_or_empty {
        Jsi_OpCodes *cond = $4;
        Jsi_OpCodes *stat = $6;
        code_reserved_replace(stat, 1, 0, $1, 0);
        $$ = codes_join4(cond, code_jfalse(stat->code_len + 2), stat,
                           code_jmp(-(stat->code_len + cond->code_len + 1)));
    }
;

do_statement:
    label_opt DO statement_or_empty WHILE '(' expr ')' {
        Jsi_OpCodes *stat = $3;
        Jsi_OpCodes *cond = $6;
        code_reserved_replace(stat, cond->code_len + 1, 0, $1, 0);
        $$ = codes_join3(stat, cond,
                            code_jtrue(-(stat->code_len + cond->code_len)));
    }
;

func_expr:
    FUNC '(' args_opt ')' func_statement_block {
        $$ = code_push_func(pstate,  &@3, jsi_FuncMake(pstate, $3, $5, &@1, NULL, 0));
        jsi_PstatePop(pstate);
    }
    | FUNC '(' args_opt ')' ':' rettype func_statement_block {
        $3->retType = $6;
        $$ = code_push_func(pstate,  &@3, jsi_FuncMake(pstate, $3, $7, &@1, NULL, 0));
        jsi_PstatePop(pstate);
    }
    | FUNC IDENTIFIER '(' args_opt ')' func_statement_block {
        $$ = code_push_func(pstate, &@3, jsi_FuncMake(pstate, $4, $6, &@1, $2, 0));
        jsi_PstatePop(pstate);
    }
    | FUNC IDENTIFIER '(' args_opt ')' ':' rettype func_statement_block {
        $4->retType = $7;
        $$ = code_push_func(pstate, &@3, jsi_FuncMake(pstate, $4, $8, &@1, $2, 0));
        jsi_PstatePop(pstate);
    }
;

args_opt: { $$ = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew()); }
    | args {
        $$ = jsi_ArgsOptAdd(pstate, $1);
    }
    | '.' '.' '.' {
        Jsi_ScopeStrs *s = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew());
        s->varargs = 1;
        $$ = s;
    }
    | args ',' '.' '.' '.' {
        Jsi_ScopeStrs *s = jsi_ArgsOptAdd(pstate, $1);
        s->varargs = 1;
        $$ = s;
    }
;

typeid:
    TYPESTRING {    $$ = (pstate->argType |= JSI_TT_STRING); }
    | TYPENUMBER {  $$ = (pstate->argType |= JSI_TT_NUMBER); }
    | TYPEBOOLEAN { $$ = (pstate->argType |= JSI_TT_BOOLEAN); }
    | TYPEREGEXP {  $$ = (pstate->argType |= JSI_TT_REGEXP); }
    | TYPEOBJECT {  $$ = (pstate->argType |= JSI_TT_OBJECT); }
    | TYPEUSEROBJ { $$ = (pstate->argType |= JSI_TT_USEROBJ); }
    | TYPEITEROBJ { $$ = (pstate->argType |= JSI_TT_ITEROBJ); }
    | TYPEANY {     $$ = (pstate->argType |= JSI_TT_ANY); }
    | TYPEARRAY {   $$ = (pstate->argType |= JSI_TT_ARRAY); }
    | TYPENULL {    $$ = (pstate->argType |= JSI_TT_NULL); }
    | FUNC {        $$ = (pstate->argType |= JSI_TT_FUNCTION); }
    | UNDEF {       $$ = (pstate->argType |= JSI_TT_UNDEFINED); }
    | VOID {        $$ = (pstate->argType |= JSI_TT_VOID); }
    
rettype:
    argtype {
        if (pstate->args)
            pstate->args->retType = pstate->argType;
        $$ = pstate->argType;
        pstate->argType = 0;
    }

argtype:
    typeid {
        $$ = pstate->argType;
    }
    | argtype '|' typeid {
        $$ = pstate->argType;
    }

strlit: STRING { $$ = $1; }

argdefault:
    UNDEF {     $$ = Jsi_ValueNew(pstate->interp); $$->d.num = 0; }
    | VOID {    $$ = Jsi_ValueNew(pstate->interp); $$->d.num = 1; }
    | '-' FNUMBER %prec NEG { *$2 = *$2 * -1; $$ = Jsi_ValueNewNumber(pstate->interp, *$2); Jsi_Free($2);}
    | FNUMBER { $$ = Jsi_ValueNewNumber(pstate->interp, *$1); Jsi_Free($1); }
    | _TRUE {   $$ = Jsi_ValueNewBoolean(pstate->interp, 1); }
    | _FALSE {  $$ = Jsi_ValueNewBoolean(pstate->interp, 0); }
    | TYPENULL {$$ = Jsi_ValueNewNull(pstate->interp); }
    | strlit {  $$ = Jsi_ValueNewBlob(pstate->interp, (uchar*)$1->str, $1->len); }
    
args:
    IDENTIFIER  { $$ = jsi_argInsert(pstate, NULL, $1, NULL, &@1, 0 ); }
    | IDENTIFIER '=' argdefault { $$ = jsi_argInsert(pstate, NULL, $1, $3, &@1, 0); }
    | IDENTIFIER ':' argtype { $$ = jsi_argInsert(pstate, NULL, $1, NULL, &@1, 0);}
    | IDENTIFIER ':' argtype '=' argdefault { $$ = jsi_argInsert(pstate, NULL, $1, $5, &@1, 0);}
    | args ',' IDENTIFIER { $$ = jsi_argInsert(pstate, $1, $3, NULL, &@1, 0); }
    | args ',' IDENTIFIER '=' argdefault { $$ = jsi_argInsert(pstate, $1, $3, $5, &@1, 0); }
    | args ',' IDENTIFIER ':' argtype { $$ = jsi_argInsert(pstate, $1, $3, NULL, &@1, 0);}
    | args ',' IDENTIFIER ':' argtype '=' argdefault { $$ = jsi_argInsert(pstate, $1, $3, $7, &@1, 0);}
;

argsa:
    IDENTIFIER              { $$ = jsi_argInsert(pstate, NULL, $1, NULL, &@1, 0); }
    | argsa ',' IDENTIFIER  { $$ = jsi_argInsert(pstate, $1, $3, NULL, &@1, 0); }
;

arrowargs:
    IDENTIFIER                      { $$ = jsi_argInsert(pstate, NULL, $1, NULL, &@1, 0); }
    | '(' IDENTIFIER ',' argsa ')'  { $$ = jsi_argInsert(pstate, $4, $2, NULL, &@1, 1); }
    | '('  ')'            { $$ = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew()); }
;

func_statement_block: '{' statements '}'    { $$ = $2; }
    | '{' '}'                               { $$ = code_nop(); }
;

expr:
    value                   { $$ = $1; }
    | func_expr             { $$ = $1; }
    | lvalue                { 
        if (($1)->lvalue_flag&2) $$ = codes_join($1, code_subscript(pstate, &@1, 1 |(($1)->lvalue_flag&4?2:0))); 
        else $$ = $1;
    }
    | expr ',' expr         { $$ = codes_join3($1, code_pop(1), $3); }
    | expr '[' expr ']'     { $$ = codes_join3($1, $3, code_subscript(pstate, &@1, 1)); }
    | expr '.' IDENTIFIER   { $$ = codes_join3($1, code_push_string(pstate,&@3,$3), code_subscript(pstate, &@3, 3)); }
    | '-' expr %prec NEG    { $$ = codes_join($2, code_neg()); }
    | '+' expr %prec NEG    { $$ = codes_join($2, code_pos()); }
    | '~' expr              { $$ = codes_join($2, code_bnot()); }
    | '!' expr              { $$ = codes_join($2, code_not()); }
    | VOID expr             { $$ = codes_join3($2, code_pop(1), code_push_undef()); }
    | expr '*' expr         { $$ = codes_join3($1, $3, code_mul()); }
    | expr '/' expr         { $$ = codes_join3($1, $3, code_div()); }
    | expr '%' expr         { $$ = codes_join3($1, $3, code_mod()); }
    | expr '+' expr         { $$ = codes_join3($1, $3, code_add()); }
    | expr '-' expr         { $$ = codes_join3($1, $3, code_sub()); }
    | expr IN expr          { $$ = codes_join3($1, $3, code_in()); }
    | lvalue INC            {
        if (($1)->lvalue_flag&2) $$ = codes_join3($1, code_subscript(pstate, &@1, 0), code_inc(pstate, &@1, 1));
        else $$ = codes_join($1, code_inc(pstate, &@1, 1));
    }
    | lvalue DEC            { 
        if (($1)->lvalue_flag&2) $$ = codes_join3($1, code_subscript(pstate, &@1, 0), code_dec(pstate, &@1, 1));
        else $$ = codes_join($1, code_dec(pstate, &@1, 1)); 
    }
    | INC lvalue            {
        if (($2)->lvalue_flag&2) $$ = codes_join3($2, code_subscript(pstate, &@2, 0), code_inc(pstate, &@2, 0));
        else $$ = codes_join($2, code_inc(pstate, &@2, 0));
    }
    | TYPEOF expr {
        if (($2)->lvalue_flag&2) $$ = codes_join3($2, code_subscript(pstate, &@2, 0), code_typeof(pstate, &@2, 0));
        else $$ = codes_join($2, code_typeof(pstate, &@2, 0));
    }
    | DEC lvalue            { 
        if (($2)->lvalue_flag&2) $$ = codes_join3($2, code_subscript(pstate, &@2, 0), code_dec(pstate, &@2, 0));
        else $$ = codes_join($2, code_dec(pstate, &@2, 0));
    }
    | '(' expr ')'          { $$ = $2; }
    | arrowargs ARROW expr %prec MIN_PRI {
        jsi_PstatePush(pstate);
        $$ = code_push_func(pstate,  &@1, jsi_FuncMake(pstate, $1, codes_join($3, code_ret(pstate, &@3, 1)), &@1, NULL, 1)); 
        jsi_PstatePop(pstate);
    }
    | arrowargs ARROW func_statement_block {
        jsi_PstatePush(pstate);
        $$ = code_push_func(pstate,  &@1, jsi_FuncMake(pstate, $1, $3, &@1, NULL, 1));
        jsi_PstatePop(pstate);
    }
    | expr AND expr         {
        Jsi_OpCodes *expr2 = codes_join(code_pop(1), $3);
        $$ = codes_join3($1, code_jfalse_np(expr2->code_len + 1), expr2);
    }
    | expr OR expr          {
        Jsi_OpCodes *expr2 = codes_join(code_pop(1), $3);
        $$ = codes_join3($1, code_jtrue_np(expr2->code_len + 1), expr2);
    }
    | expr '<' expr         { $$ = codes_join3($1, $3, code_less()); }
    | expr '>' expr         { $$ = codes_join3($1, $3, code_greater()); }
    | expr LEQ expr         { $$ = codes_join3($1, $3, code_lessequ()); }
    | expr GEQ expr         { $$ = codes_join3($1, $3, code_greaterequ()); }
    | expr EQU expr         { $$ = codes_join3($1, $3, code_equal()); }
    | expr NEQ expr         { $$ = codes_join3($1, $3, code_notequal()); }
    | expr EEQU expr        { $$ = codes_join3($1, $3, code_eequ());    }
    | expr NNEQ expr        { $$ = codes_join3($1, $3, code_nneq()); }
    | expr '&' expr         { $$ = codes_join3($1, $3, code_band()); }
    | expr '|' expr         { $$ = codes_join3($1, $3, code_bor()); }
    | expr '^' expr         { $$ = codes_join3($1, $3, code_bxor()); }
    | expr LSHF expr        { $$ = codes_join3($1, $3, code_shf(0)); }
    | expr RSHF expr        { $$ = codes_join3($1, $3, code_shf(1)); }
    | expr URSHF expr       { $$ = codes_join3($1, $3, code_shf(2)); }
    | lvalue '=' expr       { $$ = codes_join3($1, $3, code_assign(pstate, &@1, ($1)->lvalue_flag)); }
    | lvalue ADDAS expr     { $$ = opassign(pstate, &@1, $1, $3, code_add()); }
    | lvalue MNSAS expr     { $$ = opassign(pstate, &@1, $1, $3, code_sub()); }
    | lvalue MULAS expr     { $$ = opassign(pstate, &@1, $1, $3, code_mul()); }
    | lvalue MODAS expr     { $$ = opassign(pstate, &@1, $1, $3, code_mod()); }
    | lvalue LSHFAS expr    { $$ = opassign(pstate, &@1, $1, $3, code_shf(0)); }
    | lvalue RSHFAS expr    { $$ = opassign(pstate, &@1, $1, $3, code_shf(1)); }
    | lvalue URSHFAS expr   { $$ = opassign(pstate, &@1, $1, $3, code_shf(2)); }
    | lvalue BANDAS expr    { $$ = opassign(pstate, &@1, $1, $3, code_band()); }
    | lvalue BORAS expr     { $$ = opassign(pstate, &@1, $1, $3, code_bor()); }
    | lvalue BXORAS expr    { $$ = opassign(pstate, &@1, $1, $3, code_bxor()); }
    | lvalue DIVAS expr     { $$ = opassign(pstate, &@1, $1, $3, code_div()); }
    | lvalue INSTANCEOF expr  { $$ = codes_join3($1, $3, code_instanceof()); }
    | fcall_exprs           { $$ = $1; }
    
    | NEW value             { $$ = codes_join($2, code_newfcall(pstate, &@1, 0, NULL, $2)); }
    | NEW lvalue            { 
        if (($2)->lvalue_flag&2) $$ = codes_join3($2, code_subscript(pstate, &@2, 1), code_newfcall(pstate, &@2, 0, NULL, $2));
        else $$ = codes_join($2, code_newfcall(pstate, &@2, 0, NULL, $2));}
    | NEW '(' expr ')'      { $$ = codes_join($3, code_newfcall(pstate, &@1,0, NULL, $3)); }
    | NEW func_expr         { $$ = codes_join($2, code_newfcall(pstate, &@1,0, NULL, $2)); }
    | NEW value '(' exprlist_opt ')'        {
        Jsi_OpCodes *lval = $2;
        Jsi_OpCodes *opl = $4;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3($2, (opl ? opl : code_nop()), code_newfcall(pstate, &@1, expr_cnt, lval->lvalue_name, opl));
    }
    | NEW lvalue '(' exprlist_opt ')'       {
        Jsi_OpCodes *opl = $4;
        int expr_cnt = opl ? opl->expr_counter:0;
        Jsi_OpCodes *lv = NULL;
        if (($2)->lvalue_flag&2) lv = codes_join($2, code_subscript(pstate, &@2, 1));
        else lv = $2;
        $$ = codes_join3(lv, (opl ? opl : code_nop()), code_newfcall(pstate, &@1,expr_cnt, lv?lv->lvalue_name:NULL, opl));
    }
    | NEW '(' expr ')' '(' exprlist_opt ')' { 
        Jsi_OpCodes *opl = $6;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3($3, (opl ? opl : code_nop()), code_newfcall(pstate, &@1,expr_cnt, NULL, opl));
    }
    | NEW func_expr '(' exprlist_opt ')'    {
        Jsi_OpCodes *opl = $4;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3($2, (opl ? opl : code_nop()), code_newfcall(pstate, &@1,expr_cnt, NULL, opl));
    }
    | func_expr '(' exprlist_opt ')'    {
        Jsi_OpCodes *opl = $3;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3($1, (opl ? opl : code_nop()), code_fcall(pstate, &@1,expr_cnt, NULL, NULL, opl, NULL));
    }
    | expr '?' expr ':' expr {
        Jsi_OpCodes *expr2 = codes_join($3, code_jmp(($5)->code_len + 1));
        $$ = codes_join4($1, code_jfalse(expr2->code_len + 1), expr2, $5);
    }
    | __DEBUG { $$ = code_debug(pstate,&@1); }
;

fcall_exprs:
    expr '.' IDENTIFIER '(' exprlist_opt ')' {
        Jsi_OpCodes *lval = $1;
        const char *n1 = lval->lvalue_name;
        const char *n2 = $3;
        Jsi_OpCodes *ff = codes_join4($1, code_push_string(pstate,&@3, $3), code_chthis(pstate,&@1, 1), code_subscript(pstate, &@1, 1));
        Jsi_OpCodes *opl = $5;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3(ff, (opl ? opl : code_nop()), code_fcall(pstate, &@3, expr_cnt, n1, n2, opl, NULL));
    }
    | expr '[' expr ']' '(' exprlist_opt ')' {
        Jsi_OpCodes *ff = codes_join4($1, $3, code_chthis(pstate,&@1, 1), code_subscript(pstate, &@1, 1));
        Jsi_OpCodes *opl = $6;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join3(ff, (opl ? opl : code_nop()), code_fcall(pstate, &@3, expr_cnt, NULL, NULL, opl, NULL));
    }
    | '(' expr ')' '(' exprlist_opt ')' {
        Jsi_OpCodes *opl = $5;
        int expr_cnt = opl ? opl->expr_counter:0;
        $$ = codes_join4($2, code_chthis(pstate,&@1, 0), (opl ? opl : code_nop()), code_fcall(pstate, &@3,expr_cnt, NULL, NULL, opl, NULL));
    }
    | lvalue '(' exprlist_opt ')' {
        Jsi_OpCodes *opl = $3;
        int expr_cnt = opl ? opl->expr_counter:0;
        Jsi_OpCodes *pref;
        Jsi_OpCodes *lval = $1;
        const char *n1 = lval->lvalue_name;
        if (lval->lvalue_flag&2) {
            const char *n2 = NULL;
            pref = codes_join3($1, code_chthis(pstate,&@1, 1), code_subscript(pstate, &@1, 1));
            if (pref->code_len>=2 && pref->codes[0].op == OP_PUSHVAR && pref->codes[1].op == OP_PUSHSTR && !n1) {
                jsi_FastVar *fv = (typeof(fv))pref->codes[0].data;
                n2 = fv->varname;
                n1 = (const char*)pref->codes[1].data;
            }
            $$ = codes_join3(pref, (opl ? opl : code_nop()), code_fcall(pstate, &@2, expr_cnt, n1, n2, opl, NULL));
        } else {
            if (lval->lvalue_name && Jsi_Strcmp(lval->lvalue_name, "eval") == 0) {
                $$ = codes_join((opl ? opl : code_nop()), code_eval(pstate, &@1, expr_cnt, lval));
            } else {
                jsi_Pline *jpl = &@1;
                pref = codes_join($1, code_chthis(pstate,&@1, 0));
                $$ = codes_join3(pref, (opl ? opl : code_nop()), code_fcall(pstate, jpl, expr_cnt, n1, NULL, opl, pref));
            }
        }
    }
;

lvalue:
    IDENTIFIER              {
        Jsi_OpCodes *lval = code_push_index(pstate, &@1, $1, 0); 
        $$ = lval;
        lval->lvalue_flag = 1; 
        lval->lvalue_name = $1; 
    }
    | ARGUMENTS             { $$ = code_push_args(); ($$)->lvalue_flag = 1; }
    | _THIS                 { $$ = code_push_this(pstate,&@1); ($$)->lvalue_flag = 1; }
    | lvalue '[' expr ']'   {
        if (($1)->lvalue_flag&2) $$ = codes_join3($1, code_subscript(pstate, &@1, 1), $3); 
        else $$ = codes_join($1, $3); 
        ($$)->lvalue_flag = 2;
    }
    | lvalue '.' IDENTIFIER {
        if (($1)->lvalue_flag&2) {
            $$ = codes_join3($1, code_subscript(pstate, &@1, 3), code_push_string(pstate,&@3, $3)); 
            ($$)->lvalue_flag = 2;
        } else {
            $$ = codes_join($1, code_push_string(pstate,&@3, $3));
            ($$)->lvalue_flag = 6;
        }
    }
;

exprlist_opt:   { $$ = NULL; }
    | exprlist  { $$ = $1; }
;

exprlist:
    expr %prec ARGCOMMA { $$ = $1; ($$)->expr_counter = 1; }
    | exprlist ',' expr %prec ARGCOMMA { 
        int exprcnt = ($1)->expr_counter + 1;
        $$ = codes_join($1, $3);
        ($$)->expr_counter = exprcnt;
    }
;

value:
    strlit { $$ = code_push_vstring(pstate,&@1, $1); }
    | TYPENULL { $$ = code_push_null(); }
    | UNDEF { $$ = code_push_undef(); }
    | _TRUE { $$ = code_push_bool(1); }
    | _FALSE { $$ = code_push_bool(0); }
    | FNUMBER { $$ = code_push_num($1); }
    | REGEXP { $$ = code_push_regex(pstate, &@1, $1); }
    | object { $$ = $1; }
    | array { $$ = $1; }
;

object:
    '{' items '}'   { $$ = codes_join($2, code_object(pstate, &@2, ($2)->expr_counter)); }
;

itemfunc:
    IDENTIFIER '(' args_opt ')' func_statement_block {
        Jsi_OpCodes *lval = code_push_func(pstate, &@3, jsi_FuncMake(pstate, $3, $5, &@1, $1, 0));
        lval->lvalue_flag = 1; 
        lval->lvalue_name = $1; 
        $$ = codes_join(code_push_string(pstate,&@1, $1), lval);
        jsi_PstatePop(pstate);
    }
;

itemident:
    itemfunc { $$ = $1; }
    | IDENTIFIER  {
        Jsi_OpCodes *lval = code_push_index(pstate, &@1, $1, 0); 
        lval->lvalue_flag = 1; 
        lval->lvalue_name = $1; 
        $$ = codes_join(code_push_string(pstate,&@1, $1), lval);
    }
    | item  { $$ = $1; }

// Note first item still needs a colon eg. {a:a, b, c, d}
items:
    { $$ = code_nop(); ($$)->expr_counter = 0; }
    | item  { $$ = $1; ($$)->expr_counter = 1; }
    | items ',' itemident {
        int cnt = ($1)->expr_counter + 1;
        $$ = codes_join($1, $3);
        ($$)->expr_counter = cnt;
    }
    | items ',' {
        $$ = $1;
    }
    /* | items '.' '.' '.' IDENTIFIER { } //TODO:??? */
;

item:
    IDENTIFIER ':' expr { $$ = codes_join(code_push_string(pstate,&@1, $1), $3); }
    | strlit ':' expr   { $$ = codes_join(code_push_vstring(pstate,&@1, $1), $3); }
    | FNUMBER ':' expr { $$ = codes_join(code_push_num($1), $3);  }
    | _TRUE ':' expr { $$ = codes_join(code_push_bool(1), $3);  }
    | _FALSE ':' expr { $$ = codes_join(code_push_bool(0), $3);  }
    | UNDEF ':' expr { $$ = codes_join(code_push_undef(), $3);  }
    | TYPENULL ':' expr { $$ = codes_join(code_push_null(), $3);  }
;

array:
    '[' exprlist ']' { $$ = codes_join($2, code_array(pstate, &@2, ($2)->expr_counter)); }
    | '[' ']' { $$ = code_array(pstate, &@1, 0); }
;


%%

