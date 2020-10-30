default = [ '-DCT_MALLOC=malloc', '-DCT_FREE=free', '-DCT_REALLOC=realloc' ]

small = [ '-DCT_MM_SMALL=1' ]
fast = [ '-DCT_MM_FAST=1' ]

test('small', executable('small', 'compile.c', 
    dependencies : ct_dep,
    c_args : default + small
))

test('fast', executable('fast', 'compile.c', 
    dependencies : ct_dep,
    c_args : default + fast
))
