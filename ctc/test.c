#include "ctc.h"

#include <stdio.h>

void* posix_open(const char* path)
{
    return fopen(path, "r");
}

int main(int argc, const char** argv)
{
    CtStream* stream;
    CtAST* ast;

    CtStreamCallbacks callbacks;
    callbacks.close = fclose;
    callbacks.next = fgetc;
    callbacks.open = posix_open;

    stream = ctOpen(callbacks, argv[1]);

    ast = ctParse(stream);
}
