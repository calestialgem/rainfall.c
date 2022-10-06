// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Signed, 1-byte integer. */
typedef int8_t   i1;
/* Signed, 2-byte integer. */
typedef int16_t  i2;
/* Signed, 4-byte integer. */
typedef int32_t  i4;
/* Signed, 8-byte integer. */
typedef int64_t  i8;
/* Signed, pointer-size integer. */
typedef intptr_t ix;

/* Unsigned, 1-byte integer. */
typedef uint8_t   u1;
/* Unsigned, 2-byte integer. */
typedef uint16_t  u2;
/* Unsigned, 4-byte integer. */
typedef uint32_t  u4;
/* Unsigned, 8-byte integer. */
typedef uint64_t  u8;
/* Unsigned, pointer-size integer. */
typedef uintptr_t ux;

/* Floating-point, 4-byte real. */
typedef float  f4;
/* Floating-point, 8-byte real. */
typedef double f8;

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
  ux      len;
} Set;

/* Pair of a string as the key and a corresponding index as a value. */
typedef struct {
  /* String that differentiates the index. */
  String key;
  /* The index value. */
  ux     val;
} MapEntry;

/* Dynamicly allocated array of string and index pairs that uses hashcode for
 * fast element existance checking and access. */
typedef struct {
  /* Pointer to the first entry if it exists. */
  MapEntry* bgn;
  /* Pointer to one after the last entry. */
  MapEntry* end;
  /* Amount of strings. */
  ux        len;
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
char   strAt(String str, ux i);
/* Amount of characters in the string. */
ux     strLen(String str);
/* Whether the given strings equal. */
bool   strEq(String lhs, String rhs);
/* Hashcode of the given string. */
ux     strHash(String str);
/* Stream out all the characters in the given string to the given stream. */
void   strWrite(String str, FILE* stream);

/* Set with the given initial capacity. */
Set           setOf(ux cap);
/* Release the memory resources used by the given set. */
void          setFree(Set* set);
/* Put the given string to the given set. */
void          setPut(Set* set, String str);
/* Pointer to the string in the given set that is equal to the given string.
 * Returns null if there is no string that is equal. */
String const* setGet(Set set, String str);

/* Map with the given initial capacity. */
Map             mapOf(ux cap);
/* Release the memory resources used by the given map. */
void            mapFree(Map* map);
/* Put the given index with the given string as the key to the given map. */
void            mapPut(Map* map, String key, ux val);
/* Pointer to the entry in the given map that corresponds to the given key.
 * Returns null if there is no entry with the given key. */
MapEntry const* mapGet(Map map, String key);
/* Pointer to the string in the given map that corresponds to the given key.
 * Returns null if there is no entry with the given key. */
String const*   mapGetKey(Map map, String key);
/* Pointer to the index in the given map that corresponds to the given key.
 * Returns null if there is no entry with the given key. */
ux const*       mapGetVal(Map map, String key);

/* Buffer with the given initial capacity. */
Buffer bfrOf(ux cap);
/* Copy of the given buffer. */
Buffer bfrOfCopy(Buffer bfr);
/* Release the memory resources used by the given buffer. */
void   bfrFree(Buffer* bfr);
/* Amount of characters in the given buffer. */
ux     bfrLen(Buffer bfr);
/* Amount of allocated characters in the given buffer. */
ux     bfrCap(Buffer bfr);
/* Character at the given index in the given buffer. */
char   bfrAt(Buffer bfr, ux i);
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
