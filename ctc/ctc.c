#include "ctc.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

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

CtLexer* ctLexOpen(CtStream* stream)
{
    CtLexer* self = malloc(sizeof(CtLexer));
    self->stream = stream;

    return self;
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
    /* TODO: hex escapes */
    default:
        bufPush(buf, (char)c);
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
            printf("newline\n");
            break;
            /* error */
        }
        else if (res == STR_ERR)
        {
            printf("error\n");
            break;
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
            printf("error\n");
            break;
            /* error */
        }
        else if (res == STR_NL)
        {
            /* newlines are fine in multiline strings */
            bufPush(buf, '\n');
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
            /* TODO: need some globals and stuff */
            buf.ptr = cstrdup("");
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
        out->kind = TK_INVALID;
        break;
    }
}


/* base 16 integer */
static CtDigit lexHexDigit(CtLexer* self)
{
    int c;
    CtDigit out;
    CtBuffer buf = bufNew(32);
    streamNext(self->stream);

    while (1)
    {
        c = streamPeek(self->stream);
        if (isxdigit(c))
        {
            bufPush(&buf, (char)streamNext(self->stream));
        }
        else
        {
            break;
        }
    }

    out = strtoul(buf.ptr, NULL, 16);
    free(buf.ptr);

    return out;
}

/* base 2 integer */
static CtDigit lexBinDigit(CtLexer* self)
{
    int c;
    CtDigit out;
    CtBuffer buf = bufNew(32);
    streamNext(self->stream);

    while (1)
    {
        c = streamPeek(self->stream);
        if (c == '0' || c == '1')
        {
            bufPush(&buf, (char)streamNext(self->stream));
        }
        else
        {
            break;
        }
    }

    out = strtoul(buf.ptr, NULL, 2);
    free(buf.ptr);

    return out;
}

/* base 10 integer */
static CtDigit lexDigit1(CtLexer* self, int c)
{
    CtDigit out;
    CtBuffer buf = bufNew(32);
    bufPush(&buf, (char)c);

    while (1)
    {
        c = streamPeek(self->stream);
        if (isdigit(c))
        {
            bufPush(&buf, (char)streamNext(self->stream));
        }
        else
        {
            break;
        }
    }

    out = strtoul(buf.ptr, NULL, 10);
    free(buf.ptr);

    return out;
}

static CtDigit lexDigit0(CtLexer* self)
{
    int c = streamPeek(self->stream);

    if (c == 'x')
    {
        /* base 16 */
        return lexHexDigit(self);
    }
    else if (c == 'b')
    {
        /* base 2 */
        return lexBinDigit(self);
    }
    else if (isdigit(c))
    {
        /* base 10 */
        return lexDigit1(self, streamNext(self->stream));
    }
    else
    {
        /* just 0 */
        return 0;
    }
}

static CtNumber lexDigit(CtLexer* self, int c)
{
    CtToken suffix;
    CtNumber num;

    if (c == '0')
    {
        num.digit = lexDigit0(self);
    }
    else
    {
        num.digit = lexDigit1(self, c);
    }

    if (isident1(streamPeek(self->stream)))
    {
        /* we have a digit suffix */
        lexIdent(self, &suffix, '\0');

        if (suffix.kind != TK_IDENT)
        {
            /* keywords directly after a string are invalid */
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
        lexIdent(self, &tok, c);
    }
    else if (c == '"')
    {
        /* is a string */
        tok.data.string = lexString(self);
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

static const char* keyStr(CtKeyword key)
{
#define OP(id, str) case id: return str;
#define KEY(id, str) case id: return str;
    switch (key)
    {
#include "keys.inc"
    case K_INVALID: return "invalid";
    }
}

static void tokPrint(CtToken* tok)
{
    printf("[%s:%d:%d] = ", tok->pos.parent->name, tok->pos.line + 1, tok->pos.col);

    switch (tok->kind)
    {
    case TK_EOF:
        printf("EOF\n");
        break;
    case TK_INVALID:
        printf("INVALID\n");
        break;
    case TK_IDENT:
        printf("IDENT(%s)\n", tok->data.ident);
        break;
    case TK_KEYWORD:
        printf("KEYWORD(%s)\n", keyStr(tok->data.key));
        break;
    case TK_INT:
        printf("INT(%lu[%s])\n", tok->data.digit.digit, tok->data.digit.suffix);
        break;
    case TK_STRING:
        printf("STR([%s]\"\"\"%s\"\"\")\n", tok->data.string.prefix, tok->data.string.string);
        break;
    }
}

static CtASTList* astListNew(CtAST* item)
{
    CtASTList* self = malloc(sizeof(CtASTList));

    self->item = item;
    self->next = NULL;

    return self;
}

static CtASTList* astListAdd(CtASTList* self, CtAST* item)
{
    CtASTList* next;
    if (self->item == NULL)
    {
        self->item = item;
        next = self;
    }
    else
    {
        next = astListNew(item);
        self->next = next;
    }

    return next;
}

CtParser* ctParseOpen(CtLexer* source)
{
    CtParser* self = malloc(sizeof(CtParser));
    self->source = source;
    self->tok.kind = TK_INVALID;
    self->attribs = astListNew(NULL);
    self->tail = self->attribs;

    /* TODO */
    (void)tokPrint;

    return self;
}

static CtAST* astNew(CtASTKind kind)
{
    CtAST* self = malloc(sizeof(CtAST));

    self->kind = kind;

    return self;
}

static CtToken parseNext(CtParser* self)
{
    CtToken tok = self->tok;

    if (tok.kind == TK_INVALID)
    {
        tok = lexNext(self->source);
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

static void parseExpectKey(CtParser* self, CtKeyword key)
{
    CtToken tok = parseNext(self);

    if (tok.kind != TK_KEYWORD || tok.data.key != key)
    {
        printf("oh no 2\n");
        /* error */
    }
}

static CtAST* parseExpectIdent(CtParser* self)
{
    CtAST* out = astNew(AK_IDENT);
    CtToken tok = parseNext(self);

    if (tok.kind != TK_IDENT)
    {
        printf("oh no\n");
        /* error */
    }

    out->tok = tok;

    return out;
}

static CtASTList* parseItems(CtParser* self, CtKeyword sep)
{
    CtASTList* items = astListNew(parseExpectIdent(self));
    CtASTList* tail = items;

    while (parseConsumeKey(self, sep))
    {
        tail = astListAdd(tail, parseExpectIdent(self));
    }

    return items;
}

static CtAST* parseImport(CtParser* self)
{
    CtASTList* path;
    CtASTList* items;
    CtAST* dep;

    path = parseItems(self, K_COLON2);

    if (parseConsumeKey(self, K_LPAREN))
    {
        items = parseItems(self, K_COMMA);
        parseExpectKey(self, K_RPAREN);
    }
    else
    {
        items = NULL;
    }

    dep = astNew(AK_IMPORT);
    dep->data.import.path = path;
    dep->data.import.items = items;

    return dep;
}

static CtASTList* parseImports(CtParser* self)
{
    CtASTList* imports = astListNew(NULL);

    while (parseConsumeKey(self, K_IMPORT))
    {
        astListAdd(imports, parseImport(self));
        parseExpectKey(self, K_SEMI);
    }

    return imports;
}

static CtASTList* takeAttribs(CtParser* self)
{
    CtASTList* out = self->attribs;

    self->attribs = astListNew(NULL);
    self->tail = self->attribs;

    return out;
}

static void parseAttrib(CtParser* self)
{
    CtASTList* path;
    CtAST* attrib;

    path = parseItems(self, K_COLON2);

    attrib = astNew(AK_ATTRIB);
    attrib->data.attrib.path = path;

    self->tail = astListAdd(self->tail, attrib);
}

static void parseAttribs(CtParser* self)
{
    if (parseConsumeKey(self, K_LSQUARE))
    {
        do { parseAttrib(self); } while (parseConsumeKey(self, K_COMMA));
        parseExpectKey(self, K_RSQUARE);
    }
    else
    {
        parseAttrib(self);
    }
}

static CtAST* parseType(CtParser* self);

static CtAST* parsePointerType(CtParser* self)
{
    CtAST* type;
    CtAST* body;

    body = parseType(self);

    type = astNew(AK_PTRTYPE);
    type->data.ptr.type = body;

    return type;
}

static CtAST* parseArrayType(CtParser* self)
{
    CtAST* type;
    CtAST* body;
    CtAST* size;

    body = parseType(self);

    parseExpectKey(self, K_COLON);

    if (parseConsumeKey(self, K_VAR))
    {
        size = NULL;
    }
    else
    {
        size = parseExpr(self);
    }

    type = astNew(AK_ARRTYPE);
    type->data.arr.type = body;
    type->data.arr.size = size;

    return type;
}

static CtAST* parseFuncType(CtParser* self)
{
    CtAST* type;
    CtAST* ret;
    CtASTList* args = astListNew(NULL);
    CtASTList* tail = args;

    parseExpectKey(self, K_LPAREN);

    do { tail = astListAdd(tail, parseType(self)); } while (parseConsumeKey(self, K_COMMA));

    parseExpectKey(self, K_RPAREN);

    if (parseConsumeKey(self, K_ARROW))
    {
        ret = parseType(self);
    }
    else
    {
        ret = NULL;
    }

    type = astNew(AK_FUNCTYPE);
    type->data.funcptr.args = args;
    type->data.funcptr.ret = ret;

    return type;
}

static CtAST* parseBuiltinType(CtToken tok)
{
    char* name = tok.data.ident;

    /* TODO */

    if (strcmp(name, "i8") == 0)
    {

    }
    else if (strcmp(name, "i16") == 0)
    {

    }
    else if (strcmp(name, "i32") == 0)
    {

    }
    else if (strcmp(name, "i64") == 0)
    {

    }
    else if (strcmp(name, "void") == 0)
    {

    }
    else
    {
        return NULL;
    }
}

static CtAST* parseNameType(CtParser* self, CtToken tok)
{
    CtAST* type;
    CtASTList* name;
    CtASTList* tail;

    type = parseBuiltinType(tok);

    if (type != NULL)
        return type;

    type = astNew(AK_IDENT);
    type->tok = tok;

    name = astListNew(type);

    while (parseConsumeKey(self, K_COLON2))
    {
        tail = astListAdd(self, parseExpectIdent(self));
    }

    type = astNew(AK_NAMETYPE);
    type->data.nametype.path = name;

    return type;
}

static CtAST* parseType(CtParser* self)
{
    CtAST* type;
    CtToken tok = parseNext(self);

    if (tok.kind == TK_KEYWORD)
    {
        if (tok.data.key == K_MUL)
        {
            type = parsePointerType(self);
        }
        else if (tok.data.key == K_LSQUARE)
        {
            type = parseArrayType(self);
        }
        else if (tok.data.key == K_DEF)
        {
            type = parseFuncType(self);
        }
        else
        {
            /* error */
        }
    }
    else if (tok.kind == TK_IDENT)
    {
        type = parseNameType(self, tok);
    }
    else
    {
        /* error */
    }

    return type;
}

static CtAST* parseStructField(CtParser* self)
{
    CtAST* name;
    CtAST* type;
    CtAST* field;

    name = parseExpectIdent(self);

    parseExpectKey(self, K_COLON);

    type = parseType(self);

    parseExpectKey(self, K_SEMI);


    field = astNew(AK_FIELD);
    field->data.field.name = name;
    field->data.field.type = type;

    return field;
}

static CtASTList* parseStructFields(CtParser* self)
{
    CtASTList* fields = astListNew(NULL);
    CtASTList* tail = fields;

    while (!parseConsumeKey(self, K_RBRACE))
    {
        tail = astListAdd(tail, parseStructField(self));
    }

    return fields;
}

static CtAST* parseStruct(CtParser* self)
{
    CtAST* struc;
    CtAST* name;
    CtASTList* fields;

    name = parseExpectIdent(self);

    parseExpectKey(self, K_LBRACE);

    fields = parseStructFields(self);

    struc = astNew(AK_STRUCT);
    struc->data.struc.attribs = takeAttribs(self);
    struc->data.struc.name = name;
    struc->data.struc.fields = fields;

    return struc;
}

static CtASTList* parseBody(CtParser* self)
{
    CtASTList* items = astListNew(NULL);
    CtASTList* tail = items;

    while (1)
    {
        if (parseConsumeKey(self, K_AT))
        {
            parseAttribs(self);
        }
        else if (parseConsumeKey(self, K_STRUCT))
        {
            tail = astListAdd(tail, parseStruct(self));
        }
        else
        {
            break;
        }
    }

    return items;
}

CtAST* ctParseUnit(CtParser* self)
{
    CtAST* unit = astNew(AK_UNIT);

    unit->data.unit.imports = parseImports(self);

    unit->data.unit.body = parseBody(self);

    return unit;
}
