package com.cthulhu;

public abstract class Token<T> {
    T data;
    public Token() {}
    public Token(T self) {
        data = self;
    }

    @Override
    public boolean equals(Object other) {
        return data.equals(other);
    }
}
