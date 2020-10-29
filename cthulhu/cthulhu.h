#ifndef CTHULHU_H
#define CTHULHU_H 

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* allocate and free global objects */
void ctInit();
void ctFree();

typedef struct {
    /* owns */
    char *ptr;
    size_t len;
    size_t size;
} CtBuffer;

CtBuffer ctBufferAlloc(size_t init);
void ctBufferFree(CtBuffer self);

void ctPush(CtBuffer *self, char c);
void ctAppend(CtBuffer *self, const char *str, size_t len);
size_t ctOffset(CtBuffer *self);
const char *ctAt(CtBuffer *self, size_t off);
void ctRewind(CtBuffer *self, size_t off);

typedef struct {
    size_t offset;
    size_t line;
    size_t col;
    size_t len;
} CtRange;

typedef struct {
    /* doesnt own */
    void *stream;
    char(*get)(void*);

    /* owns */
    char ahead;
    CtBuffer buffer;
    CtRange where;
} CtStream;

CtStream ctStreamAlloc(void *stream, char(*fun)(void*));
void ctStreamFree(CtStream self);

char ctNext(CtStream *self);
char ctPeek(CtStream *self);
bool ctEat(CtStream *self, char c);
CtRange ctHere(CtStream *self);

typedef enum {
    TK_INVALID, TK_END,
    TK_IDENT, TK_STRING,
    TK_INT, TK_KEY
} CtTokenKind;

typedef enum {
#define KEY(id, str) id,
#define OP(id, str) id,
#include "keys.h"
    K_INVALID
} CtKey;

typedef union {
    CtKey key;
    /* CtToken::where ident; */
} CtTokenData;

typedef struct {
    CtTokenKind kind;
    CtRange where;
    CtTokenData data;
} CtToken;

typedef struct {
    /* doesnt own */
    CtStream *source;

#ifdef CT_MM_FAST
    /* owns */
    CtBuffer buf;
#endif
} CtLex;

CtLex ctLexAlloc(CtStream *source);
void ctLexFree(CtLex self);
CtToken ctLexNext(CtLex *self);

#endif /* CTHULHU_H */
