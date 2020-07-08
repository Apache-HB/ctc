#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc(stream); }

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
        /*CtToken tok = ctLexNext(lex);

        char* str = tokStr(tok);
        puts(str);

        tokFree(tok);
        CT_FREE(str);

        if (tok.kind == TK_ERROR || tok.kind == TK_EOF)
            break;*/
    }

    ctParseClose(parse);
    ctLexClose(lex);
    ctStreamClose(stream);

    return 0;
}
