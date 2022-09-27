// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdbool.h>
#include <stdint.h>

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

/* String of the null-terminated array of characters. */
String stringOf(char const* terminated);
/* Character at the given index in the given string. */
char   stringAt(String str, ux i);
/* Amount of characters in the string. */
ux     stringLength(String str);
/* Whether the given strings equal. */
bool   stringEqual(String lhs, String rhs);
