package com.cthulhu.tokens;

import java.math.BigInteger;

import com.cthulhu.Token;

public class IntToken implements Token {
    BigInteger num;
    public IntToken(String n, int radix) throws NumberFormatException {
        this.num = new BigInteger(n, radix);
    }

    public IntToken(int n) {
        this.num = BigInteger.valueOf(n);
    }

    boolean equals(int n) {
        return num.equals(n);
    }
}