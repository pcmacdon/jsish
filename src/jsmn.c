#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#ifndef JSMN_FREE
#define JSMN_FREE(p) free(p)
#define JSMN_MALLOC(l) malloc(l)
#define JSMN_REALLOC(p,l) realloc(p,l)
#endif
static Jsi_JsonErrEnum jsmn_error(Jsi_JsonParser *parser, Jsi_JsonErrEnum err, const char *str) {
    switch (err) {
        case JSI_JSON_ERR_NOMEM: str = "out of memory"; break;
        case JSI_JSON_ERR_PART: str = "expected more bytes"; break;
        default: break;
    }
    parser->errStr = str;
    return err;
}
/**
 * Allocates a fresh unused token from the token pull.
 */
static Jsi_JsonTok *jsmn_alloc_token(Jsi_JsonParser *parser, int *indexPtr) {
    Jsi_JsonTok *tok;
    uint index, oldsz;
    if (parser->toknext >= parser->num_tokens) {
        if (parser->no_malloc) return NULL;
        oldsz = parser->num_tokens;
        parser->num_tokens *= 2;
        if (parser->tokens != parser->static_tokens || parser->static_tokens == NULL)
            parser->tokens = (Jsi_JsonTok*)JSMN_REALLOC(parser->tokens, sizeof(Jsi_JsonTok)*parser->num_tokens);
        else {
            parser->tokens = (Jsi_JsonTok*)JSMN_MALLOC(sizeof(Jsi_JsonTok)*parser->num_tokens);
            memcpy(parser->tokens, parser->static_tokens, sizeof(Jsi_JsonTok)*oldsz);
        }
    }
    index = parser->toknext++;
    tok = &parser->tokens[index];
    tok->start = tok->end = -1;
    tok->size = 0;
    if (indexPtr) *indexPtr = index;
    tok->parent = -1;
    return tok;
}

void Jsi_JsonFree(Jsi_JsonParser* parser) {
    if (parser->tokens != parser->static_tokens && parser->tokens) 
        JSMN_FREE(parser->tokens);
    parser->tokens = NULL;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(Jsi_JsonTok *token, Jsi_JsonTypeEnum type, 
                            int start, int end) {
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static Jsi_JsonErrEnum Jsi_JsonParse_primitive(Jsi_JsonParser *parser, const char *js) {
    Jsi_JsonTok *token;
    int start, index;

    start = parser->pos;

    for (; js[parser->pos] != '\0'; parser->pos++) {
        switch (js[parser->pos]) {
            /* In strict mode primitive must be followed by "," or "}" or "]" */
            case ':':
                if (parser->strict) break;
            case '\t' : case '\r' : case '\n' : case ' ' :
            case ','  : case ']'  : case '}' :
                goto found;
        }
        if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
            parser->pos = start;
            return jsmn_error(parser,JSI_JSON_ERR_INVAL, "invalid char");
        }
    }
    if (parser->strict) {
        /* In strict mode primitive must be followed by a comma/object/array */
        parser->pos = start;
        return jsmn_error(parser,JSI_JSON_ERR_PART, "expected comma/object/array");
    }

found:
    token = jsmn_alloc_token(parser, &index);
    if (token == NULL) {
        parser->pos = start;
        return jsmn_error(parser,JSI_JSON_ERR_NOMEM, "");
    }
    jsmn_fill_token(token, JSI_JTYPE_PRIMITIVE, start, parser->pos);
    token->parent = parser->toksuper;
    parser->pos--;
    return JSI_JSON_ERR_NONE;
}

/**
 * Fills next token with JSON string.
 */
static Jsi_JsonErrEnum Jsi_JsonParse_string(Jsi_JsonParser *parser, const char *js) {
    Jsi_JsonTok *token;

    int index, start = parser->pos;

    parser->pos++;

    /* Skip starting quote */
    for (; js[parser->pos] != '\0'; parser->pos++) {
        char c = js[parser->pos];

        /* Quote: end of string */
        if (c == '\"') {
            token = jsmn_alloc_token(parser, &index);
            if (token == NULL) {
                parser->pos = start;
                return jsmn_error(parser,JSI_JSON_ERR_NOMEM, "");
            }
            jsmn_fill_token(token, JSI_JTYPE_STRING, start+1, parser->pos);
            token->parent = parser->toksuper;
            return JSI_JSON_ERR_NONE;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\') {
            parser->pos++;
            switch (js[parser->pos]) {
                /* Allowed escaped symbols */
                case '\"': case '/' : case '\\' : case 'b' :
                case 'f' : case 'r' : case 'n'  : case 't' :
                    break;
                /* Allows escaped symbol \uXXXX */
                case 'u':
                    if (!(isxdigit(js[parser->pos+1]) && isxdigit(js[parser->pos+2]) &&
                    isxdigit(js[parser->pos+3]) && isxdigit(js[parser->pos+4]))) {
                        parser->pos = start;
                        return jsmn_error(parser,JSI_JSON_ERR_INVAL,"expected X digit in \\uXXXX escape");
                    }
                    break;
                /* Unexpected symbol */
                default:
                    parser->pos = start;
                    return jsmn_error(parser,JSI_JSON_ERR_INVAL, "unexpected symbol");
            }
        }
    }
    parser->pos = start;
    return jsmn_error(parser,JSI_JSON_ERR_PART,"");
}

/**
 * Parse JSON string and fill tokens.
 */
Jsi_JsonErrEnum Jsi_JsonParse(Jsi_JsonParser *parser, const char *js) {
    Jsi_JsonErrEnum r;
    int i, index = -1;
    Jsi_JsonTok *token = NULL;

    for (; js[parser->pos] != '\0'; parser->pos++) {
        char c;
        int match;
        Jsi_JsonTypeEnum type;

        c = js[parser->pos];
        match = 0;
        switch (c) {
            case '{': case '[':
                token = jsmn_alloc_token(parser, &index);
                if (token == NULL)
                    return jsmn_error(parser,JSI_JSON_ERR_NOMEM,"");
                if (parser->toksuper != -1) {
                    parser->tokens[parser->toksuper].size++;
                    token->parent = parser->toksuper;
                }
                token->type = (c == '{' ? JSI_JTYPE_OBJECT : JSI_JTYPE_ARRAY);
                token->start = parser->pos;
                parser->toksuper = parser->toknext - 1;
                break;
            case '}': case ']':
                type = (c == '}' ? JSI_JTYPE_OBJECT : JSI_JTYPE_ARRAY);
#ifdef JSMN_PARENT_LINKS
                if (parser->toknext < 1) {
                    return jsmn_error(parser,JSI_JSON_ERR_INVAL,"expected more");
                }
                token = &tokens[parser->toknext - 1];
                for (;;) {
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return jsmn_error(parser,JSI_JSON_ERR_INVAL,"type mismatch");
                        }
                        token->end = parser->pos + 1;
                        parser->toksuper = token->parent;
                        break;
                    }
                    if (token->parent == -1) {
                        break;
                    }
                    token = &parser->tokens[token->parent];
                }
#else
                for (i = parser->toknext - 1; i >= 0; i--) {
                    token = &parser->tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return jsmn_error(parser,JSI_JSON_ERR_INVAL,"type mismatch");
                        }
                        parser->toksuper = -1;
                        token->end = parser->pos + 1;
                        break;
                    }
                }
                /* Error if unmatched closing bracket */
                if (i == -1)
                    return jsmn_error(parser,JSI_JSON_ERR_INVAL, "unmatched closing bracket");
                for (; i >= 0; i--) {
                    token = &parser->tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        parser->toksuper = i;
                        break;
                    }
                }
#endif
                break;
            case '\"':
                r = Jsi_JsonParse_string(parser, js);
                if (r < 0) return r;
                if (index>=0) token = &parser->tokens[index];
                if (parser->toksuper != -1)
                    parser->tokens[parser->toksuper].size++;
                else if (js[0] == '\"')
                    parser->tokens[0].size++;
                break;
            case ':' :
                if (!token)
                    return jsmn_error(parser,JSI_JSON_ERR_INVAL,"null token");
                if (parser->strict && token->type != JSI_JTYPE_OBJECT)
                    if (parser->toksuper<0 || parser->tokens[parser->toksuper].type != JSI_JTYPE_OBJECT)
                        return jsmn_error(parser,JSI_JSON_ERR_INVAL,"got ':' in a non-object");
            case ',': 
                if (parser->strict == 0 || (token && token->type == JSI_JTYPE_ARRAY))
                    break;
                if (!token || token->type != JSI_JTYPE_OBJECT)
                    return jsmn_error(parser,JSI_JSON_ERR_INVAL,"expected object");
            case '\t' : case '\r' : case '\n' : case ' ': 
                break;

            /* In strict mode primitives are: numbers and booleans */
            case '-': case '0': case '1' : case '2': case '3' : case '4':
            case '5': case '6': case '7' : case '8': case '9':
            case 't': case 'f': case 'n' :
                match = 1;
                
            default:
                if (parser->strict && !match)   /* Unexpected char in strict mode */
                    return jsmn_error(parser,JSI_JSON_ERR_INVAL, "unexpected char in strict mode");
                r = Jsi_JsonParse_primitive(parser, js);
                if (r < 0) return r;
                if (index>=0) token = &parser->tokens[index];
                if (parser->toksuper != -1)
                    parser->tokens[parser->toksuper].size++;
                break;

        }
    }

    for (i = parser->toknext - 1; i >= 0; i--) {
        /* Unmatched opened object or array */
        if (parser->tokens[i].start != -1 && parser->tokens[i].end == -1) {
            return jsmn_error(parser,JSI_JSON_ERR_PART,"");
        }
    }

    return JSI_JSON_ERR_NONE;
}

/**
 * Initialize parser based over a given  buffer with an array of tokens 
 * available.
 */
void Jsi_JsonInit(Jsi_JsonParser *parser, Jsi_JsonTok *tokens, 
        unsigned int num_tokens) {
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
    parser->tokens = parser->static_tokens = tokens;
    if (tokens == NULL && num_tokens==0)
        num_tokens = 100;
    if (tokens == NULL)
        parser->tokens = (Jsi_JsonTok*)malloc(sizeof(Jsi_JsonTok)*num_tokens);
    parser->num_tokens = num_tokens;
    parser->no_malloc = 0;  /* Can set this to zero after call to suppress malloc. */
}

void Jsi_JsonReset(Jsi_JsonParser* parser) {
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}

/* Helper functions */

void Jsi_JsonDump(Jsi_JsonParser *parser, const char *js) {
    uint i;
    for (i=0; i<parser->toknext; i++)
        fprintf(stderr, "TOK(%s): %.*s\n", Jsi_JsonGetTypename(parser->tokens[i].type),
            (parser->tokens[i].end-parser->tokens[i].start),
            js+parser->tokens[i].start);
}
Jsi_JsonTok *Jsi_JsonGetToken(Jsi_JsonParser *parser, unsigned int index) {
    if (index>=parser->toknext)
        return NULL;
    return parser->tokens+index;
}

Jsi_JsonTypeEnum Jsi_JsonGetType(Jsi_JsonParser *parser, unsigned int index) {
    if (index>=parser->toknext)
        return JSI_JTYPE_INVALID;
    return parser->tokens[index].type;
}

int Jsi_JsonTokLen(Jsi_JsonParser *parser, unsigned int index) {
    if (index>=parser->toknext)
        return -1;
    return (parser->tokens[index].end-parser->tokens[index].start);
}

const char* Jsi_JsonGetTokstr(Jsi_JsonParser *parser, const char *js, uint index, uint *lenPtr) {
    int len;
    if (index>=parser->toknext)
        return NULL;
    len = (parser->tokens[index].end-parser->tokens[index].start);
    if (lenPtr)
        *lenPtr = len;
    return (js + parser->tokens[index].start);
}

const char* Jsi_JsonGetTypename(int type) {
    static const char *types[] = { "JSI_JTYPE_PRIMITIVE", "OBJECT", "ARRAY", "STRING", "<INVALID>" };
    if (type < JSI_JTYPE_PRIMITIVE || type > JSI_JTYPE_STRING)
        type = JSI_JTYPE_STRING+1;
    return types[type];
}

const char* Jsi_JsonGetErrname(int code) {
    static const char *codes[] = { "SUCCESS", "NOMEM", "INVAL", "PART", "<BADCODE>" };
    code = -code;
    if (code < 0 || code > 3)
        code = 4;
    return codes[code];
}
