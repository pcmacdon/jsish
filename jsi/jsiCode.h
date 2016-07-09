#ifndef __JSICODE_H__
#define __JSICODE_H__
#ifdef __WIN32
#include "win/regex.h"
#else
#include <regex.h>
#endif

/* stack change */
/* 0  nothing change */
/* +1 push */
/* -1 pop */
typedef enum {      /* SC   type of data    comment                             */
    OP_NOP,         /* 0 */
    OP_PUSHNUM,     /* +1   *double         number                              */
    OP_PUSHSTR,     /* +1   *char        string                              */
    OP_PUSHVAR,     /* +1   *FastVar        variable name                       */
    OP_PUSHUND,     /* +1   -               undefined                           */
    OP_PUSHBOO,     /* +1   int             bool                                */
    OP_PUSHFUN,     /* +1   *Jsi_Func           function                            */
    OP_PUSHREG,     /* +1   *regex_t        regex                               */
    OP_PUSHARG,     /* +1   -               push arguments(cur scope)           */
    OP_PUSHTHS,     /* +1   -               push this                           */
    OP_PUSHTOP,     /* +1   -               duplicate top                       */
    OP_PUSHTOP2,    /* +2   -               duplicate toq and top               */
    OP_UNREF,       /* 0    -               make top be right value             */
    OP_POP,         /* -n   int             pop n elements                      */
    OP_LOCAL,       /* 0    *char        add a var to current scope          */
    OP_NEG,         /* 0    -               make top = - top                    */
    OP_POS,         /* 0    -               make top = + top, (conv to number)  */
    OP_NOT,         /* 0    -               reserve top                         */
    OP_BNOT,        /* 0    -               bitwise not                         */
    OP_ADD,         /* -1   -               all math opr pop 2 elem from stack, */
    OP_SUB,         /* -1   -                calc and push back in to the stack */
    OP_MUL,         /* -1   -                                                   */
    OP_DIV,         /* -1   -                                                   */
    OP_MOD,         /* -1   -                                                   */
    OP_LESS,        /* -1   -               logical opr, same as math opr       */
    OP_GREATER,     /* -1   -                                                   */
    OP_LESSEQU,     /* -1   -                                                   */
    OP_GREATEREQU,  /* -1   -                                                   */
    OP_EQUAL,       /* -1   -                                                   */
    OP_NOTEQUAL,    /* -1   -                                                   */
    OP_STRICTEQU,   /* -1   -                                                   */
    OP_STRICTNEQ,   /* -1   -                                                   */
    OP_BAND,        /* -1   -               bitwise and                         */
    OP_BOR,         /* -1   -               bitwise or                          */
    OP_BXOR,        /* -1   -               bitwise xor                         */
    OP_SHF,         /* -1   int(right)      signed shift left or shift right    */
    OP_INSTANCEOF,  /* -1 */
    OP_ASSIGN,      /* -n   int             if n = 1, assign to lval,           */
                    /*                      n = 2, assign to object member      */
    OP_SUBSCRIPT,   /* -1   -               do subscript TOQ[TOP]               */
    OP_INC,         /* 0    int             data indicate prefix/postfix inc/dec                */
    OP_TYPEOF,      /* 0    obj                                                                 */
    OP_IN,          /* 0    obj                                                                 */
    OP_DEC,         /* 0    int                                                                 */
    OP_KEY,         /* +1   -               push an iter object that contain all key in top     */
    OP_NEXT,        /* -1   -               assign next key to top, make top be res of this opr */
    OP_JTRUE,       /* -1   int             jmp to offset if top is true,                       */
    OP_JFALSE,      /* -1   int             jmp to offset if top is false,                      */
    OP_JTRUE_NP,    /* 0    int             jtrue no pop version                                */
    OP_JFALSE_NP,   /* 0    int             jfalse no pop version                               */
    OP_JMP,         /* 0    int             jmp to offset                                       */
    OP_JMPPOP,      /* -n   *JmpPopInfo     jmp to offset with pop n                            */
    OP_FCALL,       /* -n+1 int             call func with n args, pop then, make ret to be top */
    OP_NEWFCALL,    /* -n+1 int             same as fcall, call as a constructor                */
    OP_RET,         /* -n   int             n = 0|1, return with arg                            */
    OP_DELETE,      /* -n   int             n = 1, delete var, n = 2, delete object member      */
    OP_CHTHIS,      /* 0,   -               make toq as new 'this'                              */
    OP_OBJECT,      /* -n*2+1   int         create object from stack, and push back in          */
    OP_ARRAY,       /* -n+1 int             create array object from stack, and push back in    */
    OP_EVAL,        /* -n+1 int             eval can not be assign to other var                 */
    OP_STRY,        /* 0    *TryInfo        push try statment poses Jsi_LogWarn to trylist             */
    OP_ETRY,        /* 0    -               end of try block, jmp to finally                    */
    OP_SCATCH,      /* 0    *char        create new scope, assign to current excption        */
    OP_ECATCH,      /* 0    -               jmp to finally                                      */
    OP_SFINAL,      /* 0    -               restore scope chain create by Scatch                */
    OP_EFINAL,      /* 0    -               end of finally, any unfinish code in catch, do it   */
    OP_THROW,       /* 0    -               make top be last exception, pop trylist till catched*/
    OP_WITH,        /* -1   -               make top be top of scopechain, add to trylist       */
    OP_EWITH,       /* 0    -               pop trylist                                         */
    OP_RESERVED,    /* 0    ReservedInfo*   reserved, be replaced by iterstat by jmp/jmppop     */
    OP_DEBUG,       /* 0    -               DEBUG OPCODE, output top                            */
    OP_LASTOP       /* 0    -               END OF OPCODE                                       */
} Eopcode;

#define RES_CONTINUE    1
#define RES_BREAK       2


typedef struct OpCode {
    Eopcode op;
    void *data;
    unsigned int line:24;
    unsigned char alloc;
    const char *fname;
} OpCode;

typedef struct OpCodes_ {
    OpCode *codes;
    int code_len;
    int code_size;
    
    int expr_counter;           /* context related expr count */
    int lvalue_flag;            /* left value count/flag */
    const char *lvalue_name; /* left value name */
#ifdef JSI_MEM_DEBUG
    Jsi_HashEntry *hPtr;
    int id;
#endif
} OpCodes;


typedef struct TryInfo {
    int trylen;
    int catchlen;
    int finallen;
} TryInfo;

typedef struct ReservedInfo {
    int type;
    const char *label;
    int topop;
} ReservedInfo;

typedef struct JmpPopInfo {
    int off;
    int topop;
} JmpPopInfo;

typedef struct YYLTYPE jsi_Pline;

//void jsi_codes_print(OpCodes *ops);
void jsi_code_decode(OpCode *op, int currentip, char *buf, int bsiz);

#endif /* __JSICODE_H__ */
