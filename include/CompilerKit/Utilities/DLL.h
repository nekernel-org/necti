/* ========================================

  Copyright (C) 2025 Amlal El Mahrouss, licensed under the Apache 2.0 license

======================================== */

#pragma once

#include <CompilerKit/Detail/Config.h>
#include <dlfcn.h>
#include <mutex>

namespace CompilerKit {
class DLLLoader final {
 public:
  typedef Int32 (*EntryT)(Int32 argc, Char const* argv[]);
  using DLL   = VoidPtr;
  using Mutex = std::mutex;
  EntryT fEntrypoint{nullptr};

 private:
  DLL   mDLL{nullptr};
  Mutex mMutex;

 public:
  explicit operator bool() { return this->mDLL && this->fEntrypoint; }

  DLLLoader& operator()(const Char* path, const Char* fEntrypoint) {
    if (!path || !fEntrypoint) return *this;

    std::lock_guard<Mutex> lock(this->mMutex);

    if (this->mDLL) {
      this->Destroy();
    }

    this->mDLL = ::dlopen(path, RTLD_LAZY);

    if (!this->mDLL) {
      return *this;
    }

    this->fEntrypoint = reinterpret_cast<EntryT>(::dlsym(this->mDLL, fEntrypoint));

    if (!this->fEntrypoint) {
      this->Destroy();
      return *this;
    }

    return *this;
  }

  NECTI_COPY_DELETE(DLLLoader)

  explicit DLLLoader() = default;
  ~DLLLoader() { this->Destroy(); }

 private:
  void Destroy() noexcept {
    if (this->mDLL) {
      ::dlclose(this->mDLL);
      this->mDLL = nullptr;
    }

    this->fEntrypoint = nullptr;
  }
};
}  // namespace CompilerKit