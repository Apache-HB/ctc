#include "cthulhu/cthulhu.hpp"

#include <sstream>
#include <istream>
#include <string>
#include <string_view>
#include <iostream>

using namespace std::literals;

int main()
{
    std::istream* in = new std::stringstream("ident");

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

    if (tok.kind != TK_IDENT || tok.data.ident != "ident"sv)
    {
        printf("oh no %s\n", tok.data.str.str);
        return 1;
    }
}
