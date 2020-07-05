#include "ctc.h"

#include <stdio.h>

static void* posix_open(const char* path)
{
    return fopen(path, "r");
}

static int posix_next(void* ptr)
{
    return fgetc(ptr);
}

int main(int argc, const char** argv)
{
    CtStream* stream;
    CtAST* ast;
    CtStreamCallbacks callbacks;

    (void)argc;
    callbacks.next = posix_next;
    callbacks.open = posix_open;

    stream = ctOpen(callbacks, argv[1]);

    ast = ctParse(stream);
}
