project('cthulhu', 'cpp',
    default_options : [
        'cpp_std=c++17',
        'cpp_rtti=false',
        'cpp_eh=none',
        'werror=true',
        'warning_level=3'
    ]
)

fmt = subproject('fmt').get_variable('fmt_dep')

cthulhu = executable('cthulhu', 'cthulhu/cthulhu.cpp',
    dependencies : fmt
)
