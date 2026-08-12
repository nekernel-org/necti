// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-app-eu/ncc

#ifndef NCC_COMPILERKIT_REF_H
#define NCC_COMPILERKIT_REF_H

#include <CompilerKit/Detail/Config.h>

namespace CompilerKit {
/// @author Amlal El Mahrouss
/// @brief Reference holder class, refers to a pointer of data in static memory.
template <typename T>
class StrongRef {
 public:
  StrongRef() = default;

  virtual ~StrongRef() {
    if (mStrong) {
      MUST_PASS(mClass);
      if (mClass) delete mClass;
      mClass = nullptr;
    }
  }

  NCC_COPY_DEFAULT(StrongRef)

  using Type = T;

 protected:
  StrongRef(Type* cls, const bool strong) : mClass(cls), mStrong(strong) {}

 public:
  StrongRef(Type* cls) : mClass(cls), mStrong(true) {}

  StrongRef& operator=(Type* ref) {
    mClass = ref;
    return *this;
  }

 public:
  Type* operator->() const { return mClass; }

  Type* Leak() { return mClass; }

  Type* operator*() { return mClass; }

  bool IsStrong() const { return mStrong; }

  explicit operator bool() { return mClass != nullptr; }

 private:
  Type* mClass{nullptr};
  bool  mStrong{false};
};

template <typename T>
class WeakRef final : public StrongRef<T> {
 public:
  WeakRef()  = delete;
  ~WeakRef() = default;

  NCC_COPY_DEFAULT(WeakRef)

 public:
  using Type = T;

  WeakRef(Type* cls) : StrongRef<Type>(cls, false) {}
};

/// @author Amlal El Mahrouss
/// @brief Non null reference holder class, refers to a pointer of data in static memory.
template <typename Type>
class NonNullRef final {
 public:
  explicit NonNullRef() = delete;
  NonNullRef(Type* ref) : mRef(ref, true) {}

  StrongRef<Type>& operator->() {
    MUST_PASS(mRef);
    return mRef;
  }

  NonNullRef& operator=(const NonNullRef<Type>& ref) = delete;
  NonNullRef(const NonNullRef<Type>& ref)            = default;

 private:
  StrongRef<Type> mRef{nullptr};
};

using StrongAny = StrongRef<VoidPtr>;
using WeakAny   = WeakRef<VoidPtr>;
}  // namespace CompilerKit

#endif  // NCC_COMPILERKIT_REF_H
