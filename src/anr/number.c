// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/mod.h"
#include "dbg/api.h"
#include "utl/api.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Type that converts an integer with fixed amount of bytes to an array of
 * bytes. */
typedef union {
  uint64_t u8;
  char     data[sizeof(uint64_t)];
} Converter;

iptr const BITS = CHAR_BIT;
int const  MAX  = UINT8_MAX + 1;

/* Bit of the given value at the given index. */
static int bitGet(uint64_t const val, iptr const i) { return val >> i & 1; }

/* Number that is the modified version of the given value such that the bit at
 * the given index equlas to the given bit. */
static uint64_t bitSet(uint64_t const val, iptr const i, int const bit) {
  return val | ((uint64_t)bit << i);
}

/* Bit of the given number at the given index. */
static int numBitGet(Number const num, iptr const i) {
  return bitGet((char)bfrAt(num.sig, i / BITS), i % BITS);
}

/* Index of the most significant bit of the given number. */
static iptr numBitMost(Number const num) {
  iptr bit = bfrLen(num.sig) * BITS - 1;
  while (bit > 1 && !numBitGet(num, bit)) bit--;
  return bit;
}

/* Copy of the given number. */
static Number numOfCopy(Number const num) {
  return (Number){.sig = bfrOfCopy(num.sig), .exp = num.exp};
}

/* Add the given value to the given number. */
static void numAdd(Number* const num, int const val) {
  int rem = val;
  for (char* i = num->sig.bgn; i < num->sig.end; i++) {
    rem += *i;
    if (rem < MAX) {
      *i = rem;
      return;
    }
    int const byte = rem % MAX;
    *i             = byte;
    rem -= byte;
    rem /= MAX;
  }
  if (rem) bfrPut(&num->sig, rem);
}

/* Multiply the given number with the given value. */
static void numMul(Number* const num, int const val) {
  int rem = 0;
  for (char* i = num->sig.bgn; i < num->sig.end; i++) {
    rem += *i * val;
    int const byte = rem % MAX;
    *i             = byte;
    rem -= byte;
    rem /= MAX;
  }
  if (rem) bfrPut(&num->sig, rem);
}

/* Divide the given number as integer to the given value. */
static void numDiv(Number* const num, int const val) {
  int rem = 0;
  for (char* i = num->sig.end - 1; i >= num->sig.bgn; i--) {
    rem *= MAX;
    rem += *i;
    *i = rem / val;
    rem %= val;
  }
}

/* Reminder after integer division of the given number with the given value. */
static char numRem(Number const num, int const val) {
  int res = 0;
  for (char const* i = num.sig.end - 1; i >= num.sig.bgn; i--) {
    res *= MAX;
    res += *i;
    res %= val;
  }
  return res;
}

/* Remove the trailing zeros from the given number and add those to the
 * exponent. */
static void numTrim(Number* const num, int const base) {
  while (!numRem(*num, base)) {
    numDiv(num, base);
    num->exp++;
  }
}

/* Change the base of the given number from the given current base to the given
 * target base. */
static void numBase(Number* const num, int const base, int const target) {
  numTrim(num, base);
  if (num->exp >= 0) {
    while (num->exp) {
      numMul(num, base);
      num->exp--;
    }
    numTrim(num, target);
    return;
  }
  int scaledUp = 0;
  while (num->exp) {
    numMul(num, target);
    scaledUp++;
    numTrim(num, base);
  }
  numTrim(num, base);
  num->exp -= scaledUp;
}

/* Decimal integer from the given string. */
static bool decOf(int* const res, String str) {
  int const BASE = 10;
  int const MAX  = INT_MAX / BASE;

  // Consume the sign character.
  bool negative = strAt(str, 0) == '-';
  if (negative || strAt(str, 0) == '+') str.bgn++;

  *res = 0;
  for (char const* i = str.bgn; i < str.end; i++) {
    if (*i == '_') continue;
    if (*res > MAX || (*res == MAX && *i != '0')) return true;
    *res *= BASE;
    *res += *i - '0';
  }

  if (negative) *res *= -1;
  return false;
}

Number numOfZero() {
  Number res = {.sig = bfrOf(1)};
  bfrPut(&res.sig, 0);
  return res;
}

void numFree(Number* const num) { bfrFree(&num->sig); }

bool numSetDec(Number* const num, String const str) {
  int const BASE  = 10;
  num->exp        = 0;
  char const* i   = str.bgn;
  bool        dot = false;

  for (; i < str.end; i++) {
    switch (*i) {
    case '.': dot = true;
    case '_': continue;
    case 'e':
    case 'E':
      int exp = 0;
      // Skip 'e' or 'E' before parsing the exponent.
      if (decOf(&exp, (String){.bgn = i + 1, .end = str.end})) return true;
      num->exp += exp;
      goto success;
    }
    if (dot) num->exp--;
    numMul(num, BASE);
    numAdd(num, *i - '0');
  }
success:
  numBase(num, BASE, 2);
  return false;
}

int numCmp(Number const num, uint64_t const val) {
  Converter const con    = {.u8 = val};
  iptr            valLen = sizeof(uint64_t);
  while (valLen > 1 && !con.data[valLen - 1]) valLen--;
  iptr const len = bfrLen(num.sig);

  if (len != valLen) return len - valLen;

  for (char const* i = num.sig.end - 1; i >= num.sig.bgn; i--) {
    char const byte = con.data[i - num.sig.bgn];
    if (*i != byte) return *i - byte;
  }

  return 0;
}

bool numIsInt(Number const num) { return num.exp >= 0; }

uint64_t numAsInt(Number const num) {
  Converter con = {0};
  for (char const* i = num.sig.bgn; i < num.sig.end; i++)
    con.data[i - num.sig.bgn] = *i;
  return con.u8;
}

int const FLOAT_EXPONENT  = 8;
int const DOUBLE_EXPONENT = 11;

bool isFloat(Number const num, int const exponent) {
  return num.exp + numBitMost(num) <= 1 << (exponent - 1);
}

bool numIsFloat(Number const num) { return isFloat(num, FLOAT_EXPONENT); }

bool numIsDouble(Number const num) { return isFloat(num, DOUBLE_EXPONENT); }

#define asFloat(type, exponentArgument, integer)                              \
  union {                                                                     \
    type    val;                                                              \
    integer i;                                                                \
  } con = {0};                                                                \
                                                                              \
  iptr const most     = numBitMost(num);                                      \
  int const  exponent = exponentArgument;                                     \
  int const  mantissa = sizeof(integer) * CHAR_BIT - 1 - exponent;            \
                                                                              \
  int i = 0;                                                                  \
  for (; i < mantissa && i < most; i++)                                       \
    con.i = bitSet(con.i, mantissa - 1 - i, numBitGet(num, most - 1 - i));    \
  int exp = num.exp;                                                          \
  if (                                                                        \
    i < most && numBitGet(num, most - 1 - i) &&                               \
    (numBitGet(num, most - 1 - i - 1) || numBitGet(num, most - 1 - i + 1))) { \
    con.i++;                                                                  \
    if (con.i >= (integer)1 << mantissa) {                                    \
      con.i >> 1;                                                             \
      exp++;                                                                  \
    }                                                                         \
  }                                                                           \
                                                                              \
  int const bias = (1 << (exponent - 1)) - 1;                                 \
  int const mask = (1 << exponent) - 1;                                       \
  con.i |= ((exp + bias + most) & mask) << mantissa;                          \
                                                                              \
  return con.val

float numAsFloat(Number const num) { asFloat(float, FLOAT_EXPONENT, uint32_t); }

double numAsDouble(Number const num) {
  asFloat(double, DOUBLE_EXPONENT, uint64_t);
}

void numWrite(
  Number const num, int const base, char const exp, FILE* const stream) {
  Buffer digits = bfrOf((bfrLen(num.sig) + 1) * 5 / 2);
  Number rem    = numOfCopy(num);
  while (numCmp(rem, 0)) {
    bfrPut(&digits, (char)('0' + numRem(rem, base)));
    numDiv(&rem, base);
  }
  for (char const* i = digits.end - 1; i >= digits.bgn; i--) fputc(*i, stream);
  bfrFree(&digits);
  if (num.exp) fprintf(stream, "%c%i", exp, num.exp);
}
