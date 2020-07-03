#ifndef CTC_H
#define CTC_H

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

typedef struct CtStream {
    /**
     * function pointers for opening, closing, 
     * and reading the next character from a stream
     */
    void*(*open)(const char*);
    void(*close)(void*);
    int(*next)(void*);

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

typedef unsigned long long CtDigit;

typedef enum {
#define OP(id, str) id,
#define KEY(id, str) id,
#include "keys.inc"

    /* invalid keyword, should never be used */
    K_INVALID
} CtKeyword;

typedef union {
    /* TK_IDENT */
    char* ident;

    /* TK_STRING */
    char* string;

    /* TK_INT */
    CtDigit digit;

    /* TK_KEYWORD */
    CtKeyword key;

    /* TK_EOF, TK_INVALID */
} CtTokenData;

typedef struct {
    CtTokenKind kind;
    CtTokenData data;
    CtStreamPos pos;
} CtToken;

typedef struct {
    CtStream stream;
} CtLexer;

typedef struct {
    CtLexer stream;
    CtToken tok;
} CtParser;

typedef struct {

} CtAST;

CtAST* ctParse(CtStream* stream);

#endif /* CTC_H */
