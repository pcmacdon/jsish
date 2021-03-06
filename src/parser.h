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
    LOCALCONST = 274,
    LOCALLET = 275,
    OF = 276,
    NEW = 277,
    DELETE = 278,
    TRY = 279,
    CATCH = 280,
    FINALLY = 281,
    THROW = 282,
    WITH = 283,
    UNDEF = 284,
    _TRUE = 285,
    _FALSE = 286,
    _THIS = 287,
    ARGUMENTS = 288,
    FNUMBER = 289,
    REGEXP = 290,
    TYPESTRING = 291,
    TYPENUMBER = 292,
    TYPENULL = 293,
    TYPEOBJECT = 294,
    TYPEBOOLEAN = 295,
    TYPEUSEROBJ = 296,
    TYPEITEROBJ = 297,
    TYPEREGEXP = 298,
    TYPEANY = 299,
    TYPEARRAY = 300,
    ELLIPSIS = 301,
    EXPORT = 302,
    OBJSET = 303,
    OBJGET = 304,
    ARROW = 305,
    __DEBUG = 306,
    MIN_PRI = 307,
    ARGCOMMA = 308,
    ADDAS = 309,
    MNSAS = 310,
    MULAS = 311,
    MODAS = 312,
    LSHFAS = 313,
    RSHFAS = 314,
    URSHFAS = 315,
    BANDAS = 316,
    BORAS = 317,
    BXORAS = 318,
    DIVAS = 319,
    OR = 320,
    AND = 321,
    EQU = 322,
    NEQ = 323,
    EEQU = 324,
    NNEQ = 325,
    LEQ = 326,
    GEQ = 327,
    INSTANCEOF = 328,
    LSHF = 329,
    RSHF = 330,
    URSHF = 331,
    NEG = 332,
    INC = 333,
    DEC = 334,
    TYPEOF = 335,
    VOID = 336,
    MAX_PRI = 337
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

#line 150 "src/parser.h" /* yacc.c:1909  */
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
