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

size_t characters(String source) { return source.after - source.first; }

bool equalStrings(String left, String right) {
  size_t length = characters(left);
  if (length != characters(right)) return false;
  for (size_t index = 0; index < length; index++)
    if (left.first[index] != right.first[index]) return false;
  return true;
}

/* Prime number for combining hashes. `53` is selected because the English
 * alphabet has 26 characters. Identifiers in the Thrice source are made out of
 * upper and lower case English letters, and underscores; thus, the prime is
 * the amount of different hashcodes there could be for a character. */
#define PRIME 53

size_t hashcode(String source) {
  size_t hash = 0;
  for (char const* i = source.first; i < source.after; i++) {
    hash *= PRIME;
    hash += *i;
  }
  return hash;
}
