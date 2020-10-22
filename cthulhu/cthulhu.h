#pragma once

#include <cstdint>

struct Where {
    int dist;
    int line;
    int col;
    int len;
};

enum struct Type {
    invalid,
    end,
    ident,
    key,
    str
};

struct String {
    char *ptr;
    int len;
};

#define UNION(name) struct name { union {
#define END }; };

struct TokenData {
    union {
        String str;
        String ident;
    };
};

struct Token : TokenData {
    Where pos;
    Type type;
    TokenData data;
};
