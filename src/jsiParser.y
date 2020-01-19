%include {
// The input grammer file for lemon parser generator. (not yet used)
#ifndef JSI_AMALGAMATION
#define JSI_LEMON_PARSE
#include "jsiInt.h"
#include "jsiCode.c"
typedef struct jsi_Token {
    const char *str;
    Jsi_String *vstr;
    Jsi_Number *num;
    Jsi_Regex  *regex;
    int line, col;
    int lvalue_flag;
    jsi_Pline ln;
} jsi_Token;

#endif
}
%extra_argument {struct jsi_Pstate *pstate}
%token_type {jsi_Token}

%left MIN_PRI.
%left COMMA.
%left ARGCOMMA.
%right QUESTION COLON.
%right EQUAL ADDAS MNSAS MULAS MODAS LSHFAS RSHFAS URSHFAS BANDAS BORAS BXORAS DIVAS.
%left OR.
%left AND.
%left VERTBAR.
%left CARET.
%left AMPERSAND.
%left EQU NEQ EEQU NNEQ.
%left LESSTHAN GREATERTHAN LEQ GEQ INSTANCEOF.
%left LSHF RSHF URSHF.
%left PLUS MINUS.
%left TIMES DIVIDE PERCENT.
%right NEG LOGICNOT INC DEC TILDE TYPEOF VOID ITEROBJ.
%right NEW.
%left PERIOD LBRACKET LPAREN.
%left MAX_PRI.
%right IN.

%type array {Jsi_OpCodes*}
%type commonstatement {Jsi_OpCodes*}
%type delete_statement {Jsi_OpCodes*}
%type do_statement {Jsi_OpCodes*}
%type expr {Jsi_OpCodes*}
%type expr_opt {Jsi_OpCodes*}
%type empty_block {Jsi_OpCodes*}
%type exprlist {Jsi_OpCodes*}
%type exprlist_opt {Jsi_OpCodes*}
%type fcall_exprs {Jsi_OpCodes*}
%type for_cond {Jsi_OpCodes*}
%type for_init {Jsi_OpCodes*}
%type for_statement {Jsi_OpCodes*}
%type func_expr {Jsi_OpCodes*}
%type func_statement {Jsi_OpCodes*}
%type func_statement_block {Jsi_OpCodes*}
%type if_statement {Jsi_OpCodes*}
%type item {Jsi_OpCodes*}
%type items {Jsi_OpCodes*}
%type iterstatement {Jsi_OpCodes*}
%type lvalue {Jsi_OpCodes*}
%type object {Jsi_OpCodes*}
%type statement {Jsi_OpCodes*}
%type statements {Jsi_OpCodes*}
%type statement_or_empty {Jsi_OpCodes*}
%type STRING {Jsi_String*}
%type switch_statement {Jsi_OpCodes*}
%type try_statement {Jsi_OpCodes*}
%type value {Jsi_OpCodes*}
%type vardec {Jsi_OpCodes*}
%type vardecs {Jsi_OpCodes*}
%type while_statement {Jsi_OpCodes*}
%type with_statement {Jsi_OpCodes*}

%type args {Jsi_ScopeStrs *}
%type args_opt {Jsi_ScopeStrs *}
%type arrowargs {Jsi_ScopeStrs *}
%type argsa {Jsi_ScopeStrs *}
%type typeid {int}
%type inof {int}
%type rettype {int}
%type argtype {int}
%type identifier_opt {const char*}
%type label_opt  {const char*}
%type func_prefix  {const char*}
%type case {struct jsi_CaseExprStat*}
%type cases {struct jsi_CaseList*}
%type argdefault {Jsi_Value *}

%destructor array {jsi_FreeOpcodes($$);}
%destructor case { Jsi_Free($$); }
%destructor cases { caselist_free($$); }
%destructor commonstatement {jsi_FreeOpcodes($$);}
%destructor delete_statement {jsi_FreeOpcodes($$);}
%destructor do_statement {jsi_FreeOpcodes($$);}
%destructor expr {jsi_FreeOpcodes($$);}
%destructor expr_opt {jsi_FreeOpcodes($$);}
%destructor empty_block {jsi_FreeOpcodes($$);}
%destructor exprlist {jsi_FreeOpcodes($$);}
%destructor exprlist_opt {jsi_FreeOpcodes($$);}
%destructor fcall_exprs {jsi_FreeOpcodes($$);}
%destructor for_cond {jsi_FreeOpcodes($$);}
%destructor for_init {jsi_FreeOpcodes($$);}
%destructor for_statement {jsi_FreeOpcodes($$);}
%destructor func_expr {jsi_FreeOpcodes($$);}
%destructor func_statement {jsi_FreeOpcodes($$);}
%destructor func_statement_block {jsi_FreeOpcodes($$);}
%destructor if_statement {jsi_FreeOpcodes($$);}
%destructor item {jsi_FreeOpcodes($$);}
%destructor items {jsi_FreeOpcodes($$);}
%destructor iterstatement {jsi_FreeOpcodes($$);}
%destructor lvalue {jsi_FreeOpcodes($$);}
%destructor object {jsi_FreeOpcodes($$);}
%destructor statement {jsi_FreeOpcodes($$);}
%destructor statements {jsi_FreeOpcodes($$);}
%destructor statement_or_empty {jsi_FreeOpcodes($$);}
%destructor switch_statement {jsi_FreeOpcodes($$);}
%destructor try_statement {jsi_FreeOpcodes($$);}
%destructor value {jsi_FreeOpcodes($$);}
%destructor vardec {jsi_FreeOpcodes($$);}
%destructor vardecs {jsi_FreeOpcodes($$);}
%destructor while_statement {jsi_FreeOpcodes($$);}
%destructor with_statement {jsi_FreeOpcodes($$);}

%destructor args {jsi_ScopeStrsFree(pstate->interp, $$);}
%destructor args_opt {jsi_ScopeStrsFree(pstate->interp, $$);}
%include {/*%destructor typeid {int inum}
%destructor inof {int inum}
%destructor rettype {int inum}
%destructor argtype {int inum}
%destructor identifier_opt {const char*}
%destructor label_opt  {const char*}
%destructor func_prefix  {const char*}
*/}
%destructor case {Jsi_Free($$);}
%destructor cases {Jsi_Free($$);}
%destructor argdefault { Jsi_ValueFree(pstate->interp, $$);}

file ::= .   { pstate->opcodes = code_nop(); }
file ::=  statements(B). {
        pstate->opcodes = B;
    }
file ::= statements(B) expr(C). {
        pstate->opcodes = codes_join3(B, C, code_ret(pstate, NULL, 1));
    }
file ::=  expr(B). {   
        pstate->opcodes = codes_join(B, code_ret(pstate, NULL, 1));
    }

statements(A) ::= statement(B).       { A = B; }
statements(A) ::=  statements(B) statement(C).  { A = codes_join(B, C); }

statement(A) ::=  iterstatement(B).       { A = B; }
statement(A) ::= commonstatement(B).    { A = B; }
statement(A) ::= IDENTIFIER COLON commonstatement(B). { A = B; }

commonstatement(A) ::= expr(B) SEMICOLON. { A = codes_join(B, code_pop(1)); }
commonstatement(A) ::= if_statement(B).  { A = B; }
commonstatement(A) ::= delete_statement(B).  { A = B; }
commonstatement(A) ::= BREAK(I) identifier_opt(B) SEMICOLON.      { A = code_reserved(pstate, &I.ln, RES_BREAK, B); }
commonstatement(A) ::= CONTINUE(I) identifier_opt(B) SEMICOLON.   { A = code_reserved(pstate, &I.ln, RES_CONTINUE, B); }
commonstatement(A) ::= RETURN(I) expr(B) SEMICOLON.   { A = codes_join(B, code_ret(pstate, &I.ln, 1)); }
commonstatement(A) ::= RETURN(I) SEMICOLON.        { A = code_ret(pstate, &I.ln, 0); }
commonstatement(A) ::= LOCAL vardecs(B) SEMICOLON. {
        jsi_mark_local(B);
        A = B;
    }
commonstatement(A) ::= THROW(I) expr(B) SEMICOLON.    { A = codes_join(B, code_throw(pstate, &I.ln)); }
commonstatement(A) ::= try_statement(B).     { A = B; }
commonstatement(A) ::= with_statement(B).    { A = B; }
commonstatement(A) ::= SEMICOLON.                   { A = code_nop(); }
commonstatement(A) ::= LCURLEY statements(B) RCURLEY.    { A = B; }
commonstatement(A) ::= func_statement(B).        { A = B; }


func_statement(A) ::= func_prefix(B) LPAREN(I) args_opt(C) RPAREN COLON rettype(D) func_statement_block(E). {
        C->retType = D;
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &I.ln, B, 0),
          code_push_func(pstate, &I.ln, jsi_FuncMake(pstate, C, E, &I.ln, B, 0)),
          code_assign(pstate, &I.ln, 1), code_pop(1));
        if (pstate->eval_flag) ret = codes_join(code_local(pstate, &I.ln, B), ret);
        jsi_PstatePop(pstate);
        A = ret;
    }
func_statement(A) ::=  func_prefix(B) LPAREN(I) args_opt(C) RPAREN func_statement_block(D). {
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &I.ln, B, 0),
          code_push_func(pstate, &I.ln, jsi_FuncMake(pstate, C, D, &I.ln, B, 0)),
          code_assign(pstate, &I.ln, 1), code_pop(1));
        if (pstate->eval_flag) ret = codes_join(code_local(pstate, &I.ln, B), ret);
        jsi_PstatePop(pstate);
        A = ret;
    }

func_prefix(A) ::= FUNC IDENTIFIER(B). [MAX_PRI] {
        if (!pstate->eval_flag) {
            jsi_PstateAddVar(pstate, &B.ln, B.str);
        }
        A = B.str;
    }

iterstatement(A) ::= for_statement(B).      { A = B; }
iterstatement(A) ::=  while_statement(B).   { A = B; }
iterstatement(A) ::=  do_statement(B).      { A = B; }
iterstatement(A) ::=  switch_statement(B).  { A = B; }

identifier_opt(A) ::= . { A = NULL; }
identifier_opt(A) ::=   IDENTIFIER(B). { A = B.str; }

label_opt(A) ::= . { A = NULL; }
label_opt(A) ::= IDENTIFIER(B) COLON. {
        A = B.str;
    }

empty_block(A) ::=  LCURLEY RCURLEY.  { A = code_nop(); }

statement_or_empty(A) ::= statement(B).   { A = B; }
statement_or_empty(A) ::= empty_block(B).   { A = B; }

with_statement(A) ::= WITH(I) LPAREN expr(B) RPAREN statement_or_empty(C). { 
        A = codes_join4(B, code_with(pstate, &I.ln, (C)->code_len + 1), C, code_ewith(pstate, &I.ln));
    }

switch_statement(A) ::=  label_opt SWITCH LPAREN expr(B) RPAREN empty_block. { A = codes_join(B, code_pop(1)); }
switch_statement(A) ::=  label_opt(D) SWITCH(I) LPAREN expr(B) RPAREN LCURLEY cases(C) RCURLEY.   {
        jsi_CaseList *cl = C;
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
                    yyerror(&I.ln, pstate, "switch with more then one default\n");
                }
                cldefault = t;
            } else {
                t->next = head;
                head = t;
            }
        }
        code_reserved_replace(allstats, 0, 1, D, 1);
        
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
        A = codes_join4(codes_join(B, code_unref()), ophead, allstats, code_pop(1));
    }


cases(A) ::= case(B).           { A = caselist_new(pstate, B); }
cases(A) ::= cases(B) case(C).    { A = caselist_insert(pstate, B, C); }


case(A) ::= CASE expr(B) COLON statements(C).    { A = exprstat_new(pstate, B, C, 0); }
case(A) ::= DEFAULT COLON statements(B).    { A = exprstat_new(pstate, NULL, B, 1); }
case(A) ::= DEFAULT COLON.                  { A = exprstat_new(pstate, NULL, code_nop(), 1); }
case(A) ::= CASE expr(B) COLON.    { A = exprstat_new(pstate, B, code_nop(), 0); }


try_statement(A) ::= TRY(I) func_statement_block(B) CATCH(J) LPAREN IDENTIFIER(C) RPAREN func_statement_block(D). {
        Jsi_OpCodes *catchblock = codes_join3(code_scatch(pstate, &J.ln, C.str), D, code_ecatch(pstate, &J.ln));
        Jsi_OpCodes *finallyblock = codes_join(code_sfinal(pstate, &I.ln), code_efinal(pstate, &J.ln));
        Jsi_OpCodes *tryblock = codes_join(B, code_etry(pstate, &I.ln));
        A = codes_join4(code_stry(pstate, &I.ln, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
try_statement(A) ::= TRY(I) func_statement_block(B) FINALLY(J) func_statement_block(D). {
        Jsi_OpCodes *catchblock = codes_join(code_scatch(pstate, &I.ln, NULL), code_ecatch(pstate, &I.ln));
        Jsi_OpCodes *finallyblock = codes_join3(code_sfinal(pstate, &I.ln), D, code_efinal(pstate, &J.ln));
        Jsi_OpCodes *tryblock = codes_join(B, code_etry(pstate, &I.ln));
        A = codes_join4(code_stry(pstate, &I.ln, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
try_statement(A) ::= TRY(I) func_statement_block(B) CATCH(J) LPAREN IDENTIFIER(C) RPAREN func_statement_block(D)
        FINALLY(K) func_statement_block(E). {
        Jsi_OpCodes *catchblock = codes_join3(code_scatch(pstate, &J.ln, C.str), D, code_ecatch(pstate, &J.ln));
        Jsi_OpCodes *finallyblock = codes_join3(code_sfinal(pstate, &K.ln), E, code_efinal(pstate, &K.ln));
        Jsi_OpCodes *tryblock = codes_join(B, code_etry(pstate, &I.ln));
        A = codes_join4(code_stry(pstate, &I.ln, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }

vardecs(A) ::= vardec(B).                     { A = B; }
vardecs(A) ::= vardecs(B) COMMA vardec(C).    { A = codes_join(B, C); }

vardec(A) ::= IDENTIFIER(B).             {
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &B.ln, B.str, 1),
                            code_push_undef(),
                            code_assign(pstate, &B.ln, 1),
                            code_pop(1));
        if (!pstate->eval_flag) jsi_PstateAddVar(pstate, &B.ln, B.str);
        else ret = codes_join(code_local(pstate, &B.ln, B.str), ret);
        A = ret;
    }
vardec(A) ::=  IDENTIFIER(B) EQUAL expr(C).   {
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &B.ln, B.str, 1),
                            C,
                            code_assign(pstate, &B.ln, 1),
                            code_pop(1));
        if (!pstate->eval_flag) jsi_PstateAddVar(pstate, &B.ln, B.str);
        else ret = codes_join(code_local(pstate, &B.ln, B.str), ret);
        A = ret;
    }

delete_statement(A) ::= DELETE lvalue(B) SEMICOLON.       {
        if (B->lvalue_flag == 2) {
            A = codes_join(B, code_delete(2));
        } else {
            A = codes_join(B, code_delete(1));
        }
    }

if_statement(A) ::= IF LPAREN expr(B) RPAREN statement_or_empty(C). {
        int offset = (C)->code_len;
        A = codes_join3(B, code_jfalse(offset + 1), C);
    }
if_statement(A) ::= IF LPAREN expr(B) RPAREN statement_or_empty(C) ELSE statement_or_empty(D). {
        int len_block2 = (D)->code_len;
        Jsi_OpCodes *block1 = codes_join(C, code_jmp(len_block2 + 1));
        Jsi_OpCodes *condi = codes_join(B, code_jfalse(block1->code_len + 1));
        A = codes_join3(condi, block1, D);
    }

inof(A) ::= IN.   { A = 0; }
inof(A) ::= OF.   { A = 1; }
    
for_statement(A) ::= label_opt(B) FOR LPAREN for_init(C) for_cond(D) SEMICOLON expr_opt(E) RPAREN statement_or_empty(F). {
        Jsi_OpCodes *init = C;
        Jsi_OpCodes *cond = D;
        Jsi_OpCodes *step = (E ? codes_join(E, code_pop(1)) : code_nop());
        Jsi_OpCodes *stat = F;
        Jsi_OpCodes *cont_jmp = code_jfalse(step->code_len + stat->code_len + 2);
        Jsi_OpCodes *step_jmp = code_jmp(-(cond->code_len + step->code_len + stat->code_len + 1));
        code_reserved_replace(stat, step->code_len + 1, 0, B, 0);
        A = codes_join(codes_join3(init, cond, cont_jmp),
                           codes_join3(stat, step, step_jmp));
    }
for_statement(A) ::= label_opt(B) FOR(I) LPAREN LOCAL IDENTIFIER(C) inof(D) expr(E) RPAREN statement_or_empty(S). {
        jsi_ForinVar *fv;
        int inof = D;
        Jsi_OpCodes *loc = code_local(pstate, &I.ln, C.str);
        jsi_mark_local(loc);
        fv = forinvar_new(pstate, C.str, loc, NULL);
        Jsi_OpCodes *lval;
        if (fv->varname) lval = code_push_index(pstate, &I.ln, fv->varname, 1);
        else lval = fv->lval;
        
        Jsi_OpCodes *ret = make_forin(lval, &I.ln, E, S, B, (inof!=0));
        if (fv->varname && fv->local) {
            if (!pstate->eval_flag) {
                jsi_PstateAddVar(pstate, &I.ln, fv->varname);
                jsi_FreeOpcodes(fv->local);
            } else ret = codes_join(fv->local, ret);
        }
        Jsi_Free(fv);
        A = ret;
    }
for_statement(A) ::= label_opt(B) FOR(I) LPAREN lvalue(C) inof(D) expr(E) RPAREN statement_or_empty(F). {
        jsi_ForinVar *fv;
        int inof = D;
        if ((C)->lvalue_flag == 2) 
            fv = forinvar_new(pstate, NULL, NULL, codes_join(C, code_subscript(pstate, &I.ln, 0)));
        else fv = forinvar_new(pstate, NULL, NULL, C);
        Jsi_OpCodes *lval;
        if (fv->varname) lval = code_push_index(pstate, &I.ln, fv->varname, 0);
        else lval = fv->lval;
        
        Jsi_OpCodes *ret = make_forin(lval, &I.ln, E, F, B, (inof!=0));
        if (fv->varname && fv->local) {
            if (!pstate->eval_flag) {
                jsi_PstateAddVar(pstate, &I.ln,fv->varname);
                jsi_FreeOpcodes(fv->local);
            } else ret = codes_join(fv->local, ret);
        }
        Jsi_Free(fv);
        A = ret;
    }


 
for_init(A) ::= SEMICOLON.               { A = code_nop(); }
for_init(A) ::= expr(B) SEMICOLON.         { A = codes_join(B, code_pop(1)); }
for_init(A) ::= LOCAL vardecs(B) SEMICOLON. {
        jsi_mark_local(B);
        A = B;
    }

for_cond(A) ::=  .          { A = code_push_bool(1); }
for_cond(A) ::=  expr(B).   { A = B; }

expr_opt(A) ::= .           { A = NULL; }
expr_opt(A) ::= expr(B).    { A = B; }

while_statement(A) ::= label_opt(D) WHILE LPAREN expr(B) RPAREN statement_or_empty(C). {
        Jsi_OpCodes *cond = B;
        Jsi_OpCodes *stat = C;
        code_reserved_replace(stat, 1, 0, D, 0);
        A = codes_join4(cond, code_jfalse(stat->code_len + 2), stat,
                           code_jmp(-(stat->code_len + cond->code_len + 1)));
    }


do_statement(A) ::= label_opt(B) DO statement_or_empty(C) WHILE LPAREN expr(D) RPAREN. {
        Jsi_OpCodes *stat = C;
        Jsi_OpCodes *cond = D;
        code_reserved_replace(stat, cond->code_len + 1, 0, B, 0);
        A = codes_join3(stat, cond,
                            code_jtrue(-(stat->code_len + cond->code_len)));
    }

func_expr(A) ::= FUNC(F) LPAREN(I) args_opt(B) RPAREN func_statement_block(C). {
        A = code_push_func(pstate,  &I.ln, jsi_FuncMake(pstate, B, C, &F.ln, NULL, 0));
        jsi_PstatePop(pstate);
    }
func_expr(A) ::= FUNC(F) LPAREN(I) args_opt(B) RPAREN COLON rettype(E) func_statement_block(C). {
        B->retType = E;
        A = code_push_func(pstate,  &I.ln, jsi_FuncMake(pstate, B, C, &F.ln, NULL, 0));
        jsi_PstatePop(pstate);
    }
func_expr(A) ::= FUNC(F) IDENTIFIER(B) LPAREN(I) args_opt(C) RPAREN func_statement_block(D). {
        A = code_push_func(pstate, &I.ln, jsi_FuncMake(pstate, C, D, &F.ln, B.str, 0));
        jsi_PstatePop(pstate);
    }
func_expr(A) ::= FUNC(F) IDENTIFIER(B) LPAREN(I) args_opt(C) RPAREN COLON rettype(E) func_statement_block(D). {
        C->retType = E;
        A = code_push_func(pstate, &I.ln, jsi_FuncMake(pstate, C, D, &F.ln, B.str, 0));
        jsi_PstatePop(pstate);
    }

args_opt(A) ::= . { A = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew()); }
args_opt(A) ::= args(B). {
        A = jsi_ArgsOptAdd(pstate, B);
    }
args_opt(A) ::= PERIOD PERIOD PERIOD. {
        Jsi_ScopeStrs *s = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew());
        s->varargs = 1;
        A = s;
    }
args_opt(A) ::= args(B) COMMA PERIOD PERIOD PERIOD. {
        Jsi_ScopeStrs *s = jsi_ArgsOptAdd(pstate, B);
        s->varargs = 1;
        A = s;
    }

typeid(A) ::=  TYPESTRING. {  A = (pstate->argType |= JSI_TT_STRING); }
typeid(A) ::=  TYPENUMBER. {  A = (pstate->argType |= JSI_TT_NUMBER); }
typeid(A) ::=  TYPEBOOLEAN. { A = (pstate->argType |= JSI_TT_BOOLEAN); }
typeid(A) ::=  TYPEREGEXP. {  A = (pstate->argType |= JSI_TT_REGEXP); }
typeid(A) ::=  TYPEOBJECT. {  A = (pstate->argType |= JSI_TT_OBJECT); }
typeid(A) ::=  TYPEUSEROBJ. { A = (pstate->argType |= JSI_TT_USEROBJ); }
typeid(A) ::=  TYPEITEROBJ. { A = (pstate->argType |= JSI_TT_ITEROBJ); }
typeid(A) ::=  TYPEANY. {     A = (pstate->argType |= JSI_TT_ANY); }
typeid(A) ::=  TYPEARRAY. {   A = (pstate->argType |= JSI_TT_ARRAY); }
typeid(A) ::=  TYPENULL. {    A = (pstate->argType |= JSI_TT_NULL); }
typeid(A) ::=  FUNC. {        A = (pstate->argType |= JSI_TT_FUNCTION); }
typeid(A) ::=  UNDEF. {       A = (pstate->argType |= JSI_TT_UNDEFINED); }
typeid(A) ::=  VOID. {        A = (pstate->argType |= JSI_TT_VOID); }
    
rettype(A) ::= argtype. {
        if (pstate->args)
            pstate->args->retType = pstate->argType;
        A = pstate->argType;
        pstate->argType = 0;
    }

argtype(A) ::= typeid. {
        A = pstate->argType;
    }
argtype(A) ::= argtype VERTBAR typeid. {
        A = pstate->argType;
    }

argdefault(A) ::= VOID. {      A = Jsi_ValueNew(pstate->interp); A->d.num = 1; }
argdefault(A) ::= UNDEF. {     A = Jsi_ValueNew(pstate->interp); A->d.num = 0; }
argdefault(A) ::= FNUMBER(B). { A = Jsi_ValueNewNumber(pstate->interp, *B.num); }
argdefault(A) ::= JSTRUE. {   A = Jsi_ValueNewBoolean(pstate->interp, 1); }
argdefault(A) ::= JSFALSE. {  A = Jsi_ValueNewBoolean(pstate->interp, 0); }
argdefault(A) ::= TYPENULL. {A = Jsi_ValueNewNull(pstate->interp); }
argdefault(A) ::= STRING(B). {  A = Jsi_ValueNewBlob(pstate->interp, (uchar*)B.vstr->str, B.vstr->len); }
    
args(A) ::= IDENTIFIER(B).  { A = jsi_argInsert(pstate, NULL, B.str, NULL, &B.ln, 0); }
args(A) ::= IDENTIFIER(B) EQUAL argdefault(C). { A = jsi_argInsert(pstate, NULL, B.str, C, &B.ln, 0); }
args(A) ::= IDENTIFIER(B) COLON argtype. { A = jsi_argInsert(pstate, NULL, B.str, NULL, &B.ln, 0);}
args(A) ::= IDENTIFIER(B) COLON argtype EQUAL argdefault(C). { A = jsi_argInsert(pstate, NULL, B.str, C, &B.ln, 0);}
args(A) ::= args(B) COMMA IDENTIFIER(C). { A = jsi_argInsert(pstate, B, C.str, NULL, &C.ln, 0); }
args(A) ::= args(B) COMMA IDENTIFIER(C) EQUAL argdefault(D). { A = jsi_argInsert(pstate, B, C.str, D, &C.ln, 0); }
args(A) ::= args(B) COMMA IDENTIFIER(C) COLON argtype. { A = jsi_argInsert(pstate, B, C.str, NULL, &C.ln, 0);}
args(A) ::= args(B) COMMA IDENTIFIER(C) COLON argtype EQUAL argdefault(D). { A = jsi_argInsert(pstate, B, C.str, D, &C.ln, 0);}

argsa(A) ::= IDENTIFIER(B).  { A = jsi_argInsert(pstate, NULL, B.str, NULL, &B.ln, 0); }
argsa(A) ::= argsa(C) COMMA IDENTIFIER(B).  { A = jsi_argInsert(pstate, C, B.str, NULL, &B.ln, 0); }

arrowargs(A) ::= IDENTIFIER(B).  { A = jsi_argInsert(pstate, NULL, B.str, NULL, &B.ln, 0); }
arrowargs(A) ::= LPAREN IDENTIFIER(B) COMMA argsa(C) RPAREN.   { A = jsi_argInsert(pstate, C, B.str, NULL, &B.ln, 0); }
arrowargs(A) ::= LPAREN RPAREN.   { A = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew()); }

func_statement_block(A) ::= LCURLEY statements(B) RCURLEY.    { A = B; }
func_statement_block(A) ::= empty_block(B).                  { A = B; }


expr(A) ::= value(B).              { A = B; }
expr(A) ::= func_expr(B).          { A = B; }
expr(A) ::=  lvalue(B).            { 
        if (B->lvalue_flag == 2) A = codes_join(B, code_subscript(pstate, NULL, 1)); 
        else A = B;
    }
expr(A) ::=  expr(B) COMMA expr(C).     { A = codes_join3(B, code_pop(1), C); }
expr(A) ::=  expr(B) LBRACKET(I) expr(C) RBRACKET.     { A = codes_join3(B, C, code_subscript(pstate, &I.ln, 1)); }
expr(A) ::=  expr(B) PERIOD(I) IDENTIFIER(C). {
    A = codes_join3(B, code_push_string(pstate,&I.ln,C.str), code_subscript(pstate, &I.ln, 1)); }
expr(A) ::=  MINUS expr(B). [NEG]       { A = codes_join(B, code_neg()); }
expr(A) ::=  PLUS expr(B).  [NEG]       { A = codes_join(B, code_pos()); }
expr(A) ::=  TILDE expr(B).             { A = codes_join(B, code_bnot()); }
expr(A) ::=  LOGICNOT expr(B).          { A = codes_join(B, code_not()); }
expr(A) ::=  VOID expr(B).              { A = codes_join3(B, code_pop(1), code_push_undef()); }
expr(A) ::=  expr(B) TIMES expr(C).      { A = codes_join3(B, C, code_mul()); }
expr(A) ::=  expr(B) DIVIDE expr(C).    { A = codes_join3(B, C, code_div()); }
expr(A) ::=  expr(B) PERCENT expr(C).   { A = codes_join3(B, C, code_mod()); }
expr(A) ::=  expr(B) PLUS expr(C).      { A = codes_join3(B, C, code_add()); }
expr(A) ::=  expr(B) MINUS expr (C).    { A = codes_join3(B, C, code_sub()); }

expr(A) ::=  expr(B) IN expr(C).          { A = codes_join3(B, C, code_in()); }
expr(A) ::=  lvalue(B) INC(I).            {
        if (B->lvalue_flag == 2) A = codes_join3(B, code_subscript(pstate, &I.ln, 0), code_inc(pstate, &I.ln, 1));
        else A = codes_join(B, code_inc(pstate, &I.ln, 1));
    }
expr(A) ::=  lvalue(B) DEC(I).            { 
        if (B->lvalue_flag == 2) A = codes_join3(B, code_subscript(pstate, &I.ln, 0), code_dec(pstate, &I.ln, 1));
        else A = codes_join(B, code_dec(pstate, &I.ln, 1)); 
    }
expr(A) ::=  INC(I) lvalue(B).            {
        if (B->lvalue_flag == 2) A = codes_join3(B, code_subscript(pstate, &I.ln, 0), code_inc(pstate, &I.ln, 0));
        else A = codes_join(B, code_inc(pstate, &I.ln, 0));
    }
expr(A) ::=  TYPEOF(I) expr(B). {
        if (B->lvalue_flag == 2) A = codes_join3(B, code_subscript(pstate, &I.ln, 0), code_typeof(pstate, &I.ln, 0));
        else A = codes_join(B, code_typeof(pstate, &I.ln, 0));
    }
expr(A) ::=  DEC(I) lvalue(B).            { 
        if (B->lvalue_flag == 2) A = codes_join3(B, code_subscript(pstate, &I.ln, 0), code_dec(pstate, &I.ln, 0));
        else A = codes_join(B, code_dec(pstate, &I.ln, 0));
    }
expr(A) ::=  LPAREN expr(B) RPAREN.          { A = B; }
expr(A) ::= arrowargs(B) ARROW(I) expr(C). {
        A = code_push_func(pstate,  &I.ln, jsi_FuncMake(pstate, B, codes_join(C, code_ret(pstate, &I.ln, 1)), &I.ln, NULL, 0));
    }
expr(A) ::= arrowargs(B) ARROW(I) func_statement_block(C). {
        A = code_push_func(pstate,  &I.ln, jsi_FuncMake(pstate, B, C, &I.ln, NULL, 0));
    }
expr(A) ::=  expr(B) AND expr(C).         {
        Jsi_OpCodes *expr2 = codes_join(code_pop(1), C);
        A = codes_join3(B, code_jfalse_np(expr2->code_len + 1), expr2);
    }
expr(A) ::=  expr(B) OR expr(C).          {
        Jsi_OpCodes *expr2 = codes_join(code_pop(1), C);
        A = codes_join3(B, code_jtrue_np(expr2->code_len + 1), expr2);
    }
expr(A) ::=  expr(B) LESSTHAN expr(C).         { A = codes_join3(B, C, code_less()); }
expr(A) ::=  expr(B) GREATERTHAN expr(C).         { A = codes_join3(B, C, code_greater()); }
expr(A) ::=  expr(B) LEQ expr(C).         { A = codes_join3(B, C, code_lessequ()); }
expr(A) ::=  expr(B) GEQ expr(C).         { A = codes_join3(B, C, code_greaterequ()); }
expr(A) ::=  expr(B) EQU expr(C).         { A = codes_join3(B, C, code_equal()); }
expr(A) ::=  expr(B) NEQ expr(C).         { A = codes_join3(B, C, code_notequal()); }
expr(A) ::=  expr(B) EEQU expr(C).        { A = codes_join3(B, C, code_eequ());    }
expr(A) ::=  expr(B) NNEQ expr(C).        { A = codes_join3(B, C, code_nneq()); }
expr(A) ::=  expr(B) AMPERSAND expr(C).   { A = codes_join3(B, C, code_band()); }
expr(A) ::=  expr(B) VERTBAR expr(C).         { A = codes_join3(B, C, code_bor()); }
expr(A) ::=  expr(B) CARET expr(C).       { A = codes_join3(B, C, code_bxor()); }
expr(A) ::=  expr(B) LSHF expr(C).        { A = codes_join3(B, C, code_shf(0)); }
expr(A) ::=  expr(B) RSHF expr(C).        { A = codes_join3(B, C, code_shf(1)); }
expr(A) ::=  expr(B) URSHF expr(C).       { A = codes_join3(B, C, code_shf(2)); }
expr(A) ::=  lvalue(B) EQUAL(I) expr(C).     { A = codes_join3(B, C, code_assign(pstate, &I.ln, B->lvalue_flag)); }
expr(A) ::=  lvalue(B) ADDAS(I) expr(C).     { A = opassign(pstate, &I.ln, B, C, code_add()); }
expr(A) ::=  lvalue(B) MNSAS(I) expr(C).     { A = opassign(pstate, &I.ln, B, C, code_sub()); }
expr(A) ::=  lvalue(B) MULAS(I) expr(C).     { A = opassign(pstate, &I.ln, B, C, code_mul()); }
expr(A) ::=  lvalue(B) MODAS(I) expr(C).     { A = opassign(pstate, &I.ln, B, C, code_mod()); }
expr(A) ::=  lvalue(B) LSHFAS(I) expr(C).    { A = opassign(pstate, &I.ln, B, C, code_shf(0)); }
expr(A) ::=  lvalue(B) RSHFAS(I) expr(C).    { A = opassign(pstate, &I.ln, B, C, code_shf(1)); }
expr(A) ::=  lvalue(B) URSHFAS(I) expr(C).   { A = opassign(pstate, &I.ln, B, C, code_shf(2)); }
expr(A) ::=  lvalue(B) BANDAS(I) expr(C).    { A = opassign(pstate, &I.ln, B, C, code_band()); }
expr(A) ::=  lvalue(B) BORAS(I) expr(C).     { A = opassign(pstate, &I.ln, B, C, code_bor()); }
expr(A) ::=  lvalue(B) BXORAS(I) expr(C).    { A = opassign(pstate, &I.ln, B, C, code_bxor()); }
expr(A) ::=  lvalue(B) DIVAS(I) expr(C).     { A = opassign(pstate, &I.ln, B, C, code_div()); }
expr(A) ::=  lvalue(B) INSTANCEOF expr(C).{ A = codes_join3(B, C, code_instanceof()); }
expr(A) ::=  fcall_exprs(B).           { A = B; }
    
expr(A) ::=  NEW(I) value(B).             { A = codes_join(B, code_newfcall(pstate, &I.ln, 0, NULL, B)); }
expr(A) ::=  NEW(I) lvalue(B).            { 
        if (B->lvalue_flag == 2) A = codes_join3(B, code_subscript(pstate, &I.ln, 1), code_newfcall(pstate, &I.ln, 0, NULL, B));
        else A = codes_join(B, code_newfcall(pstate, &I.ln, 0, NULL, B));}
expr(A) ::=  NEW(I) LPAREN expr(B) RPAREN.      { A = codes_join(B, code_newfcall(pstate, &I.ln, 0, NULL, B)); }
expr(A) ::=  NEW(I) func_expr(B).         { A = codes_join(B, code_newfcall(pstate, &I.ln, 0, NULL, B)); }
expr(A) ::=  NEW(I) value(B) LPAREN exprlist_opt(C) RPAREN.        {
        Jsi_OpCodes *lval = B;
        Jsi_OpCodes *opl = C;
        int expr_cnt = opl ? opl->expr_counter:0;
        A = codes_join3(B, (opl ? opl : code_nop()), code_newfcall(pstate, &I.ln, expr_cnt, lval->lvalue_name, opl));
    }
expr(A) ::=  NEW(I) lvalue(B) LPAREN exprlist_opt(C) RPAREN.      {
        Jsi_OpCodes *opl = C;
        int expr_cnt = opl ? opl->expr_counter:0;
        Jsi_OpCodes *lv = NULL;
        if (B->lvalue_flag == 2) lv = codes_join(B, code_subscript(pstate, &I.ln, 1));
        else lv = B;
        A = codes_join3(lv, (opl ? opl : code_nop()), code_newfcall(pstate, &I.ln, expr_cnt, lv?lv->lvalue_name:NULL, opl));
    }
expr(A) ::=  NEW(I) LPAREN expr(B) RPAREN LPAREN exprlist_opt(C) RPAREN. { 
        Jsi_OpCodes *opl = C;
        int expr_cnt = opl ? opl->expr_counter:0;
        A = codes_join3(B, (opl ? opl : code_nop()), code_newfcall(pstate, &I.ln, expr_cnt, NULL, opl));
    }
expr(A) ::=  NEW(I) func_expr(B) LPAREN exprlist_opt(C) RPAREN.    {
        Jsi_OpCodes *opl = C;
        int expr_cnt = opl ? opl->expr_counter:0;
        A = codes_join3(B, (opl ? opl : code_nop()), code_newfcall(pstate, &I.ln, expr_cnt, NULL, opl));
    }
expr(A) ::=  func_expr(B) LPAREN(I) exprlist_opt(C) RPAREN.   {
        Jsi_OpCodes *opl = C;
        int expr_cnt = opl ? opl->expr_counter:0;
        A = codes_join3(B, (opl ? opl : code_nop()), code_fcall(pstate, &I.ln, expr_cnt, NULL, NULL, opl, NULL));
    }
expr(A) ::=  expr(B) QUESTION expr(C) COLON expr(D). {
        Jsi_OpCodes *expr2 = codes_join(C, code_jmp((D)->code_len + 1));
        A = codes_join4(B, code_jfalse(expr2->code_len + 1), expr2, D);
    }
expr(A) ::=  JSDEBUG(I). { A = code_debug(pstate, &I.ln); }


fcall_exprs(A) ::= expr(B) PERIOD IDENTIFIER(C) LPAREN exprlist_opt(D) RPAREN. {
        Jsi_OpCodes *lval = B;
        const char *n1 = lval->lvalue_name;
        const char *n2 = C.str;
        Jsi_OpCodes *ff = codes_join4(B, code_push_string(pstate, &C.ln, C.str), code_chthis(pstate, &C.ln, 1), code_subscript(pstate, &C.ln, 1));
        Jsi_OpCodes *opl = D;
        int expr_cnt = opl ? opl->expr_counter:0;
        A = codes_join3(ff, (opl ? opl : code_nop()), code_fcall(pstate, &C.ln, expr_cnt, n1, n2, opl, NULL));
    }
fcall_exprs(A) ::= expr(B) LBRACKET(I) expr(C) RBRACKET LPAREN exprlist_opt(D) RPAREN. {
        Jsi_OpCodes *ff = codes_join4(B, C, code_chthis(pstate, &I.ln, 1), code_subscript(pstate, &I.ln, 1));
        Jsi_OpCodes *opl = D;
        int expr_cnt = (opl ? opl->expr_counter:0);
        A = codes_join3(ff, (opl ? opl : code_nop()), code_fcall(pstate, &I.ln, expr_cnt, NULL, NULL, opl, NULL));
    }
fcall_exprs(A) ::= LPAREN(I) expr(B) RPAREN LPAREN exprlist_opt(C) RPAREN. {
        Jsi_OpCodes *opl = C;
        int expr_cnt = opl ? opl->expr_counter:0;
        A = codes_join4(B, code_chthis(pstate, &I.ln, 0), (opl ? opl : code_nop()), code_fcall(pstate, &I.ln, expr_cnt, NULL, NULL, opl, NULL));
    }
fcall_exprs(A) ::= lvalue(B) LPAREN(I) exprlist_opt(C) RPAREN. {
        Jsi_OpCodes *opl = C;
        int expr_cnt = opl ? opl->expr_counter:0;
        Jsi_OpCodes *pref;
        Jsi_OpCodes *lval = B;
        const char *n1 = lval->lvalue_name;
        if (lval->lvalue_flag == 2) {
            const char *n2 = NULL;
            pref = codes_join3(B, code_chthis(pstate, &I.ln, 1), code_subscript(pstate, &I.ln, 1));
            if (pref->code_len>=2 && pref->codes[0].op == OP_PUSHVAR && pref->codes[1].op == OP_PUSHSTR && !n1) {
                jsi_FastVar *fv = (jsi_FastVar*)pref->codes[0].data;
                n2 = fv->varname;
                n1 = (const char*)pref->codes[1].data;
            }
            A = codes_join3(pref, (opl ? opl : code_nop()), code_fcall(pstate, &I.ln, expr_cnt, n1, n2, opl, NULL));
        } else {
            if (lval->lvalue_name && Jsi_Strcmp(lval->lvalue_name, "eval") == 0) {
                A = codes_join((opl ? opl : code_nop()), code_eval(pstate, &I.ln, expr_cnt, lval));
            } else {
                jsi_Pline *jpl = &I.ln;
                pref = codes_join(B, code_chthis(pstate, &I.ln, 0));
                A = codes_join3(pref, (opl ? opl : code_nop()), code_fcall(pstate, jpl, expr_cnt, n1, NULL, opl, pref));
            }
        }
    }


lvalue(A) ::= IDENTIFIER(B).              {
        Jsi_OpCodes *lval = code_push_index(pstate, &B.ln, B.str, 0); 
        A = lval;
        lval->lvalue_flag = 1; 
        lval->lvalue_name = B.str; 
    }
lvalue(A) ::=  ARGUMENTS.             { A = code_push_args(); (A)->lvalue_flag = 1; }
lvalue(A) ::=  JSTHIS(I).                 { A = code_push_this(pstate, &I.ln); (A)->lvalue_flag = 1; }
lvalue(A) ::=  lvalue(B) LBRACKET(I) expr(C) RBRACKET.   {
        if (B->lvalue_flag == 2) A = codes_join3(B, code_subscript(pstate, &I.ln, 1), C); 
        else A = codes_join(B, C); 
        (A)->lvalue_flag = 2;
    }
lvalue(A) ::=  lvalue(B) PERIOD(I) IDENTIFIER(C). {
        if (B->lvalue_flag == 2) A = codes_join3(B, code_subscript(pstate, &I.ln, 1), code_push_string(pstate, &I.ln, C.str)); 
        else A = codes_join(B, code_push_string(pstate,&I.ln, C.str));
        (A)->lvalue_flag = 2;
    }


exprlist_opt(A) ::= exprlist(B).  { A = B; }
exprlist_opt(A) ::= .   { A = NULL; }


exprlist(A) ::=  expr(B). [ARGCOMMA] { A = B; (A)->expr_counter = 1; }
exprlist(A) ::=  exprlist(B) COMMA expr(C). [ARGCOMMA] { 
        int exprcnt = (B)->expr_counter + 1;
        A = codes_join(B, C);
        (A)->expr_counter = exprcnt;
    }

value(A) ::=  STRING(B). { A = code_push_vstring(pstate, &B.ln, B.vstr); }
value(A) ::=  TYPENULL. { A = code_push_null(); }
value(A) ::=  UNDEF. { A = code_push_undef(); }
value(A) ::=  JSTRUE. { A = code_push_bool(1); }
value(A) ::=  JSFALSE. { A = code_push_bool(0); }
// Following rule not necessary as lexer will accept negative floats.
// value(A) ::=  MINUS FNUMBER(B). [NEG] { A = code_push_num(-B.num); }
value(A) ::=  FNUMBER(B). { A = code_push_num(B.num); }
value(A) ::=  REGEXP(B). { A = code_push_regex(pstate, &B.ln, B.regex); }
value(A) ::=  object(B). { A = B; }
value(A) ::=  array(B). { A = B; }

object(A) ::= LCURLEY(I) items(B) RCURLEY.   { A = codes_join(B, code_object(pstate, &I.ln, (B)->expr_counter)); }

items(A) ::=   .   { A = code_nop(); (A)->expr_counter = 0; }
items(A) ::=  item(B).  { A = B; (A)->expr_counter = 1; }
items(A) ::=  items(B) COMMA item(C). {
        int cnt = (B)->expr_counter + 1;
        A = codes_join(B, C);
        (A)->expr_counter = cnt;
    }

item(A) ::= IDENTIFIER(B) COLON expr(C). { A = codes_join(code_push_string(pstate, &B.ln, B.str), C); }
item(A) ::= STRING(B) COLON expr(C).   { A = codes_join(code_push_vstring(pstate, &B.ln, B.vstr), C); }
item(A) ::= FNUMBER(B) COLON expr(C).   { A = codes_join(code_push_num(B.num), C); }
item(A) ::= JSTRUE COLON expr(C).   { A = codes_join(code_push_bool(1), C); }
item(A) ::= JSFALSE COLON expr(C).   { A = codes_join(code_push_bool(0), C); }
item(A) ::= UNDEF COLON expr(C).   { A = codes_join(code_push_undef(), C); }
item(A) ::= TYPENULL COLON expr(C).   { A = codes_join(code_push_null(), C); }

array(A) ::= LBRACKET(I) exprlist(B) RBRACKET. { A = codes_join(B, code_array(pstate, &I.ln, (B)->expr_counter)); }
array(A) ::= LBRACKET(I) RBRACKET. { A = code_array(pstate, &I.ln, 0); }
