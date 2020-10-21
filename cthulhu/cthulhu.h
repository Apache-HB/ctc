#ifndef CTHULHU_H
#define CTHULHU_H

/* an error code */
typedef enum {
    /* no error */
    CT_OK = 0,

    /**
     *  invalid character in stream
     * such as ` or $ which are both currently
     * not in the grammar
     */
    CT_CHAR
} CtErr;

/* any error with extra data as needed */
typedef struct {
    CtErr err;
} CtError;

typedef struct {
    long dist;
    long line;
    int col;
} CtPos;

typedef struct {
    void *stream;
    int(*next)(void*);

    int c;

    CtPos pos;
} CtLex;

typedef enum {
    TK_INVALID,

    TK_END,
    TK_IDENT,
    TK_STRING,
    TK_KEY,
    TK_DIGIT
} CtTokenKind;

typedef struct {
    char *ptr;
    int len;
    int multiline;
} CtString;

typedef union {
    CtString str;
} CtTokenData;

typedef struct {
    CtTokenKind kind;
    CtTokenData data;
    CtPos pos;
} CtToken;

CtLex ctLexInit(void *stream, int(*next)(void*));

CtToken ctTok(CtLex *self);

#endif /* CTHULHU_H */



#if 0

#include <stdint.h>
#include <stddef.h>

typedef int(*CtNextFunc)(void*);

typedef struct {
    char *ptr;
    size_t len;
    size_t alloc;
} CtBuffer;

typedef struct {
    struct CtState *source;
    size_t dist;
    size_t col;
    size_t line;
} CtOffset;

typedef enum {
#define KEY(id, str, flags) id,
#define OP(id, str) id,
#include "keys.inc"

    K_INVALID
} CtKey;

typedef struct {
    size_t offset;
    size_t len;
} CtView;

typedef struct {
    size_t offset;
    size_t len;

    /* TODO: optimize this */
    int multiline;
} CtString;

typedef struct {
    enum { BASE2, BASE10, BASE16 } enc;
    size_t num;
    CtView suffix;
} CtDigit;

typedef struct {
    enum {
        TK_IDENT,
        TK_KEY,
        TK_INT,
        TK_STRING,
        TK_CHAR,
        TK_END,

        TK_LOOKAHEAD
    } type;

    union {
        CtView ident;
        CtString str;
        size_t letter;
        CtKey key;
        CtDigit digit;
    } data;

    CtOffset pos;
    size_t len;
} CtToken;

typedef enum {
    ERR_NONE = 0,

    /* non-fatal */

    /* integer literal was too large to fit into max size */
    ERR_OVERFLOW,

    /* invalid escape sequence in string/char literal */
    ERR_INVALID_ESCAPE,

    /* a linebreak was found inside a single line string */
    ERR_STRING_LINEBREAK,



    /* fatal */

    /* invalid character found while lexing */
    ERR_INVALID_SYMBOL,

    /* the EOF was found while lexing a string */
    ERR_STRING_EOF,

    /* the closing ' was missing while parsing a char literal */
    ERR_CHAR_CLOSING,

    /* an unexpected keyword was encountered while parsing */
    ERR_UNEXPECTED_KEY,

    /* missing closing ) */
    ERR_MISSING_BRACE
} CtErrorKind;

typedef struct {
    CtErrorKind type;
    CtOffset pos;
    size_t len;

    /* associated token */
    CtToken tok;
} CtError;


typedef enum {
    AK_BINARY,
    AK_UNARY,
    AK_LITERAL
} CtASTKind;

typedef struct CtAST {
    CtASTKind type;
    CtToken tok;

    union {
        struct {
            struct CtAST *lhs;
            struct CtAST *rhs;
        } binary;
        struct CtAST *expr;
    } data;
} CtAST;

typedef struct CtState {
    /* stream state */
    const char *name;
    void *stream;
    CtNextFunc next;
    int ahead;
    CtBuffer source;

    /* lexing state */
    CtOffset pos;
    size_t len;
    CtBuffer strings;

    enum {
#define FLAG(name, bit) name = (1 << bit),
#include "keys.inc"
        LF_DEFAULT = LF_CORE
    } flags;

    int depth;

    /* error handling state */
    CtError lerr;
    CtError perr;

    CtError *errs;
    size_t err_idx;
    size_t max_errs;

    /* parsing state */
    CtToken tok;
} CtState;

void ctStateNew(
    CtState *self,
    void *stream,
    CtNextFunc next,
    const char *name,
    size_t max_errs
);

#endif