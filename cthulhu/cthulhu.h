#ifndef CTHULHU_H
#define CTHULHU_H

#include <stdint.h>

typedef uint32_t CtBool;
typedef size_t CtSize;

typedef void*(*CtAllocFunc)(void*, CtSize);
typedef void(*CtDeallocFunc)(void*, void*);

typedef struct {
    void *data;
    CtAllocFunc alloc;
    CtDeallocFunc dealloc;
} CtAllocator;

typedef struct {
    CtSize dist;
    CtSize len;
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

typedef struct {
    enum { DE_BASE2, DE_BASE10, DE_BASE16 } enc;
    CtSize num;
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
} CtToken;

typedef int(*CtStreamNextFunc)(void*);

typedef struct {
    void *data;
    CtStreamNextFunc next;

    CtPosition pos;
    int ahead;
} CtStream;

CtStream ctStreamNew();
void ctStreamDelete(CtStream* self);

typedef struct {
    const char* str;
    CtUserKey id;
} CtUserKeyword;

typedef struct {
    CtStream source;
    
    /**
     * user provided keywords.
     * this array *can* be modified while parsing
     * this is also a stack so it cannot become fragmented
     */
    CtUserKeyword* ukeys;
    CtSize nkeys;

    /* current template depth */
    int depth;
} CtLexer;

void ctPushKey(CtLexer* self, CtUserKeyword key);
void ctPopKey(CtLexer* self);

CtLexer ctLexerNew();
void ctLexerDelete(CtLexer* lex);

#endif /* CTHULHU_H */

#if 0

#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>

typedef void*(*CtAllocFunction)(void*, size_t);
typedef void(*CtDeallocFunction)(void*, void*);

typedef struct {
    void *data;

    CtAllocFunction alloc;
    CtDeallocFunction dealloc;
} CtAllocator;

typedef struct {
    CtAllocator* allocator;
} CtInstance;

CtInstance ctInstanceNew(CtAllocator *alloc);
void ctInstanceDelete(CtInstance *inst);

typedef enum {
    /* an identifier */
    TK_IDENT,

    /* a single or multiline string */
    TK_STRING,

    /* standard keyword */
    TK_KEYWORD,

    /* custom keyword defined for builtin blocks */
    TK_USER_KEYWORD,

    /* a single character */
    TK_CHAR,

    /* a base 2, 10, or 16 integer */
    TK_INT,

    /* the end of the stream */
    TK_EOF,

    /* an invalid character */
    TK_ERROR,

    /* internal value for easy parser lookahead */
    TK_LOOKAHEAD = 0xFF
} CtTokenKind;

typedef struct {
    /* the length of the string in bytes */
    size_t length;

    /* a string with possible null bytes inside */
    char *str;
} CtString;

typedef struct {
    /* will be either 2, 10, or 16 */
    int base;

    /* the actual number */
    size_t num;
} CtDigit;

typedef enum {
#define KEY(id, str) id,
#define OP(id, str) id,
#include "keys.inc"

    K_INVALID
} CtKeyword;

typedef size_t CtChar;
typedef size_t CtUserKey;

typedef struct {
    /* TK_IDENT */
    char *ident;

    /* TK_STRING */
    CtString str;

    /* TK_INT */
    CtDigit digit;

    /* TK_CHAR */
    CtChar letter;

    /* TK_KEYWORD */
    CtKeyword key;

    /* TK_USER_KEYWORD */
    CtUserKey ckey;
} CtTokenData;

typedef struct {
    size_t dist;
    size_t len;
    size_t col;
    size_t line;
} CtPosition;

typedef struct {
    CtTokenKind kind;
    CtTokenData data;
    CtPosition pos;
} CtToken;

typedef int(*CtStreamNextFunc)(void* data);

typedef struct {
    void *data;
    CtStreamNextFunc next;

    CtPosition pos;
} CtStream;

typedef struct {

} CtLexer;

typedef struct {

} CtParser;

typedef struct {

} CtInterp;

#endif /* CTHULHU_H */

#endif
