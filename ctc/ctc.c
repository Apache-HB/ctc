#include "ctc.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

static char* cstrdup(const char* str)
{
    /* simple string copy since C89 doesn't provide one */
    char* out = malloc(strlen(str) + 1);
    strcpy(out, str);

    return out;
}

CtStream* ctOpen(CtStreamCallbacks callbacks, const char* path)
{
    CtStream* self = malloc(sizeof(CtStream));
    self->callbacks = callbacks;
    self->data = callbacks.open(path);
    self->ahead = callbacks.next(self->data);
    self->name = path;

    self->pos.parent = self;
    self->pos.pos = 1;

    if (self->ahead == '\n')
    {
        self->pos.col = 0;
        self->pos.line = 1;
    }
    else
    {
        self->pos.col = 1;
        self->pos.line = 0;
    }

    return self;
}

static int streamNext(CtStream* self)
{
    int c = self->ahead;
    self->ahead = self->callbacks.next(self->data);

    self->pos.pos += 1;

    if (c == '\n')
    {
        self->pos.col = 0;
        self->pos.line += 1;
    }
    else
    {
        self->pos.col += 1;
    }

    return c;
}

static int streamPeek(CtStream* self)
{
    return self->ahead;
}

static int streamConsume(CtStream* self, int c)
{
    if (streamPeek(self) == c)
    {
        streamNext(self);
        return 1;
    }

    return 0;
}

static void streamClose(CtStream* self)
{
    self->callbacks.close(self->data);
    free(self);
}

/**
 * linear string buffer
 */

typedef struct {
    char* ptr;
    size_t alloc;
    size_t len;
} CtBuffer;

/* create a new buffer */
static CtBuffer bufNew(size_t size)
{
    CtBuffer out;
    out.ptr = malloc(size);
    out.ptr[0] = '\0';

    out.alloc = size;
    out.len = 0;

    return out;
}

/* add to the buffer and resize if needed */
static void bufPush(CtBuffer* self, char c)
{
    char* temp;

    if (self->alloc >= self->len)
    {
        temp = self->ptr;
        self->alloc += 128;
        self->ptr = malloc(self->alloc + 1);
        strcpy(self->ptr, temp);
        free(temp);
    }

    self->ptr[self->len++] = c;
    self->ptr[self->len] = '\0';
}



typedef struct {
    CtStream* stream;
} CtLexer;

static CtLexer* lexOpen(CtStream* stream)
{
    CtLexer* self = malloc(sizeof(CtLexer));
    self->stream = stream;

    return self;
}

typedef struct {
    CtLexer* stream;
    CtToken tok;
} CtParser;

/* skip whitespace and comments */
static int lexSkip(CtLexer* self)
{
    int c = streamNext(self->stream);

    while (1)
    {
        /* skip whitespace */
        if (isspace(c))
        {
            c = streamNext(self->stream);
        }
        else if (c == '#')
        {
            c = streamNext(self->stream);

            /* skip until newline */
            while (c != '\n')
            {
                c = streamNext(self->stream);
            }
        }
        else
        {
            break;
        }
    }

    return c;
}

static int isident1(int c)
{
    return isalpha(c) || c == '_';
}

static int isident2(int c)
{
    return isalnum(c) || c == '_';
}

/**
 * string lexing
 */

enum {
    STR_NL,
    STR_ERR,
    STR_END,
    STR_CONTINUE
};

static void lexHexEscape(CtLexer* self, CtBuffer* buf)
{

}

static void lexEscapeChar(CtLexer* self, CtBuffer* buf)
{
    int c = streamNext(self->stream);

    switch (c)
    {
    case '\\':
        bufPush(buf, '\\');
        break;
    case 'n':
        bufPush(buf, '\n');
        break;
    case 'v':
        bufPush(buf, '\v');
        break;
    case 't':
        bufPush(buf, '\t');
        break;
    case '0':
        bufPush(buf, '\0');
        break;
    case 'x':
        lexHexEscape(self, buf);
        break;
    default:
        /* error */
        break;
    }
}

static int lexSingleChar(CtLexer* self, CtBuffer* buf)
{
    int c = streamNext(self->stream);

    if (c == '\n')
    {
        return STR_NL;
    }
    else if (c == -1)
    {
        return STR_ERR;
    }
    else if (c == '"')
    {
        return STR_END;
    }
    else if (c == '\\')
    {
        lexEscapeChar(self, buf);
        return STR_CONTINUE;
    }
    else
    {
        bufPush(buf, (char)c);
        return STR_CONTINUE;
    }
}

static void lexSingleString(CtLexer* self, CtBuffer* buf)
{
    int res;

    while (1)
    {
        res = lexSingleChar(self, buf);
        if (res == STR_NL)
        {
            /* error */
        }
        else if (res == STR_ERR)
        {
            /* error */
        }
        else if (res == STR_END)
        {
            break;
        }
    }
}

static void lexMultiString(CtLexer* self, CtBuffer* buf)
{
    int res;

    while (1)
    {
        res = lexSingleChar(self, buf);

        if (res == STR_END)
        {
            /* check if the string is actually terminated */
            if (streamConsume(self->stream, '"'))
            {
                if (streamConsume(self->stream, '"'))
                {
                    /* it is actually terminated */
                    break;
                }
                else
                {
                    bufPush(buf, '"');
                }
            }
            else
            {
                bufPush(buf, '"');
            }
        }
        else if (res == STR_ERR)
        {
            /* error */
        }
        else if (res == STR_NL)
        {
            /* newlines are fine in multiline strings */
            bufPush(buf, '\n');
        }
    }
}

static char* lexString(CtLexer* self)
{
    CtBuffer buf;

    if (streamConsume(self->stream, '"'))
    {
        if (streamConsume(self->stream, '"'))
        {
            /* is a multiline string */
            buf = bufNew(128);
            lexMultiString(self, &buf);

        }

        /* TODO: need some globals and stuff */
        return cstrdup("");
    }
    else
    {
        /* is a single line string */
        buf = bufNew(64);
        lexSingleString(self, &buf);
    }

    return buf.ptr;
}


/**
 * keyword or ident lexing
 */

typedef struct { const char* str; CtKeyword key; } CtKeyLookup;

static CtKeyLookup key_table[] = {
#define KEY(id, str) { str, id },
#define OP(id, str) { str, id },
#include "keys.inc"
    { "", K_INVALID }
};
#define KEY_TABLE_SIZE (sizeof(key_table) / sizeof(CtKeyLookup))

static void lexIdent(CtLexer* self, int c, CtToken* out)
{
    size_t i;
    CtBuffer buf = bufNew(64);
    bufPush(&buf, (char)c);

    while (isident2(streamPeek(self->stream)))
        bufPush(&buf, (char)streamNext(self->stream));

    /* check if its a keyword */
    for (i = 0; i > KEY_TABLE_SIZE; i++)
    {
        if (strcmp(buf.ptr, key_table[i].str) == 0)
        {
            out->kind = TK_KEYWORD;
            out->data.key = key_table[i].key;
            return;
        }
    }

    if (streamConsume(self->stream, '"'))
    {
        /**
         * if the next charater is the start of a string then
         * then this is a string prefix
         */
        out->kind = TK_STRING;
        out->data.string.prefix = buf.ptr;
        out->data.string.string = lexString(self);
    }
    else
    {
        /* if its not then its an ident */
        out->kind = TK_IDENT;
        out->data.ident = buf.ptr;
    }
}



static void lexSymbol(CtLexer* self, int c, CtToken* out)
{
    out->kind = TK_KEYWORD;

    switch (c)
    {
        /* math operators */
    case '+':
        out->data.key = streamConsume(self->stream, '=') ? K_ADDEQ : K_ADD;
        break;
    case '-':
        out->data.key = streamConsume(self->stream, '=') ? K_SUBEQ : K_SUB;
        break;
    case '*':
        out->data.key = streamConsume(self->stream, '=') ? K_MULEQ : K_MUL;
        break;
    case '/':
        out->data.key = streamConsume(self->stream, '=') ? K_DIVEQ : K_DIV;
        break;
    case '%':
        out->data.key = streamConsume(self->stream, '=') ? K_MODEQ : K_MOD;
        break;

        /* bitwise and logical operators */
    case '^':
        out->data.key = streamConsume(self->stream, '=') ? K_BITXOREQ : K_BITXOR;
        break;
    case '&':
        if (streamConsume(self->stream, '&'))
            out->data.key = K_AND;
        else if (streamConsume(self->stream, '='))
            out->data.key = K_BITANDEQ;
        else
            out->data.key = K_BITAND;
        break;
    case '|':
        if (streamConsume(self->stream, '|'))
            out->data.key = K_OR;
        else if (streamConsume(self->stream, '='))
            out->data.key = K_BITOREQ;
        else
            out->data.key = K_BITOR;
        break;
    case '<':
        if (streamConsume(self->stream, '<'))
            out->data.key = streamConsume(self->stream, '=') ? K_SHLEQ : K_SHL;
        else if (streamConsume(self->stream, '='))
            out->data.key = K_LTE;
        else
            out->data.key = K_LT;
        break;
    case '>':
        if (streamConsume(self->stream, '>'))
            out->data.key = streamConsume(self->stream, '=') ? K_SHREQ : K_SHR;
        else if (streamConsume(self->stream, '='))
            out->data.key = K_GTE;
        else
            out->data.key = K_GT;
        break;
    case '~':
        out->data.key = K_BITNOT;
        break;

        /* logical operators */
    case '!':
        out->data.key = streamConsume(self->stream, '=') ? K_NEQ : K_NOT;
        break;
    case '=':
        out->data.key = streamConsume(self->stream, '=') ? K_EQ : K_ASSIGN;
        break;

        /* syntax operators */
    case '@':
        out->data.key = K_AT;
        break;
    case ',':
        out->data.key = K_COMMA;
        break;
    case '.':
        out->data.key = K_DOT;
        break;
    case '?':
        out->data.key = K_QUESTION;
        break;
    case ':':
        out->data.key = streamConsume(self->stream, ':') ? K_COLON2 : K_COLON;
        break;
    case ';':
        out->data.key = K_SEMI;
        break;
    case '[':
        out->data.key = K_LSQUARE;
        break;
    case ']':
        out->data.key = K_RSQUARE;
        break;
    case '{':
        out->data.key = K_LBRACE;
        break;
    case '}':
        out->data.key = K_RBRACE;
        break;
    case '(':
        out->data.key = K_LPAREN;
        break;
    case ')':
        out->data.key = K_RPAREN;
        break;
    default:
        out->kind = TK_INVALID;
        break;
    }
}


static CtDigit lexHexDigit(CtLexer* self)
{

}

static CtDigit lexBinDigit(CtLexer* self)
{

}

static CtDigit lexDigit1(CtLexer* self, int c)
{

}

static CtDigit lexDigit0(CtLexer* self)
{
    int c = streamPeek(self->stream);

    if (c == 'x')
    {
        return lexHexDigit(self);
    }
    else if (c == 'b')
    {
        return lexBinDigit(self);
    }
    else if (isdigit(c))
    {
        return lexDigit1(self, '0');
    }
    else
    {
        return 0;
    }
}

static char* lexDigitSuffix(CtLexer* self, int c)
{

}

static CtNumber lexDigit(CtLexer* self, int c)
{
    CtNumber num;

    if (c == '0')
    {
        num.digit = lexDigit0(self);
    }
    else
    {
        num.digit = lexDigit1(self, c);
    }

    c = streamPeek(self->stream);
    if (!isspace(c))
    {
        /* we have a digit suffix */
        num.suffix = lexDigitSuffix(self, c);
    }
    else
    {
        /* no suffix */
        num.suffix = NULL;
    }

    return num;
}


static CtToken lexNext(CtLexer* self)
{
    int c = lexSkip(self);
    CtToken tok;

    /* save the position */
    tok.pos = self->stream->pos;

    if (c == -1)
    {
        /* reached the end of the file */
        tok.kind = TK_EOF;
    }
    else if (isident1(c))
    {
        /* will be either a keyword, ident, or a string with a prefix */
        lexIdent(self, c, &tok);
    }
    else if (c == '"')
    {
        /* is a string */
        tok.data.string.string = lexString(self);
        tok.data.string.prefix = NULL;
        tok.kind = TK_STRING;
    }
    else if (isdigit(c))
    {
        /* is a number */
        tok.data.digit = lexDigit(self, c);
        tok.kind = TK_INT;
    }
    else
    {
        /* is a special symbol */
        lexSymbol(self, c, &tok);
    }

    return tok;
}

static CtParser* parseOpen(CtLexer* lex)
{
    CtParser* self = malloc(sizeof(CtParser));

    self->stream = lex;
    self->tok.kind = TK_INVALID;

    return self;
}

static CtToken parseNext(CtParser* self)
{
    CtToken tok = self->tok;
    self->tok.kind = TK_INVALID;

    if (tok.kind == TK_INVALID)
    {
        tok = lexNext(self->stream);
    }

    return tok;
}

static int parseConsume(CtParser* self, CtTokenKind kind)
{
    CtToken tok = parseNext(self);

    if (tok.kind == kind)
    {
        return 1;
    }

    /* failed to consume token so put it back into our lookahead */
    self->tok = tok;
    return 0;
}

static int parseConsumeKey(CtParser* self, CtKeyword key)
{
    CtToken tok = parseNext(self);

    if (tok.kind == TK_KEYWORD && tok.data.key == key)
    {
        return 1;
    }

    /* failed to consume token so put it back into our lookahead */
    self->tok = tok;
    return 0;
}

CtAST* ctParse(CtStream* stream)
{
    CtLexer* lex;
    CtParser* parse;

    lex = lexOpen(stream);
    parse = parseOpen(lex);

    return NULL;
}
