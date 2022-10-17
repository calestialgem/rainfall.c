// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility/api.h"

#include <stdbool.h>
#include <stddef.h>

String const EMPTY_STRING = {.first = NULL, .after = NULL};

String createString(char const* first, char const* after) {
  return (String){.first = first, .after = after};
}

String viewTerminated(char const* terminatedArray) {
  // Find the null character, and create a string upto it.
  char const* termination = terminatedArray;
  while (*termination) termination++;
  return createString(terminatedArray, termination);
}

size_t countCharacters(String counted) { return counted.after - counted.first; }

bool compareStringEquality(String leftChecked, String rightChecked) {
  // First compare whether the sizes are equal.
  size_t count = countCharacters(leftChecked);
  if (count != countCharacters(rightChecked)) return false;

  // If at any index the characters do not match, they are not equal.
  for (size_t index = 0; index < count; index++)
    if (leftChecked.first[index] != rightChecked.first[index]) return false;
  return true;
}

/* Prime number for combining hashes. `53` is selected because the English
 * alphabet has 26 characters. Identifiers in the Thrice source are made out of
 * upper and lower case English letters, and underscores; thus, the prime is
 * the amount of different hashcodes there could be for a character. */
#define HASHCODE_PRIME_FACTOR 53

size_t calculateHashcode(String calculated) {
  // Combine and accumulate the hashcodes for all the characters.
  size_t hashcode = 0;
  for (char const* i = calculated.first; i < calculated.after; i++) {
    hashcode *= HASHCODE_PRIME_FACTOR;
    hashcode += *i;
  }
  return hashcode;
}
