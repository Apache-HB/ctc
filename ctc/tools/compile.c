#include <stdio.h>

#include "ctc/ctc_impl.h"

static int posixNext(void* stream) { return fgetc(stream); }

typedef enum {
    CCT_I8086,
    CCT_X86,
    CCT_X64
} CtCompileTarget;

typedef struct HashEntry {
    char* key;
    int used;
    void* data;

    HashEntry* next;
} HashEntry;

typedef struct {
    int size;
    HashEntry* data;
} HashMap;

static HashMap hashNew(int size)
{
    HashMap map;
    map.size = size;
    map.data = malloc(sizeof(HashEntry) * size);

    for (int i = 0; i < size; i++)
    {
        map.data[i].used = 0;
        map.data[i].next = NULL;
    }

    return map;
}

static void hashAdd(HashMap* self, char* key, void* data)
{

}

static void hashGet(HashMap* self, char* key)
{

}

typedef struct {
    FILE* out;
    CtCompileTarget target;
} CtCompile;

typedef struct {
    int sign;
    int width;
} CtBuiltinType;

typedef struct {
    int len;
    struct CtType* of;
} CtArrayType;

typedef struct CtType {
    enum { CTT_BUILTIN, CTT_PTR, CTT_ARR } type;

    union {
        CtBuiltinType builtin;
        struct CtType* ptr;
        CtArrayType arr;
    };
} CtType;

static CtType* resolveType(CtCompile* self, CtAST* type)
{

}

static size_t typeSize(CtCompile* self, CtAST* type)
{
    
}

/**
 * calculate required stack size for function
 */
static size_t stackSize(CtCompile* self, CtAST* func)
{
    size_t size = 0;
}

static void compileUnit(CtCompile* self, CtAST* unit)
{
    for (size_t i = 0; i < unit->data.unit.symbols.len; i++)
    {

    }
}

int main(int argc, const char** argv)
{
    (void)argc;

    CtStreamCallbacks callbacks = { posixNext };
    CtStream stream = ctStreamOpen(callbacks, fopen(argv[1], "r"), argv[1]);
    CtLexer lex = ctLexOpen(&stream);
    CtParser parse = ctParseOpen(&lex);

    CtCompile compile;
    compile.target = CCT_I8086;
    compile.out = fopen("out.asm", "wt");

    CtAST* unit = ctParseUnit(&parse);

    compileUnit(&compile, unit);

    return 0;
}
