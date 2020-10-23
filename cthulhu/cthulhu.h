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

struct Range {
    char* ptr;
    int len;

    static bool cmp(const Range& lhs, const Range& rhs);
};

union TokenData {
    Range ident;
};

struct Token {
    Where pos;
    Type type;
    TokenData data;
};

struct Buffer {
    Buffer(int init);

    void push(char c);

    char* ptr;
    int len;
    int size;
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

struct Node {
    Node* lhs;
    Node* rhs;
    Range it;
};

struct Intern : Buffer {
    Node* base = nullptr;

    char* begin();
    int end(char* str);

    Range intern(char* str, int width);
};

struct Lexer : Stream {
    Token lex();

    Token ident(int c);
    Token digit(int c);
    Token string();
    Token symbol(int c);

    // skip whitespace and comments
    int skip();
};
