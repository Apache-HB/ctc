#include "cthulhu.h"

#include <ctype.h>
#include <string.h>

/**
 * config stuff
 * substitute these functions as needed
 *
 * TODO: remove this
 */

#include <malloc.h>

#ifndef CT_MALLOC
#   define CT_MALLOC(expr) malloc(expr)
#endif

#ifndef CT_FREE
#   define CT_FREE(expr) free(expr)
#endif

#ifndef CT_SPRINTF
#   include "printf/printf.c"
#   define CT_SPRINTF sprintf__
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
#define CT_FMT_ARGS(out, fmt, ...) { int len = CT_SPRINTF(NULL, fmt, __VA_ARGS__); out = (char*)CT_MALLOC((size_t)len + 1); out[len] = '\0'; CT_SPRINTF(out, fmt, __VA_ARGS__); }

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

typedef struct {
    char* ptr;
    size_t alloc;
    size_t len;
} CtBuffer;

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

/**
 * wrap streamNext so we can track length easily
 */
static int lexGet(CtLexer* self)
{
    self->len++;
    return streamNext(self->stream);
}

static int lexConsume(CtLexer* self, int c)
{
    if (streamPeek(self->stream) == c)
    {
        lexGet(self);
        return 1;
    }

    return 0;
}

/* skip whitespace and comments */
static int lexSkip(CtLexer* self)
{
    int c = lexGet(self);

    while (1)
    {
        /* skip whitespace */
        if (isspace(c))
        {
            c = lexGet(self);
        }
        else if (c == '#')
        {
            c = lexGet(self);

            /* skip until newline */
            while (c != '\n')
            {
                c = lexGet(self);
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
    int c = lexGet(self);

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
    int c = lexGet(self);

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
            if (lexConsume(self, '"'))
            {
                if (lexConsume(self, '"'))
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

    if (lexConsume(self, '"'))
    {
        if (lexConsume(self, '"'))
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
        bufPush(&buf, (char)lexGet(self));

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

    if (lexConsume(self, '"'))
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
        out->data.key = lexConsume(self, '=') ? K_ADDEQ : K_ADD;
        break;
    case '-':
        if (lexConsume(self, '>'))
            out->data.key = K_ARROW;
        else
            out->data.key = lexConsume(self, '=') ? K_SUBEQ : K_SUB;
        break;
    case '*':
        out->data.key = lexConsume(self, '=') ? K_MULEQ : K_MUL;
        break;
    case '/':
        out->data.key = lexConsume(self, '=') ? K_DIVEQ : K_DIV;
        break;
    case '%':
        out->data.key = lexConsume(self, '=') ? K_MODEQ : K_MOD;
        break;

        /* bitwise and logical operators */
    case '^':
        out->data.key = lexConsume(self, '=') ? K_BITXOREQ : K_BITXOR;
        break;
    case '&':
        if (lexConsume(self, '&'))
            out->data.key = K_AND;
        else if (lexConsume(self, '='))
            out->data.key = K_BITANDEQ;
        else
            out->data.key = K_BITAND;
        break;
    case '|':
        if (lexConsume(self, '|'))
            out->data.key = K_OR;
        else if (lexConsume(self, '='))
            out->data.key = K_BITOREQ;
        else
            out->data.key = K_BITOR;
        break;
    case '<':
        if (lexConsume(self, '<'))
            out->data.key = lexConsume(self, '=') ? K_SHLEQ : K_SHL;
        else if (lexConsume(self, '='))
            out->data.key = K_LTE;
        else if (lexConsume(self, '|')) /* the <| operator for streams */
            out->data.key = K_STREAM;
        else
            out->data.key = K_LT;
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
        if (self->depth == 0 && lexConsume(self, '>'))
            out->data.key = lexConsume(self, '=') ? K_SHREQ : K_SHR;
        else if (lexConsume(self, '='))
            out->data.key = K_GTE;
        else
            out->data.key = K_GT;
        break;
    case '~':
        out->data.key = K_BITNOT;
        break;

        /* logical operators */
    case '!':
        out->data.key = lexConsume(self, '=') ? K_NEQ : K_NOT;
        break;
    case '=':
        out->data.key = lexConsume(self, '=') ? K_EQ : K_ASSIGN;
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
        out->data.key = lexConsume(self, ':') ? K_COLON2 : K_COLON;
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
    lexGet(self);

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

        lexGet(self);
    }

    return out;
}

/* base 2 integer */
static CtDigit lexBinDigit(CtLexer* self)
{
    int i = 0;
    int c;
    CtDigit out = 0;
    lexGet(self);

    while (1)
    {
        c = streamPeek(self->stream);
        if (c == '0' || c == '1')
        {
            out *= 2;
            if (c == '1')
                out += 1;

            lexGet(self);

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
    CtDigit out = (CtDigit)(c - '0');

    while (1)
    {
        c = streamPeek(self->stream);
        if (isdigit(c))
        {
            out *= (10 + ((uint8_t)c - '0'));
            lexGet(self);

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
        return lexDigit1(self, lexGet(self));
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

    int64_t res2 = lexGet(self);

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

#define IN_TEMPLATE(self, ...) { ctLexBeginTemplate(self->source); __VA_ARGS__ ctLexEndTemplate(self->source); }

static CtASTArray makeArray(size_t size)
{
    CtASTArray arr = { CT_MALLOC(sizeof(CtAST) * size), size, 0 };

    return arr;
}

static void arrayAdd(CtASTArray* self, CtAST* item)
{
    if (self->alloc >= self->len + 1)
    {
        self->alloc += 8;
        CtAST* temp = CT_MALLOC(sizeof(CtAST) * self->alloc);
        memcpy(temp, self->data, self->len * sizeof(CtAST));
        CT_FREE(self->data);
        self->data = temp;
    }

    self->data[self->len++] = *item;
}

static CtAST* parseStmtList(CtParser* self);

static CtASTArray takeAttribs(CtParser* self)
{
    CtASTArray temp = self->attribs;
    self->attribs = makeArray(4);
    return temp;
}

static CtAST* makeAST(CtASTKind kind)
{
    CtAST* node = CT_MALLOC(sizeof(CtAST));
    node->kind = kind;
    return node;
}

static CtToken parseNext(CtParser* self)
{
    CtToken tok = self->tok;
    self->tok.kind = TK_INVALID;

    if (tok.kind == TK_INVALID)
    {
        tok = ctLexNext(self->source);
    }

    return tok;
}

static CtToken parsePeek(CtParser* self)
{
    CtToken tok = self->tok;

    if (tok.kind == TK_INVALID)
    {
        self->tok = ctLexNext(self->source);
        tok = self->tok;
    }

    return tok;
}

static CtToken parseExpect(CtParser* self, CtKeyword key)
{
    CtToken tok = parseNext(self);

    if (tok.kind != TK_KEYWORD || tok.data.key != key)
    {
        char* str = tokStr(tok);
        CT_FMT_ARGS(self->err, "expected keyword `%s` when parsing but found %s instead", keyStr(key), str)
        CT_FREE(str);
    }

    return tok;
}

static int parseConsume(CtParser* self, CtKeyword key)
{
    CtToken tok = parseNext(self);

    if (tok.kind != TK_KEYWORD || tok.data.key != key)
    {
        self->tok = tok;
        return 0;
    }

    return 1;
}

static CtAST* parseIdent(CtParser* self)
{
    CtToken tok = parseNext(self);

    CtAST* node = makeAST(AK_IDENT);

    if (tok.kind != TK_IDENT)
    {
        node->data.ident = NULL;

        char* msg = tokStr(tok);
        CT_FMT_ARGS(self->err, "expected ident but found %s instead", msg)
        CT_FREE(msg);
    }
    else
    {
        node->tok = tok;
        node->data.ident = tok.data.ident;
    }

    return node;
}

static CtASTArray parseMany(CtParser* self, CtAST*(*func)(CtParser*), CtKeyword key)
{
    CtASTArray arr = makeArray(8);

    do { arrayAdd(&arr, func(self)); } while (parseConsume(self, key));

    return arr;
}

static CtASTArray parseCollect(CtParser* self, CtAST*(*func)(CtParser*))
{
    CtASTArray arr = makeArray(8);

    CtAST* node = func(self);
    while (node)
    {
        arrayAdd(&arr, node);
        node = func(self);
    }

    return arr;
}

static CtASTArray parseUntil(CtParser* self, CtAST*(*func)(CtParser*), CtKeyword sep, CtKeyword end)
{
    if (parseConsume(self, end))
        return makeArray(0);

    CtASTArray arr = parseMany(self, func, sep);
    parseExpect(self, end);

    return arr;
}

static CtASTArray parseUntilEnd(CtParser* self, CtAST*(*func)(CtParser*), CtKeyword end)
{
    CtASTArray arr = makeArray(0);

    while (!parseConsume(self, end))
    {
        arrayAdd(&arr, func(self));
    }

    return arr;
}

static CtASTArray parseImportSymbols(CtParser* self)
{
    CtASTArray symbols;
    if (parseConsume(self, K_LPAREN))
    {
        symbols = parseMany(self, parseIdent, K_COMMA);
        parseExpect(self, K_RPAREN);
    }
    else
    {
        symbols = makeArray(0);
    }

    return symbols;
}

static CtASTArray parsePath(CtParser* self)
{
    CtASTArray path = parseMany(self, parseIdent, K_COLON2);
    return path;
}

static CtAST* parseImport(CtParser* self)
{
    if (!parseConsume(self, K_IMPORT))
        return NULL;

    CtAST* node = makeAST(AK_IMPORT);

    node->data.include.path = parsePath(self);
    node->data.include.symbols = parseImportSymbols(self);

    parseExpect(self, K_SEMI);

    return node;
}

static CtAST* parseExpr(CtParser* self);
static CtAST* parseType(CtParser* self);

static CtAST* parseCallArg(CtParser* self)
{
    CtAST* arg = makeAST(AK_CALL_ARG);

    if (parseConsume(self, K_LSQUARE))
    {
        arg->data.carg.name = parseIdent(self);
        parseExpect(self, K_RSQUARE);
        parseExpect(self, K_ASSIGN);
    }
    else
    {
        arg->data.carg.name = NULL;
    }

    arg->data.carg.val = parseExpr(self);

    return arg;
}

static CtAST* parseBuiltin(CtParser* self)
{
    CtASTArray name = parseMany(self, parseIdent, K_COLON2);

    /* TODO */

    //return node;
    (void)name;
    return NULL;
}

static CtAST* parseAttrib(CtParser* self)
{
    CtASTArray path = parseMany(self, parseIdent, K_COLON2);
    CtASTArray args = parseConsume(self, K_LPAREN)
        ? parseUntil(self, parseCallArg, K_COMMA, K_RPAREN)
        : makeArray(0);

    CtAST* attrib = makeAST(AK_ATTRIB);
    attrib->data.attrib.name = path;
    attrib->data.attrib.args = args;

    return attrib;
}

typedef enum {
    OP_ERROR = 0,

    OP_ASSIGN = 1,
    OP_TERNARY = 2,
    OP_COMPARE = 3,
    OP_EQUAL = 4,
    OP_LOGIC = 5,
    OP_BITS = 6,
    OP_SHIFT = 7,
    OP_ADD = 8,
    OP_MUL = 9,
} CtOpPrec;

static CtOpPrec binopPrec(CtToken tok)
{
    if (tok.kind != TK_KEYWORD)
        return OP_ERROR;

    switch (tok.data.key)
    {
    case K_MUL: case K_DIV: case K_MOD:
        return OP_MUL;
    case K_ADD: case K_SUB:
        return OP_ADD;
    case K_SHL: case K_SHR:
        return OP_SHIFT;
    case K_BITXOR: case K_BITAND: case K_BITOR:
        return OP_BITS;
    case K_AND: case K_OR:
        return OP_LOGIC;
    case K_EQ: case K_NEQ:
        return OP_EQUAL;
    case K_LT: case K_LTE: case K_GT: case K_GTE:
        return OP_COMPARE;
    case K_QUESTION:
        return OP_TERNARY;
    case K_ASSIGN: case K_ADDEQ: case K_SUBEQ:
    case K_MULEQ: case K_DIVEQ: case K_MODEQ:
    case K_SHLEQ: case K_SHREQ: case K_BITXOREQ:
    case K_BITANDEQ: case K_BITOREQ:
        return OP_ASSIGN;

    default:
        return OP_ERROR;
    }
}

static CtAST* parseCoerce(CtParser* self)
{
    CtAST* node = makeAST(AK_COERCE);
    IN_TEMPLATE(self, {
        parseExpect(self, K_LT);
        node->data.cast.type = parseType(self);
        parseExpect(self, K_GT);
    })

    parseExpect(self, K_LPAREN);
    node->data.cast.expr = parseExpr(self);
    parseExpect(self, K_RPAREN);

    return node;
}

static CtAST* parseCall(CtParser* self, CtAST* lhs)
{
    CtAST* call = makeAST(AK_CALL);
    call->data.call.func = lhs;
    call->data.call.args = parseUntil(self, parseCallArg, K_COMMA, K_RPAREN);

    return call;
}

static CtAST* parseInitArg(CtParser* self)
{
    CtAST* arg = makeAST(AK_INIT_ARG);

    if (parseConsume(self, K_LSQUARE))
    {
        arg->data.iarg.field = parseExpr(self);
        parseExpect(self, K_RSQUARE);
        parseExpect(self, K_ASSIGN);
    }
    else
    {
        arg->data.iarg.field = NULL;
    }

    arg->data.iarg.val = parseExpr(self);

    return arg;
}

static CtAST* parseInit(CtParser* self, CtAST* lhs)
{
    CtAST* init = makeAST(AK_INIT);
    init->data.init.type = lhs;
    init->data.init.args = parseUntil(self, parseInitArg, K_COMMA, K_RBRACE);

    return init;
}

static CtAST* parseSubscript(CtParser* self, CtAST* lhs)
{
    CtAST* sub = makeAST(AK_SUBSCRIPT);
    sub->data.subscript.expr = lhs;
    sub->data.subscript.idx = parseExpr(self);
    parseExpect(self, K_RSQUARE);

    return sub;
}

static CtAST* parseAccess(CtParser* self, CtAST* lhs)
{
    CtAST* node = makeAST(AK_ACCESS);
    node->data.access.expr = lhs;
    node->data.access.field = parseIdent(self);
    return node;
}

static CtAST* parseDeref(CtParser* self, CtAST* lhs)
{
    CtAST* node = makeAST(AK_DEREF);
    node->data.deref.expr = lhs;
    node->data.deref.field = parseIdent(self);
    return node;
}

static CtAST* parseClosureArg(CtParser* self)
{
    CtAST* arg = makeAST(AK_FUNC_ARG);
    arg->data.arg.name = parseIdent(self);
    arg->data.arg.type = parseConsume(self, K_COLON) ? parseType(self) : NULL;
    return arg;
}

static CtAST* parseClosure(CtParser* self)
{
    CtASTArray args = parseConsume(self, K_LPAREN)
        ? parseUntil(self, parseClosureArg, K_COMMA, K_RPAREN)
        : makeArray(0);
    /* TODO: captures */
    CtAST* result = parseConsume(self, K_ARROW) ? parseType(self) : NULL;
    CtAST* body = parseStmtList(self);

    CtAST* closure = makeAST(AK_CLOSURE);
    closure->data.closure.args = args;
    closure->data.closure.result = result;
    closure->data.closure.body = body;

    return closure;
}

static CtAST* parsePrimaryExpr(CtParser* self)
{
    CtToken tok = parsePeek(self);
    CtAST* node = NULL;

    if (tok.kind == TK_KEYWORD)
    {
        switch (tok.data.key)
        {
        case K_LPAREN:
            /* (expr) */
            parseNext(self);
            node = parseExpr(self);
            parseExpect(self, K_RPAREN);
            break;
        case K_ADD: case K_SUB: case K_MUL:
        case K_BITAND: case K_NOT: case K_BITNOT:
            node = makeAST(AK_UNARY);
            node->tok = parseNext(self);
            node->data.expr = parseExpr(self);
            break;
        case K_COERCE:
            parseNext(self);
            node = parseCoerce(self);
            break;
        case K_LBRACE:
            parseNext(self);
            node = makeAST(AK_INIT);
            node->data.init.type = NULL;
            node->data.init.args = parseUntil(self, parseInitArg, K_COMMA, K_RBRACE);
            break;
        case K_AT:
            node = parseBuiltin(self);
            break;
        case K_DEF:
            parseNext(self);
            node = parseClosure(self);
            break;
        default:
            /* error */
            break;
        }
    }
    else if (tok.kind == TK_CHAR || tok.kind == TK_STRING || tok.kind == TK_INT)
    {
        node = makeAST(AK_LITERAL);
        node->tok = parseNext(self);
    }
    else if (tok.kind == TK_IDENT)
    {
        node = makeAST(AK_NAME);
        node->data.names = parseMany(self, parseIdent, K_COLON2);
    }

    while (1)
    {
        if (parseConsume(self, K_LPAREN))
        {
            node = parseCall(self, node);
        }
        else if (parseConsume(self, K_LBRACE))
        {
            node = parseInit(self, node);
        }
        else if (parseConsume(self, K_LSQUARE))
        {
            node = parseSubscript(self, node);
        }
        else if (parseConsume(self, K_DOT))
        {
            node = parseAccess(self, node);
        }
        else if (parseConsume(self, K_ARROW))
        {
            node = parseDeref(self, node);
        }
        else
        {
            break;
        }
    }

    return node;
}

static CtAST* makeBinop(CtAST* lhs, CtAST* rhs, CtToken op)
{
    CtAST* node = makeAST(AK_BINOP);
    node->tok = op;
    node->data.binop.op = op.data.key;
    node->data.binop.lhs = lhs;
    node->data.binop.rhs = rhs;
    return node;
}

static CtAST* parseBinaryExpr(CtParser* self, CtAST* lhs, CtOpPrec min_prec)
{
    CtToken ahead = parsePeek(self);

    if (binopPrec(ahead) == OP_TERNARY)
    {
        parseNext(self);

        /* special case for ternarys */
        CtAST* expr = makeAST(AK_TERNARY);
        expr->data.ternary.cond = lhs;

        if (parseConsume(self, K_COLON))
        {
            expr->data.ternary.truthy = lhs;
        }
        else
        {
            expr->data.ternary.truthy = parsePrimaryExpr(self);
            parseExpect(self, K_COLON);
        }

        expr->data.ternary.falsey = parseExpr(self);

        return expr;
    }

    while (binopPrec(ahead) >= min_prec)
    {
        CtToken op = parseNext(self);
        CtAST* rhs = parsePrimaryExpr(self);
        ahead = parsePeek(self);

        while (binopPrec(ahead) >= binopPrec(op) && binopPrec(ahead) != OP_ERROR)
        {
            rhs = parseBinaryExpr(self, rhs, binopPrec(ahead));
            ahead = parsePeek(self);
        }

        lhs = makeBinop(lhs, rhs, op);
    }

    return lhs;
}

static CtAST* parseExpr(CtParser* self)
{
    return parseBinaryExpr(self, parsePrimaryExpr(self), OP_ASSIGN);
}

static CtAST* parseType(CtParser* self);

static CtAST* parseFuncType(CtParser* self)
{
    CtAST* node = makeAST(AK_FUNC_TYPE);
    node->data.sig.args = parseConsume(self, K_LPAREN)
        ? parseUntil(self, parseType, K_COMMA, K_RPAREN)
        : makeArray(0);
    node->data.sig.result = parseConsume(self, K_ARROW)
        ? parseType(self) : NULL;

    return node;
}

static CtAST* parseArray(CtParser* self)
{
    CtAST* node = makeAST(AK_ARR);
    node->data.arr.type = parseType(self);
    node->data.arr.size = parseConsume(self, K_COLON)
        ? parseExpr(self) : NULL;

    parseExpect(self, K_RSQUARE);
    return node;
}

static CtAST* parseQualTypeParam(CtParser* self)
{
    CtAST* node = makeAST(AK_TYPE_PARAM);

    if (parseConsume(self, K_COLON))
    {
        node->data.param.name = parseIdent(self);
        parseExpect(self, K_ASSIGN);
    }
    else
    {
        node->data.param.name = NULL;
    }

    node->data.param.type = parseType(self);

    return node;
}

static CtASTArray parseQualTypeParams(CtParser* self)
{
    CtASTArray arr;

    self->tok = parseNext(self);

    if (parseConsume(self, K_LT))
    {
        IN_TEMPLATE(self, {
            arr = parseMany(self, parseQualTypeParam, K_COMMA);
            parseExpect(self, K_GT);
        })
    }
    else
    {
        arr = makeArray(0);
    }

    return arr;
}

static CtAST* parseQualType(CtParser* self)
{
    CtAST* node = makeAST(AK_QUAL);
    node->data.qual.name = parseIdent(self);
    self->tok = parseNext(self);
    node->data.qual.params = parseQualTypeParams(self);

    return node;
}

static CtAST* parseType(CtParser* self)
{
    CtToken tok = parseNext(self);
    CtAST* node = NULL;

    if (tok.kind == TK_IDENT)
    {
        node = makeAST(AK_TYPE);

        self->tok = tok;
        node->data.types = parseMany(self, parseQualType, K_COLON2);
    }
    else if (tok.kind == TK_KEYWORD)
    {
        switch (tok.data.key)
        {
        case K_MUL:
            node = makeAST(AK_PTR);
            node->data.ptr = parseType(self);
            break;
        case K_BITAND:
            node = makeAST(AK_REF);
            node->data.ref = parseMany(self, parseQualType, K_COLON2);
            break;
        case K_LSQUARE:
            node = parseArray(self);
            break;
        case K_DEF:
            node = parseFuncType(self);
            break;
        default:
            break;
        }
    }

    return node;
}

static CtAST* parseAlias(CtParser* self)
{
    CtAST* node = makeAST(AK_ALIAS);

    node->data.alias.name = parseIdent(self);
    parseExpect(self, K_ASSIGN);

    /* for now aliases can only be for types */
    node->data.alias.body = parseType(self);
    parseExpect(self, K_SEMI);

    return node;
}

static CtAST* parseField(CtParser* self)
{
    CtAST* field = makeAST(AK_FIELD);

    field->data.field.name = parseIdent(self);
    parseExpect(self, K_COLON);
    field->data.field.type = parseType(self);
    parseExpect(self, K_SEMI);

    return field;
}

static CtAST* parseStruct(CtParser* self)
{
    CtAST* struc = makeAST(AK_STRUCT);
    struc->data.struc.name = parseIdent(self);

    parseExpect(self, K_LBRACE);
    struc->data.struc.fields = parseUntilEnd(self, parseField, K_RBRACE);

    struc->data.struc.attribs = takeAttribs(self);

    return struc;
}

static CtAST* parseUnion(CtParser* self)
{
    CtAST* uni = makeAST(AK_UNION);
    uni->data.uni.name = parseIdent(self);

    parseExpect(self, K_LBRACE);
    uni->data.uni.fields = parseUntilEnd(self, parseField, K_RBRACE);

    uni->data.uni.attribs = takeAttribs(self);

    return uni;
}

static CtAST* parseEnumField(CtParser* self)
{
    CtAST* field = makeAST(AK_ENUM_FIELD);
    field->data.efield.name = parseIdent(self);
    field->data.efield.fields = parseConsume(self, K_LBRACE) ? parseUntilEnd(self, parseField, K_RBRACE) : makeArray(0);
    field->data.efield.val = parseConsume(self, K_ASSIGN) ? parseExpr(self) : NULL;

    return field;
}

static CtAST* parseEnum(CtParser* self)
{
    CtAST* enu = makeAST(AK_ENUM);
    enu->data.enu.name = parseIdent(self);
    enu->data.enu.backing = parseConsume(self, K_COLON) ? parseType(self) : NULL;

    parseExpect(self, K_LBRACE);
    enu->data.enu.fields = parseUntil(self, parseEnumField, K_COMMA, K_RBRACE);

    enu->data.enu.attribs = takeAttribs(self);

    return enu;
}

static CtAST* parseVarName(CtParser* self)
{
    CtAST* name = makeAST(AK_VAR_NAME);
    name->data.vname.name = parseIdent(self);
    name->data.vname.type = parseConsume(self, K_COLON) ? parseType(self) : NULL;

    return name;
}

static CtASTArray parseVarNames(CtParser* self)
{
    CtASTArray names;
    if (parseConsume(self, K_LSQUARE))
    {
        names = parseUntil(self, parseVarName, K_COMMA, K_RSQUARE);
    }
    else
    {
        names = makeArray(1);
        arrayAdd(&names, parseVarName(self));
    }

    return names;
}

static CtAST* parseVar(CtParser* self)
{
    CtAST* node = makeAST(AK_VAR);
    node->data.var.names = parseVarNames(self);
    node->data.var.init = parseConsume(self, K_ASSIGN) ? parseExpr(self) : NULL;

    parseExpect(self, K_SEMI);

    node->data.var.attribs = takeAttribs(self);

    return node;
}

static CtAST* parseStmt(CtParser* self);

static CtAST* parseStmtList(CtParser* self)
{
    CtAST* list = makeAST(AK_STMT_LIST);
    list->data.stmts = parseUntilEnd(self, parseStmt, K_RBRACE);
    return list;
}

static CtAST* parseFuncArg(CtParser* self)
{
    CtAST* arg = makeAST(AK_FUNC_ARG);
    arg->data.arg.name = parseIdent(self);
    parseExpect(self, K_COLON);
    arg->data.arg.type = parseType(self);
    arg->data.arg.init = parseConsume(self, K_ASSIGN) ? parseExpr(self) : NULL;

    return arg;
}

static CtAST* parseFuncBody(CtParser* self)
{
    CtAST* node;

    if (parseConsume(self, K_SEMI))
    {
        node = NULL;
    }
    else if (parseConsume(self, K_ASSIGN))
    {
        node = parseExpr(self);
        parseExpect(self, K_SEMI);
    }
    else if (parseConsume(self, K_LBRACE))
    {
        node = parseStmtList(self);
    }
    else
    {
        node = NULL;
        /* error */
    }

    return node;
}

static CtAST* parseFunc(CtParser* self)
{
    CtAST* func = makeAST(AK_FUNCTION);
    func->data.func.name = parseIdent(self);
    func->data.func.args = parseConsume(self, K_LPAREN)
        ? parseUntil(self, parseFuncArg, K_COMMA, K_RPAREN)
        : makeArray(0);
    func->data.func.result = parseConsume(self, K_ARROW)
        ? parseType(self) : NULL;
    func->data.func.body = parseFuncBody(self);

    func->data.func.attribs = takeAttribs(self);

    return func;
}

static CtAST* parseBody(CtParser* self)
{
    CtAST* node = NULL;

    if (parseConsume(self, K_ALIAS))
    {
        node = parseAlias(self);
    }
    else if (parseConsume(self, K_STRUCT))
    {
        node = parseStruct(self);
    }
    else if (parseConsume(self, K_ENUM))
    {
        node = parseEnum(self);
    }
    else if (parseConsume(self, K_UNION))
    {
        node = parseUnion(self);
    }
    else if (parseConsume(self, K_VAR))
    {
        node = parseVar(self);
    }
    else if (parseConsume(self, K_DEF))
    {
        node = parseFunc(self);
    }
    else if (parseConsume(self, K_AT))
    {
        if (parseConsume(self, K_LSQUARE))
        {
            do { arrayAdd(&self->attribs, parseAttrib(self)); } while (parseConsume(self, K_LPAREN));
            parseExpect(self, K_RSQUARE);
            node = parseBody(self);
        }
        else
        {
            node = parseBuiltin(self);
        }
    }

    return node;
}

static CtAST* parseBranchLeaf(CtParser* self)
{
    CtAST* leaf = makeAST(AK_BRANCH_LEAF);

    if (parseConsume(self, K_IF))
    {
        parseExpect(self, K_LPAREN);
        leaf->data.leaf.cond = parseExpr(self);
        parseExpect(self, K_RPAREN);
    }
    else
    {
        leaf->data.leaf.cond = NULL;
    }

    leaf->data.leaf.body = parseStmt(self);

    return leaf;
}

static CtAST* parseBranchStmt(CtParser* self)
{
    CtASTArray branches = makeArray(4);

    parseExpect(self, K_LPAREN);
    CtAST* leaf = makeAST(AK_BRANCH_LEAF);
    leaf->data.leaf.cond = parseExpr(self);
    parseExpect(self, K_RPAREN);
    leaf->data.leaf.body = parseStmt(self);

    arrayAdd(&branches, leaf);

    while (parseConsume(self, K_ELSE))
    {
        arrayAdd(&branches, parseBranchLeaf(self));
    }

    CtAST* node = makeAST(AK_BRANCH);
    node->data.branches = branches;

    return node;
}

static CtAST* parseForStmt(CtParser* self)
{
    CtAST* loop = makeAST(AK_FOR);
    parseExpect(self, K_LPAREN);
    loop->data.loop.names = parseVarNames(self);
    parseExpect(self, K_STREAM);
    loop->data.loop.range = parseExpr(self);
    parseExpect(self, K_RPAREN);
    loop->data.loop.body = parseStmt(self);

    return loop;
}

static CtAST* parseWhileStmt(CtParser* self)
{
    CtAST* loop = makeAST(AK_WHILE);
    parseExpect(self, K_LPAREN);
    loop->data.wloop.cond = parseExpr(self);
    parseExpect(self, K_RPAREN);
    loop->data.wloop.cond = parseStmt(self);

    return loop;
}

static CtAST* parseDoWhileStmt(CtParser* self)
{
    CtAST* loop = makeAST(AK_DO_WHILE);
    loop->data.wloop.body = parseStmt(self);
    parseExpect(self, K_WHILE);
    parseExpect(self, K_LPAREN);
    loop->data.wloop.cond = parseExpr(self);
    parseExpect(self, K_RPAREN);
    parseExpect(self, K_SEMI);

    return loop;
}

static CtAST* parseStmt(CtParser* self)
{
    CtAST* node;

    if (parseConsume(self, K_LBRACE))
    {
        node = parseStmtList(self);
    }
    else if (parseConsume(self, K_RETURN))
    {
        node = makeAST(AK_RETURN);
        if (parseConsume(self, K_SEMI))
        {
            node->data.expr = NULL;
        }
        else
        {
            node->data.expr = parseExpr(self);
            parseExpect(self, K_SEMI);
        }
    }
    else if (parseConsume(self, K_IF))
    {
        node = parseBranchStmt(self);
    }
    else if (parseConsume(self, K_FOR))
    {
        node = parseForStmt(self);
    }
    else if (parseConsume(self, K_WHILE))
    {
        node = parseWhileStmt(self);
    }
    else if (parseConsume(self, K_DO))
    {
        node = parseDoWhileStmt(self);
    }
    else if (parseConsume(self, K_ALIAS))
    {
        node = parseAlias(self);
    }
    else if (parseConsume(self, K_VAR))
    {
        node = parseVar(self);
    }
    else
    {
        node = parseExpr(self);
        parseExpect(self, K_SEMI);
    }

    return node;
}

/**
 * public api
 */

CtStream ctStreamOpen(CtStreamCallbacks callbacks, void* data, const char* path)
{
    CtStream self;
    self.data = data;
    self.callbacks = callbacks;
    self.ahead = callbacks.next(self.data);
    self.name = path;

    self.pos.name = path;
    self.pos.pos = 1;

    if (self.ahead == '\n')
    {
        self.pos.col = 0;
        self.pos.line = 1;
    }
    else
    {
        self.pos.col = 1;
        self.pos.line = 0;
    }

    return self;
}

CtLexer ctLexOpen(CtStream* stream)
{
    CtLexer self;
    self.stream = stream;
    self.err = NULL;
    self.depth = 0;

    return self;
}

CtToken ctLexNext(CtLexer* self)
{
    int c = lexSkip(self);
    CtToken tok;

    /* reset the length of the current token */
    self->len = 0;

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

    /* save the length for error reporting if needed */
    tok.pos.len = self->len;

    if (self->err)
    {
        tok.kind = TK_ERROR;
        tok.data.reason = self->err;
        self->err = NULL;
    }

    return tok;
}

void ctLexBeginTemplate(CtLexer* self) { self->depth++; }
void ctLexEndTemplate(CtLexer* self) { self->depth--; }

void ctFreeToken(CtToken tok)
{
    tokFree(tok);
}

CtParser ctParseOpen(CtLexer* source)
{
    CtParser self;
    self.source = source;
    self.tok.kind = TK_INVALID;
    self.err = NULL;
    self.node = NULL;
    self.attribs = makeArray(4);

    return self;
}

CtAST* ctParseUnit(CtParser* self)
{
    CtAST* node = makeAST(AK_UNIT);

    node->data.unit.imports = parseCollect(self, parseImport);
    node->data.unit.symbols = parseCollect(self, parseBody);

    return node;
}
