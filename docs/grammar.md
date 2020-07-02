# Grammar

## Keywords
`def`, `var`, `struct`, `object`, `operator`, `enum`, `alias`, `union`, `import`, `return`, `while`, `for`, `if`, `else`, `coerce`, `break`, `continue`

## Tokens
`=`, `==`, `!`, `!=`, `+`, `+=`, `-`, `-=`, `*`, `*=`, `/`, `/=`, `%`, `%=`, `^`, `^=`, `|`, `|=`, `&`, `&=`, `||`, `&&`, `<`, `<=`, `>`, `>=`, `<<`, `<<=`, `>>`, `>>=`, `<=>`, `@`, `~`, `[`, `]`, `{`, `}`, `(`, `)`, `.`, `,`, `:`, `::`, `;`, `?`

## Literals

Cthulhu has no builtin support for floating point numbers or booleans.
The concept of null is also not builtin to the compiler and is provided
by the standard library. As such literal parsing is quite easy.

```ebnf
Literal : IntLiteral | StringLiteral | CharLiteral ;
```

```ebnf
IntLiteral : Base10 | Base2 | Base16 ;

Base10 : [0-9]+ ;

Base2 : `0b` [01]+ ;

Base16 : `0x` [0-9a-fA-F]+ ;
```

```ebnf
StringLiteral : SingleString | MultiString ;

StringString : `"` SingleChar* `"` ;

MultiString : `"""` SingleChar* `"""` ;

SingleChar : . ;
```

```ebnf
CharLiteral : `'` SingleChar `'` ;
```

### Toplevel grammar

```ebnf
CompilationUnit : ImportDecl* BodyDecl* ;

ImportDecl : `import` Path ImportSpec? `;` ;

ImportSpec : `(` ImportItems `)` ;

ImportItems : Ident (`,` Ident)* ;

BodyDecl : AliasDecl | StructDecl | UnionDecl | EnumDecl | FunctionDecl | VarDecl ;



AliasDecl : `alias` Ident `=` AliasBody `;` ;

AliasBody : Type ;



StructDecl : `struct` Ident `{` StructField* `}` ;

StructField : Ident `:` Type `;`



UnionDecl : `union` Ident `{` UnionField* `}` ;

UnionField : Ident `:` Type `;`



EnumDecl : `enum` Ident EnumBacking? `{` EnumFields? `}` ;

EnumBacking : `:` Type ;

EnumFields : EnumField (`,` EnumField)* ;

EnumField : Ident `=` Expression ;



FunctionDecl : `def` FunctionName FunctionArgs? FunctionRet? FunctionBody ;

FunctionName : Ident ;

FunctionArgs : `(` FunctionArgsBody? `)` ;

FunctionArgsBody : FunctionArg (',' FunctionArgsBody) | DefaultFunctionArgs ;

FunctionArg : Ident `:` Type ;

DefaultFunctionArgs : DefaultFunctionArg (',' DefaultFunctionArg)* ;

DefaultFunctionArg : Ident `:` Type `=` Expression ;

FunctionRet : `->` Type ;

FunctionBody : StmtList | `=` Expression | `;` ;



VarDecl : `var` VarName VarBody `;` ;

VarName : Ident | `[` VarDestructureBody `]` ;

VarDestructureBody : Ident (`,` Ident)* ;

VarBody : VarType | VarAssign | VarType VarAssign ;

VarType : `:` Type ;

VarAssign : `=` Expression ;



Type : NameType | PointerType | ArrayType | FunctionType ;

NameType : Ident ;

PointerType : `*` Type ;

ArrayType : `[` ArrayTypeBody `]` Type ;

ArrayTypeBody : Expression | `var` ;

FunctionType : `def` `(` FunctionTypeArgs? `)` FunctionTypeTail? ;

FunctionTypeArgs : Type (`,` Type)* ;

FunctionTypeTail : `->` Type ;



Stmt : Expression `;` | StmtList | ReturnStmt | WhileStmt | IfStmt | ForStmt | VarDecl | `break` `;` | `continue` `;` ;

StmtList : `{` Stmt* `}` ;

ReturnStmt : `return` Expression? `;` ;

WhileStmt : `while` Expression Stmt ;



IfStmt : `if` Expression Stmt ElseIfStmt* ElseStmt? ;

ElseIfStmt : `else` `if` Expression Stmt ;

ElseStmt : `else` Stmt ;


ForStmt : `for` (RangeBody | ForBody) Stmt ;

RangeBody : VarName `:` Expression ;

ForBody : `(` VarDecl? `;` Expression `;` Expression `)` ;


Expression : Literal | UnaryExpression | CoerceExpression | SubscriptExpression | `(` Expression `)` | BinaryExpression | TernaryExpression ;


TernaryExpression : Expression `?` Expression? `:` Expression ;

BinaryExpression : Expression BinaryOp Expression ;

SubscriptExpression : Expression `[` Expression `]` ;


CoerceExpression : `coerce` `<` Type `>` `(` Expression `)` ;


UnaryOp : `+` | `-` | `!` | `~` | `&` | `*` ;

UnaryExpression : UnaryOp Expression ;



Ident : [a-zA-Z_][a-zA-Z0-9_]* ;
```