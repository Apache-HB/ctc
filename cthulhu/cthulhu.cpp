#include "cthulhu.h"

#include <stdio.h>

int main(int argc, char** argv) {
    (void)argc;
    void* file = (void*)fopen(argv[1], "r");
    auto stream = Stream(file, [](void* f) -> int {
        auto n = fgetc((FILE*)f);
        if (n == -1)
            return 0;
        return n;
    }, argv[1]);

    while (stream.next() != 0);

    Where here = { 9, 1, 9, 11, &stream };

    fmt::print("{}", error(here, "something\nvery\nlong", { "funny", "thing" }));
}
