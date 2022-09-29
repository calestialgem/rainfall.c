// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "psr/api.h"
#include "psr/mod.h"
#include "utl/api.h"

#include <stdlib.h>

/* Make sure the given amount of space exists at the end of the given
 * expression. When necessary, grows by at least half of the current capacity.
 */
static void reserve(Expression* const exp, ux const amount) {
  ux const cap   = exp->all - exp->bgn;
  ux const len   = expLen(*exp);
  ux const space = cap - len;
  if (space >= amount) return;

  ux const growth    = amount - space;
  ux const minGrowth = cap / 2;
  ux const newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  ExpressionNode* const mem =
    realloc(exp->bgn, newCap * sizeof(ExpressionNode));
  dbgExpect(mem, "Could not reallocate!");

  exp->bgn = mem;
  exp->end = mem + len;
  exp->all = mem + newCap;
}

Expression expOf(ux const cap) {
  Expression res = {0};
  if (cap) reserve(&res, cap);
  return res;
}

void expFree(Expression* const exp) {
  free(exp->bgn);
  exp->bgn = NULL;
  exp->end = NULL;
  exp->all = NULL;
}

ux expLen(Expression const exp) { return exp.end - exp.bgn; }

ExpressionNode expAt(Expression const exp, ux const i) { return exp.bgn[i]; }

void expAdd(Expression* const exp, ExpressionNode const node) {
  reserve(exp, 1);
  *exp->end++ = node;
}
