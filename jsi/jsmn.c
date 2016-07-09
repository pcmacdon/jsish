#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#include "jsmn.h"
#endif
#ifndef JSMN_FREE
#define JSMN_FREE(p) free(p)
#define JSMN_MALLOC(l) malloc(l)
#define JSMN_REALLOC(p,l) realloc(p,l)
#endif
static jsmnerr_t jsmn_error(jsmn_parser *parser, jsmnerr_t err, const char *str) {
    switch (err) {
        case JSMN_ERROR_NOMEM: str = "out of memory"; break;
        case JSMN_ERROR_PART: str = "expected more bytes"; break;
        default: break;
    }
    parser->errStr = str;
    return err;
}
/**
 * Allocates a fresh unused token from the token pull.
 */
static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser, int *indexPtr) {
    jsmntok_t *tok;
    uint index, oldsz;
    if (parser->toknext >= parser->num_tokens) {
        if (parser->no_malloc) return NULL;
        oldsz = parser->num_tokens;
        parser->num_tokens *= 2;
        if (parser->tokens != parser->static_tokens || parser->static_tokens == NULL)
            parser->tokens = (jsmntok_t*)JSMN_REALLOC(parser->tokens, sizeof(jsmntok_t)*parser->num_tokens);
        else {
            parser->tokens = (jsmntok_t*)JSMN_MALLOC(sizeof(jsmntok_t)*parser->num_tokens);
            memcpy(parser->tokens, parser->static_tokens, sizeof(jsmntok_t)*oldsz);
        }
    }
    index = parser->toknext++;
    tok = &parser->tokens[index];
    tok->start = tok->end = -1;
    tok->size = 0;
    if (indexPtr) *indexPtr = index;
#ifdef JSMN_PARENT_LINKS
    tok->parent = -1;
#endif
    return tok;
}

void jsmn_free(jsmn_parser* parser) {
    if (parser->tokens != parser->static_tokens && parser->tokens) 
        JSMN_FREE(parser->tokens);
    parser->tokens = NULL;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmntok_t *token, jsmntype_t type, 
                            int start, int end) {
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static jsmnerr_t jsmn_parse_primitive(jsmn_parser *parser, const char *js) {
    jsmntok_t *token;
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
            return jsmn_error(parser,JSMN_ERROR_INVAL, "invalid char");
        }
    }
    if (parser->strict) {
        /* In strict mode primitive must be followed by a comma/object/array */
        parser->pos = start;
        return jsmn_error(parser,JSMN_ERROR_PART, "expected comma/object/array");
    }

found:
    token = jsmn_alloc_token(parser, &index);
    if (token == NULL) {
        parser->pos = start;
        return jsmn_error(parser,JSMN_ERROR_NOMEM, "");
    }
    jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
    token->parent = parser->toksuper;
#endif
    parser->pos--;
    return JSMN_SUCCESS;
}

/**
 * Fills next token with JSON string.
 */
static jsmnerr_t jsmn_parse_string(jsmn_parser *parser, const char *js) {
    jsmntok_t *token;

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
                return jsmn_error(parser,JSMN_ERROR_NOMEM, "");
            }
            jsmn_fill_token(token, JSMN_STRING, start+1, parser->pos);
#ifdef JSMN_PARENT_LINKS
            token->parent = parser->toksuper;
#endif
            return JSMN_SUCCESS;
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
                        return jsmn_error(parser,JSMN_ERROR_INVAL,"expected X digit in \\uXXXX escape");
                    }
                    break;
                /* Unexpected symbol */
                default:
                    parser->pos = start;
                    return jsmn_error(parser,JSMN_ERROR_INVAL, "unexpected symbol");
            }
        }
    }
    parser->pos = start;
    return jsmn_error(parser,JSMN_ERROR_PART,"");
}

/**
 * Parse JSON string and fill tokens.
 */
jsmnerr_t jsmn_parse(jsmn_parser *parser, const char *js) {
    jsmnerr_t r;
    int i, index = -1;
    jsmntok_t *token = NULL;

    for (; js[parser->pos] != '\0'; parser->pos++) {
        char c;
        int match;
        jsmntype_t type;

        c = js[parser->pos];
        match = 0;
        switch (c) {
            case '{': case '[':
                token = jsmn_alloc_token(parser, &index);
                if (token == NULL)
                    return jsmn_error(parser,JSMN_ERROR_NOMEM,"");
                if (parser->toksuper != -1) {
                    parser->tokens[parser->toksuper].size++;
#ifdef JSMN_PARENT_LINKS
                    token->parent = parser->toksuper;
#endif
                }
                token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
                token->start = parser->pos;
                parser->toksuper = parser->toknext - 1;
                break;
            case '}': case ']':
                type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
                if (parser->toknext < 1) {
                    return jsmn_error(parser,JSMN_ERROR_INVAL,"expected more");
                }
                token = &tokens[parser->toknext - 1];
                for (;;) {
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return jsmn_error(parser,JSMN_ERROR_INVAL,"type mismatch");
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
                            return jsmn_error(parser,JSMN_ERROR_INVAL,"type mismatch");
                        }
                        parser->toksuper = -1;
                        token->end = parser->pos + 1;
                        break;
                    }
                }
                /* Error if unmatched closing bracket */
                if (i == -1)
                    return jsmn_error(parser,JSMN_ERROR_INVAL, "unmatched closing bracket");
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
                r = jsmn_parse_string(parser, js);
                if (r < 0) return r;
                if (index>=0) token = &parser->tokens[index];
                if (parser->toksuper != -1)
                    parser->tokens[parser->toksuper].size++;
                break;
            case ':' :
                if (parser->strict && token->type != JSMN_OBJECT)
                    if (parser->toksuper<0 || parser->tokens[parser->toksuper].type != JSMN_OBJECT)
                    return jsmn_error(parser,JSMN_ERROR_INVAL,"got ':' in a non-object");
            case ',': 
                if (parser->strict == 0 || token->type == JSMN_ARRAY)
                    break;
                if (token->type != JSMN_OBJECT)
                    return jsmn_error(parser,JSMN_ERROR_INVAL,"expected object");
            case '\t' : case '\r' : case '\n' : case ' ': 
                break;

            /* In strict mode primitives are: numbers and booleans */
            case '-': case '0': case '1' : case '2': case '3' : case '4':
            case '5': case '6': case '7' : case '8': case '9':
            case 't': case 'f': case 'n' :
                match = 1;
                
            default:
                if (parser->strict && !match)   /* Unexpected char in strict mode */
                    return jsmn_error(parser,JSMN_ERROR_INVAL, "unexpected char in strict mode");
                r = jsmn_parse_primitive(parser, js);
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
            return jsmn_error(parser,JSMN_ERROR_PART,"");
        }
    }

    return JSMN_SUCCESS;
}

/**
 * Initialize parser based over a given  buffer with an array of tokens 
 * available.
 */
void jsmn_init(jsmn_parser *parser, jsmntok_t *tokens, 
        unsigned int num_tokens) {
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
    parser->tokens = parser->static_tokens = tokens;
    if (tokens == NULL && num_tokens==0)
        num_tokens = 100;
    if (tokens == NULL)
        parser->tokens = (jsmntok_t*)malloc(sizeof(jsmntok_t)*num_tokens);
    parser->num_tokens = num_tokens;
    parser->no_malloc = 0;  /* Can set this to zero after call to suppress malloc. */
}

void jsmn_reset(jsmn_parser* parser) {
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}

/* Helper functions */

void jsmn_dump(jsmn_parser *parser, const char *js) {
    uint i;
    for (i=0; i<parser->toknext; i++)
        printf("TOK(%s): %.*s\n", jsmn_typename(parser->tokens[i].type),
            (parser->tokens[i].end-parser->tokens[i].start),
            js+parser->tokens[i].start);
}
jsmntok_t *jsmn_token(jsmn_parser *parser, unsigned int index) {
    if (index>=parser->toknext)
        return NULL;
    return parser->tokens+index;
}
jsmntype_t jsmn_type(jsmn_parser *parser, unsigned int index) {
    if (index>=parser->toknext)
        return JSMN_INVALID;
    return parser->tokens[index].type;
}
int jsmn_toklen(jsmn_parser *parser, unsigned int index) {
    if (index>=parser->toknext)
        return -1;
    return (parser->tokens[index].end-parser->tokens[index].start);
}

const char* jsmn_tokstr(jsmn_parser *parser, const char *js, uint index, uint *lenPtr) {
    int len;
    if (index>=parser->toknext)
        return NULL;
    len = (parser->tokens[index].end-parser->tokens[index].start);
    if (lenPtr)
        *lenPtr = len;
    return (js + parser->tokens[index].start);
}
const char* jsmn_typename(int type) {
    static const char *types[] = { "PRIMITIVE", "OBJECT", "ARRAY", "STRING", "<INVALID>" };
    if (type < JSMN_PRIMITIVE || type > JSMN_STRING)
        type = JSMN_STRING+1;
    return types[type];
}

const char* jsmn_errname(int code) {
    static const char *codes[] = { "SUCCESS", "NOMEM", "INVAL", "PART", "<BADCODE>" };
    code = -code;
    if (code < 0 || code > 3)
        code = 4;
    return codes[code];
}
