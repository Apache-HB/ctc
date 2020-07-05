#ifndef OP
#   define OP(id, str)
#endif

#ifndef KEY
#   define KEY(id, str)
#endif

/* language keywords */
KEY(K_IMPORT, "import")
KEY(K_ALIAS, "alias")
KEY(K_DEF, "def")
KEY(K_VAR, "var")
KEY(K_STRUCT, "struct")
KEY(K_ENUM, "enum")
KEY(K_UNION, "union")

/* control flow keywords */
KEY(K_RETURN, "return")
KEY(K_IF, "if")
KEY(K_ELSE, "else")
KEY(K_WHILE, "while")
KEY(K_FOR, "for")
KEY(K_BREAK, "break")
KEY(K_CONTINUE, "continue")
KEY(K_COERCE, "coerce")

/* reserved keywords */
KEY(K_MACRO, "macro")
KEY(K_TRAIT, "trait")
KEY(K_CONCEPT, "concept")
KEY(K_REQUIRES, "requires")
KEY(K_TEMPLATE, "template")
KEY(K_OBJECT, "object")
KEY(K_EXPORT, "export")
KEY(K_MODULE, "module")
KEY(K_APPLY, "apply")
KEY(K_FINAL, "final")
KEY(K_OVERRIDE, "override")
KEY(K_GOTO, "goto")
KEY(K_MATCH, "match")
KEY(K_WHEN, "when")
KEY(K_SWITCH, "switch")
KEY(K_CASE, "case")
KEY(K_FALLTHROUGH, "fallthrough")
KEY(K_DO, "do")
KEY(K_IS, "is")
KEY(K_AS, "as")
KEY(K_IN, "in")
KEY(K_CONST, "const")
KEY(K_MUT, "mut")
KEY(K_LOOP, "loop")

/* math operators */
OP(K_ADD, "+")
OP(K_ADDEQ, "+=")

OP(K_SUB, "-")
OP(K_SUBEQ, "-=")

OP(K_MUL, "*")
OP(K_MULEQ, "*=")

OP(K_DIV, "/")
OP(K_DIVEQ, "/=")

OP(K_MOD, "%")
OP(K_MODEQ, "%=")

/* bitwise operators */
OP(K_BITAND, "&")
OP(K_BITANDEQ, "&=")

OP(K_BITOR, "|")
OP(K_BITOREQ, "|=")

OP(K_SHL, "<<")
OP(K_SHLEQ, "<<=")

OP(K_SHR, ">>")
OP(K_SHREQ, ">>=")

OP(K_BITXOR, "^")
OP(K_BITXOREQ, "^=")

OP(K_BITNOT, "~")

/* logical operators */
OP(K_AND, "&&")
OP(K_OR, "||")

OP(K_GT, ">")
OP(K_GTE, ">=")

OP(K_LT, "<")
OP(K_LTE, "<=")

OP(K_EQ, "==")
OP(K_NEQ, "!=")

OP(K_NOT, "!")

/* syntax operators */
OP(K_ASSIGN, "=")
OP(K_DOT, ".")
OP(K_COMMA, ",")
OP(K_QUESTION, "?")
OP(K_COLON, ":")
OP(K_COLON2, "::")
OP(K_SEMI, ";")
OP(K_AT, "@")
OP(K_ARROW, "->")

OP(K_LPAREN, "(")
OP(K_RPAREN, ")")

OP(K_LBRACE, "{")
OP(K_RBRACE, "}")

OP(K_LSQUARE, "[")
OP(K_RSQUARE, "]")

#undef OP
#undef KEY
