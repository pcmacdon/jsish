/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SRC_PARSER_H_INCLUDED
# define YY_YY_SRC_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDENTIFIER = 258,              /* IDENTIFIER  */
    STRING = 259,                  /* STRING  */
    IF = 260,                      /* IF  */
    ELSE = 261,                    /* ELSE  */
    FOR = 262,                     /* FOR  */
    IN = 263,                      /* IN  */
    WHILE = 264,                   /* WHILE  */
    DO = 265,                      /* DO  */
    CONTINUE = 266,                /* CONTINUE  */
    SWITCH = 267,                  /* SWITCH  */
    CASE = 268,                    /* CASE  */
    DEFAULT = 269,                 /* DEFAULT  */
    BREAK = 270,                   /* BREAK  */
    FUNC = 271,                    /* FUNC  */
    RETURN = 272,                  /* RETURN  */
    LOCAL = 273,                   /* LOCAL  */
    LOCALCONST = 274,              /* LOCALCONST  */
    LOCALLET = 275,                /* LOCALLET  */
    OF = 276,                      /* OF  */
    NEW = 277,                     /* NEW  */
    DELETE = 278,                  /* DELETE  */
    TRY = 279,                     /* TRY  */
    CATCH = 280,                   /* CATCH  */
    FINALLY = 281,                 /* FINALLY  */
    THROW = 282,                   /* THROW  */
    WITH = 283,                    /* WITH  */
    UNDEF = 284,                   /* UNDEF  */
    _TRUE = 285,                   /* _TRUE  */
    _FALSE = 286,                  /* _FALSE  */
    _THIS = 287,                   /* _THIS  */
    ARGUMENTS = 288,               /* ARGUMENTS  */
    FNUMBER = 289,                 /* FNUMBER  */
    REGEXP = 290,                  /* REGEXP  */
    TYPESTRING = 291,              /* TYPESTRING  */
    TYPENUMBER = 292,              /* TYPENUMBER  */
    TYPENULL = 293,                /* TYPENULL  */
    TYPEOBJECT = 294,              /* TYPEOBJECT  */
    TYPEBOOLEAN = 295,             /* TYPEBOOLEAN  */
    TYPEUSEROBJ = 296,             /* TYPEUSEROBJ  */
    TYPEITEROBJ = 297,             /* TYPEITEROBJ  */
    TYPEREGEXP = 298,              /* TYPEREGEXP  */
    TYPEANY = 299,                 /* TYPEANY  */
    TYPEARRAY = 300,               /* TYPEARRAY  */
    ELLIPSIS = 301,                /* ELLIPSIS  */
    EXPORT = 302,                  /* EXPORT  */
    OBJSET = 303,                  /* OBJSET  */
    OBJGET = 304,                  /* OBJGET  */
    ARROW = 305,                   /* ARROW  */
    __DEBUG = 306,                 /* __DEBUG  */
    MIN_PRI = 307,                 /* MIN_PRI  */
    ARGCOMMA = 308,                /* ARGCOMMA  */
    ADDAS = 309,                   /* ADDAS  */
    MNSAS = 310,                   /* MNSAS  */
    MULAS = 311,                   /* MULAS  */
    MODAS = 312,                   /* MODAS  */
    LSHFAS = 313,                  /* LSHFAS  */
    RSHFAS = 314,                  /* RSHFAS  */
    URSHFAS = 315,                 /* URSHFAS  */
    BANDAS = 316,                  /* BANDAS  */
    BORAS = 317,                   /* BORAS  */
    BXORAS = 318,                  /* BXORAS  */
    DIVAS = 319,                   /* DIVAS  */
    OR = 320,                      /* OR  */
    AND = 321,                     /* AND  */
    EQU = 322,                     /* EQU  */
    NEQ = 323,                     /* NEQ  */
    EEQU = 324,                    /* EEQU  */
    NNEQ = 325,                    /* NNEQ  */
    LEQ = 326,                     /* LEQ  */
    GEQ = 327,                     /* GEQ  */
    INSTANCEOF = 328,              /* INSTANCEOF  */
    LSHF = 329,                    /* LSHF  */
    RSHF = 330,                    /* RSHF  */
    URSHF = 331,                   /* URSHF  */
    NEG = 332,                     /* NEG  */
    INC = 333,                     /* INC  */
    DEC = 334,                     /* DEC  */
    TYPEOF = 335,                  /* TYPEOF  */
    VOID = 336,                    /* VOID  */
    MAX_PRI = 337                  /* MAX_PRI  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 18 "src/parser.y"

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

#line 159 "src/parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




int yyparse (struct jsi_Pstate *pstate);


#endif /* !YY_YY_SRC_PARSER_H_INCLUDED  */
