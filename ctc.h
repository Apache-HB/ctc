#ifndef CTC_H
#define CTC_H

#define CTC_JOIN_INNER(A, B) A ## B
#define CTC_JOIN(A, B) CTC_JOIN_INNER(A, B)
#define CTC_ASSERT(expr, msg) typedef char CTC_JOIN(ASSERT, __LINE__)[(expr) ? 1 : -1]

typedef struct {
    void(*close)(void*);
    int(*next)(void*);
} CtFileCallbacks;

typedef struct {
    struct CtLexer* lex;
    int dist;
    int line;
    int col;
} CtFilePos;

typedef struct CtLexer {
    void* data;
    int ahead;
    CtFileCallbacks* callbacks;
    CtFilePos pos;

    char buffer[512];
    int index;
} CtLexer;

typedef enum {
    TK_IDENT,
    TK_STRING,
    TK_CHAR,
    TK_INT,
    TK_KEY,
    TK_INVALID
} CtTokenKind;

typedef enum {
#define KEY(id, str) id,
#define OP(id, str) id,
#include "key.inc"
    K_INVALID
} CtKey;

typedef unsigned long long CtDigit;
typedef signed int CtChar;

CTC_ASSERT(sizeof(CtChar) == 4, "char must be wide enough for any utf code point");
CTC_ASSERT(sizeof(CtDigit) == 8, "digit must be wide enough for any 64 bit value");

typedef union {
    char* ident;
    CtChar* string;
    CtDigit digit;
    CtChar letter;
    CtKey key;
} CtTokenData;

CTC_ASSERT(sizeof(CtTokenData) == 8, "token data must be 8 bytes wide");

typedef struct {
    CtTokenKind kind;
    CtFilePos pos;
    CtTokenData data;
} CtToken;

CtLexer* ctCreateLexer(void* data, CtFileCallbacks* callbacks);

/**
 * path
 *  : IDENT ('::' IDENT)*
 *  ;
 */

typedef enum {
    /**
     * unit
     *  : (importDecl | decorator)* bodyDecl* EOF
     *  ;
     *
     * bodyDecl
     *  : aliasDecl
     *  | structDecl
     *  | unionDecl
     *  | objectDecl
     *  | functionDecl
     *  | varDecl
     *  | decorator
     *  ;
     */
    AK_UNIT,

    /**
     * importDecl
     *  : 'import' path importSpec? ';'
     *  ;
     *
     * importSpec
     *  : '(' importBody ')'
     *  ;
     *
     * importBody
     *  : IDENT (',' IDENT)*
     *  ;
     */
    AK_IMPORT,

    /**
     * varDecl
     *  : 'var' IDENT ':' type ';'
     *  | 'var' IDENT '=' expr ';'
     *  | 'var' IDENT ':' type '=' expr ';'
     *  ;
     */
    AK_VAR,

    /**
     * aliasDecl
     *  : 'alias' IDENT '=' aliasBody ';'
     *  ;
     *
     * aliasBody
     *  : type
     *  ;
     */
    AK_ALIAS,

    /**
     * type
     *  : typebody templateType?
     *  | type '&'
     *  | type '*'
     *  | type arrayType
     *  | type '::' IDENT
     *  | decorator type
     *  ;
     *
     * typebody
     *  : IDENT
     *  ;
     *
     * arrayType
     *  : '[' (expr | 'var') ']'
     *  ;
     *
     * templateType
     *  : '<' templateTypeBody '>'
     *  ;
     *
     * templateTypeBody
     *  : templateTypeArg (',' templateTypeArg)*
     *  ;
     *
     * templateTypeArg
     *  : expr
     *  | type
     *  ;
     */
    AK_TYPE,

    /**
     * structDecl
     *  : 'struct' IDENT '{' structField* '}'
     *  ;
     *
     * structField
     *  : IDENT ':' type ';'
     *  ;
     */
    AK_STRUCT,

    /**
     * unionDecl
     *  : 'union' IDENT '{' unionField* '}'
     *  ;
     *
     * unionField
     *  : IDENT ':' type ';'
     *  ;
     */
    AK_UNION,

    /**
     * functionDecl
     *  : 'def' IDENT funcArgs? funcRet? funcBody
     *  ;
     *
     * funcArgs
     *  : '(' funcArgsBody? ')'
     *  ;
     *
     * funcArgsBody
     *  : funcArg (',' funcArg)*
     *  ;
     *
     * funcArg
     *  : IDENT ':' type
     *  ;
     *
     * funcRet
     *  : '->' type
     *  ;
     *
     * funcBody
     *  : ';'
     *  | '=' expr ';'
     *  | stmtList
     *  ;
     */
    AK_FUNCTION,

    /**
     * objectDecl
     *  : 'object' IDENT '{' objectField* '}'
     *  ;
     *
     * objectField
     *  : varField
     *  | funcField
     *  | aliasField
     *  | decorator
     *  ;
     *
     * varField
     *  : 'var' IDENT ':' type ('=' expr)? ';'
     *  ;
     *
     * funcField
     *  : 'def' IDENT funcArgs? funcRet? funcBody
     *  ;
     *
     * aliasField
     *  : 'alias' IDENT '=' type ';'
     *  ;
     */
    AK_OBJECT,

    /**
     * decorator
     *  : '@' decoratorItem
     *  | '@' '[' decoratorItem (',' decoratorItem)* ']'
     *  ;
     *
     * decoratorItem
     *  : decoratorName decoratorArgs?
     *  ;
     *
     * decoratorName
     *  : IDENT ('::' IDENT)*
     *  ;
     *
     * decoratorArgs
     *  : '(' decoratorArgBody? ')'
     *  ;
     *
     * decoratorArgBody
     *  : decoratorArg (',' decoratorArgBody)
     *  | decoratorNamedArgs
     *  ;
     *
     * decoratorArg
     *  : expr
     *  ;
     *
     * decoratorNamedArgs
     *  : decoratorNamedArg (',' decoratorNamedArg)*
     *  ;
     *
     * decoratorNamedArg
     *  : IDENT '=' expr
     *  ;
     */
    AK_DECORATOR,

    /**
     * expr
     *  : unop expr
     *  | LITERAL
     *  | STRING
     *  | IDENT
     *  | expr binop expr
     *  | '(' expr ')'
     *  | expr '(' args? ')'
     *  | expr '?' expr? ';' expr
     *  | expr '[' expr ']'
     *  | 'coerce' '<' type '>' '(' expr ')'
     *  ;
     *
     * unop
     *  : '+' | '-' | '!' | '~' | '*' | '&'
     *  ;
     *
     * binop
     *  : '.' | '->' | '::'
     *  | '+' | '+='
     *  | '-' | '-='
     *  | '*' | '*='
     *  | '/' | '/='
     *  | '%' | '%='
     *  | '==' | '!='
     *  | '&' | '&='
     *  | '|' | '|='
     *  | '&&' | '||'
     *  | '<' | '<='
     *  | '>' | '>='
     *  | '^' | '^='
     *  | '<<' | '<<='
     *  | '>>' | '>>='
     *  | '<=>'
     *  ;
     *
     * args
     *  : expr (',' expr)*
     *  ;
     */
    AK_EXPR,

    /**
     * stmt
     *  : expr
     *  | aliasDecl
     *  | varDecl
     *  | stmtList
     *  | returnStmt
     *  | whileStmt
     *  | ifStmt
     *  | forStmt
     *  | 'break' ';'
     *  | 'continue' ';'
     *  ;
     *
     * stmtList
     *  : '{' stmt* '}'
     *  ;
     *
     * returnStmt
     *  : 'return' expr? ';'
     *  ;
     *
     * whileStmt
     *  : 'while' '(' expr ')' stmt
     *  ;
     *
     * ifStmt
     *  : 'if' '(' expr ')' stmt ('else' 'if' '(' expr ')' stmt)* ('else' stmt)?
     *  ;
     *
     * forStmt
     *  : 'for' '(' IDENT ':' expr ')' stmt
     *  | 'for' '(' varDecl? ';' expr? ';' expr? ')' stmt
     *  ;
     */
    AK_STMT
} CtASTKind;

typedef struct CtAST {
    CtASTKind kind;
} CtAST;

CtAST* ctParse(CtLexer* lex);

#endif /* CTC_H */
