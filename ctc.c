#include "ctc.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static CtFileCallbacks posix_callbacks = {
    (void(*)(void*))fclose,
    (int(*)(void*))fgetc
};

static int lexNext(CtLexer* self)
{
    int c = self->ahead;
    self->ahead = self->callbacks->next(self->data);

    self->pos.dist++;

    if (c == '\n')
    {
        self->pos.col = 0;
        self->pos.line++;
    }
    else
    {
        self->pos.col++;
    }

    return c;
}

static int lexPeek(CtLexer* self)
{
    return self->ahead;
}

CtLexer* ctCreateLexer(void* data, CtFileCallbacks* callbacks)
{
    CtLexer* lex = malloc(sizeof(CtLexer));

    lex->buffer[0] = '\0';
    lex->index = 0;

    lex->data = data;
    lex->callbacks = callbacks ? callbacks : &posix_callbacks;
    lex->ahead = lex->callbacks->next(lex->data);
    lex->pos.lex = lex;
    lex->pos.dist = 1;

    if (lex->ahead == '\n')
    {
        lex->pos.col = 0;
        lex->pos.line = 1;
    }
    else
    {
        lex->pos.col = 1;
        lex->pos.line = 0;
    }

    return lex;
}

static int skipWhitespace(CtLexer* self)
{
    int c = lexNext(self);

    while (isspace(c))
        c = lexNext(self);

    return c;
}

static int nextRealChar(CtLexer* self)
{
    int c = skipWhitespace(self);

    while (1)
    {
        if (c == ' ' || c == '\n' || c == '\t' || c == '\v')
        {
            c = lexNext(self);
        }
        else if (c == '#')
        {
            while (c != '\n')
                c = lexNext(self);

            while (isspace(c))
                c = lexNext(self);
        }
        else
        {
            break;
        }
    }

    return c;
}

static int isIdent1(int c) { return isalpha(c) || c == '_'; }
static int isIdent2(int c) { return isalnum(c) || c == '_'; }

static void lexPush(CtLexer* self, int c)
{
    self->buffer[self->index++] = (char)c;
    self->buffer[self->index] = '\0';
}

static void lexClear(CtLexer* self)
{
    self->buffer[0] = '\0';
    self->index = 0;
}

static CtToken lexIdent(CtLexer* self, int c)
{
    CtToken out;
    lexPush(self, c);

    while (isIdent2(lexPeek(self)))
        lexPush(self, lexNext(self));

    out.kind = TK_KEY;

    if (strcmp(self->buffer, "def") == 0)
    {
        out.data.key = K_DEF;
    }
    else if (strcmp(self->buffer, "alias") == 0)
    {
        out.data.key = K_ALIAS;
    }
    else if (strcmp(self->buffer, "import") == 0)
    {
        out.data.key = K_IMPORT;
    }
    else if (strcmp(self->buffer, "var") == 0)
    {
        out.data.key = K_VAR;
    }
    else
    {
        out.kind = TK_IDENT;
        out.data.ident = strdup(self->buffer);
    }

    return out;
}

static CtDigit lexBase2(CtLexer* self)
{
    int c = lexPeek(self);

    while (1)
    {
        if (c == '0' || c == '1')
        {
            lexPush(self, lexNext(self));
        }
        else if (c == '_')
        {
            lexNext(self);
        }
        else
        {
            break;
        }

        c = lexPeek(self);
    }

    return strtoull(self->buffer, NULL, 2);
}

static CtDigit lexBase10(CtLexer* self)
{
    while (isdigit(lexPeek(self)))
        lexPush(self, lexNext(self));

    return strtoull(self->buffer, NULL, 10);
}

static CtDigit lexBase16(CtLexer* self)
{
    int c = lexPeek(self);

    while (1)
    {
        if (isxdigit(c))
        {
            lexPush(self, lexNext(self));
        }
        else if (c == '_')
        {
            lexNext(self);
        }
        else
        {
            break;
        }

        c = lexPeek(self);
    }

    return strtoull(self->buffer, NULL, 16);
}

static CtDigit lexSpecialDigit(CtLexer* self)
{
    int c = lexPeek(self);

    if (c == 'x')
    {
        return lexBase16(self);
    }
    else if (c == 'b')
    {
        return lexBase2(self);
    }
    else if (isdigit(c))
    {
        /* error */
        return 0ull;
    }
    else
    {
        return 0ull;
    }
}

static CtToken lexDigit(CtLexer* self, int c)
{
    CtDigit digit;
    CtToken tok;
    if (c == '0')
    {
        digit = lexSpecialDigit(self);
    }
    else
    {
        lexPush(self, c);
        digit = lexBase10(self);
    }

    tok.kind = TK_INT;
    tok.data.digit = digit;

    return tok;
}

static int lexConsume(CtLexer* self, int c)
{
    if (lexPeek(self) == c)
    {
        lexNext(self);
        return 1;
    }

    return 0;
}

static int lexConsumePattern(CtLexer* self, int(*func)(int))
{
    int c = lexPeek(self);
    if (func(c))
    {
        lexNext(self);
        return c;
    }

    return 0;
}

static CtKey lexKey(CtLexer* self, int c)
{
    switch (c)
    {
    case '=': return lexConsume(self, '=') ? K_EQ : K_ASSIGN;
    case '+': return lexConsume(self, '=') ? K_ADDEQ : K_ADD;
    case '-': return lexConsume(self, '=') ? K_SUBEQ : K_SUB;
    case '!': return lexConsume(self, '=') ? K_NEQ : K_NOT;
    case '@': return K_AT;
    case '[': return K_LSQUARE;
    case ']': return K_RSQUARE;
    case '(': return K_LPAREN;
    case ')': return K_RPAREN;
    case '{': return K_LBRACE;
    case '}': return K_RBRACE;
    case ':': return lexConsume(self, ':') ? K_COLON2 : K_COLON;
    case ';': return K_SEMI;
    case ',': return K_COMMA;
    case '.': return K_DOT;
    case '?': return K_QUEST;
    default: return 0;
    }
}

static CtToken lexSymbol(CtLexer* self, int c)
{
    CtKey key = lexKey(self, c);
    CtToken tok;
    tok.kind = TK_KEY;
    tok.data.key = key;

    return tok;
}

static CtChar lexSingleChar(CtLexer* self)
{
    int c = lexNext(self);

    if (c == '\\')
    {
        switch ((c = lexNext(self)))
        {
        case 'a': return '\a';
        case 'b': return '\b';
        case 'r': return '\r';
        case 't': return '\t';
        case 'v': return '\v';
        case 'e': return '\033';
        case 'n': return '\n';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
        default: break;
        }
    }

    return (CtChar)c;
}

static CtToken lexChar(CtLexer* self)
{
    CtToken out;
    out.kind = TK_CHAR;

    out.data.letter = lexSingleChar(self);

    if (lexNext(self) != '\'')
    {
        /* error */
    }

    return out;
}

typedef struct {
    CtChar* ptr;
    int idx;
} CtBuffer;

static void bufPush(CtBuffer* self, CtChar c)
{
    self->ptr[self->idx++] = c;
    self->ptr[self->idx] = '\0';
}

static CtBuffer bufNew(size_t size)
{
    CtBuffer buf;
    buf.ptr = malloc(sizeof(CtChar) * size);
    buf.idx = 0;
    buf.ptr[0] = '\0';

    return buf;
}

/**
 * var s = """
 *  some string
 *  with linebreaks
 * """;
 */
static CtChar* lexMultilineString(CtLexer* self)
{
    CtBuffer buf = bufNew(512);
    CtChar c;

    while (1)
    {
        c = lexSingleChar(self);
        if (c == '"')
        {
            if (lexConsume(self, '"') && lexConsume(self, '"'))
            {
                break;
            }
        }

        bufPush(&buf, c);
    }

    /* TODO: will need a better string buffer */
    return buf.ptr;
}

/**
 * var s = "some string";
 */
static CtChar* lexSimpleString(CtLexer* self)
{
    CtBuffer buf = bufNew(512);
    CtChar c;

    while (1)
    {
        c = lexSingleChar(self);
        if (c == '"')
        {
            break;
        }
        else if (c == '\n')
        {
            /* newlines are invalid here */
        }
        else
        {
            bufPush(&buf, c);
        }
    }

    return buf.ptr;
}

static CtChar* emptyString = NULL;

static CtToken lexString(CtLexer* self)
{
    CtToken out;
    out.kind = TK_STRING;

    if (lexConsume(self, '"'))
    {
        if (lexConsume(self, '"'))
        {
            out.data.string = lexMultilineString(self);
        }
        else
        {
            out.data.string = emptyString;
        }
    }
    else
    {
        out.data.string = lexSimpleString(self);
    }

    return out;
}

static CtToken lexNextToken(CtLexer* self)
{
    int c = nextRealChar(self);
    CtFilePos pos = self->pos;
    CtToken tok;

    /* clear the buffer */
    lexClear(self);

    if (isIdent1(c))
    {
        tok = lexIdent(self, c);
    }
    else if (c == '\'')
    {
        tok = lexChar(self);
    }
    else if (c == '"')
    {
        tok = lexString(self);
    }
    else if (isdigit(c))
    {
        tok = lexDigit(self, c);
    }
    else
    {
        tok = lexSymbol(self, c);
    }

    tok.pos = pos;

    return tok;
}

typedef struct {
    CtLexer* lex;
    CtToken tok;
} CtParser;

static CtToken parseNext(CtParser* parser)
{
    CtToken tok = parser->tok;

    if (tok.kind == TK_INVALID)
    {
        tok = lexNextToken(parser->lex);
    }

    parser->tok.kind = TK_INVALID;

    return tok;
}

static int parseConsume(CtParser* self, CtTokenKind kind, CtTokenData data)
{
    CtToken tok = parseNext(self);

    if (tok.kind == kind && tok.data.digit == data.digit)
    {
        return 1;
    }

    self->tok = tok;
    return 0;
}

static CtAST* parseType(CtParser* self)
{

}

static CtAST* parseExpr(CtParser* self)
{

}

static CtAST* parseImport(CtParser* self)
{

}

static CtAST* parseDecorator(CtParser* self)
{

}

static CtAST* parseFunction(CtParser* self)
{

}

static CtAST* parseAlias(CtParser* self)
{

}

static CtAST* parseStruct(CtParser* self)
{

}

static CtAST* parseUnion(CtParser* self)
{

}

static CtAST* parseObject(CtParser* self)
{

}

static CtAST* parseUnit(CtParser* self)
{

}

CtAST* ctParse(CtLexer* lex)
{
    CtParser* self = malloc(sizeof(CtParser));
    CtAST* unit;
    self->tok.kind = TK_INVALID;
    self->lex = lex;

    unit = parseUnit(self);
    free(self);

    return unit;
}
