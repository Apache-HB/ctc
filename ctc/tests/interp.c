#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc(stream); }

static void writeIdent(CtAST* node)
{
    printf("%s", node->tok.data.ident);
}

static void writeType(CtAST* node);

static void writeName(CtASTList name)
{
    for (size_t i = 0; i < name.len; i++)
    {
        if (i != 0)
            printf("::");
        writeIdent(&name.items[i]);
    }
}

static void writePointer(CtAST* node)
{
    printf("*");
    writeType(node);
}

static void writeType(CtAST* node)
{
    if (node->kind == AK_NAME)
        writeName(node->data.name);
    else if (node->kind == AK_POINTER)
        writePointer(node->data.ptr);
    else
        printf("oh no");
}

static void writeAlias(CtASTAlias node)
{
    printf("alias ");
    writeIdent(node.name);
    printf(" = ");
    writeType(node.symbol);
    printf(";\n");
}

static void writeItems(CtASTList list, const char* sep)
{
    for (size_t i = 0; i < list.len; i++)
    {
        if (i != 0)
            printf("%s", sep);
        writeIdent(&list.items[i]);
    }
}

static void writeImport(CtASTImport node)
{
    printf("import ");
    writeName(node.path);

    if (node.items.items)
    {
        printf("(");
        writeItems(node.items, ", ");
        printf(")");
    }

    printf(";\n");
}

static void writeNode(CtAST* node)
{
    if (node->kind == AK_ALIAS)
    {
        writeAlias(node->data.alias);
    }
    else if (node->kind == AK_IMPORT)
    {
        writeImport(node->data.import);
    }
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

        writeNode(node);
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
