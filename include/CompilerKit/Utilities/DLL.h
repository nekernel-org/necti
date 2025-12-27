// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_COMPILERKIT_UTILITIES_DLL_H
#define NECTAR_COMPILERKIT_UTILITIES_DLL_H

#include <CompilerKit/Detail/Config.h>
#include <dlfcn.h>
#include <mutex>

namespace CompilerKit {
class DLLLoader final {
 public:
  using EntryT  = Int32 (*)(Int32 argc, Char const* argv[]);
  using HandleT = VoidPtr;
  using MutexT  = std::mutex;

  EntryT fEntrypoint{nullptr};

 private:
  HandleT mDLL{nullptr};
  MutexT  mMutex;

 public:
  explicit operator bool() { return this->mDLL; }

  DLLLoader& operator()(const Char* path, const Char* entrypoint) {
    if (!path || !entrypoint) return *this;

    std::lock_guard<MutexT> lock(this->mMutex);

    if (this->mDLL) {
      this->Destroy();
    }

    this->mDLL = ::dlopen(path, RTLD_LAZY);

    if (!this->mDLL) {
      return *this;
    }

    this->fEntrypoint = reinterpret_cast<EntryT>(::dlsym(this->mDLL, entrypoint));

    if (!this->fEntrypoint) {
      this->Destroy();
      return *this;
    }

    return *this;
  }

  NECTAR_COPY_DELETE(DLLLoader)

  DLLLoader() = default;
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

#endif  // NECTAR_COMPILERKIT_UTILITIES_DLL_H
