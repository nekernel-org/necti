// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_COMPILERKIT_ERROROR_H
#define NECTAR_COMPILERKIT_ERROROR_H

/// =========================================================== ///
/// @file ErrorOr.h
/// @author Amlal El Mahrouss
/// @brief ErrorOr for CompilerKit.
/// =========================================================== ///

#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/Ref.h>

namespace CompilerKit {
using ErrorT = Int32;

template <typename T>
class ErrorOr final {
 public:
  ErrorOr()  = default;
  ~ErrorOr() = default;

 public:
  using RefType   = StrongRef<T>;
  using Reference = T&;
  using Ptr       = T*;

  explicit ErrorOr(ErrorT err) : mId(err) {}
  explicit ErrorOr(std::nullptr_t null) {}
  explicit ErrorOr(T klass) : mRef(klass) {}

  ErrorOr& operator=(const ErrorOr&) = default;
  ErrorOr(const ErrorOr&)            = default;

  RefType& Leak() { return mRef; }

  ErrorT Error() { return mId; }

  bool HasError() { return mId != NECTAR_SUCCESS; }

  explicit operator bool() { return mRef; }

 private:
  RefType mRef;
  ErrorT  mId{0};
};

using ErrorOrAny    = ErrorOr<VoidPtr>;
using ErrorOrString = ErrorOr<STLString>;
}  // namespace CompilerKit

#endif  // NECTAR_COMPILERKIT_ERROROR_H
