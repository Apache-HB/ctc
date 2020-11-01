package com.cthulhu.tokens;

import com.cthulhu.Token;

public class InvalidToken extends Token<String> {
    public InvalidToken(String msg) {
        super(msg);
    }
}