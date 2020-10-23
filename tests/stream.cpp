#include "cthulhu/cthulhu.h"

#include <stdio.h>

struct SS {
    const char *str;
    int idx = 0;
};

int main() {
    auto ss = SS { R"(hello
    again)" };

    auto s = Stream((void*)&ss, [](auto c) -> int { auto ss = (SS*)c; return ss->str[ss->idx++]; });

    auto here = s.here();

    printf("%c\n", s.next());
    printf("{ %d, %d, %d, %d }\n", here.dist, here.line, here.col, here.len);

    here = s.here();

    printf("%c\n", s.next());
    printf("{ %d, %d, %d, %d }\n", here.dist, here.line, here.col, here.len);
here = s.here();
    printf("%c\n", s.next());
    printf("{ %d, %d, %d, %d }\n", here.dist, here.line, here.col, here.len);
here = s.here();
    printf("%c\n", s.next());
    printf("{ %d, %d, %d, %d }\n", here.dist, here.line, here.col, here.len);
here = s.here();
    printf("%c\n", s.next());
    printf("{ %d, %d, %d, %d }\n", here.dist, here.line, here.col, here.len);
here = s.here();
    printf("%c\n", s.next());
    printf("{ %d, %d, %d, %d }\n", here.dist, here.line, here.col, here.len);
here = s.here();
    printf("%c\n", s.next());
    printf("{ %d, %d, %d, %d }\n", here.dist, here.line, here.col, here.len);
here = s.here();
    printf("%c\n", s.next());
    printf("{ %d, %d, %d, %d }\n", here.dist, here.line, here.col, here.len);
}
