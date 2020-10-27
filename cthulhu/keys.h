#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

OP(K_LPAREN, "(")
OP(K_RPAREN, ")")
OP(K_SEMI, ";")

#undef KEY
#undef OP