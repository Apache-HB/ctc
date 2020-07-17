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

#undef KEY
#undef OP
