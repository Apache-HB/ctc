#ifndef CTC_H
#define CTC_H

#include <stddef.h>

typedef struct {
    /* filename */
    const char* name;

    /**
     * absolute offset, line, and column
     * of token
     */
    int pos;
    int line;
    int col;
} CtStreamPos;

typedef struct {
    /* get the next character from a stream, return -1 on EOF */
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
    TK_CHAR,
    TK_INT,
    TK_KEYWORD,
    TK_ERROR,
    TK_EOF,

    /* internal token kind */
    TK_INVALID
} CtTokenKind;

typedef unsigned long long CtDigit;
typedef unsigned long long CtChar;

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

typedef enum {
    NE_INT,
    NE_HEX,
    NE_BIN
} CtNumberEncoding;

typedef struct {
    CtDigit digit;
    char* suffix;
    CtNumberEncoding enc;
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

    /* TK_CHAR */
    CtChar letter;

    /* TK_ERROR */
    char* reason;

    /* TK_EOF */
} CtTokenData;

typedef struct {
    CtTokenKind kind;
    CtTokenData data;
    CtStreamPos pos;
} CtToken;

/* cleanup a token */
void ctFreeToken(CtToken tok);

CtStream ctStreamOpen(CtStreamCallbacks callbacks, void* data, const char* path);

typedef struct {
    CtStream* stream;
    char* err;

    int depth;
} CtLexer;

/* open a lexing stream */
CtLexer ctLexOpen(CtStream* stream);

/* get a single token from the stream */
CtToken ctLexNext(CtLexer* self);

typedef enum {
    AK_IDENT
} CtASTKind;

typedef union {
    /* AK_IDENT */
    char* ident;
} CtASTData;

typedef struct CtAST {
    CtASTKind kind;
    CtASTData data;

    /* token related to this ast, used mainly for error reporting */
    CtToken tok;
} CtAST;

typedef struct {
    CtLexer* source;
    CtToken tok;

    /* an error message if there is one */
    char* err;
    /* the current node being parsed */
    CtAST* node;
} CtParser;

CtParser ctParseOpen(CtLexer* source);


#endif /* CTC_H */
