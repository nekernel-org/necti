# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

Nectar uses **NeBuild** (https://github.com/nekernel-org/nebuild) as its build system, configured via JSON files.

### Building the Project

```bash
# Build CompilerKit library (required first)
cd src/CompilerKit
nebuild ck-posix.json    # Linux
nebuild ck-osx.json      # macOS
nebuild ck-osx-san.json  # macOS with sanitizers

# Build DebuggerKit library
cd src/DebuggerKit
nebuild dk-posix.json    # POSIX systems
nebuild dk-osx.json      # macOS
nebuild dk-nekernel.json # NeKernel target

# Build command-line tools (examples)
cd src/CommandLine
nebuild asm.json         # Assembler driver
nebuild ld64.json        # Linker driver
nebuild cppdrv.json      # Preprocessor driver
nebuild pef-amd64-necdrv.json  # Nectar compiler driver
```

### Testing

Tests use CMake + Google Test:

```bash
# Run code generation tests
cd test/test_01_codegen
cmake . && make && ./CodegenTestBasic

# Run linker tests
cd test/test_02_linker
cmake . && make && ./LinkerTestBasic
```

### Code Formatting

**CRITICAL:** Always run `./format.sh` before committing. This uses clang-format to format all `.cc`, `.c`, `.h`, and `.inl` files.

```bash
./format.sh
```

## Architecture Overview

Nectar is a compiler toolchain for the NeKernel operating system, producing **PEF (Preferred Executable Format)** executables from **AE (Advanced Executable)** object files.

### Component Structure

```
src/
├── CompilerKit/          # Core compiler infrastructure (library)
│   ├── src/Assemblers/   # Multi-architecture assemblers
│   ├── src/Compilers/    # Nectar compiler frontend
│   ├── src/Linkers/      # Dynamic linker (PEF format)
│   └── src/Preprocessor/ # Generic preprocessor
│
├── DebuggerKit/          # Debugging infrastructure (library)
│   └── src/              # POSIX/Mach and NeKernel debuggers
│
├── CommandLine/          # CLI tool frontends
│   ├── asm.cc            # Assembler driver
│   ├── ld64.cc           # Linker driver
│   ├── cppdrv.cc         # Preprocessor driver
│   ├── pef-amd64-necdrv.cc  # Nectar compiler driver
│   ├── dbg.cc            # User-space debugger
│   └── kdbg.cc           # Kernel debugger
│
└── LibNectar/               # Nectar ABI runtime (header-only)
```

### Compilation Pipeline

```
Source (.cc) → Preprocessor (cppdrv) → Preprocessed (.pp)
             → Nectar Compiler (pef-amd64-necdrv) → Assembly (.masm)
             → Assembler (asm) → Object File (.obj, AE format)
             → Linker (ld64) → Executable (.exec, PEF format)
```

Example workflow from tests:
```bash
pef-amd64-necdrv sample.cc           # Compile to assembly
asm -asm:x64 sample.cc.pp.masm       # Assemble to object
ld64 -amd64 sample.cc.pp.obj -start __NECTAR_main -output main.exec  # Link
```

### Supported Architectures

The project supports **5 CPU architectures**:
- **AMD64** (x86-64) - Primary target
- **ARM64** (AArch64)
- **PowerPC** (64-bit POWER)
- **64x0** - Open64x0 RISC architecture
- **32x0** - 32-bit variant of Open64x0

Each architecture has dedicated assembler implementations in `src/CompilerKit/src/Assemblers/Assembler+<ARCH>.cc`.

## File Naming Conventions

### Implementation Files
- **Pattern:** `ClassName+Architecture.cc` or `ClassName+Format.cc`
- Examples:
  - `Assembler+AMD64.cc` - AMD64 assembler
  - `CPlusPlusCompiler+AMD64.cc` - Nectar compiler frontend for AMD64
  - `DynamicLinker64+PEF.cc` - 64-bit linker for PEF format

### Test Files
- **Pattern:** `<name>.test.cc`
- Examples: `codegen.test.cc`, `linker.test.cc`

### Build Configuration Files
- **Pattern:** `<component>-<platform>.json`
- Examples: `ck-posix.json`, `dk-osx.json`, `asm.json`

### File Extensions
- `.cc` - Nectar source
- `.h` - Headers
- `.inl` - Inline implementations
- `.obj` - AE object files
- `.exec` - PEF executables
- `.pp` - Preprocessed files
- `.masm`, `.s`, `.S`, `.x64`, `.64x`, `.32x` - Assembly files

## Code Style and Patterns

### Type Aliases (from PreConfig.h)
Use these instead of standard types:
- `Int32`, `UInt32`, `Int64`, `UInt64` instead of `int`, `unsigned`, etc.
- `Char` instead of `char`
- `SizeType` instead of `size_t`
- `STLString` instead of `std::string`
- `VoidPtr`, `UIntPtr` for pointers

### Common Macros
```cpp
// Class semantics
NECTAR_COPY_DEFAULT(ClassName)
NECTAR_COPY_DELETE(ClassName)
NECTAR_MOVE_DEFAULT(ClassName)
NECTAR_MOVE_DELETE(ClassName)

// Assertions and utilities
MUST_PASS(expression)
PACKED  // For packed structs

// Interface patterns
CK_ASSEMBLY_INTERFACE       // : public IAssembly
CK_COMPILER_FRONTEND        // : public ICompilerFrontend
DK_DEBUGGER_CONTRACT        // : public IDebuggerContract
```

### Error Codes
```cpp
NECTAR_SUCCESS = 0
NECTAR_EXEC_ERROR = -30
NECTAR_FILE_NOT_FOUND = -31
NECTAR_UNIMPLEMENTED = -36
NECTAR_INVALID_ARCH = -38
```

### Reference Management
```cpp
StrongRef<T> strong_ref(new T());  // Owns the pointer
WeakRef<T> weak_ref = strong_ref.Leak();  // Non-owning reference
```

## Key Headers and Utilities

### CompilerKit
- `/include/CompilerKit/PEF.h` - PEF executable format definitions
- `/include/CompilerKit/AE.h` - AE object format definitions
- `/include/CompilerKit/AST.h` - Abstract syntax tree for Nectar compiler
- `/include/CompilerKit/Detail/<ARCH>.h` - Architecture-specific definitions
- `/include/CompilerKit/Utilities/Compiler.h` - Common compiler helpers
- `/include/CompilerKit/ErrorOr.h` - Error handling utilities

### DebuggerKit
- `/include/DebuggerKit/DebuggerContract.h` - Debugger interface contracts

### Common Patterns in Code

**Error handling:**
```cpp
if (error_condition) {
  Detail::print_error("reason", "filename");
  return NECTAR_EXEC_ERROR;
}
```

**Factory pattern for assemblers:**
```cpp
AssemblyFactory factory;
factory.Mount(WeakRef<IAssembly>(new AMD64Assembler()));
factory.Compile(sourceFile, AssemblyFactory::kArchAMD64);
```

## NeBuild Configuration Structure

Example JSON configuration:
```json
{
  "compiler_path": "clang++",
  "compiler_std": "c++20",
  "headers_path": ["../../include/CompilerKit", "../../include/"],
  "sources_path": ["src/*.cc", "src/*/*.cc"],
  "output_name": "/usr/lib/libCompilerKit.so",
  "compiler_flags": ["-fPIC", "-shared"],
  "cpp_macros": ["__NECTAR__=202505", "CK_USE_STRUCTS=1"],
  "run_after_build": true
}
```

## Important Constants

- **C++ Standard:** C++20
- **Nectar Version Macro:** `__NECTAR__=202505`
- **PEF Magic:** `"Open"` (or `"nepO"` for FAT binaries)
- **AE Magic:** `"AEF"`
- **Default PEF Base Address:** `0x40000000`
- **Entry Point Symbol:** `__ImageStart`
- **Main Function Symbol:** `__NECTAR_main`

## Git Workflow

- **Main Branch:** `stable` (use this for PRs)
- **Development Branch:** `develop`
- **CI:** Runs on push/PR to `develop` branch
- **Commit Convention:** Run `./format.sh` before every commit

## Project-Specific Context

### Output Formats
- **PEF (Preferred Executable Format):** Custom executable format for NeKernel
  - Extensions: `.exec` (executable), `.dylib` (shared), `.sys` (driver)
  - Sections: `.code64`, `.data64`, `.zero64` (and 128-bit variants)

- **AE (Advanced Executable):** Object file format
  - Relocatable by offset
  - Contains code, data, and BSS records
  - Symbol length: 256 bytes

### Adding New Features

**New assembler instruction:**
- Architecture opcodes: `/include/CompilerKit/Detail/<ARCH>.h`
- Implementation: `/src/CompilerKit/src/Assemblers/Assembler+<ARCH>.cc`

**Modifying Nectar compiler:**
- Frontend: `/src/CompilerKit/src/Compilers/NectarCompiler+AMD64.cc`
- AST: `/include/CompilerKit/AST.h`

**Modifying linker:**
- Implementation: `/src/CompilerKit/src/Linkers/DynamicLinker64+PEF.cc`
- Format specs: `/include/CompilerKit/PEF.h` and `/include/CompilerKit/AE.h`

## Dependencies

- Clang (C++20 support)
- Git
- Boost
- NeBuild (https://github.com/nekernel-org/nebuild)
- Doxygen (for documentation)
- CoreUtils
