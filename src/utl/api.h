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

/* Immutable view of a range of characters. */
typedef struct {
  /* Pointer to the first character if it exists. */
  char const* bgn;
  /* Pointer to one after the last character. */
  char const* end;
} String;

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
/* Stream out all the characters in the given string to the given stream. */
void   strWrite(String str, FILE* stream);

/* Buffer with the given initial capacity. */
Buffer bfrOf(ux cap);
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
