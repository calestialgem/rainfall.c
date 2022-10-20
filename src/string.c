// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "test.c"

#include <stdbool.h>
#include <stddef.h>

/* Immutable view of a range of characters. */
typedef struct {
  /* Pointer to the first character if it exists. */
  char const* first;
  /* Pointer to the one after the last character. */
  char const* after;
} String;

/* String without characters. */
String const EMPTY_STRING = {NULL, NULL};

/* View of the characters from the given first one and upto the given one after
 * the range. */
String ViewRange(char const* const first, char const* const after) {
  return (String){first, after};
}

/* Test viewing a range of characters. */
bool TestViewingARange(void) {
  char const* const first = "some characters.";
  char const* const after = first + 5;
  String const      view  = ViewRange(first, after);
  return view.first == first && view.after == after;
}

/* View of the given null-terminated array of characters. */
String ViewTerminated(char const viewed[]) {
  char const* end = viewed;
  while (*end) end++;
  return ViewRange(viewed, end);
}

/* Test viewing a finite null-terminated string. */
bool TestViewFiniteTerminatedString(void) {
  char const   terminated[13] = "some string.";
  size_t const characters     = 12;
  String const view           = ViewTerminated(terminated);
  return view.first == terminated && view.after == terminated + characters;
}

/* Test viewing a null-terminated string of length `1`. */
bool TestViewOneCharacterLongTerminatedString(void) {
  char const   terminated[2] = "s";
  size_t const characters    = 1;
  String const view          = ViewTerminated(terminated);
  return view.first == terminated && view.after == terminated + characters;
}

/* Test viewing an empty null-terminated string. */
bool TestViewEmptyTerminatedString(void) {
  char const   terminated[1] = "";
  size_t const characters    = 0;
  String const view          = ViewTerminated(terminated);
  return view.first == terminated && view.after == terminated + characters;
}

/* Amount of characters in the given view. */
size_t CountCharacters(String const counted) {
  return counted.after - counted.first;
}

/* Test counting characters in a string. */
bool TestCountCharacters(void) {
  size_t const      characters = 5;
  char const* const first      = "some characters.";
  char const* const after      = first + characters;
  String const      view       = ViewRange(first, after);
  return CountCharacters(view) == characters;
}

/* Whether the given strings are equal. */
bool CompareStrings(String const leftCompared, String const rightCompared) {
  // If the character counts are different, they cannot be the same.
  size_t const count = CountCharacters(leftCompared);
  if (count != CountCharacters(rightCompared)) return false;

  // Check all characters.
  for (size_t character = 0; character < count; character++)
    if (leftCompared.first[character] != rightCompared.first[character])
      return false;

  // If all characters are the same, they are equal.
  return true;
}

/* Test comparing same strings for equality. */
bool TestCompareEqualStrings(void) {
  return CompareStrings(
    ViewTerminated("some string"), ViewTerminated("some string"));
}

/* Test comparing strings of different length. */
bool TestCompareDifferentLengthStrings(void) {
  return !CompareStrings(
    ViewTerminated("some string"), ViewTerminated("some other string"));
}

/* Test comparing different strings of same length. */
bool TestCompareEqualLengthDifferentStrings(void) {
  return !CompareStrings(
    ViewTerminated("some three characters: aaa"),
    ViewTerminated("some three characters: bbb"));
}

/* Hashcode of the given string. */
size_t HashString(String const hashed) {
  // Prime number for combining hashes. `53` is selected because the English
  // alphabet has `26` characters. Identifiers in the Thrice source are made out
  // of upper and lower case English letters, and underscores; thus, the prime
  // is the amount of different hashcodes there could be for a character.
  size_t const factor = 53;

  // Combine and accumulate the hashcodes for all the characters.
  size_t hashcode = 0;
  for (char const* character = hashed.first; character < hashed.after;
       character++) {
    hashcode *= factor;

    // A character's hashcode is itself.
    hashcode += *character;
  }

  return hashcode;
}

/* Test hashing an empty string. */
bool TestHashingEmptyString(void) { return !HashString(ViewTerminated("")); }

/* Test hashing a string of length `1`. */
bool TestHashingOneCharacterLongString(void) {
  return HashString(ViewTerminated("C")) == 'C';
}

/* Register the tests in the string module. */
void RegisterStringModuleTests(void) {
  // Test `ViewRange`.
  RegisterUnitTest(TestViewingARange);

  // Test `ViewTerminated`.
  RegisterUnitTest(TestViewFiniteTerminatedString);
  RegisterUnitTest(TestViewOneCharacterLongTerminatedString);
  RegisterUnitTest(TestViewEmptyTerminatedString);

  // Test `CountCharacters`.
  RegisterUnitTest(TestCountCharacters);

  // Test `CompareStrings`.
  RegisterUnitTest(TestCompareEqualStrings);
  RegisterUnitTest(TestCompareDifferentLengthStrings);
  RegisterUnitTest(TestCompareEqualLengthDifferentStrings);

  // Test `HashString`.
  RegisterUnitTest(TestHashingEmptyString);
  RegisterUnitTest(TestHashingOneCharacterLongString);
}
