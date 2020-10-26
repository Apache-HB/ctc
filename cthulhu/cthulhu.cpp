#include "cthulhu.h"

#include <stdio.h>
#include <cstddef>

Stream::Stream(void* in, int(*func)(void*), std::string id)
    : stream(in)
    , get(func)
    , name(id)
    , ahead(get(stream))
{ }

int Stream::next() { 
    int c = ahead;
    if (!c)
        return c;

    ahead = get(stream);
    buffer.push_back(c);

    pos.dist++;
    if (c == '\n') {
        pos.line++;
        pos.col = 0;
    } else {
        pos.col++;
    }

    return c;
}

int Stream::peek() {
    return ahead;
}

bool Stream::eat(char c) {
    if (peek() == c) {
        next();
        return true;
    }
    return false;
}

bool isident1(char c) { return std::isalpha(c) || c == '_'; }
bool isident2(char c) { return std::isalnum(c) || c == '_'; }

struct Pair { std::string_view str; key_t id; };

Pair keys[] = {
#define KEY(id, str) { str, id },
#include "keys.inc"
};

Token* Lex::ident(char c) {
    std::string buf = c;
    while (isident2(peek()))
        buf += next();

    for (auto pair : keys) {
        if (pair.str == buf) {
            return new Key(pair.id);
        }
    }

    return new Ident(buf);
}

Token* Lex::get() {
    auto c = next();

    if (c == 0) {
        return new End();
    } else if (isident1(c)) {

    }

    return nullptr;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
}
