#ifndef CTHULHU_H
#define CTHULHU_H

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

typedef struct CtState {
    const char *name;
    void *stream;
    CtNextFunc next;
    int ahead;

    CtOffset pos;
    size_t len;
    CtBuffer source;
    CtBuffer strings;

    enum {
#define FLAG(name, bit) name = (1 << bit),
#include "keys.inc"
        LF_DEFAULT = LF_CORE
    } flags;

    int depth;
} CtState;

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
    enum { BASE2, BASE10, BASE16 } enc;
    size_t num;
    CtView suffix;
} CtDigit;

typedef struct {
    enum {
        TK_IDENT,
        TK_KEY,
        TK_INT,
        TK_END,

        TK_LOOKAHEAD
    } type;

    union {
        CtView ident;
        CtKey key;
        CtDigit digit;
    } data;

    CtOffset pos;
} CtToken;

void ctStateNew(
    CtState *self,
    void *stream,
    CtNextFunc next,
    const char *name
);

#endif /* CTHULHU_H */
