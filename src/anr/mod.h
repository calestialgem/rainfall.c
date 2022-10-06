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

/* Result of type comparison. */
typedef enum {
  /* Types are the same. */
  TYPE_CMP_SAME,
  /* Types cannot be converted to each other. */
  TYPE_CMP_DISTINCT,
  /* Left type can be converted to the right type. */
  TYPE_CMP_TO,
  /* Left type can be converted from the right type. */
  TYPE_CMP_FROM
} TypeComparison;

/* Whether the given types are equal. */
bool        typeEq(Type lhs, Type rhs);
/*  */
/* Whether the given type is a built-in signed integer. */
bool        typeSigned(Type type);
/* Whether the given type is a built-in unsigned integer. */
bool        typeUnsigned(Type type);
/* Whether the given type is a built-in floating-point real. */
bool        typeFloat(Type type);
/* Whether the given type is a built-in signed integer, unsigned integer or
 * floating-point real. */
bool        typeScalar(Type type);
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
/* Root node of the given evaluation. */
EvaluationNode evlRoot(Evaluation evl);
/* Type of the given evaluation. */
Type           evlType(Evaluation evl);
/* Value of the given evaluation. */
Value          evlVal(Evaluation evl);
/* Whether the value of the given evaluation is known at compile-time. */
bool           evlHas(Evaluation evl);

/* Whether the given symbol is user-defined. */
bool symUsr(Symbol sym);

/* Amount of symbols in the given table. */
ux               tblLen(Table tbl);
/* Symbol at the given index in the given table. */
Symbol           tblAt(Table tbl, ux i);
/* Add the given symbol to the end of the given table. */
void             tblAdd(Table* tbl, Symbol sym);
/* Remove the last added symbol from the given table. */
void             tblPop(Table* tbl);
/* Add the given operation to the end of the given table. */
void             tblOpnAdd(Table* tbl, Operation opn);
/* Pointer to the operation with the given operator from the given table.
 * Returns null if no operation is registered for the operator. */
Operation const* tblOpnGet(Table tbl, Operator op);

/* Zero number. */
Number numOfZero();
/* Release the memory resources used by the given number. */
void   numFree(Number* num);
/* Parse the given decimal string into the given number. Returns true signaling
 * error, when the exponent has too many digits. */
bool   numSetDec(Number* num, String str);
/* Comparison of the given number with the given value. Returns positive, zero,
 * or negative depending on whether the number is greater than, equals to, or
 * less than the given value, respectively. */
i8     numCmp(Number num, u8 val);
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
void analyze(Table* tbl, Outcome* otc, Parse prs);
