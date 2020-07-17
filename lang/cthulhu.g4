grammar cthulhu;

unit : include* body* EOF ;

include : 'import' path symbols? SEMI ;
symbols : '(' items ')' ;

attrib : path call? ;
attribs : '@' '[' attrib (',' attrib)* ']' ;

builtinBody : '{' /* the contents of builtinBody are IDB */ '}' ;
builtin : '@' path call? builtinBody? ;

body : function | alias | var | struct | union | enum | object | builtin ;

bound : path ;
bounds : bound ('+' bound)* ;
templateArg : Ident (':' bounds)? ('=' type)? ;
template : '!<' templateArg (',' templateArg)* '>' ;

field : attrib* Ident ':' type ;

objectField : function | alias | var | builtin ;

implements : '(' path (',' path)* ')' ;
object : attribs* 'object' template? implements? '{' objectField* '}' ;

enumFieldData : Ident ':' type ;
enumData : '{' enumFieldData (',' enumFieldData)* '}' ;
enumField : Ident enumData? ('=' expr)? ;
enumBody : enumField (',' enumField)* ;
enum : attribs* 'enum' Ident template? (':' type)? '{' enumBody? '}' ;
union : attribs* 'union' Ident template? '{' field* '}' ;
struct : attribs* 'struct' Ident template? '{' field* '}' ;

varName : Ident (':' type) ;
varNames : varName | '[' varName (',' varName)* ']' ;

var : attribs* 'var' varNames ('=' expr)? SEMI ;

alias : attribs* 'alias' Ident '=' type SEMI ;

parameter : Ident ':' type ('=' expr)? ;
parameterBody : parameter (',' parameter)* ;
parameters : '(' parameterBody? ')' ;
result : '->' type ;

functionBody : SEMI | '=' expr SEMI | stmts ;
function : attribs* 'def' Ident parameters? result? functionBody ;

type : attribs* (ptr | ref | quals | array | signature) ;

ptr : '*' ptr ;
ref : '&' quals ;
array : '[' type (':' expr)? ']' ;

tparam : type | ':' Ident '=' type ;
qual : Ident ('!<' tparam (',' tparam)* '>')? ;
quals : qual ('::' qual)* ;

types : type (',' type)* ;
typelist : '(' types? ')' ;
signature : 'def' typelist? result? ;


expr : assign ;

assign : ternary (('=' | '+=' | '-=' | '*=' | '/=' | '%=' | '^=' | '&=' | '|=' | '<<=' | '>>=') ternary)* ;
ternary : logic ('?' primary ':' ternary)? ;
logic : equality (('&&' | '||') equality)* ;
equality : compare (('==' | '!=') compare)* ;
compare : bitwise (('<=' | '<' | '>=' | '>') bitwise)* ;
bitwise : bitshift (('^' | '&' | '|') bitshift)* ;
bitshift : arithmatic (('<<' | '>>') arithmatic)* ;
arithmatic : multiplicative (('+' | '-') multiplicative)* ;
multiplicative : unary (('*' | '/' | '%') unary)* ;
unary : ('+' | '-' | '~' | '!')? postfix ;

postfix
    : primary
    | postfix '[' expr ']'
    | postfix call
    | postfix '.' Ident
    | postfix '->' Ident
    ;

primary
    : IntLiteral
    | StringLiteral
    | CharLiteral
    | closure
    | coerce
    | quals init?
    | init
    | '(' expr ')'
    ;

initArg : expr | '[' (expr | 'else') ']' '=' expr ;
initArgs : initArg (',' initArg)* ;
init : '{' initArgs? '}' ;

arg : expr | '[' Ident ']' '=' expr ;
callArgs : arg (',' arg)* ;
call : '(' callArgs? ')' ;

coerce : 'coerce' '!<' type '>' '(' expr ')' ;

cparam : Ident (':' type)? ;
cparams : '(' (cparam (',' cparam)*)? ')' ;
lambda : '=>' expr | stmts ;
closure : 'def' cparams? result? lambda;


stmt : stmts | expr | var | return | alias | for | while | if | branch | builtin | 'break' SEMI | 'continue' SEMI | SEMI ;

if : 'if' '(' expr ')' stmt ('else' 'if' '(' expr ')' stmt)* ('else' stmt)? ;
while : 'while' '(' expr ')' stmt ;

branch : 'branch' '(' expr ')' '{' branches* '}' ;
branches : case '=>' stmt ;
case : expr varNames? ;

forBody : var SEMI expr? SEMI expr? | varNames '<|' expr ;
for : 'for' '(' forBody ')' stmt ;

return : 'return' expr? SEMI ;

stmts : '{' stmt* '}' ;

items : Ident (',' Ident)* ;
path : Ident ('::' Ident)* ;

SEMI : ';' ;

IntLiteral : (Base2 | Base10 | Base16) Ident? ;
StringLiteral : SingleString | MultiString ;
CharLiteral : '\'' Letter '\'' ;

Ident : [a-zA-Z_][a-zA-Z0-9_]* ;

fragment SingleString : '"' Letter* '"' ;
fragment MultiString : 'R"' .*? '"' ;

fragment Letter : '\\' ['"ntv0\\] | ~[\\\r\n] ;

fragment Base2 : '0b' [01]+ ;
fragment Base10 : [0-9]+ ;
fragment Base16 : '0x' [0-9a-fA-F]+ ;