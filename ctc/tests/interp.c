#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc(stream); }

#define C_ASSERT(expr, msg) if (!(expr)) { printf(msg "\n"); exit(100); }

static void writeIdent(CtAST* node)
{
    C_ASSERT(node->kind == AK_IDENT, "invalid ident node")
    printf(node->tok.data.ident);
}

static void forList(CtASTList items, void(*func)(CtAST*), const char* sep)
{
    for (size_t i = 0; i < items.len; i++)
    {
        if (i != 0 && sep)
            printf(sep);

        func(&items.items[i]);
    }
}

static void evalImport(CtASTImport decl)
{
    printf("import ");
    forList(decl.path, writeIdent, "::");

    if (decl.items.len)
    {
        printf("(");

        forList(decl.items, writeIdent, ", ");

        printf(")");
    }

    printf(";\n");
}

static void reportError(CtAST* err)
{
    printf(err->data.reason);
}

static void evalNode(CtAST* node)
{
    if (node->kind == AK_IMPORT)
    {
        evalImport(node->data.include);
    }
    else if (node->kind == AK_ERROR)
    {
        reportError(node);
    }
    else
    {

    }
}

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

        evalNode(node);
    }

    ctInterpClose(interp);
    ctParseClose(parse);
    ctLexClose(lex);
    ctStreamClose(stream);

    return 0;
}
