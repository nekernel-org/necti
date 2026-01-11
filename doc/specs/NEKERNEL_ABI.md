# Specification of NeKernel's ABI

The PEF ABI has multiple versions depending on the ISA.

===================================

# 0: General Information (ISA agnostic)

===================================

- Arguments goes from a0 to max argument register of ISA.
- SIMD arguments uses SIMD registers.
- The ABI is stack based, rbp is preserved.

===================================

# 0: General Information (x64)

===================================

- RBP is saved.
- SIMD is saved.

===================================

# 0: General Information (ARM64)

===================================

- BP is saved.
- SIMD is saved.
