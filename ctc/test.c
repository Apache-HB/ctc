#include "ctc.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static void* posixOpen(const char* path)
{
    return fopen(path, "r");
}

static int posixNext(void* ptr)
{
    return fgetc(ptr);
}

static void dumpErr(CtStreamPos pos, char* msg)
{
    printf("[%s:%d:%d] --> %s\n", pos.parent->name, pos.line + 1, pos.col, msg);
    fflush(stdout);

    /* free(msg); */
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

static void dumpBuiltin(CtBuiltin blt)
{
    switch (blt)
    {
    case BL_I8: printf("i8"); break;
    case BL_I16: printf("i16"); break;
    case BL_I32: printf("i32"); break;
    case BL_I64: printf("i64"); break;

    case BL_U8: printf("u8"); break;
    case BL_U16: printf("u16"); break;
    case BL_U32: printf("u32"); break;
    case BL_U64: printf("u64"); break;

    case BL_VOID: printf("void"); break;
    }
}

static void dumpType(CtAST* type);

static void dumpFuncptr(CtFuncptr func)
{
    printf("def(");

    if (func.args)
        dumpList(func.args, dumpType, ",");

    printf(") -> ");

    if (func.result)
    {
        dumpType(func.result);
    }
    else
    {
        printf("void");
    }
}

static void dumpType(CtAST* type)
{
    if (type->kind == AK_BUILTIN)
    {
        dumpBuiltin(type->data.builtin);
    }
    else if (type->kind == AK_TYPENAME)
    {
        dumpList(type->data.name, dumpIdent, "::");
    }
    else if (type->kind == AK_POINTER)
    {
        printf("*");
        dumpType(type->data.ptr);
    }
    else if (type->kind == AK_FUNCPTR)
    {
        dumpFuncptr(type->data.funcptr);
    }
    else
    {
        /* error */
    }
}

static void dumpAlias(CtAST* alias)
{
    printf("alias %s = ", alias->data.alias.name->tok.data.ident);
    dumpType(alias->data.alias.symbol);
    printf(";");
}

static void dumpBody(CtAST* item)
{
    if (item->kind == AK_ALIAS)
    {
        dumpAlias(item);
    }
}

static void dumpList(CtASTList* list, void(*func)(CtAST*), const char* sep)
{
    (void)dumpBody;
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
    callbacks.next = posixNext;
    callbacks.open = posixOpen;
    callbacks.err = dumpErr;

    stream = ctOpen(callbacks, argv[1]);
    lex = ctLexOpen(stream);
    parse = ctParseOpen(lex);

    ast = ctParseUnit(parse);

    dumpList(ast->data.unit.imports, dumpImport, "\n");

    printf("\n");

    dumpList(ast->data.unit.body, dumpBody, "\n");

    printf("\n");

    return 0;
}
