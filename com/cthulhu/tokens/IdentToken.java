package com.cthulhu.tokens;

import com.cthulhu.Token;

public class IdentToken implements Token {
    String id;
    IdentToken(String i) {
        id = i;
    }
}