package com.cthulhu;

import com.cthulhu.tokens.*;

import java.io.Reader;

public class Lexer {
    Lexer(Reader source) {
        this.ahead = source.read();
        this.in = source;
    }

    Reader in;
    char ahead;

    char get() {
        char c = ahead;
        ahead = in.read();

        return ahead;
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
        int c = get();

        while (Character.isWhitespace(c)) {
            c = in.read();
            if (c == '#') {
                while ((c = in.read()) != System.lineSeparator());            }
        }
        
        return c;
    }

    static boolean isIdent1(char c) {
        return Character.isLetter(c) || c == '_';
    }

    static boolean isIdent2(char c) {
        return Character.isLetterOrDigit(c) || c == '_';
    }

    Token ident(char c) {
        String buf = c;

        while (isIdent2(peek())) {
            buf += get();
        }

        if (KeyToken.keys.containsKey(buf)) {
            return new KeyToken(keys.get(buf));
        } else {
            return new IdentToken(buf);
        }
    }

    Token num(char c) {
        return switch (c) {
            case '0' -> null;
            default -> base10(c);
        }
    }

    Token str() {
        return null;
    }

    Token symbol(char c) {
        return switch (c) {
            '!' -> eat('=') ? KeyToken.NEQ : KeyToken.NOT;
            default -> null;
        }
    }

    Token next() {
        char c = skip();

        if (c == 0) {
            return new EOFToken();
        } else if (isident1(c)) {
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