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

CtLexer* ctLexOpen(CtStream* stream);

typedef struct {
    CtLexer* source;
    CtToken tok;

    struct CtASTList* attribs;
    struct CtASTList* tail;
} CtParser;

CtParser* ctParseOpen(CtLexer* source);


typedef enum {
    AK_UNIT,
    AK_IMPORT,
    AK_IDENT,
    AK_ATTRIB,
    AK_STRUCT,
    AK_FIELD,

    AK_PTRTYPE,
    AK_ARRTYPE,
    AK_FUNCTYPE,
    AK_NAMETYPE,
    AK_BUILTINTYPE
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
    CtASTList* path;
} CtASTAttrib;

typedef struct {
    struct CtAST* name;
    struct CtAST* type;
} CtASTField;

typedef struct {
    CtASTList* attribs;
    struct CtAST* name;
    CtASTList* fields;
} CtASTStruct;


typedef struct {
    struct CtAST* type;
} CtASTPtrType;

typedef struct {
    struct CtAST* type;
    struct CtAST* size;
} CtASTArrType;

typedef struct {
    CtASTList* args;
    struct CtAST* ret;
} CtASTFuncType;

typedef struct {
    CtASTList* path;
} CtASTNameType;

typedef enum {
    BT_I8,
    BT_I16,
    BT_I32,
    BT_I64,
    BT_VOID
} CtASTBuiltinType;

typedef union {
    CtASTUnit unit;
    CtASTImport import;
    CtASTAttrib attrib;
    CtASTStruct struc;
    CtASTField field;

    CtASTPtrType ptr;
    CtASTArrType arr;
    CtASTFuncType funcptr;
    CtASTNameType nametype;
    CtASTBuiltinType builtin;
} CtASTData;

typedef struct CtAST {
    CtToken tok;
    CtASTKind kind;
    CtASTData data;
} CtAST;

/* parse a full compilation unit */
CtAST* ctParseUnit(CtParser* self);

#endif /* CTC_H */
