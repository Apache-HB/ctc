#include "cthulhu.h"

#include <stdio.h>

Stream::Stream(void* in, int(*func)(void*), std::string id)
    : stream(in)
    , get(func)
    , name(id)
{ 
    ahead = get(stream);
    pos.source = this;
}

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

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
}
