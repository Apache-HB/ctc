package com.tests;

import java.io.Reader;
import java.io.StringReader;
import java.math.BigInteger;

import com.cthulhu.Lexer;
import com.cthulhu.Token;
import com.cthulhu.tokens.IntToken;

public class Lex1 {
    public static void main(String[] args) {
        Reader r = new StringReader("500 0x500 0b1100");
        var lex = new Lexer(r);
        Token n;

        n = lex.next();
        assert n instanceof IntToken : "expected int";
        assert n.equals(BigInteger.valueOf(500)) : "wrong value " + n.toString();

        n = lex.next();
        assert n instanceof IntToken : "expected int";
        assert n.equals(BigInteger.valueOf(0x500)) : "wrong value " + n.toString();

        n = lex.next();
        assert n instanceof IntToken : "expected int";
        assert n.equals(BigInteger.valueOf(0b1100)) : "wrong value " + n.toString();
    }
}
