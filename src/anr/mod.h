// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#pragma once

#include "anr/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdio.h>

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

/* Analyze the given parse into the given table by reporting to the given
 * outcome. */
void analyzerAnalyze(Table* tbl, Outcome* otc, Parse prs);
