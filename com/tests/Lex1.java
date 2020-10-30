package com.tests;

import java.io.Reader;
import java.io.StringReader;

import com.cthulhu.Lexer;
import com.cthulhu.tokens.IntToken;

public class Lex1 {
    public static void main(String[] args) {
        Reader r = new StringReader(
            """
            500
            """
        );
        Lexer lex = new Lexer(r);

        var n = lex.next();
        assert n instanceof IntToken : "bad type";
        assert n.equals(500) : "wrong value";
        assert false;
    }
}
