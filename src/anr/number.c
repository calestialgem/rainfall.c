// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/mod.h"
#include "utl/api.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FIXED_BYTES 8

/* Type that converts a number with fixed amount of bytes to an array of
 * characters. */
typedef union {
  u8 u8;
  f8 f8;
  u1 data[FIXED_BYTES];
} Converter;

/* Bit of the given value at the given index. */
static u1 bitGet(u8 const val, ux const i) { return val >> i & 1U; }

/* Number that is the modified version of the given value such that the bit at
 * the given index equlas to the given bit. */
static u8 bitSet(u8 const val, ux const i, u8 const bit) {
  return val | (bit << i);
}

/* Bit of the given number at the given index. */
static u8 numBitGet(Number const num, ux const i) {
  u1 const BITS = CHAR_BIT;
  return bitGet((u1)bfrAt(num.sig, i / BITS), i % BITS);
}

/* Index of the most significant bit of the given number. */
static ux numBitMost(Number const num) {
  u1 const BITS = CHAR_BIT;
  ux       bit  = bfrLen(num.sig) * BITS - 1;
  while (bit > 1 && !numBitGet(num, bit)) bit--;
  return bit;
}

Number numOfZero() {
  Number res = {.sig = bfrOf(1)};
  bfrPut(&res.sig, 0);
  return res;
}

Number numOfCopy(Number const num) {
  return (Number){.sig = bfrOfCopy(num.sig), .exp = num.exp};
}

void numFree(Number* const num) { bfrFree(&num->sig); }

void numAdd(Number* const num, u1 const val) {
  u8 const MAX = UINT8_MAX + 1;
  u8       rem = val;
  for (char* i = num->sig.bgn; i < num->sig.end; i++) {
    rem += (u1)*i;
    if (rem < MAX) {
      *i = (char)rem;
      return;
    }
    u8 const byte = rem % MAX;
    *i            = (char)byte;
    rem -= byte;
    rem /= MAX;
  }
  if (rem) bfrPut(&num->sig, (char)rem);
}

void numMul(Number* const num, u1 const val) {
  u8 const MAX = UINT8_MAX + 1;
  u8       rem = 0;
  for (char* i = num->sig.bgn; i < num->sig.end; i++) {
    rem += (u8)(u1)*i * val;
    u8 const byte = rem % MAX;
    *i            = (char)byte;
    rem -= byte;
    rem /= MAX;
  }
  if (rem) bfrPut(&num->sig, (char)rem);
}

void numDiv(Number* const num, u1 const val) {
  u8 const MAX = UINT8_MAX + 1;
  u8       rem = 0;
  for (char* i = num->sig.end - 1; i >= num->sig.bgn; i--) {
    rem *= MAX;
    rem += (u8)(u1)*i;
    *i = (char)(rem / val);
    rem %= val;
  }
}

u1 numRem(Number const num, u1 const val) {
  u8 const MAX = UINT8_MAX + 1;
  u8       res = 0;
  for (char const* i = num.sig.end - 1; i >= num.sig.bgn; i--) {
    res *= MAX;
    res += (u1)*i;
    res %= val;
  }
  return res;
}

i8 numCmp(Number const num, u8 const val) {
  Converter const con    = {.u8 = val};
  ux              valLen = sizeof(u8);
  while (valLen > 1 && !con.data[valLen - 1]) valLen--;
  ux const len = bfrLen(num.sig);

  if (len != valLen) return (i8)len - (i8)valLen;

  for (char const* i = num.sig.end - 1; i >= num.sig.bgn; i--) {
    u1 const byte = con.data[i - num.sig.bgn];
    if ((u1)*i != byte) return *i - (i1)byte;
  }

  return 0;
}

void numTrim(Number* const num, u1 const base) {
  while (!numRem(*num, base)) {
    numDiv(num, base);
    num->exp++;
  }
}

void numScale(Number* const num, i8 const exp) { num->exp += exp; }

void numBase(Number* const num, u1 const base, u1 const target) {
  numTrim(num, base);
  if (num->exp >= 0) {
    while (num->exp) {
      numMul(num, base);
      num->exp--;
    }
    numTrim(num, target);
    return;
  }
  ux scaledUp = 0;
  while (num->exp) {
    numMul(num, target);
    scaledUp++;
    numTrim(num, base);
  }
  numTrim(num, base);
  num->exp -= (ix)scaledUp;
}

bool numInt(Number const num) { return num.exp >= 0; }

u8 numU8(Number const num) {
  Converter con = {0};
  for (char const* i = num.sig.bgn; i < num.sig.end; i++)
    con.data[i - num.sig.bgn] = *i;
  return con.u8;
}

f8 numF8(Number const num) {
  ux const  most = numBitMost(num);
  Converter con  = {0};

  ux const MANTISSA = 52;
  // Skips the most significand bit because it is implied one.
  for (ux i = 0; i < MANTISSA && i < most; i++)
    con.u8 = bitSet(con.u8, MANTISSA - 1 - i, numBitGet(num, most - 1 - i));

  ux const EXPONENT = 11;
  ux const BIAS     = (1U << (EXPONENT - 1)) - 1;
  ux const MASK     = (1U << EXPONENT) - 1;
  con.u8 |= ((num.exp + BIAS + most) & MASK) << MANTISSA;

  return con.f8;
}

void numWrite(
  Number const num, u1 const base, char const exp, FILE* const stream) {
  Buffer digits = bfrOf((bfrLen(num.sig) + 1) * 5 / 2);
  Number rem    = numOfCopy(num);
  while (numCmp(rem, 0)) {
    bfrPut(&digits, (char)('0' + numRem(rem, base)));
    numDiv(&rem, base);
  }
  for (char const* i = digits.end - 1; i >= digits.bgn; i--) fputc(*i, stream);
  bfrFree(&digits);
  if (num.exp) fprintf(stream, "%c%lli", exp, num.exp);
}
