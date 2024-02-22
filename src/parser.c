/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "src/parser.y"

#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#include "jsiCode.c"
#endif

#line 78 "src/parser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENTIFIER = 3,                 /* IDENTIFIER  */
  YYSYMBOL_STRING = 4,                     /* STRING  */
  YYSYMBOL_IF = 5,                         /* IF  */
  YYSYMBOL_ELSE = 6,                       /* ELSE  */
  YYSYMBOL_FOR = 7,                        /* FOR  */
  YYSYMBOL_IN = 8,                         /* IN  */
  YYSYMBOL_WHILE = 9,                      /* WHILE  */
  YYSYMBOL_DO = 10,                        /* DO  */
  YYSYMBOL_CONTINUE = 11,                  /* CONTINUE  */
  YYSYMBOL_SWITCH = 12,                    /* SWITCH  */
  YYSYMBOL_CASE = 13,                      /* CASE  */
  YYSYMBOL_DEFAULT = 14,                   /* DEFAULT  */
  YYSYMBOL_BREAK = 15,                     /* BREAK  */
  YYSYMBOL_FUNC = 16,                      /* FUNC  */
  YYSYMBOL_RETURN = 17,                    /* RETURN  */
  YYSYMBOL_LOCAL = 18,                     /* LOCAL  */
  YYSYMBOL_LOCALCONST = 19,                /* LOCALCONST  */
  YYSYMBOL_LOCALLET = 20,                  /* LOCALLET  */
  YYSYMBOL_OF = 21,                        /* OF  */
  YYSYMBOL_NEW = 22,                       /* NEW  */
  YYSYMBOL_DELETE = 23,                    /* DELETE  */
  YYSYMBOL_TRY = 24,                       /* TRY  */
  YYSYMBOL_CATCH = 25,                     /* CATCH  */
  YYSYMBOL_FINALLY = 26,                   /* FINALLY  */
  YYSYMBOL_THROW = 27,                     /* THROW  */
  YYSYMBOL_WITH = 28,                      /* WITH  */
  YYSYMBOL_UNDEF = 29,                     /* UNDEF  */
  YYSYMBOL__TRUE = 30,                     /* _TRUE  */
  YYSYMBOL__FALSE = 31,                    /* _FALSE  */
  YYSYMBOL__THIS = 32,                     /* _THIS  */
  YYSYMBOL_ARGUMENTS = 33,                 /* ARGUMENTS  */
  YYSYMBOL_FNUMBER = 34,                   /* FNUMBER  */
  YYSYMBOL_REGEXP = 35,                    /* REGEXP  */
  YYSYMBOL_TYPESTRING = 36,                /* TYPESTRING  */
  YYSYMBOL_TYPENUMBER = 37,                /* TYPENUMBER  */
  YYSYMBOL_TYPENULL = 38,                  /* TYPENULL  */
  YYSYMBOL_TYPEOBJECT = 39,                /* TYPEOBJECT  */
  YYSYMBOL_TYPEBOOLEAN = 40,               /* TYPEBOOLEAN  */
  YYSYMBOL_TYPEUSEROBJ = 41,               /* TYPEUSEROBJ  */
  YYSYMBOL_TYPEITEROBJ = 42,               /* TYPEITEROBJ  */
  YYSYMBOL_TYPEREGEXP = 43,                /* TYPEREGEXP  */
  YYSYMBOL_TYPEANY = 44,                   /* TYPEANY  */
  YYSYMBOL_TYPEARRAY = 45,                 /* TYPEARRAY  */
  YYSYMBOL_ELLIPSIS = 46,                  /* ELLIPSIS  */
  YYSYMBOL_EXPORT = 47,                    /* EXPORT  */
  YYSYMBOL_OBJSET = 48,                    /* OBJSET  */
  YYSYMBOL_OBJGET = 49,                    /* OBJGET  */
  YYSYMBOL_ARROW = 50,                     /* ARROW  */
  YYSYMBOL___DEBUG = 51,                   /* __DEBUG  */
  YYSYMBOL_MIN_PRI = 52,                   /* MIN_PRI  */
  YYSYMBOL_53_ = 53,                       /* ','  */
  YYSYMBOL_ARGCOMMA = 54,                  /* ARGCOMMA  */
  YYSYMBOL_55_ = 55,                       /* '='  */
  YYSYMBOL_ADDAS = 56,                     /* ADDAS  */
  YYSYMBOL_MNSAS = 57,                     /* MNSAS  */
  YYSYMBOL_MULAS = 58,                     /* MULAS  */
  YYSYMBOL_MODAS = 59,                     /* MODAS  */
  YYSYMBOL_LSHFAS = 60,                    /* LSHFAS  */
  YYSYMBOL_RSHFAS = 61,                    /* RSHFAS  */
  YYSYMBOL_URSHFAS = 62,                   /* URSHFAS  */
  YYSYMBOL_BANDAS = 63,                    /* BANDAS  */
  YYSYMBOL_BORAS = 64,                     /* BORAS  */
  YYSYMBOL_BXORAS = 65,                    /* BXORAS  */
  YYSYMBOL_DIVAS = 66,                     /* DIVAS  */
  YYSYMBOL_67_ = 67,                       /* '?'  */
  YYSYMBOL_68_ = 68,                       /* ':'  */
  YYSYMBOL_OR = 69,                        /* OR  */
  YYSYMBOL_AND = 70,                       /* AND  */
  YYSYMBOL_71_ = 71,                       /* '|'  */
  YYSYMBOL_72_ = 72,                       /* '^'  */
  YYSYMBOL_73_ = 73,                       /* '&'  */
  YYSYMBOL_EQU = 74,                       /* EQU  */
  YYSYMBOL_NEQ = 75,                       /* NEQ  */
  YYSYMBOL_EEQU = 76,                      /* EEQU  */
  YYSYMBOL_NNEQ = 77,                      /* NNEQ  */
  YYSYMBOL_78_ = 78,                       /* '>'  */
  YYSYMBOL_79_ = 79,                       /* '<'  */
  YYSYMBOL_LEQ = 80,                       /* LEQ  */
  YYSYMBOL_GEQ = 81,                       /* GEQ  */
  YYSYMBOL_INSTANCEOF = 82,                /* INSTANCEOF  */
  YYSYMBOL_LSHF = 83,                      /* LSHF  */
  YYSYMBOL_RSHF = 84,                      /* RSHF  */
  YYSYMBOL_URSHF = 85,                     /* URSHF  */
  YYSYMBOL_86_ = 86,                       /* '+'  */
  YYSYMBOL_87_ = 87,                       /* '-'  */
  YYSYMBOL_88_ = 88,                       /* '*'  */
  YYSYMBOL_89_ = 89,                       /* '/'  */
  YYSYMBOL_90_ = 90,                       /* '%'  */
  YYSYMBOL_NEG = 91,                       /* NEG  */
  YYSYMBOL_92_ = 92,                       /* '!'  */
  YYSYMBOL_INC = 93,                       /* INC  */
  YYSYMBOL_DEC = 94,                       /* DEC  */
  YYSYMBOL_95_ = 95,                       /* '~'  */
  YYSYMBOL_TYPEOF = 96,                    /* TYPEOF  */
  YYSYMBOL_VOID = 97,                      /* VOID  */
  YYSYMBOL_98_ = 98,                       /* '.'  */
  YYSYMBOL_99_ = 99,                       /* '['  */
  YYSYMBOL_100_ = 100,                     /* '('  */
  YYSYMBOL_MAX_PRI = 101,                  /* MAX_PRI  */
  YYSYMBOL_102_ = 102,                     /* ';'  */
  YYSYMBOL_103_ = 103,                     /* '{'  */
  YYSYMBOL_104_ = 104,                     /* '}'  */
  YYSYMBOL_105_ = 105,                     /* ')'  */
  YYSYMBOL_106_ = 106,                     /* ']'  */
  YYSYMBOL_YYACCEPT = 107,                 /* $accept  */
  YYSYMBOL_file = 108,                     /* file  */
  YYSYMBOL_statements = 109,               /* statements  */
  YYSYMBOL_statement = 110,                /* statement  */
  YYSYMBOL_localvar = 111,                 /* localvar  */
  YYSYMBOL_objexport = 112,                /* objexport  */
  YYSYMBOL_commonstatement = 113,          /* commonstatement  */
  YYSYMBOL_func_statement = 114,           /* func_statement  */
  YYSYMBOL_func_prefix = 115,              /* func_prefix  */
  YYSYMBOL_iterstatement = 116,            /* iterstatement  */
  YYSYMBOL_identifier_opt = 117,           /* identifier_opt  */
  YYSYMBOL_label_opt = 118,                /* label_opt  */
  YYSYMBOL_statement_or_empty = 119,       /* statement_or_empty  */
  YYSYMBOL_with_statement = 120,           /* with_statement  */
  YYSYMBOL_switch_statement = 121,         /* switch_statement  */
  YYSYMBOL_cases = 122,                    /* cases  */
  YYSYMBOL_case = 123,                     /* case  */
  YYSYMBOL_try_statement = 124,            /* try_statement  */
  YYSYMBOL_vardecs = 125,                  /* vardecs  */
  YYSYMBOL_vardec = 126,                   /* vardec  */
  YYSYMBOL_delete_statement = 127,         /* delete_statement  */
  YYSYMBOL_if_statement = 128,             /* if_statement  */
  YYSYMBOL_inof = 129,                     /* inof  */
  YYSYMBOL_for_statement = 130,            /* for_statement  */
  YYSYMBOL_for_init = 131,                 /* for_init  */
  YYSYMBOL_for_cond = 132,                 /* for_cond  */
  YYSYMBOL_expr_opt = 133,                 /* expr_opt  */
  YYSYMBOL_while_statement = 134,          /* while_statement  */
  YYSYMBOL_do_statement = 135,             /* do_statement  */
  YYSYMBOL_func_expr = 136,                /* func_expr  */
  YYSYMBOL_args_opt = 137,                 /* args_opt  */
  YYSYMBOL_typeid = 138,                   /* typeid  */
  YYSYMBOL_rettype = 139,                  /* rettype  */
  YYSYMBOL_argtype = 140,                  /* argtype  */
  YYSYMBOL_strlit = 141,                   /* strlit  */
  YYSYMBOL_argdefault = 142,               /* argdefault  */
  YYSYMBOL_args = 143,                     /* args  */
  YYSYMBOL_argsa = 144,                    /* argsa  */
  YYSYMBOL_arrowargs = 145,                /* arrowargs  */
  YYSYMBOL_func_statement_block = 146,     /* func_statement_block  */
  YYSYMBOL_expr = 147,                     /* expr  */
  YYSYMBOL_fcall_exprs = 148,              /* fcall_exprs  */
  YYSYMBOL_lvalue = 149,                   /* lvalue  */
  YYSYMBOL_exprlist_opt = 150,             /* exprlist_opt  */
  YYSYMBOL_exprlist = 151,                 /* exprlist  */
  YYSYMBOL_value = 152,                    /* value  */
  YYSYMBOL_object = 153,                   /* object  */
  YYSYMBOL_itemfunc = 154,                 /* itemfunc  */
  YYSYMBOL_itemident = 155,                /* itemident  */
  YYSYMBOL_items = 156,                    /* items  */
  YYSYMBOL_itemres = 157,                  /* itemres  */
  YYSYMBOL_item = 158,                     /* item  */
  YYSYMBOL_array = 159                     /* array  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  145
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3084

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  53
/* YYNRULES -- Number of rules.  */
#define YYNRULES  271
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  520

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   337


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    92,     2,     2,     2,    90,    73,     2,
     100,   105,    88,    86,    53,    87,    98,    89,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    68,   102,
      79,    55,    78,    67,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    99,     2,   106,    72,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   103,    71,   104,    95,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    54,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      69,    70,    74,    75,    76,    77,    80,    81,    82,    83,
      84,    85,    91,    93,    94,    96,    97,   101
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   128,   128,   129,   132,   135,   140,   141,   146,   147,
     148,   152,   153,   154,   158,   159,   165,   168,   174,   175,
     176,   177,   178,   179,   180,   181,   185,   186,   187,   188,
     189,   190,   191,   195,   204,   215,   224,   225,   226,   227,
     230,   231,   234,   235,   241,   242,   246,   252,   253,   306,
     307,   311,   312,   313,   314,   318,   325,   332,   342,   343,
     347,   356,   368,   378,   382,   391,   392,   396,   407,   427,
     451,   452,   453,   459,   460,   463,   464,   468,   478,   488,
     492,   497,   501,   508,   509,   512,   517,   525,   526,   527,
     528,   529,   530,   531,   532,   533,   534,   535,   536,   537,
     540,   548,   551,   555,   558,   559,   560,   561,   562,   563,
     564,   565,   568,   569,   570,   571,   572,   573,   574,   575,
     579,   580,   584,   585,   586,   589,   590,   594,   595,   596,
     600,   601,   602,   603,   604,   605,   606,   607,   608,   609,
     610,   611,   612,   613,   614,   618,   622,   626,   630,   634,
     635,   640,   645,   649,   653,   654,   655,   656,   657,   658,
     659,   660,   661,   662,   663,   664,   665,   666,   667,   668,
     669,   670,   671,   672,   673,   674,   675,   676,   677,   678,
     679,   680,   682,   683,   686,   687,   688,   694,   702,   707,
     712,   717,   721,   725,   734,   740,   745,   750,   778,   784,
     785,   786,   791,   800,   811,   812,   816,   817,   822,   827,
     835,   836,   837,   838,   839,   840,   841,   842,   843,   847,
     851,   862,   863,   870,   874,   875,   876,   881,   888,   889,
     890,   891,   892,   893,   894,   895,   896,   897,   898,   899,
     900,   901,   902,   903,   904,   905,   906,   907,   908,   909,
     910,   911,   912,   913,   914,   915,   916,   917,   918,   922,
     923,   924,   925,   926,   927,   928,   929,   930,   935,   944,
     945,   946
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENTIFIER", "STRING",
  "IF", "ELSE", "FOR", "IN", "WHILE", "DO", "CONTINUE", "SWITCH", "CASE",
  "DEFAULT", "BREAK", "FUNC", "RETURN", "LOCAL", "LOCALCONST", "LOCALLET",
  "OF", "NEW", "DELETE", "TRY", "CATCH", "FINALLY", "THROW", "WITH",
  "UNDEF", "_TRUE", "_FALSE", "_THIS", "ARGUMENTS", "FNUMBER", "REGEXP",
  "TYPESTRING", "TYPENUMBER", "TYPENULL", "TYPEOBJECT", "TYPEBOOLEAN",
  "TYPEUSEROBJ", "TYPEITEROBJ", "TYPEREGEXP", "TYPEANY", "TYPEARRAY",
  "ELLIPSIS", "EXPORT", "OBJSET", "OBJGET", "ARROW", "__DEBUG", "MIN_PRI",
  "','", "ARGCOMMA", "'='", "ADDAS", "MNSAS", "MULAS", "MODAS", "LSHFAS",
  "RSHFAS", "URSHFAS", "BANDAS", "BORAS", "BXORAS", "DIVAS", "'?'", "':'",
  "OR", "AND", "'|'", "'^'", "'&'", "EQU", "NEQ", "EEQU", "NNEQ", "'>'",
  "'<'", "LEQ", "GEQ", "INSTANCEOF", "LSHF", "RSHF", "URSHF", "'+'", "'-'",
  "'*'", "'/'", "'%'", "NEG", "'!'", "INC", "DEC", "'~'", "TYPEOF", "VOID",
  "'.'", "'['", "'('", "MAX_PRI", "';'", "'{'", "'}'", "')'", "']'",
  "$accept", "file", "statements", "statement", "localvar", "objexport",
  "commonstatement", "func_statement", "func_prefix", "iterstatement",
  "identifier_opt", "label_opt", "statement_or_empty", "with_statement",
  "switch_statement", "cases", "case", "try_statement", "vardecs",
  "vardec", "delete_statement", "if_statement", "inof", "for_statement",
  "for_init", "for_cond", "expr_opt", "while_statement", "do_statement",
  "func_expr", "args_opt", "typeid", "rettype", "argtype", "strlit",
  "argdefault", "args", "argsa", "arrowargs", "func_statement_block",
  "expr", "fcall_exprs", "lvalue", "exprlist_opt", "exprlist", "value",
  "object", "itemfunc", "itemident", "items", "itemres", "item", "array", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-417)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-254)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     521,   -22,  -417,   -45,    88,    88,     8,  1830,  -417,  -417,
    -417,   288,    24,    -8,  2038,    14,  -417,  -417,  -417,  -417,
    -417,  -417,  -417,  -417,    91,  -417,  2038,  2038,  2038,    24,
      24,  2038,  2038,  2038,   397,   676,  -417,   779,   121,   558,
    -417,   126,  -417,  -417,    31,  -417,   174,  -417,  -417,  -417,
    -417,  -417,  -417,  -417,  -417,    38,  -417,    94,  2735,    58,
     381,  -417,  -417,  -417,  1669,  2038,  -417,    80,    83,  -417,
      17,   127,    12,  -417,  2606,  2788,  -417,  2038,   141,   140,
     142,    70,  1428,    71,  2827,  2038,    16,    89,    89,    89,
     135,   135,    89,    89,    89,  1915,  -417,   769,   -35,   123,
    -417,  2181,   -20,    10,  -417,  -417,    13,     8,  1830,   175,
     179,   193,   288,    24,  2038,    14,   195,   196,   197,   199,
     202,   204,  -417,  -417,   212,  -417,  -417,  -417,  -417,  -417,
    -417,  -417,    91,   287,   296,   214,  -417,  2038,  2038,  1530,
     232,  2735,   -27,   233,  -417,  -417,  -417,  2735,   252,   -39,
    -417,    17,   209,   213,  1706,   216,  1915,  2061,  2038,  2038,
    2038,  2038,  2038,  2038,  2038,  2038,  2038,  2038,  2038,  2038,
    2038,  2038,  2038,  2038,  2038,  2038,  2038,  2038,  2038,  2038,
    2038,  2038,   309,  2038,  -417,  1915,  2038,  2038,  2038,  2038,
    2038,  2038,  2038,  2038,  2038,  2038,  2038,  2038,  2038,  -417,
    -417,  2750,  2038,  1915,  -417,  2220,  -417,  -417,    17,    48,
     226,   231,   261,   245,   278,  -417,  -417,  -417,  -417,  -417,
    -417,  -417,  -417,  -417,  -417,  -417,   195,   196,   197,  -417,
    -417,   204,   212,  -417,  -417,  -417,  -417,   232,  -417,  2264,
    1915,  1915,  1915,  -417,  -417,  1567,   247,    -8,  -417,  2303,
    -417,  -417,  -417,  -417,  -417,  -417,   643,  -417,   345,   250,
    1669,  2038,  2038,  2038,  2038,  2038,   251,   256,  -417,  2038,
    2678,  -417,  2038,  2038,   126,  -417,   249,  1807,  2038,   881,
    -417,   348,  2038,   255,   306,   983,  -417,  1280,   279,   769,
    1076,   872,   974,    76,   254,  1799,   170,   170,   170,   170,
     279,   279,   279,   279,    47,    47,    47,   188,   188,    89,
      89,    89,   270,  2096,   266,   769,   769,   769,   769,   769,
     769,   769,   769,   769,   769,   769,   769,   279,  -417,  -417,
    -417,  -417,  2134,   268,  1706,   271,    45,   590,   277,   -17,
      22,  2038,   280,   274,   276,   281,  -417,   382,  -417,  1706,
    1953,  -417,  -417,   769,  -417,   -36,  1915,  2910,   769,   769,
     769,   769,   769,   387,   289,   769,    26,  -417,  -417,  -417,
     769,   769,  -417,   -15,  -417,   389,  2038,  2871,  2984,  2347,
    -417,   284,  2386,  -417,  2038,  1915,   293,  -417,  -417,  -417,
     390,   -14,  -417,  -417,  -417,  -417,  -417,   361,  -417,  -417,
    -417,  -417,  -417,  -417,  -417,  -417,  -417,  -417,  -417,  -417,
    -417,  -417,  -417,  -417,  -417,   -26,  -417,   590,  -417,    75,
     300,   769,  1915,  -417,  -417,  -417,   294,  -417,   399,  -417,
     298,   299,    -8,    17,   590,  -417,    72,   -10,   303,  1280,
    -417,  -417,  -417,  2038,  1706,  2038,   304,   769,   301,  1915,
    1706,   590,  -417,  -417,    45,   590,    -8,   337,    45,   590,
     312,   307,    -8,  -417,  -417,    -8,  -417,   310,    -8,  2038,
    -417,  2038,  2430,  -417,  2469,    18,  -417,   311,  -417,    -8,
    -417,  -417,  -417,  -417,   -19,  -417,  -417,   385,  -417,    -8,
    -417,  2513,   315,  1280,  1706,  -417,  2038,   346,  -417,    21,
    -417,  -417,  -417,    45,    -8,  -417,  1706,  1706,  -417,  1178,
    1085,  -417,  -417,  -417,  -417,  -417,  -417,  1187,  1289,  1391
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
      42,   198,   103,     0,    40,    40,     0,     0,    11,    13,
      12,     0,     0,     0,     0,     0,   212,   213,   214,   200,
     199,   215,   216,   211,     0,   192,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    29,    42,     0,    42,
       6,     0,     9,    31,     0,     8,     0,    28,    39,    27,
      20,    19,    36,    37,    38,   128,   210,     0,     5,   181,
     129,   127,   217,   218,    43,     0,    41,     0,     0,    35,
      83,   198,     0,    24,   224,     0,   198,     0,   185,   183,
     182,     0,    42,     0,     0,     0,     0,   134,   133,   136,
     146,   148,   135,   147,   137,     0,   271,   206,     0,   198,
     124,     0,   198,    40,   235,   234,    40,   256,   231,    11,
      13,    12,   248,   247,   233,   255,   212,   213,   214,   200,
     199,   215,   239,   240,   211,   254,   241,   244,   243,   242,
     237,   238,   236,   228,   229,   192,   258,   257,   252,    42,
     210,     0,     0,     0,   225,     1,     7,     4,    60,     0,
      58,    83,     0,     0,    42,     0,   204,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,   204,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   144,
     145,     0,     0,   204,    10,     0,    22,    21,    83,   112,
       0,     0,    84,     0,     0,   230,   232,   256,   231,   249,
     251,   250,   248,   247,   233,   255,     0,     0,     0,   246,
     245,     0,     0,   236,   253,   257,   252,     0,    23,     0,
     204,   204,   204,    62,   126,    42,     0,     0,    26,     0,
      15,    17,    16,    32,    14,   209,     0,   269,     0,   149,
      43,     0,     0,     0,     0,     0,     0,     0,    30,     0,
     227,   219,     0,     0,     0,    25,     0,     0,     0,    42,
      44,     0,     0,     0,   205,    42,   151,   150,   143,   130,
       0,   153,   152,   163,   164,   162,   158,   159,   160,   161,
     155,   154,   156,   157,   165,   166,   167,   141,   142,   138,
     139,   140,   132,     0,     0,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   202,   228,
     229,   203,     0,     0,    42,     0,     0,     0,     0,     0,
       0,     0,   184,     0,     0,     0,   125,     0,    56,    42,
       0,   208,   270,   207,   120,     0,   204,   259,   265,   263,
     264,   262,   266,     0,     0,   260,   222,   221,   226,   223,
     261,    61,    59,     0,    70,     0,    73,     0,   129,     0,
      45,     0,     0,   190,     0,   204,   131,   195,   201,   197,
      63,     0,   104,   108,   109,   107,   110,     0,   105,   111,
     113,    97,    98,    87,    88,    96,    91,    89,    92,    93,
      90,    94,    95,    99,   101,   114,    85,     0,    79,   116,
       0,   259,   204,   189,   187,   186,     0,    46,     0,   123,
       0,     0,     0,    83,     0,    34,    60,     0,     0,    74,
      71,    65,    66,     0,    42,     0,     0,   191,     0,   204,
      42,     0,    81,   106,     0,     0,     0,   100,     0,     0,
       0,     0,     0,   121,   196,     0,   268,     0,     0,     0,
      72,    75,     0,    77,     0,     0,   193,     0,    64,     0,
     115,   102,    80,   117,   118,    86,   188,    55,   267,     0,
      33,     0,     0,    76,    42,    78,     0,     0,    47,     0,
      49,   194,    82,     0,     0,   220,    42,    42,    69,     0,
      42,    48,    50,   119,    57,    68,    67,    42,    42,    42
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -417,  -417,     3,     0,   145,  -417,   -43,  -417,  -417,  -417,
       5,  -417,  -327,  -417,  -417,  -417,   -76,  -417,    49,   159,
    -417,  -417,    -2,  -417,  -417,  -417,  -417,  -417,  -417,    -3,
    -150,    -4,  -361,  -335,   -33,  -416,  -417,  -417,  -417,  -107,
      33,  -417,    -6,  -141,   -25,     1,   363,  -417,  -417,  -417,
     253,   182,  -417
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    38,   139,   280,    41,   253,    42,    43,    44,    45,
      67,    46,   281,    47,    48,   499,   500,    49,   149,   150,
      50,    51,   443,    52,   376,   438,   492,    53,    54,    55,
     211,   414,   456,   457,    56,   400,   212,   355,    57,    83,
     141,    59,    60,   283,   284,    61,    62,   367,   368,   142,
     143,   144,    63
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      40,   276,   415,    39,   140,    79,    81,   390,    78,    98,
      68,    69,    80,    66,   274,   213,    66,   428,   256,   250,
     209,   204,   427,    90,    91,   419,   270,    76,  -122,   454,
    -122,   496,   497,    58,   496,   497,   503,    40,   480,   146,
      75,   237,   483,   274,   314,   455,    64,    84,   260,     2,
     286,   417,   455,   434,   451,    65,    19,    20,   335,    87,
      88,    89,   333,   275,    92,    93,    94,    97,   101,   429,
     255,   257,   147,   468,   392,   393,   394,   271,  -230,   395,
     441,  -232,    40,   396,   158,   245,    82,   513,    82,    82,
     479,    66,   470,   442,   341,    82,   246,   247,   205,   343,
     344,   345,   251,   336,   252,    86,    79,    81,    70,    78,
     239,    68,    70,    80,    85,   210,   337,   473,   249,    74,
     420,   145,   498,   478,   484,   511,   433,   273,    97,   148,
     458,   151,   397,   177,   178,   179,   180,   181,   156,   146,
     348,    75,   398,   459,   157,   182,   183,    84,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   185,   174,
     175,   176,   177,   178,   179,   180,   181,   508,   201,   202,
      93,    94,   243,  -122,   182,   183,   258,  -122,   158,   515,
     516,   152,   206,   153,   154,   207,   155,   182,   183,    97,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   430,   313,   204,    97,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   418,   201,   202,   332,    97,   237,   201,   202,
     241,   240,   242,  -249,   448,   146,   140,  -251,   170,   171,
     172,   173,   140,   174,   175,   176,   177,   178,   179,   180,
     181,  -250,   158,   261,   262,   263,   435,  -246,   182,   183,
    -245,   378,   264,    97,    97,    97,   179,   180,   181,    40,
     265,   461,  -253,   467,   452,    40,   182,   183,   245,   353,
     266,    76,     2,   357,   358,   359,   360,   361,   362,   267,
     269,   272,   365,   399,    72,   370,   371,   273,   477,   277,
     377,   379,   312,   278,   340,   382,   282,    16,    17,    18,
      19,    20,    21,    22,   338,   466,    23,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   339,   174,   175,   176,
     177,   178,   179,   180,   181,   208,   341,   347,   354,   482,
     356,   363,   182,   183,   373,   487,   364,   381,   488,   350,
     383,   490,   174,   175,   176,   177,   178,   179,   180,   181,
     385,   387,   502,   389,   421,   416,   391,   182,   183,   423,
     422,   424,   505,   353,   445,   426,   425,    34,    77,    97,
     431,    74,   436,   449,   432,   453,   450,   514,   460,   462,
      71,     2,   463,   464,   465,   471,   476,   475,   455,   439,
     485,   504,   486,    72,   510,   489,   501,   447,    97,    11,
     507,   399,   375,   512,   437,   399,    16,    17,    18,    19,
      20,    21,    22,   372,   469,    23,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    25,   254,
      95,   481,   369,     0,   331,    97,     0,     0,     0,     0,
       0,     0,     0,   198,     0,     0,     0,     0,     0,     0,
     399,     0,     0,     0,   199,   200,   472,     0,   474,   201,
     202,   203,    97,    26,    27,     0,     0,     0,     0,    28,
      29,    30,    31,    32,    33,     0,    34,    35,     0,     0,
      74,     0,   491,    96,   493,     0,     0,     0,     0,     0,
      40,     0,     0,   518,     0,     0,     0,    40,   146,   146,
     519,    -2,     0,     0,     1,     2,     3,     0,     0,   509,
       0,     0,     4,     0,     0,     0,     5,     6,     7,     8,
       9,    10,     0,    11,    12,    13,     0,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,    -3,    23,
       0,     1,     2,     3,     0,     0,     0,     0,    24,     4,
       0,     0,    25,     5,     6,     7,     8,     9,    10,     0,
      11,    12,    13,     0,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,     0,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,    24,   401,    26,    27,    25,
       0,     0,     0,    28,    29,    30,    31,    32,    33,   402,
      34,    35,     0,    36,    37,     0,   403,   404,   405,   406,
     407,   408,   409,   410,   411,   412,     0,     0,     0,     0,
       0,     0,     0,     0,    26,    27,    71,     2,     0,     0,
      28,    29,    30,    31,    32,    33,     0,    34,    35,    72,
      36,    37,     0,     0,     0,    11,     0,     0,     0,     0,
       0,     0,    16,    17,    18,    19,    20,    21,    22,    99,
       2,    23,     0,     0,     0,     0,     0,   413,     0,     0,
       0,     0,    72,     0,    25,     0,   351,     0,    11,     0,
       0,     0,     0,     0,     0,    16,    17,    18,    19,    20,
      21,    22,     0,     0,    23,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    25,     0,    26,
      27,     0,     0,     0,     0,    28,    29,    30,    31,    32,
      33,     0,    34,    35,     0,     0,    74,     0,     0,   352,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    26,    27,     0,     0,     0,     0,    28,    29,
      30,    31,    32,    33,     0,    34,    35,   158,     0,    74,
       0,   100,   102,     2,     3,     0,     0,     0,     0,     0,
     103,     0,   104,   105,   106,   107,   108,   109,   110,   111,
       0,   112,   113,    13,     0,     0,   114,   115,   116,   117,
     118,   119,   120,   121,    22,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   132,   133,   134,     0,
     135,     0,  -224,     0,     0,     0,   160,     0,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,     0,   174,   175,   176,   177,   178,   179,   180,   181,
       0,   136,     0,     0,     0,    26,    27,   182,   183,     0,
       0,    28,    29,    30,    31,   137,   138,     0,    34,    35,
     158,    36,    37,  -224,   102,     2,     3,     0,     0,     0,
       0,     0,   103,     0,   104,   105,   106,   107,   108,   109,
     110,   111,     0,   112,   113,    13,     0,     0,   114,   115,
     116,   117,   118,   119,   120,   121,    22,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,     0,   132,   133,
     134,     0,   135,     0,  -224,     0,     0,     0,     0,     0,
       0,     0,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,     0,   174,   175,   176,   177,   178,
     179,   180,   181,   136,     0,     0,     0,    26,    27,     0,
     182,   183,     0,    28,    29,    30,    31,   137,   138,     0,
      34,    35,   158,    36,    37,   380,   102,     2,     3,     0,
       0,     0,     0,     0,   103,     0,   104,   105,   106,   107,
     108,   109,   110,   111,     0,   112,   113,    13,     0,     0,
     114,   115,   116,   117,   118,   119,   120,   121,    22,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,     0,
     132,   133,   134,     0,   135,     0,  -224,     0,     0,     0,
       0,     0,     0,     0,     0,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,     0,   174,   175,   176,
     177,   178,   179,   180,   181,   136,     0,     0,     0,    26,
      27,     0,   182,   183,     0,    28,    29,    30,    31,   137,
     138,     0,    34,    35,   158,    36,    37,   244,     1,     2,
       3,     0,     0,     0,     0,     0,     4,     0,   -53,   -53,
       5,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,     0,    23,     0,     0,     0,     0,     0,   159,
       0,     0,    24,     0,     0,     0,    25,     0,     0,     0,
       0,     0,     0,   160,   384,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,     0,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,    26,    27,     0,   182,   183,     0,    28,    29,    30,
      31,    32,    33,     0,    34,    35,   158,    36,    37,   -53,
       1,     2,     3,     0,     0,     0,     0,     0,     4,     0,
     -54,   -54,     5,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,     0,    23,     0,     0,     0,     0,
       0,   159,     0,     0,    24,     0,     0,     0,    25,     0,
       0,     0,     0,     0,     0,   160,   517,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
       0,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,    26,    27,     0,   182,   183,     0,    28,
      29,    30,    31,    32,    33,     0,    34,    35,   158,    36,
      37,   -54,     1,     2,     3,     0,     0,     0,     0,     0,
       4,     0,   -52,   -52,     5,     6,     7,     8,     9,    10,
       0,    11,    12,    13,     0,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,     0,    23,     0,     0,
       0,     0,     0,   159,     0,     0,    24,     0,     0,     0,
      25,     0,     0,     0,     0,     0,     0,   160,     0,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,     0,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,     0,     0,    26,    27,     0,   182,   183,
       0,    28,    29,    30,    31,    32,    33,     0,    34,    35,
       0,    36,    37,   -52,     1,     2,     3,     0,     0,     0,
       0,     0,     4,     0,   -51,   -51,     5,     6,     7,     8,
       9,    10,     0,    11,    12,    13,     0,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,     0,    23,
       0,     1,     2,     3,     0,     0,     0,     0,    24,     4,
       0,     0,    25,     5,     6,     7,     8,     9,    10,     0,
      11,    12,    13,     0,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,     0,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,    24,     0,    26,    27,    25,
       0,     0,     0,    28,    29,    30,    31,    32,    33,     0,
      34,    35,     0,    36,    37,   -51,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    26,    27,     0,     0,     0,     0,
      28,    29,    30,    31,    32,    33,     0,    34,    35,     0,
      36,    37,   244,     1,     2,     3,     0,     0,     0,     0,
       0,     4,     0,     0,     0,     5,     6,     7,     8,     9,
      10,     0,    11,    12,    13,     0,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,     0,     0,    23,     0,
       1,     2,     3,     0,     0,     0,     0,    24,     4,     0,
       0,    25,     5,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,     0,    23,     0,     0,     0,     0,
       0,     0,     0,     0,    24,     0,    26,    27,    25,     0,
       0,     0,    28,    29,    30,    31,    32,    33,     0,    34,
      35,     0,    36,    37,   268,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    26,    27,     0,     0,     0,     0,    28,
      29,    30,    31,    32,    33,     0,    34,    35,     0,    36,
      37,   346,    71,     2,     3,     0,     0,     0,     0,     0,
       4,     0,     0,     0,     5,     6,     7,     8,     9,    10,
       0,    11,    12,    13,     0,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,     0,    23,     0,     1,
       2,     3,     0,     0,     0,     0,    24,     4,     0,     0,
      25,     5,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,     0,     0,    23,     0,     0,     0,     0,     0,
       0,     0,     0,    24,     0,    26,    27,    25,     0,     0,
       0,    28,    29,    30,    31,    32,    33,     0,    34,    35,
       0,    36,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    26,    27,     0,     0,     0,     0,    28,    29,
      30,    31,    32,    33,     0,    34,    35,   158,    36,   279,
      71,     2,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,     0,     8,     9,    10,     0,    11,
       0,     0,     0,    71,     2,     0,    16,    17,    18,    19,
      20,    21,    22,     0,     0,    23,    72,     0,     0,     0,
       0,     0,    11,     0,     0,     0,     0,     0,    25,    16,
      17,    18,    19,    20,    21,    22,     0,     0,    23,     0,
       0,     0,     0,   166,   167,   168,   169,   170,   171,   172,
     173,    25,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,    26,    27,     0,     0,   182,   183,    28,
      29,    30,    31,    32,    33,     0,    34,    35,     0,   374,
      74,     0,     0,     0,     0,     0,    26,    27,    71,     2,
       0,     0,    28,    29,    30,    31,    32,    33,     0,    34,
      35,    72,    73,    74,     0,     0,     0,    11,     0,     0,
       0,     0,     0,     0,    16,    17,    18,    19,    20,    21,
      22,     0,     0,    23,     0,     0,    71,     2,     0,     0,
       0,     0,     0,     0,     0,     0,    25,     0,    95,    72,
       0,     0,     0,     0,     0,    11,     0,     0,     0,     0,
       0,     0,    16,    17,    18,    19,    20,    21,    22,     0,
       0,    23,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    26,    27,     0,    25,     0,   351,    28,    29,    30,
      31,    32,    33,     0,    34,    35,     0,     0,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    26,
      27,    71,     2,     0,     0,    28,    29,    30,    31,    32,
      33,     0,    34,    35,    72,     0,    74,     0,     0,     0,
      11,     0,     0,     0,    71,     2,     0,    16,    17,    18,
      19,    20,    21,    22,     0,     0,    23,    72,     0,     0,
       0,     0,     0,    11,     0,     0,     0,     0,     0,    25,
      16,    17,    18,    19,    20,    21,    22,     0,     0,    23,
       0,     0,     0,     0,   158,     0,     0,     0,     0,     0,
       0,     0,    25,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    26,    27,     0,     0,     0,     0,
      28,    29,    30,    31,    32,    33,     0,    34,    35,     0,
       0,    74,   158,     0,     0,     0,     0,    26,    27,   159,
       0,     0,     0,    28,    29,    30,    31,    32,    33,     0,
      34,    35,     0,   160,   285,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,     0,   174,
     175,   176,   177,   178,   179,   180,   181,   159,     0,   158,
       0,     0,     0,     0,   182,   183,     0,     0,     0,     0,
       0,   160,   386,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,     0,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,   158,     0,
       0,     0,   182,   183,   159,     0,     0,     0,     0,     0,
     388,     0,     0,     0,     0,     0,     0,     0,   160,     0,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,     0,   174,   175,   176,   177,   178,   179,
     180,   181,   158,   159,     0,     0,     0,     0,     0,   182,
     183,     0,     0,     0,     0,     0,   259,   160,     0,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,     0,   174,   175,   176,   177,   178,   179,   180,
     181,   158,     0,     0,     0,     0,     0,   159,   182,   183,
       0,     0,     0,     0,     0,   334,     0,     0,     0,     0,
       0,   160,     0,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,     0,   174,   175,   176,
     177,   178,   179,   180,   181,   158,   159,     0,     0,     0,
       0,     0,   182,   183,     0,     0,     0,     0,     0,   342,
     160,     0,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,     0,   174,   175,   176,   177,
     178,   179,   180,   181,   158,     0,     0,     0,     0,     0,
     159,   182,   183,     0,     0,     0,     0,     0,   349,     0,
       0,     0,     0,     0,   160,     0,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,     0,
     174,   175,   176,   177,   178,   179,   180,   181,   158,   159,
       0,     0,     0,     0,     0,   182,   183,     0,     0,     0,
       0,     0,   444,   160,     0,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,     0,   174,
     175,   176,   177,   178,   179,   180,   181,   158,     0,     0,
       0,     0,     0,   159,   182,   183,     0,     0,     0,     0,
       0,   446,     0,     0,     0,     0,     0,   160,     0,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,     0,   174,   175,   176,   177,   178,   179,   180,
     181,   158,   159,     0,     0,     0,     0,     0,   182,   183,
       0,     0,     0,     0,     0,   494,   160,     0,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,     0,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,   159,   182,   183,     0,
       0,     0,     0,     0,   495,     0,     0,     0,     0,     0,
     160,     0,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,     0,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,   214,
       2,   182,   183,     0,     0,     0,     0,   215,   506,   104,
     105,   216,   217,   218,   219,   220,   221,     0,   222,   223,
       0,     0,     0,   224,   225,   226,   227,   228,   229,   230,
     231,     0,   122,   123,   232,   125,   126,   127,   128,   129,
     130,   131,     0,   233,   133,   134,     0,   234,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   366,     2,     0,     0,     0,     0,     0,   136,   215,
       0,   104,   105,   216,   217,   218,   219,   220,   221,     0,
     222,   223,   235,   236,     0,   224,   225,   226,   227,   228,
     229,   230,   231,     0,   122,   123,   232,   125,   126,   127,
     128,   129,   130,   131,     0,   233,   133,   134,     0,   234,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   158,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   328,     0,     0,     0,     0,     0,     0,
     136,   215,     0,   104,   105,   216,   217,   218,   219,   220,
     221,     0,   222,   223,   235,   236,     0,   224,   225,     0,
       0,     0,   229,   230,     0,     0,   122,   123,   159,   125,
     126,   127,   128,   129,   130,   131,   158,   233,   329,   330,
       0,   234,   160,     0,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,     0,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   136,   182,   183,   158,     0,   184,     0,     0,
       0,   159,     0,     0,     0,     0,   235,   236,     0,     0,
       0,     0,     0,     0,     0,   160,     0,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
       0,   174,   175,   176,   177,   178,   179,   180,   181,   158,
     159,     0,     0,     0,     0,     0,   182,   183,     0,     0,
     238,     0,     0,     0,   160,     0,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,     0,
     174,   175,   176,   177,   178,   179,   180,   181,   158,     0,
       0,     0,     0,     0,   159,   182,   183,     0,     0,   248,
       0,     0,     0,     0,     0,     0,     0,     0,   160,     0,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,     0,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,     0,   182,
     183,     0,     0,   440,     0,     0,     0,   160,     0,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   441,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,     0,     0,   442,     0,     0,   182,   183,
       0,     0,   184,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   198,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   199,   200,     0,
       0,     0,   201,   202,   203
};

static const yytype_int16 yycheck[] =
{
       0,   151,   337,     0,    37,    11,    12,   334,    11,    34,
       5,     3,    11,     3,    53,     3,     3,    53,    53,     3,
       3,    64,   349,    29,    30,     3,    53,     3,    50,    55,
      50,    13,    14,     0,    13,    14,    55,    37,   454,    39,
       7,    74,   458,    53,   185,    71,    68,    14,    68,     4,
     157,    68,    71,    68,    68,   100,    32,    33,   208,    26,
      27,    28,   203,   102,    31,    32,    33,    34,    35,   105,
      95,   106,    39,   434,    29,    30,    31,   104,    68,    34,
       8,    68,    82,    38,     8,    82,   103,   503,   103,   103,
     451,     3,   102,    21,    68,   103,    25,    26,    65,   240,
     241,   242,    86,    55,    88,    14,   112,   113,   100,   112,
      77,   106,   100,   112,   100,    98,    68,   444,    85,   103,
      98,     0,   104,   450,   459,   104,   100,    55,    95,     3,
      55,   100,    87,    86,    87,    88,    89,    90,   100,   139,
     247,   108,    97,    68,    50,    98,    99,   114,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,   100,    83,
      84,    85,    86,    87,    88,    89,    90,   494,    98,    99,
     137,   138,   102,    50,    98,    99,    53,    50,     8,   506,
     507,     7,   102,     9,    10,   102,    12,    98,    99,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   356,   183,   260,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   339,    98,    99,   202,   203,   270,    98,    99,
     100,   100,   100,    68,   385,   245,   279,    68,    78,    79,
      80,    81,   285,    83,    84,    85,    86,    87,    88,    89,
      90,    68,     8,    68,    68,    68,   373,    68,    98,    99,
      68,   277,    68,   240,   241,   242,    88,    89,    90,   279,
      68,   422,    68,   433,   391,   285,    98,    99,   285,   256,
       3,     3,     4,   260,   261,   262,   263,   264,   265,     3,
      68,    68,   269,   336,    16,   272,   273,    55,   449,   100,
     277,   278,     3,   100,    53,   282,   100,    29,    30,    31,
      32,    33,    34,    35,    98,   432,    38,    73,    74,    75,
      76,    77,    78,    79,    80,    81,   105,    83,    84,    85,
      86,    87,    88,    89,    90,   100,    68,   100,     3,   456,
     100,   100,    98,    99,   105,   462,   100,     9,   465,    53,
     105,   468,    83,    84,    85,    86,    87,    88,    89,    90,
     100,   105,   479,   105,   341,    98,   105,    98,    99,   105,
     100,   105,   489,   350,   100,     3,   105,    99,   100,   356,
       3,   103,     3,   100,   105,    34,     6,   504,    98,   105,
       3,     4,     3,   105,   105,   102,   105,   103,    71,   376,
      98,    26,   105,    16,    68,   105,   105,   384,   385,    22,
     105,   454,   277,   499,   375,   458,    29,    30,    31,    32,
      33,    34,    35,   274,   436,    38,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    51,    86,
      53,   455,   270,    -1,   201,   422,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,
     503,    -1,    -1,    -1,    93,    94,   443,    -1,   445,    98,
      99,   100,   449,    86,    87,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    -1,    99,   100,    -1,    -1,
     103,    -1,   469,   106,   471,    -1,    -1,    -1,    -1,    -1,
     510,    -1,    -1,   510,    -1,    -1,    -1,   517,   518,   519,
     517,     0,    -1,    -1,     3,     4,     5,    -1,    -1,   496,
      -1,    -1,    11,    -1,    -1,    -1,    15,    16,    17,    18,
      19,    20,    -1,    22,    23,    24,    -1,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    -1,     0,    38,
      -1,     3,     4,     5,    -1,    -1,    -1,    -1,    47,    11,
      -1,    -1,    51,    15,    16,    17,    18,    19,    20,    -1,
      22,    23,    24,    -1,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    16,    86,    87,    51,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    29,
      99,   100,    -1,   102,   103,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,     3,     4,    -1,    -1,
      92,    93,    94,    95,    96,    97,    -1,    99,   100,    16,
     102,   103,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    30,    31,    32,    33,    34,    35,     3,
       4,    38,    -1,    -1,    -1,    -1,    -1,    97,    -1,    -1,
      -1,    -1,    16,    -1,    51,    -1,    53,    -1,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    86,
      87,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    -1,    99,   100,    -1,    -1,   103,    -1,    -1,   106,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    -1,    99,   100,     8,    -1,   103,
      -1,   105,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,
      11,    -1,    13,    14,    15,    16,    17,    18,    19,    20,
      -1,    22,    23,    24,    -1,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    47,    48,    49,    -1,
      51,    -1,    53,    -1,    -1,    -1,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    -1,    83,    84,    85,    86,    87,    88,    89,    90,
      -1,    82,    -1,    -1,    -1,    86,    87,    98,    99,    -1,
      -1,    92,    93,    94,    95,    96,    97,    -1,    99,   100,
       8,   102,   103,   104,     3,     4,     5,    -1,    -1,    -1,
      -1,    -1,    11,    -1,    13,    14,    15,    16,    17,    18,
      19,    20,    -1,    22,    23,    24,    -1,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,    47,    48,
      49,    -1,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    -1,    83,    84,    85,    86,    87,
      88,    89,    90,    82,    -1,    -1,    -1,    86,    87,    -1,
      98,    99,    -1,    92,    93,    94,    95,    96,    97,    -1,
      99,   100,     8,   102,   103,   104,     3,     4,     5,    -1,
      -1,    -1,    -1,    -1,    11,    -1,    13,    14,    15,    16,
      17,    18,    19,    20,    -1,    22,    23,    24,    -1,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      47,    48,    49,    -1,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    -1,    83,    84,    85,
      86,    87,    88,    89,    90,    82,    -1,    -1,    -1,    86,
      87,    -1,    98,    99,    -1,    92,    93,    94,    95,    96,
      97,    -1,    99,   100,     8,   102,   103,   104,     3,     4,
       5,    -1,    -1,    -1,    -1,    -1,    11,    -1,    13,    14,
      15,    16,    17,    18,    19,    20,    -1,    22,    23,    24,
      -1,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    53,
      -1,    -1,    47,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    -1,    83,
      84,    85,    86,    87,    88,    89,    90,    -1,    -1,    -1,
      -1,    86,    87,    -1,    98,    99,    -1,    92,    93,    94,
      95,    96,    97,    -1,    99,   100,     8,   102,   103,   104,
       3,     4,     5,    -1,    -1,    -1,    -1,    -1,    11,    -1,
      13,    14,    15,    16,    17,    18,    19,    20,    -1,    22,
      23,    24,    -1,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    53,    -1,    -1,    47,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      -1,    83,    84,    85,    86,    87,    88,    89,    90,    -1,
      -1,    -1,    -1,    86,    87,    -1,    98,    99,    -1,    92,
      93,    94,    95,    96,    97,    -1,    99,   100,     8,   102,
     103,   104,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,
      11,    -1,    13,    14,    15,    16,    17,    18,    19,    20,
      -1,    22,    23,    24,    -1,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    53,    -1,    -1,    47,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    -1,    83,    84,    85,    86,    87,    88,    89,
      90,    -1,    -1,    -1,    -1,    86,    87,    -1,    98,    99,
      -1,    92,    93,    94,    95,    96,    97,    -1,    99,   100,
      -1,   102,   103,   104,     3,     4,     5,    -1,    -1,    -1,
      -1,    -1,    11,    -1,    13,    14,    15,    16,    17,    18,
      19,    20,    -1,    22,    23,    24,    -1,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    -1,    -1,    38,
      -1,     3,     4,     5,    -1,    -1,    -1,    -1,    47,    11,
      -1,    -1,    51,    15,    16,    17,    18,    19,    20,    -1,
      22,    23,    24,    -1,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    86,    87,    51,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    -1,
      99,   100,    -1,   102,   103,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    -1,    99,   100,    -1,
     102,   103,   104,     3,     4,     5,    -1,    -1,    -1,    -1,
      -1,    11,    -1,    -1,    -1,    15,    16,    17,    18,    19,
      20,    -1,    22,    23,    24,    -1,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    -1,    38,    -1,
       3,     4,     5,    -1,    -1,    -1,    -1,    47,    11,    -1,
      -1,    51,    15,    16,    17,    18,    19,    20,    -1,    22,
      23,    24,    -1,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    86,    87,    51,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    -1,    99,
     100,    -1,   102,   103,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    -1,    99,   100,    -1,   102,
     103,   104,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,
      11,    -1,    -1,    -1,    15,    16,    17,    18,    19,    20,
      -1,    22,    23,    24,    -1,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    -1,    -1,    38,    -1,     3,
       4,     5,    -1,    -1,    -1,    -1,    47,    11,    -1,    -1,
      51,    15,    16,    17,    18,    19,    20,    -1,    22,    23,
      24,    -1,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    86,    87,    51,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    -1,    99,   100,
      -1,   102,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    -1,    99,   100,     8,   102,   103,
       3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    18,    19,    20,    -1,    22,
      -1,    -1,    -1,     3,     4,    -1,    29,    30,    31,    32,
      33,    34,    35,    -1,    -1,    38,    16,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    51,    29,
      30,    31,    32,    33,    34,    35,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    80,
      81,    51,    83,    84,    85,    86,    87,    88,    89,    90,
      -1,    -1,    -1,    86,    87,    -1,    -1,    98,    99,    92,
      93,    94,    95,    96,    97,    -1,    99,   100,    -1,   102,
     103,    -1,    -1,    -1,    -1,    -1,    86,    87,     3,     4,
      -1,    -1,    92,    93,    94,    95,    96,    97,    -1,    99,
     100,    16,   102,   103,    -1,    -1,    -1,    22,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,    34,
      35,    -1,    -1,    38,    -1,    -1,     3,     4,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    16,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    30,    31,    32,    33,    34,    35,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    87,    -1,    51,    -1,    53,    92,    93,    94,
      95,    96,    97,    -1,    99,   100,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,     3,     4,    -1,    -1,    92,    93,    94,    95,    96,
      97,    -1,    99,   100,    16,    -1,   103,    -1,    -1,    -1,
      22,    -1,    -1,    -1,     3,     4,    -1,    29,    30,    31,
      32,    33,    34,    35,    -1,    -1,    38,    16,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    51,
      29,    30,    31,    32,    33,    34,    35,    -1,    -1,    38,
      -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    -1,    99,   100,    -1,
      -1,   103,     8,    -1,    -1,    -1,    -1,    86,    87,    53,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    -1,
      99,   100,    -1,    67,   103,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    -1,    83,
      84,    85,    86,    87,    88,    89,    90,    53,    -1,     8,
      -1,    -1,    -1,    -1,    98,    99,    -1,    -1,    -1,    -1,
      -1,    67,   106,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    -1,    83,    84,    85,
      86,    87,    88,    89,    90,    -1,    -1,    -1,     8,    -1,
      -1,    -1,    98,    99,    53,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    -1,    83,    84,    85,    86,    87,    88,
      89,    90,     8,    53,    -1,    -1,    -1,    -1,    -1,    98,
      99,    -1,    -1,    -1,    -1,    -1,   105,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    -1,    83,    84,    85,    86,    87,    88,    89,
      90,     8,    -1,    -1,    -1,    -1,    -1,    53,    98,    99,
      -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    -1,    83,    84,    85,
      86,    87,    88,    89,    90,     8,    53,    -1,    -1,    -1,
      -1,    -1,    98,    99,    -1,    -1,    -1,    -1,    -1,   105,
      67,    -1,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    -1,    83,    84,    85,    86,
      87,    88,    89,    90,     8,    -1,    -1,    -1,    -1,    -1,
      53,    98,    99,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    -1,
      83,    84,    85,    86,    87,    88,    89,    90,     8,    53,
      -1,    -1,    -1,    -1,    -1,    98,    99,    -1,    -1,    -1,
      -1,    -1,   105,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    -1,    83,
      84,    85,    86,    87,    88,    89,    90,     8,    -1,    -1,
      -1,    -1,    -1,    53,    98,    99,    -1,    -1,    -1,    -1,
      -1,   105,    -1,    -1,    -1,    -1,    -1,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    -1,    83,    84,    85,    86,    87,    88,    89,
      90,     8,    53,    -1,    -1,    -1,    -1,    -1,    98,    99,
      -1,    -1,    -1,    -1,    -1,   105,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    -1,    83,    84,    85,    86,    87,    88,    89,    90,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    98,    99,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    -1,    83,    84,    85,    86,
      87,    88,    89,    90,    -1,    -1,    -1,    -1,    -1,     3,
       4,    98,    99,    -1,    -1,    -1,    -1,    11,   105,    13,
      14,    15,    16,    17,    18,    19,    20,    -1,    22,    23,
      -1,    -1,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    -1,    47,    48,    49,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,    -1,    -1,    -1,    -1,    82,    11,
      -1,    13,    14,    15,    16,    17,    18,    19,    20,    -1,
      22,    23,    96,    97,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    47,    48,    49,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,    -1,    -1,    -1,    -1,    -1,    -1,
      82,    11,    -1,    13,    14,    15,    16,    17,    18,    19,
      20,    -1,    22,    23,    96,    97,    -1,    27,    28,    -1,
      -1,    -1,    32,    33,    -1,    -1,    36,    37,    53,    39,
      40,    41,    42,    43,    44,    45,     8,    47,    48,    49,
      -1,    51,    67,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    -1,    83,    84,
      85,    86,    87,    88,    89,    90,    -1,    -1,    -1,    -1,
      -1,    -1,    82,    98,    99,     8,    -1,   102,    -1,    -1,
      -1,    53,    -1,    -1,    -1,    -1,    96,    97,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      -1,    83,    84,    85,    86,    87,    88,    89,    90,     8,
      53,    -1,    -1,    -1,    -1,    -1,    98,    99,    -1,    -1,
     102,    -1,    -1,    -1,    67,    -1,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    -1,
      83,    84,    85,    86,    87,    88,    89,    90,     8,    -1,
      -1,    -1,    -1,    -1,    53,    98,    99,    -1,    -1,   102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    -1,    83,    84,    85,    86,    87,    88,
      89,    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      99,    -1,    -1,   102,    -1,    -1,    -1,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,     8,    83,    84,    85,    86,    87,    88,    89,
      90,    -1,    -1,    -1,    -1,    21,    -1,    -1,    98,    99,
      -1,    -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    -1,
      -1,    -1,    98,    99,   100
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,    11,    15,    16,    17,    18,    19,
      20,    22,    23,    24,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    38,    47,    51,    86,    87,    92,    93,
      94,    95,    96,    97,    99,   100,   102,   103,   108,   109,
     110,   111,   113,   114,   115,   116,   118,   120,   121,   124,
     127,   128,   130,   134,   135,   136,   141,   145,   147,   148,
     149,   152,   153,   159,    68,   100,     3,   117,   117,     3,
     100,     3,    16,   102,   103,   147,     3,   100,   136,   149,
     152,   149,   103,   146,   147,   100,    14,   147,   147,   147,
     149,   149,   147,   147,   147,    53,   106,   147,   151,     3,
     105,   147,     3,    11,    13,    14,    15,    16,    17,    18,
      19,    20,    22,    23,    27,    28,    29,    30,    31,    32,
      33,    34,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    47,    48,    49,    51,    82,    96,    97,   109,
     141,   147,   156,   157,   158,     0,   110,   147,     3,   125,
     126,   100,     7,     9,    10,    12,   100,    50,     8,    53,
      67,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    83,    84,    85,    86,    87,    88,
      89,    90,    98,    99,   102,   100,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    82,    93,
      94,    98,    99,   100,   113,   147,   102,   102,   100,     3,
      98,   137,   143,     3,     3,    11,    15,    16,    17,    18,
      19,    20,    22,    23,    27,    28,    29,    30,    31,    32,
      33,    34,    38,    47,    51,    96,    97,   141,   102,   147,
     100,   100,   100,   102,   104,   109,    25,    26,   102,   147,
       3,    86,    88,   112,   153,   151,    53,   106,    53,   105,
      68,    68,    68,    68,    68,    68,     3,     3,   104,    68,
      53,   104,    68,    55,    53,   102,   137,   100,   100,   103,
     110,   119,   100,   150,   151,   103,   146,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,     3,   147,   150,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,     3,    48,
      49,   157,   147,   150,   105,   137,    55,    68,    98,   105,
      53,    68,   105,   150,   150,   150,   104,   100,   146,   105,
      53,    53,   106,   147,     3,   144,   100,   147,   147,   147,
     147,   147,   147,   100,   100,   147,     3,   154,   155,   158,
     147,   147,   126,   105,   102,   111,   131,   147,   149,   147,
     104,     9,   147,   105,    68,   100,   106,   105,   106,   105,
     119,   105,    29,    30,    31,    34,    38,    87,    97,   141,
     142,    16,    29,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    97,   138,   140,    98,    68,   146,     3,
      98,   147,   100,   105,   105,   105,     3,   119,    53,   105,
     150,     3,   105,   100,    68,   146,     3,   125,   132,   147,
     102,     8,    21,   129,   105,   100,   105,   147,   150,   100,
       6,    68,   146,    34,    55,    71,   139,   140,    55,    68,
      98,   150,   105,     3,   105,   105,   146,   137,   139,   129,
     102,   102,   147,   119,   147,   103,   105,   150,   119,   139,
     142,   138,   146,   142,   140,    98,   105,   146,   146,   105,
     146,   147,   133,   147,   105,   105,    13,    14,   104,   122,
     123,   105,   146,    55,    26,   146,   105,   105,   119,   147,
      68,   104,   123,   142,   146,   119,   119,    68,   109,   109
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   107,   108,   108,   108,   108,   109,   109,   110,   110,
     110,   111,   111,   111,   112,   112,   112,   112,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   114,   114,   115,   116,   116,   116,   116,
     117,   117,   118,   118,   119,   119,   120,   121,   121,   122,
     122,   123,   123,   123,   123,   124,   124,   124,   125,   125,
     126,   126,   127,   128,   128,   129,   129,   130,   130,   130,
     131,   131,   131,   132,   132,   133,   133,   134,   135,   136,
     136,   136,   136,   137,   137,   137,   137,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     139,   140,   140,   141,   142,   142,   142,   142,   142,   142,
     142,   142,   143,   143,   143,   143,   143,   143,   143,   143,
     144,   144,   145,   145,   145,   146,   146,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   148,   148,   148,   148,   148,   149,   149,
     149,   149,   149,   149,   150,   150,   151,   151,   151,   151,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   153,
     154,   155,   155,   155,   156,   156,   156,   156,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   159,
     159,   159
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     2,     1,     1,     2,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       1,     3,     3,     3,     2,     3,     3,     1,     1,     1,
       3,     1,     3,     7,     5,     2,     1,     1,     1,     1,
       0,     1,     0,     2,     1,     2,     5,     7,     8,     1,
       2,     4,     3,     2,     3,     7,     4,     9,     1,     3,
       1,     3,     3,     5,     7,     1,     1,     9,     9,     8,
       1,     2,     3,     0,     1,     0,     1,     6,     7,     5,
       7,     6,     8,     0,     1,     3,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     3,     3,     5,     3,     5,     5,     7,
       1,     3,     1,     5,     2,     3,     2,     1,     1,     1,
       3,     4,     3,     2,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     2,     2,     4,     2,     5,     5,     7,     5,
       4,     5,     1,     6,     7,     4,     6,     4,     1,     1,
       1,     4,     3,     3,     0,     1,     1,     3,     3,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       5,     1,     1,     1,     0,     1,     3,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     6,     5,     3,
       4,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, pstate, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, pstate); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct jsi_Pstate *pstate)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (pstate);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct jsi_Pstate *pstate)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, pstate);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, struct jsi_Pstate *pstate)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), pstate);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, pstate); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, struct jsi_Pstate *pstate)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (pstate);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_FNUMBER: /* FNUMBER  */
#line 33 "src/parser.y"
            { Jsi_Free(((*yyvaluep).num)); }
#line 2201 "src/parser.c"
        break;

    case YYSYMBOL_statements: /* statements  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2207 "src/parser.c"
        break;

    case YYSYMBOL_statement: /* statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2213 "src/parser.c"
        break;

    case YYSYMBOL_objexport: /* objexport  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2219 "src/parser.c"
        break;

    case YYSYMBOL_commonstatement: /* commonstatement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2225 "src/parser.c"
        break;

    case YYSYMBOL_func_statement: /* func_statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2231 "src/parser.c"
        break;

    case YYSYMBOL_iterstatement: /* iterstatement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2237 "src/parser.c"
        break;

    case YYSYMBOL_statement_or_empty: /* statement_or_empty  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2243 "src/parser.c"
        break;

    case YYSYMBOL_with_statement: /* with_statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2249 "src/parser.c"
        break;

    case YYSYMBOL_switch_statement: /* switch_statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2255 "src/parser.c"
        break;

    case YYSYMBOL_cases: /* cases  */
#line 37 "src/parser.y"
            { caselist_free(((*yyvaluep).caselist));}
#line 2261 "src/parser.c"
        break;

    case YYSYMBOL_case: /* case  */
#line 36 "src/parser.y"
            { Jsi_Free(((*yyvaluep).caseitem)); }
#line 2267 "src/parser.c"
        break;

    case YYSYMBOL_try_statement: /* try_statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2273 "src/parser.c"
        break;

    case YYSYMBOL_vardecs: /* vardecs  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2279 "src/parser.c"
        break;

    case YYSYMBOL_vardec: /* vardec  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2285 "src/parser.c"
        break;

    case YYSYMBOL_delete_statement: /* delete_statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2291 "src/parser.c"
        break;

    case YYSYMBOL_if_statement: /* if_statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2297 "src/parser.c"
        break;

    case YYSYMBOL_for_statement: /* for_statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2303 "src/parser.c"
        break;

    case YYSYMBOL_for_init: /* for_init  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2309 "src/parser.c"
        break;

    case YYSYMBOL_for_cond: /* for_cond  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2315 "src/parser.c"
        break;

    case YYSYMBOL_expr_opt: /* expr_opt  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2321 "src/parser.c"
        break;

    case YYSYMBOL_while_statement: /* while_statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2327 "src/parser.c"
        break;

    case YYSYMBOL_do_statement: /* do_statement  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2333 "src/parser.c"
        break;

    case YYSYMBOL_func_expr: /* func_expr  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2339 "src/parser.c"
        break;

    case YYSYMBOL_args_opt: /* args_opt  */
#line 34 "src/parser.y"
            { jsi_ScopeStrsFree(pstate->interp, ((*yyvaluep).scopes)); }
#line 2345 "src/parser.c"
        break;

    case YYSYMBOL_argdefault: /* argdefault  */
#line 32 "src/parser.y"
            { Jsi_ValueFree(pstate->interp, ((*yyvaluep).value)); }
#line 2351 "src/parser.c"
        break;

    case YYSYMBOL_args: /* args  */
#line 34 "src/parser.y"
            { jsi_ScopeStrsFree(pstate->interp, ((*yyvaluep).scopes)); }
#line 2357 "src/parser.c"
        break;

    case YYSYMBOL_argsa: /* argsa  */
#line 34 "src/parser.y"
            { jsi_ScopeStrsFree(pstate->interp, ((*yyvaluep).scopes)); }
#line 2363 "src/parser.c"
        break;

    case YYSYMBOL_arrowargs: /* arrowargs  */
#line 34 "src/parser.y"
            { jsi_ScopeStrsFree(pstate->interp, ((*yyvaluep).scopes)); }
#line 2369 "src/parser.c"
        break;

    case YYSYMBOL_func_statement_block: /* func_statement_block  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2375 "src/parser.c"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2381 "src/parser.c"
        break;

    case YYSYMBOL_fcall_exprs: /* fcall_exprs  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2387 "src/parser.c"
        break;

    case YYSYMBOL_lvalue: /* lvalue  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2393 "src/parser.c"
        break;

    case YYSYMBOL_exprlist_opt: /* exprlist_opt  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2399 "src/parser.c"
        break;

    case YYSYMBOL_exprlist: /* exprlist  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2405 "src/parser.c"
        break;

    case YYSYMBOL_value: /* value  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2411 "src/parser.c"
        break;

    case YYSYMBOL_object: /* object  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2417 "src/parser.c"
        break;

    case YYSYMBOL_itemfunc: /* itemfunc  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2423 "src/parser.c"
        break;

    case YYSYMBOL_itemident: /* itemident  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2429 "src/parser.c"
        break;

    case YYSYMBOL_items: /* items  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2435 "src/parser.c"
        break;

    case YYSYMBOL_itemres: /* itemres  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2441 "src/parser.c"
        break;

    case YYSYMBOL_item: /* item  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2447 "src/parser.c"
        break;

    case YYSYMBOL_array: /* array  */
#line 35 "src/parser.y"
            { jsi_FreeOpcodes(((*yyvaluep).opcodes)); }
#line 2453 "src/parser.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (struct jsi_Pstate *pstate)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc, pstate);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* file: %empty  */
#line 128 "src/parser.y"
        { pstate->opcodes = code_nop(); }
#line 2759 "src/parser.c"
    break;

  case 3: /* file: statements  */
#line 129 "src/parser.y"
                 {
        pstate->opcodes = (yyvsp[0].opcodes);
    }
#line 2767 "src/parser.c"
    break;

  case 4: /* file: statements expr  */
#line 132 "src/parser.y"
                      {
        pstate->opcodes = codes_join3((yyvsp[-1].opcodes), (yyvsp[0].opcodes), code_ret(pstate, &(yylsp[-1]), 1));
    }
#line 2775 "src/parser.c"
    break;

  case 5: /* file: expr  */
#line 135 "src/parser.y"
           {    /* for json */
        pstate->opcodes = codes_join((yyvsp[0].opcodes), code_ret(pstate, &(yylsp[0]), 1));
    }
#line 2783 "src/parser.c"
    break;

  case 6: /* statements: statement  */
#line 140 "src/parser.y"
                            { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2789 "src/parser.c"
    break;

  case 7: /* statements: statements statement  */
#line 141 "src/parser.y"
                            { (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), (yyvsp[0].opcodes)); }
#line 2795 "src/parser.c"
    break;

  case 8: /* statement: iterstatement  */
#line 146 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2801 "src/parser.c"
    break;

  case 9: /* statement: commonstatement  */
#line 147 "src/parser.y"
                         { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2807 "src/parser.c"
    break;

  case 10: /* statement: IDENTIFIER ':' commonstatement  */
#line 148 "src/parser.y"
                                     { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2813 "src/parser.c"
    break;

  case 11: /* localvar: LOCAL  */
#line 152 "src/parser.y"
          { (yyval.inum) = LOCAL; }
#line 2819 "src/parser.c"
    break;

  case 12: /* localvar: LOCALLET  */
#line 153 "src/parser.y"
               { (yyval.inum) = LOCALLET;  code_es6(pstate, "let");}
#line 2825 "src/parser.c"
    break;

  case 13: /* localvar: LOCALCONST  */
#line 154 "src/parser.y"
                 { (yyval.inum) = LOCALCONST; code_es6(pstate, "const"); }
#line 2831 "src/parser.c"
    break;

  case 14: /* objexport: object  */
#line 158 "src/parser.y"
           { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2837 "src/parser.c"
    break;

  case 15: /* objexport: IDENTIFIER  */
#line 159 "src/parser.y"
                 {
        Jsi_OpCodes *lval = code_push_index(pstate, &(yylsp[0]), (yyvsp[0].sstr), 0); 
        (yyval.opcodes) = lval;
        lval->lvalue_flag = 1; 
        lval->lvalue_name = (yyvsp[0].sstr); 
    }
#line 2848 "src/parser.c"
    break;

  case 16: /* objexport: '*'  */
#line 165 "src/parser.y"
          {
        (yyval.opcodes) = code_push_null();
    }
#line 2856 "src/parser.c"
    break;

  case 17: /* objexport: '+'  */
#line 168 "src/parser.y"
          {
        (yyval.opcodes) = code_push_undef();
    }
#line 2864 "src/parser.c"
    break;

  case 18: /* commonstatement: expr ';'  */
#line 174 "src/parser.y"
             { (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_pop(1)); }
#line 2870 "src/parser.c"
    break;

  case 19: /* commonstatement: if_statement  */
#line 175 "src/parser.y"
                    { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2876 "src/parser.c"
    break;

  case 20: /* commonstatement: delete_statement  */
#line 176 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2882 "src/parser.c"
    break;

  case 21: /* commonstatement: BREAK identifier_opt ';'  */
#line 177 "src/parser.y"
                                    { (yyval.opcodes) = code_reserved(pstate, &(yylsp[-1]), RES_BREAK, (yyvsp[-1].sstr)); }
#line 2888 "src/parser.c"
    break;

  case 22: /* commonstatement: CONTINUE identifier_opt ';'  */
#line 178 "src/parser.y"
                                    { (yyval.opcodes) = code_reserved(pstate, &(yylsp[-1]), RES_CONTINUE, (yyvsp[-1].sstr)); }
#line 2894 "src/parser.c"
    break;

  case 23: /* commonstatement: RETURN expr ';'  */
#line 179 "src/parser.y"
                        { (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_ret(pstate, &(yylsp[-1]), 1)); }
#line 2900 "src/parser.c"
    break;

  case 24: /* commonstatement: RETURN ';'  */
#line 180 "src/parser.y"
                        { (yyval.opcodes) = code_ret(pstate, &(yylsp[-1]), 0); }
#line 2906 "src/parser.c"
    break;

  case 25: /* commonstatement: localvar vardecs ';'  */
#line 181 "src/parser.y"
                           {
        jsi_mark_local((yyvsp[-1].opcodes), (yyvsp[-2].inum));
        (yyval.opcodes) = (yyvsp[-1].opcodes);
    }
#line 2915 "src/parser.c"
    break;

  case 26: /* commonstatement: THROW expr ';'  */
#line 185 "src/parser.y"
                        { (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_throw(pstate, &(yylsp[-1]))); }
#line 2921 "src/parser.c"
    break;

  case 27: /* commonstatement: try_statement  */
#line 186 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2927 "src/parser.c"
    break;

  case 28: /* commonstatement: with_statement  */
#line 187 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2933 "src/parser.c"
    break;

  case 29: /* commonstatement: ';'  */
#line 188 "src/parser.y"
                            { (yyval.opcodes) = code_nop(); }
#line 2939 "src/parser.c"
    break;

  case 30: /* commonstatement: '{' statements '}'  */
#line 189 "src/parser.y"
                            { (yyval.opcodes) = (yyvsp[-1].opcodes); }
#line 2945 "src/parser.c"
    break;

  case 31: /* commonstatement: func_statement  */
#line 190 "src/parser.y"
                            { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 2951 "src/parser.c"
    break;

  case 32: /* commonstatement: EXPORT DEFAULT objexport  */
#line 191 "src/parser.y"
                               { (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_export(pstate, &(yylsp[0]), 1)); }
#line 2957 "src/parser.c"
    break;

  case 33: /* func_statement: func_prefix '(' args_opt ')' ':' rettype func_statement_block  */
#line 195 "src/parser.y"
                                                                  {
        (yyvsp[-4].scopes)->retType = (yyvsp[-1].inum);
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &(yylsp[-6]), (yyvsp[-6].sstr), 0),
          code_push_func(pstate, &(yylsp[-4]), jsi_FuncMake(pstate, (yyvsp[-4].scopes), (yyvsp[0].opcodes), &(yylsp[-6]), (yyvsp[-6].sstr), 0)),
          code_assign(pstate, &(yylsp[-6]), 1), code_pop(1));
        if (pstate->eval_flag) ret = codes_join(code_local(pstate, &(yylsp[-6]), (yyvsp[-6].sstr)), ret);
        jsi_PstatePop(pstate);
        (yyval.opcodes) = ret;
    }
#line 2971 "src/parser.c"
    break;

  case 34: /* func_statement: func_prefix '(' args_opt ')' func_statement_block  */
#line 204 "src/parser.y"
                                                        {
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &(yylsp[-4]), (yyvsp[-4].sstr), 0),
          code_push_func(pstate, &(yylsp[-2]), jsi_FuncMake(pstate, (yyvsp[-2].scopes), (yyvsp[0].opcodes), &(yylsp[-4]), (yyvsp[-4].sstr), 0)),
          code_assign(pstate, &(yylsp[-4]), 1), code_pop(1));
        if (pstate->eval_flag) ret = codes_join(code_local(pstate, &(yylsp[-4]), (yyvsp[-4].sstr)), ret);
        jsi_PstatePop(pstate);
        (yyval.opcodes) = ret;
    }
#line 2984 "src/parser.c"
    break;

  case 35: /* func_prefix: FUNC IDENTIFIER  */
#line 215 "src/parser.y"
                                  {
        if (!pstate->eval_flag) {
            jsi_PstateAddVar(pstate, &(yylsp[0]), (yyvsp[0].sstr));
        }
        (yyval.sstr) = (yyvsp[0].sstr);
    }
#line 2995 "src/parser.c"
    break;

  case 36: /* iterstatement: for_statement  */
#line 224 "src/parser.y"
                    { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3001 "src/parser.c"
    break;

  case 37: /* iterstatement: while_statement  */
#line 225 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3007 "src/parser.c"
    break;

  case 38: /* iterstatement: do_statement  */
#line 226 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3013 "src/parser.c"
    break;

  case 39: /* iterstatement: switch_statement  */
#line 227 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3019 "src/parser.c"
    break;

  case 40: /* identifier_opt: %empty  */
#line 230 "src/parser.y"
                { (yyval.sstr) = NULL; }
#line 3025 "src/parser.c"
    break;

  case 41: /* identifier_opt: IDENTIFIER  */
#line 231 "src/parser.y"
                 { (yyval.sstr) = (yyvsp[0].sstr); }
#line 3031 "src/parser.c"
    break;

  case 42: /* label_opt: %empty  */
#line 234 "src/parser.y"
           { (yyval.sstr) = NULL; }
#line 3037 "src/parser.c"
    break;

  case 43: /* label_opt: IDENTIFIER ':'  */
#line 235 "src/parser.y"
                     {
        (yyval.sstr) = (yyvsp[-1].sstr);
    }
#line 3045 "src/parser.c"
    break;

  case 44: /* statement_or_empty: statement  */
#line 241 "src/parser.y"
                { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3051 "src/parser.c"
    break;

  case 45: /* statement_or_empty: '{' '}'  */
#line 242 "src/parser.y"
                { (yyval.opcodes) = code_nop(); }
#line 3057 "src/parser.c"
    break;

  case 46: /* with_statement: WITH '(' expr ')' statement_or_empty  */
#line 246 "src/parser.y"
                                         { 
        (yyval.opcodes) = codes_join4((yyvsp[-2].opcodes), code_with(pstate, &(yylsp[-2]), ((yyvsp[0].opcodes))->code_len + 1), (yyvsp[0].opcodes), code_ewith(pstate, &(yylsp[0])));
    }
#line 3065 "src/parser.c"
    break;

  case 47: /* switch_statement: label_opt SWITCH '(' expr ')' '{' '}'  */
#line 252 "src/parser.y"
                                          { (yyval.opcodes) = codes_join((yyvsp[-3].opcodes), code_pop(1)); }
#line 3071 "src/parser.c"
    break;

  case 48: /* switch_statement: label_opt SWITCH '(' expr ')' '{' cases '}'  */
#line 253 "src/parser.y"
                                                    {
        jsi_CaseList *cl = (yyvsp[-1].caselist);
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
                    yyerror(&(yylsp[0]), pstate, "switch with more then one default\n");
                }
                cldefault = t;
            } else {
                t->next = head;
                head = t;
            }
        }
        code_reserved_replace(allstats, 0, 1, (yyvsp[-7].sstr), 1);
        
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
        (yyval.opcodes) = codes_join4(codes_join((yyvsp[-4].opcodes), code_unref()), ophead, allstats, code_pop(1));
    }
#line 3126 "src/parser.c"
    break;

  case 49: /* cases: case  */
#line 306 "src/parser.y"
                    { (yyval.caselist) = caselist_new(pstate, (yyvsp[0].caseitem)); }
#line 3132 "src/parser.c"
    break;

  case 50: /* cases: cases case  */
#line 307 "src/parser.y"
                    { (yyval.caselist) = caselist_insert(pstate, (yyvsp[-1].caselist), (yyvsp[0].caseitem)); }
#line 3138 "src/parser.c"
    break;

  case 51: /* case: CASE expr ':' statements  */
#line 311 "src/parser.y"
                                { (yyval.caseitem) = exprstat_new(pstate, (yyvsp[-2].opcodes), (yyvsp[0].opcodes), 0); }
#line 3144 "src/parser.c"
    break;

  case 52: /* case: DEFAULT ':' statements  */
#line 312 "src/parser.y"
                                { (yyval.caseitem) = exprstat_new(pstate, NULL, (yyvsp[0].opcodes), 1); }
#line 3150 "src/parser.c"
    break;

  case 53: /* case: DEFAULT ':'  */
#line 313 "src/parser.y"
                       { (yyval.caseitem) = exprstat_new(pstate, NULL, code_nop(), 1); }
#line 3156 "src/parser.c"
    break;

  case 54: /* case: CASE expr ':'  */
#line 314 "src/parser.y"
                       { (yyval.caseitem) = exprstat_new(pstate, (yyvsp[-1].opcodes), code_nop(), 0); }
#line 3162 "src/parser.c"
    break;

  case 55: /* try_statement: TRY func_statement_block CATCH '(' IDENTIFIER ')' func_statement_block  */
#line 318 "src/parser.y"
                                                                           {
        Jsi_OpCodes *catchblock = codes_join3(code_scatch(pstate, &(yylsp[-2]), (yyvsp[-2].sstr)), (yyvsp[0].opcodes), code_ecatch(pstate, &(yylsp[0])));
        Jsi_OpCodes *finallyblock = codes_join(code_sfinal(pstate, &(yylsp[-2])), code_efinal(pstate, &(yylsp[-2])));
        Jsi_OpCodes *tryblock = codes_join((yyvsp[-5].opcodes), code_etry(pstate, &(yylsp[-5])));
        (yyval.opcodes) = codes_join4(code_stry(pstate, &(yylsp[-6]), tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
#line 3174 "src/parser.c"
    break;

  case 56: /* try_statement: TRY func_statement_block FINALLY func_statement_block  */
#line 325 "src/parser.y"
                                                            {
        Jsi_OpCodes *catchblock = codes_join(code_scatch(pstate, &(yylsp[-3]), NULL), code_ecatch(pstate, &(yylsp[-3])));
        Jsi_OpCodes *finallyblock = codes_join3(code_sfinal(pstate, &(yylsp[-3])), (yyvsp[0].opcodes), code_efinal(pstate, &(yylsp[0])));
        Jsi_OpCodes *tryblock = codes_join((yyvsp[-2].opcodes), code_etry(pstate, &(yylsp[-2])));
        (yyval.opcodes) = codes_join4(code_stry(pstate, &(yylsp[-3]), tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
#line 3186 "src/parser.c"
    break;

  case 57: /* try_statement: TRY func_statement_block CATCH '(' IDENTIFIER ')' func_statement_block FINALLY func_statement_block  */
#line 333 "src/parser.y"
                                     {
        Jsi_OpCodes *catchblock = codes_join3(code_scatch(pstate, &(yylsp[-4]), (yyvsp[-4].sstr)), (yyvsp[-2].opcodes), code_ecatch(pstate, &(yylsp[-2])));
        Jsi_OpCodes *finallyblock = codes_join3(code_sfinal(pstate, &(yylsp[-8])), (yyvsp[0].opcodes), code_efinal(pstate, &(yylsp[-8])));
        Jsi_OpCodes *tryblock = codes_join((yyvsp[-7].opcodes), code_etry(pstate, &(yylsp[-7])));
        (yyval.opcodes) = codes_join4(code_stry(pstate, &(yylsp[-8]), tryblock->code_len, catchblock->code_len, finallyblock->code_len),
                            tryblock, catchblock, finallyblock);
    }
#line 3198 "src/parser.c"
    break;

  case 58: /* vardecs: vardec  */
#line 342 "src/parser.y"
                            { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3204 "src/parser.c"
    break;

  case 59: /* vardecs: vardecs ',' vardec  */
#line 343 "src/parser.y"
                            { (yyval.opcodes) = codes_join((yyvsp[-2].opcodes), (yyvsp[0].opcodes)); }
#line 3210 "src/parser.c"
    break;

  case 60: /* vardec: IDENTIFIER  */
#line 347 "src/parser.y"
                            {
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &(yylsp[0]), (yyvsp[0].sstr), 1),
                            code_push_undef(),
                            code_assign(pstate, &(yylsp[0]), 1),
                            code_pop(1));
        if (!pstate->eval_flag) jsi_PstateAddVar(pstate, &(yylsp[0]), (yyvsp[0].sstr));
        else ret = codes_join(code_local(pstate, &(yylsp[0]), (yyvsp[0].sstr)), ret);
        (yyval.opcodes) = ret;
    }
#line 3224 "src/parser.c"
    break;

  case 61: /* vardec: IDENTIFIER '=' expr  */
#line 356 "src/parser.y"
                            {
        Jsi_OpCodes *ret = codes_join4(code_push_index(pstate, &(yylsp[-2]), (yyvsp[-2].sstr), 1),
                            (yyvsp[0].opcodes),
                            code_assign(pstate, &(yylsp[-2]), 1),
                            code_pop(1));
        if (!pstate->eval_flag) jsi_PstateAddVar(pstate, &(yylsp[-2]), (yyvsp[-2].sstr));
        else ret = codes_join(code_local(pstate, &(yylsp[-2]), (yyvsp[-2].sstr)), ret);
        (yyval.opcodes) = ret;
    }
#line 3238 "src/parser.c"
    break;

  case 62: /* delete_statement: DELETE lvalue ';'  */
#line 368 "src/parser.y"
                                {
        if (((yyvsp[-1].opcodes))->lvalue_flag&2) {
            (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_delete(2));
        } else {
            (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_delete(1));
        }
    }
#line 3250 "src/parser.c"
    break;

  case 63: /* if_statement: IF '(' expr ')' statement_or_empty  */
#line 378 "src/parser.y"
                                       {
        int offset = ((yyvsp[0].opcodes))->code_len;
        (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), code_jfalse(offset + 1), (yyvsp[0].opcodes));
    }
#line 3259 "src/parser.c"
    break;

  case 64: /* if_statement: IF '(' expr ')' statement_or_empty ELSE statement_or_empty  */
#line 382 "src/parser.y"
                                                                 {
        int len_block2 = ((yyvsp[0].opcodes))->code_len;
        Jsi_OpCodes *block1 = codes_join((yyvsp[-2].opcodes), code_jmp(len_block2 + 1));
        Jsi_OpCodes *condi = codes_join((yyvsp[-4].opcodes), code_jfalse(block1->code_len + 1));
        (yyval.opcodes) = codes_join3(condi, block1, (yyvsp[0].opcodes));
    }
#line 3270 "src/parser.c"
    break;

  case 65: /* inof: IN  */
#line 391 "src/parser.y"
            { (yyval.inum) = 0; }
#line 3276 "src/parser.c"
    break;

  case 66: /* inof: OF  */
#line 392 "src/parser.y"
            { (yyval.inum) = 1;  code_es6(pstate, "for of"); }
#line 3282 "src/parser.c"
    break;

  case 67: /* for_statement: label_opt FOR '(' for_init for_cond ';' expr_opt ')' statement_or_empty  */
#line 396 "src/parser.y"
                                                                            {
        Jsi_OpCodes *init = (yyvsp[-5].opcodes);
        Jsi_OpCodes *cond = (yyvsp[-4].opcodes);
        Jsi_OpCodes *step = ((yyvsp[-2].opcodes) ? codes_join((yyvsp[-2].opcodes), code_pop(1)) : code_nop());
        Jsi_OpCodes *stat = (yyvsp[0].opcodes);
        Jsi_OpCodes *cont_jmp = code_jfalse(step->code_len + stat->code_len + 2);
        Jsi_OpCodes *step_jmp = code_jmp(-(cond->code_len + step->code_len + stat->code_len + 1));
        code_reserved_replace(stat, step->code_len + 1, 0, (yyvsp[-8].sstr), 0);
        (yyval.opcodes) = codes_join(codes_join3(init, cond, cont_jmp),
                           codes_join3(stat, step, step_jmp));
    }
#line 3298 "src/parser.c"
    break;

  case 68: /* for_statement: label_opt FOR '(' localvar IDENTIFIER inof expr ')' statement_or_empty  */
#line 407 "src/parser.y"
                                                                             {
        jsi_ForinVar *fv;
        int inof = (yyvsp[-3].inum);
        Jsi_OpCodes *loc = code_local(pstate, &(yylsp[-4]), (yyvsp[-4].sstr));
        jsi_mark_local(loc, (yyvsp[-5].inum));
        fv = forinvar_new(pstate, (yyvsp[-4].sstr), loc, NULL);
        Jsi_OpCodes *lval;
        if (fv->varname) lval = code_push_index(pstate, &(yylsp[-7]), fv->varname, 1);
        else lval = fv->lval;
        
        Jsi_OpCodes *ret = make_forin(lval, &(yylsp[-7]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), (yyvsp[-8].sstr), (inof!=0));
        if (fv->varname && fv->local) {
            if (!pstate->eval_flag) {
                jsi_PstateAddVar(pstate, &(yylsp[-4]), fv->varname);
                jsi_FreeOpcodes(fv->local);
            } else ret = codes_join(fv->local, ret);
        }
        Jsi_Free(fv);
        (yyval.opcodes) = ret;
    }
#line 3323 "src/parser.c"
    break;

  case 69: /* for_statement: label_opt FOR '(' lvalue inof expr ')' statement_or_empty  */
#line 427 "src/parser.y"
                                                                {
        jsi_ForinVar *fv;
        int inof = (yyvsp[-3].inum);
        if (((yyvsp[-4].opcodes))->lvalue_flag&2) 
            fv = forinvar_new(pstate, NULL, NULL, codes_join((yyvsp[-4].opcodes), code_subscript(pstate, &(yylsp[-4]), 0)));
        else fv = forinvar_new(pstate, NULL, NULL, (yyvsp[-4].opcodes));
        Jsi_OpCodes *lval;
        if (fv->varname) lval = code_push_index(pstate, &(yylsp[-7]), fv->varname, 0);
        else lval = fv->lval;
        
        Jsi_OpCodes *ret = make_forin(lval, &(yylsp[-6]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), (yyvsp[-7].sstr), (inof!=0));
        if (fv->varname && fv->local) {
            if (!pstate->eval_flag) {
                jsi_PstateAddVar(pstate, &(yylsp[-4]), fv->varname);
                jsi_FreeOpcodes(fv->local);
            } else ret = codes_join(fv->local, ret);
        }
        Jsi_Free(fv);
        (yyval.opcodes) = ret;
    }
#line 3348 "src/parser.c"
    break;

  case 70: /* for_init: ';'  */
#line 451 "src/parser.y"
                        { (yyval.opcodes) = code_nop(); }
#line 3354 "src/parser.c"
    break;

  case 71: /* for_init: expr ';'  */
#line 452 "src/parser.y"
                        { (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_pop(1)); }
#line 3360 "src/parser.c"
    break;

  case 72: /* for_init: localvar vardecs ';'  */
#line 453 "src/parser.y"
                           {
        jsi_mark_local((yyvsp[-1].opcodes), (yyvsp[-2].inum));
        (yyval.opcodes) = (yyvsp[-1].opcodes);
    }
#line 3369 "src/parser.c"
    break;

  case 73: /* for_cond: %empty  */
#line 459 "src/parser.y"
                        { (yyval.opcodes) = code_push_bool(1); }
#line 3375 "src/parser.c"
    break;

  case 74: /* for_cond: expr  */
#line 460 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3381 "src/parser.c"
    break;

  case 75: /* expr_opt: %empty  */
#line 463 "src/parser.y"
                        { (yyval.opcodes) = NULL; }
#line 3387 "src/parser.c"
    break;

  case 76: /* expr_opt: expr  */
#line 464 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3393 "src/parser.c"
    break;

  case 77: /* while_statement: label_opt WHILE '(' expr ')' statement_or_empty  */
#line 468 "src/parser.y"
                                                    {
        Jsi_OpCodes *cond = (yyvsp[-2].opcodes);
        Jsi_OpCodes *stat = (yyvsp[0].opcodes);
        code_reserved_replace(stat, 1, 0, (yyvsp[-5].sstr), 0);
        (yyval.opcodes) = codes_join4(cond, code_jfalse(stat->code_len + 2), stat,
                           code_jmp(-(stat->code_len + cond->code_len + 1)));
    }
#line 3405 "src/parser.c"
    break;

  case 78: /* do_statement: label_opt DO statement_or_empty WHILE '(' expr ')'  */
#line 478 "src/parser.y"
                                                       {
        Jsi_OpCodes *stat = (yyvsp[-4].opcodes);
        Jsi_OpCodes *cond = (yyvsp[-1].opcodes);
        code_reserved_replace(stat, cond->code_len + 1, 0, (yyvsp[-6].sstr), 0);
        (yyval.opcodes) = codes_join3(stat, cond,
                            code_jtrue(-(stat->code_len + cond->code_len)));
    }
#line 3417 "src/parser.c"
    break;

  case 79: /* func_expr: FUNC '(' args_opt ')' func_statement_block  */
#line 488 "src/parser.y"
                                               {
        (yyval.opcodes) = code_push_func(pstate,  &(yylsp[-2]), jsi_FuncMake(pstate, (yyvsp[-2].scopes), (yyvsp[0].opcodes), &(yylsp[-4]), NULL, 0));
        jsi_PstatePop(pstate);
    }
#line 3426 "src/parser.c"
    break;

  case 80: /* func_expr: FUNC '(' args_opt ')' ':' rettype func_statement_block  */
#line 492 "src/parser.y"
                                                             {
        (yyvsp[-4].scopes)->retType = (yyvsp[-1].inum);
        (yyval.opcodes) = code_push_func(pstate,  &(yylsp[-4]), jsi_FuncMake(pstate, (yyvsp[-4].scopes), (yyvsp[0].opcodes), &(yylsp[-6]), NULL, 0));
        jsi_PstatePop(pstate);
    }
#line 3436 "src/parser.c"
    break;

  case 81: /* func_expr: FUNC IDENTIFIER '(' args_opt ')' func_statement_block  */
#line 497 "src/parser.y"
                                                            {
        (yyval.opcodes) = code_push_func(pstate, &(yylsp[-3]), jsi_FuncMake(pstate, (yyvsp[-2].scopes), (yyvsp[0].opcodes), &(yylsp[-5]), (yyvsp[-4].sstr), 0));
        jsi_PstatePop(pstate);
    }
#line 3445 "src/parser.c"
    break;

  case 82: /* func_expr: FUNC IDENTIFIER '(' args_opt ')' ':' rettype func_statement_block  */
#line 501 "src/parser.y"
                                                                        {
        (yyvsp[-4].scopes)->retType = (yyvsp[-1].inum);
        (yyval.opcodes) = code_push_func(pstate, &(yylsp[-5]), jsi_FuncMake(pstate, (yyvsp[-4].scopes), (yyvsp[0].opcodes), &(yylsp[-7]), (yyvsp[-6].sstr), 0));
        jsi_PstatePop(pstate);
    }
#line 3455 "src/parser.c"
    break;

  case 83: /* args_opt: %empty  */
#line 508 "src/parser.y"
          { (yyval.scopes) = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew()); }
#line 3461 "src/parser.c"
    break;

  case 84: /* args_opt: args  */
#line 509 "src/parser.y"
           {
        (yyval.scopes) = jsi_ArgsOptAdd(pstate, (yyvsp[0].scopes));
    }
#line 3469 "src/parser.c"
    break;

  case 85: /* args_opt: '.' '.' '.'  */
#line 512 "src/parser.y"
                  {
        Jsi_ScopeStrs *s = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew());
        s->varargs = 1;
        (yyval.scopes) = s;
    }
#line 3479 "src/parser.c"
    break;

  case 86: /* args_opt: args ',' '.' '.' '.'  */
#line 517 "src/parser.y"
                           {
        Jsi_ScopeStrs *s = jsi_ArgsOptAdd(pstate, (yyvsp[-4].scopes));
        s->varargs = 1;
        (yyval.scopes) = s;
    }
#line 3489 "src/parser.c"
    break;

  case 87: /* typeid: TYPESTRING  */
#line 525 "src/parser.y"
               {    (yyval.inum) = (pstate->argType |= JSI_TT_STRING); }
#line 3495 "src/parser.c"
    break;

  case 88: /* typeid: TYPENUMBER  */
#line 526 "src/parser.y"
                 {  (yyval.inum) = (pstate->argType |= JSI_TT_NUMBER); }
#line 3501 "src/parser.c"
    break;

  case 89: /* typeid: TYPEBOOLEAN  */
#line 527 "src/parser.y"
                  { (yyval.inum) = (pstate->argType |= JSI_TT_BOOLEAN); }
#line 3507 "src/parser.c"
    break;

  case 90: /* typeid: TYPEREGEXP  */
#line 528 "src/parser.y"
                 {  (yyval.inum) = (pstate->argType |= JSI_TT_REGEXP); }
#line 3513 "src/parser.c"
    break;

  case 91: /* typeid: TYPEOBJECT  */
#line 529 "src/parser.y"
                 {  (yyval.inum) = (pstate->argType |= JSI_TT_OBJECT); }
#line 3519 "src/parser.c"
    break;

  case 92: /* typeid: TYPEUSEROBJ  */
#line 530 "src/parser.y"
                  { (yyval.inum) = (pstate->argType |= JSI_TT_USEROBJ); }
#line 3525 "src/parser.c"
    break;

  case 93: /* typeid: TYPEITEROBJ  */
#line 531 "src/parser.y"
                  { (yyval.inum) = (pstate->argType |= JSI_TT_ITEROBJ); }
#line 3531 "src/parser.c"
    break;

  case 94: /* typeid: TYPEANY  */
#line 532 "src/parser.y"
              {     (yyval.inum) = (pstate->argType |= JSI_TT_ANY); }
#line 3537 "src/parser.c"
    break;

  case 95: /* typeid: TYPEARRAY  */
#line 533 "src/parser.y"
                {   (yyval.inum) = (pstate->argType |= JSI_TT_ARRAY); }
#line 3543 "src/parser.c"
    break;

  case 96: /* typeid: TYPENULL  */
#line 534 "src/parser.y"
               {    (yyval.inum) = (pstate->argType |= JSI_TT_NULL); }
#line 3549 "src/parser.c"
    break;

  case 97: /* typeid: FUNC  */
#line 535 "src/parser.y"
           {        (yyval.inum) = (pstate->argType |= JSI_TT_FUNCTION); }
#line 3555 "src/parser.c"
    break;

  case 98: /* typeid: UNDEF  */
#line 536 "src/parser.y"
            {       (yyval.inum) = (pstate->argType |= JSI_TT_UNDEFINED); }
#line 3561 "src/parser.c"
    break;

  case 99: /* typeid: VOID  */
#line 537 "src/parser.y"
           {        (yyval.inum) = (pstate->argType |= JSI_TT_VOID); }
#line 3567 "src/parser.c"
    break;

  case 100: /* rettype: argtype  */
#line 540 "src/parser.y"
            {
        if (pstate->args)
            pstate->args->retType = pstate->argType;
        (yyval.inum) = pstate->argType;
        pstate->argType = 0;
    }
#line 3578 "src/parser.c"
    break;

  case 101: /* argtype: typeid  */
#line 548 "src/parser.y"
           {
        (yyval.inum) = pstate->argType;
    }
#line 3586 "src/parser.c"
    break;

  case 102: /* argtype: argtype '|' typeid  */
#line 551 "src/parser.y"
                         {
        (yyval.inum) = pstate->argType;
    }
#line 3594 "src/parser.c"
    break;

  case 103: /* strlit: STRING  */
#line 555 "src/parser.y"
               { (yyval.vstr) = (yyvsp[0].vstr); }
#line 3600 "src/parser.c"
    break;

  case 104: /* argdefault: UNDEF  */
#line 558 "src/parser.y"
          {     (yyval.value) = Jsi_ValueNew(pstate->interp); (yyval.value)->d.num = 0; }
#line 3606 "src/parser.c"
    break;

  case 105: /* argdefault: VOID  */
#line 559 "src/parser.y"
           {    (yyval.value) = Jsi_ValueNew(pstate->interp); (yyval.value)->d.num = 1; }
#line 3612 "src/parser.c"
    break;

  case 106: /* argdefault: '-' FNUMBER  */
#line 560 "src/parser.y"
                            { *(yyvsp[0].num) = *(yyvsp[0].num) * -1; (yyval.value) = Jsi_ValueNewNumber(pstate->interp, *(yyvsp[0].num)); Jsi_Free((yyvsp[0].num));}
#line 3618 "src/parser.c"
    break;

  case 107: /* argdefault: FNUMBER  */
#line 561 "src/parser.y"
              { (yyval.value) = Jsi_ValueNewNumber(pstate->interp, *(yyvsp[0].num)); Jsi_Free((yyvsp[0].num)); }
#line 3624 "src/parser.c"
    break;

  case 108: /* argdefault: _TRUE  */
#line 562 "src/parser.y"
            {   (yyval.value) = Jsi_ValueNewBoolean(pstate->interp, 1); }
#line 3630 "src/parser.c"
    break;

  case 109: /* argdefault: _FALSE  */
#line 563 "src/parser.y"
             {  (yyval.value) = Jsi_ValueNewBoolean(pstate->interp, 0); }
#line 3636 "src/parser.c"
    break;

  case 110: /* argdefault: TYPENULL  */
#line 564 "src/parser.y"
               {(yyval.value) = Jsi_ValueNewNull(pstate->interp); }
#line 3642 "src/parser.c"
    break;

  case 111: /* argdefault: strlit  */
#line 565 "src/parser.y"
             {  (yyval.value) = Jsi_ValueNewBlob(pstate->interp, (uchar*)(yyvsp[0].vstr)->str, (yyvsp[0].vstr)->len); }
#line 3648 "src/parser.c"
    break;

  case 112: /* args: IDENTIFIER  */
#line 568 "src/parser.y"
                { (yyval.scopes) = jsi_argInsert(pstate, NULL, (yyvsp[0].sstr), NULL, &(yylsp[0]), 0 ); }
#line 3654 "src/parser.c"
    break;

  case 113: /* args: IDENTIFIER '=' argdefault  */
#line 569 "src/parser.y"
                                { (yyval.scopes) = jsi_argInsert(pstate, NULL, (yyvsp[-2].sstr), (yyvsp[0].value), &(yylsp[-2]), 0); }
#line 3660 "src/parser.c"
    break;

  case 114: /* args: IDENTIFIER ':' argtype  */
#line 570 "src/parser.y"
                             { (yyval.scopes) = jsi_argInsert(pstate, NULL, (yyvsp[-2].sstr), NULL, &(yylsp[-2]), 0);}
#line 3666 "src/parser.c"
    break;

  case 115: /* args: IDENTIFIER ':' argtype '=' argdefault  */
#line 571 "src/parser.y"
                                            { (yyval.scopes) = jsi_argInsert(pstate, NULL, (yyvsp[-4].sstr), (yyvsp[0].value), &(yylsp[-4]), 0);}
#line 3672 "src/parser.c"
    break;

  case 116: /* args: args ',' IDENTIFIER  */
#line 572 "src/parser.y"
                          { (yyval.scopes) = jsi_argInsert(pstate, (yyvsp[-2].scopes), (yyvsp[0].sstr), NULL, &(yylsp[-2]), 0); }
#line 3678 "src/parser.c"
    break;

  case 117: /* args: args ',' IDENTIFIER '=' argdefault  */
#line 573 "src/parser.y"
                                         { (yyval.scopes) = jsi_argInsert(pstate, (yyvsp[-4].scopes), (yyvsp[-2].sstr), (yyvsp[0].value), &(yylsp[-4]), 0); }
#line 3684 "src/parser.c"
    break;

  case 118: /* args: args ',' IDENTIFIER ':' argtype  */
#line 574 "src/parser.y"
                                      { (yyval.scopes) = jsi_argInsert(pstate, (yyvsp[-4].scopes), (yyvsp[-2].sstr), NULL, &(yylsp[-4]), 0);}
#line 3690 "src/parser.c"
    break;

  case 119: /* args: args ',' IDENTIFIER ':' argtype '=' argdefault  */
#line 575 "src/parser.y"
                                                     { (yyval.scopes) = jsi_argInsert(pstate, (yyvsp[-6].scopes), (yyvsp[-4].sstr), (yyvsp[0].value), &(yylsp[-6]), 0);}
#line 3696 "src/parser.c"
    break;

  case 120: /* argsa: IDENTIFIER  */
#line 579 "src/parser.y"
                            { (yyval.scopes) = jsi_argInsert(pstate, NULL, (yyvsp[0].sstr), NULL, &(yylsp[0]), 0); }
#line 3702 "src/parser.c"
    break;

  case 121: /* argsa: argsa ',' IDENTIFIER  */
#line 580 "src/parser.y"
                            { (yyval.scopes) = jsi_argInsert(pstate, (yyvsp[-2].scopes), (yyvsp[0].sstr), NULL, &(yylsp[-2]), 0); }
#line 3708 "src/parser.c"
    break;

  case 122: /* arrowargs: IDENTIFIER  */
#line 584 "src/parser.y"
                                    { (yyval.scopes) = jsi_argInsert(pstate, NULL, (yyvsp[0].sstr), NULL, &(yylsp[0]), 0); }
#line 3714 "src/parser.c"
    break;

  case 123: /* arrowargs: '(' IDENTIFIER ',' argsa ')'  */
#line 585 "src/parser.y"
                                    { (yyval.scopes) = jsi_argInsert(pstate, (yyvsp[-1].scopes), (yyvsp[-3].sstr), NULL, &(yylsp[-4]), 1); }
#line 3720 "src/parser.c"
    break;

  case 124: /* arrowargs: '(' ')'  */
#line 586 "src/parser.y"
                          { (yyval.scopes) = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew()); }
#line 3726 "src/parser.c"
    break;

  case 125: /* func_statement_block: '{' statements '}'  */
#line 589 "src/parser.y"
                                            { (yyval.opcodes) = (yyvsp[-1].opcodes); }
#line 3732 "src/parser.c"
    break;

  case 126: /* func_statement_block: '{' '}'  */
#line 590 "src/parser.y"
                                            { (yyval.opcodes) = code_nop(); }
#line 3738 "src/parser.c"
    break;

  case 127: /* expr: value  */
#line 594 "src/parser.y"
                            { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3744 "src/parser.c"
    break;

  case 128: /* expr: func_expr  */
#line 595 "src/parser.y"
                            { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 3750 "src/parser.c"
    break;

  case 129: /* expr: lvalue  */
#line 596 "src/parser.y"
                            { 
        if (((yyvsp[0].opcodes))->lvalue_flag&2) (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_subscript(pstate, &(yylsp[0]), 1 |(((yyvsp[0].opcodes))->lvalue_flag&4?2:0))); 
        else (yyval.opcodes) = (yyvsp[0].opcodes);
    }
#line 3759 "src/parser.c"
    break;

  case 130: /* expr: expr ',' expr  */
#line 600 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), code_pop(1), (yyvsp[0].opcodes)); }
#line 3765 "src/parser.c"
    break;

  case 131: /* expr: expr '[' expr ']'  */
#line 601 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-3].opcodes), (yyvsp[-1].opcodes), code_subscript(pstate, &(yylsp[-3]), 1)); }
#line 3771 "src/parser.c"
    break;

  case 132: /* expr: expr '.' IDENTIFIER  */
#line 602 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), code_push_string(pstate,&(yylsp[0]),(yyvsp[0].sstr)), code_subscript(pstate, &(yylsp[0]), 3)); }
#line 3777 "src/parser.c"
    break;

  case 133: /* expr: '-' expr  */
#line 603 "src/parser.y"
                            { (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_neg()); }
#line 3783 "src/parser.c"
    break;

  case 134: /* expr: '+' expr  */
#line 604 "src/parser.y"
                            { (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_pos()); }
#line 3789 "src/parser.c"
    break;

  case 135: /* expr: '~' expr  */
#line 605 "src/parser.y"
                            { (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_bnot()); }
#line 3795 "src/parser.c"
    break;

  case 136: /* expr: '!' expr  */
#line 606 "src/parser.y"
                            { (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_not()); }
#line 3801 "src/parser.c"
    break;

  case 137: /* expr: VOID expr  */
#line 607 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[0].opcodes), code_pop(1), code_push_undef()); }
#line 3807 "src/parser.c"
    break;

  case 138: /* expr: expr '*' expr  */
#line 608 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_mul()); }
#line 3813 "src/parser.c"
    break;

  case 139: /* expr: expr '/' expr  */
#line 609 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_div()); }
#line 3819 "src/parser.c"
    break;

  case 140: /* expr: expr '%' expr  */
#line 610 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_mod()); }
#line 3825 "src/parser.c"
    break;

  case 141: /* expr: expr '+' expr  */
#line 611 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_add()); }
#line 3831 "src/parser.c"
    break;

  case 142: /* expr: expr '-' expr  */
#line 612 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_sub()); }
#line 3837 "src/parser.c"
    break;

  case 143: /* expr: expr IN expr  */
#line 613 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_in()); }
#line 3843 "src/parser.c"
    break;

  case 144: /* expr: lvalue INC  */
#line 614 "src/parser.y"
                            {
        if (((yyvsp[-1].opcodes))->lvalue_flag&2) (yyval.opcodes) = codes_join3((yyvsp[-1].opcodes), code_subscript(pstate, &(yylsp[-1]), 0), code_inc(pstate, &(yylsp[-1]), 1));
        else (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_inc(pstate, &(yylsp[-1]), 1));
    }
#line 3852 "src/parser.c"
    break;

  case 145: /* expr: lvalue DEC  */
#line 618 "src/parser.y"
                            { 
        if (((yyvsp[-1].opcodes))->lvalue_flag&2) (yyval.opcodes) = codes_join3((yyvsp[-1].opcodes), code_subscript(pstate, &(yylsp[-1]), 0), code_dec(pstate, &(yylsp[-1]), 1));
        else (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_dec(pstate, &(yylsp[-1]), 1)); 
    }
#line 3861 "src/parser.c"
    break;

  case 146: /* expr: INC lvalue  */
#line 622 "src/parser.y"
                            {
        if (((yyvsp[0].opcodes))->lvalue_flag&2) (yyval.opcodes) = codes_join3((yyvsp[0].opcodes), code_subscript(pstate, &(yylsp[0]), 0), code_inc(pstate, &(yylsp[0]), 0));
        else (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_inc(pstate, &(yylsp[0]), 0));
    }
#line 3870 "src/parser.c"
    break;

  case 147: /* expr: TYPEOF expr  */
#line 626 "src/parser.y"
                  {
        if (((yyvsp[0].opcodes))->lvalue_flag&2) (yyval.opcodes) = codes_join3((yyvsp[0].opcodes), code_subscript(pstate, &(yylsp[0]), 0), code_typeof(pstate, &(yylsp[0]), 0));
        else (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_typeof(pstate, &(yylsp[0]), 0));
    }
#line 3879 "src/parser.c"
    break;

  case 148: /* expr: DEC lvalue  */
#line 630 "src/parser.y"
                            { 
        if (((yyvsp[0].opcodes))->lvalue_flag&2) (yyval.opcodes) = codes_join3((yyvsp[0].opcodes), code_subscript(pstate, &(yylsp[0]), 0), code_dec(pstate, &(yylsp[0]), 0));
        else (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_dec(pstate, &(yylsp[0]), 0));
    }
#line 3888 "src/parser.c"
    break;

  case 149: /* expr: '(' expr ')'  */
#line 634 "src/parser.y"
                            { (yyval.opcodes) = (yyvsp[-1].opcodes); }
#line 3894 "src/parser.c"
    break;

  case 150: /* expr: arrowargs ARROW expr  */
#line 635 "src/parser.y"
                                         {
        jsi_PstatePush(pstate);
        (yyval.opcodes) = code_push_func(pstate,  &(yylsp[-2]), jsi_FuncMake(pstate, (yyvsp[-2].scopes), codes_join((yyvsp[0].opcodes), code_ret(pstate, &(yylsp[0]), 1)), &(yylsp[-2]), NULL, 1)); 
        jsi_PstatePop(pstate);
    }
#line 3904 "src/parser.c"
    break;

  case 151: /* expr: arrowargs ARROW func_statement_block  */
#line 640 "src/parser.y"
                                           {
        jsi_PstatePush(pstate);
        (yyval.opcodes) = code_push_func(pstate,  &(yylsp[-2]), jsi_FuncMake(pstate, (yyvsp[-2].scopes), (yyvsp[0].opcodes), &(yylsp[-2]), NULL, 1));
        jsi_PstatePop(pstate);
    }
#line 3914 "src/parser.c"
    break;

  case 152: /* expr: expr AND expr  */
#line 645 "src/parser.y"
                            {
        Jsi_OpCodes *expr2 = codes_join(code_pop(1), (yyvsp[0].opcodes));
        (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), code_jfalse_np(expr2->code_len + 1), expr2);
    }
#line 3923 "src/parser.c"
    break;

  case 153: /* expr: expr OR expr  */
#line 649 "src/parser.y"
                            {
        Jsi_OpCodes *expr2 = codes_join(code_pop(1), (yyvsp[0].opcodes));
        (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), code_jtrue_np(expr2->code_len + 1), expr2);
    }
#line 3932 "src/parser.c"
    break;

  case 154: /* expr: expr '<' expr  */
#line 653 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_less()); }
#line 3938 "src/parser.c"
    break;

  case 155: /* expr: expr '>' expr  */
#line 654 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_greater()); }
#line 3944 "src/parser.c"
    break;

  case 156: /* expr: expr LEQ expr  */
#line 655 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_lessequ()); }
#line 3950 "src/parser.c"
    break;

  case 157: /* expr: expr GEQ expr  */
#line 656 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_greaterequ()); }
#line 3956 "src/parser.c"
    break;

  case 158: /* expr: expr EQU expr  */
#line 657 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_equal()); }
#line 3962 "src/parser.c"
    break;

  case 159: /* expr: expr NEQ expr  */
#line 658 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_notequal()); }
#line 3968 "src/parser.c"
    break;

  case 160: /* expr: expr EEQU expr  */
#line 659 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_eequ());    }
#line 3974 "src/parser.c"
    break;

  case 161: /* expr: expr NNEQ expr  */
#line 660 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_nneq()); }
#line 3980 "src/parser.c"
    break;

  case 162: /* expr: expr '&' expr  */
#line 661 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_band()); }
#line 3986 "src/parser.c"
    break;

  case 163: /* expr: expr '|' expr  */
#line 662 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_bor()); }
#line 3992 "src/parser.c"
    break;

  case 164: /* expr: expr '^' expr  */
#line 663 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_bxor()); }
#line 3998 "src/parser.c"
    break;

  case 165: /* expr: expr LSHF expr  */
#line 664 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_shf(0)); }
#line 4004 "src/parser.c"
    break;

  case 166: /* expr: expr RSHF expr  */
#line 665 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_shf(1)); }
#line 4010 "src/parser.c"
    break;

  case 167: /* expr: expr URSHF expr  */
#line 666 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_shf(2)); }
#line 4016 "src/parser.c"
    break;

  case 168: /* expr: lvalue '=' expr  */
#line 667 "src/parser.y"
                            { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_assign(pstate, &(yylsp[-2]), ((yyvsp[-2].opcodes))->lvalue_flag)); }
#line 4022 "src/parser.c"
    break;

  case 169: /* expr: lvalue ADDAS expr  */
#line 668 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_add()); }
#line 4028 "src/parser.c"
    break;

  case 170: /* expr: lvalue MNSAS expr  */
#line 669 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_sub()); }
#line 4034 "src/parser.c"
    break;

  case 171: /* expr: lvalue MULAS expr  */
#line 670 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_mul()); }
#line 4040 "src/parser.c"
    break;

  case 172: /* expr: lvalue MODAS expr  */
#line 671 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_mod()); }
#line 4046 "src/parser.c"
    break;

  case 173: /* expr: lvalue LSHFAS expr  */
#line 672 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_shf(0)); }
#line 4052 "src/parser.c"
    break;

  case 174: /* expr: lvalue RSHFAS expr  */
#line 673 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_shf(1)); }
#line 4058 "src/parser.c"
    break;

  case 175: /* expr: lvalue URSHFAS expr  */
#line 674 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_shf(2)); }
#line 4064 "src/parser.c"
    break;

  case 176: /* expr: lvalue BANDAS expr  */
#line 675 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_band()); }
#line 4070 "src/parser.c"
    break;

  case 177: /* expr: lvalue BORAS expr  */
#line 676 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_bor()); }
#line 4076 "src/parser.c"
    break;

  case 178: /* expr: lvalue BXORAS expr  */
#line 677 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_bxor()); }
#line 4082 "src/parser.c"
    break;

  case 179: /* expr: lvalue DIVAS expr  */
#line 678 "src/parser.y"
                            { (yyval.opcodes) = opassign(pstate, &(yylsp[-2]), (yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_div()); }
#line 4088 "src/parser.c"
    break;

  case 180: /* expr: lvalue INSTANCEOF expr  */
#line 679 "src/parser.y"
                              { (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), (yyvsp[0].opcodes), code_instanceof()); }
#line 4094 "src/parser.c"
    break;

  case 181: /* expr: fcall_exprs  */
#line 680 "src/parser.y"
                            { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 4100 "src/parser.c"
    break;

  case 182: /* expr: NEW value  */
#line 682 "src/parser.y"
                            { (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_newfcall(pstate, &(yylsp[-1]), 0, NULL, (yyvsp[0].opcodes))); }
#line 4106 "src/parser.c"
    break;

  case 183: /* expr: NEW lvalue  */
#line 683 "src/parser.y"
                            { 
        if (((yyvsp[0].opcodes))->lvalue_flag&2) (yyval.opcodes) = codes_join3((yyvsp[0].opcodes), code_subscript(pstate, &(yylsp[0]), 1), code_newfcall(pstate, &(yylsp[0]), 0, NULL, (yyvsp[0].opcodes)));
        else (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_newfcall(pstate, &(yylsp[0]), 0, NULL, (yyvsp[0].opcodes)));}
#line 4114 "src/parser.c"
    break;

  case 184: /* expr: NEW '(' expr ')'  */
#line 686 "src/parser.y"
                            { (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_newfcall(pstate, &(yylsp[-3]),0, NULL, (yyvsp[-1].opcodes))); }
#line 4120 "src/parser.c"
    break;

  case 185: /* expr: NEW func_expr  */
#line 687 "src/parser.y"
                            { (yyval.opcodes) = codes_join((yyvsp[0].opcodes), code_newfcall(pstate, &(yylsp[-1]),0, NULL, (yyvsp[0].opcodes))); }
#line 4126 "src/parser.c"
    break;

  case 186: /* expr: NEW value '(' exprlist_opt ')'  */
#line 688 "src/parser.y"
                                            {
        Jsi_OpCodes *lval = (yyvsp[-3].opcodes);
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        (yyval.opcodes) = codes_join3((yyvsp[-3].opcodes), (opl ? opl : code_nop()), code_newfcall(pstate, &(yylsp[-4]), expr_cnt, lval->lvalue_name, opl));
    }
#line 4137 "src/parser.c"
    break;

  case 187: /* expr: NEW lvalue '(' exprlist_opt ')'  */
#line 694 "src/parser.y"
                                            {
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        Jsi_OpCodes *lv = NULL;
        if (((yyvsp[-3].opcodes))->lvalue_flag&2) lv = codes_join((yyvsp[-3].opcodes), code_subscript(pstate, &(yylsp[-3]), 1));
        else lv = (yyvsp[-3].opcodes);
        (yyval.opcodes) = codes_join3(lv, (opl ? opl : code_nop()), code_newfcall(pstate, &(yylsp[-4]),expr_cnt, lv?lv->lvalue_name:NULL, opl));
    }
#line 4150 "src/parser.c"
    break;

  case 188: /* expr: NEW '(' expr ')' '(' exprlist_opt ')'  */
#line 702 "src/parser.y"
                                            { 
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        (yyval.opcodes) = codes_join3((yyvsp[-4].opcodes), (opl ? opl : code_nop()), code_newfcall(pstate, &(yylsp[-6]),expr_cnt, NULL, opl));
    }
#line 4160 "src/parser.c"
    break;

  case 189: /* expr: NEW func_expr '(' exprlist_opt ')'  */
#line 707 "src/parser.y"
                                            {
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        (yyval.opcodes) = codes_join3((yyvsp[-3].opcodes), (opl ? opl : code_nop()), code_newfcall(pstate, &(yylsp[-4]),expr_cnt, NULL, opl));
    }
#line 4170 "src/parser.c"
    break;

  case 190: /* expr: func_expr '(' exprlist_opt ')'  */
#line 712 "src/parser.y"
                                        {
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        (yyval.opcodes) = codes_join3((yyvsp[-3].opcodes), (opl ? opl : code_nop()), code_fcall(pstate, &(yylsp[-3]),expr_cnt, NULL, NULL, opl, NULL));
    }
#line 4180 "src/parser.c"
    break;

  case 191: /* expr: expr '?' expr ':' expr  */
#line 717 "src/parser.y"
                             {
        Jsi_OpCodes *expr2 = codes_join((yyvsp[-2].opcodes), code_jmp(((yyvsp[0].opcodes))->code_len + 1));
        (yyval.opcodes) = codes_join4((yyvsp[-4].opcodes), code_jfalse(expr2->code_len + 1), expr2, (yyvsp[0].opcodes));
    }
#line 4189 "src/parser.c"
    break;

  case 192: /* expr: __DEBUG  */
#line 721 "src/parser.y"
              { (yyval.opcodes) = code_debug(pstate,&(yylsp[0])); }
#line 4195 "src/parser.c"
    break;

  case 193: /* fcall_exprs: expr '.' IDENTIFIER '(' exprlist_opt ')'  */
#line 725 "src/parser.y"
                                             {
        Jsi_OpCodes *lval = (yyvsp[-5].opcodes);
        const char *n1 = lval->lvalue_name;
        const char *n2 = (yyvsp[-3].sstr);
        Jsi_OpCodes *ff = codes_join4((yyvsp[-5].opcodes), code_push_string(pstate,&(yylsp[-3]), (yyvsp[-3].sstr)), code_chthis(pstate,&(yylsp[-5]), 1), code_subscript(pstate, &(yylsp[-5]), 1));
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        (yyval.opcodes) = codes_join3(ff, (opl ? opl : code_nop()), code_fcall(pstate, &(yylsp[-3]), expr_cnt, n1, n2, opl, NULL));
    }
#line 4209 "src/parser.c"
    break;

  case 194: /* fcall_exprs: expr '[' expr ']' '(' exprlist_opt ')'  */
#line 734 "src/parser.y"
                                             {
        Jsi_OpCodes *ff = codes_join4((yyvsp[-6].opcodes), (yyvsp[-4].opcodes), code_chthis(pstate,&(yylsp[-6]), 1), code_subscript(pstate, &(yylsp[-6]), 1));
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        (yyval.opcodes) = codes_join3(ff, (opl ? opl : code_nop()), code_fcall(pstate, &(yylsp[-4]), expr_cnt, NULL, NULL, opl, NULL));
    }
#line 4220 "src/parser.c"
    break;

  case 195: /* fcall_exprs: fcall_exprs '(' exprlist_opt ')'  */
#line 740 "src/parser.y"
                                       {
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        (yyval.opcodes) = codes_join4((yyvsp[-3].opcodes), code_chthis(pstate,&(yylsp[-3]), 0), (opl ? opl : code_nop()), code_fcall(pstate, &(yylsp[-2]),expr_cnt, NULL, NULL, opl, NULL));
    }
#line 4230 "src/parser.c"
    break;

  case 196: /* fcall_exprs: '(' expr ')' '(' exprlist_opt ')'  */
#line 745 "src/parser.y"
                                        {
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        (yyval.opcodes) = codes_join4((yyvsp[-4].opcodes), code_chthis(pstate,&(yylsp[-5]), 0), (opl ? opl : code_nop()), code_fcall(pstate, &(yylsp[-3]),expr_cnt, NULL, NULL, opl, NULL));
    }
#line 4240 "src/parser.c"
    break;

  case 197: /* fcall_exprs: lvalue '(' exprlist_opt ')'  */
#line 750 "src/parser.y"
                                  {
        Jsi_OpCodes *opl = (yyvsp[-1].opcodes);
        int expr_cnt = opl ? opl->expr_counter:0;
        Jsi_OpCodes *pref;
        Jsi_OpCodes *lval = (yyvsp[-3].opcodes);
        const char *n1 = lval->lvalue_name;
        if (lval->lvalue_flag&2) {
            const char *n2 = NULL;
            pref = codes_join3((yyvsp[-3].opcodes), code_chthis(pstate,&(yylsp[-3]), 1), code_subscript(pstate, &(yylsp[-3]), 1));
            if (pref->code_len>=2 && pref->codes[0].op == OP_PUSHVAR && pref->codes[1].op == OP_PUSHSTR && !n1) {
                jsi_FastVar *fv = (typeof(fv))pref->codes[0].data;
                n2 = fv->varname;
                n1 = (const char*)pref->codes[1].data;
            }
            (yyval.opcodes) = codes_join3(pref, (opl ? opl : code_nop()), code_fcall(pstate, &(yylsp[-2]), expr_cnt, n1, n2, opl, NULL));
        } else {
            if (lval->lvalue_name && Jsi_Strcmp(lval->lvalue_name, "eval") == 0) {
                (yyval.opcodes) = codes_join((opl ? opl : code_nop()), code_eval(pstate, &(yylsp[-3]), expr_cnt, lval));
            } else {
                jsi_Pline *jpl = &(yylsp[-3]);
                pref = codes_join((yyvsp[-3].opcodes), code_chthis(pstate,&(yylsp[-3]), 0));
                (yyval.opcodes) = codes_join3(pref, (opl ? opl : code_nop()), code_fcall(pstate, jpl, expr_cnt, n1, NULL, opl, pref));
            }
        }
    }
#line 4270 "src/parser.c"
    break;

  case 198: /* lvalue: IDENTIFIER  */
#line 778 "src/parser.y"
                            {
        Jsi_OpCodes *lval = code_push_index(pstate, &(yylsp[0]), (yyvsp[0].sstr), 0); 
        (yyval.opcodes) = lval;
        lval->lvalue_flag = 1; 
        lval->lvalue_name = (yyvsp[0].sstr); 
    }
#line 4281 "src/parser.c"
    break;

  case 199: /* lvalue: ARGUMENTS  */
#line 784 "src/parser.y"
                            { (yyval.opcodes) = code_push_args(); ((yyval.opcodes))->lvalue_flag = 1; }
#line 4287 "src/parser.c"
    break;

  case 200: /* lvalue: _THIS  */
#line 785 "src/parser.y"
                            { (yyval.opcodes) = code_push_this(pstate,&(yylsp[0])); ((yyval.opcodes))->lvalue_flag = 1; }
#line 4293 "src/parser.c"
    break;

  case 201: /* lvalue: lvalue '[' expr ']'  */
#line 786 "src/parser.y"
                            {
        if (((yyvsp[-3].opcodes))->lvalue_flag&2) (yyval.opcodes) = codes_join3((yyvsp[-3].opcodes), code_subscript(pstate, &(yylsp[-3]), 1), (yyvsp[-1].opcodes)); 
        else (yyval.opcodes) = codes_join((yyvsp[-3].opcodes), (yyvsp[-1].opcodes)); 
        ((yyval.opcodes))->lvalue_flag = 2;
    }
#line 4303 "src/parser.c"
    break;

  case 202: /* lvalue: lvalue '.' IDENTIFIER  */
#line 791 "src/parser.y"
                            {
        if (((yyvsp[-2].opcodes))->lvalue_flag&2) {
            (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), code_subscript(pstate, &(yylsp[-2]), 3), code_push_string(pstate,&(yylsp[0]), (yyvsp[0].sstr))); 
            ((yyval.opcodes))->lvalue_flag = 2;
        } else {
            (yyval.opcodes) = codes_join((yyvsp[-2].opcodes), code_push_string(pstate,&(yylsp[0]), (yyvsp[0].sstr)));
            ((yyval.opcodes))->lvalue_flag = 6;
        }
    }
#line 4317 "src/parser.c"
    break;

  case 203: /* lvalue: lvalue '.' itemres  */
#line 800 "src/parser.y"
                         {
        if (((yyvsp[-2].opcodes))->lvalue_flag&2) {
            (yyval.opcodes) = codes_join3((yyvsp[-2].opcodes), code_subscript(pstate, &(yylsp[-2]), 3), (yyvsp[0].opcodes)); 
            ((yyval.opcodes))->lvalue_flag = 2;
        } else {
            (yyval.opcodes) = codes_join((yyvsp[-2].opcodes), (yyvsp[0].opcodes));
            ((yyval.opcodes))->lvalue_flag = 6;
        }
    }
#line 4331 "src/parser.c"
    break;

  case 204: /* exprlist_opt: %empty  */
#line 811 "src/parser.y"
                { (yyval.opcodes) = NULL; }
#line 4337 "src/parser.c"
    break;

  case 205: /* exprlist_opt: exprlist  */
#line 812 "src/parser.y"
                { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 4343 "src/parser.c"
    break;

  case 206: /* exprlist: expr  */
#line 816 "src/parser.y"
                        { (yyval.opcodes) = (yyvsp[0].opcodes); ((yyval.opcodes))->expr_counter = 1; }
#line 4349 "src/parser.c"
    break;

  case 207: /* exprlist: exprlist ',' expr  */
#line 817 "src/parser.y"
                                       { 
        int exprcnt = ((yyvsp[-2].opcodes))->expr_counter + 1;
        (yyval.opcodes) = codes_join((yyvsp[-2].opcodes), (yyvsp[0].opcodes));
        ((yyval.opcodes))->expr_counter = exprcnt;
    }
#line 4359 "src/parser.c"
    break;

  case 208: /* exprlist: exprlist ',' ','  */
#line 822 "src/parser.y"
                        { 
        int exprcnt = ((yyvsp[-2].opcodes))->expr_counter + 1;
        (yyval.opcodes) = codes_join((yyvsp[-2].opcodes), code_push_undef());
        ((yyval.opcodes))->expr_counter = exprcnt;
    }
#line 4369 "src/parser.c"
    break;

  case 209: /* exprlist: ',' exprlist  */
#line 827 "src/parser.y"
                    { 
        int exprcnt = ((yyvsp[0].opcodes))->expr_counter + 1;
        (yyval.opcodes) = codes_join(code_push_undef(), (yyvsp[0].opcodes));
        ((yyval.opcodes))->expr_counter = exprcnt;
    }
#line 4379 "src/parser.c"
    break;

  case 210: /* value: strlit  */
#line 835 "src/parser.y"
           { (yyval.opcodes) = code_push_vstring(pstate,&(yylsp[0]), (yyvsp[0].vstr)); }
#line 4385 "src/parser.c"
    break;

  case 211: /* value: TYPENULL  */
#line 836 "src/parser.y"
               { (yyval.opcodes) = code_push_null(); }
#line 4391 "src/parser.c"
    break;

  case 212: /* value: UNDEF  */
#line 837 "src/parser.y"
            { (yyval.opcodes) = code_push_undef(); }
#line 4397 "src/parser.c"
    break;

  case 213: /* value: _TRUE  */
#line 838 "src/parser.y"
            { (yyval.opcodes) = code_push_bool(1); }
#line 4403 "src/parser.c"
    break;

  case 214: /* value: _FALSE  */
#line 839 "src/parser.y"
             { (yyval.opcodes) = code_push_bool(0); }
#line 4409 "src/parser.c"
    break;

  case 215: /* value: FNUMBER  */
#line 840 "src/parser.y"
              { (yyval.opcodes) = code_push_num((yyvsp[0].num)); }
#line 4415 "src/parser.c"
    break;

  case 216: /* value: REGEXP  */
#line 841 "src/parser.y"
             { (yyval.opcodes) = code_push_regex(pstate, &(yylsp[0]), (yyvsp[0].regex)); }
#line 4421 "src/parser.c"
    break;

  case 217: /* value: object  */
#line 842 "src/parser.y"
             { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 4427 "src/parser.c"
    break;

  case 218: /* value: array  */
#line 843 "src/parser.y"
            { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 4433 "src/parser.c"
    break;

  case 219: /* object: '{' items '}'  */
#line 847 "src/parser.y"
                    { (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_object(pstate, &(yylsp[-1]), (yyvsp[-1].opcodes))); }
#line 4439 "src/parser.c"
    break;

  case 220: /* itemfunc: IDENTIFIER '(' args_opt ')' func_statement_block  */
#line 851 "src/parser.y"
                                                     {
        Jsi_OpCodes *lval = code_push_func(pstate, &(yylsp[-2]), jsi_FuncMake(pstate, (yyvsp[-2].scopes), (yyvsp[0].opcodes), &(yylsp[-4]), (yyvsp[-4].sstr), 0));
        lval->lvalue_flag = 1; 
        lval->lvalue_name = (yyvsp[-4].sstr);
        code_es6(pstate, "object shorthand func");
        (yyval.opcodes) = codes_join_item(code_push_string(pstate,&(yylsp[-4]), (yyvsp[-4].sstr)), lval);
        jsi_PstatePop(pstate);
    }
#line 4452 "src/parser.c"
    break;

  case 221: /* itemident: itemfunc  */
#line 862 "src/parser.y"
             { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 4458 "src/parser.c"
    break;

  case 222: /* itemident: IDENTIFIER  */
#line 863 "src/parser.y"
                  {
        code_es6(pstate, "object shorthands");
        Jsi_OpCodes *lval = code_push_index(pstate, &(yylsp[0]), (yyvsp[0].sstr), 0); 
        lval->lvalue_flag = 1; 
        lval->lvalue_name = (yyvsp[0].sstr); 
        (yyval.opcodes) = codes_join_item(code_push_string(pstate,&(yylsp[0]), (yyvsp[0].sstr)), lval);
    }
#line 4470 "src/parser.c"
    break;

  case 223: /* itemident: item  */
#line 870 "src/parser.y"
            { (yyval.opcodes) = (yyvsp[0].opcodes); }
#line 4476 "src/parser.c"
    break;

  case 224: /* items: %empty  */
#line 874 "src/parser.y"
    { (yyval.opcodes) = code_nop(); ((yyval.opcodes))->expr_counter = 0; }
#line 4482 "src/parser.c"
    break;

  case 225: /* items: item  */
#line 875 "src/parser.y"
            { (yyval.opcodes) = (yyvsp[0].opcodes); ((yyval.opcodes))->expr_counter = 1; }
#line 4488 "src/parser.c"
    break;

  case 226: /* items: items ',' itemident  */
#line 876 "src/parser.y"
                          {
        int cnt = ((yyvsp[-2].opcodes))->expr_counter + 1;
        (yyval.opcodes) = codes_join((yyvsp[-2].opcodes), (yyvsp[0].opcodes));
        ((yyval.opcodes))->expr_counter = cnt;
    }
#line 4498 "src/parser.c"
    break;

  case 227: /* items: items ','  */
#line 881 "src/parser.y"
                {
        (yyval.opcodes) = (yyvsp[-1].opcodes);
    }
#line 4506 "src/parser.c"
    break;

  case 228: /* itemres: OBJSET  */
#line 888 "src/parser.y"
           { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "set"); }
#line 4512 "src/parser.c"
    break;

  case 229: /* itemres: OBJGET  */
#line 889 "src/parser.y"
             { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "get"); }
#line 4518 "src/parser.c"
    break;

  case 230: /* itemres: CONTINUE  */
#line 890 "src/parser.y"
               { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "continue"); }
#line 4524 "src/parser.c"
    break;

  case 231: /* itemres: RETURN  */
#line 891 "src/parser.y"
             { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "return"); }
#line 4530 "src/parser.c"
    break;

  case 232: /* itemres: BREAK  */
#line 892 "src/parser.y"
            { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "break"); }
#line 4536 "src/parser.c"
    break;

  case 233: /* itemres: THROW  */
#line 893 "src/parser.y"
            { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "throw"); }
#line 4542 "src/parser.c"
    break;

  case 234: /* itemres: DEFAULT  */
#line 894 "src/parser.y"
              { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "default"); }
#line 4548 "src/parser.c"
    break;

  case 235: /* itemres: CASE  */
#line 895 "src/parser.y"
           { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "case"); }
#line 4554 "src/parser.c"
    break;

  case 236: /* itemres: EXPORT  */
#line 896 "src/parser.y"
             { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "export"); }
#line 4560 "src/parser.c"
    break;

  case 237: /* itemres: TYPEANY  */
#line 897 "src/parser.y"
              { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "any"); }
#line 4566 "src/parser.c"
    break;

  case 238: /* itemres: TYPEARRAY  */
#line 898 "src/parser.y"
                { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "array"); }
#line 4572 "src/parser.c"
    break;

  case 239: /* itemres: TYPESTRING  */
#line 899 "src/parser.y"
                 { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "string"); }
#line 4578 "src/parser.c"
    break;

  case 240: /* itemres: TYPENUMBER  */
#line 900 "src/parser.y"
                 { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "number"); }
#line 4584 "src/parser.c"
    break;

  case 241: /* itemres: TYPEBOOLEAN  */
#line 901 "src/parser.y"
                  { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "boolean"); }
#line 4590 "src/parser.c"
    break;

  case 242: /* itemres: TYPEREGEXP  */
#line 902 "src/parser.y"
                 { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "regexp"); }
#line 4596 "src/parser.c"
    break;

  case 243: /* itemres: TYPEITEROBJ  */
#line 903 "src/parser.y"
                  { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "iterobj"); }
#line 4602 "src/parser.c"
    break;

  case 244: /* itemres: TYPEUSEROBJ  */
#line 904 "src/parser.y"
                  { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "userobj"); }
#line 4608 "src/parser.c"
    break;

  case 245: /* itemres: ARGUMENTS  */
#line 905 "src/parser.y"
                { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "arguments"); }
#line 4614 "src/parser.c"
    break;

  case 246: /* itemres: _THIS  */
#line 906 "src/parser.y"
            { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "this"); }
#line 4620 "src/parser.c"
    break;

  case 247: /* itemres: DELETE  */
#line 907 "src/parser.y"
             { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "delete"); }
#line 4626 "src/parser.c"
    break;

  case 248: /* itemres: NEW  */
#line 908 "src/parser.y"
          { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "deletnewe"); }
#line 4632 "src/parser.c"
    break;

  case 249: /* itemres: LOCAL  */
#line 909 "src/parser.y"
            { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "var"); }
#line 4638 "src/parser.c"
    break;

  case 250: /* itemres: LOCALLET  */
#line 910 "src/parser.y"
               { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "let"); }
#line 4644 "src/parser.c"
    break;

  case 251: /* itemres: LOCALCONST  */
#line 911 "src/parser.y"
                 { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "const"); }
#line 4650 "src/parser.c"
    break;

  case 252: /* itemres: VOID  */
#line 912 "src/parser.y"
           { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "void"); }
#line 4656 "src/parser.c"
    break;

  case 253: /* itemres: __DEBUG  */
#line 913 "src/parser.y"
              { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "debugger"); }
#line 4662 "src/parser.c"
    break;

  case 254: /* itemres: TYPEOBJECT  */
#line 914 "src/parser.y"
                 { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "object"); }
#line 4668 "src/parser.c"
    break;

  case 255: /* itemres: WITH  */
#line 915 "src/parser.y"
           { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "with"); }
#line 4674 "src/parser.c"
    break;

  case 256: /* itemres: FUNC  */
#line 916 "src/parser.y"
           { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "function"); }
#line 4680 "src/parser.c"
    break;

  case 257: /* itemres: TYPEOF  */
#line 917 "src/parser.y"
             { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "typeof"); }
#line 4686 "src/parser.c"
    break;

  case 258: /* itemres: INSTANCEOF  */
#line 918 "src/parser.y"
                 { (yyval.opcodes) = code_push_string(pstate,&(yylsp[0]), "instanceof"); }
#line 4692 "src/parser.c"
    break;

  case 259: /* item: IDENTIFIER ':' expr  */
#line 922 "src/parser.y"
                        { (yyval.opcodes) = codes_join_item(code_push_string(pstate,&(yylsp[-2]), (yyvsp[-2].sstr)), (yyvsp[0].opcodes)); }
#line 4698 "src/parser.c"
    break;

  case 260: /* item: strlit ':' expr  */
#line 923 "src/parser.y"
                        { (yyval.opcodes) = codes_join_item(code_push_vstring(pstate,&(yylsp[-2]), (yyvsp[-2].vstr)), (yyvsp[0].opcodes)); }
#line 4704 "src/parser.c"
    break;

  case 261: /* item: itemres ':' expr  */
#line 924 "src/parser.y"
                        { (yyval.opcodes) = codes_join_item((yyvsp[-2].opcodes), (yyvsp[0].opcodes)); }
#line 4710 "src/parser.c"
    break;

  case 262: /* item: FNUMBER ':' expr  */
#line 925 "src/parser.y"
                       { (yyval.opcodes) = codes_join_item(code_push_num((yyvsp[-2].num)), (yyvsp[0].opcodes));  }
#line 4716 "src/parser.c"
    break;

  case 263: /* item: _TRUE ':' expr  */
#line 926 "src/parser.y"
                     { (yyval.opcodes) = codes_join_item(code_push_bool(1), (yyvsp[0].opcodes));  }
#line 4722 "src/parser.c"
    break;

  case 264: /* item: _FALSE ':' expr  */
#line 927 "src/parser.y"
                      { (yyval.opcodes) = codes_join_item(code_push_bool(0), (yyvsp[0].opcodes));  }
#line 4728 "src/parser.c"
    break;

  case 265: /* item: UNDEF ':' expr  */
#line 928 "src/parser.y"
                     { (yyval.opcodes) = codes_join_item(code_push_undef(), (yyvsp[0].opcodes));  }
#line 4734 "src/parser.c"
    break;

  case 266: /* item: TYPENULL ':' expr  */
#line 929 "src/parser.y"
                        { (yyval.opcodes) = codes_join_item(code_push_null(), (yyvsp[0].opcodes));  }
#line 4740 "src/parser.c"
    break;

  case 267: /* item: OBJSET IDENTIFIER '(' IDENTIFIER ')' func_statement_block  */
#line 930 "src/parser.y"
                                                                {
        Jsi_ScopeStrs *args = jsi_argInsert(pstate, NULL, (yyvsp[-2].sstr), NULL, &(yylsp[-2]), 0 );
        (yyval.opcodes) = codes_join_item(code_push_string(pstate,&(yylsp[-4]), (yyvsp[-4].sstr)), 
            code_push_func(pstate,  &(yylsp[-3]), jsi_FuncMake(pstate, args, (yyvsp[0].opcodes), &(yylsp[-5]), (yyvsp[-4].sstr), 2)));
    }
#line 4750 "src/parser.c"
    break;

  case 268: /* item: OBJGET IDENTIFIER '(' ')' func_statement_block  */
#line 935 "src/parser.y"
                                                     {
        Jsi_ScopeStrs *args = jsi_ArgsOptAdd(pstate, jsi_ScopeStrsNew());
        (yyval.opcodes) = codes_join_item(code_push_string(pstate,&(yylsp[-3]), (yyvsp[-3].sstr)), 
            code_push_func(pstate,  &(yylsp[-2]), jsi_FuncMake(pstate, args, (yyvsp[0].opcodes), &(yylsp[-4]), (yyvsp[-3].sstr), 4)));
        (yyval.opcodes)->codes[0].setget = 1;
    }
#line 4761 "src/parser.c"
    break;

  case 269: /* array: '[' exprlist ']'  */
#line 944 "src/parser.y"
                     { (yyval.opcodes) = codes_join((yyvsp[-1].opcodes), code_array(pstate, &(yylsp[-1]), ((yyvsp[-1].opcodes))->expr_counter)); }
#line 4767 "src/parser.c"
    break;

  case 270: /* array: '[' exprlist ',' ']'  */
#line 945 "src/parser.y"
                           { (yyval.opcodes) = codes_join((yyvsp[-2].opcodes), code_array(pstate, &(yylsp[-2]), ((yyvsp[-2].opcodes))->expr_counter)); }
#line 4773 "src/parser.c"
    break;

  case 271: /* array: '[' ']'  */
#line 946 "src/parser.y"
              { (yyval.opcodes) = code_array(pstate, &(yylsp[-1]), 0); }
#line 4779 "src/parser.c"
    break;


#line 4783 "src/parser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, pstate, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, pstate);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, pstate);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, pstate, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, pstate);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, pstate);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 950 "src/parser.y"


