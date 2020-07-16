#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc(stream); }

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    CtStreamCallbacks callbacks = { posixNext };
    CtStream stream = ctStreamOpen(callbacks, stdin, "stdin");
    CtLexer lex = ctLexOpen(&stream);
    CtParser parse = ctParseOpen(&lex);

    return 0;
}
