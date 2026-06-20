// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-app-eu/ncc

/// BUGS: 0

#include <CompilerKit/AST.h>
#include <CompilerKit/Detail/AMD64.h>
#include <CompilerKit/PEF.h>
#include <CompilerKit/UUID.h>
#include <CompilerKit/Utils/Compiler.h>

/* Nectar Compiler Check Driver. */
/* This is part of the CompilerKit. */
/* (c) Amlal El Mahrouss 2026 */

using namespace CompilerKit;

static bool kInIfBody    = false;
static bool kInElseBody  = false;
static bool kInTraitBody = false;

NC_IMPORT_C bool NectarCheckLine(CompilerKit::STLString& input) {
  if (input.empty()) return false;

  if (input.ends_with(":")) {
    if (!input.ends_with("):")) {
      Detail::print_error("Invalid keyword 'else if' is not a Nectar keyword!", "check");
      return false;
    }

    kInIfBody = true;
  }

  if (input.find("(") != CompilerKit::STLString::npos) {
    if (input.find(")") == CompilerKit::STLString::npos) {
      Detail::print_error("Invalid call to function, Nectar expects the ')' character at the end!",
                          "check");
      return false;
    }
  }

  if (input.find("let ") != CompilerKit::STLString::npos && !input.ends_with(";")) {
    if (input.find(":=") != CompilerKit::STLString::npos) {
      Detail::print_error("A declaration must always end with ';'", "check");
      return false;
    }
  }

  if (input.find(":=") != CompilerKit::STLString::npos) {
    if (input.find(";") == CompilerKit::STLString::npos) {
      Detail::print_error("An assignment call must always end with ';'", "check");
      return false;
    }
  }

  if (input.find("(") != CompilerKit::STLString::npos &&
      input.find("const") == CompilerKit::STLString::npos &&
      input.find("let") == CompilerKit::STLString::npos &&
      input.find("if") == CompilerKit::STLString::npos &&
      input.find("else") == CompilerKit::STLString::npos) {
    if (input.find(";") == CompilerKit::STLString::npos) {
      Detail::print_error("A function call must always end with ';'", "check");
      return false;
    }
  }

  if (input.find("const ") != CompilerKit::STLString::npos && !input.ends_with(";")) {
    if (input.find(":=") != CompilerKit::STLString::npos) {
      Detail::print_error("A declaration must always end with ';'", "check");
      return false;
    }
  }

  if (input.find("let ") != CompilerKit::STLString::npos && input.ends_with(";")) {
    if (input.find(":=") == CompilerKit::STLString::npos) {
      Detail::print_error("A declaration must always include with ':='", "check");
      return false;
    }
  }

  if (input.find("const ") != CompilerKit::STLString::npos && input.ends_with(";")) {
    if (input.find(":=") == CompilerKit::STLString::npos) {
      Detail::print_error("A declaration must always end with ':='", "check");
      return false;
    }
  }

  if (input == "}" || input == "}\n" || input == "}\r\n") {
    if (kInIfBody) kInIfBody = false;
  }

  if (input == "}" || input == "}\n" || input == "}\r\n") {
    if (kInTraitBody) kInTraitBody = false;
  }

  return true;
}
