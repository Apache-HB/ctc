# Expressions

## Presedence

From highest to lowest. operators with the same precedence level are evaluated from left to right.

1. Unary `~`, `!`, `*`, `&`
2. Binary `*`, `/`, `%`
3. Binary `<<`, `>>`
4. Binary `^`, `|`, `&`
5. Binary `+`, `-`
6. Binary `==`, `!=`
7. Binary `<`, `<=`, `>=`, `>`
8. Binary `&&`, `||`
9. Ternary `?:`
10. Binary `=`, `*=`, `/=`, `%=`, `+=`, `-=`, `<<=`, `=>>`, `&=`, `|=`, `^=`


## Casting

Casting as seen in many other languages like C or D is a foreign concept to cthulhu. as we aim for 0 overhead and little UB adding consistent casting rules would be difficult. to this end cthulhu supports type coercion instead. simmilar to `reinterpret_cast` from C++. this is named the `coerce` keyword. when `coerce`ing a value to another type, a few requirements must be met.

```ct
val i = coerce<i32>(100u32);
```

the `size` of the target type must be less than or equal to the `size` of the original expression.
`coerce`ing away qualifiers like `const` or `volatile` is UB.

it is invalid to `coerce` to an unsized array type.