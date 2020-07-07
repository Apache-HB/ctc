# Imports

A CU may include 0 or more other CUs using the `import` keyword followed by a path.

A CU is named based on its path and file name. Valid CU names must match `[a-zA-Z_][a-zA-Z0-9_]*.ct`, CUs violating this convention are UB. In addition CUs may not be named `builtin.ct`, `__builtin.ct`, or `std.ct`.

## Symbols

A symbol refers to the symbolic name of an [alias](alias.md), [function](function.md), [decorator](decorator.md), or [type](type.md) in a CU. The internal name of a symbol is IDB.

## Importing a CU

When importing a CU without an [Import Specifier](#import-specifier) all the contents of the named CU are available through the name of the CU followed by the name of the symbol.

```ct
# a/b.ct

def function = 0;
```

```ct
# c.ct
import a::b;

def function = b::function();
```

By default all symbols are exported from a CU.

## Re-exporting CUs

External CUs are not re-exported.

```ct
# a.ct

# contains function()
import other;
```

```ct
# b.ct

# invalid: `other` is not visible outside of `a`
# import a::other;
```

## Import Specifier

Importing items from a CU using an import specifier limits the amount of symbols imported and removes the need to specify the module name. The same symbol may not be imported multiple times from the same CU and clashing symbols from multiple CUs is IDB.

```ct
# a/b.ct

def function = 0;
```

```ct
# c.ct
import a::b(function);

def function2 = function();
```

If an import specifier is used at 1 or more symbols must be imported.

## Symbol visibility

By default all symbols in a CU are externally visible. This functionality may be overridden using the `export` decorator from the `std::builtin` library.

Valid values of `export` are `hidden`, `default`, and `always`.
Each function may only be exported with a single option, specifying multiple export options is invalid.

```ct
# a.ct
import std::builtin(export);

@export(hidden)
def function0 = 0;

@export(default)
def function1 = 0;

@export(always)
def function2 = 0;

# by default this is exported
def function3 = function0();
```

```ct
# b.ct

import a;

# a::function0 is not visible
# a::function1 is visible
# a::function2 is visible
# a::function3 is visible despite using a::function0 which is not
```

```ct
# c.ct

# import a(function0); is invalid

# the rest of the functions may be imported as usual
import a(function1, function2, function3);
```