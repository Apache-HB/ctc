#include <stdlib.h>
#include <stdio.h>

#define CT_MALLOC malloc
#define CT_REALLOC realloc
#define CT_FREE free

#include "cthulhu/cthulhu.c"

static int next(void *ptr) { return fgetc(ptr); }

int main(void)
{
    CtState state;
    ctStateNew(&state, stdin, next, "stdin");

    CtToken tok = lexToken(&state);

    printf("%d\n", tok.type);
}