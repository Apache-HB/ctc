#ifndef OP
#   define OP(id, str)
#endif

#ifndef KEY
#   define KEY(id, str)
#endif

KEY(K_DEF, "def")
KEY(K_ALIAS, "alias")
KEY(K_IMPORT, "import")
KEY(K_VAR, "var")
KEY(K_STRUCT, "struct")
KEY(K_UNION, "union")
KEY(K_ENUM, "enum")

/* return 5; */
KEY(K_RETURN, "return")

/* if (5 == 5) {} */
KEY(K_IF, "if")

/* else if (5 == 5) {} */
KEY(K_ELSE, "else")

/* while (5 == 5) {} */
KEY(K_WHILE, "while")

/* for (var i = 0; i < 5; i += 1) {} */
/* for (i : range(10)) {} */
KEY(K_FOR, "for")

/* coerce<i8>(0) */
KEY(K_COERCE, "coerce")

/* break; */
KEY(K_BREAK, "break")

/* continue; */
KEY(K_CONTINUE, "continue")

OP(K_ADD, "+")
OP(K_ADDEQ, "+=")

OP(K_SUB, "-")
OP(K_SUBEQ, "-=")

OP(K_EQ, "==")
OP(K_NEQ, "!=")
OP(K_NOT, "!")

OP(K_AT, "@")
OP(K_ASSIGN, "=")

OP(K_LPAREN, "(")
OP(K_RPAREN, ")")
OP(K_LSQUARE, "[")
OP(K_RSQUARE, "]")
OP(K_LBRACE, "{")
OP(K_RBRACE, "}")

OP(K_COLON, ":")
OP(K_COLON2, "::")
OP(K_SEMI, ";")
OP(K_COMMA, ",")
OP(K_DOT, ".")
OP(K_QUEST, "?")

#undef OP
#undef KEY
