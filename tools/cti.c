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

    printf(")\n");
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

    printf("\")\n");
}

static void printUKey(CtUserKey key)
{
    (void)key;
    printf("UKEY()\n");
}

static void printToken(CtToken tok)
{
    printf("[%ld:%ld]\n    ", tok.pos.line, tok.pos.col);
    switch (tok.kind)
    {
    case TK_CHAR:
        printf("CHAR('%c')\n", (char)tok.data.letter);
        break;
    case TK_KEYWORD:
        printf("KEYWORD(%s)\n", keyString(tok.data.key));
        break;
    case TK_IDENT:
        printf("IDENT(%s)\n", tok.data.ident);
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
        printf("`INVALID`\n");
        break;
    }
}

int main(int argc, const char **argv)
{
    (void)argc;
    (void)argv;

    CtLexer lex = ctLexerNew(stdin, posixGet);

    while (1)
    {
        CtToken tok = ctLexerNext(&lex);

        printToken(tok);

        ctFreeToken(tok);

        if (tok.kind == TK_EOF || tok.kind == TK_ERROR)
            break;
    }
}
