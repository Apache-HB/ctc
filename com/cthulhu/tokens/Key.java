package com.cthulhu.tokens;

import java.util.Map;
import java.util.stream.Collectors;

import com.cthulhu.Token;

import java.util.Arrays;

public enum Key {
    DEF("def"),
    STRUCT("struct"),

    ASSIGN(":="),
    EQ("=="),

    NOT("!"),
    NEQ("!=");

    public String id;
    Key(String i) {
        this.id = i;
    }

    public Token tok() {
        return new KeyToken(this);
    }

    public static Map<String, KeyToken> keys = Arrays.asList(Key.values())
        .stream()
        .collect(Collectors.toMap(it -> it.id, it -> new KeyToken(it)));
}