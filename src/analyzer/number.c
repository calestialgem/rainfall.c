// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "analyzer/api.h"
#include "analyzer/mod.h"
#include "utility/api.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/* Amount of bits in a digit of the significand portion of the number. */
#define SIGNIFICAND_DIGIT_BITS CHAR_BIT
/* Base of the significand portion of the number. */
#define SIGNIFICAND_BASE       (1 << SIGNIFICAND_DIGIT_BITS)

/* Write the portion of the given value to the given digit. Returns the
 * remaining value after converted to equivalent value for the digit at the next
 * place. */
static char extractDigit(int* extractedReminder) {
  // Extract the portion of the value that belongs to the current digit.
  int digit = *extractedReminder % SIGNIFICAND_BASE;

  // Convert the reminder to an equivalent value for the place of the next
  // digit.
  *extractedReminder = (*extractedReminder - digit) / SIGNIFICAND_BASE;

  return digit;
}

/* Append the given value to the end of the given significand. */
static void
appendToSignificand(Buffer* targetSignificand, int* appendedReminder) {
  // While there is remaining value, add it as a new digit.
  while (*appendedReminder)
    appendCharacter(targetSignificand, extractDigit(appendedReminder));
}

/* Add the given value to the given significand. */
static void addToSignificand(Buffer* targetSignificand, int added) {
  // Start with the value to be added.
  int reminder = added;

  for (char* digit = targetSignificand->first; digit < targetSignificand->after;
       digit++) {
    // Add the value of the digit.
    reminder += (uint8_t)*digit;
    *digit = extractDigit(&reminder);

    // Early return if the reminder is empty.
    if (!reminder) return;
  }
  appendToSignificand(targetSignificand, &reminder);
}

/* Multiply the given significand with the given value. */
static void multiplySignificand(Buffer* multiplied, int multiplier) {
  int reminder = 0;
  for (char* digit = multiplied->first; digit < multiplied->after; digit++) {
    // Add the value of the digit after the multiplication.
    reminder += (uint8_t)*digit * multiplier;
    *digit = extractDigit(&reminder);

    // Cannot early return here like addition, because the remaining digits are
    // yet to be multiplied. In addition, the value is added once; however, the
    // multiplication should be done for every digit. Hence, cannot short
    // circuit out.
  }
  appendToSignificand(multiplied, &reminder);
}

/* Remove the leading zeros from the given significand. */
static void trimLeadingZeros(Buffer* trimmed) {
  // Make sure to leave at least a single zero digit in the significand.
  while (trimmed->after > trimmed->first + 1 && !trimmed->after[-1])
    trimmed->after--;
}

/* Divide the given significand with the given value. */
static void divideSignificand(Buffer* divided, int divider) {
  int reminder = 0;

  for (char* digit = divided->after - 1; digit >= divided->first; digit--) {
    // Convert the reminder to an equivalent value for the place of the previous
    // digit.
    reminder *= SIGNIFICAND_BASE;

    // Add the value of the digit.
    reminder += (uint8_t)*digit;

    // Set the current digit to whole part of the division.
    *digit = reminder / divider;

    // Leave the reminder from the division for the previous digit.
    reminder %= divider;
  }

  // Remove any leading zeros created by the division.
  trimLeadingZeros(divided);
}

/* Modulus of the given significand with the given value. */
static int findSignificandModulus(Buffer modded, int mod) {
  int reminder = 0;

  for (char const* digit = modded.after - 1; digit >= modded.first; digit--) {
    // Convert the reminder to an equivalent value for the place of the previous
    // digit.
    reminder *= SIGNIFICAND_BASE;

    // Add the value of the digit.
    reminder += (uint8_t)*digit;

    // Leave the reminder from the division for the previous digit.
    reminder %= mod;
  }

  return reminder;
}

/* Remove the leading and trailing zeros from the given number by adding them to
 * the exponent, which has the given base. */
static void trimNumber(Number* trimmed, int exponentBase) {
  // Divide with the base and increment exponent until there is no whole integer
  // division possible.
  while (!findSignificandModulus(trimmed->significand, exponentBase)) {
    divideSignificand(&trimmed->significand, exponentBase);
    trimmed->exponent++;
  }
  trimLeadingZeros(&trimmed->significand);
}

/* Bit of the given value at the given index. */
static int getBitAt(uint64_t source, size_t gottenBit) {
  return source >> gottenBit & 1;
}

/* Given value that has the bit in the given index modified to the given value.
 */
static uint64_t setBitAt(uint64_t target, size_t setBit, int setValue) {
  return target | ((uint64_t)setValue << setBit);
}

/* Bit of the given significand at the given index. */
static int getBitOfSignificandAt(Buffer source, size_t gottenBit) {
  // Find the digit the bit is in and get the bit in the digit.
  return getBitAt(
    (uint8_t)source.first[gottenBit / SIGNIFICAND_DIGIT_BITS],
    gottenBit % SIGNIFICAND_DIGIT_BITS);
}

/* Index of the most significant bit of the given significand. */
static size_t findMostSignificantBit(Buffer found) {
  // Start from the highest possible index and return the first index that has a
  // set bit.
  for (size_t highestIndex = countBytes(found) * SIGNIFICAND_DIGIT_BITS - 1;
       highestIndex > 0; highestIndex--)
    if (getBitOfSignificandAt(found, highestIndex)) return highestIndex;
  return 0;
}

/* Maximum amount of bits to round the significand at when rebasing. */
#define REBASE_SIGNIFICAND_MAX_BITS 64

/* Change the base of the given number from the given base to 2. */
static void rebaseNumber(Number* rebased, int currentBase) {
  // If the exponent is positive, just multiply them away.
  if (rebased->exponent >= 0) {
    while (rebased->exponent) {
      multiplySignificand(&rebased->significand, currentBase);
      rebased->exponent--;
    }

    // Trim to the new base and return.
    trimNumber(rebased, 2);
    return;
  }

  // Check whether the exponent can be made positive by dividing the
  // significand.
  while (!findSignificandModulus(rebased->significand, currentBase)) {
    divideSignificand(&rebased->significand, currentBase);
    rebased->exponent++;

    // If the exponent could be raised to zero, trim and return.
    if (rebased->exponent) {
      // Trim to the new base and return.
      trimNumber(rebased, 2);
      return;
    }
  }

  // Exponent is negative, rounding might be necessary depending on the prime
  // numbers that make up the bases. Try to limit the significand to 64 bits.
  size_t leadingBitIndex = findMostSignificantBit(rebased->significand);
  int    scaledUp        = 0;

  // Scale up the number with the new base until exponent is zerod or the
  // scaling up becomes too much.
  while (rebased->exponent) {
    // As the new base is 2, the scaling up just increses the index of the
    // leading bit, which is the limited quantitiy.
    if (scaledUp + leadingBitIndex >= REBASE_SIGNIFICAND_MAX_BITS) break;
    multiplySignificand(&rebased->significand, 2);
    trimNumber(rebased, currentBase);
    scaledUp++;
  }

  // If scaling up did not work, which means the current base has a prime in it
  // that is not present in the new base, round up. Other rounding modes are too
  // fancy to implement here and they do not change the results. The double,
  // which has the biggest precision is only 53 bits long. (64-bit unsigned
  // integer, which has more precision than double, should have a positive
  // exponent anyways, otherwise it would not be an integer.)
  while (rebased->exponent) {
    int reminder = findSignificandModulus(rebased->significand, currentBase);
    addToSignificand(&rebased->significand, currentBase - reminder);
    trimNumber(rebased, currentBase);
  }

  // Trim and remove the scale up.
  trimNumber(rebased, 2);
  rebased->exponent -= scaledUp;
}

/* Base of the parsed exponent integer. */
#define EXPONENT_BASE  10
/* Limiting amount of the exponent where it is considered too big. */
#define EXPONENT_LIMIT (1 << 16)

/* Parse the exponent of a number. */
Number parseExponent(String parsedExponent) {
  // Consume the sign character.
  bool negative = *parsedExponent.first == '-';
  if (negative || *parsedExponent.first == '+') parsedExponent.after++;

  int exponent = 0;

  for (char const* digit = parsedExponent.first; digit < parsedExponent.after;
       digit++) {
    // Skip any undercores in the number.
    if (*digit == '_') continue;

    // Increase the decimal place of the already parse digits and parse the
    // current one.
    exponent *= EXPONENT_BASE;
    exponent += *digit - '0';

    // Check the exponent, if it at the limit return zero or infinite depending
    // on the sign of the exponent.
    if (exponent >= EXPONENT_LIMIT) {
      return (Number){
        .significand = createBuffer(0),
        .exponent    = 0,
        .flag        = negative ? NUMBER_ZERO : NUMBER_INFINITE};
    }
  }

  // If negative, set the sign of the exponent.
  if (negative) exponent = -exponent;
  return (Number){
    .significand = createBuffer(0),
    .exponent    = exponent,
    .flag        = NUMBER_NORMAL};
}

/* Comparison of the given number with the given value. Returns positive, zero,
 * or negative depending on whether the number is greater than, equals to, or
 * less than the given value, respectively. */
int compareNumber(Number comparedNumber, uint64_t comparedValue) {
  int mostSignificant = findMostSignificantBit(comparedNumber.significand) +
                        comparedNumber.exponent;

  // Find the most significand bit of the compared value. Make sure at least a
  // bit is left.
  int mostSignificantValueBit = sizeof(uint64_t) * CHAR_BIT - 1;
  while (mostSignificantValueBit > 1 &&
         !getBitAt(comparedValue, mostSignificantValueBit))
    mostSignificantValueBit--;

  // If the indicies of the most significant bits are not the same, the one with
  // the more bits is bigger.
  if (mostSignificant != mostSignificantValueBit)
    return mostSignificant - mostSignificantValueBit;

  // If they have equal number of bits, compared them one by one from the most
  // significant to least. The bits of the number is all zeros after the
  // exponent scale up is reached.
  for (int bitIndex = mostSignificant; bitIndex >= comparedNumber.exponent;
       bitIndex--) {
    int valueBit  = getBitAt(comparedValue, bitIndex);
    int numberBit = getBitOfSignificandAt(
      comparedNumber.significand, bitIndex - comparedNumber.exponent);
    if (numberBit != valueBit) return numberBit - valueBit;
  }

  // If the value has a set bit in the part that corresponds to the scaled up
  // part of the number, the value is bigger.
  for (int bitIndex = comparedNumber.exponent - 1; bitIndex >= 0; bitIndex--)
    if (getBitAt(comparedValue, bitIndex)) return -1;

  // If all are the same, they are equal.
  return 0;
}

/* Whether the given number is an integer. */
bool checkNumberIntegerness(Number checked) {
  return checked.flag == NUMBER_NORMAL && checked.exponent >= 0;
}

/* Value of the given number as an 64-bit unsigned integer. */
uint64_t convertNumberToInteger(Number converted) {
  union {
    uint64_t asInteger;
    char     asBytes[sizeof(uint64_t)];
  } converter = {.asInteger = 0};

  // Copy the bytes to the integer; then, shift it by the exponent.
  for (size_t index = 0; index < countBytes(converted.significand); index++)
    converter.asBytes[index] = converted.significand.first[index];
  return converter.asInteger << converted.exponent;
}

/* Amount of bits that are reserved for the exponent in floats. */
#define FLOAT_EXPONENT_WIDTH 8
/* Bias on the exponent of floats. */
#define FLOAT_EXPONENT_BIAS  ((1 << (FLOAT_EXPONENT_WIDTH - 1)) - 1)
/* Mask that is as wide as the exponent of floats. */
#define FLOAT_EXPONENT_MASK  ((1 << FLOAT_EXPONENT_WIDTH) - 1)
/* Amount of bits that is reserved for the significand in floats. */
#define FLOAT_MANTISSA_WIDTH \
  (sizeof(float) * CHAR_BIT - FLOAT_EXPONENT_WIDTH - 1)

/* Value of the given number as a float. */
float convertNumberToFloat(Number converted) {
  // Handle special cases.
  if (converted.flag == NUMBER_INFINITE) return (float)1 / 0;
  if (converted.flag == NUMBER_ZERO) return 0;

  union {
    float    asFloat;
    uint32_t asInteger;
  } converter = {.asInteger = 0};

  // Copy the bits upto the end of the mantissa or the most significant bit.
  size_t mostSignificantBit = findMostSignificantBit(converted.significand);
  size_t bitIndex           = 0;
  for (; bitIndex < FLOAT_MANTISSA_WIDTH && bitIndex < mostSignificantBit;
       bitIndex++)
    converter.asInteger = setBitAt(
      converter.asInteger, FLOAT_MANTISSA_WIDTH - 1 - bitIndex,
      getBitOfSignificandAt(
        converted.significand, mostSignificantBit - 1 - bitIndex));

  // Round the value up if necessary.
  int exponent = converted.exponent;
  if (
    bitIndex < mostSignificantBit &&
    getBitOfSignificandAt(
      converted.significand, mostSignificantBit - 1 - bitIndex)) {
    converter.asInteger++;
    // Scale the exponent if the mantissa became too big.
    if (converter.asInteger >= (uint32_t)1 << FLOAT_MANTISSA_WIDTH) {
      converter.asInteger >>= 1;
      exponent++;
    }
  }

  // Find the final exponent, check if it is too big or zero.
  exponent += FLOAT_EXPONENT_BIAS + mostSignificantBit;
  if (exponent > FLOAT_EXPONENT_MASK) return (float)1 / 0;
  expect(exponent, "Number is subnormal!");
  converter.asInteger |= (uint32_t)exponent << FLOAT_MANTISSA_WIDTH;

  return converter.asFloat;
}

/* Amount of bits that are reserved for the exponent in doubles. */
#define DOUBLE_EXPONENT_WIDTH 11
/* Bias on the exponent of doubles. */
#define DOUBLE_EXPONENT_BIAS  ((1 << (DOUBLE_EXPONENT_WIDTH - 1)) - 1)
/* Mask that is as wide as the exponent of doubles. */
#define DOUBLE_EXPONENT_MASK  ((1 << DOUBLE_EXPONENT_WIDTH) - 1)
/* Amount of bits that is reserved for the significand in doubles. */
#define DOUBLE_MANTISSA_WIDTH \
  (sizeof(double) * CHAR_BIT - DOUBLE_EXPONENT_WIDTH - 1)

/* Value of the given number as a double. */
double convertNumberToDouble(Number converted) {
  // Handle special cases.
  if (converted.flag == NUMBER_INFINITE) return (double)1 / 0;
  if (converted.flag == NUMBER_ZERO) return 0;

  union {
    double   asDouble;
    uint64_t asInteger;
  } converter = {.asInteger = 0};

  // Copy the bits upto the end of the mantissa or the most significant bit.
  size_t mostSignificantBit = findMostSignificantBit(converted.significand);
  size_t bitIndex           = 0;
  for (; bitIndex < DOUBLE_MANTISSA_WIDTH && bitIndex < mostSignificantBit;
       bitIndex++)
    converter.asInteger = setBitAt(
      converter.asInteger, DOUBLE_MANTISSA_WIDTH - 1 - bitIndex,
      getBitOfSignificandAt(
        converted.significand, mostSignificantBit - 1 - bitIndex));

  // Round the value up if necessary.
  int exponent = converted.exponent;
  if (
    bitIndex < mostSignificantBit &&
    getBitOfSignificandAt(
      converted.significand, mostSignificantBit - 1 - bitIndex)) {
    converter.asInteger++;
    // Scale the exponent if the mantissa became too big.
    if (converter.asInteger >= (uint64_t)1 << DOUBLE_MANTISSA_WIDTH) {
      converter.asInteger >>= 1;
      exponent++;
    }
  }

  // Find the final exponent, check if it is too big or zero.
  exponent += DOUBLE_EXPONENT_BIAS + mostSignificantBit;
  if (exponent > DOUBLE_EXPONENT_MASK) return (double)1 / 0;
  expect(exponent, "Number is subnormal!");
  converter.asInteger |= (uint64_t)exponent << DOUBLE_MANTISSA_WIDTH;

  return converter.asDouble;
}

/* Maximum amount of digits to parse for a number. */
#define PARSED_DIGIT_LIMIT (1 << 16)
/* Base of the decimal number parser. */
#define DECIMAL_BASE       10

Number parseDecimal(String parsedDecimal) {
  // Check the parsed character limit.
  if (countCharacters(parsedDecimal) > PARSED_DIGIT_LIMIT) {
    return (Number){
      .significand = createBuffer(0),
      .exponent    = 0,
      .flag        = NUMBER_TOO_PRECISE};
  }

  bool afterDot = false;

  // Create a number with zero in it.
  Number decimal = {
    .significand = createBuffer(1), .exponent = 0, .flag = NUMBER_NORMAL};
  appendCharacter(&decimal.significand, 0);

  for (char const* digit = parsedDecimal.first; digit < parsedDecimal.after;
       digit++) {
    switch (*digit) {
    case '.': afterDot = true;
    case '_': continue;
    case 'e':
    case 'E':
      // If the number is zero, skip any exponent checks.
      if (countBytes(decimal.significand) == 1 && !*decimal.significand.first)
        goto success;

      // Skip 'e' or 'E' before parsing the exponent.
      Number exponent =
        parseExponent(createString(digit + 1, parsedDecimal.after));
      decimal.exponent += exponent.exponent;
      decimal.flag = exponent.flag;
      disposeNumber(&exponent);

      // Check the exponent after parsed exponent was added. No need to check
      // the upper bound as the initial exponent is always non-positive and the
      // parsed exponent is always below the maximum limit.
      if (decimal.exponent <= -EXPONENT_LIMIT) decimal.flag = NUMBER_ZERO;
      if (decimal.flag != NUMBER_NORMAL) return decimal;

      goto success;
    default:
      if (afterDot) decimal.exponent--;
      multiplySignificand(&decimal.significand, DECIMAL_BASE);
      addToSignificand(&decimal.significand, *digit - '0');
    }
  }

success:
  rebaseNumber(&decimal, DECIMAL_BASE);
  return decimal;
}

void disposeNumber(Number* disposed) { disposeBuffer(&disposed->significand); }

NumberConversionResult convertNumberToArithmetic(
  Type destination, Value* target, Number converted, bool negativeSign) {
  // Check whether the destination is an integer.
  if (checkIntegerness(destination)) {
    // Check whether the converted number is integer.
    if (!checkNumberIntegerness(converted))
      return NUMBER_CONVERSION_NOT_INTEGER;

    // Check whether the destination is signed.
    if (checkSignedness(destination)) {
      // Compare to one plus the maximum value because in the two's complement
      // representation the positive numbers contain the `0` value and they have
      // one less limit because of that.
      int comparison =
        compareNumber(converted, getMaximumValue(destination) + 1);

      // A valid signed number can be smaller than the maximum, or it can be
      // equal to maximum if its negative. Thus, if it is bigger than the
      // maximum, or it equals to the maximum but it is not negative, it is out
      // of bounds.
      if (comparison == 1 || (comparison == 0 && !negativeSign))
        return NUMBER_CONVERSION_OUT_OUF_BOUNDS;

      *target = convertSignedArithmetic(
        destination,
        (negativeSign ? -1 : 1) * (int64_t)convertNumberToInteger(converted));
    } else {
      // Compare to the maximum value.
      if (compareNumber(converted, getMaximumValue(destination)) == 1)
        return NUMBER_CONVERSION_OUT_OUF_BOUNDS;

      // The sign can only be negative if the value is zero.
      uint64_t value = convertNumberToInteger(converted);
      if (negativeSign && value != 0) return NUMBER_CONVERSION_NOT_UNSIGNED;

      *target = convertUnsignedArithmetic(destination, value);
    }

    return NUMBER_CONVERSION_SUCCESS;
  }

  // If the destination is not an integer, it can only be a float or double.
  if (compareTypeEquality(destination, FLOAT_TYPE_INSTANCE))
    target->asFloat = convertNumberToFloat(converted);
  else target->asDouble = convertNumberToDouble(converted);
  return NUMBER_CONVERSION_SUCCESS;
}
