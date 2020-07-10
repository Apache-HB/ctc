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
unionDecl : 'union' Ident '{' field+ '}' ;

enumData : '{' field* '}' ;
enumField : Ident enumData? ('=' expr)? ;
enumFields : enumField (',' enumField)* ;
enumDecl : 'enum' Ident (':' type)? '{' enumFields? '}' ;

funcDecl : 'def' funcName funcArgs? funcTail? funcBody ;
funcName : Ident ;
funcArgs : '(' funcArg (',' funcArg)* ')' ;
funcArg : Ident ':' type ('=' expr)? ;
funcTail : '->' type ;
funcBody : stmtList | '=' expr ';' | ';' ;

varDecl : 'var' varNames ('=' expr) ';' ;
varName : Ident (':' type) ;
varNames : varName | '[' varName (',' varName)* ']' ;

aliasDecl : 'alias' Ident '=' aliasBody ';' ;
aliasBody : type ;

stmt : stmtList | returnStmt | forStmt | whileStmt | ifStmt | varDecl | aliasDecl | funcDecl | expr ';' | ';' ;

// we use <| to solve some abiguity with varNames
forStmt : 'for' varNames '<|' expr stmt ;
whileStmt : 'while' expr stmt ;
ifStmt : 'if' expr stmt elifStmt* elseStmt? ;
elifStmt : 'else' 'if' expr stmt ;
elseStmt : 'else' stmt ;

returnStmt : 'return' expr? ';' ;

stmtList : '{' stmt* '}' ;

type
    : qualType
    | ptrType
    | refType
    | arrType
    | funcType
    ;

// so we use < and > for templates
// this leads to some ambiguity with >> on closing templates
// C++ also suffers from this, as such our lexer is context senstive

// the ':' prefix is there to make parsing less painful
// since Ident is a valid type
typeArg : type | ':' Ident '=' type ;
typeArgs : '<' typeArg (',' typeArg)* '>' ;
qualType : Ident typeArgs? ('::' qualType)* ;

arrType : '[' type (':' expr)? ']' ;
ptrType : '*' type ;
refType : '&' type ;

typeList : type (',' type)* ;
funcTypeArgs : '(' typeList? ')' ;
funcTypeResult : '->' type ;
funcType : 'def' funcTypeArgs? funcTypeResult? ;

expr
    : expr '[' expr ']'
    | expr '.' Ident
    | expr '->' Ident
    | expr call
    | 'coerce' '<' type '>' '(' expr ')'
    | unaryExpr
    ;

arg : expr | '[' Ident ']' '=' expr ;
args : arg (',' arg)* ;
call : '(' args? ')' ;

// lots of nice operator precedence
unaryExpr : ('-' | '~' | '*' | '&') binaryModExpr ;
binaryModExpr : bitshiftExpr ('*' | '/' | '%') bitshiftExpr ;
bitshiftExpr : bitwiseExpr ('<<' | '>>') bitwiseExpr ;
bitwiseExpr : binaryMathExpr ('^' | '|' | '&') binaryMathExpr ;
binaryMathExpr : equalityExpr ('+' | '-') equalityExpr ;
equalityExpr : compareExpr ('==' | '!=') compareExpr ;
compareExpr : logicExpr ('>=' | '>' | '<=' | '<') logicExpr ;
logicExpr : ternaryExpr ('&&' | '||') ternaryExpr ;
ternaryExpr : sideExpr '?' sideExpr? ':' sideExpr ;
sideExpr : atom ('=' | '*=' | '/=' | '%=' | '+=' | '-=' | '<<=' | '>>=' | '&=' | '^=' | '|=') atom;

initArg : expr | '[' expr ']' '=' expr ;
initArgs : initArg (',' initArg)* ;
init : '{' initArgs? '}' ;

atom : IntLiteral | StringLiteral | CharLiteral | qualType init? | '(' expr ')' ;


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
