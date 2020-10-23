#pragma once

#include <variant>
#include <string>

struct Where {
    int dist = 0;
    int line = 0;
    int col = 0;
    int len = 0;
};

struct Stream {
    Stream(void* in, int(*func)(void*))
        : stream(in)
        , get(func)
    { 
        ahead = get(stream);
    }

    int next() { 
        int c = ahead;
        ahead = get(stream);

        return c;
    }

    int peek() {
        return ahead;
    }

    void* stream;
    int(*get)(void*);
    int ahead;
    Where pos;
};

enum struct Key {

};

struct Token {
    std::variant<Key> data;
    Where where;
};

struct Lexer {
    Token next() {

    }

    Stream in;
};
