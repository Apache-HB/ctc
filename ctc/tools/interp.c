#include <stdio.h>

#include "ctc/ctc_impl.h"

#include <signal.h>

static int posixNext(void* stream) { return fgetc(stream); }

typedef enum {
    OT_FUNCTION
} CtObjectType;

typedef struct {
    char* key;
    int used;
    void* data;
} HashEntry;

typedef struct {
    size_t size;
    HashEntry* data;
} CtMap;

typedef struct {

} CtFunction;

typedef struct {
    CtMap fields;
} CtObjectData;

typedef struct CtObject {
    CtObjectType type;
    CtObjectData data;
} CtObject;

typedef struct CtScope {
    /* symbols in the current scope */
    CtMap symbols;

    /* scopes inside this scope */
    CtMap scopes;
} CtScope;

typedef struct {

} CtInterp;

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    CtStreamCallbacks callbacks = { posixNext };
    CtStream stream = ctStreamOpen(callbacks, stdin, "stdin");
    CtLexer lex = ctLexOpen(&stream);
    CtParser parse = ctParseOpen(&lex);

    return 0;
}
