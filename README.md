<!-- Read Me of NCTI -->

<div align="center">
  <img src="meta/png/nekernel.png" alt="Logo" width="256"/>
</div>

<br/>

![CI](https://github.com/amlel-el-mahrouss/cc/actions/workflows/c-cpp.yml/badge.svg)
[![License: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](LICENSE)

## Overview:

NeCTI is a modern, multi-platform compiler instractucture designed for modularity, and performance. It features a custom debugger engine, advanced linker, and flexible backend/frontend system. NeCTI is built for research, education, and next-generation toolchain development.

## Structure:

- `dev/CompilerKit` – Compiler Infrastructure Framework written in modern C++
- `dev/LibC++` – C++ ABI Library
- `dev/LibStdC++` – Standard C++ Library
- `tools/` – Frontend Tools
- `dev/DebuggerKit` – Debugging Library written in modern C++


## Requirements:

- [Clang](https://clang.llvm.org/)
- [Git](https://git-scm.com/)
- [NeBuild](https://github.com/nekernel-org/nebuild)

## Notice for Doxygen:

- Use doxygen to build documentation.
- You need it installed in order to generate offline documentation!

## Notice for Contributors:

- Always use `format.sh` before commiting and pushing your code!

## Getting Started:

```sh
git clone git@github.com:nekernel-org/necti.git
cd necti
# Either build debugger or compiler libraries/tools using nebuild.
```

###### Copyright (C) 2024-2025 Amlal El Mahrouss & NeKernel.org Contributors, all rights reserved.
