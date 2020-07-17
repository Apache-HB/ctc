project('ctc', ['c', 'cpp'])

cthulhu = library('cthulhu', 'cthulhu/cthulhu.c')

sources = [
    'cthulhu/tests/interp.c'
]

cti = executable('cti', sources,
    include_directories : include_directories('.')
)
