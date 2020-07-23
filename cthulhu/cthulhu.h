#ifndef CTHULHU_H
#define CTHULHU_H

#include <stdint.h>
#include <stddef.h>

typedef size_t CtSize;

typedef int(*CtNextFunc)(void*);
typedef void*(CtAlloc)(void*, CtSize);
typedef void(CtDealloc)(void*, void*);

typedef struct {
    void *data;
    CtAlloc alloc;
    CtDealloc dealloc;
} CtState;

#endif /* CTHULHU_H */
