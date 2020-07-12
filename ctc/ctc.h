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
    /* a single identifier */
    AK_IDENT,

    /* an array of asts */
    AK_ARRAY,

    /* types */

    /* AK_QUAL_TYPES | AK_PTR | AK_REF | AK_ARR | AK_FUNC_TYPE */
    /* AK_TYPE */

    /* AK_IDENT ('<' AK_TYPE_ARGS '>')? */
    AK_QUAL,

    /* '*' AK_TYPE */
    AK_PTR,

    /* '&' AK_TYPE */
    AK_REF,

    /* '[' AK_TYPE (':' AK_EXPR)? ']' */
    AK_ARR,

    /* 'def' AK_TYPES? ('->' AK_TYPE)? */
    AK_FUNC_TYPE,

    /* AK_TYPE | ':' AK_IDENT '=' AK_TYPE */
    AK_TYPE_ARG,

    /* a flat array of types */
    /* AK_TYPE_ARG (',' AK_TYPE_ARG)* */
    AK_TYPE_ARGS,

    /* AK_QUAL ('::' AK_QUAL)* */
    AK_QUAL_TYPES,

    /* toplevel declarations */

    /* AK_PATH AK_PATH? */
    AK_IMPORT,

    AK_UNIT
} CtASTKind;

typedef struct {
    struct CtAST* data;
    size_t alloc;
    size_t len;
} CtASTArray;

typedef struct {
    /* AK_IDENT */
    struct CtAST* name;

    /* AK_TYPE_ARGS? */
    struct CtAST* params;
} CtASTQualType;

typedef struct {
    /* AK_TYPE */
    struct CtAST* type;

    /* AK_EXPR? */
    struct CtAST* size;
} CtASTTypeArray;

typedef struct {
    /* AK_TYPES? */
    struct CtAST* args;

    /* AK_TYPE? */
    struct CtAST* result;
} CtASTFuncType;

typedef struct {
    /* AK_IDENT? */
    struct CtAST* name;

    /* AK_TYPE */
    struct CtAST* type;
} CtASTTypeArg;

typedef struct {
    /* AK_ARRAY<AK_IDENT> */
    struct CtAST* path;

    /* AK_ARRAY<AK_IDENT> */
    struct CtAST* items;
} CtASTImport;

typedef struct {
    /* AK_ARRAY<AK_IMPORT> */
    struct CtAST* imports;

    /* AK_ARRAY<AK_ALIAS | AK_STRUCT | AK_UNION | AK_FUNCTION> */
    struct CtAST* body;
} CtASTUnit;

typedef union {
    /* AK_IDENT */
    char* ident;

    /* AK_ARRAY */
    CtASTArray array;

    /* AK_QUAL */
    CtASTQualType qual_type;

    /* AK_PTR */
    struct CtAST* ptr;

    /* AK_REF */
    struct CtAST* ref;

    /* AK_ARR */
    CtASTTypeArray arr;

    /* AK_FUNC_TYPE */
    CtASTFuncType func_type;

    /* AK_TYPE_ARG */
    CtASTTypeArg type_arg;

    /* AK_TYPE_ARGS */
    CtASTArray type_args;

    /* AK_QUAL_TYPES */
    CtASTArray qual_types;

    /* AK_IMPORT */
    CtASTImport include;

    /* AK_UNIT */
    CtASTUnit unit;
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
