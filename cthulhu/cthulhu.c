#include "cthulhu.h"

#include <string.h>

#ifndef CT_MALLOC
#   error "CT_MALLOC must be defined"
#endif

#ifndef CT_FREE
#   error "CT_FREE must be defined"
#endif

#ifndef CT_REALLOC
#   error "CT_REALLOC must be defined"
#endif

#if !defined(CT_MM_SMALL) && !defined(CT_MM_FAST)
#   error "either CT_MM_SMALL or CT_MM_FAST must be defined"
#elif defined(CT_MM_SMALL) && defined(CT_MM_FAST)
#   error "CT_MM_SMALL and CT_MM_FAST must not both be defined"
#endif

#if defined(CT_MM_SMALL) && !defined(CT_MM_STEP)
#   define CT_MM_STEP 0x1000
#endif

void ctPush(CtBuffer *self, char c) {
    if (self->len + 1 >= self->size) {
#if defined(CT_MM_SMALL)
        self->size += CT_MM_STEP;
#elif defined(CT_MM_FAST)
        self->size *= 2;
#endif
        self->ptr = CT_REALLOC(self->ptr, self->size);
    }

    self->ptr[self->len++] = c;
}

CtBuffer ctBufferAlloc(size_t init) {
    CtBuffer self = {
        .ptr = CT_MALLOC(init),
        .size = init,
        .len = 0
    };

    return self;
}

void ctBufferFree(CtBuffer self) {
    CT_FREE(self.ptr);
}

CtStream ctStreamAlloc(void *stream, char(*fun)(void*)) {
    CtStream self = {
        .stream = stream,
        .get = fun,
        .ahead = fun(stream),
        .buffer = ctBufferAlloc(0x1000),
        .where = { 0, 0, 0 }
    };

    return self;
}

void ctStreamFree(CtStream self) {
    ctBufferFree(self.buffer);
}

char ctNext(CtStream *self) {
    char c = self->ahead;
    self->ahead = self->get(self->stream);

    self->where.offset += 1;

    if (c == '\n') {
        self->where.line += 1;
        self->where.col = 0;
    } else {
        self->where.col += 1;
    }

    return c;
}

char ctPeek(CtStream *self) {
    return self->ahead;
}

bool ctEat(CtStream *self, char c) {
    if (ctPeek(self) == c) {
        ctNext(self);
        return true;
    }
    return false;
}

CtWhere ctHere(CtStream *self) {
    return self->where;
}

CtLex ctLexAlloc(CtStream *source) {
    CtLex lex = {
        .source = source
    };

    return lex;
}

void ctLexFree(CtLex self) {
    (void)self;
}