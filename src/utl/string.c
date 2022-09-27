// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "api.h"

#include <stdbool.h>

String stringOf(char const* const terminated) {
  String res = {.bgn = terminated, .end = terminated};
  while (*res.end) res.end++;
  return res;
}

char stringAt(String const str, ux const i) { return str.bgn[i]; }

ux stringLength(String const str) { return str.end - str.bgn; }

bool stringEqual(String const lhs, String const rhs) {
  ux const length = stringLength(lhs);
  if (length != stringLength(rhs)) return false;
  for (ux i = 0; i < length; i++)
    if (stringAt(lhs, i) != stringAt(rhs, i)) return false;
  return true;
}
