#ifndef CTC_H
#define CTC_H

#include <stddef.h>

typedef struct {
    /* source of the position */
    struct CtStream* parent;

    /**
     * absolute offset, line, and column
     * of token
     */
    int pos;
    int line;
    int col;
} CtStreamPos;

typedef struct {
    void*(*open)(const char*);
    int(*next)(void*);
} CtStreamCallbacks;

typedef struct CtStream {
    /**
     * function pointers for opening, closing,
     * and reading the next character from a stream
     */
    CtStreamCallbacks callbacks;

    /* file stream data */
    void* data;

    /* lookahead character */
    int ahead;

    /* name of the file */
    const char* name;

    /* current position */
    CtStreamPos pos;
} CtStream;

typedef enum {
    TK_IDENT,
    TK_STRING,
    TK_INT,
    TK_KEYWORD,
    TK_EOF,

    /* internal token kind */
    TK_INVALID
} CtTokenKind;

typedef unsigned long CtDigit;
typedef unsigned long CtChar;

typedef enum {
#define OP(id, str) id,
#define KEY(id, str) id,
#include "keys.inc"

    /* invalid keyword, should never be used */
    K_INVALID
} CtKeyword;

typedef struct {
    char* string;
    size_t len;
    char* prefix;
} CtString;

typedef struct {
    CtDigit digit;
    char* suffix;
} CtNumber;

typedef union {
    /* TK_IDENT */
    char* ident;

    /* TK_STRING */
    CtString string;

    /* TK_INT */
    CtNumber digit;

    /* TK_KEYWORD */
    CtKeyword key;

    /* TK_INVALID */
    char* reason;

    /* TK_EOF */
} CtTokenData;

typedef struct {
    CtTokenKind kind;
    CtTokenData data;
    CtStreamPos pos;
} CtToken;

CtStream* ctOpen(CtStreamCallbacks callbacks, const char* path);

typedef struct {
    CtStream* stream;
} CtLexer;

CtLexer* lexOpen(CtStream* stream);

typedef struct {
    CtLexer* source;
    CtToken tok;
} CtParser;

CtParser* parseOpen(CtLexer* source);

typedef struct {
    CtToken source;
} CtAST;

/* parse a full compilation unit */
CtAST* parseUnit(CtParser* parser);

/* parse a single item in a stream, for repl stuff */
CtAST* parseItem(CtParser* parser);

#endif /* CTC_H */
