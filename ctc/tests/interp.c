#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc((FILE*)stream); }

typedef struct CtSymbols {
    CtASTList symbols;

    struct CtSymbols* children;
    size_t num;
    size_t alloc;
} CtSymbols;

/* interpreter state */
typedef struct {
    CtSymbols symbols;
} CtInterp;

void evalNode(CtInterp* self, CtAST* node)
{

}

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    CtStreamCallbacks callbacks;
    CtStream* stream;
    CtLexer* lex;
    CtParser* parse;

    callbacks.next = posixNext;
    stream = ctStreamOpen(callbacks, stdin, "stdin");
    lex = ctLexOpen(stream);
    parse = ctParseOpen(lex);

    while (1)
    {
        CtAST* node = ctParseNext(parse);

        if (!node)
            break;


    }

    ctParseClose(parse);
    ctLexClose(lex);
    ctStreamClose(stream);

    return 0;
}
