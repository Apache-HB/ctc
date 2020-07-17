#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

KEY(K_IMPORT, "import")
KEY(K_DEF, "def")
KEY(K_ALIAS, "alias")
KEY(K_STRUCT, "struct")
KEY(K_ENUM, "enum")
KEY(K_UNION, "union")
KEY(K_OBJECT, "object")
KEY(K_VAR, "var")
KEY(K_TRAIT, "trait")

KEY(K_IF, "if")
KEY(K_ELSE, "else")
KEY(K_WHILE, "while")
KEY(K_FOR, "for")
KEY(K_RETURN, "return")
KEY(K_BRANCH, "branch")
KEY(K_COERCE, "coerce")
KEY(K_BREAK, "break")
KEY(K_CONTINUE, "continue")

OP(K_AT, "@")
OP(K_GENERIC, "!<")
OP(K_SEMI, ";")
OP(K_COLON, ":")
OP(K_COLON2, "::")
OP(K_ARROW, "->")
OP(K_BIGARROW, "=>")
OP(K_COMMA, ",")
OP(K_DOT, ".")
OP(K_STREAM, "<|")

#undef KEY
#undef OP
