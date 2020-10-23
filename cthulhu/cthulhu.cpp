#include "cthulhu.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <algorithm>

Stream::Stream(void* in, int(*func)(void*)) 
    : file(0x1000)
    , stream(in)
    , get(func)
    , ahead(get(stream))
{ }

Buffer::Buffer(int init) 
    : ptr((char*)std::malloc(init))
    , len(0)
    , size(init)
{ }

void Buffer::push(char c) {
    if (len + 1 >= size) {
        size += 0x1000;
        ptr = (char*)std::realloc(ptr, size);
    }

    ptr[len++] = c;
}

int Stream::next() {
    int c = ahead;
    ahead = get(stream);
    file.push(c);

    pos.dist += 1;

    if (c == '\n') {
        pos.col = 0;
        pos.line += 1;
    } else {
        pos.col += 1;
    }

    return c;
}

Where Stream::here() const {
    return pos;
}

int Stream::peek() {
    return ahead;
}

int Lexer::skip() {
    int c = next();
    while (std::isspace(c))
        c = next();

    return c;
}

Where range(Where begin, Where end) {
    Where out = begin;
    out.len = end.dist - begin.dist;
    return out;
}

bool isident1(int c) { return isalpha(c) || c == '_'; }
bool isident2(int c) { return isalnum(c) || c == '_'; }

bool Range::cmp(const Range& lhs, const Range& rhs) {
    return std::strncmp(lhs.ptr, rhs.ptr, std::min(lhs.len, rhs.len) + 1) == 0;
}

char* Intern::begin() {
    return ptr + len;
}

int Intern::end(char* str) {
    return (ptr + len) - str;
}

Range Intern::intern(char* str, int width) {
    Range it = { str, width };
    Node** walk = &base;
    while (*walk) {
        
    }
}

Token Lexer::lex() {

    // get first real character
    auto c = skip();
    auto begin = here();

    Token out;

    if (c == 0) {
        out.type = Type::end; 
    } else if (isident1(c)) {
        out = ident(c);
    } else if (std::isdigit(c)) {
        out = digit(c);
    } else if (c == '"') {
        out = string();
    } else {
        out = symbol(c);
    }

    return out;
}