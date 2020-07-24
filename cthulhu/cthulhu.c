#include "cthulhu.h"

#include <ctype.h>
#include <string.h>

#if !defined(CT_MALLOC) || !defined(CT_REALLOC) || !defined(CT_FREE)
#   error "CT_MALLOC, CT_REALLOC, and CT_FREE must be defined"
#endif

static int isident1(int c) { return isalpha(c) || c == '_'; }
static int isident2(int c) { return isalnum(c) || c == '_'; }

static CtBuffer bufferNew(size_t size)
{
    CtBuffer self = {
        .ptr = CT_MALLOC(size),
        .len = 0,
        .alloc = size
    };

    return self;
}

static void bufferPush(CtBuffer *self, int c)
{
    if (self->len + 1 >= self->alloc)
    {
        self->alloc *= 2;
        self->ptr = CT_REALLOC(self->ptr, self->alloc);
    }

    self->ptr[self->len++] = c;
    self->ptr[self->len] = 0;
}

static CtError err(CtErrorKind type, CtOffset pos)
{
    CtError error = {
        .type = type,
        .pos = pos
    };

    return error;
}

static void report(CtState *self, CtError err)
{
    if (self->err_idx < self->max_errs)
        self->errs[self->err_idx++] = err;
}

static int lexNext(CtState *self)
{
    int c = self->ahead;
    self->ahead = self->next(self->stream);

    bufferPush(&self->source, c);

    self->pos.dist++;
    self->len++;

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

static int lexPeek(CtState *self)
{
    return self->ahead;
}

static int lexConsume(CtState *self, int c)
{
    if (lexPeek(self) == c)
    {
        lexNext(self);
        return 1;
    }
    return 0;
}

static int lexSkip(CtState *self)
{
    int c = lexNext(self);

    while (1)
    {
        if (c == '#')
        {
            while (lexPeek(self) != '\n')
                lexNext(self);

            c = lexSkip(self);
        }
        else if (isspace(c))
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

static size_t lexOff(CtState *self)
{
    return self->pos.dist;
}

struct CtKeyEntry { const char *str; int key; int flags; };

static struct CtKeyEntry keys[] = {
#define KEY(id, str, flags) { str, id, flags },
#include "keys.inc"
    { "", K_INVALID, 0 }
};

#define NUM_KEYS (sizeof(keys) / sizeof(struct CtKeyEntry))

static const char *lexView(CtState *self, size_t off)
{
    return self->source.ptr + off;
}

static void lexIdent(CtState *self, CtToken *tok)
{
    size_t off = lexOff(self) - 1;
    while (isident2(lexPeek(self)))
        lexNext(self);

    for (size_t i = 0; i < NUM_KEYS; i++)
    {
        if (
            /**
             * comparing an extra byte is safe because of the null terminator
             * it also makes sure stuff like `a` doesnt match `alias`
             * (the first letter of both is `a`)
             */
            memcmp(lexView(self, off), keys[i].str, self->len + 1) == 0
            && keys[i].flags & self->flags
        )
        {
            tok->type = TK_KEY;
            tok->data.key = keys[i].key;
            return;
        }
    }

    tok->type = TK_IDENT;
    tok->data.ident.len = self->len;
    tok->data.ident.offset = off;
}

static void lexSymbol(CtState *self, CtToken *tok, int c)
{
    tok->type = TK_KEY;

    switch (c)
    {
    case '!':
        if (lexConsume(self, '<'))
        {
            self->depth++;
            tok->data.key = K_TBEGIN;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_NEQ : K_NOT;
        }
        break;

    case '>':
        if (self->depth)
        {
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
        if (lexConsume(self, '<'))
        {
            tok->data.key = lexConsume(self, '=') ? K_SHLEQ : K_SHL;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_GTE : K_GT;
        }
        break;
    case '*':
        tok->data.key = lexConsume(self, '=') ? K_MULEQ : K_MUL;
        break;
    case '/':
        tok->data.key = lexConsume(self, '=') ? K_DIVEQ : K_DIV;
        break;
    case '%':
        tok->data.key = lexConsume(self, '=') ? K_MODEQ : K_MOD;
        break;
    case '^':
        tok->data.key = lexConsume(self, '=') ? K_XOREQ : K_XOR;
        break;
    case '&':
        if (lexConsume(self, '&'))
        {
            tok->data.key = K_AND;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_BITANDEQ : K_BITAND;
        }
        break;
    case '|':
        if (lexConsume(self, '|'))
        {
            tok->data.key = K_OR;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_BITOREQ : K_BITOR;
        }
        break;
    case '@':
        tok->data.key = K_AT;
        break;
    case '?':
        tok->data.key = K_QUESTION;
        break;
    case '~':
        tok->data.key = K_BITNOT;
        break;
    case '(':
        tok->data.key = K_LPAREN;
        break;
    case ')':
        tok->data.key = K_RPAREN;
        break;
    case '[':
        tok->data.key = K_LSQUARE;
        break;
    case ']':
        tok->data.key = K_RSQUARE;
        break;
    case '{':
        tok->data.key = K_LBRACE;
        break;
    case '}':
        tok->data.key = K_RBRACE;
        break;

    case '=':
        if (lexConsume(self, '>'))
        {
            tok->data.key = K_ARROW;
        }
        else
        {
            tok->data.key = lexConsume(self, '=') ? K_EQ : K_ASSIGN;
        }
        break;

    case ':':
        tok->data.key = lexConsume(self, ':') ? K_COLON2 : K_COLON;
        break;
    case '+':
        tok->data.key = lexConsume(self, '=') ? K_ADDEQ : K_ADD;
        break;
    case '-':
        if (lexConsume(self, '>'))
        {
            tok->data.key = K_PTR;
        }
        else 
        {
            tok->data.key = lexConsume(self, '=') ? K_SUBEQ : K_SUB;
        }
        break;
    case '.':
        tok->data.key = K_DOT;
        break;
    case ',':
        tok->data.key = K_COMMA;
        break;
    case ';':
        tok->data.key = K_SEMI;
        break;

    default:
        tok->data.key = K_INVALID;
        report(self, err(ERR_INVALID_SYMBOL, self->pos));
        break;
    }
}

static CtToken lexToken(CtState *self)
{
    int c = lexSkip(self);
    self->len = 1;

    CtToken tok;

    if (c == -1)
    {
        tok.type = TK_END;
    }
    else if (isident1(c))
    {
        lexIdent(self, &tok);
    }
    else if (isdigit(c))
    {

    }
    else
    {
        lexSymbol(self, &tok, c);
    }

    return tok;
}

/**
 * public api
 */

void ctStateNew(
    CtState *self,
    void *stream,
    CtNextFunc next,
    const char *name,
    size_t max_errs
)
{
    (void)lexToken;
    self->stream = stream;
    self->next = next;
    self->ahead = next(stream);
    self->name = name;

    self->source = bufferNew(0x1000);
    self->strings = bufferNew(0x1000);

    self->pos.source = self;
    self->pos.dist = 0;
    self->pos.col = 0;
    self->pos.line = 0;

    self->flags = LF_DEFAULT;

    self->errs = CT_MALLOC(sizeof(CtError) * max_errs);
    self->max_errs = max_errs;
    self->err_idx = 0;
}
