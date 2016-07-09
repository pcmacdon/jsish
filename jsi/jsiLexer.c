#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#define COMMENT (-128)

static int lexer_getchar(Lexer *lex)
{
    Jsi_Interp *interp = lex->pstate->interp;
    int c = 0;
    if (!lex) Jsi_LogBug("No lexer init");
    if (lex->ltype == LT_FILE) {
        c = Jsi_Getc(lex->d.fp);
        if (c == EOF) c = 0;
    } else {
        if (lex->ungot) {
            c = lex->unch[--lex->ungot];
        } else {
            c = lex->d.str[lex->cur];
            if (c != 0) lex->cur++;
        }
    }
    //printf("%c", c);
    if (c == '\n') {
        //printf("!!!!!!!!!!\n");
        lex->cur_line++;
        lex->cur_char = 0;
    }
    lex->cur_char++;
    //printf("<%2x\n",c);
    return c;
}

static void lexer_ungetc(int c, Lexer *lex)
{
    Jsi_Interp *interp = lex->pstate->interp;
    if (!lex) Jsi_LogBug("No lexer init");
    if (!c) return;
    if (c == '\n') {
        lex->cur_line--;
    }

    //printf("^<%c>", c);
    if (lex->ltype == LT_FILE) {
        Jsi_Ungetc(lex->d.fp, c);
    } else if (lex->ungot<=0 && lex->cur>0 && c == lex->d.str[lex->cur-1]) {
        lex->cur--;
    } else if ((lex->ungot+2)<(int)sizeof(lex->unch)) {
        lex->unch[lex->ungot++] = c;
    }
    //printf(">%2x\n",c);
}

int jsi_LexerInit(Jsi_Interp *interp)
{
    static struct st_kw {
        const char *name;
        int value;
    } keywords[] = {
        { "if", IF },
        { "else", ELSE },
        { "for", FOR },
        { "in", IN },
        { "while", WHILE },
        { "do", DO },
        { "continue", CONTINUE },
        { "switch", SWITCH },
        { "case", CASE },
        { "default", DEFAULT },
        { "break", BREAK },
        { "function", FUNC },
        { "return", RETURN },
        { "var", LOCAL },
        { "new", NEW },
        { "delete", DELETE },
        { "try", TRY },
        { "catch", CATCH },
        { "throw", THROW },
        { "finally", FINALLY },
        { "with", WITH },
        { "undefined", UNDEF },
        { "true", _TRUE },
        { "false", _FALSE },
        { "this", _THIS },
        { "arguments", ARGUMENTS },
        { "void", VOID },
        { "typeof", TYPEOF },
        { "instanceof", INSTANCEOF },
        { "string", TYPESTRING },
        { "number", TYPENUMBER },
        { "regexp", TYPEREGEXP },
        { "any", TYPEANY },
        { "userobj", TYPEUSEROBJ },
        { "boolean", TYPEBOOLEAN },
        { "array", TYPEARRAY },
        { "...", ELLIPSIS },
        { "__debug", __DEBUG }
    };
    uint i;
    Jsi_HashEntry *hPtr;
    if (!interp->lexkeyTbl->numEntries) {
        int isNew;
        for (i = 0; i < sizeof(keywords) / sizeof(struct st_kw); ++i) {
            hPtr = Jsi_HashEntryNew(interp->lexkeyTbl, keywords[i].name, &isNew);
            assert(hPtr);
            if (hPtr)
                Jsi_HashValueSet(hPtr, (void*)keywords[i].value);
        }
    }
    return JSI_OK;
}

static int iskey(const char *word, Lexer *lex)
{
    Jsi_Interp *interp = lex->pstate->interp;
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->lexkeyTbl, word);
    if (hPtr)
        return (int)Jsi_HashValueGet(hPtr);
    return 0;
}

static char *do_string(Lexer *lex)
{
    Jsi_Interp *interp = lex->pstate->interp;
    int n, c = lexer_getchar(lex);
    int endchar = c;
    
    int bufi = 0, bsiz, done = 0;
    char unibuf[bsiz=BUFSIZ], *buf = unibuf, *ret;
    
    while (!done) {
        if (bufi >= (bsiz-5)) {
            int nsiz = bsiz+=BUFSIZ;
            if (buf!=unibuf) 
                buf = (char*)Jsi_Realloc(buf, nsiz);
            else {
                buf = (char*)Jsi_Malloc(nsiz);
                memcpy(buf, unibuf, sizeof(unibuf));
            }
            bsiz = nsiz;
        }
        c = lexer_getchar(lex);
        if (c == EOF || c == 0) {
            goto saw_eof;
        }
        if (c == '\\') {
            n = lexer_getchar(lex);
            switch(n) {
                case 'b': buf[bufi++] = '\b'; break;
                case 'f': buf[bufi++] = '\f'; break;
                case 'n': buf[bufi++] = '\n'; break;
                case 'r': buf[bufi++] = '\r'; break;
                case 't': buf[bufi++] = '\t'; break;
                case EOF: 
                case 0:
saw_eof:
                    Jsi_LogError("Unexpected EOF parsing string.");
                    buf[bufi++] = 0;
                    done = 1;
                    break;
                default: buf[bufi++] = n;
            }
        } else {
            buf[bufi++] = c;
        }
        if (c == endchar) {
            bufi --;
            break;
        }
    }
    buf[bufi] = 0;
    Jsi_HashEntry *hPtr = Jsi_HashSet(lex->pstate->strTbl, buf, NULL);
    ret = (char*)Jsi_HashKeyGet(hPtr);
    if (buf != unibuf)
        Jsi_Free(buf);
    return ret;
}

static char *do_regex(Lexer *lex)
{
    Jsi_Interp *interp = lex->pstate->interp;

    int n, bufi = 0, bsiz;
    char unibuf[bsiz=BUFSIZ], *buf = unibuf, *ret = NULL;
    
    buf[bufi++] = lexer_getchar(lex);     /* first '/'*/
    while (1) {
        if (bufi >= (bsiz-5)) {
            int nsiz = bsiz+=BUFSIZ;
            if (buf!=unibuf) 
                buf = (char*)Jsi_Realloc(buf, nsiz);
            else {
                buf = (char*)Jsi_Malloc(nsiz);
                memcpy(buf, unibuf, sizeof(unibuf));
            }
            bsiz = nsiz;
        }
        int c = lexer_getchar(lex);
        if (c == EOF || c == 0) {
            goto saw_eof;
        }
        if (c == '\\') {
            n = lexer_getchar(lex);
            if (n == EOF || c == 0) {
saw_eof:
                Jsi_LogError("Unexpected EOF parsing regular expression.\n");
                buf[bufi++] = 0;
                break;
            }
            
            buf[bufi++] = c;
            buf[bufi++] = n;
        } else if (c == '/') {
            buf[bufi++] = '/';
            while (1) {
                buf[bufi++] = c = lexer_getchar(lex);
                if (!isalnum(c)) break;
            }
            buf[bufi-1] = 0;
            lexer_ungetc(c, lex);
            break;
        } else {
            buf[bufi++] = c;
        }
    }
    Jsi_HashEntry *hPtr = Jsi_HashSet(lex->pstate->strTbl, buf, NULL);
    ret = (char*)Jsi_HashKeyGet(hPtr);
    ret = Jsi_Strdup(buf);
    if (buf != unibuf)
        Jsi_Free(buf);
    return ret;
}

static int do_sign(Lexer *lex)
{
    static struct st_sn {
        const char *name;
        int len;
        int value;
    } signs[] = {
        { ">>>=", 4, URSHFAS },
        { "<<=", 3, LSHFAS },
        { ">>=", 3, RSHFAS },
        { "===", 3, EEQU },
        { "!==", 3, NNEQ },
        { ">>>", 3, URSHF },
        { "==", 2, EQU },
        { "!=", 2, NEQ },
        { "<=", 2, LEQ },
        { ">=", 2, GEQ },
        { "++", 2, INC },
        { "--", 2, DEC },
        { "&&", 2, AND },
        { "||", 2, OR },
        { "+=", 2, ADDAS },
        { "-=", 2, MNSAS },
        { "*=", 2, MULAS },
        { "/=", 2, DIVAS },
        { "%=", 2, MODAS },
        { "&=", 2, BANDAS },
        { "|=", 2, BORAS },
        { "^=", 2, BXORAS },
        { "<<", 2, LSHF },
        { ">>", 2, RSHF }
    };

    int bufi;
    char buf[4];
    uint i;
    for (bufi = 0; bufi < 4; ++bufi) {
        int c = lexer_getchar(lex);
        if (c == 0 || c == '\n') break;
        buf[bufi] = c;
    }
    if (!bufi) return 0;
    
    for (i = 0; i < sizeof(signs)/sizeof(struct st_sn); ++i) {
        if (bufi < signs[i].len) continue;
        if (strncmp(buf, signs[i].name, signs[i].len) == 0) {
            int j;
            for (j = bufi - 1; j >= signs[i].len; --j)
                lexer_ungetc(buf[j], lex);

            return signs[i].value;
        }
    }
    
    for (i = bufi - 1; i >= 1; --i)
        lexer_ungetc(buf[i], lex);
    
    return buf[0];
}

#define LOCATION_START(loc, lex) do {       \
    (loc)->first_line = (lex)->cur_line;    \
    (loc)->first_column = (lex)->cur_char;  \
    } while(0)
#define LOCATION_END(loc, lex) do {         \
    (loc)->last_line = (lex)->cur_line;     \
    (loc)->last_column = (lex)->cur_char;   \
    } while(0)

static void eat_comment(Lexer *lex)
{
    Jsi_Interp *interp = lex->pstate->interp;
    int c;
    while((c = lexer_getchar(lex))) {
        if (c == '*') {
            c = lexer_getchar(lex);
            if (c == '/') return;
            lexer_ungetc(c, lex);
        }
    }
    Jsi_LogError("Comment reach end of file");
}

static int _yylex (YYSTYPE *yylvalp, YYLTYPE *yyllocp, Lexer *lex)
{
    int c, c2;
    Jsi_Interp *interp = lex->pstate->interp;
    
    char word[BUFSIZ];
    int wi = 0;

    LOCATION_START(yyllocp, lex);
    while ((c = lexer_getchar(lex)) == ' ' || c == '\t' || c == '\n' || c == '\r');
    
    if (c=='.') {
        c2 = lexer_getchar(lex);
        if (!isdigit(c2)) {
            lexer_ungetc(c2, lex);
        } else {
            word[wi++] = c;
            c = c2;
        }
    }
            
    if (isdigit(c)) {
        int base = 10, digCnt = 1, isdig, cpre=0;
        Jsi_Number fval;
        char *eptr = NULL;
        word[wi++] = c;
        while (wi < 1020) {
            c = lexer_getchar(lex);
            isdig = isxdigit(c);
            if (isdig)
                digCnt++;
            if (isdig || c == '.' || toupper(c)=='P' || toupper(c)=='E'
                || (toupper(c)=='X' && wi==1 && word[0] == '0')
                || ((c == '-' || c == '+') && toupper(cpre)=='E')
                || (base == 16 && isxdigit(c))) {
                if (toupper(c)=='X')
                    base = 16;
                word[wi++] = c;
                cpre = c;
                continue;
            }
            lexer_ungetc(c, lex);
            break;
        }
        word[wi] = 0;

        if (word[0] == '0' && (digCnt+(base==16)) == wi)
            fval = (Jsi_Number)strtoll(word, &eptr, base);
        else
            fval = (Jsi_Number)strtod(word, &eptr);
        LOCATION_END(yyllocp, lex);
        if (eptr == NULL || *eptr)
            Jsi_LogError("invalid number: %s", word); 
        Jsi_Number *db = (Jsi_Number *)Jsi_Malloc(sizeof(Jsi_Number));
        *db = fval;
        *yylvalp = db;
        return FNUMBER;
    } else if (c == '"' || c == '\'') {
        lexer_ungetc(c, lex);
        *yylvalp = do_string(lex);
        LOCATION_END(yyllocp, lex);
        return STRING;
    } else if (isalpha(c) || c == '_' || c == '$') {
        lexer_ungetc(c, lex);
        while (wi < 1020) {
            c = lexer_getchar(lex);
            if (!isalnum(c) && c != '_' && c != '$') break;
            word[wi++] = c;
        }
        lexer_ungetc(c, lex);
        
        word[wi] = 0;
        int r = iskey(word, lex);
        if (r) return r;
        *yylvalp = (char*)Jsi_KeyAdd(interp,word); /*Jsi_Strdup(word);*/
        LOCATION_END(yyllocp, lex);
        return IDENTIFIER;
    } else if (c == '/') {
        int d = lexer_getchar(lex);
        if (d == '/') {
            while ((d = lexer_getchar(lex)) != '\r' && d != '\n' && d != 0);
            return COMMENT;
        } else if (d == '*') {
            eat_comment(lex);
            return COMMENT;
        } else lexer_ungetc(d, lex);
        
        if (lex->last_token == '=' || lex->last_token == '(' || lex->last_token == ':'
            || lex->last_token == '?' || lex->last_token == ','
            || lex->last_token == '[' || lex->last_token == '{')
        {
            lexer_ungetc(c, lex);
            
            char *regtxt = do_regex(lex);
            Jsi_Regex *re = Jsi_RegExpNew(interp, regtxt, JSI_REG_STATIC);
            if (!(*yylvalp = re)) {
                 Jsi_Free(regtxt);
                 return -1;
            }
            Jsi_Free(regtxt);
            return REGEXP;
        }
    }
    
    lexer_ungetc(c, lex);
    
    int r = do_sign(lex);
    LOCATION_END(yyllocp, lex);
    return r;
}

int yylex (YYSTYPE *yylvalp, YYLTYPE *yyllocp, jsi_Pstate *pstate)
{
    int ret;
    do {
        ret = _yylex(yylvalp, yyllocp, pstate->lexer);
    } while (ret == COMMENT);
/*
    if (ret < 128 && ret > 0) printf("%c\n", ret);
    else printf("%d\n", ret);
*/
    pstate->lexer->last_token = ret;
    return ret;
}

void yyerror(YYLTYPE *yylloc, jsi_Pstate *ps, const char *msg)
{
    Jsi_Interp *interp = ps->interp;
    interp->errLine = yylloc->first_line;
    interp->errCol = yylloc->first_column;
    Jsi_LogParse("%s:%d.%d: error: %s\n", interp->curFile?interp->curFile:"@", yylloc->first_line, 
        yylloc->first_column, msg);
    /*if (interp->curFile)
        fprintf(stderr, "%s:%d.%d: %s\n",  interp->curFile, yylloc->first_line, yylloc->first_column, msg);
    else
        fprintf(stderr, "@%d.%d: %s\n", yylloc->first_line, yylloc->first_column, msg);*/
    ps->err_count++;
}
#endif
