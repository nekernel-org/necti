/* -------------------------------------------

  Copyright (C) 2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#pragma once

#include <CompilerKit/Defines.h>
#include <dlfcn.h>

typedef Int32 (*CompilerKitEntrypoint)(Int32 argc, Char const* argv[]);
typedef VoidPtr CompilerKitDylib;
