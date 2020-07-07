#include <stdio.h>

#define CT_ERROR(data, msg) puts(msg);

#include "ctc/ctc_impl.h"

static int posixNext(void* ptr)
{
    return fgetc(ptr);
}

int main(int argc, const char** argv)
{
    CtStream* stream;
    CtLexer* lex;
    CtParser* parse;
    CtAST* ast;
    CtStreamCallbacks callbacks;

    (void)argc;
    callbacks.next = posixNext;

    stream = ctOpen(callbacks, fopen(argv[1], "rt"), argv[1]);
    lex = ctLexOpen(stream);
    parse = ctParseOpen(lex);

    ast = ctParseUnit(parse);

    return 0;
}
