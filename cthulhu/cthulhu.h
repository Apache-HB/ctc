#pragma once

#include <variant>
#include <string>
#include <fmt/core.h>

struct Where {
    int dist = 0;
    int line = 0;
    int col = 0;
    int len = 0;
};

struct Stream {
    Stream(void* in, int(*func)(void*), std::string id);

    // get the next character from the file
    // and then move to the next character
    int next();

    // get the next character without
    // moving forward in the stream
    int peek();

    // check if the next char is c
    // and if it is get the next character
    bool eat(char c);

    // native file stream
    void* stream;
    // native function to get next char
    // should return 0 at EOF
    int(*get)(void*);
    // filename
    std::string name;

    // lookahead char
    int ahead;
    // current location
    Where pos;
    // files read contents
    std::string buffer;
};

struct Token {
    Where where;
};

struct Ident : Token {
    std::string id;
};

struct Key : Token {
    enum { invalid } key;
};

struct End : Token { };
struct Invalid : Token { };


struct Lex : Stream {
    Token* get();
};

// errors are formatted like
// file.ct -> [line:col] [line:col]
//        * error message
//        |
//  line1 | line content
//        | ^~~~ error 
//        | ...
// line20 | more content here
//        |      ^~~~~~~ message
//        * note1
//        * note2
#if 0
std::string error(const std::vector<Error>& errors) {
    int len = 0;
    for (auto err : errors) {
        len = std::max(len, fmt::format("{}", err.where.line).length());
    }

    auto in = it.source;
    std::string out = fmt::format("{} -> [{}:{}]\n", in->name, it.line, it.col);

    auto len = fmt::format(" {} ", it.line).length();

    auto begin = std::max(it.dist - it.col, 0);
    auto end = in->buffer.find_first_of("\n\0", it.dist);

    auto line = in->buffer.substr(begin, end);

    out += std::string(len, ' ') + "|\n";
    out += fmt::format(" {} | ", it.line) + line + "\n";
    auto under = std::string(len, ' ') + "| " + std::string(it.col, ' ') + '^' + std::string(it.len - 1, '~');
    out += under;

    if (msg.length() > 0) {
        bool once = true;
        for (auto part : split(msg, "\n")) {
            if (once) {
                once = false;
                out += " " + part + '\n';
            } else {
                out += std::string(under.length() + 1, ' ') + part + '\n';
            }
        }
    }

    for (auto note : notes) {
        out += std::string(len, ' ') + "* " + note + '\n';
    }

    return out;
}
#endif
