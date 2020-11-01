project('ct', 'java')

src = [
    'com/cthulhu/Token.java',
    'com/cthulhu/Lexer.java',

    'com/cthulhu/tokens/EOFToken.java',
    'com/cthulhu/tokens/InvalidToken.java',
    'com/cthulhu/tokens/IdentToken.java',
    'com/cthulhu/tokens/KeyToken.java',
    'com/cthulhu/tokens/StringToken.java',
    'com/cthulhu/tokens/IntToken.java'
]

out = jar('cthulhuc', src + ['com/cthulhu/Main.java'],
    java_args : ['-Xlint:unchecked']
)

java = find_program('java')


lex1 = jar('lex1', 'com/tests/Lex1.java', main_class : 'com.tests.Lex1', link_with : out)
test('test-lex1', lex1)
