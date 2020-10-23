project('cthulhu', 'cpp',
    default_options : [
        'cpp_std=c++17',
        'cpp_rtti=false',
        'cpp_eh=none',
        'werror=true',
        'warning_level=3'
    ]
)

cthulhu = library('cthulhu', 'cthulhu/cthulhu.cpp')

test('stream', executable('stream', 'tests/stream.cpp', link_with : cthulhu))
