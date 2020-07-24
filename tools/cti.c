#include <stdlib.h>
#include <stdio.h>

#define CT_MALLOC malloc
#define CT_REALLOC realloc
#define CT_FREE free

#include "cthulhu/cthulhu.c"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static int next(void *ptr) { return fgetc(ptr); }

static void range(CtView view, CtBuffer src)
{
    for (size_t i = 0; i < view.len; i++)
    {
        int c = src.ptr[view.offset + i];
        if (c == '\n')
            break;
        putc(c, stdout);
    }
}

static void range2(CtOffset off, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        int c = off.source->source.ptr[off.dist + i - 1];
        if (c == '\n')
            break;
        putc(c, stdout);
    }
}

static void ident(CtToken tok)
{
    range(tok.data.ident, tok.pos.source->source);
}

static void key(CtToken tok)
{
    switch (tok.data.key)
    {
#define KEY(id, str, flags) case id: printf(str); break;
#define OP(id, str) case id: printf("%s", str); break;
#include "cthulhu/keys.inc"
    default: printf("invalid"); break;
    }
}

static void bin(size_t num)
{
    while (num)
    {
        printf("%d", (num & 1) == 1);
        num >>= 1;
    }
}

static void num(CtToken tok)
{
    switch (tok.data.digit.enc)
    {
    case BASE2:
        printf("0b");
        bin(tok.data.digit.num);
        break;
    case BASE10:
        printf("%lu", tok.data.digit.num);
        break;
    case BASE16:
        printf("0x%lx", tok.data.digit.num);
        break;
    }

    if (tok.data.digit.suffix.offset)
        range(tok.data.digit.suffix, tok.pos.source->source);
}

static void letter(CtToken tok)
{
    printf("%lu", tok.data.letter);
}

static void string(CtToken tok)
{
    if (tok.data.str.multiline)
        printf("R(");

    for (size_t i = 0; i < tok.data.str.len; i++)
    {
        char c = tok.pos.source->strings.ptr[tok.data.str.offset + i];
        switch (c)
        {
        case '\0': printf("\\0"); break;
        case '\n': printf("\\n"); break;
        default: putc(c, stdout); break;
        }
    }
    printf(")");
}

static void wtok(CtToken tok)
{
    switch (tok.type)
    {
    case TK_END: printf("eof"); break;
    case TK_LOOKAHEAD: printf("lookahead"); break;
    case TK_IDENT: printf("ident("); ident(tok); printf(")"); break;
    case TK_KEY: printf("key("); key(tok); printf(")"); break;
    case TK_INT: printf("int("); num(tok); printf(")"); break;
    case TK_CHAR: printf("char("); letter(tok); printf(")"); break;
    case TK_STRING: printf("string("); string(tok); printf(")"); break;
    }
}

static void underline(size_t indent, size_t num)
{
    for (size_t i = 0; i < indent; i++)
        putc(' ', stdout);
    for (size_t i = 0; i < num; i++)
        putc('^', stdout);
}

static void ptok(CtToken tok)
{
    size_t begin = tok.pos.dist - tok.pos.col;
    printf("%s [%lu:%lu]: ", tok.pos.source->name, tok.pos.line, tok.pos.col);
    wtok(tok);
    printf("\n | \n");
    printf(" | ");
    size_t len = 0;
    while (1)
    {
        char c = tok.pos.source->source.ptr[begin++];
        if (c == '\n' || c == '\0')
            break;
        putc(c, stdout);
        len++;
    }
    printf("\n | ");
    underline(tok.pos.col - 1, MIN(tok.len, len));
    printf("\n");
}

static void error(CtError err)
{
    printf("error [%d]: ", err.type);
    switch (err.type)
    {
    case ERR_NONE:
        printf("no error\n");
        break;
    case ERR_OVERFLOW:
        printf("integer literal ");
        range2(err.pos, err.len);
        printf(" is too large to fit into largest available integer\n");
        break;
    case ERR_INVALID_SYMBOL:
        range2(err.pos, err.len);
        printf(" is an invalid symbol\n");
        break;
    case ERR_STRING_LINEBREAK:
        printf("single line literal ");
        range2(err.pos, err.len);
        printf(" contains a line break\n");
        break;
    default:
        printf("internal compiler error\n");
        break;
    }
}

static int errors(CtState *self)
{
    int ret = 0;
    for (size_t i = 0; i < self->err_idx; i++)
    {
        ret = 1;
        error(self->errs[i]);
    }

    self->err_idx = 0;

    return ret;
}

int main(void)
{
    CtState state;
    ctStateNew(&state, stdin, next, "stdin", 20);

    for (;;)
    {
        CtToken tok = lexToken(&state);

        if (!errors(&state))
            ptok(tok);

        if (tok.type == TK_END)
            break;
    }
}