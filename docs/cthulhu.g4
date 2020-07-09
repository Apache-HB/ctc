grammar cthulhu;

// compiling a file
unit : importDecl* bodyDecl* EOF ;

// interpreting a file
interp : (importDecl | bodyDecl | stmt)* EOF ;

importDecl : 'import' importPath importSpec? ';' ;

importPath : Ident ('::' Ident)* ;
importSpec : '(' Ident (',' Ident)* ')' ;

bodyDecl : funcDecl | varDecl | aliasDecl | structDecl | unionDecl | enumDecl | objectDecl ;

objectField : (varDecl | aliasDecl | funcDecl) ;
objectDecl : 'object' Ident '{' objectField* '}' ;

field : Ident ':' type ';' ;

structDecl : 'struct' Ident '{' field* '}' ;
unionDecl : 'union' Ident '{' field* '}' ;

enumData : '{' field* '}' ;
enumField : Ident enumData? ('=' expr)? ;
enumFields : enumField (',' enumField)* ;
enumDecl : 'enum' Ident (':' type)? '{' enumFields? '}' ;

funcDecl : 'def' funcName funcArgs? funcTail? funcBody ;
funcName : Ident ;
funcArgs : '(' funcArgsBody? ')' ;
funcArgsBody : funcArg (',' funcArgsBody) | defaultFuncArgs ;
funcArg : Ident ':' type ;
defaultFuncArgs : defaultFuncArg (',' defaultFuncArg)*;
defaultFuncArg : Ident ':' type '=' expr ;
funcTail : '->' type ;
funcBody : stmtList | '=' expr ';' | ';' ;

varDecl : 'var' varNames ('=' expr) ';' ;
varName : Ident (':' type) ;
varNames : varName | '[' varName (',' varName)* ']' ;

aliasDecl : 'alias' Ident '=' aliasBody ';' ;
aliasBody : type ;

stmt : expr ';' | stmtList | varDecl | aliasDecl | returnStmt | forStmt | whileStmt | ifStmt | ';' | funcDecl ;

returnStmt : 'return' expr? ';' ;

forStmt : 'for' (forRange | forLoop) stmt ;
forRange : varNames ':' expr ;
forLoop : varDecl? ';' expr? ';' expr? ';' ;

whileStmt : 'while' expr stmt ;

ifStmt : 'if' expr stmt elifStmt* elseStmt? ;
elifStmt : 'else' 'if' expr stmt ;
elseStmt : 'else' stmt ;

stmtList : '{' stmt* '}' ;

type
    : nameType
    | ptrType
    | arrayType
    | funcType
    | refType
    ;

nameType : Ident ('::' Ident)* ;
ptrType : '*' type ;
refType : '&' type ;
arrayType : '[' type (':' expr)? ']' ;
funcType : 'def' '(' funcTypeArgs? ')' funcTypeTail? ;
funcTypeArgs : type (',' type)* ;
funcTypeTail : '->' type ;

expr
    : (IntLiteral | StringLiteral | CharLiteral | unaryExpr | nameExpr | coerceExpr | '(' expr ')' | compoundExpr) (binaryExpr | ternaryExpr | subscriptExpr | accessExpr | derefExpr | callExpr initExpr? | initExpr)*
    | initExpr
    ;

coerceExpr : 'coerce' '<' type '>' '(' expr ')' ;

unaryOp : '+' | '-' | '!' | '~' | '&' | '*' ;
binaryOp
    : '+' | '+='
    | '-' | '-='
    | '*' | '*='
    | '/' | '/='
    | '%' | '%='
    | '&' | '&='
    | '|' | '|='
    | '^' | '^='
    | '<<' | '<<='
    | '>>' | '>>='
    | '||' | '&&'
    | '<' | '<='
    | '>' | '>='
    | '!=' | '=='
    | '='
    ;

compoundExpr : '[' expr (',' expr)* ']' ;
nameExpr : Ident ('::' Ident) ;
unaryExpr : unaryOp expr ;
binaryExpr : binaryOp expr ;
ternaryExpr : '?' expr? ':' expr ;
subscriptExpr : '[' expr ']' ;
callExpr : '(' callArgs? ')' ;
callArgs : expr (',' callArgs)? | namedArgs ;
namedArgs : namedArg (',' namedArg)* ;
namedArg : '[' Ident ']' '=' expr ;
initExpr : '{' initBody? '}' ;
initBody : expr (',' initBody)? | namedInitBody ;
namedInitBody : namedInitArg (',' namedInitArg)* ;
namedInitArg : '[' expr ']' '=' expr ;
accessExpr : '.' Ident ;
derefExpr : '->' Ident ;

IntLiteral : (Base10 | Base2 | Base16) Ident? ;
StringLiteral : Ident? (SingleString | MultiString) ;
CharLiteral : '\'' Letter '\'' ;

Ident : [a-zA-Z_][a-zA-Z0-9_]* ;

fragment SingleString : '"' Letter* '"' ;
fragment MultiString : '"""' (Letter | '\n')* '"""' ;

fragment Letter : EscapeSequence | ~[\\\r\n] ;

fragment EscapeSequence : '\\' ['"abfnrtv\\] ;

fragment Base10 : [0-9]+ ;
fragment Base2 : '0b' [01]+ ;
fragment Base16 : '0x' [0-9a-fA-F]+ ;
