// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#pragma once

#include "anr/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Flag that explores the situation of a number. */
typedef enum {
  /* Number has an acceptable exponent and the significand is valid. */
  NUM_NORMAL,
  /* Exponent is too big, regardless of the significand take the number as
     infinite. */
  NUM_INFINITE,
  /* Exponent is too small, regardless of the significand take the number as
     zero. */
  NUM_ZERO,
  /* Number has too many digits. */
  NUM_TOO_PRECISE
} NumberFlag;

/* Dynamicly allocated, infinite precision, signed real. */
typedef struct {
  /* Significand that is an integer. */
  Buffer     sig;
  /* Exponent whose base can vary. */
  int        exp;
  /* Flag. */
  NumberFlag flag;
} Number;

/* Whether the given types are equal. */
bool        typeEq(Type lhs, Type rhs);
/* Rank of the given arithmetic type. Returns -1 if the given type is not an
 * arithmetic type. */
iptr        typeRank(Type type);
/* Name of the given type. */
char const* typeName(Type type);
/* Stream out the given type as string to the given stream. */
void        typeWrite(Type type, FILE* stream);

/* Default value of the given type. */
Value valDefault(Type type);
/* Stream out the given value of the given type as string to the given stream.
 */
void  valWrite(Type type, Value val, FILE* stream);

/* Evaluation with the given initial node capacity. */
Evaluation     evlOf(iptr cap);
/* Release the memory resources used by the given evaluation. */
void           evlFree(Evaluation* evl);
/* Amount of nodes in the given evaluation. */
iptr           evlLen(Evaluation evl);
/* Node at the given index in the given evaluation. */
EvaluationNode evlAt(Evaluation evl, iptr i);
/* Add the given node to the end of the given evaluation. */
void           evlAdd(Evaluation* evl, EvaluationNode node);
/* Stream out the given evaluation as string to the given stream. */
void           evlWrite(Evaluation evl, FILE* stream);
/* Stream out the given evaluation tree as string to the given stream. */
void           evlTree(Evaluation evl, FILE* stream);
/* Root node of the given evaluation. */
EvaluationNode evlRoot(Evaluation evl);
/* Type of the given evaluation. */
Type           evlType(Evaluation evl);
/* Value of the given evaluation. */
Value          evlVal(Evaluation evl);
/* Whether the value of the given evaluation is known at compile-time. */
bool           evlHas(Evaluation evl);

/* Amount of symbols in the given table. */
iptr   tblLen(Table tbl);
/* Symbol at the given index in the given table. */
Symbol tblAt(Table tbl, iptr i);
/* Add the given symbol to the end of the given table. */
void   tblAdd(Table* tbl, Symbol sym);
/* Remove the last added symbol from the given table. */
void   tblPop(Table* tbl);

/* Parse the given decimal string into the given number. */
Number   numOfDec(String str);
/* Release the memory resources used by the given number. */
void     numFree(Number* num);
/* Comparison of the given number with the given value. Number should be based
 * in 2. Returns positive, zero, or negative depending on whether the number is
 * greater than, equals to, or less than the given value, respectively. */
int      numCmp(Number num, uint64_t val);
/* Whether the given number is an integer. */
bool     numIsInt(Number num);
/* Value of the given number as integer. Number should be based in 2. */
uint64_t numAsInt(Number num);
/* Value of the given number as a float. Number should be based in 2,
 * because floting-point exponents are based in 2. */
float    numAsFloat(Number num);
/* Value of the given number as a double. Number should be based in 2,
 * because floting-point exponents are based in 2. */
double   numAsDouble(Number num);

/* Analyze the given parse into the given table by reporting to the given
 * outcome. */
void analyze(Table* tbl, Outcome* otc, Parse prs);
