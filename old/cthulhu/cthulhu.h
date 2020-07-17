#ifndef CTC_H
#define CTC_H

#include <stddef.h>

typedef struct {
    /* user data */
    void* data;

    void*(*alloc_func)(void*, size_t);
    void(*free_func)(void*, void*);
} CtAllocator;

typedef struct {
    /* filename */
    const char* name;

    /**
     * absolute offset, line, and column
     * of token
     */
    int pos;
    int line;
    int col;

    /* the length of the token */
    int len;
} CtStreamPos;

typedef struct {
    /* get the next character from a stream, return -1 on EOF */
    int(*next)(void*);
} CtStreamCallbacks;

typedef struct CtStream {
    /**
     * function pointers for opening, closing,
     * and reading the next character from a stream
     */
    CtStreamCallbacks callbacks;

    /* file stream data */
    void* data;

    /* lookahead character */
    int ahead;

    /* name of the file */
    const char* name;

    /* current position */
    CtStreamPos pos;
} CtStream;

typedef enum {
    TK_IDENT,
    TK_STRING,
    TK_CHAR,
    TK_INT,
    TK_KEYWORD,
    TK_ERROR,
    TK_EOF,

    /* internal token kind */
    TK_INVALID
} CtTokenKind;

typedef unsigned long long CtDigit;
typedef unsigned long long CtChar;

typedef enum {
#define OP(id, str) id,
#define KEY(id, str) id,
#include "keys.inc"

    /* invalid keyword, should never be used */
    K_INVALID
} CtKeyword;

typedef struct {
    char* string;
    size_t len;
    char* prefix;
} CtString;

typedef enum {
    NE_INT,
    NE_HEX,
    NE_BIN
} CtNumberEncoding;

typedef struct {
    CtDigit digit;
    char* suffix;
    CtNumberEncoding enc;
} CtNumber;

typedef union {
    /* TK_IDENT */
    char* ident;

    /* TK_STRING */
    CtString string;

    /* TK_INT */
    CtNumber digit;

    /* TK_KEYWORD */
    CtKeyword key;

    /* TK_CHAR */
    CtChar letter;

    /* TK_ERROR */
    char* reason;

    /* TK_EOF */
} CtTokenData;

typedef struct {
    CtTokenKind kind;
    CtTokenData data;
    CtStreamPos pos;
} CtToken;

/* cleanup a token */
void ctFreeToken(CtToken tok);

CtStream ctStreamOpen(CtStreamCallbacks callbacks, void* data, const char* path);

typedef struct {
    CtStream* stream;
    char* err;

    /* used for tracking the length of the current token */
    int len;

    int depth;
} CtLexer;

/* open a lexing stream */
CtLexer ctLexOpen(CtStream* stream);

/* get a single token from the stream */
CtToken ctLexNext(CtLexer* self);

/* use these for handling context sensitive lexing in templates */
void ctLexBeginTemplate(CtLexer* self);
void ctLexEndTemplate(CtLexer* self);

typedef enum {
    /* a single identifier */
    AK_IDENT,

    /* a raw token, used for custom parsing */
    AK_TOKEN,

    /* a type is always an array of qualified types */
    AK_TYPE,

    AK_QUAL,
    AK_REF,
    AK_PTR,
    AK_ARR,
    AK_FUNC_TYPE,

    AK_TYPE_PARAM,

    /* expressions */
    AK_LITERAL,
    AK_UNARY,
    AK_BINOP,
    AK_NAME,
    AK_ACCESS,
    AK_DEREF,
    AK_TERNARY,
    AK_COERCE,
    AK_INIT,
    AK_CALL,
    AK_INIT_ARG,
    AK_CALL_ARG,
    AK_SUBSCRIPT,
    AK_CLOSURE,
    AK_CAPTURE,

    /* toplevel declarations */
    AK_IMPORT,
    AK_ALIAS,
    AK_STRUCT,
    AK_UNION,
    AK_ENUM,
    AK_VAR,
    AK_FUNCTION,
    AK_ATTRIB,
    AK_BUILTIN,

    AK_FIELD,
    AK_ENUM_FIELD,
    AK_VAR_NAME,
    AK_FUNC_ARG,

    AK_STMT_LIST,
    AK_RETURN,

    /* if, else if, else */
    AK_BRANCH,
    AK_BRANCH_LEAF,
    AK_WHILE,
    AK_FOR,
    AK_DO_WHILE,

    AK_UNIT
} CtASTKind;

typedef struct {
    struct CtAST* data;
    size_t alloc;
    size_t len;
} CtASTArray;

typedef struct {
    CtKeyword op;
    struct CtAST* lhs;
    struct CtAST* rhs;
} CtASTBinop;

typedef struct {
    struct CtAST* type;
    struct CtAST* size;
} CtASTArr;

typedef struct {
    CtASTArray args;
    struct CtAST* result;
} CtASTFuncType;

typedef struct {
    struct CtAST* name;
    CtASTArray params;
} CtASTQual;

typedef struct {
    struct CtAST* name;
    struct CtAST* type;
} CtASTTypeParam;

typedef struct {
    struct CtAST* expr;
    struct CtAST* field;
} CtASTAccess;

typedef struct {
    struct CtAST* cond;
    struct CtAST* truthy;
    struct CtAST* falsey;
} CtASTTernary;

typedef struct {
    struct CtAST* type;
    struct CtAST* expr;
} CtASTCast;

typedef struct {
    struct CtAST* name;
    struct CtAST* body;
} CtASTAlias;

typedef struct {
    struct CtAST* name;
    struct CtAST* type;
} CtASTField;

typedef struct {
    struct CtAST* name;
    CtASTArray fields;

    CtASTArray attribs;
} CtASTStruct;

typedef struct {
    struct CtAST* name;
    CtASTArray fields;

    CtASTArray attribs;
} CtASTUnion;

typedef struct {
    struct CtAST* name;
    struct CtAST* backing;
    CtASTArray fields;

    CtASTArray attribs;
} CtASTEnum;

typedef struct {
    struct CtAST* name;
    CtASTArray fields;
    struct CtAST* val;
} CtASTEnumField;

typedef struct {
    CtASTArray names;
    struct CtAST* init;

    CtASTArray attribs;
} CtASTVar;

typedef struct {
    struct CtAST* name;
    struct CtAST* type;
} CtASTVarName;

typedef struct {
    struct CtAST* name;
    struct CtAST* type;
    struct CtAST* init;
} CtASTFuncArg;

typedef struct {
    struct CtAST* name;
    CtASTArray args;
    struct CtAST* result;
    struct CtAST* body;

    CtASTArray attribs;
} CtASTFunction;

typedef enum {
    ACT_REF,
    ACT_VALUE
} CtASTCaptureType;

typedef struct {
    struct CtAST* symbol;
    CtASTCaptureType type;
} CtASTCapture;

typedef struct {
    struct CtAST* result;
    CtASTArray args;
    CtASTArray captures;
    struct CtAST* body;
} CtASTClosure;

typedef struct {
    struct CtAST* cond;
    struct CtAST* body;
} CtASTBranchLeaf;

typedef struct {
    CtASTArray names;
    struct CtAST* range;
    struct CtAST* body;
} CtASTForStmt;

typedef struct {
    struct CtAST* cond;
    struct CtAST* body;
} CtASTWhileStmt;

typedef struct {
    struct CtAST* func;
    CtASTArray args;
} CtASTCall;

typedef struct {
    struct CtAST* name;
    struct CtAST* val;
} CtASTCallArg;

typedef struct {
    struct CtAST* type;
    CtASTArray args;
} CtASTInit;

typedef struct {
    struct CtAST* field;
    struct CtAST* val;
} CtASTInitArg;

typedef struct {
    struct CtAST* expr;
    struct CtAST* idx;
} CtASTSubscript;

typedef struct {
    CtASTArray path;
    CtASTArray symbols;
} CtASTImport;

typedef struct {
    CtASTArray name;
    CtASTArray args;
} CtASTAttrib;

typedef struct {
    CtASTArray name;

    /* custom data provided by implementation parsers */
    void* body;
    void* args;
} CtASTBuiltin;

typedef struct {
    CtASTArray imports;
    CtASTArray symbols;
} CtASTUnit;

typedef union {
    /* AK_IDENT */
    char* ident;

    /* AK_UNARY, AK_RETURN */
    struct CtAST* expr;

    /* AK_NAME */
    CtASTArray names;

    /* AK_ACCESS */
    CtASTAccess access;

    /* AK_DEREF */
    CtASTAccess deref;

    /* AK_BINOP */
    CtASTBinop binop;

    /* AK_TERNARY */
    CtASTTernary ternary;

    /* AK_COERCE */
    CtASTCast cast;

    /* AK_TYPE */
    CtASTArray types;

    /* AK_TYPE_PARAM */
    CtASTTypeParam param;

    /* AK_FUNC_TYPE */
    CtASTFuncType sig;

    /* AK_QUAL */
    CtASTQual qual;

    /* AK_ARR */
    CtASTArr arr;

    /* AK_REF */
    CtASTArray ref;

    /* AK_PTR */
    struct CtAST* ptr;

    /* AK_ALIAS */
    CtASTAlias alias;

    /* AK_STRUCT */
    CtASTStruct struc;

    /* AK_UNION */
    CtASTUnion uni;

    /* AK_ENUM */
    CtASTEnum enu;

    /* AK_ENUM_FIELD */
    CtASTEnumField efield;

    /* AK_FIELD */
    CtASTField field;

    /* AK_VAR */
    CtASTVar var;

    /* AK_VAR_NAME */
    CtASTVarName vname;

    /* AK_FUNCTION */
    CtASTFunction func;

    /* AK_FUNC_ARG */
    CtASTFuncArg arg;

    /* AK_STMT_LIST */
    CtASTArray stmts;

    /* AK_BRANCH */
    CtASTArray branches;

    /* AK_BRANCH_LEAF */
    CtASTBranchLeaf leaf;

    /* AK_FOR */
    CtASTForStmt loop;

    /* AK_WHILE, AK_DO_WHILE */
    CtASTWhileStmt wloop;

    /* AK_CALL */
    CtASTCall call;

    /* AK_CALL_ARG */
    CtASTCallArg carg;

    /* AK_INIT */
    CtASTInit init;

    /* AK_INIT_ARG */
    CtASTInitArg iarg;

    /* AK_SUBSCRIPT */
    CtASTSubscript subscript;

    /* AK_ATTRIB */
    CtASTAttrib attrib;

    /* AK_BUILTIN */
    CtASTBuiltin builtin;

    /* AK_CLOSURE */
    CtASTClosure closure;

    /* AK_CAPTURE */
    CtASTCapture capture;

    /* AK_IMPORT, we name it include because import is a c++20 keyword */
    CtASTImport include;

    /* AK_UNIT */
    CtASTUnit unit;
} CtASTData;

typedef struct CtAST {
    CtASTKind kind;
    CtASTData data;

    /* token related to this ast, used mainly for error reporting */
    CtToken tok;
} CtAST;

struct CtParser;

/* handle custom parsing sections */
typedef struct {
    /* the builtin name to match */
    const char* name;

    /* the function to use for custom parsing */
    void*(*func)(struct CtParser*);
} CtParseHandle;

typedef struct CtParser {
    CtLexer* source;
    CtToken tok;

    /* the current array of attributes to add to the next ast node */
    CtASTArray attribs;

    int n_handles;
    CtParseHandle* handles;

    /* an error message if there is one */
    char* err;
    /* the current node being parsed */
    CtAST* node;
} CtParser;

CtParser ctParseOpen(CtLexer* source);

CtAST* ctParseUnit(CtParser* self);

#endif /* CTC_H */
