#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

KEY(K_DEF, "def")
KEY(K_VAR, "var")
KEY(K_LET, "let")

OP(K_LPAREN, "(")
OP(K_RPAREN, ")")
OP(K_SEMI, ";")
OP(K_LSQUARE, "[")
OP(K_RSQUARE, "]")
OP(K_LBRACE, "{")
OP(K_RBRACE, "}")

OP(K_NOT, "!")
OP(K_NEQ, "!=")

#undef KEY
#undef OP
