/*
 *	========================================================
 *
 *	CompilerKit
 * 	Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license.
 *
 * 	========================================================
 */

/**
 * @file StringKit.cc
 * @author Amlal (amlal@nekernel.org)
 * @brief C++ string manipulation API.
 * @version 0.0.2
 * @date 2024-01-23
 *
 * @copyright Copyright (c) Amlal El Mahrouss
 *
 */

#include <CompilerKit/StringKit.h>

namespace CompilerKit {

Char* NEString::Data() {
  return m_Data;
}

const Char* NEString::CData() const {
  return m_Data;
}

SizeType NEString::Length() const {
  return strlen(m_Data);
}

bool NEString::operator==(const NEString& rhs) const {
  const SizeType len = Length();
  if (rhs.Length() != len) return false;
  return memcmp(m_Data, rhs.m_Data, len) == 0;
}

bool NEString::operator==(const Char* rhs) const {
  const SizeType rhs_len = string_length(rhs);
  const SizeType len     = Length();
  if (rhs_len != len) return false;
  return memcmp(m_Data, rhs, len) == 0;
}

bool NEString::operator!=(const NEString& rhs) const {
  return !(*this == rhs);
}

bool NEString::operator!=(const Char* rhs) const {
  return !(*this == rhs);
}

NEString NEStringBuilder::Construct(const Char* data) {
  if (!data || *data == 0) return NEString(0);

  NEString view(strlen(data));
  view += data;

  return view;
}

NEString NEStringBuilder::FromInt(const char* fmt, int i) {
  if (!fmt) return NEString(0);

  Char result[sizeof(int64_t)] = {0};
  if (!to_str(result, sizeof(int64_t), i)) return NEString(0);

  const SizeType fmt_len = string_length(fmt);
  const SizeType res_len = string_length(result);

  NEString output(fmt_len + res_len);
  bool        inserted = false;

  for (SizeType idx = 0; idx < fmt_len; ++idx) {
    if (!inserted && fmt[idx] == '%') {
      output += result;
      inserted = true;
      continue;
    }
    output += Char{fmt[idx]};
  }

  return output;
}

NEString NEStringBuilder::FromBool(const char* fmt, bool val) {
  if (!fmt) return NEString(0);

  const Char*    boolean_expr = val ? "true" : "false";
  const SizeType fmt_len      = string_length(fmt);
  const SizeType res_len      = string_length(boolean_expr);

  NEString output(fmt_len + res_len);
  bool        inserted = false;

  for (SizeType idx = 0; idx < fmt_len; ++idx) {
    if (!inserted && fmt[idx] == '%') {
      output += boolean_expr;
      inserted = true;
      continue;
    }
    output += Char{fmt[idx]};
  }

  return output;
}

bool NEStringBuilder::Equals(const char* lhs, const char* rhs) {
  const SizeType lhs_len = string_length(lhs);
  const SizeType rhs_len = string_length(rhs);

  if (lhs_len != rhs_len) return false;
  return memcmp(lhs, rhs, lhs_len) == 0;
}

NEString NEStringBuilder::Format(const char* fmt, const char* fmtRight) {
  if (!fmt || !fmtRight) return NEString(0);

  const SizeType fmt_len = string_length(fmt);
  const SizeType rhs_len = string_length(fmtRight);

  NEString output(fmt_len + rhs_len);
  bool        inserted = false;

  for (SizeType idx = 0; idx < fmt_len; ++idx) {
    if (!inserted && fmt[idx] == '%') {
      output += fmtRight;
      inserted = true;
      continue;
    }
    output += Char{fmt[idx]};
  }

  return output;
}

NEString& NEString::operator+=(const Char* rhs) {
  const SizeType rhs_len = strlen(rhs);
  if (this->m_Cur + rhs_len >= this->m_Sz) {
    throw std::runtime_error("out_of_bounds: NEString");
  }

  memcpy(this->m_Data + this->m_Cur, rhs, rhs_len);

  this->m_Cur += rhs_len;
  this->m_Data[this->m_Cur] = '\0';

  return *this;
}

NEString& NEString::operator+=(const NEString& rhs) {
  if (this->m_Cur + rhs.m_Cur >= this->m_Sz) {
    throw std::runtime_error("out_of_bounds: NEString");
  }

  memcpy(this->m_Data + this->m_Cur, rhs.CData(), rhs.m_Cur);
  this->m_Cur += rhs.m_Cur;
  this->m_Data[this->m_Cur] = '\0';

  return *this;
}

NEString& NEString::operator+=(const Char ch) {
  if (this->m_Cur + 1 >= this->m_Sz) {
    throw std::runtime_error("out_of_bounds.");
  }

  this->m_Data[this->m_Cur++] = ch;
  this->m_Data[this->m_Cur]   = '\0';

  return *this;
}

}  // namespace CompilerKit
