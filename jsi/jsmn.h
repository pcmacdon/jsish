#ifndef __JSMN_H__
#define __JSMN_H__

#define JSMN_STATIC_DEFAULT 100

#define JSMN_DECLARE(p,token) \
    jsmn_parser p = {0}; \
    jsmntok_t tokens[JSMN_STATIC_DEFAULT]; \
    jsmn_init(&p, tokens, JSMN_STATIC_DEFAULT)

/**
 * JSON type identifier. Basic types are:
 *  o Object
 *  o Array
 *  o String
 *  o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
    JSMN_PRIMITIVE = 0,
    JSMN_OBJECT = 1,
    JSMN_ARRAY = 2,
    JSMN_STRING = 3,
    JSMN_INVALID=-1
} jsmntype_t;

typedef enum {
    /* Not enough tokens were provided */
    JSMN_ERROR_NOMEM = -1,
    /* Invalid character inside JSON string */
    JSMN_ERROR_INVAL = -2,
    /* The string is not a full JSON packet, more bytes expected */
    JSMN_ERROR_PART = -3,
    /* Everything was fine */
    JSMN_SUCCESS = 0
} jsmnerr_t;

/**
 * JSON token description.
 * @param       type    type (object, array, string etc.)
 * @param       start   start position in JSON data string
 * @param       end     end position in JSON data string
 */
typedef struct {
    jsmntype_t type;
    int start;
    int end;
    uint size;
#ifdef JSMN_PARENT_LINKS
    int parent;
#endif
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
    uint pos; /* offset in the JSON string */
    uint toknext;    /* next token to allocate */
    int toksuper;   /* superior token node, e.g parent object or array */
    jsmntok_t *tokens, *static_tokens;
    unsigned int num_tokens;
    int no_malloc;  /* Disallow parser to dynamically grow tokens array. */
    unsigned char strict;   /* Strict parsing. */
    unsigned char flags;
    const char *errStr;
} jsmn_parser;

/**
 * Setup JSON parser with a static array of tokens and size.
 * If static_tokens is NULL, malloc array of size num_tokens (0 means use default of 100).
 */
void jsmn_init(jsmn_parser *parser, jsmntok_t *static_tokens, unsigned int num_tokens);

/**
 * Reset parser before another call to parse()
 */
void jsmn_reset(jsmn_parser *parser);

/**
 * Free allocated memory after parse.
 */
void jsmn_free(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.  Called after jsmn_init().
 */
jsmnerr_t jsmn_parse(jsmn_parser *parser, const char *js);

/**
 * Helper functions.
 */
jsmntok_t *jsmn_token(jsmn_parser *parser, unsigned int index);
jsmntype_t jsmn_type(jsmn_parser *parser, unsigned int index);
int jsmn_toklen(jsmn_parser *parser, unsigned int index);
const char* jsmn_tokstr(jsmn_parser *parser, const char *js, unsigned int index, uint *len);
const char* jsmn_typename(int type);
const char* jsmn_errname(int code);
void jsmn_dump(jsmn_parser *parser, const char *js);

#endif /* __JSMN_H_ */
