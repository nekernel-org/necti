# Specification of Nectar.

===================================

# 0: General Information

===================================

- Output format: PEF
- Object format: AE
- Used Languages: Assembly, and C++

===================================

# 1: CompilerKit

===================================

- Shall support compilation, assembling and linking.
- Shall be written in C++
- Shall support the assembler too.
- Shall provide primitives for compilers, assemblers and linkers.
- Shall support multiple architectures.
- Shall be easy to expand and maintain.

===================================

# 2: GenericsLibrary

===================================

- Shall support Nectar runtime and ABI of NeSystem.
- Shall support a basic subset of the Nectar library.
- Shall be written in Nectar.
- Shall provide Nectar with the required implementation to write programs.

===================================

# 3: DebuggerKit

===================================

- Shall be written in C++
- Shall have a debugger.
- Shall support multiple architectures.
- Shall have a kernel deubgger.
- Shall be easy to expand and maintain.