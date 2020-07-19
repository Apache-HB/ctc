#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>
#include <stdint.h>

typedef size_t CtSize;
typedef uint64_t CtInt;

typedef void*(*CtAllocFunc)(void*, CtSize);
typedef void(*CtDeallocFunc)(void*, void*);

typedef struct {
    CtSize dist;
    CtSize line;
    CtSize col;
} CtPosition;


typedef enum {
    /* no error */
    ERR_NONE,
    /* integer overflow */
    ERR_OVERFLOW,
    /* bad escape sequence in string/char literal */
    ERR_INVALID_ESCAPE,

    /* EOF inside a string/char literal */
    ERR_EOF,

    /* newline inside a single line string/char literal */
    ERR_NEWLINE,

    /* missing trailing ' in a char literal */
    ERR_UNTERMINATED,

    /* invalid symbol while lexing */
    ERR_INVALID_SYMBOL,

    /* unexpected token encountered when parsing */
    ERR_UNEXPECTED
} CtError;

typedef enum {
    TK_IDENT,
    TK_KEYWORD,
    TK_USER_KEYWORD,
    TK_STRING,
    TK_CHAR,
    TK_INT,
    TK_EOF,
    TK_ERROR,

    /* used internally by the parser */
    TK_LOOKAHEAD
} CtTokenKind;

#define CT_MULTILINE_FLAG (1ULL << 63)

typedef struct {
    /* hold the length in bytes because strings can have null bytes in them */
    CtSize len;
    char *str;
} CtString;

#define CT_STRLEN(str) (str.len & ~CT_MULTILINE_FLAG)

typedef enum {
    DE_BASE2, DE_BASE10, DE_BASE16
} CtDigitEncoding;

typedef struct {
    CtDigitEncoding enc;
    CtInt num;
    char *suffix;
} CtDigit;

typedef CtSize CtUserKey;

typedef enum {
#define KEY(id, str) id,
#define OP(id, str) id,
#include "keys.inc"

    K_INVALID
} CtKey;

typedef union {
    /* TK_IDENT */
    char *ident;

    /* TK_KEYWORD */
    CtKey key;

    /* TK_USER_KEYWORD */
    CtUserKey ukey;

    /* TK_STRING */
    CtString str;

    /* TK_CHAR */
    CtSize letter;

    /* TK_INT */
    CtDigit num;

    /* TK_ERROR */
    CtError err;

    /* TK_EOF, TK_LOOKAHEAD */
} CtTokenData;

typedef struct CtToken {
    CtTokenKind kind;
    CtTokenData data;
    CtPosition pos;

    /* length of the token for debugging */
    CtSize len;
} CtToken;

typedef int(*CtLexerNextFunc)(void*);

typedef struct {
    const char *str;
    CtUserKey id;
} CtUserKeyword;

typedef struct {
    /* stream state */
    void *stream;
    CtLexerNextFunc next;
    int ahead;

    CtPosition pos;
    CtSize len;

    /**
     * user provided keywords.
     */
    const CtUserKeyword *ukeys;
    CtSize nkeys;

    /* current template depth */
    int depth;

    CtError err;
} CtLexer;

void ctSetKeys(CtLexer *self, const CtUserKeyword *keys, CtSize num);
void ctResetKeys(CtLexer *self);

CtLexer ctLexerNew(void *stream, CtLexerNextFunc next);

CtToken ctLexerNext(CtLexer *self);
void ctFreeToken(CtToken tok);

typedef struct {
    CtLexer lex;

    /* lookahead token */
    CtToken tok;

    CtError err;
} CtParser;

typedef enum {
    NT_IDENT,
    NT_ERROR,

    NT_LITERAL,
    NT_UNARY,
    NT_BINARY,
    NT_TERNARY
} CtNodeType;

typedef struct {
    CtKey op;
    struct CtNode *expr;
} CtUnary;

typedef struct {
    CtKey op;
    struct CtNode *lhs;
    struct CtNode *rhs;
} CtBinary;

typedef struct {
    /* cond ? yes : no; */
    /* cond ?: no; yes = cond */
    struct CtNode *cond;
    struct CtNode *yes;
    struct CtNode *no;
} CtTernary;

typedef union {
    /* NT_IDENT */
    char *ident;

    /* NT_ERROR */
    CtError err;

    /* NT_LITERAL */
    CtToken literal;

    /* NT_UNARY */
    CtUnary unary;

    /* NT_BINARY */
    CtBinary binary;

    /* NT_TERNARY */
    CtTernary ternary;
} CtNodeData;

typedef struct CtNode {
    CtNodeType type;
    CtToken tok;
    CtNodeData data;
} CtNode;

CtParser ctParserNew(CtLexer lex);
CtNode *ctParseUnit(CtParser *self);
CtNode *ctParseEval(CtParser *self);

#endif /* CTHULHU_H */
