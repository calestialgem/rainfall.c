// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "psr/api.h"
#include "psr/mod.h"
#include "utl/api.h"

#include <stdio.h>
#include <stdlib.h>
#include <vcruntime.h>

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

/* Stream out the expression node at the given position and its childeren as
 * string to the given stream. Returns the position after all the childeren of
 * the node. */
static ExpressionNode const*
nodeWrite(ExpressionNode const* const i, FILE* const stream) {
  switch (i->op.tag) {
  case OP_NULL: strWrite(i->val, stream); return i - 1;
  case OP_PRE: lxmWrite(i->op.pre.op, stream); return nodeWrite(i - 1, stream);
  case OP_POST: {
    ExpressionNode const* const op = nodeWrite(i - 1, stream);
    lxmWrite(i->op.post.op, stream);
    return op;
  }
  case OP_CIR: {
    lxmWrite(i->op.cir.lop, stream);
    ExpressionNode const* const op = nodeWrite(i - 1, stream);
    lxmWrite(i->op.cir.rop, stream);
    return op;
  }
  case OP_BIN: {
    ExpressionNode const* const rop = nodeWrite(i - 1, NULL);
    ExpressionNode const* const lop = nodeWrite(rop, stream);
    lxmWrite(i->op.bin.op, stream);
    nodeWrite(i - 1, stream);
    return lop;
  }
  case OP_VAR: {
    ExpressionNode const* ops[i->ary];
    ops[0] = nodeWrite(i - 1, NULL);
    for (ux j = 1; j < i->ary; j++) ops[j] = nodeWrite(ops[j - 1], NULL);
    nodeWrite(ops[i->ary - 1], stream);
    lxmWrite(i->op.var.lop, stream);
    for (ux j = i->ary - 1; j >= 0; j--) {
      nodeWrite(ops[j], stream);
      lxmWrite(i->op.var.sep, stream);
    }
    lxmWrite(i->op.var.rop, stream);
    return ops[i->ary - 1];
  }
  default: dbgUnexpected("Unknown operator tag!");
  }
}

void expWrite(Expression const exp, FILE* const stream) {
  for (ExpressionNode const* i = exp.end - 1; i >= exp.bgn;)
    i = nodeWrite(i, stream);
}
