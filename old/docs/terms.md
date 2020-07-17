# Terms

## Platform Defined Behavior (PDB)

Behavior that is defined but may vary between targets.

## Implementation Defined Behavior (IDB)

Behavior that is defined on a per-compiler basis.

## Undefined Behavior (UB)

Behavior that is undefined and has no guarantees. Compilers are not required to warn about UB.

## Compilation Unit (CU)

Each file compiled is a single compilation unit. Compilers are allowed to combine multiple files into single compilation units as long as symbol hierarchy is properly preserved.