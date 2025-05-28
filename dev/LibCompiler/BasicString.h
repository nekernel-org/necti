/*
 *	========================================================
 *
 *	LibCompiler
 * 	Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <LibCompiler/Defines.h>
#include <LibCompiler/ErrorOr.h>

namespace LibCompiler {
class StringBuilder;
class BasicString;

/**
 * @brief BasicString class, contains a C string and manages it.
 * @note No need to manage it it's getting deleted by default.
 */

class BasicString final {
 public:
  explicit BasicString() = delete;

  explicit BasicString(SizeType Sz) noexcept : m_Sz(Sz) {
    m_Data = new CharType[Sz];
    assert(m_Data);
  }

  ~BasicString() noexcept {
    if (m_Data) {
      memset(m_Data, 0, m_Sz);
      delete[] m_Data;

      m_Data = nullptr;
    }
  }

  LIBCOMPILER_COPY_DEFAULT(BasicString);

  CharType*       Data();
  const CharType* CData() const;
  SizeType        Length() const;

  bool operator==(const CharType* rhs) const;
  bool operator!=(const CharType* rhs) const;

  bool operator==(const BasicString& rhs) const;
  bool operator!=(const BasicString& rhs) const;

  BasicString& operator+=(const CharType* rhs);
  BasicString& operator+=(const BasicString& rhs);

  operator bool() { return m_Data && m_Data[0] != 0; }

  bool operator!() { return !m_Data || m_Data[0] == 0; }

 private:
  CharType* m_Data{nullptr};
  SizeType  m_Sz{0};
  SizeType  m_Cur{0};

  friend class StringBuilder;
};

/**
 * @brief StringBuilder class
 * @note These results shall call be delete[] after they're used.
 */
struct StringBuilder final {
  static BasicString Construct(const CharType* data);
  static const char* FromInt(const char* fmt, int n);
  static const char* FromBool(const char* fmt, bool n);
  static const char* Format(const char* fmt, const char* from);
  static bool        Equals(const char* lhs, const char* rhs);
};

using PStringOr = ErrorOr<BasicString>;
}  // namespace LibCompiler
