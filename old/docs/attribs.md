# Attributes

Attributes are compiler and user defined constructs used at compile time to effect semantics and code generation.

## Builtin Attributes

These are provided through the `std::builtin` module and give access to behavior that is unable to be provided through the language itself such as export visibility or function calling conventions.

### Docs

`std::builtin::doc` provides a standard way of documenting functions, types, and aliases. Its signature is `doc<T>(brief: str, desc: str = "", refs: [str:var] = {}, args: IDB = IDB)` where args is a map of parameter, or field names similar to pythons `**kwargs`. This aids tooling in generating docs based off of source code as documentation no longer requires special handling with comment parsing.

```ct
import std::builtin(doc);

@doc("doubles a value")
def multiply_by_2(val: i32) = val * 2;

@doc(
    [brief] = "triples a value",
    [desc] = "takes the value of val and multiplies it by 3",
    [see] = { multiply_by_2 },
    [val] = "the value to multiply by 3"
)
def multiply_by_3(val: i32) = val * 3;

@doc(
    [brief] = "a pair of ints",
    [first] = "the first integer",
    [second] = "the second integer"
)
struct IntPair {
    first: i32;
    second: i32;
}
```

### Compile

`std::builtin::compile` marks a function as executable at compile time as opposed to runtime. Compile time functions are not guaranteed to be executed at compile time and may still be run at runtime. Compile time functions must not produce side effects and may not contain UB or PDB, IDB is allowed.

```ct
import std::builtin(compile);

@[compile]
def add(a: i32, b: i32) = a + b;
```

### Memory permissions

`std::builtin::perms` allows the user to mark the memory and execution permissions of a function or variable. This can be used to mark a function as writable to allow builtins that require runtime code modification or allow executable data for JIT compilers. Permissions are represented as a bitflag enum and can be combined.

```ct
import std::builtin(perms);
import std::builtin::perms(write, exec);

# coercing this data to a function and calling it is now PDB instead of invalid
@perms(write | exec)
var data: [u8:512] = {};

# coercing this data to raw bytes and modifying it is now PDB instead of invalid
@perms(write | exec)
def func {
    return 10;
}
```

perms cannot be applied to functions that are marked with the `compile` Attribute.

### Callconv

`std::arch::callconv` allows the user to override the default calling convention of a function to use a specific one. This is useful for interop with other languages such as C.