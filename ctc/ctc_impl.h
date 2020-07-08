#include "ctc.h"

#include <ctype.h>
#include <string.h>
#include <stdarg.h>

/**
 * config data
 */

#include <stdlib.h>

#ifndef CT_MALLOC
#   define CT_MALLOC(expr) malloc(expr)
#endif

#ifndef CT_FREE
#   define CT_FREE(expr) free(expr)
#endif

#ifndef CT_SPRINTF
#   include "printf/printf.c"
#   define CT_SPRINTF sprintf_
#endif

#ifndef CT_ERROR
#   define CT_ERROR(data, msg)
#endif

/**
 * error reporting
 */

static char* cstrdup(const char* str)
{
    char* out = CT_MALLOC(strlen(str) + 1);
    strcpy(out, str);

    return out;
}

#define CT_FMT(out, fmt) { out = cstrdup(fmt); }
#define CT_FMT_ARGS(out, fmt, ...) { int len = CT_SPRINTF(NULL, fmt, __VA_ARGS__); out = CT_MALLOC((size_t)len + 1); out[len] = '\0'; CT_SPRINTF(out, fmt, __VA_ARGS__); }

static const char* keyStr(CtKeyword key)
{
#define OP(id, str) case id: return str;
#define KEY(id, str) case id: return str;
    switch (key)
    {
#include "keys.inc"
    default: return "invalid";
    }
}

static char* intStr(CtDigit val, CtDigit base, char* buf)
{
	int i = 30;

	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];

	return buf + i + 1;
}

static char* numStr(CtNumber num)
{
    char* out;

    if (num.enc == NE_INT)
    {
        CT_FMT_ARGS(out, "%lu", num.digit)
    }
    else if (num.enc == NE_BIN)
    {
        char bin[64] = { 0 };
        char* temp = intStr(num.digit, 2, bin);

        CT_FMT_ARGS(out, "0b%s", temp)
    }
    else if (num.enc == NE_HEX)
    {
        CT_FMT_ARGS(out, "0x%x", num.digit)
    }
    else
    {
        out = NULL;
    }

    return out;
}

static const char* charStr(const CtChar* c)
{
    switch (*c)
    {
    case '\n': return "\\n";
    case '\0': return "\\0";
    default: return (const char*)c;
    }
}

static char* tokStr(CtToken tok)
{
    char* out;
    char* num;

    switch (tok.kind)
    {
    case TK_EOF: CT_FMT(out, "eof") break;
    case TK_IDENT: CT_FMT_ARGS(out, "identifier: `%s`", tok.data.ident) break;
    case TK_INT:
        num = numStr(tok.data.digit);
        if (tok.data.digit.suffix)
            CT_FMT_ARGS(out, "integer: `%s%s` (%lu)", num, tok.data.digit.suffix, tok.data.digit.digit)
        else
            CT_FMT_ARGS(out, "integer: `%s` (%lu)", num, tok.data.digit.digit)

        if (num)
            CT_FREE(num);

        break;
    case TK_CHAR: CT_FMT_ARGS(out, "character: '%s'", charStr(&tok.data.letter)) break;
    case TK_KEYWORD: CT_FMT_ARGS(out, "keyword: `%s`", keyStr(tok.data.key)) break;
    case TK_STRING:
        /* TODO: handle strings with null bytes in them */
        if (tok.data.string.prefix)
            CT_FMT_ARGS(out, "string: %s\"%s\"", tok.data.string.string, tok.data.string.prefix)
        else
            CT_FMT_ARGS(out, "string: \"%s\"", tok.data.string.string)
        break;
    case TK_ERROR: CT_FMT_ARGS(out, "error: %s", tok.data.reason) break;
    default: CT_FMT(out, "invalid token") break;
    }

    return out;
}

static void tokFree(CtToken tok)
{
    switch (tok.kind)
    {
    case TK_ERROR:
        CT_FREE(tok.data.reason);
        break;
    case TK_IDENT:
        CT_FREE(tok.data.ident);
        break;
    case TK_STRING:
        if (tok.data.string.string)
            CT_FREE(tok.data.string.string);
        if (tok.data.string.prefix)
            CT_FREE(tok.data.string.prefix);
        break;
    case TK_INT:
        if (tok.data.digit.suffix)
            CT_FREE(tok.data.digit.suffix);
        break;
    default:
        break;
    }
}

/**
 * linear string buffer
 */


/* create a new buffer */
static CtBuffer bufNew(size_t size)
{
    CtBuffer out;
    out.ptr = CT_MALLOC(size);
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
        self->ptr = CT_MALLOC(self->alloc + 1);
        strcpy(self->ptr, temp);
        CT_FREE(temp);
    }

    self->ptr[self->len++] = c;
    self->ptr[self->len] = '\0';
}

/**
 * lexer internals
 */

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

static void lexEscapeChar(CtLexer* self, char* out)
{
    int c = streamNext(self->stream);

    switch (c)
    {
    case '\\': *out = '\\'; break;
    case 'n': *out = '\n'; break;
    case 'v': *out = '\v'; break;
    case 't': *out = '\t'; break;
    case '0': *out = '\0'; break;
    case '\'': *out = '\''; break;
    case '"': *out = '"'; break;
    default:
        *out = (char)c;
        CT_FMT_ARGS(self->err, "invalid escaped char %c", *out)
        break;
    }
}

static int lexSingleChar(CtLexer* self, char* out)
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
        lexEscapeChar(self, out);
        return STR_CONTINUE;
    }
    else
    {
        *out = (char)c;
        return STR_CONTINUE;
    }
}

static void lexSingleString(CtLexer* self, CtBuffer* buf)
{
    int res;

    while (1)
    {
        char c;
        res = lexSingleChar(self, &c);
        if (res == STR_NL)
        {
            CT_FMT(self->err, "newline in string")
            break;
        }
        else if (res == STR_ERR)
        {
            CT_FMT(self->err, "EOF in string")
            break;
        }
        else if (res == STR_END)
        {
            break;
        }
        else
        {
            bufPush(buf, c);
        }
    }
}

static void lexMultiString(CtLexer* self, CtBuffer* buf)
{
    int res;

    while (1)
    {
        char c;
        res = lexSingleChar(self, &c);

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
            CT_FMT(self->err, "EOF in string")
            break;
        }
        else if (res == STR_NL)
        {
            /* newlines are fine in multiline strings */
            bufPush(buf, '\n');
        }
        else
        {
            bufPush(buf, c);
        }
    }
}

static CtString lexString(CtLexer* self)
{
    CtString out;
    CtBuffer buf;

    if (streamConsume(self->stream, '"'))
    {
        if (streamConsume(self->stream, '"'))
        {
            /* is a multiline string */
            buf = bufNew(128);
            lexMultiString(self, &buf);
        }
        else
        {
            buf.ptr = NULL;
            buf.len = 0;
        }
    }
    else
    {
        /* is a single line string */
        buf = bufNew(64);
        lexSingleString(self, &buf);
    }

    out.string = buf.ptr;
    out.prefix = NULL;
    out.len = buf.len;

    return out;
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

static void lexIdent(CtLexer* self, CtToken* out, int c)
{
    size_t i;
    CtBuffer buf = bufNew(64);

    if (c)
        bufPush(&buf, (char)c);

    while (isident2(streamPeek(self->stream)))
        bufPush(&buf, (char)streamNext(self->stream));

    /* check if its a keyword */
    for (i = 0; i < KEY_TABLE_SIZE; i++)
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
        out->data.string = lexString(self);
        out->data.string.prefix = buf.ptr;
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
        if (streamConsume(self->stream, '>'))
            out->data.key = K_ARROW;
        else
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
        CT_FMT_ARGS(self->err, "invalid character `%c`", (char)c)
        break;
    }
}

/* base 16 integer */
static CtDigit lexHexDigit(CtLexer* self)
{
    int i = 0;
    int c;
    CtDigit out = 0;
    streamNext(self->stream);

    while (1)
    {
        c = streamPeek(self->stream);

        if (isxdigit(c))
        {
            uint8_t n = (uint8_t)c;
            uint64_t v = (n & 0xF) + (n >> 6) | ((n >> 3) & 0x8);
            out = (out << 4) | (uint64_t)v;

            if (i++ > 16)
            {
                CT_FMT(self->err, "integer overflow")
                break;
            }
        }
        else
        {
            break;
        }

        streamNext(self->stream);
    }

    return out;
}

/* base 2 integer */
static CtDigit lexBinDigit(CtLexer* self)
{
    int i = 0;
    int c;
    CtDigit out = 0;
    streamNext(self->stream);

    while (1)
    {
        c = streamPeek(self->stream);
        if (c == '0' || c == '1')
        {
            out *= 2;
            if (c == '1')
                out += 1;

            streamNext(self->stream);

            if (i++ > 64)
            {
                CT_FMT(self->err, "integer overflow")
                break;
            }
        }
        else
        {
            break;
        }
    }

    return out;
}

/* base 10 integer */
static CtDigit lexDigit1(CtLexer* self, int c)
{
    int i = 0;
    CtDigit out = 0;

    while (1)
    {
        c = streamPeek(self->stream);
        if (isdigit(c))
        {
            out *= (10 + ((uint8_t)c - '0'));
            streamNext(self->stream);

            if (i++ > 22)
            {
                CT_FMT(self->err, "integer overflow")
                break;
            }
        }
        else
        {
            break;
        }
    }

    return out;
}

static CtDigit lexDigit0(CtLexer* self, CtNumberEncoding* enc)
{
    int c = streamPeek(self->stream);

    if (c == 'x')
    {
        /* base 16 */
        *enc = NE_HEX;
        return lexHexDigit(self);
    }
    else if (c == 'b')
    {
        /* base 2 */
        *enc = NE_BIN;
        return lexBinDigit(self);
    }
    else if (isdigit(c))
    {
        /* base 10 */
        *enc = NE_INT;
        return lexDigit1(self, streamNext(self->stream));
    }
    else
    {
        *enc = NE_INT;
        /* just 0 */
        return 0;
    }
}

static CtChar lexChar(CtLexer* self)
{
    CtChar c = 0;
    int res = lexSingleChar(self, (char*)&c);

    if (res == STR_END)
    {
        c = '"';
    }
    else if (res != STR_CONTINUE)
    {
        CT_FMT_ARGS(self->err, "invalid character literal: `%d`", (int)c)
        return c;
    }

    int64_t res2 = streamNext(self->stream);

    if (res != '\'')
        CT_FMT_ARGS(self->err, "expected ' after character literal but got `%s` instead", charStr((CtChar*)&res2))

    return c;
}

static CtNumber lexDigit(CtLexer* self, int c)
{
    CtToken suffix;
    CtNumber num;

    if (c == '0')
    {
        num.digit = lexDigit0(self, &num.enc);
    }
    else
    {
        num.enc = NE_INT;
        num.digit = lexDigit1(self, c);
    }

    /* check for self->err here to prevent memory leaks */
    if (isident1(streamPeek(self->stream)) && !self->err)
    {
        /* we have a digit suffix */
        lexIdent(self, &suffix, '\0');

        if (suffix.kind != TK_IDENT)
        {
            CT_FMT(self->err, "keywords are invalid digit suffixes")
            /* keywords directly after a digit are invalid */
            num.suffix = NULL;
        }
        else
        {
            num.suffix = suffix.data.ident;
        }
    }
    else
    {
        /* no suffix */
        num.suffix = NULL;
    }

    return num;
}

/**
 * parser internals
 */

static CtAST* astNew(CtASTKind kind)
{
    CtAST* self = CT_MALLOC(sizeof(CtAST));
    self->kind = kind;
    return self;
}

static CtToken parseNext(CtParser* self)
{
    CtToken tok = self->tok;

    if (tok.kind == TK_INVALID)
    {
        tok = ctLexNext(self->source);
    }

    self->tok.kind = TK_INVALID;
    return tok;
}

static int parseConsumeKey(CtParser* self, CtKeyword key)
{
    CtToken tok = parseNext(self);

    if (tok.kind == TK_KEYWORD && tok.data.key == key)
    {
        return 1;
    }

    self->tok = tok;
    return 0;
}

/**
 * if (parseExpectKey(self, key))
 * {
 *     // proper parsing
 * }
 * else
 * {
 *    // handle error
 * }
 */
static int parseExpectKey(CtParser* self, CtKeyword key)
{
    CtToken tok = parseNext(self);

    if (tok.kind != TK_KEYWORD || tok.data.key != key)
    {
        printf("oh no\n");
        /* TODO: error */
        return 0;
    }

    return 1;
}

#define PARSE_EXPECT(self, key)

static CtAST* parseExpectIdent(CtParser* self)
{
    CtAST* out;
    CtToken tok = parseNext(self);

    if (tok.kind == TK_IDENT)
    {
        out = astNew(AK_IDENT);
        out->tok = tok;
    }
    else
    {
        out = astNew(AK_ERROR);
        out->tok = tok;

        char* str = tokStr(tok);
        CT_FMT_ARGS(out->data.reason, "expected identifier but found %s instead", str)
        CT_FREE(str);
    }

    return out;
}

static CtASTList astListNew(CtAST* init)
{
    CtASTList list;
    list.alloc = 8;
    list.items = CT_MALLOC(sizeof(CtAST) * 8);

    if (!init)
    {
        list.len = 0;
    }
    else
    {
        list.len = 1;
        memcpy(list.items, init, sizeof(CtAST));
    }

    return list;
}

static void astListAdd(CtASTList* self, CtAST* node)
{
    if (self->len >= self->alloc)
    {
        self->alloc += 8;
        CtAST* temp = CT_MALLOC(sizeof(CtAST) * self->alloc);
        memcpy(temp, self->items, self->len * sizeof(CtAST));
        CT_FREE(self->items);
        self->items = temp;
    }

    self->items[self->len++] = *node;
}

static CtASTList parseCollect(CtParser* self, CtAST*(*func)(CtParser*), CtKeyword sep)
{
    CtASTList parts = astListNew(func(self));

    while (parseConsumeKey(self, sep))
    {
        astListAdd(&parts, func(self));
    }

    return parts;
}

static CtAST* parseType(CtParser* self);

static CtAST* parseExpr(CtParser* self)
{
    (void)self;
    return NULL;
}

static CtAST* parseTypeName(CtParser* self)
{
    /* matches ident (`::` ident)* */
    CtASTList parts = parseCollect(self, parseExpectIdent, K_COLON2);
    CtAST* out = astNew(AK_NAME);

    out->data.name = parts;

    return out;
}

static CtAST* parseArrayType(CtParser* self)
{
    CtAST* type = parseType(self);
    CtAST* size;

    /* TODO: distinguish between no size and var size */
    if (parseConsumeKey(self, K_COLON))
    {
        size = parseConsumeKey(self, K_VAR) ? NULL : parseExpr(self);
    }
    else
    {
        size = NULL;
    }

    parseExpectKey(self, K_RSQUARE);

    CtAST* node = astNew(AK_ARRAY);
    node->data.arr.type = type;
    node->data.arr.size = size;

    return node;
}

static CtAST* parsePointerType(CtParser* self)
{
    CtAST* node = astNew(AK_POINTER);
    node->data.ptr = parseType(self);

    return node;
}

static CtAST* parseReferenceType(CtParser* self)
{
    CtAST* node = astNew(AK_REFERENCE);
    node->data.ref = parseType(self);

    return node;
}

static CtAST* parseClosureType(CtParser* self)
{
    if(!parseExpectKey(self, K_LPAREN))
        return NULL;

    CtASTList args = parseCollect(self, parseType, K_COMMA);
    
    if(!parseExpectKey(self, K_RPAREN))
        return NULL;

    CtAST* result = parseConsumeKey(self, K_ARROW) ? NULL : parseType(self);

    CtAST* node = astNew(AK_CLOSURE);
    node->data.closure.args = args;
    node->data.closure.result = result;

    return node;
}

static CtAST* parseType(CtParser* self)
{
    if (parseConsumeKey(self, K_LSQUARE))
    {
        return parseArrayType(self);
    }
    else if (parseConsumeKey(self, K_MUL))
    {
        return parsePointerType(self);
    }
    else if (parseConsumeKey(self, K_BITAND))
    {
        return parseReferenceType(self);
    }
    else if (parseConsumeKey(self, K_DEF))
    {
        return parseClosureType(self);
    }
    else
    {
        return parseTypeName(self);
    }
}

static CtAST* parseAliasBody(CtParser* self)
{
    CtAST* node;

    node = parseType(self);

    return node;
}

static CtAST* parseAlias(CtParser* self)
{
    CtAST* node = astNew(AK_ALIAS);
    CtASTAlias data;

    data.name = parseExpectIdent(self);

    parseExpectKey(self, K_ASSIGN);

    data.symbol = parseAliasBody(self);

    parseExpectKey(self, K_SEMI);

    node->data.alias = data;

    return node;
}

static CtAST* parseImport(CtParser* self)
{
    CtASTList path = parseCollect(self, parseExpectIdent, K_COLON2);

    CtASTList items;

    if (parseConsumeKey(self, K_LPAREN))
    {
        items = parseCollect(self, parseExpectIdent, K_COMMA);
        parseExpectKey(self, K_RPAREN);
    }
    else
    {
        items.items = NULL;
    }

    parseExpectKey(self, K_SEMI);

    CtAST* node = astNew(AK_IMPORT);
    node->data.import.path = path;
    node->data.import.items = items;

    return node;
}

/**
 * public api
 */

CtStream* ctStreamOpen(CtStreamCallbacks callbacks, void* data, const char* path)
{
    CtStream* self = CT_MALLOC(sizeof(CtStream));
    self->data = data;
    self->callbacks = callbacks;
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

void ctStreamClose(CtStream* self)
{
    CT_FREE(self);
}

CtLexer* ctLexOpen(CtStream* stream)
{
    CtLexer* self = CT_MALLOC(sizeof(CtLexer));
    self->stream = stream;
    self->err = NULL;

    return self;
}

void ctLexClose(CtLexer* self)
{
    CT_FREE(self);
}

CtToken ctLexNext(CtLexer* self)
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
        lexIdent(self, &tok, c);
    }
    else if (c == '\'')
    {
        tok.data.letter = lexChar(self);
        tok.kind = TK_CHAR;
    }
    else if (c == '"')
    {
        /* is a string */
        tok.data.string = lexString(self);
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

    if (self->err)
    {
        tok.kind = TK_ERROR;
        tok.data.reason = self->err;
        self->err = NULL;
    }

    return tok;
}

CtParser* ctParseOpen(CtLexer* source)
{
    CtParser* self = CT_MALLOC(sizeof(CtParser));
    self->source = source;
    self->tok.kind = TK_INVALID;

    return self;
}

void ctParseClose(CtParser* self)
{
    tokFree(self->tok);
    CT_FREE(self);
}

CtAST* ctParseNext(CtParser* self)
{
    if (parseConsumeKey(self, K_ALIAS))
    {
        return parseAlias(self);
    }
    else if (parseConsumeKey(self, K_IMPORT))
    {
        return parseImport(self);
    }
    else
    {
        printf("how did we get here\n");
        return NULL;
    }
}

CtAST* ctParseUnit(CtParser* self)
{
    (void)self;
    return NULL;
}
