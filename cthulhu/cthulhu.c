#include "cthulhu.h"

#if !defined(CT_MALLOC) || !defined(CT_FREE)
#   error "CT_MALLOC and CT_FREE must be defined"
#endif

#include <string.h>
#include <ctype.h>

/**
 * util functions
 */

CtBuffer bufferNew(CtSize size)
{
    CtBuffer buf;

    buf.ptr = CT_MALLOC(size);
    buf.ptr[0] = 0;
    buf.len = 0;
    buf.alloc = size;

    return buf;
}

void bufferPush(CtBuffer *self, int c)
{
    if (self->alloc >= self->len)
    {
        char *temp = self->ptr;
        self->alloc += 0x1000;
        self->ptr = CT_MALLOC(self->alloc + 1);
        memcpy(self->ptr, temp, self->len + 1);
        CT_FREE(temp);
    }

    self->ptr[self->len++] = c;
    self->ptr[self->len] = 0;
}

/**
 * lexer internals
 */

static int lexNext(CtLexer *self)
{
    int c = self->ahead;
    self->ahead = self->next(self->stream);

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

static void lexIdent(CtLexer *self, CtToken *tok, int c)
{

}

static void lexMultiString(CtLexer *self, CtToken *tok)
{

}

static void lexSingleString(CtLexer *self, CtToken *tok)
{
    
}

/**
 * public API
 */

CtLexer ctlexerNew(void *stream, CtNextFunc next)
{
    CtLexer self;

    self.stream = stream;
    self.next = next;
    self.ahead = next(stream);

    self.pos.dist = 0;
    self.pos.col = 0;
    self.pos.line = 0;

    /* setup out buffers */
    self.strings = bufferNew(0x1000);
    self.source = bufferNew(0x1000);
    bufferPush(&self.source, self.ahead);

    return self;
}

void ctLexerDelete(CtLexer *self)
{
    CT_FREE(self->source.ptr);
}

CtToken ctLexerNext(CtLexer *self)
{
    int c = lexSkip(self);

    CtToken tok;

    tok.pos = self->pos;
    self->len = 0;

    if (c == -1)
    {
        tok.kind = TK_EOF;
    }
    else if (c == 'r' || c == 'R')
    {
        /* might be a multiline string */
        if (lexConsume(self, '"'))
        {
            /* is a multiline string */
            lexMultiString(self, &tok);
        }
        else
        {
            lexIdent(self, &tok, c);
        }
    }

    tok.len = self->len;

    return tok;
}

#if 0

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

/**
 * lexer internals
 */

CtStrBuffer bufNew(CtSize size)
{
    CtStrBuffer buf = {
        .ptr = CT_MALLOC(size),
        .len = 0,
        .alloc = size
    };

    buf.ptr[0] = '\0';

    return buf;
}

void bufPush(CtStrBuffer *buf, char c)
{
    char *temp;

    if (buf->alloc >= buf->len)
    {
        temp = buf->ptr;
        buf->alloc += 32;
        buf->ptr = CT_MALLOC(buf->alloc + 1);
        memCopy(buf->ptr, temp, buf->len + 1);
        CT_FREE(temp);
    }

    buf->ptr[buf->len++] = c;
    buf->ptr[buf->len] = '\0';
}

/**
 * get the current offset in source
 */
const char *lexCur(CtLexer *self)
{
    return self->source.ptr + self->pos.dist;
}

static int lexNext(CtLexer *self)
{
    int c = self->ahead;
    self->ahead = self->next(self->stream);

    bufPush(&self->source, c);

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
    c = lexPeek(self); if (pattern) { __VA_ARGS__ lexNext(self); if (i++ > limit) { self->err = ERR_OVERFLOW; } } else { break; } } }

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

#define BASE10_LIMIT (18446744073709551615ULL % 10)
#define BASE10_CUTOFF (18446744073709551615ULL / 10)

static CtInt lexBase10(CtLexer *self, int c)
{
    CtInt out = (CtInt)(c - '0');

    /* we handle overflow differently here due to how base10 works */
    while (1)
    {
        c = lexPeek(self);
        if (isDigit(c))
        {
            CtInt n = (CtInt)(c - '0');
            if (out > BASE10_CUTOFF || (out == BASE10_CUTOFF && n > BASE10_LIMIT))
                self->err = ERR_OVERFLOW;

            out = (out * 10) + (c - '0');
        }
        else
        {
            break;
        }

        lexNext(self);
    }

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
        const char *suffix = lexCur(self);
        CtSize len = 1;
        while (isIdent2(lexPeek(self)))
        {
            lexNext(self);
            len++;
        }

        digit.suffix.len = len;
        digit.suffix.str = suffix;
    }
    else
    {
        digit.suffix.len = 0;
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
    CtStrBuffer buf = bufNew(16);
    bufPush(&buf, c);

    while (isIdent2(lexPeek(self)))
        bufPush(&buf, lexNext(self));

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
        self->err = ERR_INVALID_ESCAPE;
        return c;
    }
}

static CharResult lexSingleChar(CtLexer *self, char *out)
{
    int c = lexNext(self);

    switch (c)
    {
    case '\n':
        *out = '\n';
        return CR_NL;
    case -1:
        self->err = ERR_EOF;
        return CR_END;
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
    const char *str = self->source.ptr;
    CtSize len = 0;

    while (1)
    {
        char c;
        CharResult res = lexSingleChar(self, &c);

        if (res != CR_CONTINUE && res != CR_NL)
            break;

        len++;
    }

    tok->kind = TK_STRING;
    tok->data.str.len = len | CT_MULTILINE_FLAG;
    tok->data.str.str = str;
}

static void lexSingleString(CtLexer *self, CtToken *tok)
{
    const char *str = self->source.ptr;
    CtSize len = 0;

    while (1)
    {
        char c;
        CharResult res = lexSingleChar(self, &c);

        if (res == CR_END)
            break;

        if (res == CR_NL)
            self->err = ERR_NEWLINE;

        len++;
    }

    tok->kind = TK_STRING;
    tok->data.str.len = len;
    tok->data.str.str = str;
}

static void lexChar(CtLexer *self, CtToken *tok)
{
    tok->kind = TK_CHAR;
    char c;

    lexSingleChar(self, &c);

    if (lexNext(self) != '\'')
        self->err = ERR_UNTERMINATED;

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
        self->err = ERR_INVALID_SYMBOL;
        break;
    }
}

#undef OP_CASE
#undef OP_CASE2

/**
 * parser internals
 */

static CtNode *parseExpr(CtParser *self);
static CtNode *parsePrimary(CtParser *self);
static CtNode *parseType(CtParser* self);

static CtToken parseNext(CtParser *self)
{
    CtToken tok = self->tok;

    self->tok.kind = TK_LOOKAHEAD;

    if (tok.kind == TK_LOOKAHEAD)
    {
        tok = ctLexerNext(&self->lex);
    }

    if (tok.kind == TK_ERROR)
    {
        self->err = ERR_UNEXPECTED;
        self->errt = tok;
    }

    return tok;
}

static CtToken parsePeek(CtParser *self)
{
    CtToken tok = parseNext(self);
    self->tok = tok;
    return tok;
}

static int parseConsumeKey(CtParser *self, CtKey key)
{
    CtToken tok = parseNext(self);

    if (tok.kind == TK_KEYWORD && tok.data.key == key)
    {
        return 1;
    }

    self->tok = tok;
    return 0;
}

static void parseExpect(CtParser *self, CtKey key)
{
    CtToken tok = parseNext(self);

    if (tok.kind != TK_KEYWORD || tok.data.key != key)
    {
        self->errt = tok;
        self->err = ERR_UNEXPECTED;

        /* if we get a bad keyword, set the error then consume the rest of the statement */
        do tok = parseNext(self); while (tok.kind != TK_KEYWORD || tok.data.key != K_SEMI);

        /* clear the template depth */
        self->lex.depth = 0;
    }
}

static CtNode *nodeNew(CtNodeType type)
{
    CtNode *node = CT_MALLOC(sizeof(CtNode));
    node->type = type;
    return node;
}

static CtNode *nodeFrom(CtNodeType type, CtToken tok)
{
    CtNode *node = nodeNew(type);
    node->tok = tok;
    return node;
}

static CtNodeArray arrNew(CtSize size)
{
    CtNodeArray arr = {
        .data = CT_MALLOC(sizeof(CtNode) * size),
        .len = 0,
        .alloc = size
    };

    return arr;
}

static CtNodeArray emptyArr()
{
    CtNodeArray arr = {
        .data = NULL,
        .len = 0,
        .alloc = 0
    };

    return arr;
}

static void arrAdd(CtNodeArray *self, CtNode *node)
{
    if (self->alloc >= self->len + 1)
    {
        self->alloc += 4;
        CtNode *temp = CT_MALLOC(sizeof(CtNode) * self->alloc);
        memCopy(temp, self->data, self->len * sizeof(CtNode));
        CT_FREE(self->data);
        self->data = temp;
    }

    self->data[self->len++] = *node;
}

/**
 * parse an array of at least one item
 * seperated by sep and terminated by end
 */
static CtNodeArray parseUntil(CtParser *self, CtNode*(*func)(CtParser*), CtKey sep, CtKey end)
{
    CtNodeArray arr = arrNew(4);

    arrAdd(&arr, func(self));

    while (!parseConsumeKey(self, end))
    {
        parseExpect(self, sep);
        arrAdd(&arr, func(self));
    }

    return arr;
}

static CtNodeArray parseCollect(CtParser *self, CtNode*(*func)(CtParser*), CtKey sep)
{
    CtNodeArray arr = arrNew(4);

    do { arrAdd(&arr, func(self)); } while (parseConsumeKey(self, sep));

    return arr;
}

static CtNode *parseIdent(CtParser *self)
{
    CtToken tok = parseNext(self);

    if (tok.kind != TK_IDENT)
    {
        self->err = ERR_UNEXPECTED;
        self->errt = tok;
        return NULL;
    }
    else
    {
        CtNode *node = nodeFrom(NT_IDENT, tok);
        node->data.ident = tok.data.ident;
        return node;
    }
}

static CtNode *parseTParam(CtParser *self)
{
    CtNode *node = nodeNew(NT_TPARAM);

    /* :Ident = type */
    if (parseConsumeKey(self, K_COLON))
    {
        node->data.tparam.name = parseIdent(self);
        parseExpect(self, K_ASSIGN);
    }
    else
    {
        /* type */
        node->data.tparam.name = NULL;
    }

    node->data.tparam.type = parseType(self);

    return node;
}

static CtNode *parseQualType(CtParser *self)
{
    CtNode *qual = nodeNew(NT_QUAL);
    qual->data.qual.name = parseIdent(self);
    qual->data.qual.params = parseConsumeKey(self, K_TBEGIN)
        ? parseUntil(self, parseTParam, K_COMMA, K_TEND)
        : emptyArr();

    return qual;
}

static CtNode *parseQualTypes(CtParser *self)
{
    CtNode *node = nodeNew(NT_QUALS);
    node->data.quals = parseCollect(self, parseQualType, K_COLON2);
    return node;
}

static CtNode *parseArray(CtParser *self)
{
    CtNode *node = nodeNew(NT_ARR);
    node->data.arr.type = parseType(self);
    node->data.arr.size = parseConsumeKey(self, K_COLON)
        ? parseExpr(self) : NULL;

    parseExpect(self, K_RSQUARE);

    return node;
}

static CtNode *parseClosure(CtParser *self)
{
    CtNodeArray args = parseConsumeKey(self, K_LPAREN)
        ? parseUntil(self, parseType, K_COMMA, K_RPAREN)
        : emptyArr();
    CtNode *result = parseConsumeKey(self, K_ARROW)
        ? parseType(self) : NULL;

    CtNode *node = nodeNew(NT_CLOSURE);
    node->data.closure.args = args;
    node->data.closure.result = result;

    return node;
}

static CtNode *parseType(CtParser *self)
{
    CtToken tok = parseNext(self);
    CtNode *node = NULL;

    if (tok.kind == TK_IDENT)
    {
        /* qualified typenames */

        /* put the token back so parseQualTypes works properly */
        self->tok = tok;
        node = parseQualTypes(self);
    }
    else if (tok.kind == TK_KEYWORD)
    {
        switch (tok.data.key)
        {
        case K_MUL:
            /* pointer */
            node = nodeFrom(NT_PTR, tok);
            node->data.type = parseType(self);
            break;
        case K_BITAND:
            /* reference */
            node = nodeFrom(NT_REF, tok);
            node->data.type = parseType(self);
            break;
        case K_LSQUARE:
            /* array */
            node = parseArray(self);
            break;
        case K_DEF:
            /* closure */
            node = parseClosure(self);
            break;
        default:
            /* error */
            break;
        }
    }

    return node;
}

typedef enum {
    OP_ERROR = 0,

    OP_ASSIGN = 1,
    OP_TERNARY = 2,
    OP_LOGIC = 3,
    OP_EQUAL = 4,
    OP_COMPARE = 5,
    OP_BITS = 6,
    OP_SHIFT = 7,
    OP_ADD = 8,
    OP_MUL = 9
} CtOpPrec;

static CtOpPrec binopPrec(CtToken key)
{
    if (key.kind != TK_KEYWORD)
        return TK_ERROR;

    switch (key.data.key)
    {
    case K_ASSIGN: case K_ADDEQ: case K_SUBEQ:
    case K_MULEQ: case K_DIVEQ: case K_MODEQ:
    case K_SHLEQ: case K_SHREQ: case K_XOREQ:
    case K_BITANDEQ: case K_BITOREQ:
        return OP_ASSIGN;
    case K_QUESTION:
        return OP_TERNARY;
    case K_AND: case K_OR:
        return OP_LOGIC;
    case K_EQ: case K_NEQ:
        return OP_EQUAL;
    case K_GT: case K_GTE: case K_LT: case K_LTE:
        return OP_COMPARE;
    case K_XOR: case K_BITAND: case K_BITOR:
        return OP_BITS;
    case K_SHL: case K_SHR:
        return OP_SHIFT;
    case K_ADD: case K_SUB:
        return OP_ADD;
    case K_MUL: case K_DIV: case K_MOD:
        return OP_MUL;

    default:
        return OP_ERROR;
    }
}

#define VALID_UNARY(expr) ((expr) == K_ADD || (expr) == K_SUB || (expr) == K_NOT || (expr) == K_AND || (expr) == K_MUL)

static CtNode *parseUnary(CtParser *self)
{
    CtToken tok = parsePeek(self);
    CtNode *node;

    if (tok.data.key == K_LPAREN)
    {
        node = parseExpr(self);
        parseExpect(self, K_RPAREN);
    }
    else if (VALID_UNARY(tok.data.key))
    {
        node = nodeFrom(NT_UNARY, parseNext(self));
        node->data.unary.op = tok.data.key;
        node->data.unary.expr = parsePrimary(self);
    }
    else
    {
        node = NULL;
    }

    return node;
}

static CtNode *parsePrimary(CtParser *self)
{
    CtToken tok = parsePeek(self);
    CtNode *node;

    switch (tok.kind)
    {
    case TK_KEYWORD:
        node = parseUnary(self);
        break;
    case TK_IDENT:
        node = parseType(self);
        node->type = NT_NAME;
        break;
    case TK_INT: case TK_STRING: case TK_CHAR:
        node = nodeFrom(NT_LITERAL, parseNext(self));
        break;
    default:
        self->err = ERR_UNEXPECTED;
        self->errt = tok;
        node = NULL;
        break;
    }

    if (!node)
        return NULL;

    while (1)
    {
        if (parseConsumeKey(self, K_DOT))
        {
            CtNode *access = nodeNew(NT_ACCESS);
            access->data.access.expr = node;
            access->data.access.field = parseIdent(self);
            node = access;
        }
        else if (parseConsumeKey(self, K_ARROW))
        {
            CtNode *access = nodeNew(NT_DEREF);
            access->data.access.expr = node;
            access->data.access.field = parseIdent(self);
            node = access;
        }
        else if (parseConsumeKey(self, K_LSQUARE))
        {
            CtNode *subscript = nodeNew(NT_SUBSCRIPT);
            subscript->data.subscript.expr = node;
            subscript->data.subscript.index = parseExpr(self);
            node = subscript;
        }
        else
        {
            break;
        }
    }

    return node;
}

static CtNode *makeBinary(CtNode *lhs, CtNode *rhs, CtToken tok)
{
    CtNode *node = nodeFrom(NT_BINARY, tok);
    node->data.binary.op = tok.data.key;
    node->data.binary.lhs = lhs;
    node->data.binary.rhs = rhs;
    return node;
}

static CtNode *parseBinary(CtParser *self, CtNode *lhs, CtOpPrec minprec)
{
    CtToken ahead = parsePeek(self);

    if (binopPrec(ahead) == OP_TERNARY)
    {
        parseNext(self);

        CtNode *node = nodeNew(NT_TERNARY);
        node->data.ternary.cond = lhs;

        if (parseConsumeKey(self, K_COLON))
        {
            node->data.ternary.yes = lhs;
        }
        else
        {
            node->data.ternary.yes = parseExpr(self);
            parseExpect(self, K_COLON);
        }

        node->data.ternary.no = parseExpr(self);

        return node;
    }

    while (binopPrec(ahead) >= minprec)
    {
        CtToken op = parseNext(self);
        CtNode *rhs = parsePrimary(self);
        ahead = parsePeek(self);

        while (binopPrec(ahead) >= binopPrec(op) && binopPrec(ahead) != OP_ERROR)
        {
            rhs = parseBinary(self, rhs, binopPrec(ahead));
            ahead = parsePeek(self);
        }

        lhs = makeBinary(lhs, rhs, op);
    }

    return lhs;
}

static CtNode *parseExpr(CtParser *self)
{
    return parseBinary(self, parsePrimary(self), OP_ASSIGN);
}

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

CtLexer ctLexerNew(void *stream, CtLexerNextFunc next)
{
    CtLexer self = {
        .depth = 0,
        .nkeys = 0,
        .ukeys = NULL,
        .next = next,
        .stream = stream,
        .ahead = next(stream),
        .len = 0,
        .source = bufNew(1024)
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
    self->err = ERR_NONE;

    tok.pos = self->pos;

    if (tok.pos.col > 2)
        tok.pos.col -= 2;

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

    /* add 1 here to account for lookahead token */
    tok.len = self->len + 1;

    if (self->err != ERR_NONE)
    {
        tok.kind = TK_ERROR;
        tok.data.err = self->err;
        self->err = ERR_NONE;
    }

    return tok;
}

void ctLexerFree(CtLexer *self)
{
    CT_FREE(self->source.ptr);
}

CtParser ctParserNew(CtLexer lex)
{
    CtParser parse = {
        .lex = lex,
        .err = ERR_NONE
    };

    parse.tok.kind = TK_LOOKAHEAD;

    return parse;
}

CtNode *ctParseUnit(CtParser *self)
{
    (void)self;
    return NULL;
}

CtNode *ctParseEval(CtParser *self)
{
    CtNode *node = parseExpr(self);

    if (self->err != ERR_NONE)
    {
        ctFreeNode(node);
        node = nodeFrom(NT_ERROR, self->errt);
        node->data.err = self->err;
        self->err = ERR_NONE;
    }

    return node;
}

void ctFreeNode(CtNode *node)
{
    (void)node;
    /* UHHHH */
}

#endif
