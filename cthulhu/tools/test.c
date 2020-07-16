#include <stdio.h>

#include "cthulhu/cthulhu.h"

static int posixNext(void* stream) { return fgetc(stream); }

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    CtStreamCallbacks callbacks = { posixNext };
    CtStream stream = ctStreamOpen(callbacks, stdin, "stdin");
    CtLexer lex = ctLexOpen(&stream);
    CtParser parse = ctParseOpen(&lex);

    (void)parse;

    return 0;
}
