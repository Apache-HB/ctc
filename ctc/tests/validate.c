#include <stdio.h>

#include "ctc/ctc_impl.h"

#include <signal.h>

static int posixNext(void* stream) { return fgetc(stream); }

#define CT_ASSERT(expr, msg) { if (!(expr)) { printf(msg "\n"); exit(100); } }

#define KIND(node, k) CT_ASSERT(node->kind == k, "invalid kind")

static void validateArray(CtASTArray arr, void(*func)(CtAST*))
{
    for (size_t i = 0; i < arr.len; i++)
    {
        func(&arr.data[i]);
    }
}

static void validateIdent(CtAST* id)
{
    KIND(id, AK_IDENT)
    CT_ASSERT(id->data.ident != NULL, "ident was null")
}

static void validateImport(CtAST* imp)
{
    KIND(imp, AK_IMPORT)
    validateArray(imp->data.include.path, validateIdent);
}

static void validateTypeParam(CtAST* node, int* named)
{
    KIND(node, AK_TYPE_PARAM)

    if (node->data.param.name)
    {
        validateIdent(node->data.param.name);
        *named = 1;
    }
    else
    {
        *named = 0;
    }

    validateType(node->data.param.type);
}

static void validateTypeParams(CtASTArray arr)
{
    int named = 0;
    int found = 0;

    for (int i = 0; i < arr.len; i++)
    {
        validateTypeParam(&arr.data[i], &named);
        
        if (named)
            found = 1;

        if (!named && found)
            CT_ASSERT(0, "unnamed type param after a named type param is invalid")
    }
}

static void validateQualType(CtAST* qual)
{
    KIND(qual, AK_QUAL)
    validateIdent(qual->data.qual.name);
    validateTypeParams(qual->data.qual.params);
}

static void validateQualTypes(CtASTArray arr)
{
    validateArray(arr, validateQualType);
}

static void validateType(CtAST* type)
{
    switch (type->kind)
    {
    case AK_PTR: validateType(type->data.ptr); break;
    case AK_REF: validateQualTypes(type->data.ref); break;
    case AK_TYPE: validateQualTypes(type->data.types); break;
    case AK_ARR: validateArr(type); break;
    default:
        CT_ASSERT(0, "invalid kind for type")
    }
}

static void validateAlias(CtAST* alias)
{
    KIND(alias, AK_ALIAS)
    validateIdent(alias->data.alias.name);
    validateType(alias->data.alias.body);
}

static void validateBody(CtAST* body)
{
    switch (body->kind)
    {
    case AK_ALIAS: validateAlias(body); break;
    case AK_STRUCT: validateStruct(body); break;
    case AK_UNION: validateUnion(body); break;
    case AK_ENUM: validateEnum(body); break;
    case AK_FUNCTION: validateFunction(body); break;
    case AK_VAR: validateVar(body); break;
    default:
        CT_ASSERT(0, "invalid body kind")
    }
}

static void validateUnit(CtAST* unit)
{
    KIND(unit, AK_UNIT)
    validateArray(unit->data.unit.imports, validateImport);
    validateArray(unit->data.unit.symbols, validateBody);
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

    CtAST* unit = ctParseUnit(&parse);

    validateUnit(unit);

    return 0;
}
