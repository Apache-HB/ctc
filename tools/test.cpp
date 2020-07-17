extern "C" {
#include "cthulhu/cthulhu.h"
}

int main(int argc, const char **argv)
{
    auto stream = ctStreamNew();
    auto lexer = ctLexerNew();
    auto parser = ctParserNew();
    auto interp = ctInterpNew();

    ctInterpDelete(&interp);
    ctParserDelete(&parser);
    ctLexerDelete(&lexer);
    ctStreamDelete(&stream);
}
