project('cthulhu', 'c',
    default_options : [
        'c_std=c99',
        'werror=true',
        'warning_level=3'
    ]
)

ct_dep = declare_dependency(
    include_directories : include_directories('.')
)

subdir('tests')
