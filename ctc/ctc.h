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

    int depth;
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
    /* generic stuff */
    AK_ERROR,
    AK_IDENT,

    /* types */
    AK_PTR_TYPE,
    AK_REF_TYPE,
    AK_ARR_TYPE,
    AK_FUNC_TYPE,
    AK_QUAL_TYPE,
    AK_TYPE_ARG,
    AK_QUAL_TYPE_LIST,

    /* expressions */
    AK_COERCE,
    AK_UNARY,

    /* string, char, or int */
    AK_LITERAL,

    /* (expr) */
    AK_PAREN,

    /* declarations */
    AK_ALIAS
} CtASTKind;

typedef struct {
    struct CtAST* name;
    struct CtAST* symbol;
} CtASTAlias;

typedef struct {
    struct CtAST* type;
    struct CtAST* size;
} CtASTArrType;

typedef struct {
    CtASTList args;
    struct CtAST* result;
} CtASTFuncType;

typedef struct {
    struct CtAST* name;
    struct CtAST* type;
} CtASTTypeArg;

typedef struct {
    struct CtAST* name;
    CtASTList params;
} CtASTQualType;

typedef struct {
    struct CtAST* type;
    struct CtAST* expr;
} CtASTCoerce;

typedef union {
    /* AK_ERROR */
    char* reason;

    /* AK_ALIAS */
    CtASTAlias alias;

    /* AK_PTR_TYPE, AK_REF_TYPE */
    struct CtAST* to;

    /* AK_ARR_TYPE */
    CtASTArrType arr;

    /* AK_FUNC_TYPE */
    CtASTFuncType func_type;

    /* AK_TYPE_ARG */
    CtASTTypeArg type_arg;

    /* AK_QUAL_TYPE */
    CtASTQualType qual_type;

    /* AK_QUAL_TYPE_LIST */
    CtASTList types;

    /* AK_COERCE */
    CtASTCoerce coerce;

    /* AK_PAREN */
    /* AK_UNARY, the unary operator is stored in tok */
    struct CtAST* body;
} CtASTData;

typedef struct CtAST {
    /* AK_IDENT */
    CtToken tok;
    CtASTKind kind;
    CtASTData data;
} CtAST;

/* parse a single item using the `interp` rule in cthulhu.g4 */
CtAST* ctParseNext(CtParser* self);

/* parse a full compilation unit using the `unit` rule in cthulhu.g4 */
CtAST* ctParseUnit(CtParser* self);

/* cleanup a node */
void ctFreeAST(CtAST* node);

typedef struct {
    /* user provided data */
    void* data;
} CtInterpData;

typedef struct CtSymbols {
    /* all the symbols in the current scope */
    CtASTList symbols;
} CtSymbols;

/* interpreter */
typedef struct {
    /* all the symbols currently available */
    CtSymbols symbols;
    CtInterpData userdata;
} CtInterp;

CtInterp* ctInterpOpen(CtInterpData data);
void ctInterpEval(CtInterp* self, CtAST* ast);
void ctInterpClose(CtInterp* self);

#endif /* CTC_H */
