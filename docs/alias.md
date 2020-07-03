# Aliasing

Using the `alias` keyword allows aliasing of symbols to shorten names or provide more clarity in its use.

## Aliasing types

Types may be aliased to better convey meaning

```ct
# a.ct

alias Any = *void;
```

Aliasing a type doesn't create a distinct type and only renames the type. Casting between aliases that resolve to the same type is implicit.

```ct
# a.ct

alias VarArgs = [void:var];

def vla_function(args: VarArgs);
```

is identical to using `[void:var]` instead of `VarArgs`

## Aliasing functions

Aliasing functions behaves the same as aliasing a type and is used only to rename them.
