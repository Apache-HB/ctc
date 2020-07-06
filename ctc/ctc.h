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

    void(*err)(CtStreamPos, char*);
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
    char* err;
} CtLexer;

CtLexer* ctLexOpen(CtStream* stream);

typedef struct {
    CtLexer* source;
    CtToken tok;

    char* err;
} CtParser;

CtParser* ctParseOpen(CtLexer* source);


typedef enum {
    AK_UNIT,
    AK_IMPORT,
    AK_IDENT,

    AK_ALIAS,

    AK_BUILTIN,
    AK_TYPENAME,
    AK_POINTER,
    AK_FUNCPTR
} CtASTKind;

typedef struct CtASTList {
    struct CtAST* item;
    struct CtASTList* next;
} CtASTList;

typedef struct {
    CtASTList* imports;
    CtASTList* body;
} CtASTUnit;

typedef struct {
    CtASTList* path;
    CtASTList* items;
} CtASTImport;

typedef struct {
    struct CtAST* name;
    struct CtAST* symbol;
} CtASTAlias;

typedef enum {
    BL_U8,
    BL_U16,
    BL_U32,
    BL_U64,
    BL_I8,
    BL_I16,
    BL_I32,
    BL_I64,
    BL_VOID
} CtBuiltin;

typedef struct {
    CtASTList* args;
    struct CtAST* result;
} CtFuncptr;

typedef union {
    CtASTUnit unit;
    CtASTImport import;
    CtASTAlias alias;
    CtBuiltin builtin;
    CtASTList* name;
    struct CtAST* ptr;
    CtFuncptr funcptr;
} CtASTData;

typedef struct CtAST {
    CtToken tok;
    CtASTKind kind;
    CtASTData data;
} CtAST;

/* parse a full compilation unit */
CtAST* ctParseUnit(CtParser* self);

#endif /* CTC_H */
