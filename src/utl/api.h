// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* Immutable view of a range of characters. */
typedef struct {
  /* Pointer to the first character if it exists. */
  char const* first;
  /* Pointer to one after the last character. */
  char const* after;
} String;

/* View of the characters between the given pointers. */
String stringOf(char const* first, char const* after);
/* View of the given null-terminated array. */
String nullTerminated(char const* array);
/* String without any characters. */
String emptyString(void);
/* Amount of characters in the given string. */
size_t characters(String source);
/* Whether the given strings are equal. */
bool   equalStrings(String left, String right);
/* Hashcode of the given string. */
size_t hashcode(String source);

/* Dynamicly allocated array of bytes. Allocation costs are amortized by growing
 * at least the half of the current capacity. */
typedef struct {
  /* Pointer to the first character if it exists. */
  char* first;
  /* Pointer to one after the last character. */
  char* after;
  /* Pointer to one after the last allocated character. */
  char* bound;
} Buffer;

/* Clean buffer that does not allocate any memory. */
Buffer emptyBuffer(void);
/* Copy of the given buffer. */
Buffer copyBuffer(Buffer source);
/* Release the memory used by the given buffer. */
void   disposeBuffer(Buffer* target);
/* Amount of bytes in the given buffer. */
size_t bytes(Buffer source);
/* Insert the given byte to the end of the given buffer. */
void   put(Buffer* target, char putted);
/* Insert the bytes in the given string to the end of the given buffer. */
void   append(Buffer* target, String appended);
/* Insert all the bytes from the given stream to the end of the given buffer. */
void   read(Buffer* target, FILE* read);

/* Dynamicly allocated array of strings that uses hashcode for fast element
 * existance checking. */
typedef struct {
  /* Pointer to the first string if it exists. */
  String* first;
  /* Pointer to one after the last string. */
  String* after;
  /* Amount of strings. */
  size_t  members;
} Set;

/* Set without any elements. */
Set           emptySet(void);
/* Release the memory used by the given set. */
void          disposeSet(Set* target);
/* Put the given string to the given set as a new member. String should not be a
 * member, it sould be a new string! */
void          insertMember(Set* target, String inserted);
/* Pointer to the string in the given set that is equal to the given string.
 * Returns null if there is no string that is equal. */
String const* accessMember(Set source, String accessed);

/* Pair of a string as the key and a corresponding index as a value. */
typedef struct {
  /* String that differentiates the index. */
  String key;
  /* The index value. */
  size_t value;
} MapEntry;

/* Dynamicly allocated array of string and index pairs that uses hashcode for
 * fast element existance checking and access. */
typedef struct {
  /* Pointer to the first entry if it exists. */
  MapEntry* first;
  /* Pointer to one after the last entry. */
  MapEntry* after;
  /* Amount of entries. */
  size_t    entries;
} Map;

/* Map without any entries. */
Map  emptyMap(void);
/* Release the memory used by the given map. */
void disposeMap(Map* target);
/* Put the given index with the given string as the key to the given map. String
 * should not be a member, it sould be a new string! */
void insertEntry(Map* target, String insertedKey, size_t insertedValue);
/* Pointer to the entry in the given map that corresponds to the given key.
 * Returns null if there is no entry with the given key. */
MapEntry const* accessEntry(Map source, String accessedKey);
/* Pointer to the string in the given map that corresponds to the given key.
 * Returns null if there is no entry with the given key. */
String const*   accessKey(Map source, String accessedKey);
/* Pointer to the index in the given map that corresponds to the given key.
 * Returns null if there is no entry with the given key. */
size_t const*   accessValue(Map source, String accessedKey);
