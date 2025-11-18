
/* -------------------------------------------

  Copyright (C) 2025, Amlal El Mahrouss, licensed under the Apache 2.0 license.

------------------------------------------- */

#pragma once

#include <LibC++/defines.h>

void* operator new(size_t);
void* operator new[](size_t);

void operator delete(void*) noexcept;
void operator delete(void*, unsigned long);
void operator delete[](void*) noexcept;