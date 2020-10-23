#pragma once

struct Where {
    int dist = 0;
    int line = 0;
    int col = 0;
    int len = 0;
};

enum struct Type {
    invalid,
    end,
    ident,
    key,
    str
};

struct Ident {
    char *ptr;
    int len;
};

union TokenData {
    Ident ident;
};

struct Token {
    Where pos;
    Type type;
    TokenData data;
};

struct Buffer {
    char* ptr;
    int len;
    int size;

    void push(char c);
};

struct Stream {
    Stream(void* stream, int(*get)(void*));

    int next();
    int peek();
    Where here() const;

    Buffer file;
    void* stream;
    int(*get)(void*);
    int ahead;
    Where pos;
};

struct Lexer : Stream {
    Token lex();

    // skip whitespace and comments
    int skip();
};
