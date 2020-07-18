#include "cthulhu.h"

#include "printf.c"

#define CT_SNPRINTF snprintf__

/**
 * utils
 */

static int isDigit(int c) { return '0' <= c && c <= '9'; }
static int isXDigit(int c) { return ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F') || isDigit(c); }
static int isLetter(int c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
static int isIdent1(int c) { return isLetter(c) || c == '_'; }
static int isIdent2(int c) { return isIdent1(c) || isDigit(c); }
static int isSpace(int c) { return c == ' ' || c == '\n' || c == '\t' || c == '\v'; }

static void memCopy(void *dst, const void *src, CtSize size)
{
    for (CtSize i = 0; i < size; i++)
        ((char*)dst)[i] = ((const char*)src)[i];
}

static int strEq(const char *lhs, const char *rhs)
{
    while (*lhs && (*lhs == *rhs))
    {
        lhs++;
        rhs++;
    }

    return !(*lhs - *rhs);
}

#define CT_MALLOC(ctx, size) (alloc.alloc(alloc.data, (size)))
#define CT_FREE(ctx, addr) (alloc.dealloc(alloc.data, (addr)))

/**
 * lexer internals
 */

typedef struct {
    char *ptr;
    CtSize len;
    CtSize alloc;
} StrBuffer;

StrBuffer bufNew(CtAllocator alloc, CtSize size)
{
    StrBuffer buf = {
        .ptr = CT_MALLOC(alloc, size),
        .len = 0,
        .alloc = size
    };

    buf.ptr[0] = '\0';

    return buf;
}

void bufPush(CtAllocator alloc, StrBuffer *buf, char c)
{
    char *temp;

    if (buf->alloc >= buf->len)
    {
        temp = buf->ptr;
        buf->alloc += 32;
        buf->ptr = CT_MALLOC(alloc, buf->alloc + 1);
        memCopy(buf->ptr, temp, buf->len + 1);
        CT_FREE(alloc, temp);
    }

    buf->ptr[buf->len++] = c;
    buf->ptr[buf->len] = '\0';
}

#define BUF_NEW(size) bufNew(self->alloc, size)
#define BUF_PUSH(buf, c) bufPush(self->alloc, buf, (char)c)

static int lexNext(CtLexer *self)
{
    int c = self->ahead;
    self->ahead = self->next(self->data);

    self->len++;
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

static int lexPeek(CtLexer *self)
{
    return self->ahead;
}

static int lexConsume(CtLexer *self, int c)
{
    if (lexPeek(self) == c)
    {
        lexNext(self);
        return 1;
    }

    return 0;
}

static int lexSkip(CtLexer *self)
{
    int c = lexNext(self);

    while (1)
    {
        if (c == '#')
        {
            while (lexPeek(self) != '\n')
                c = lexNext(self);

            c = lexSkip(self);
        }
        else if (isSpace(c))
        {
            c = lexNext(self);
        }
        else
        {
            break;
        }
    }

    return c;
}

/* mom said its my turn on the unreadable macro */
#define LEX_COLLECT(limit, c, pattern, ...) { int i = 0; while (1) { \
    c = lexPeek(self); if (pattern) { __VA_ARGS__ lexNext(self); if (i++ > limit) { /* overflow */ break; } } else { break; } } }

static CtInt lexBase16(CtLexer *self)
{
    CtInt out = 0;
    int c;

    LEX_COLLECT(16, c, isXDigit(c), {
        uint8_t n = (uint8_t)c;
        uint64_t v = ((n & 0xF) + (n >> 6)) | ((n >> 3) & 0x8);
        out = (out << 4) | (uint64_t)v;
    })

    return out;
}

static CtInt lexBase10(CtLexer *self, int c)
{
    CtInt out = (CtInt)(c - '0');

    LEX_COLLECT(22, c, isDigit(c), {
        out = (out * 10) + ((uint8_t)c - '0');
    })

    return out;
}

static CtInt lexBase2(CtLexer *self)
{
    CtInt out = 0;
    int c;

    LEX_COLLECT(64, c, (c == '0' || c == '1'), {
        out *= 2;
        out += c == '1';
    })

    return out;
}

static void lexDigit(CtLexer *self, CtToken *tok, int c)
{
    CtDigit digit;

    if (c == '0')
    {
        if (lexConsume(self, 'x'))
        {
            digit.num = lexBase16(self);
            digit.enc = DE_BASE16;
        }
        else if (lexConsume(self, 'b'))
        {
            digit.num = lexBase2(self);
            digit.enc = DE_BASE2;
        }
        else if (isDigit(lexPeek(self)))
        {
            digit.num = lexBase10(self, lexNext(self));
            digit.enc = DE_BASE10;
        }
        else
        {
            digit.enc = DE_BASE10;
            digit.num = 0;
        }
    }
    else
    {
        digit.num = lexBase10(self, c);
        digit.enc = DE_BASE10;
    }

    if (isIdent1(lexPeek(self)))
    {
        StrBuffer buf = BUF_NEW(16);
        BUF_PUSH(&buf, lexNext(self));
        while (isIdent2(lexPeek(self)))
            BUF_PUSH(&buf, lexNext(self));

        digit.suffix = buf.ptr;
    }
    else
    {
        digit.suffix = NULL;
    }

    tok->kind = TK_INT;
    tok->data.num = digit;
}

struct CtKeyEntry { const char *str; CtKey key; };

static struct CtKeyEntry key_table[] = {
#define KEY(id, str) { str, id },
#include "keys.inc"
    { "", K_INVALID }
};

#define KEY_TABLE_SIZE (sizeof(key_table) / sizeof(struct CtKeyEntry))

static void lexIdent(CtLexer *self, CtToken *tok, int c)
{
    StrBuffer buf = BUF_NEW(16);

    if (c)
        BUF_PUSH(&buf, c);

    while (isIdent2(lexPeek(self)))
        BUF_PUSH(&buf, lexNext(self));

    for (CtSize i = 0; i < KEY_TABLE_SIZE; i++)
    {
        if (strEq(buf.ptr, key_table[i].str))
        {
            tok->kind = TK_KEYWORD;
            tok->data.key = key_table[i].key;
            return;
        }
    }

    for (CtSize i = 0; i < self->nkeys; i++)
    {
        if (strEq(buf.ptr, self->ukeys[i].str))
        {
            tok->kind = TK_USER_KEYWORD;
            tok->data.ukey = self->ukeys[i].id;
            return;
        }
    }

    tok->kind = TK_IDENT;
    tok->data.ident = buf.ptr;
}

typedef enum {
    /* found a newline */
    CR_NL,
    /* invalid escape */
    CR_ERR,
    /* found the end of the string */
    CR_END,
    /* just keep going */
    CR_CONTINUE
} CharResult;

static char lexEscapeChar(CtLexer *self)
{
    int c = lexNext(self);

    switch (c)
    {
    case '\\': return '\\';
    case 'n': return '\n';
    case 'v': return '\v';
    case 't': return '\t';
    case '0': return '\0';
    case '\'': return '\'';
    case '"': return '"';
    default:
        /* error */
        return c;
    }
}

static CharResult lexSingleChar(CtLexer *self, char *out)
{
    int c = lexNext(self);

    switch (c)
    {
    case '\n': return CR_NL;
    case -1: return CR_END;
    case '"': return CR_END;
    case '\\':
        *out = lexEscapeChar(self);
        return CR_CONTINUE;
    default:
        *out = (char)c;
        return CR_CONTINUE;
    }
}

static void lexMultiString(CtLexer *self, CtToken *tok)
{
    StrBuffer buf = BUF_NEW(64);

    while (1)
    {
        char c;
        CharResult res = lexSingleChar(self, &c);

        if (res == CR_END)
            break;

        /* TODO: handle error */

        BUF_PUSH(&buf, c);
    }

    tok->kind = TK_STRING;
    tok->data.str.len = buf.len;
    tok->data.str.str = buf.ptr;
}

static void lexSingleString(CtLexer *self, CtToken *tok)
{
    StrBuffer buf = BUF_NEW(32);

    while (1)
    {
        char c;
        CharResult res = lexSingleChar(self, &c);

        if (res == CR_END)
            break;

        /* TODO: handle error */

        BUF_PUSH(&buf, c);
    }

    tok->kind = TK_STRING;
    tok->data.str.len = buf.len;
    tok->data.str.str = buf.ptr;
}

static void lexChar(CtLexer *self, CtToken *tok)
{
    tok->kind = TK_CHAR;
    char c;

    CharResult res = lexSingleChar(self, &c);

    if (lexNext(self) != '\'')
    {
        (void)res;
        /* error */
    }

    tok->data.letter = c;
}

#define OP_CASE(sym, op) case sym: tok->data.key = op; break;
#define OP_CASE2(sym, next, lhs, rhs) OP_CASE(sym, lexConsume(self, next) ? lhs : rhs)

static void lexSymbol(CtLexer *self, CtToken *tok, int c)
{
    tok->kind = TK_KEYWORD;

    switch (c)
    {
    case '!':
        if (lexConsume(self, '<'))
        {
            /* context sensitive stuff for templates */
            self->depth++;
            tok->data.key = K_TBEGIN;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_NEQ : K_NOT;
        }
        break;
    case '>':
        /**
         * we use self->depth here because templates require context
         * sensitive lexing
         *
         * while this does mean that >>= becomes > >= instead of >> =
         * thats hardly the end of the world
         *
         * NOTE: the self->depth == 0 check has to be first due to
         * short circuiting rules
         */
        if (self->depth)
        {
            /* inside a template */
            self->depth--;
            tok->data.key = K_TEND;
        }
        else if (lexConsume(self, '>'))
        {
            tok->data.key = lexConsume(self, '=') ? K_SHREQ : K_SHR;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_LTE : K_LT;
        }
        break;
    case '<':
        if (lexConsume(self, '|'))
            tok->data.key = K_STREAM;
        else if (lexConsume(self, '<'))
            tok->data.key = lexConsume(self, '=') ? K_SHLEQ : K_SHL;
        else
            tok->data.key = lexConsume(self, '=') ? K_GTE : K_GT;
        break;
    case '=':
        tok->data.key = lexConsume(self, '=')
            ? K_EQ : lexConsume(self, '>') ? K_BIGARROW : K_ASSIGN;
        break;
    case '&':
        tok->data.key = lexConsume(self, '&')
            ? K_AND : lexConsume(self, '=') ? K_BITANDEQ : K_BITAND;
        break;
    case '|':
        tok->data.key = lexConsume(self, '|')
            ? K_OR : lexConsume(self, '=') ? K_BITOREQ : K_BITOR;
        break;
    case '-':
        tok->data.key = lexConsume(self, '>')
            ? K_ARROW : lexConsume(self, '=') ? K_SUBEQ : K_SUB;
        break;
    OP_CASE2('+', '=', K_ADDEQ, K_ADD)
    OP_CASE2('*', '=', K_MULEQ, K_MUL)
    OP_CASE2('/', '=', K_DIVEQ, K_DIV)
    OP_CASE2('%', '=', K_MODEQ, K_MOD)
    OP_CASE2('^', '=', K_XOREQ, K_XOR)
    OP_CASE('~', K_BITNOT)
    OP_CASE('.', K_DOT)
    OP_CASE(',', K_COMMA)
    OP_CASE('?', K_QUESTION)
    OP_CASE(';', K_SEMI)
    OP_CASE('@', K_AT)
    OP_CASE('[', K_LSQUARE)
    OP_CASE(']', K_RSQUARE)
    OP_CASE('(', K_LPAREN)
    OP_CASE(')', K_RPAREN)
    OP_CASE('{', K_LPAREN)
    OP_CASE('}', K_RBRACE)
    OP_CASE2(':', ':', K_COLON2, K_COLON)
    default:
        /* error */
        break;
    }
}

#undef OP_CASE
#undef OP_CASE2

/**
 * public api
 */

void ctSetKeys(CtLexer *self, const CtUserKeyword *keys, CtSize num)
{
    self->ukeys = keys;
    self->nkeys = num;
}

void ctResetKeys(CtLexer *self)
{
    self->ukeys = NULL;
    self->nkeys = 0;
}

CtLexer ctLexerNew(void *data, CtLexerNextFunc next, CtAllocator alloc)
{
    CtLexer self = {
        .alloc = alloc,
        .depth = 0,
        .nkeys = 0,
        .ukeys = NULL,
        .next = next,
        .data = data,
        .ahead = next(data),
        .len = 0
    };

    CtPosition pos = {
        .dist = 1,
    };

    if (self.ahead == '\n')
    {
        pos.col = 0;
        pos.line = 1;
    }
    else
    {
        pos.col = 1;
        pos.line = 0;
    }

    self.pos = pos;

    return self;
}

CtToken ctLexerNext(CtLexer *self)
{
    CtToken tok;

    int c = lexSkip(self);

    self->len = 0;

    tok.pos = self->pos;

    if (c == -1)
    {
        tok.kind = TK_EOF;
    }
    else if (isDigit(c))
    {
        lexDigit(self, &tok, c);
    }
    else if (c == 'R')
    {
        /* multiline string perhaps */
        if (lexConsume(self, '"'))
        {
            /* it is */
            lexMultiString(self, &tok);
        }
        else
        {
            /* nope */
            lexIdent(self, &tok, 'R');
        }
    }
    else if (isIdent1(c))
    {
        lexIdent(self, &tok, c);
    }
    else if (c == '"')
    {
        lexSingleString(self, &tok);
    }
    else if (c == '\'')
    {
        lexChar(self, &tok);
    }
    else
    {
        lexSymbol(self, &tok, c);
    }

    tok.len = self->len;

    return tok;
}

void ctFreeToken(CtToken tok, CtAllocator alloc)
{
    switch (tok.kind)
    {
    case TK_IDENT:
        alloc.dealloc(alloc.data, tok.data.ident);
        break;
    case TK_STRING:
        alloc.dealloc(alloc.data, tok.data.str.str);
        break;
    case TK_INT:
        if (tok.data.num.suffix)
            alloc.dealloc(alloc.data, tok.data.num.suffix);
        break;
    default:
        break;
    }
}

void ctFormatToken(CtToken tok, char *buf, CtSize *len)
{
    switch (tok.kind)
    {
    case TK_EOF:
        *len = CT_SNPRINTF(buf, *len, "EOF");
        break;
        /* TODO */
    default:
        break;
    }
}
