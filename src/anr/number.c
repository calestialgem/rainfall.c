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

/* Amount of bits in a digit of the significand. */
iptr const SIGBITS  = CHAR_BIT;
/* Amount of base of the significand. */
int const  SIGBASE  = UINT8_MAX + 1;
/* Maximum amount of digits of any base to consider too precise when parsing. */
int const  DIGITMAX = 1 << 16;
/* Maximum value of exponent before considering it too big. */
int const  EXPMAX   = 1 << 16;

/* Bit of the given value at the given index. */
static int bitGet(uint64_t const val, iptr const i) { return val >> i & 1; }

/* Number that is the modified version of the given value such that the bit at
 * the given index equlas to the given bit. */
static uint64_t bitSet(uint64_t const val, iptr const i, int const bit) {
  return val | ((uint64_t)bit << i);
}

/* Bit of the given number at the given index. */
static int numBitGet(Number const num, iptr const i) {
  return bitGet((char)bfrAt(num.sig, i / SIGBITS), i % SIGBITS);
}

/* Index of the most significant bit of the given number. */
static iptr numBitMost(Number const num) {
  iptr bit = bfrLen(num.sig) * SIGBITS - 1;
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
    if (rem < SIGBASE) {
      *i = rem;
      return;
    }
    int const byte = rem % SIGBASE;
    *i             = byte;
    rem -= byte;
    rem /= SIGBASE;
  }
  if (rem) bfrPut(&num->sig, rem);
}

/* Multiply the given number with the given value. */
static void numMul(Number* const num, int const val) {
  int rem = 0;
  for (char* i = num->sig.bgn; i < num->sig.end; i++) {
    rem += *i * val;
    int const byte = rem % SIGBASE;
    *i             = byte;
    rem -= byte;
    rem /= SIGBASE;
  }
  if (rem) bfrPut(&num->sig, rem);
}

/* Divide the given number as integer to the given value. */
static void numDiv(Number* const num, int const val) {
  int rem = 0;
  for (char* i = num->sig.end - 1; i >= num->sig.bgn; i--) {
    rem *= SIGBASE;
    rem += *i;
    *i = rem / val;
    rem %= val;
  }
}

/* Reminder after integer division of the given number with the given value. */
static char numRem(Number const num, int const val) {
  int res = 0;
  for (char const* i = num.sig.end - 1; i >= num.sig.bgn; i--) {
    res *= SIGBASE;
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

/* Parse the exponent of a number from the given string. */
static Number decOf(String str) {
  int const BASE = 10;

  // Consume the sign character.
  bool negative = strAt(str, 0) == '-';
  if (negative || strAt(str, 0) == '+') str.bgn++;

  int res = 0;
  for (char const* i = str.bgn; i < str.end; i++) {
    if (*i == '_') continue;
    res *= BASE;
    res += *i - '0';
    if (res > EXPMAX)
      return (Number){.exp = 0, .flag = negative ? NUM_ZERO : NUM_INFINITE};
  }

  if (negative) res *= -1;
  return (Number){.exp = res};
}

/* Zero number. */
static Number numOfZero() {
  Number res = {.sig = bfrOf(1)};
  bfrPut(&res.sig, 0);
  return res;
}

void numFree(Number* const num) { bfrFree(&num->sig); }

Number numOfDec(String const str) {
  int const   BASE = 10;
  char const* i    = str.bgn;
  bool        dot  = false;
  Number      num  = numOfZero();

  if (strLen(str) > DIGITMAX) {
    num.flag = NUM_TOO_PRECISE;
    return num;
  }

  for (; i < str.end; i++) {
    switch (*i) {
    case '.': dot = true;
    case '_': continue;
    case 'e':
    case 'E':
      // If the number is zero, skip any exponent checks.
      if (bfrLen(num.sig) == 1 && !bfrAt(num.sig, 0)) return num;
      // Skip 'e' or 'E' before parsing the exponent.
      Number exp = decOf((String){.bgn = i + 1, .end = str.end});
      num.exp += exp.exp;
      num.flag = exp.flag;
      numFree(&exp);
      if (num.exp < -EXPMAX) num.flag = NUM_ZERO;
      if (num.flag != NUM_NORMAL) return num;
      goto success;
    }
    if (dot) num.exp--;
    numMul(&num, BASE);
    numAdd(&num, *i - '0');
  }
success:
  numBase(&num, BASE, 2);
  return num;
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

bool numIsInt(Number const num) {
  return num.flag == NUM_NORMAL && num.exp >= 0;
}

uint64_t numAsInt(Number const num) {
  Converter con = {0};
  for (char const* i = num.sig.bgn; i < num.sig.end; i++)
    con.data[i - num.sig.bgn] = *i;
  return con.u8;
}

int const FLOAT_EXPONENT  = 8;
int const DOUBLE_EXPONENT = 11;

#define asFloat(type, exponentArgument, integer)                              \
  if (num.flag == NUM_INFINITE) return (type)1 / 0;                           \
  if (num.flag == NUM_ZERO) return 0;                                         \
  union {                                                                     \
    type    val;                                                              \
    integer i;                                                                \
  } con = {0};                                                                \
                                                                              \
  iptr const most     = numBitMost(num);                                      \
  iptr const exponent = exponentArgument;                                     \
  iptr const mantissa = sizeof(integer) * CHAR_BIT - 1 - exponent;            \
                                                                              \
  iptr i = 0;                                                                 \
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
  uint64_t const bias  = (1 << (exponent - 1)) - 1;                           \
  uint64_t const mask  = (1 << exponent) - 1;                                 \
  uint64_t const scale = exp + bias + most;                                   \
  /* TODO: Handle subnormals if they need special attention and make sure the \
   * infinity check below is correct. */                                      \
  if (scale > mask) return (type)1 / 0;                                       \
  con.i |= scale << mantissa;                                                 \
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