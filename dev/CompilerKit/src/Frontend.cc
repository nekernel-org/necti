/* -------------------------------------------

  Copyright (C) 2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#include <CompilerKit/Frontend.h>

namespace CompilerKit {
/// find the perfect matching word in a haystack.
/// \param haystack base string
/// \param needle the string we search for.
/// \return if we found it or not.
BOOL find_word(STLString haystack, STLString needle) noexcept {
  auto index = haystack.find(needle);

  // check for needle validity.
  if (index == STLString::npos) return false;

  // declare lambda
  auto not_part_of_word = [&](int index) {
    if (std::isspace(haystack[index]) || std::ispunct(haystack[index])) return true;

    if (index <= 0 || index >= haystack.size()) return true;

    return false;
  };

  return not_part_of_word(index - 1) && not_part_of_word(index + needle.size());
}

/// find a word within strict conditions and returns a range of it.
/// \param haystack
/// \param needle
/// \return position of needle.
SizeType find_word_range(STLString haystack, STLString needle) noexcept {
  auto index = haystack.find(needle);

  // check for needle validity.
  if (index == STLString::npos) return false;

  if (!isalnum((haystack[index + needle.size() + 1])) &&
      !isdigit(haystack[index + needle.size() + 1]) &&
      !isalnum((haystack[index - needle.size() - 1])) &&
      !isdigit(haystack[index - needle.size() - 1])) {
    return index;
  }

  return STLString::npos;
}
}  // namespace CompilerKit