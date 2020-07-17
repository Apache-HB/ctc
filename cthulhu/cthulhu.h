#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>

typedef void*(*CtAllocFunction)(void*, size_t);
typedef void(*CtFreeFunction)(void*, void*);

typedef struct {
    void* data;

    CtAllocFunction alloc;
    CtFreeFunction free;
} CtAllocator;

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
    char* str;
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
    char* ident;

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

#endif /* CTHULHU_H */
