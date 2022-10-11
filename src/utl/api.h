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

/* String without any characters in it. */
extern String const EMPTY_STRING;

/* View of the characters between the given pointers. */
String createString(char const* first, char const* after);
/* View of the given null-terminated array. */
String viewTerminated(char const* terminatedArray);
/* Amount of characters in the given string. */
size_t countCharacters(String counted);
/* Whether the given strings are equal. */
bool   compareStringEquality(String leftChecked, String rightChecked);
/* Hashcode of the given string. */
size_t calculateHashcode(String calculated);

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

/* Empty buffer with the given initial capacity. */
Buffer createBuffer(size_t initialCapacity);
/* Copy of the given buffer. */
Buffer copyBuffer(Buffer copied);
/* Release the memory used by the given buffer. */
void   disposeBuffer(Buffer* disposed);
/* Amount of bytes in the given buffer. */
size_t countBytes(Buffer counted);
/* Insert the given byte to the end of the given buffer. */
void   appendCharacter(Buffer* target, char appended);
/* Insert the bytes in the given string to the end of the given buffer. */
void   appendString(Buffer* target, String appended);
/* Insert all the bytes from the given stream to the end of the given buffer. */
void   appendStream(Buffer* target, FILE* appended);

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

/* Empty set with the given initial capacity. */
Set           createSet(size_t initialCapacity);
/* Release the memory used by the given set. */
void          disposeSet(Set* disposed);
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

/* Empty map with the given initial capacity. */
Map  createMap(size_t initialCapacity);
/* Release the memory used by the given map. */
void disposeMap(Map* disposed);
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
