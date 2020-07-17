#include "cthulhu/cthulhu.h"

#include <stdlib.h>

static void* posixMalloc(void* stub, size_t size) { (void)stub; return malloc(size); }
static void posixFree(void* stub, void* ptr) { (void)stub; free(ptr); }

int main(int argc, const char **argv)
{
    CtAllocator alloc = {
        .data = NULL,
        .alloc = posixMalloc,
        .dealloc = posixFree
    };

    CtInstance inst = ctInstanceNew(&alloc);
    CtStream stream = ctStreamNew();



    ctStreamDelete(&stream);
}
