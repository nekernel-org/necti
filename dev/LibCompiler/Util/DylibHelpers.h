/* -------------------------------------------

  Copyright (C) 2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#pragma once

#include <LibCompiler/Defines.h>
#include <dlfcn.h>

typedef Int32 (*LibCompilerEntrypoint)(Int32 argc, Char const* argv[]);
typedef VoidPtr LibCompilerDylib;
