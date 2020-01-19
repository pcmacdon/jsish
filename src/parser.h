/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_YY_SRC_PARSER_H_INCLUDED
# define YY_YY_SRC_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IDENTIFIER = 258,
    STRING = 259,
    IF = 260,
    ELSE = 261,
    FOR = 262,
    IN = 263,
    WHILE = 264,
    DO = 265,
    CONTINUE = 266,
    SWITCH = 267,
    CASE = 268,
    DEFAULT = 269,
    BREAK = 270,
    FUNC = 271,
    RETURN = 272,
    LOCAL = 273,
    OF = 274,
    NEW = 275,
    DELETE = 276,
    TRY = 277,
    CATCH = 278,
    FINALLY = 279,
    THROW = 280,
    WITH = 281,
    UNDEF = 282,
    _TRUE = 283,
    _FALSE = 284,
    _THIS = 285,
    ARGUMENTS = 286,
    FNUMBER = 287,
    REGEXP = 288,
    TYPESTRING = 289,
    TYPENUMBER = 290,
    TYPENULL = 291,
    TYPEOBJECT = 292,
    TYPEBOOLEAN = 293,
    TYPEUSEROBJ = 294,
    TYPEITEROBJ = 295,
    TYPEREGEXP = 296,
    TYPEANY = 297,
    TYPEARRAY = 298,
    ELLIPSIS = 299,
    ARROW = 300,
    __DEBUG = 301,
    MIN_PRI = 302,
    ARGCOMMA = 303,
    ADDAS = 304,
    MNSAS = 305,
    MULAS = 306,
    MODAS = 307,
    LSHFAS = 308,
    RSHFAS = 309,
    URSHFAS = 310,
    BANDAS = 311,
    BORAS = 312,
    BXORAS = 313,
    DIVAS = 314,
    OR = 315,
    AND = 316,
    EQU = 317,
    NEQ = 318,
    EEQU = 319,
    NNEQ = 320,
    LEQ = 321,
    GEQ = 322,
    INSTANCEOF = 323,
    LSHF = 324,
    RSHF = 325,
    URSHF = 326,
    NEG = 327,
    INC = 328,
    DEC = 329,
    TYPEOF = 330,
    VOID = 331,
    MAX_PRI = 332
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 18 "src/parser.y" /* yacc.c:1909  */

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

#line 145 "src/parser.h" /* yacc.c:1909  */
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
