// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss/nectar

#ifndef NECTAR_COMPILERKIT_UTILITIES_DLL_H
#define NECTAR_COMPILERKIT_UTILITIES_DLL_H

#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/Ref.h>
#include <dlfcn.h>
#include <mutex>

namespace CompilerKit {

#ifdef NC_POSIX
class ModuleLoader final {
 public:
  using EntryT  = Int32 (*)(Int32 argc, char const* argv[]);
  using HandleT = VoidPtr;
  using MutexT  = std::mutex;

  EntryT fEntrypoint{nullptr};

 private:
  HandleT mDLL{nullptr};
  MutexT  mMutex;

 public:
  explicit operator bool() { return this->mDLL != nullptr; }

  ModuleLoader& operator()(const std::string& path, const std::string& entrypoint) {
    if (path.empty() || entrypoint.empty()) return *this;

    std::lock_guard<MutexT> lock(this->mMutex);

    if (this->mDLL) {
      this->Reset();
    }

    this->mDLL = ::dlopen(path.data(), RTLD_LAZY);

    if (!this->mDLL) {
      return *this;
    }

    this->fEntrypoint = reinterpret_cast<EntryT>(::dlsym(this->mDLL, entrypoint.data()));

    if (!this->fEntrypoint) {
      this->Reset();
      return *this;
    }

    return *this;
  }

  NECTAR_COPY_DELETE(ModuleLoader)

  ModuleLoader() = default;
  ~ModuleLoader() { this->Reset(); }

  void Reset() noexcept {
    if (this->mDLL) {
      ::dlclose(this->mDLL);
      this->mDLL = nullptr;
    }

    this->fEntrypoint = nullptr;
  }
};

using StrongDLLRef = StrongRef<ModuleLoader>;
using WeakDLLRef   = WeakRef<ModuleLoader>;
#else
#error No ModuleLoader defined.
#endif

}  // namespace CompilerKit

#endif  // NECTAR_COMPILERKIT_UTILITIES_DLL_H
