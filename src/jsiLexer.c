#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#define COMMENT (-128)

static int lexer_getchar(jsi_Lexer *lex)
{
    if (!lex || !lex->pstate) { fprintf(stderr, "No lexer init\n"); return -1; }
    Jsi_Interp *interp = lex->pstate->interp;
    int c = 0;
    if (lex->ltype == LT_FILE) {
        c = Jsi_Getc(interp, lex->d.fp);
        if (c == EOF) c = 0;
    } else {
        if (lex->ungot) {
            c = lex->unch[--lex->ungot];
        } else {
            c = lex->d.str[lex->cur];
            if (c != 0) lex->cur++;
        }
        if (lex->inStr && c == '\\') {
            int nc = (lex->ungot ? lex->unch[lex->ungot-1] : lex->d.str[lex->cur-1]);
            if (nc == '\n') {
                return lexer_getchar(lex);
                return lexer_getchar(lex);
            }
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

static void lexer_ungetc(int c, jsi_Lexer *lex)
{
    if (!lex || !lex->pstate) { fprintf(stderr, "No lexer init\n"); return; }
    Jsi_Interp *interp = lex->pstate->interp;
    if (!c) return;
    if (c == '\n') {
        lex->cur_line--;
    }

    //printf("^<%c>", c);
    if (lex->ltype == LT_FILE) {
        Jsi_Ungetc(interp, lex->d.fp, c);
    } else if (lex->ungot<=0 && lex->cur>0 && c == lex->d.str[lex->cur-1]) {
        lex->cur--;
    } else if ((lex->ungot+2)<(int)sizeof(lex->unch)) {
        lex->unch[lex->ungot++] = c;
    }
    //printf(">%2x\n",c);
}

Jsi_RC jsi_InitLexer(Jsi_Interp *interp, int release)
{
    static struct st_kw {
        const char *name;
        uintptr_t value;
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
        { "let", LOCALLET },
        { "const", LOCALCONST },
        { "of", OF },
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
        { "iterobj", TYPEITEROBJ },
        { "object", TYPEOBJECT },
        { "boolean", TYPEBOOLEAN },
        { "array", TYPEARRAY },
        { "null", TYPENULL },
        { "export", EXPORT },
        { "...", ELLIPSIS },
        { "debugger", __DEBUG }
    };
    uint i;
    Jsi_HashEntry *hPtr;
    if (release) return JSI_OK;
    if (!interp->lexkeyTbl->numEntries) {
        bool isNew;
        for (i = 0; i < sizeof(keywords) / sizeof(struct st_kw); ++i) {
            hPtr = Jsi_HashEntryNew(interp->lexkeyTbl, keywords[i].name, &isNew);
            assert(hPtr);
            if (hPtr)
                Jsi_HashValueSet(hPtr, (void*)keywords[i].value);
        }
    }
    return JSI_OK;
}

static int jsi_iskey(const char *word, jsi_Lexer *lex)
{
    Jsi_Interp *interp = lex->pstate->interp;
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->lexkeyTbl, word);
    if (hPtr)
        return (uintptr_t)Jsi_HashValueGet(hPtr);
    return 0;
}

static Jsi_String* jsi_do_string(jsi_Lexer *lex)
{
    Jsi_Interp *interp = lex->pstate->interp;
    lex->inStr = 1;
    int n, c = lexer_getchar(lex);
    int endchar = c, isnull = 0;
    uint32_t flags = 0;
    
    int bufi = 0, bsiz, done = 0;
    char unibuf[bsiz=JSI_BUFSIZ], *buf = unibuf, *ret = NULL;
    
    while (!done) {
        if (bufi >= (bsiz-5)) {
            int nsiz = bsiz+=JSI_BUFSIZ;
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
                case '0': buf[bufi++] = 0; isnull=1; break;
                case 'u': {
                    char ibuf[5];
                    int ui;
                    for (ui=0; ui<4; ui++)
                        ibuf[ui] = lexer_getchar(lex);
                    ibuf[4] = 0;
                    ui = Jsi_UtfDecode(ibuf, buf+bufi);
                    if (ui>0) {
                        if (!buf[bufi])
                            isnull = 1;
                        bufi+=ui;
                    } else {
                        Jsi_LogError("Unexpected utf encoding.");
                        buf[bufi++] = 0;
                        goto done;
                    }
                    break;
                }
                case EOF: 
                case 0:
saw_eof:
                    Jsi_LogError("Unexpected EOF parsing string.");
                    buf[bufi++] = 0;
                    goto done;
                case '"':
                case '\'':
                case '`':
                case '\\':
                case '\n':
                    buf[bufi++] = n;
                    break;
                default: 
                    buf[bufi++] = n;
                    if (!interp->unitTest)
                        Jsi_LogParse("Unsupported string escape: \\%c", n);
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
    if (!isnull)
        ret = (char*)Jsi_KeyAdd(lex->pstate->interp, buf);
    else {
        flags |= 1;
        if (buf == unibuf) {
            buf = (char*)Jsi_Malloc(bufi+1);
            memcpy(buf, unibuf, bufi+1);
        }
        ret = buf;
        buf = unibuf;
    }
done:
    if (buf != unibuf)
        Jsi_Free(buf);
    lex->inStr = 0;
    if (!ret)
        return NULL;
    Jsi_String *s = (typeof(s))Jsi_Calloc(1, sizeof(*s));
    s->str = ret;
    s->len = bufi;
    s->flags = flags;
    Jsi_HashSet(lex->pstate->strTbl, s, s);
    return s;
}

static char *jsi_do_regex(jsi_Lexer *lex)
{
    Jsi_Interp *interp = lex->pstate->interp;
    int n, bufi = 0, bsiz;
    char unibuf[bsiz=JSI_BUFSIZ], *buf = unibuf, *ret = NULL;
    
    buf[bufi++] = lexer_getchar(lex);     /* first '/'*/
    while (1) {
        if (bufi >= (bsiz-5)) {
            int nsiz = bsiz+=JSI_BUFSIZ;
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
                Jsi_LogError("Unexpected overflow or EOF parsing regular expression.");
                buf[bufi++] = 0;
                goto done;
            }
            
            buf[bufi++] = c;
            buf[bufi++] = n;
        } else if (c == '/') {
            buf[bufi++] = '/';
            while (1) {
                if (bufi >= (bsiz-5))
                    goto saw_eof;
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
    ret = (char*)Jsi_KeyAdd(lex->pstate->interp, buf);
    ret = Jsi_Strdup(buf);
done:
    if (buf != unibuf)
        Jsi_Free(buf);
    return ret;
}

static int jsi_do_sign(jsi_Lexer *lex)
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
        { ">>", 2, RSHF },
        { "=>", 2, ARROW }
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

static void jsi_eat_comment(jsi_Lexer *lex)
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

static int jsi_yylex (YYSTYPE *yylvalp, YYLTYPE *yyllocp, jsi_Lexer *lex)
{
    int c, c2;
    Jsi_Interp *interp = lex->pstate->interp;
    
    char word[JSI_BUFSIZ];
    int wi = 0;

    while ((c = lexer_getchar(lex)) == ' ' || c == '\t' || c == '\n' || c == '\r');
    LOCATION_START(yyllocp, lex);
    
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

        if (word[0] == '0' && isdigit(word[1]) && base!=16)
            fval = (Jsi_Number)strtoll(word, &eptr, 8);
        else if (word[0] == '0' && (digCnt+(base==16)) == wi)
            fval = (Jsi_Number)strtoll(word, &eptr, base);
        else
            fval = (Jsi_Number)strtod(word, &eptr);
        LOCATION_END(yyllocp, lex);
        if (eptr == NULL || *eptr) {
            Jsi_LogError("invalid number: %s", word);
            return 0;
        }
        Jsi_Number *db = (Jsi_Number *)Jsi_Malloc(sizeof(Jsi_Number));
        *db = fval;
        yylvalp->num = db;
        return FNUMBER;
    } else if (c == '"' || c == '\'' || c == '`') {
        lexer_ungetc(c, lex);
        yylvalp->vstr = jsi_do_string(lex);
        if (!yylvalp->vstr)
            return 0;
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
        int r = jsi_iskey(word, lex);
        if (r) return r;
        yylvalp->sstr = (char*)Jsi_KeyAdd(interp,word);
        LOCATION_END(yyllocp, lex);
        return IDENTIFIER;
    } else if (c == '/') {
        int d = lexer_getchar(lex);
        if (d == '/') {
            while ((d = lexer_getchar(lex)) != '\r' && d != '\n' && d != 0);
            return COMMENT;
        } else if (d == '*') {
            jsi_eat_comment(lex);
            return COMMENT;
        } else lexer_ungetc(d, lex);
        
        if (lex->last_token == '=' || lex->last_token == '(' || lex->last_token == ':'
            || lex->last_token == '?' || lex->last_token == ',' || lex->last_token == 0
            || lex->last_token == '[' || lex->last_token == '{')
        {
            lexer_ungetc(c, lex);
            
            char *regtxt = jsi_do_regex(lex);
            if (!regtxt)
                return 0;
            Jsi_Regex *re = Jsi_RegExpNew(interp, regtxt, JSI_REG_STATIC);
            if (!(yylvalp->regex = re)) {
                 Jsi_Free(regtxt);
                 return -1;
            }
            Jsi_Free(regtxt);
            return REGEXP;
        }
    }
    
    lexer_ungetc(c, lex);
    
    int r = jsi_do_sign(lex);
    LOCATION_END(yyllocp, lex);
    return r;
}

int yylex (YYSTYPE *yylvalp, YYLTYPE *yyllocp, jsi_Pstate *pstate)
{
    int ret;
    do {
        ret = jsi_yylex(yylvalp, yyllocp, pstate->lexer);
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
    Jsi_LogParse("%s:%d.%d: error: %s", interp->framePtr->filePtr->fileName, yylloc->first_line, 
        yylloc->first_column, msg);
    ps->err_count++;
}
#endif
