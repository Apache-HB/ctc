#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc((FILE*)stream); }

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    CtStreamCallbacks callbacks;
    CtInterpData interp_data;
    CtStream* stream;
    CtLexer* lex;
    CtParser* parse;
    CtInterp* interp;

    callbacks.next = posixNext;

    /* TODO: what data do we need */
    interp_data.data = NULL;

    stream = ctStreamOpen(callbacks, stdin, "stdin");
    lex = ctLexOpen(stream);
    parse = ctParseOpen(lex);
    interp = ctInterpOpen(interp_data);

    while (1)
    {
        CtAST* node = ctParseNext(parse);

        if (!node)
            break;

        ctInterpEval(interp, node);
        ctFreeAST(node);
    }

    ctInterpClose(interp);
    ctParseClose(parse);
    ctLexClose(lex);
    ctStreamClose(stream);

    return 0;
}
