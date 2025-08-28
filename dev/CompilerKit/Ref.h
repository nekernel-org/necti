
/*
 *	========================================================
 *
 *	CompilerKit
 * 	Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <CompilerKit/Defines.h>

namespace CompilerKit {
// @author EL Mahrouss Amlal
// @brief Reference holder class, refers to a pointer of data in static memory.
template <typename T>
class Ref final {
 public:
  explicit Ref() = default;

  ~Ref() {
    if (m_Strong) {
      MUST_PASS(m_Class);
      if (m_Class) delete m_Class;
      m_Class = nullptr;
    }
  }

  NECTI_COPY_DEFAULT(Ref);

 public:
  explicit Ref(T* cls, const Bool& strong = false) : m_Class(cls), m_Strong(strong) {}

  Ref& operator=(T ref) {
    *m_Class = ref;
    return *this;
  }

 public:
  T* operator->() const { return m_Class; }

  T& Leak() { return *m_Class; }

  T operator*() { return *m_Class; }

  Bool IsStrong() const { return m_Strong; }

  operator bool() { return *m_Class; }

 private:
  T*   m_Class{nullptr};
  Bool m_Strong{false};
};

// @author EL Mahrouss Amlal
// @brief Non null Reference holder class, refers to a pointer of data in static memory.
template <typename T>
class NonNullRef final {
 public:
  explicit NonNullRef() = delete;

  explicit NonNullRef(T* ref) : m_Ref(ref, true) {}

  Ref<T>& operator->() {
    MUST_PASS(m_Ref);
    return m_Ref;
  }

  NonNullRef& operator=(const NonNullRef<T>& ref) = delete;
  NonNullRef(const NonNullRef<T>& ref)            = default;

 private:
  Ref<T> m_Ref{nullptr};
};
}  // namespace CompilerKit
