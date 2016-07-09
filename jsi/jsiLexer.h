#ifndef __JSILEXER_H__
#define __JSILEXER_H__

#define YYSTYPE void *

#ifndef JSI_AMALGAMATION
#include "parser.h"
#include "jsiPstate.h"
#endif

typedef enum {
    LT_NONE,
    LT_FILE,            /* read from file */
    LT_STRING           /* read from a string */
} Jsi_Lexer_Type;

/* Lexer, where input seq provided */
typedef struct Lexer {
    Jsi_Lexer_Type ltype;
    union {
        Jsi_Channel fp;           /* LT_FILE, where to read */
        char *str;          /* LT_STRING */
    } d;
    int last_token;         /* last token returned */
    int ungot, unch[100];
    int cur;                /* LT_STRING, current char */
    int cur_line;           /* current line no. */
    int cur_char;           /* current column no. */
    jsi_Pstate *pstate;
} Lexer;

int yylex (YYSTYPE *yylvalp, YYLTYPE *yyllocp, jsi_Pstate *pstate);
void yyerror(YYLTYPE *yylloc, jsi_Pstate *ps, const char *msg);

#endif /* __JSILEXER_H__ */

