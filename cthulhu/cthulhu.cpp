#include "cthulhu.h"

#include <cctype>

Stream::Stream(void* in, int(*func)(void*)) 
    : stream(in)
    , get(func)
    , ahead(get(stream)) 
{ }

void Buffer::push(char c) {

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
