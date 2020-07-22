#include "cthulhu/cthulhu.h"

#include <stdlib.h>
#include <stdio.h>

#define CT_MALLOC malloc
#define CT_FREE free

#include "cthulhu/cthulhu.c"

static int posixGet(void *ptr) { return fgetc(ptr); }

static const char *keyString(CtKey key)
{
    switch (key)
    {
#define KEY(id, str) case id: return str;
#define OP(id, str) case id: return str;
#include "cthulhu/keys.inc"
    default: return "invalid";
    }
}

static void printInt(CtDigit num)
{
    switch (num.enc)
    {
    case DE_BASE10:
        printf("INT(%ld", num.num);
        break;
    case DE_BASE16:
        printf("HEX(0x%08lX", num.num);
        break;
    case DE_BASE2:
        printf("BIN(0b");
        for(int i = sizeof(CtSize) << 3; i; i--)
            putchar('0' + ((num.num >> (i - 1)) & 1));
        break;
    }

    if (num.suffix)
    {
        printf("%s", num.suffix);
    }

    printf(")");
}

static void printChar(char c)
{
    switch (c)
    {
    case '\0':
        printf("\\0");
        break;
    default:
        putchar(c);
        break;
    }
}

static void printString(CtString str)
{
    if (str.len & CT_MULTILINE_FLAG)
    {
        printf("STRING(R\"");
    }
    else
    {
        printf("STRING(\"");
    }

    for (CtSize i = 0; i < CT_STRLEN(str); i++)
        printChar(str.str[i]);

    printf("\")");
}

static void printUKey(CtUserKey key)
{
    (void)key;
    printf("UKEY()");
}

static void printToken(CtToken tok)
{
    printf("[%ld:%ld]\n    ", tok.pos.line, tok.pos.col);
    switch (tok.kind)
    {
    case TK_CHAR:
        printf("CHAR('%c')", (char)tok.data.letter);
        break;
    case TK_KEYWORD:
        printf("KEYWORD(%s)", keyString(tok.data.key));
        break;
    case TK_IDENT:
        printf("IDENT(%s)", tok.data.ident);
        break;
    case TK_INT:
        printInt(tok.data.num);
        break;
    case TK_STRING:
        printString(tok.data.str);
        break;
    case TK_USER_KEYWORD:
        printUKey(tok.data.ukey);
        break;
    default:
        printf("`INVALID`");
        break;
    }

    printf("\n");
}

#define KIND(kind) if (node && node->type != kind) { printf("invalid kind\n"); exit(1); }

static void printExpr(CtNode *node);

static void printBinary(CtNode *node)
{
    KIND(NT_BINARY)
    printf("(");
    printExpr(node->data.binary.lhs);
    printf(" %s ", keyString(node->data.binary.op));
    printExpr(node->data.binary.rhs);
    printf(")");
}

static void printTernary(CtNode *node)
{
    KIND(NT_TERNARY)
    printf("(");
    printExpr(node->data.ternary.cond);
    printf(" ? ");
    printExpr(node->data.ternary.yes);
    printf(" : ");
    printExpr(node->data.ternary.no);
    printf(")");
}

static void printUnary(CtNode *node)
{
    KIND(NT_UNARY)
    printf("(%s", keyString(node->data.unary.op));
    printExpr(node->data.unary.expr);
    printf(")");
}

static void printLiteral(CtNode *node)
{
    KIND(NT_LITERAL)
    switch (node->tok.kind)
    {
    case TK_STRING:
        printString(node->tok.data.str);
        break;
    case TK_CHAR:
        printChar(node->tok.data.letter);
        break;
    case TK_INT:
        printInt(node->tok.data.num);
        break;
    default:
        printf("invalid literal");
        exit(1);
    }
}

static void printExpr(CtNode *node)
{
    switch (node->type)
    {
    case NT_BINARY: printBinary(node); break;
    case NT_TERNARY: printTernary(node); break;
    case NT_UNARY: printUnary(node); break;
    case NT_LITERAL: printLiteral(node); break;
    default:
        printf("invalid expr\n");
        exit(1);
    }
}

typedef struct {
    struct CtObject *val;
    char *name;
} CtStructField;

typedef struct {
    /* fields */
    CtStructField *fields;
    int num;

    /* the alignment of the struct */
    int align;

    /* the maximum packing */
    int packing;

    /* name of struct */
    char *name;
} CtStruct;

typedef struct {
    struct CtTypeInfo *type;
    char *name;
} CtUnionField;

typedef struct {
    /* fields */
    CtUnionField *fields;
    int num;

    /* the currently set field index */
    int selected;

    /* name of union */
    char *name;

    /* the currently held value */
    struct CtObject *val;
} CtUnion;

typedef struct {
    enum {
        TT_STRING,
        TT_U8,
        TT_U16,
        TT_U32,
        TT_U64,
        TT_I8,
        TT_I16,
        TT_I32,
        TT_I64,
        TT_BOOL,
        TT_REF,
        TT_BOOL,
        TT_STRUCT,
        TT_UNION,
        TT_PTR,
        TT_REF,
        TT_ARR,
        TT_CLOSURE
    } type;

    /* is the data mutable, by default everything is immutable */
    int mut;
} CtTypeInfo;

typedef struct CtObject {
    CtTypeInfo type;

    union {
        char *str;
        uint8_t _u8;
        uint16_t _u16;
        uint32_t _u32;
        uint64_t _u64;
        int8_t _i8;
        int16_t _i16;
        int32_t _i32;
        int64_t _i64;
        CtStruct struc;
        CtUnion uni;
        struct CtObject *ptr;
        struct CtObject *ref;
    } data;
} CtObject;

static CtTypeInfo *typeofObject(CtObject *object)
{

}

static CtObject *makeObject(CtNode *node)
{

}

static CtObject *evalNode(CtNode *node)
{
    switch (node->type)
    {
    case NT_ERROR:
        return NULL;
    case NT_BINARY:
        return evalBinary(node);
    case NT_UNARY:
        return evalUnary(node);
    case NT_TERNARY:
        return evalTernary(node);
    case NT_LITERAL:
        return makeObject(node);
    default:
        break;
    }

    return NULL;
}

typedef struct {
    const char *str;
    int idx;
} StringStream;

int sstreamNext(void* ptr)
{
    StringStream* self = (StringStream*)ptr;
    int c = self->str[self->idx++];
    if (!c)
        return -1;

    return c;
}

int main(int argc, const char **argv)
{
    (void)printToken;

    StringStream sstream;
    CtLexer lex;

    if (argc == 2)
    {
        sstream.str = argv[1];
        sstream.idx = 0;
        lex = ctLexerNew(&sstream, sstreamNext);
    }
    else
    {
        lex = ctLexerNew(stdin, posixGet);
    }

    CtParser parse = ctParserNew(lex);


    CtNode *node = ctParseEval(&parse);

    evalNode(node);
}
