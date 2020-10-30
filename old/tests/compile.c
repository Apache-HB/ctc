#include <stdlib.h>

#include "cthulhu/cthulhu.c"


/* simple sanity check to make sure stuff compiles */
int main(int argc, char **argv) {
    CT_UNUSED(argc);
    CT_UNUSED(argv);

    ctInit();

    ctFree();
    return 0;
}
