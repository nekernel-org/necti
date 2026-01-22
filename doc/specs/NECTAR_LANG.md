# Specification of Nectar

===================================

# 0: General Information

===================================

- ABI: PEF based.
- Typing System: Weak.
- Output: NeKernel Assembler/Netwide Assembler.
- Platforms: OS X, NeKernel.

===================================

# 1: Concepts

===================================

- `&` Checked pointer type.
- `*` Unchecked pointer type.
- `.` Checked pointer access.
- `->` Unchecked pointer access.
- `impl` Implementation data structure, useful for proxies, iterators, etc.
- `struct` Data implementation of `impl` -- useful to store fields and such.
- `let` Pointer/Reference variable declaration.
- `const` and `let` declaration.
- Functions support.
- Nested Stubs support.

===================================

# 2: Operators

===================================

- `:=` Equals/Assign To operator.

===================================

# 3: The Generics Library

===================================

Nectar runs using the Generics Library (GL) -- it contains foundational code to run nectar applications and systems.

- //@ Are considered NectarDocs.