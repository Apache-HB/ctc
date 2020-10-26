#include <stdlib.h>

#define CT_MALLOC malloc
#define CT_FREE free
#define CT_REALLOC realloc

#define CT_MM_SMALL

#include "cthulhu/cthulhu.c"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    CtBuffer buffer = ctBufferAlloc(0x100);
    ctBufferFree(buffer);
    return 0;
}
