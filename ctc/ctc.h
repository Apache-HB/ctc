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

/**
 * a range of characters in a source stream
 */
typedef struct {
    struct CtStream* parent;

    int pos;
    int line;
    int col;
    int len;
} CtStreamRange;

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

CtStream* ctStreamOpen(CtStreamCallbacks callbacks, void* data, const char* path);
void ctStreamClose(CtStream* self);

typedef struct {
    char* ptr;
    size_t alloc;
    size_t len;
} CtBuffer;

typedef struct {
    CtStream* stream;
    char* err;
} CtLexer;

/* open a lexing stream */
CtLexer* ctLexOpen(CtStream* stream);

/* get a single token from the stream */
CtToken ctLexNext(CtLexer* self);

/* cleanup the lexer */
void ctLexClose(CtLexer* self);

typedef struct {
    CtLexer* source;
    CtToken tok;

    char* err;
} CtParser;

CtParser* ctParseOpen(CtLexer* source);

void ctParseClose(CtParser* self);

typedef struct {
    struct CtAST* items;
    size_t len;
    size_t alloc;
} CtASTList;

typedef enum {
    AK_UNIT,

    /* basic stuff */
    AK_ERROR,
    AK_IDENT,

    /* types */
    AK_NAME,
    AK_POINTER,
    AK_REFERENCE,
    AK_ARRAY,
    AK_CLOSURE,

    /* declarations */
    AK_ALIAS,
    AK_IMPORT,
} CtASTKind;

typedef struct {
    struct CtAST* name;
    /* can be AK_IDENT, AK_TYPE */
    struct CtAST* symbol;
} CtASTAlias;

typedef struct {
    CtASTList path;
    CtASTList items;
} CtASTImport;

typedef struct {
    struct CtAST* type;
    struct CtAST* size;
} CtASTArray;

typedef struct {
    CtASTList args;
    struct CtAST* result;
} CtASTClosure;

typedef union {
    /* AK_ERROR */
    char* reason;

    /* AK_NAME */
    CtASTList name;

    /* AK_POINTER */
    struct CtAST* ptr;

    /* AK_REFERENCE */
    struct CtAST* ref;

    /* AK_ARRAY */
    CtASTArray arr;

    /* AK_CLOSURE */
    CtASTClosure closure;

    /* AK_IMPORT */
    CtASTImport import;

    /* AK_ALIAS */
    CtASTAlias alias;
} CtASTData;

typedef struct CtAST {
    CtToken tok;
    CtASTKind kind;
    CtASTData data;
} CtAST;

/* parse a single item using the `interp` rule in cthulhu.g4 */
CtAST* ctParseNext(CtParser* self);

/* parse a full compilation unit using the `unit` rule in cthulhu.g4 */
CtAST* ctParseUnit(CtParser* self);

#endif /* CTC_H */
