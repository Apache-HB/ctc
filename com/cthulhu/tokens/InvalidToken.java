package com.cthulhu.tokens;

import com.cthulhu.Token;

public class InvalidToken extends Token<String> {
    public InvalidToken() {
        super(null);
    }

    public InvalidToken(String msg) {
        super(msg);
    }

    @Override
    public boolean equals(Object other) {
        return false;
    }
}
