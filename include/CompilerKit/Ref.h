
/*
 *	========================================================
 *
 *	CompilerKit
 * 	Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license.
 *
 * 	========================================================
 */

#pragma once

#include <CompilerKit/Detail/Config.h>

namespace CompilerKit {
/// @author Amlal El Mahrouss
/// @brief Reference holder class, refers to a pointer of data in static memory.
template <typename T>
class StrongRef {
 public:
  StrongRef() = default;

  virtual ~StrongRef() {
    if (m_Strong) {
      MUST_PASS(m_Class);
      if (m_Class) delete m_Class;
      m_Class = nullptr;
    }
  }

  NECTI_COPY_DELETE(StrongRef)
  NECTI_MOVE_DEFAULT(StrongRef)

  using Type = T;

 protected:
  explicit StrongRef(Type* cls, const bool strong) : m_Class(cls), m_Strong(strong) {}

 public:
  explicit StrongRef(Type* cls) : m_Class(cls), m_Strong(true) {}

  StrongRef& operator=(Type ref) {
    *m_Class = ref;
    return *this;
  }

 public:
  Type* operator->() const { return m_Class; }

  Type& Leak() { return *m_Class; }

  Type operator*() { return *m_Class; }

  bool IsStrong() const { return m_Strong; }

  explicit operator bool() { return m_Class != nullptr; }

 private:
  Type* m_Class{nullptr};
  bool  m_Strong{false};
};

template <typename T>
class WeakRef final : StrongRef<T> {
 public:
  WeakRef() = default;

  ~WeakRef() = default;

  NECTI_COPY_DELETE(WeakRef)
  NECTI_MOVE_DEFAULT(WeakRef)

 public:
  using Type = T;

  explicit WeakRef(Type* cls) : StrongRef<Type>(cls, false) {}
};

/// @author Amlal El Mahrouss
/// @brief Non null reference holder class, refers to a pointer of data in static memory.
template <typename Type>
class NonNullRef final {
 public:
  explicit NonNullRef() = delete;
  explicit NonNullRef(Type* ref) : m_Ref(ref, true) {}

  StrongRef<Type>& operator->() {
    MUST_PASS(m_Ref);
    return m_Ref;
  }

  NonNullRef& operator=(const NonNullRef<Type>& ref) = delete;
  NonNullRef(const NonNullRef<Type>& ref)            = default;

 private:
  StrongRef<Type> m_Ref{nullptr};
};

using StrongAny = StrongRef<VoidPtr>;
using WeakAny = WeakRef<VoidPtr>;
}  // namespace CompilerKit
