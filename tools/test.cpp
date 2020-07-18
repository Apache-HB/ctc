#include "cthulhu/cthulhu.hpp"

#include <sstream>
#include <istream>
#include <string>
#include <string_view>
#include <iostream>

using namespace std::literals;

int main()
{
    std::istream* in = new std::stringstream("!<!<>> >> >");

    CtAllocator alloc;
    alloc.alloc = [](void*, size_t size) { return malloc(size); };
    alloc.dealloc = [](void*, void* ptr) { free(ptr); };
    alloc.data = nullptr;

    CtLexer lex = ctLexerNew((void*)in, [](void* ptr) { return ((std::istream*)ptr)->get(); }, alloc);

    CtUserKeyword keys[] = {
        { "byte", 0 },
        { "word", 1 },
        { "dword", 2 },
        { "qword", 3 }
    };

    ctSetKeys(&lex, keys, 4);

    auto tok = ctLexerNext(&lex);

    if (tok.kind != TK_KEYWORD || tok.data.key != K_TBEGIN)
    {
        printf("1oh no %d\n", tok.data.key);
        return 1;
    }

    tok = ctLexerNext(&lex);

    if (tok.kind != TK_KEYWORD || tok.data.key != K_TBEGIN)
    {
        printf("2oh no %d\n", tok.data.key);
        return 1;
    }

    tok = ctLexerNext(&lex);

    if (tok.kind != TK_KEYWORD || tok.data.key != K_TEND)
    {
        printf("3oh no %d\n", tok.data.key);
        return 1;
    }

    tok = ctLexerNext(&lex);

    if (tok.kind != TK_KEYWORD || tok.data.key != K_TEND)
    {
        printf("4oh no %d\n", tok.data.key);
        return 1;
    }

    tok = ctLexerNext(&lex);

    if (tok.kind != TK_KEYWORD || tok.data.key != K_SHR)
    {
        printf("5oh no %d\n", tok.data.key);
        return 1;
    }

    tok = ctLexerNext(&lex);

    if (tok.kind != TK_KEYWORD || tok.data.key != K_LT)
    {
        printf("6oh no %d\n", tok.data.key);
        return 1;
    }
}
