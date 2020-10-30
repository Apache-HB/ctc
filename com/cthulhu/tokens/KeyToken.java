package com.cthulhu.tokens;

import java.util.Map;
import static java.util.Map.entry;

import com.cthulhu.Token;

public enum KeyToken implements Token {
    DEF,
    STRUCT,
    
    NOT,
    NEQ;

    public static Map<String, KeyToken> keys = Map.ofEntries(
        entry("def", DEF),
        entry("struct", STRUCT)
    );
}
