grammar cthulhu;

unit : importDecl* bodyDecl* EOF ;

importDecl : 'import' importPath importSpec? ';' ;

importPath : Ident ('::' Ident)* ;
importSpec : '(' Ident (',' Ident)* ')' ;

bodyDecl : decorator* (funcDecl | varDecl | aliasDecl | structDecl | unionDecl | enumDecl | objectDecl) ;

decorator : '@' decoratorItem | '@' '[' decoratorItem (',' decoratorItem) ']' ;
decoratorItem : decoratorName callExpr? ;
decoratorName : Ident ('::' Ident)* ;

objectField : decorator* (varDecl | aliasDecl | funcDecl) ;
objectDecl : 'object' Ident '{' objectField* '}' ;

structField : Ident ':' type ';' ;
structDecl : 'struct' Ident '{' structField* '}' ;

unionField : Ident ':' type ';' ;
unionDecl : 'union' Ident '{' unionField* '}' ;

enumField : Ident ('=' expr)? ;
enumFields : enumField (',' enumField)* ;
enumBacking : ':' type ;
enumDecl : 'enum' Ident enumBacking? '{' enumFields? '}' ;

funcDecl : 'def' funcName funcArgs? funcTail? funcBody ;
funcName : Ident ;
funcArgs : '(' funcArgsBody? ')' ;
funcArgsBody : funcArg (',' funcArgsBody) | defaultFuncArgs ;
funcArg : Ident ':' type ;
defaultFuncArgs : defaultFuncArg (',' defaultFuncArg)*;
defaultFuncArg : Ident ':' type '=' expr ;
funcTail : '->' type ;
funcBody : stmtList | '=' expr ';' | ';' ;

varDecl : 'var' varNames varBody ';' ;
varNames : Ident | '[' Ident (',' Ident)* ']' ;
varBody : ':' type | (':' type)? '=' expr ;

aliasDecl : 'alias' Ident '=' aliasBody ';' ;
aliasBody : type ;

stmt : expr ';' | stmtList | varDecl | aliasDecl | returnStmt | forStmt | whileStmt | ifStmt | ';' ;

returnStmt : 'return' expr? ';' ;

forStmt : 'for' (forRange | forLoop) stmt ;
forRange : varNames ':' expr ;
forLoop : '(' varDecl? ';' expr? ';' expr? ')' ;


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
    ;

nameType : Ident ('::' Ident)* ;
ptrType : '*' type ;
arrayType : '[' type ':' arrayTypeBody ']' ;
arrayTypeBody : 'var' | expr ;
funcType : 'def' '(' funcTypeArgs? ')' funcTypeTail? ;
funcTypeArgs : type (',' type)* ;
funcTypeTail : '->' type ;

expr
    : (IntLiteral | StringLiteral | unaryExpr | nameExpr | coerceExpr | '(' expr ')' | compoundExpr) (binaryExpr | ternaryExpr | subscriptExpr | accessExpr | derefExpr | callExpr | initExpr)*
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

IntLiteral : Base10 | Base2 | Base16 ;
StringLiteral : SingleString | MultiString ;

Ident : [a-zA-Z_][a-zA-Z0-9_]* ;

fragment SingleString : '"' Letter* '"' ;
fragment MultiString : '"""' (Letter | '\n')* '"""' ;

fragment Letter : EscapeSequence | ~[\\\r\n] ;

fragment EscapeSequence : '\\' ['"abfnrtv\\] ;

fragment Base10 : [0-9]+ ;
fragment Base2 : '0b' [01]+ ;
fragment Base16 : '0x' [0-9a-fA-F]+ ;
