// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#pragma once

#include "anr/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdio.h>

/* Dynamicly allocated, infinite precision, signed real. */
typedef struct {
  /* Significand that is an integer. */
  Buffer sig;
  /* Exponent whose base can vary. */
  i8     exp;
} Number;

/* Whether the given types are equal. */
bool        typeEq(Type lhs, Type rhs);
/* Name of the given type. */
char const* typeName(Type type);
/* Stream out the given type as string to the given stream. */
void        typeWrite(Type type, FILE* stream);
/* Stream out the given value of the given type as string to the given stream.
 */
void        valWrite(Type type, Value val, FILE* stream);

/* Evaluation with the given initial node capacity. */
Evaluation     evlOf(ux cap);
/* Release the memory resources used by the given evaluation. */
void           evlFree(Evaluation* evl);
/* Amount of nodes in the given evaluation. */
ux             evlLen(Evaluation evl);
/* Node at the given index in the given evaluation. */
EvaluationNode evlAt(Evaluation evl, ux i);
/* Add the given node to the end of the given evaluation. */
void           evlAdd(Evaluation* evl, EvaluationNode node);
/* Stream out the given evaluation as string to the given stream. */
void           evlWrite(Evaluation evl, FILE* stream);
/* Stream out the given evaluation tree as string to the given stream. */
void           evlTree(Evaluation evl, FILE* stream);
/* Type of the given evaluation. */
Type           evlType(Evaluation evl);
/* Value of the given evaluation. */
Value          evlVal(Evaluation evl);
/* Whether the value of the given evaluation is known at compile-time. */
bool           evlHas(Evaluation evl);

/* Add the given symbol to the end of the given table. */
void tblAdd(Table* tbl, Symbol sym);
/* Remove the last added symbol from the given table. */
void tblPop(Table* tbl);
/* Amount of symbols in the given table. */
ux   tblLen(Table tbl);

/* Zero number. */
Number numOfZero();
/* Copy of the given number. */
Number numOfCopy(Number num);
/* Release the memory resources used by the given number. */
void   numFree(Number* num);
/* Add the given value to the given number. */
void   numAdd(Number* num, u1 val);
/* Multiply the given number with the given value. */
void   numMul(Number* num, u1 val);
/* Divide the given number as integer to the given value. */
void   numDiv(Number* num, u1 val);
/* Reminder after integer division of the given number with the given value. */
u1     numRem(Number num, u1 val);
/* Comparison of the given number with the given value. Returns positive, zero,
 * or negative depending on whether the number is greater than, equals to, or
 * less than the given value, respectively. */
i8     numCmp(Number num, u8 val);
/* Remove the trailing zeros from the given number and add those to the
 * exponent. */
void   numTrim(Number* num, u1 base);
/* Add the given value to the exponent of the given number. */
void   numScale(Number* num, i8 exp);
/* Change the base of the given number from the given current base to the given
 * target base. */
void   numBase(Number* num, u1 base, u1 target);
/* Whether the given number is an integer. */
bool   numInt(Number num);
/* Value of the given number as integer. */
u8     numU8(Number num);
/* Value of the given number as floating-point. Number should be based in 2,
 * because floting-point exponents are based in 2. */
f8     numF8(Number num);
/* Stream out the given number with the given base and given exponent separator
 * as string to the given stream. */
void   numWrite(Number num, u1 base, char exp, FILE* stream);

/* Analyze the given parse into the given table by reporting to the given
 * outcome. */
void analyzerAnalyze(Table* tbl, Outcome* otc, Parse prs);
