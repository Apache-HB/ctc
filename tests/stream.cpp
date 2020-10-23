#include "cthulhu/cthulhu.h"

#include <stdio.h>

struct SS {
    const char *str;
    int idx = 0;
};

int main() {
    auto ss = SS { R"(hello
    again)" };

    auto s = Stream((void*)&ss, [](auto s) -> int { 
        auto ss = (SS*)s; 
        auto c = ss->str[ss->idx];
        if (c != 0)
            ss->idx++;
        return c;
    });

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
