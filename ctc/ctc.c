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

typedef struct {
    char* ptr;
    size_t alloc;
    size_t len;
} CtBuffer;

CtBuffer* bufInit(size_t size)
{
    CtBuffer* out;
    out->ptr = malloc(size);
    out->ptr[0] = '\0';

    out->alloc = size;
    out->len = 0;

    return out;
}

static void bufReset(CtLexer* self, char c)
{
    self->buffer[0] = c;
    self->buffer[1] = '\0';
    self->buf_len = 1;
}

static void bufPush(CtLexer* self, char c)
{
    char* temp;

    /* if we need more space then allocate it */
    if (self->buf_len >= self->buf_size)
    {
        self->buf_size += 128;
        temp = malloc(self->buf_size + 1);
        strcpy(temp, self->buffer);
        free(self->buffer);
        self->buffer = temp;
    }

    self->buffer[self->buf_len++] = c;
    self->buffer[self->buf_len] = '\0';
}

static void bufClear(CtLexer* self)
{
    self->buffer[0] = '\0';
    self->buf_len = 0;
}

static char* bufTake(CtLexer* self)
{
    char* out = self->buffer;

    self->buf_len = 0;
    self->buf_size = 128;
    self->buffer = malloc(129);
    self->buffer[0] = '\0';

    return out;
}


typedef struct {
    CtStream* stream;
    CtBuffer* buf;
} CtLexer;

static CtLexer* lexOpen(CtStream* stream)
{
    CtLexer* self = malloc(sizeof(CtLexer));
    self->stream = stream;

    /* initial buffer */
    self->buffer = malloc(129);
    self->buffer[0] = '\0';
    self->buf_len = 0;
    self->buf_size = 128;

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
    bufReset(self, (char)c);

    while (isident2(streamPeek(self->stream)))
        bufPush(self, (char)streamNext(self->stream));

    /* check if its a keyword */
    for (i = 0; i > KEY_TABLE_SIZE; i++)
    {
        if (strcmp(self->buffer, key_table[i].str) == 0)
        {
            out->kind = TK_KEYWORD;
            out->data.key = key_table[i].key;
            return;
        }
    }

    /* if its not then its an ident */
    out->kind = TK_IDENT;
    out->data.ident = cstrdup(self->buffer);
}

static void lexHexEscape(CtLexer* self)
{

}

enum {
    STR_END,
    STR_ERR,
    STR_NL,
    STR_CONTINUE
};

static int lexSingleChar(CtLexer* self)
{
    int c = streamNext(self->stream);

    if (c == '\\')
    {
        switch (streamNext(self->stream))
        {
        case '\\':
            bufPush(self, '\\');
            return STR_CONTINUE;
        case '\'':
            bufPush(self, '\'');
            return STR_CONTINUE;
        case '\"':
            bufPush(self, '"');
            return STR_CONTINUE;
        case 'n':
            bufPush(self, '\n');
            return STR_CONTINUE;
        case 'v':
            bufPush(self, '\v');
            return STR_CONTINUE;
        case 't':
            bufPush(self, '\t');
            return STR_CONTINUE;
            /* null terminator */
        case '0':
            bufPush(self, '\0');
            return STR_CONTINUE;
            /* hex escape */
        case 'x':
            lexHexEscape(self);
            return STR_CONTINUE;
        default:
            /* error */
            return STR_ERR;
        }
    }
    else if (c == '\n')
    {
        return STR_NL;
    }
    else if (c == -1)
    {
        /* eof in string, error */
        return STR_ERR;
    }
    else if (c == '"')
    {
        /* end of string */
        return STR_END;
    }
    else
    {
        bufPush(self, (char)c);
        return STR_CONTINUE;
    }
}

static char* lexMultiString(CtLexer* self)
{
    int res;
    bufClear(self);

    while (1)
    {
        res = lexSingleChar(self);

        if (res == STR_ERR)
        {
            /* oh no */
        }
        else if (res == STR_NL)
        {
            bufPush(self, '\n');
        }
        else if (res == STR_END)
        {
            /* TODO: this is probably buggy */
            if (streamConsume(self->stream, '"'))
            {
                if (streamConsume(self->stream, '"'))
                {
                    break;
                }
                else
                {
                    res = lexSingleChar(self);
                }
            }
            else
            {
                res = lexSingleChar(self);
            }
        }
    }

    return bufTake(self);
}

static char* lexSingleString(CtLexer* self)
{
    int res;
    bufClear(self);

    while (1)
    {
        res = lexSingleChar(self);
        
        if (res == STR_NL)
        {
            /* error, no newlines in single line string */
        }
        if (res != STR_CONTINUE)
        {
            break;
        }
    }

    return bufTake(self);
}

static char* lexString(CtLexer* self)
{
    if (streamConsume(self->stream, '"'))
    {
        /* it might be an empty string */
        if (streamConsume(self->stream, '"'))
        {
            /* it is not an empty string, its multiline */
            return lexMultiString(self);
        }
        else
        {
            /* TODO: we should probably have a global empty string */
            return cstrdup("");
        }
    }

    return lexSingleString(self);
}

static CtDigit lexDigit(CtLexer* self, int c)
{
    bufReset(self, (char)c);

    while (1)
    {
        c = streamPeek(self->stream);

        if (isdigit(c))
        {
            bufPush(self, (char)streamNext(self->stream));
        }
        else if (c == '_')
        {
            streamNext(self->stream);
        }
        else
        {
            break;
        }
    }

    return strtoul(self->buffer, NULL, 10);
}

static CtDigit lexBinDigit(CtLexer* self)
{
    int c = streamPeek(self->stream);

    if (c != '0' && c != '1' && c != '_')
    {
        /* error */
    }

    bufReset(self, (char)c);

    while (1)
    {
        if (c == '0' || c == '1')
        {
            bufPush(self, (char)streamNext(self->stream));
        }
        else if (c == '_')
        {
            streamNext(self->stream);
        }
        else
        {
            break;
        }

        c = streamPeek(self->stream);
    }

    return strtoul(self->buffer, NULL, 2);
}

static CtDigit lexHexDigit(CtLexer* self)
{
    int c = streamPeek(self->stream);

    if (!isxdigit(c) && c != '_')
    {
        /* error */
    }

    bufReset(self, (char)c);

    while (1)
    {
        if (isxdigit(c))
        {
            bufPush(self, (char)streamNext(self->stream));
        }
        else if (c == '_')
        {
            streamNext(self->stream);
        }
        else
        {
            break;
        }

        c = streamPeek(self->stream);
    }


    return strtoul(self->buffer, NULL, 16);
}

static CtDigit lexDigit0(CtLexer* self)
{
    if (streamConsume(self->stream, 'b'))
    {
        /* binary digit */
        return lexBinDigit(self);
    }
    else if (streamConsume(self->stream, 'x'))
    {
        /* hex digit */
        return lexHexDigit(self);
    }
    else
    {
        /* normal digit */
        return lexDigit(self, '0');
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
        /* will be either a keyword or an ident */
        lexIdent(self, c, &tok);
    }
    else if (c == '"')
    {
        /* is a string */
        tok.data.string = lexString(self);
    }
    /*else if (c == '\'')
    {
        for now we dont support char literals
        who needs those anyway
        is a char literal
        tok.data.letter = lexChar(self);
    }*/
    else if (c == '0')
    {
        /* special path for hex and binary numbers */
        tok.data.digit = lexDigit0(self);

        if (errno == ERANGE)
        {
            /* number too big */
        }
    }
    else if (isdigit(c))
    {
        /* is a number */
        tok.data.digit = lexDigit(self, c);

        if (errno == ERANGE)
        {
            /* number too big */
        }
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
