#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc(stream); }

static void printList(CtASTArray arr, void(*func)(CtAST*), const char* sep)
{
    for (size_t i = 0; i < arr.len; i++)
    {
        if (i != 0 && sep)
            printf(sep);
        func(&arr.data[i]);
    }
}

#define CT_ASSERT(expr, msg) { if (!(expr)) { printf(msg); exit(100); } }

static void printIdent(CtAST* id)
{
    CT_ASSERT(id->kind == AK_IDENT, "invalid kind")

    printf(id->data.ident);
}

static void printImport(CtAST* imp)
{
    CT_ASSERT(imp->kind == AK_IMPORT, "invalid kind")

    printf("import ");
    printList(imp->data.include.path, printIdent, "::");

    if (imp->data.include.symbols.len)
    {
        printf("(");
        printList(imp->data.include.path, printIdent, ", ");
        printf(")");
    }

    printf(";\n");
}

static void printExpr(CtAST* expr)
{
    (void)expr;
}

static void printType(CtAST* type);

static void printTypeParam(CtAST* param)
{
    CT_ASSERT(param->kind == AK_TYPE_PARAM, "invalid kind")

    if (param->data.param.name)
    {
        printf(":");
        printIdent(param->data.param.name);
        printf(" = ");
    }

    printType(param->data.param.type);
}

static void printQualType(CtAST* type)
{
    CT_ASSERT(type->kind == AK_QUAL, "invalid kind")
    printIdent(type->data.qual.name);

    if (type->data.qual.params.len)
    {
        printf("<");
        printList(type->data.qual.params, printTypeParam, ", ");
        printf(">");
    }
}

static void printType(CtAST* type)
{
    if (type->kind == AK_PTR)
    {
        printf("*");
        printType(type->data.ptr);
    }
    else if (type->kind == AK_REF)
    {
        printf("&");
        printType(type->data.ref);
    }
    else if (type->kind == AK_FUNC_TYPE)
    {
        printf("def");
        if (type->data.sig.args.len)
        {
            printf("(");
            printList(type->data.sig.args, printType, ", ");
            printf(")");
        }

        if (type->data.sig.result)
        {
            printf(" -> ");
            printType(type->data.sig.result);
        }
    }
    else if (type->kind == AK_ARR)
    {
        printf("[");
        printType(type->data.arr.type);
        if (type->data.arr.size)
        {
            printf(":");
            printExpr(type->data.arr.size);
        }
        printf("]");
    }
    else if (type->kind == AK_TYPE)
    {
        printList(type->data.types, printQualType, "::");
    }
    else
    {
        CT_ASSERT(false, "invalid kind")
    }
}

static void printAlias(CtAST* alias)
{
    CT_ASSERT(alias->kind == AK_ALIAS, "invalid kind")
    printf("alias ");
    printIdent(alias->data.alias.name);
    printf(" = ");
    printType(alias->data.alias.body);
    printf(";\n");
}

static void printSymbol(CtAST* sym)
{
    if (sym->kind == AK_ALIAS)
        printAlias(sym);
}

static void printUnit(CtAST* unit)
{
    CT_ASSERT(unit->kind == AK_UNIT, "invalid kind")
    printList(unit->data.unit.imports, printImport, NULL);
    printList(unit->data.unit.symbols, printSymbol, NULL);
}

int main(int argc, const char** argv)
{
    (void)argc;

    CtStreamCallbacks callbacks;
    CtStream stream;
    CtLexer lex;
    CtParser parse;

    callbacks.next = posixNext;

    stream = ctStreamOpen(callbacks, fopen(argv[1], "rt"), "stdin");
    lex = ctLexOpen(&stream);
    parse = ctParseOpen(&lex);

    CtAST* unit = ctParseUnit(&parse);

    printUnit(unit);

    return 0;
}
