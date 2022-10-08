// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* Signed, index and size integer. */
typedef ptrdiff_t iptr;

/* Immutable view of a range of characters. */
typedef struct {
  /* Pointer to the first character if it exists. */
  char const* bgn;
  /* Pointer to one after the last character. */
  char const* end;
} String;

/* Dynamicly allocated array of strings that uses hashcode for fast element
 * existance checking. */
typedef struct {
  /* Pointer to the first string if it exists. */
  String* bgn;
  /* Pointer to one after the last string. */
  String* end;
  /* Amount of strings. */
  iptr    len;
} Set;

/* Pair of a string as the key and a corresponding index as a value. */
typedef struct {
  /* String that differentiates the index. */
  String key;
  /* The index value. */
  iptr   val;
} MapEntry;

/* Dynamicly allocated array of string and index pairs that uses hashcode for
 * fast element existance checking and access. */
typedef struct {
  /* Pointer to the first entry if it exists. */
  MapEntry* bgn;
  /* Pointer to one after the last entry. */
  MapEntry* end;
  /* Amount of strings. */
  iptr      len;
} Map;

/* Dynamicly allocated array of characters. Allocation costs are amortized by
 * growing at least the half of the current capacity. */
typedef struct {
  /* Pointer to the first character if it exists. */
  char* bgn;
  /* Pointer to one after the last character. */
  char* end;
  /* Pointer to one after the last allocated character. */
  char* all;
} Buffer;

/* String of the null-terminated array of characters. */
String strOf(char const* terminated);
/* Empty string. */
String strOfEmpty();
/* Character at the given index in the given string. */
char   strAt(String str, iptr i);
/* Amount of characters in the string. */
iptr   strLen(String str);
/* Whether the given strings equal. */
bool   strEq(String lhs, String rhs);
/* Hashcode of the given string. */
iptr   strHash(String str);
/* Stream out all the characters in the given string to the given stream. */
void   strWrite(String str, FILE* stream);

/* Set with the given initial capacity. */
Set           setOf(iptr cap);
/* Release the memory resources used by the given set. */
void          setFree(Set* set);
/* Put the given string to the given set. */
void          setPut(Set* set, String str);
/* Pointer to the string in the given set that is equal to the given string.
 * Returns null if there is no string that is equal. */
String const* setGet(Set set, String str);

/* Map with the given initial capacity. */
Map             mapOf(iptr cap);
/* Release the memory resources used by the given map. */
void            mapFree(Map* map);
/* Put the given index with the given string as the key to the given map. */
void            mapPut(Map* map, String key, iptr val);
/* Pointer to the entry in the given map that corresponds to the given key.
 * Returns null if there is no entry with the given key. */
MapEntry const* mapGet(Map map, String key);
/* Pointer to the string in the given map that corresponds to the given key.
 * Returns null if there is no entry with the given key. */
String const*   mapGetKey(Map map, String key);
/* Pointer to the index in the given map that corresponds to the given key.
 * Returns null if there is no entry with the given key. */
iptr const*     mapGetVal(Map map, String key);

/* Buffer with the given initial capacity. */
Buffer bfrOf(iptr cap);
/* Copy of the given buffer. */
Buffer bfrOfCopy(Buffer bfr);
/* Release the memory resources used by the given buffer. */
void   bfrFree(Buffer* bfr);
/* Amount of characters in the given buffer. */
iptr   bfrLen(Buffer bfr);
/* Amount of allocated characters in the given buffer. */
iptr   bfrCap(Buffer bfr);
/* Character at the given index in the given buffer. */
char   bfrAt(Buffer bfr, iptr i);
/* View of the given buffer. */
String bfrView(Buffer bfr);
/* Append the given string to the given buffer. */
void   bfrAppend(Buffer* bfr, String str);
/* Put the given character to the given buffer. */
void   bfrPut(Buffer* bfr, char c);
/* Stream in all the characters from the given stream to the given buffer. */
void   bfrRead(Buffer* bfr, FILE* stream);
/* Stream out all the characters in the given buffer to the given stream. */
void   bfrWrite(Buffer bfr, FILE* stream);
