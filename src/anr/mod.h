// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#pragma once

#include "anr/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Instance of meta type. */
extern Type const META_TYPE_INSTANCE;
/* Instance of void type. */
extern Type const VOID_TYPE_INSTANCE;
/* Instance of bool type. */
extern Type const BOOL_TYPE_INSTANCE;
/* Instance of byte type. */
extern Type const BYTE_TYPE_INSTANCE;
/* Instance of int type. */
extern Type const INT_TYPE_INSTANCE;
/* Instance of uxs type. */
extern Type const UXS_TYPE_INSTANCE;
/* Instance of float type. */
extern Type const FLOAT_TYPE_INSTANCE;
/* Instance of double type. */
extern Type const DOUBLE_TYPE_INSTANCE;

/* Whether the given types are equal. */
bool        compareTypeEquality(Type left, Type right);
/* Whether the given type has a default value. */
bool        checkDefaultability(Type checked);
/* Whether the given source type can be converted to given destination type. */
bool        checkConvertability(Type source, Type destination);
/* Whether the a value of the given source type can be fit inside a value
 * of the given destination type. */
bool        checkArithmeticConvertability(Type source, Type destination);
/* Whether the given type is a valid arithmetic type. */
bool        checkArithmeticity(Type checked);
/* Whether the given type is an integer. */
bool        checkIntegerness(Type checked);
/* Whether the given integer type is signed. */
bool        checkSignedness(Type checked);
/* Type that can hold the result of an arithmetic operation between the
 * given types. */
Type        findCombination(Type left, Type right);
/* Name of the given type. */
char const* nameType(Type named);
/* Default value of the given type. */
Value       defaultValue(Type defaulted);
/* Maximum value of the given integer type. */
uint64_t    getMaximumValue(Type gotten);
/* Value of the given destination type that is the converted version of the
 * given value that has the given source type. */
Value       convertValue(Type source, Type destination, Value converted);
/* Value of the given destination type that is the converted version of
 * the given value that has the given source type. */
Value       convertArithmetic(Type source, Type destination, Value converted);
/* Value of the given signed integer as the given signed integer type. */
Value       convertSignedArithmetic(Type destination, int64_t converted);
/* Value of the given unsigned integer as the given unsigned integer
 * type. */
Value       convertUnsignedArithmetic(Type destination, uint64_t converted);

/* Empty evaluation with the given initial capacity. */
Evaluation createEvaluation(size_t initialCapacity);
/* Release the resources used by the given evaluation. */
void       disposeEvaluation(Evaluation* disposed);
/* Add the given node to the end of the given evaluation. */
void       pushNode(Evaluation* target, EvaluationNode pushed);

/* Add the given symbol to the end of the given table. */
void pushSymbol(Table* target, Symbol pushed);
/* Remove the last symbol from the given table. */
void popSymbol(Table* target);

/* Flag that explores the situation of a number. */
typedef enum {
  /* Number has an acceptable exponent and the significand is valid. */
  NUMBER_NORMAL,
  /* Exponent is too big, regardless of the significand take the number as
   * infinite. */
  NUMBER_INFINITE,
  /* Exponent is too small, regardless of the significand take the number as
   * zero. */
  NUMBER_ZERO,
  /* Number has too many digits. */
  NUMBER_TOO_PRECISE
} NumberFlag;

/* Dynamicly allocated, infinite precision, signed real. */
typedef struct {
  /* Significand that is an infinite precision integer. */
  Buffer     significand;
  /* Signed exponent where the base is 2. Meaning the real value of the number
   * is `significand * 2^exponent`. */
  int        exponent;
  /* Flag. */
  NumberFlag flag;
} Number;

/* Result of converting a number to an arithmetic. */
typedef enum {
  /* Conversion was successful. */
  NUMBER_CONVERSION_SUCCESS,
  /* The converted number was not an integer, but the destination type was. */
  NUMBER_CONVERSION_NOT_INTEGER,
  /* The converted number was negative, but the destination type was an unsigned
     integer. */
  NUMBER_CONVERSION_NOT_UNSIGNED,
  /* The converted number is cannot be fitted inside the destination integer
     type. */
  NUMBER_CONVERSION_OUT_OUF_BOUNDS
} NumberConversionResult;

/* Parse the given decimal string into a number. */
Number                 parseDecimal(String parsedDecimal);
/* Release the memory used by the given number. */
void                   disposeNumber(Number* disposed);
/* Convert the given number with the given sign into the given arithmetic value
 * of the given destination type. Returns the conversion result. */
NumberConversionResult convertNumberToArithmetic(
  Type destination, Value* target, Number converted, bool negativeSign);

/* Analyze the given parse into the given table by reporting to the given
 * source. */
void analyzeParse(Table* target, Source* reported, Parse analyzed);
