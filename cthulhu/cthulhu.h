#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>
#include <stdint.h>

typedef size_t CtSize;
typedef uint64_t CtInt;

typedef void*(*CtAllocFunc)(void*, CtSize);
typedef void(*CtDeallocFunc)(void*, void*);

typedef struct {
    void *data;
    CtAllocFunc alloc;
    CtDeallocFunc dealloc;
} CtAllocator;

typedef struct {
    CtSize dist;
    CtSize line;
    CtSize col;
} CtPosition;

typedef enum {
    TK_IDENT,
    TK_KEYWORD,
    TK_USER_KEYWORD,
    TK_STRING,
    TK_CHAR,
    TK_INT,
    TK_EOF,

    /* used internally by the parser */
    TK_LOOKAHEAD
} CtTokenKind;

typedef struct {
    /* hold the length in bytes because strings can have null bytes in them */
    CtSize len;
    char *str;
} CtString;

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

    /* TK_EOF, TK_LOOKAHEAD */
} CtTokenData;

typedef struct {
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
    void *data;
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

    /* allocator */
    CtAllocator alloc;
} CtLexer;

void ctSetKeys(CtLexer *self, const CtUserKeyword *keys, CtSize num);
void ctResetKeys(CtLexer *self);

CtLexer ctLexerNew(void *data, CtLexerNextFunc next, CtAllocator alloc);

CtToken ctLexerNext(CtLexer *self);

#endif /* CTHULHU_H */
