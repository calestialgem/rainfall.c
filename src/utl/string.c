// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/api.h"

#include <stdbool.h>
#include <stddef.h>

String stringOf(char const* first, char const* after) {
  return (String){.first = first, .after = after};
}

String nullTerminated(char const* array) {
  char const* null = array;
  while (*null) null++;
  return stringOf(array, null);
}

String emptyString() { return stringOf(NULL, NULL); }

ptrdiff_t characters(String string) { return string.after - string.first; }

bool equalStrings(String left, String right) {
  ptrdiff_t length = characters(left);
  if (length != characters(right)) return false;
  for (ptrdiff_t i = 0; i < length; i++)
    if (left.first[i] != right.first[i]) return false;
  return true;
}

#define PRIME 53

ptrdiff_t hashcode(String string) {
  ptrdiff_t hash = 0;
  for (char const* i = string.first; i < string.after; i++) {
    hash *= PRIME;
    hash += *i;
  }
  return hash;
}
