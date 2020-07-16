#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc(stream); }

static void compileUnit(CtAST* unit)
{
    (void)unit;
}

int main(int argc, const char** argv)
{
    (void)argc;

    CtStreamCallbacks callbacks = { posixNext };
    CtStream stream = ctStreamOpen(callbacks, fopen(argv[1], "r"), argv[1]);
    CtLexer lex = ctLexOpen(&stream);
    CtParser parse = ctParseOpen(&lex);

    CtAST* unit = ctParseUnit(&parse);

    compileUnit(unit);

    return 0;
}
