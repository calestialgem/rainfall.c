// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/api.h"

#include <stdbool.h>

String strOf(char const* const terminated) {
  String res = {.bgn = terminated, .end = terminated};
  while (*res.end) res.end++;
  return res;
}

String strOfEmpty() { return (String){0}; }

char strAt(String const str, ux const i) { return str.bgn[i]; }

ux strLen(String const str) { return str.end - str.bgn; }

bool strEq(String const lhs, String const rhs) {
  ux const length = strLen(lhs);
  if (length != strLen(rhs)) return false;
  for (ux i = 0; i < length; i++)
    if (strAt(lhs, i) != strAt(rhs, i)) return false;
  return true;
}

ux strHash(String const str) {
  ux const PRIME = 53;
  ux       hash  = 0;
  for (char const* i = str.bgn; i < str.end; i++) {
    hash *= PRIME;
    hash += *i;
  }
  return hash;
}

void strWrite(String const str, FILE* const stream) {
  fwrite(str.bgn, sizeof(char), strLen(str), stream);
}
