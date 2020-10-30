project('ct', 'java')

src = [
    'com/cthulhu/Token.java',
    'com/cthulhu/Main.java',

    'com/cthulhu/tokens/EOFToken.java',
    'com/cthulhu/tokens/InvalidToken.java',
    'com/cthulhu/tokens/IdentToken.java',
    'com/cthulhu/tokens/KeyToken.java',
    'com/cthulhu/tokens/StringToken.java',
    'com/cthulhu/tokens/IntToken.java'
]

out = jar('cthulhuc', src, 
    main_class : 'com.cthulhu.Main'
)
test('run', out)