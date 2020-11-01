package com.cthulhu.tokens;

import java.math.BigInteger;

import com.cthulhu.Token;

public class IntToken extends Token<BigInteger> {
    public IntToken(String str, int radix) {
        super(new BigInteger(str, radix));
    }
}