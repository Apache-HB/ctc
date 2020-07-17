#pragma once

extern "C" {
#include "cthulhu.h"
}

namespace cthulhu
{
    struct Stream
    {
        CtStream data;
    };

    struct Lexer
    {
        CtLexer data;
    };

    struct Parser
    {
        CtParser data;
    };
}
