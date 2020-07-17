#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>

typedef enum {
    EK_OK = 0,
    EK_LEXER,
    EK_PARSER,
    EK_INTERP
} CtErrorKind;

typedef struct {
    CtErrorKind kind;
} CtError;

typedef void*(*CtAllocFunc)(void*, size_t);
typedef void(*CtFreeFunc)(void*, void*);
typedef void(*CtErrorFunc)(void*, CtError);

typedef struct {
    void* data;

    CtAllocFunc malloc;
    CtFreeFunc free;
    CtErrorFunc error;
} CtCallbacks;

typedef enum {
    TK_IDENT,
    TK_STRING,
    TK_CHAR,
    TK_INT,
    TK_KEYWORD,
    TK_EOF,

    TK_INVALID = 0xFF
} CtTokenKind;

typedef struct {
    char* ident;

    CtString string;

    CtChar letter;

    CtNumber digit;

    CtKeyword key;
} CtTokenData;

typedef struct {
    CtTokenKind kind;
    CtTokenData data;
} CtToken;



#endif /* CTHULHU_H */
