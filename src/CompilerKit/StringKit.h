/*
 *	========================================================
 *
 *	CompilerKit
 * 	Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license.
 *
 * 	========================================================
 */

#ifndef __NECTI_STRINGKIT__
#define __NECTI_STRINGKIT__

#include <CompilerKit/Defines.h>
#include <CompilerKit/ErrorOr.h>

/// =========================================================== ///
/// @file StringKit.h
/// @author Amlal El Mahrouss
/// @brief StringKit for CompilerKit.
/// =========================================================== ///

namespace CompilerKit {
class NEStringBuilder;
class NEString;

/**
 * @brief NEString class, contains a C string and manages it.
 * @note No need to manage it it's getting deleted by default.
 */

class NEString final {
 public:
  explicit NEString() = delete;

  explicit NEString(SizeType Sz) noexcept : m_Sz(Sz) {
    m_Data = new Char[Sz];
    assert(m_Data);
  }

  ~NEString() noexcept {
    if (m_Data) {
      memset(m_Data, 0, m_Sz);
      delete[] m_Data;

      m_Data = nullptr;
    }
  }

  NECTI_COPY_DEFAULT(NEString);

  Char*       Data();
  const Char* CData() const;
  SizeType    Length() const;

  bool operator==(const Char* rhs) const;
  bool operator!=(const Char* rhs) const;

  bool operator==(const NEString& rhs) const;
  bool operator!=(const NEString& rhs) const;

  NEString& operator+=(const Char* rhs);
  NEString& operator+=(const Char rhs);
  NEString& operator+=(const NEString& rhs);

  explicit operator bool() { return m_Data && m_Data[0] != 0; }

  bool operator!() { return !m_Data || m_Data[0] == 0; }

 private:
  Char*    m_Data{nullptr};
  SizeType m_Sz{0};
  SizeType m_Cur{0};

  friend class NEStringBuilder;
};

/**
 * @brief NEStringBuilder class
 * @note These results shall call be delete[] after they're used.
 */
struct NEStringBuilder final {
  static NEString Construct(const Char* data);
  static NEString FromInt(const char* fmt, int n);
  static NEString FromBool(const char* fmt, bool n);
  static NEString Format(const char* fmt, const char* from);
  static Bool     Equals(const char* lhs, const char* rhs);
};

using NEStringOr  = ErrorOr<NEString>;
using NEStringPtr = NEString*;
using NEStringRef = Ref<NEString>;
}  // namespace CompilerKit

#endif /* ifndef __NECTI_STRINGKIT__ */
