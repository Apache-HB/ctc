package com.cthulhu.tokens;

import com.cthulhu.Token;

public class IdentToken implements Token {
    String id;
    public IdentToken(String i) {
        id = i;
    }
}