#include <stdio.h>

#include "ctc/ctc_impl.h"

#include <signal.h>

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
        printList(type->data.types, printQualType, "::");
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

static CtDigit eval(CtAST* expr);

static CtDigit evalBin(CtAST* lhs, CtAST* rhs, CtKeyword op)
{
#define DO_OP(op, expr) case op: return eval(lhs) expr eval(rhs);
    switch (op)
    {
    DO_OP(K_ADD, +)
    DO_OP(K_SUB, -)
    DO_OP(K_MOD, %)
    DO_OP(K_DIV, /)
    DO_OP(K_MUL, *)
    DO_OP(K_SHL, <<)
    DO_OP(K_SHR, >>)
    DO_OP(K_BITXOR, ^)
    DO_OP(K_BITAND, &)
    DO_OP(K_BITOR, |)
    DO_OP(K_AND, &&)
    DO_OP(K_OR, ||)
    DO_OP(K_EQ, ==)
    DO_OP(K_NEQ, !=)
    DO_OP(K_LT, <)
    DO_OP(K_LTE, <=)
    DO_OP(K_GT, >)
    DO_OP(K_GTE, >=)
    default: 
        printf("oh no\n");
        return 0;
    }
#undef DO_OP
}

static CtDigit evalUnary(CtAST* node)
{
#define DO_OP(key, exp) case key: return exp eval(node->data.expr);

    switch (node->tok.data.key)
    {
    DO_OP(K_ADD, +)
    DO_OP(K_SUB, -)
    DO_OP(K_BITNOT, ~)
    default:
        printf("oh no\n");
        return 0;
    }
}

static CtDigit eval(CtAST* expr)
{
    if (expr->kind == AK_LITERAL)
    {
        return expr->tok.data.digit.digit;
    }
    else if (expr->kind == AK_BINOP)
    {
        return evalBin(expr->data.binop.lhs, expr->data.binop.rhs, expr->data.binop.op);
    }
    else if (expr->kind == AK_UNARY)
    {
        printf("%p\n", (void*)expr);
        return evalUnary(expr);
    }
    else if (expr->kind == AK_TERNARY)
    {
        return eval(expr->data.ternary.cond) ? eval(expr->data.ternary.truthy) : eval(expr->data.ternary.falsey);
    }
    else
    {
        printf("oh no\n");
        return 0;
    }
}

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    CtStreamCallbacks callbacks;
    CtStream stream;
    CtLexer lex;
    CtParser parse;

    callbacks.next = posixNext;

    stream = ctStreamOpen(callbacks, stdin, "stdin");
    lex = ctLexOpen(&stream);
    parse = ctParseOpen(&lex);

    //CtAST* node = ctParseInterp(&parse);

    //printf("eval %d\n", eval(node));

    CtAST* unit = ctParseUnit(&parse);

    printUnit(unit);

    return 0;
}
