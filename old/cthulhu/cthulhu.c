#include "cthulhu.h"

#include <string.h>
#include <ctype.h>

#ifndef CT_MALLOC
#   error "CT_MALLOC must be defined"
#endif

#ifndef CT_FREE
#   error "CT_FREE must be defined"
#endif

#ifndef CT_REALLOC
#   error "CT_REALLOC must be defined"
#endif

#define CT_UNUSED(it) (void)it

static CtRange zeroRange() {
    CtRange self;

    self.offset = 0;
    self.line = 0;
    self.col = 0;
    self.len = 0;

    return self;
}

void ctPush(CtBuffer *self, char c) {
    if (self->len + 1 >= self->size) {
        self->size += 0x500;
        self->ptr = CT_REALLOC(self->ptr, self->size);
    }

    self->ptr[self->len++] = c;
}

CtBuffer ctBufferAlloc(size_t init) {
    CtBuffer self;

    self.ptr = CT_MALLOC(init);
    self.size = init;
    self.len = 0;

    return self;
}

size_t ctOffset(CtBuffer *self) {
    return self->len;
}

void ctBufferFree(CtBuffer self) {
    CT_FREE(self.ptr);
}

const char *ctAt(CtBuffer *self, size_t off) {
    return self->ptr + off;
}

void ctRewind(CtBuffer *self, size_t off) {
    self->len = off;
}

CtStream ctStreamAlloc(void *stream, char(*fun)(void*)) {
    CtStream self;

    self.stream = stream;
    self.get = fun;
    self.ahead = fun(stream);
    self.buffer = ctBufferAlloc(0x1000);
    self.where = zeroRange();

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

CtRange ctHere(CtStream *self) {
    return self->where;
}

static size_t sourceOffset(CtStream *self) {
    return self->where.offset;
}
