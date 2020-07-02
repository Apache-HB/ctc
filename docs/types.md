# Types

## Integral types

`i8` is an integer type that can represent *at least* the range of -128 to 127.

`i16` is an integer type that can represent *at least* the range of -32,768 to 32,767.

`i32` is an integer type that can represent *at least* the range of -2,147,483,648 to 2,147,483,647.

`i64` is an integer type that can represent *at least* the range of -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807.

The size of integer types is PDB.
The in-memory representation of integer types is PDB.
Overflow for all operations leads to PDB.

## Array types

`[type:size]` denotes an array of `type` that is `size` long.
`size` must be an expression that evaluates to `>0` or `var`.
if `size` is `var` then the type is variable length and stored
on the stack in its current scope. upon exiting the scope all data in
`[type:var]` is removed and accessing it is UB. returning a variable length type is also IDB.

## Pointer types

Pointer types, declared as `*type` are used to point to an address in memory. The size & layout of `type` is not needed required when declaring a pointer type but must be known when dereferencing fields of `type` using `type->field`

## Structure types

structure types are user defined types declared as
```ct
struct name {
    field0: type;
    field1: type;
    ...
    fieldN: type;
}
```

The layout of data in the structure is IDB.
the size of a structure must be *at least* the sum of sizes of the fields in the structure.

## Union types

union types are user defined types declared as
```ct
union name {
    field0: type;
    field1: type;
    ...
    fieldN: type;
}
```

A union can store 1 variable at a time and accessing a field after another one has been assigned is PDB. the size of a union must be at least the size of the largest member. the layout of contents in the union is IDB.

## Enum types

enum types are user defined types declared as

```ct
enum name {
    field,
    field1,
    fieldN
}
```
or
```ct
enum field : type {
    field = expr,
    field1 = expr,
    fieldN = expr
}
```

when no backing type is specified the internal representation is PDB.
when a backing type is defined the internal representation is defined to be the same as the backing type. a backing type cannot be a pointer, struct, union, array, or void type. platforms and implementations may define additional valid backing types.

when fields do not have their value defined by the user each fields value is IDB.

## Other types

### void
`void` is a type with a size of 0 and may not be cast to or from any other type.
`*void` is a pointer to any type of any size and its size is defined by the target platform.
in addition `*void` can be implicitly cast to any other pointer type.

### [void:var] as an argument

When `[void:var]` is used as the last argument in a function such as

```ct
# valid
def vla_function(args: [void:var]) {
    @unused { args };
}

# valid
def vla_function(v: i32, args: [void:var]) {
    @unused { v, args };
}

# invalid
def vla_function(v: i32 = 0, args: [void:var]) {
    @unused { v, args };
}

# invalid
def vla_function(v: i32, args: [void:var], a: i32) {
    @unused { v, args, a };
}
```

the function `vla_function` will be able to take an arbitrary number of trailing arguments (between 0..N). the ABI of these functions is IDB.

```ct
vla_function();
vla_function(10);
vla_function(10, "string", 100);
```

In addition an argument of type `[void:var]` *must* be the last argument in a function and may not be default initialized.