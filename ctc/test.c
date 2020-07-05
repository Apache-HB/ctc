#include "ctc.h"

#include <stdio.h>

static void* posix_open(const char* path)
{
    return fopen(path, "r");
}

static int posix_next(void* ptr)
{
    return fgetc(ptr);
}

static void dumpList(CtASTList* list, void(*func)(CtAST*), const char* sep);

static void dumpIdent(CtAST* item)
{
    printf("%s", item->tok.data.ident);
}

static void dumpImport(CtAST* data)
{
    CtASTImport import = data->data.import;
    printf("import ");
    dumpList(import.path, dumpIdent, "::");

    if (import.items)
    {
        printf("(");
        dumpList(import.items, dumpIdent, ",");
        printf(")");
    }

    printf(";");
}

static void dumpStruct(CtAST* struc)
{
    printf("struct %s {", struc->data.struc->name.tok.data.ident);

    printf("}");
}

static void dumpBody(CtAST* item)
{
    switch (item->kind)
    {
    case AK_STRUCT:
        dumpStruct(item);
        break;
    default:
        printf("oh no\n");
    }
}

static void dumpList(CtASTList* list, void(*func)(CtAST*), const char* sep)
{
    while (1)
    {
        func(list->item);

        if (list->next)
            printf("%s", sep);
        else
            break;

        list = list->next;
    }
}

int main(int argc, const char** argv)
{
    CtStream* stream;
    CtLexer* lex;
    CtParser* parse;
    CtAST* ast;
    CtStreamCallbacks callbacks;

    (void)argc;
    callbacks.next = posix_next;
    callbacks.open = posix_open;

    stream = ctOpen(callbacks, argv[1]);
    lex = ctLexOpen(stream);
    parse = ctParseOpen(lex);

    ast = ctParseUnit(parse);

    dumpList(ast->data.unit.imports, dumpImport, "\n");

    printf("\n");

    return 0;
}
