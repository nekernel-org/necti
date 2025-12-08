/* ========================================

  Copyright (C) 2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license

======================================== */

#pragma once

#include <CompilerKit/Detail/Config.h>
#include <dlfcn.h>
#include <mutex>

namespace CompilerKit {
struct DLLTraits final {
  typedef Int32 (*Entrypoint)(Int32 argc, Char const* argv[]);
  using DLL = VoidPtr;

  DLL        fDylib{nullptr};
  Entrypoint fEntrypoint{nullptr};
  std::mutex fMutex;

  explicit operator bool() { return fDylib && fEntrypoint; }

  DLLTraits& operator()(const Char* path, const Char* fEntrypoint) {
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

    this->fEntrypoint = (Entrypoint) dlsym(this->fDylib, fEntrypoint);

    if (!this->fEntrypoint) {
      dlclose(this->fDylib);
      this->fDylib = nullptr;

      return *this;
    }

    return *this;
  }

  NECTI_COPY_DELETE(DLLTraits)

  explicit DLLTraits() = default;

  ~DLLTraits() {
    if (this->fDylib) {
      dlclose(this->fDylib);
      this->fDylib = nullptr;
    }

    this->fEntrypoint = nullptr;
  }
};
}  // namespace CompilerKit