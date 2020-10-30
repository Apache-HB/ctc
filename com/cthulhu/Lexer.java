package com.cthulhu;

import com.cthulhu.tokens.*;

import java.io.IOException;
import java.io.Reader;
import java.math.BigInteger;
import java.util.function.Function;

public class Lexer {
    public Lexer(Reader source) {
        this.in = source;
        this.ahead = read();
    }

    Reader in;
    char ahead;

    char read() {
        try {
            int temp = in.read();
            return (temp == -1) ? 0 : (char)temp;
        } catch (IOException err) {
            return 0;
        }
    }

    char get() {
        char c = ahead;
        ahead = read();

        return c;
    }

    char peek() {
        return ahead;
    }

    boolean eat(char c) {
        if (peek() == c) {
            get();
            return true;
        }
        return false;
    }

    char skip() {
        char c = get();

        while (Character.isWhitespace(c)) {
            c = get();

            if (c == '#')
                do { c = get(); } while (c != 0 && !System.lineSeparator().contains(Character.toString(c)));
        }
        
        return c;
    }

    String collect(char init, Function<Character, Boolean> func) {
        String out = init == 0 ? "" : Character.toString(init);
        
        while (func.apply(peek())) 
            out += get();

        return out;
    }

    static boolean isIdent1(char c) {
        return Character.isLetter(c) || c == '_';
    }

    static boolean isIdent2(char c) {
        return Character.isLetterOrDigit(c) || c == '_';
    }

    static boolean isXDigit(char c) {
        return Character.isDigit(c) || ('a' <= c && c >= 'f') || ('A' <= c && c >= 'F');
    }

    Token ident(char c) {
        String buf = collect(c, Lexer::isIdent2);

        if (KeyToken.keys.containsKey(buf)) {
            return KeyToken.keys.get(buf);
        } else {
            return new IdentToken(buf);
        }
    }

    Token base(char c, int radix, Function<Character, Boolean> func) {
        String buf = collect(c, func);

        try {
            return new IntToken(buf, radix);
        } catch (NumberFormatException err) {
            return new InvalidToken();
        }
    }

    Token basex() {
        if (eat('x')) {
            return base('\0', 16, Lexer::isXDigit);
        } else if (eat('b')) {
            return base('\0', 2, c -> c == '0' || c == '1');
        } else if (Character.isDigit(peek())) {
            return base(get(), 10, Character::isDigit);
        } else {
            return new IntToken(0);
        }
    }

    Token num(char c) {
        return switch (c) {
            case '0' -> basex();
            default -> base(c, 10, Character::isDigit);
        };
    }

    Token str() {
        return null;
    }

    Token symbol(char c) {
        return switch (c) {
            case '!' -> eat('=') ? KeyToken.NEQ : KeyToken.NOT;
            default -> null;
        };
    }

    public Token next() {
        char c = skip();

        if (c == 0) {
            return new EOFToken();
        } else if (isIdent1(c)) {
            return ident(c);
        } else if (Character.isDigit(c)) {
            return num(c);
        } else if (c == '"') {
            return str();
        } else {
            return symbol(c);
        }
    }
}