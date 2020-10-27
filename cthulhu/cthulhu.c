#include "cthulhu.h"

#include <string.h>
#include <ctype.h>

#ifndef CT_MALLOC
#   error "CT_MALLOC must be defined"
#endif

#ifndef CT_FREE
#   error "CT_FREE must be defined"
#endif

#ifndef CT_REALLOC
#   error "CT_REALLOC must be defined"
#endif

#if !defined(CT_MM_SMALL) && !defined(CT_MM_FAST)
#   error "either CT_MM_SMALL or CT_MM_FAST must be defined"
#elif defined(CT_MM_SMALL) && defined(CT_MM_FAST)
#   error "CT_MM_SMALL and CT_MM_FAST must not both be defined"
#endif

#if defined(CT_MM_SMALL)
#   if !defined(CT_MM_STEP)
#       define CT_MM_STEP 0x1000
#   endif
#endif

#if !defined(CT_MM_INIT_SIZE)
#   define CT_MM_INIT_SIZE 0x1000
#endif

#define CT_UNUSED(it) (void)it

#ifdef CT_MM_SMALL
static CtBuffer lexbuf;
#endif

void ctInit() {
#ifdef CT_MM_SMALL
    lexbuf = ctBufferAlloc(CT_MM_INIT_SIZE);
#endif
}

void ctFree() {
#ifdef CT_MM_SMALL
    ctBufferFree(lexbuf);
#endif
}

#ifdef CT_MM_SMALL
#   define LEXBUF(lex) (&lexbuf)
#else
#   define LEXBUF(lex) (&lex->buf)
#endif


void ctPush(CtBuffer *self, char c) {
    if (self->len + 1 >= self->size) {
#if defined(CT_MM_SMALL)
        self->size += CT_MM_STEP;
#elif defined(CT_MM_FAST)
        self->size *= 2;
#endif
        self->ptr = CT_REALLOC(self->ptr, self->size);
    }

    self->ptr[self->len++] = c;
}

CtBuffer ctBufferAlloc(size_t init) {
    CtBuffer self = {
        .ptr = CT_MALLOC(init),
        .size = init,
        .len = 0
    };

    return self;
}

size_t ctOffset(CtBuffer *self) {
    return self->len;
}

void ctBufferFree(CtBuffer self) {
    CT_FREE(self.ptr);
}

const char *ctAt(CtBuffer *self, size_t off) {
    return self->ptr + off;
}

CtStream ctStreamAlloc(void *stream, char(*fun)(void*)) {
    CtStream self = {
        .stream = stream,
        .get = fun,
        .ahead = fun(stream),
        .buffer = ctBufferAlloc(CT_MM_INIT_SIZE),
        .where = { 0, 0, 0 }
    };

    return self;
}

void ctStreamFree(CtStream self) {
    ctBufferFree(self.buffer);
}

char ctNext(CtStream *self) {
    char c = self->ahead;
    self->ahead = self->get(self->stream);

    self->where.offset += 1;

    if (c == '\n') {
        self->where.line += 1;
        self->where.col = 0;
    } else {
        self->where.col += 1;
    }

    return c;
}

char ctPeek(CtStream *self) {
    return self->ahead;
}

bool ctEat(CtStream *self, char c) {
    if (ctPeek(self) == c) {
        ctNext(self);
        return true;
    }
    return false;
}

CtWhere ctHere(CtStream *self) {
    return self->where;
}

static size_t sourceOffset(CtStream *self) {
    return self->where.offset;
}

CtLex ctLexAlloc(CtStream *source) {
    CtLex lex = {
        .source = source
    };

#ifdef CT_MM_FAST
    lex.buf = ctBufferAlloc(CT_MM_INIT_SIZE);
#endif

    return lex;
}

void ctLexFree(CtLex self) {
    (void)self;

#ifdef CT_MM_FAST
    ctBufferFree(self.buf);
#endif
}

/* returns the offset of the buffer */
static size_t lexBegin(CtLex *self) {
    CT_UNUSED(self);
    return ctOffset(LEXBUF(self));
}

static void lexPush(CtLex *self, char c) {
    CT_UNUSED(self);
    ctPush(LEXBUF(self), c);
}

/* returns the end of the string and null terminates it */
static size_t lexEnd(CtLex *self) {
    CT_UNUSED(self);
    size_t offset = ctOffset(LEXBUF(self));
    ctPush(LEXBUF(self), 0);
    return offset;
}

static CtRange newRange(CtWhere where) {
    CtRange self = {
        .offset = where.offset,
        .line = where.line,
        .col = where.col,
        .len = 0
    };

    return self;
}

static bool isident1(char c) { return isalpha(c) || c == '_'; }
static bool isident2(char c) { return isalnum(c) || c == '_'; }

typedef struct { const char *str; CtKey id; } KeyPair;

KeyPair keys[] = {
#define KEY(id, str) { str, id },
#include "keys.h"
    { "", K_INVALID }
};

static void lexIdent(CtLex *self, char c, CtToken *tok) {
    size_t start = lexBegin(self);
    size_t end;
    size_t len;

    lexPush(self, c);

    while (isident2(ctPeek(self->source))) {
        lexPush(self, ctNext(self->source));
    }

    end = lexEnd(self);
    len = end - start;

    for (size_t i = 0; i < sizeof(keys) / sizeof(KeyPair); i++) {
        if (strncmp(keys[i].str, ctAt(LEXBUF(self), start), len + 1) == 0) {
            tok->kind = TK_KEY;
            tok->data.key = keys[i].id;
            return;
        }
    }

    tok->kind = TK_IDENT;
}

void lexDigit(CtLex *self, char c, CtToken *tok) {

}

CtKey lexKey(CtLex *self, char c) {
    switch (c) {
    case '(': return K_LPAREN;
    case ')': return K_RPAREN;
    case ';': return K_SEMI;
    default: return K_INVALID;
    }
}

void lexSymbol(CtLex *self, char c, CtToken *tok) {
    CtKey key = lexKey(self, c);
    if (key == K_INVALID) {
        tok->kind = TK_INVALID;
    } else {
        tok->kind = TK_KEY;
        tok->data.key = key;
    }
}

CtToken ctLexNext(CtLex *self) {
    char c = ctNext(self->source);
    CtToken tok = { TK_INVALID };
    CtRange here = newRange(ctHere(self->source));

    if (c == 0) {
        tok.kind = TK_END;
    } else if (isident1(c)) {
        lexIdent(self, c, &tok);
    } else if (isdigit(c)) {
        lexDigit(self, c, &tok);
    } else {
        lexSymbol(self, c, &tok);
    }


    here.len = sourceOffset(self->source) - here.offset;
    tok.where = here;

    return tok;
}