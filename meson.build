project('cthulhu', ['c', 'cpp'],
    default_options : [
        'c_std=c99',
        'cpp_std=c++17',
        'werror=true',
        'warning_level=3'
    ]
)

cthulhu = library('cthulhu', 'cthulhu/cthulhu.c')

test('lexer', executable('lexer', 'tools/test.cpp', link_with : cthulhu))