#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>
#include <stdint.h>

typedef size_t CtSize;

typedef struct {
    char *ptr;
    CtSize len;
    CtSize alloc;
} CtBuffer;

typedef struct {
    CtSize dist;
    CtSize col;
    CtSize line;
} CtOffset;

typedef enum {
    ERR_NONE
} CtErrorType;

typedef struct {
    CtErrorType type;

    CtSize len;
    CtOffset pos;
} CtError;

typedef struct {
    CtError *errs;
    CtSize len;
    CtSize alloc;
} CtErrorBuffer;

/**
 * expected to behave like fgetc
 *
 * returns -1 on EOF, otherwise returns an int
 * in the range of 0..255, other values will make
 * the lexer error.
 *
 * we also expect \n rather than clrf line endings.
 */
typedef int(*CtNextFunc)(void*);

typedef struct {
    /* externally provided stream and next function */
    void *stream;
    CtNextFunc next;

    /* the lookahead character */
    int ahead;

    /**
     * internal buffer holding all lexed source
     *
     * we keep this around to avoid allocations.
     * this has a few added benefits such as tokens
     * not needing to be free'd as they can store
     * indicies into this buffer rather than their own data
     * for stuff like TK_STRING, TK_IDENT, and the TK_INT suffix
     *
     * this also makes error reporting easier
     * as now data can be pulled from this buffer
     * so no seeking is required
     */
    CtBuffer source;

    /**
     * the current distance in the stream.
     * we keep track of this to pass to tokens
     * incase its needed for error reporting or debug.
     *
     * this may also be useful for the interpreter
     * for reflection and the likes.
     */
    CtOffset pos;

    /**
     * the length of the token currently being lexed.
     *
     * note that this is the *total* length of the token.
     * so when lexing a string or character literal
     * this will include the enclosing quotes.
     */
    CtSize len;
} CtLexer;

typedef enum {
    /* identifier, matches [a-zA-Z_][a-zA-Z0-9_]* */
    TK_IDENT,

    /* a keyword */
    TK_KEYWORD,

    /* this is used for context sensitive or "soft" keywords */
    TK_SOFT,

    /* single or multiline string */
    TK_STRING,
    /* character literal */
    TK_CHAR,
    /* base2, base10, or base16 integer, the maximum value is dependant on sizeof(CtSize) */
    TK_INT,
    /* the end of the file */
    TK_EOF,

    /**
     * an invalid lexing token
     * can be
     *  - overflowed int
     *  - invalid symbol
     *  - unterminated string
     *  - invalid string escape
     *  - missing trailing ' on character literal
     */
    TK_ERROR,

    /* used internally by the parser */
    TK_LOOKAHEAD
} CtTokenKind;

/**
 * we store offsets into source rather than pointers
 * as the internal data might be reallocated which would
 * invalidate pointers into it
 */

typedef struct {
    CtSize offset;
    CtSize len;

    /* TODO: kinda memory inefficient */
    int multiline;
} CtString;

typedef struct {
    /**
     * we keep around the original
     * encoding of the data for error reporting
     */
    enum { ENC2, ENC10, ENC16 } encoding;
    CtSize num;

    /**
     * numbers are allowed to have suffixes,
     * if suffix == 0 then there is no suffix
     */
    CtSize suffix;
} CtDigit;

typedef struct {
#define KEY(id, str) id,
#define OP(id, str) id,
#include "keys.inc"

    K_INVALID
} CtKey;

typedef struct {
    /* TK_IDENT */
    CtSize ident;

    /* TK_STRING */
    CtString string;

    /* TK_KEYWORD */
    CtKey key;

    /* TK_INT */
    CtDigit digit;

    /* TK_CHAR */
    CtSize letter;

    /* TODO: TK_SOFT */

    /* TK_EOF, TK_LOOKAHEAD, TK_ERROR */
} CtTokenData;

typedef struct {
    CtTokenKind kind;
    CtTokenData data;

    /* the position and total length of the token */
    CtOffset pos;
    CtSize len;
} CtToken;

/**
 * create a new lexer
 */
CtLexer ctLexerNew(void *stream, CtNextFunc next);

/**
 * cleanup the lexer and free memory
 *
 * NOTE: do this only *after* you've done anything that
 * interacts with token data as this will invalidate all
 * tokens.
 */
void ctLexerDelete(CtLexer *self);

/**
 * lex the next token from the stream
 */
CtToken ctLexerNext(CtLexer *self);

#endif /* CTHULHU_H */

#if 0

#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>
#include <stdint.h>

typedef size_t CtSize;
typedef uint64_t CtInt;

typedef void*(*CtAllocFunc)(void*, CtSize);
typedef void(*CtDeallocFunc)(void*, void*);

typedef struct {
    CtSize dist;
    CtSize line;
    CtSize col;
} CtPosition;


typedef enum {
    /* no error */
    ERR_NONE,
    /* integer overflow */
    ERR_OVERFLOW,
    /* bad escape sequence in string/char literal */
    ERR_INVALID_ESCAPE,

    /* EOF inside a string/char literal */
    ERR_EOF,

    /* newline inside a single line string/char literal */
    ERR_NEWLINE,

    /* missing trailing ' in a char literal */
    ERR_UNTERMINATED,

    /* invalid symbol while lexing */
    ERR_INVALID_SYMBOL,

    /* unexpected token encountered when parsing */
    ERR_UNEXPECTED
} CtError;

typedef enum {
    TK_IDENT,
    TK_KEYWORD,
    TK_USER_KEYWORD,
    TK_STRING,
    TK_CHAR,
    TK_INT,
    TK_EOF,
    TK_ERROR,

    /* used internally by the parser */
    TK_LOOKAHEAD
} CtTokenKind;

#define CT_MULTILINE_FLAG (1ULL << 63)

typedef struct {
    /* hold the length in bytes because strings can have null bytes in them */
    CtSize len;
    CtSize str;
} CtString;

#define CT_STRLEN(str) (str.len & ~CT_MULTILINE_FLAG)

typedef enum {
    DE_BASE2, DE_BASE10, DE_BASE16
} CtDigitEncoding;

typedef struct {
    CtDigitEncoding enc;
    CtInt num;
    CtString suffix;
} CtDigit;

typedef CtSize CtUserKey;

typedef enum {
#define KEY(id, str) id,
#define OP(id, str) id,
#include "keys.inc"

    K_INVALID
} CtKey;

typedef union {
    /* TK_KEYWORD */
    CtKey key;

    /* TK_USER_KEYWORD */
    CtUserKey ukey;

    /* TK_STRING */
    CtSize len;

    /* TK_CHAR */
    CtSize letter;

    /* TK_INT */
    CtDigit num;

    /* TK_ERROR */
    CtError err;

    /* TK_EOF, TK_LOOKAHEAD, TK_IDENT */
} CtTokenData;

typedef struct CtToken {
    CtTokenKind kind;
    CtTokenData data;
    CtPosition pos;

    /* length of the token for debugging */
    CtSize len;
} CtToken;

#define CT_IDENT(lex, tok) (lex.source.ptr + tok.pos.dist)
#define CT_STRING(lex, tok) (lex.source.ptr + tok.pos.dist)

typedef int(*CtLexerNextFunc)(void*);

typedef struct {
    const char *str;
    CtUserKey id;
} CtUserKeyword;

typedef struct {
    char *ptr;
    CtSize len;
    CtSize alloc;
} CtStrBuffer;

typedef struct {
    /* stream state */
    void *stream;
    CtLexerNextFunc next;
    int ahead;

    /* the entire source file and length for error reporting */
    CtStrBuffer source;


    CtPosition pos;
    CtSize len;

    /**
     * user provided keywords.
     */
    const CtUserKeyword *ukeys;
    CtSize nkeys;

    /* current template depth */
    int depth;

    CtError err;
} CtLexer;

void ctSetKeys(CtLexer *self, const CtUserKeyword *keys, CtSize num);
void ctResetKeys(CtLexer *self);

CtLexer ctLexerNew(void *stream, CtLexerNextFunc next);
void ctLexerFree(CtLexer *self);

CtToken ctLexerNext(CtLexer *self);

typedef struct {
    CtLexer lex;

    /* lookahead token */
    CtToken tok;

    /* error data */
    CtToken errt;
    CtError err;
} CtParser;

typedef enum {
    NT_IDENT,
    NT_ERROR,

    NT_LITERAL,
    NT_UNARY,
    NT_BINARY,
    NT_TERNARY,
    NT_NAME,
    NT_DEREF,
    NT_ACCESS,
    NT_SUBSCRIPT,

    NT_QUALS,
    NT_QUAL,
    NT_PTR,
    NT_REF,
    NT_ARR,
    NT_CLOSURE,
    NT_TPARAM
} CtNodeType;

typedef struct {
    struct CtNode *expr;
    struct CtNode *field;
} CtAccess;

typedef struct {
    struct CtNode *expr;
    struct CtNode *index;
} CtSubscript;

typedef struct {
    struct CtNode *data;
    CtSize len;
    CtSize alloc;
} CtNodeArray;

typedef struct {
    struct CtNode *name;
    CtNodeArray params;
} CtQual;

typedef struct {
    struct CtNode *type;
    struct CtNode *size;
} CtArray;

typedef struct {
    CtNodeArray args;
    struct CtNode *result;
} CtClosure;

typedef struct {
    struct CtNode *name;
    struct CtNode *type;
} CtParam;

typedef struct {
    CtKey op;
    struct CtNode *expr;
} CtUnary;

typedef struct {
    CtKey op;
    struct CtNode *lhs;
    struct CtNode *rhs;
} CtBinary;

typedef struct {
    /* cond ? yes : no; */
    /* cond ?: no; yes = cond */
    struct CtNode *cond;
    struct CtNode *yes;
    struct CtNode *no;
} CtTernary;

typedef union {
    /* NT_IDENT */
    char *ident;

    /* NT_ERROR */
    CtError err;

    /* NT_LITERAL */
    CtToken literal;

    /* NT_TPARAM */
    CtParam tparam;

    /* NT_QUAL */
    CtQual qual;

    /* NT_DEREF, NT_ACCESS */
    CtAccess access;

    /* NT_SUBSCRIPT */
    CtSubscript subscript;

    /* NT_QUALS, NT_NAME */
    CtNodeArray quals;

    /* NT_REF, NT_PTR */
    struct CtNode *type;

    /* NT_ARR */
    CtArray arr;

    /* NT_CLOSURE */
    CtClosure closure;

    /* NT_UNARY */
    CtUnary unary;

    /* NT_BINARY */
    CtBinary binary;

    /* NT_TERNARY */
    CtTernary ternary;
} CtNodeData;

typedef struct CtNode {
    CtNodeType type;
    CtToken tok;
    CtNodeData data;
} CtNode;

CtParser ctParserNew(CtLexer lex);

CtNode *ctParseUnit(CtParser *self);
CtNode *ctParseEval(CtParser *self);

void ctFreeNode(CtNode *node);

#endif /* CTHULHU_H */

#endif /* 0 */
