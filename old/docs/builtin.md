# Builtins

Actions that require compiler support such as inline assembly, suppressing warnings, or modifying calling conventions are provided though the `std::builtin` module.

## Extending std::builtin and __builtin

Extending both `std::builtin` and `__builtin` is invalid.
This by extension means you cannot name files `builtin.ct` or `__builtin.ct`. you may also not name a folder `std` or `builtin` due to `std::builtin` being reserved.