/*
 *	========================================================
 *
 *	CompilerKit
 * 	Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <CompilerKit/StringKit.h>
#include <CompilerKit/Defines.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/Ref.h>

namespace CompilerKit {
using ErrorT = UInt32;

template <typename T>
class ErrorOr final {
 public:
  ErrorOr()  = default;
  ~ErrorOr() = default;

 public:
  explicit ErrorOr(ErrorT err) : mId(err) {}
  explicit ErrorOr(nullPtr null) {}
  explicit ErrorOr(T klass) : mRef(klass) {}

  ErrorOr& operator=(const ErrorOr&) = default;
  ErrorOr(const ErrorOr&)            = default;

  Ref<T> Leak() { return mRef; }

  ErrorT Error() { return mId; }

  Bool HasError() { return mId != NECTI_SUCCESS; }

  explicit operator bool() { return mRef; }

 private:
  Ref<T> mRef;
  ErrorT  mId{0};
};

using ErrorOrAny = ErrorOr<voidPtr>;
using ErrorOrString = ErrorOr<STLString>;
}  // namespace CompilerKit
