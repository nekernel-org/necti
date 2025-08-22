/* -------------------------------------------

  Copyright (C) 2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#pragma once

#include <CompilerKit/Defines.h>
#include <mutex>
#include <dlfcn.h>

struct CompilerKitDylibTraits;

typedef Int32 (*CompilerKitEntrypoint)(Int32 argc, Char const* argv[]);
typedef VoidPtr CompilerKitDylib;

struct CompilerKitDylibTraits final {
  CompilerKitDylib fDylib{nullptr};
  CompilerKitEntrypoint fEntrypoint{nullptr};
  std::mutex fMutex;

  CompilerKitDylibTraits& operator()(const Char* path, const Char* fEntrypoint) {
    std::lock_guard<std::mutex> lock(this->fMutex);
    
    if (!path || !fEntrypoint) return *this;

    if (this->fDylib) {
      dlclose(this->fDylib);
      this->fDylib = nullptr;
    }

    this->fDylib = dlopen(path, RTLD_LAZY);

    if (!this->fDylib) {
      return *this;
    }

    this->fEntrypoint = (CompilerKitEntrypoint)dlsym(this->fDylib, fEntrypoint);

    if (!this->fEntrypoint) {
      dlclose(this->fDylib);
      this->fDylib = nullptr;

      return *this;
    }

    return *this;
  }

  NECTI_COPY_DELETE(CompilerKitDylibTraits);

  CompilerKitDylibTraits() = default;

  ~CompilerKitDylibTraits() {
    if (this->fDylib) {
      dlclose(this->fDylib);
      this->fDylib = nullptr;
    }

    this->fEntrypoint = nullptr;
  }
};
