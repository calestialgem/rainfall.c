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
static void reserve(Expression* const exp, iptr const amount) {
  iptr const cap   = exp->all - exp->bgn;
  iptr const len   = expLen(*exp);
  iptr const space = cap - len;
  if (space >= amount) return;

  iptr const growth    = amount - space;
  iptr const minGrowth = cap / 2;
  iptr const newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  ExpressionNode* const mem =
    realloc(exp->bgn, newCap * sizeof(ExpressionNode));
  dbgExpect(mem, "Could not reallocate!");

  exp->bgn = mem;
  exp->end = mem + len;
  exp->all = mem + newCap;
}

/* Stream out the expression node at the given position and its childeren as
 * string to the given stream. Returns the position after all the childeren of
 * the node. */
static ExpressionNode const*
nodeWrite(ExpressionNode const* const i, FILE* const stream) {
  switch (i->op.tag) {
  case OP_NULL:
    if (stream) strWrite(i->val, stream);
    return i - 1;
  case OP_PRE: {
    if (stream) fprintf(stream, "(");
    lxmWrite(i->op.pre.op, stream);
    ExpressionNode const* const op = nodeWrite(i - 1, stream);
    if (stream) fprintf(stream, ")");
    return op;
  }
  case OP_POST: {
    if (stream) fprintf(stream, "(");
    ExpressionNode const* const op = nodeWrite(i - 1, stream);
    lxmWrite(i->op.post.op, stream);
    if (stream) fprintf(stream, ")");
    return op;
  }
  case OP_CIR: {
    lxmWrite(i->op.cir.lop, stream);
    ExpressionNode const* const op = nodeWrite(i - 1, stream);
    lxmWrite(i->op.cir.rop, stream);
    return op;
  }
  case OP_BIN: {
    if (stream) fprintf(stream, "(");
    ExpressionNode const* const rop = nodeWrite(i - 1, NULL);
    ExpressionNode const* const lop = nodeWrite(rop, stream);
    lxmWrite(i->op.bin.op, stream);
    nodeWrite(i - 1, stream);
    if (stream) fprintf(stream, ")");
    return lop;
  }
  case OP_VAR: {
    if (i->ary == 1) {
      ExpressionNode const* const st = nodeWrite(i - 1, stream);
      lxmWrite(i->op.var.lop, stream);
      lxmWrite(i->op.var.rop, stream);
      return st;
    }
    ExpressionNode const* ops[i->ary];
    ops[0] = nodeWrite(i - 1, NULL);
    for (iptr j = 1; j < i->ary; j++) ops[j] = nodeWrite(ops[j - 1], NULL);
    nodeWrite(ops[i->ary - 2], stream);
    lxmWrite(i->op.var.lop, stream);
    for (iptr j = i->ary - 2; j > 0; j--) {
      nodeWrite(ops[j - 1], stream);
      lxmWrite(i->op.var.sep, stream);
    }
    nodeWrite(i - 1, stream);
    lxmWrite(i->op.var.rop, stream);
    return ops[i->ary - 1];
  }
  default: dbgUnexpected("Unknown operator tag!");
  }
}

/* Stream out the expression node at the given position and its childeren as
 * string to the given stream. Returns the position after all the childeren of
 * the node. */
static ExpressionNode const*
nodeTree(ExpressionNode const* i, iptr const depth, FILE* const stream) {
  fprintf(stream, "%20s   ", opName(i->op));
  for (iptr j = 1; j < depth; j++) fprintf(stream, " |  ");
  if (depth > 0) fprintf(stream, " +- ");
  fprintf(stream, "`");
  strWrite(i->val, stream);
  fprintf(stream, "`\n");
  iptr const ary = i->ary;
  i--;
  for (iptr j = 0; j < ary; j++) i = nodeTree(i, depth + 1, stream);
  return i;
}

Expression expOf(iptr const cap) {
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

iptr expLen(Expression const exp) { return exp.end - exp.bgn; }

ExpressionNode expAt(Expression const exp, iptr const i) { return exp.bgn[i]; }

void expAdd(Expression* const exp, ExpressionNode const node) {
  reserve(exp, 1);
  *exp->end++ = node;
}

void expWrite(Expression const exp, FILE* const stream) {
  for (ExpressionNode const* i = exp.end - 1; i >= exp.bgn;)
    i = nodeWrite(i, stream);
}

void expTree(Expression const exp, FILE* const stream) {
  for (ExpressionNode const* i = exp.end - 1; i >= exp.bgn;)
    i = nodeTree(i, 0, stream);
}
