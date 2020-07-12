#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc(stream); }

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    CtStreamCallbacks callbacks;
    CtStream stream;
    CtLexer lex;
    CtParser parse;

    callbacks.next = posixNext;

    stream = ctStreamOpen(callbacks, stdin, "stdin");
    lex = ctLexOpen(&stream);
    parse = ctParseOpen(&lex);

    while (1)
    {

    }
}
