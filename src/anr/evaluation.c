// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdlib.h>

/* Make sure the given amount of space exists at the end of the given
 * evaluation. When necessary, grows by at least half of the current capacity.
 */
static void reserve(Evaluation* const evl, ux const amount) {
  ux const cap   = evl->all - evl->bgn;
  ux const len   = evlLen(*evl);
  ux const space = cap - len;
  if (space >= amount) return;

  ux const growth    = amount - space;
  ux const minGrowth = cap / 2;
  ux const newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  EvaluationNode* const mem =
    realloc(evl->bgn, newCap * sizeof(EvaluationNode));
  dbgExpect(mem, "Could not reallocate!");

  evl->bgn = mem;
  evl->end = mem + len;
  evl->all = mem + newCap;
}

/* Stream out the evaluation node at the given position and its childeren as
 * string to the given stream. Returns the position after all the childeren of
 * the node. */
static EvaluationNode const*
nodeWrite(EvaluationNode const* const i, FILE* const stream) {
  switch (i->exp.op.tag) {
  case OP_NULL:
    if (stream) {
      strWrite(i->exp.val, stream);
      if (i->has) {
        fprintf(stream, "`");
        valWrite(i->type, i->val, stream);
        fprintf(stream, "`");
      }
    }
    return i - 1;
  case OP_PRE: {
    if (stream) fprintf(stream, "(");
    lxmWrite(i->exp.op.pre.op, stream);
    EvaluationNode const* const op = nodeWrite(i - 1, stream);
    if (stream) {
      fprintf(stream, ")");
      if (i->has) {
        fprintf(stream, "`");
        valWrite(i->type, i->val, stream);
        fprintf(stream, "`");
      }
    }
    return op;
  }
  case OP_POST: {
    if (stream) fprintf(stream, "(");
    EvaluationNode const* const op = nodeWrite(i - 1, stream);
    lxmWrite(i->exp.op.post.op, stream);
    if (stream) {
      fprintf(stream, ")");
      if (i->has) {
        fprintf(stream, "`");
        valWrite(i->type, i->val, stream);
        fprintf(stream, "`");
      }
    }
    return op;
  }
  case OP_CIR: {
    lxmWrite(i->exp.op.cir.lop, stream);
    EvaluationNode const* const op = nodeWrite(i - 1, stream);
    lxmWrite(i->exp.op.cir.rop, stream);
    return op;
  }
  case OP_BIN: {
    if (stream) fprintf(stream, "(");
    EvaluationNode const* const rop = nodeWrite(i - 1, NULL);
    EvaluationNode const* const lop = nodeWrite(rop, stream);
    lxmWrite(i->exp.op.bin.op, stream);
    nodeWrite(i - 1, stream);
    if (stream) {
      fprintf(stream, ")");
      if (i->has) {
        fprintf(stream, "`");
        valWrite(i->type, i->val, stream);
        fprintf(stream, "`");
      }
    }
    return lop;
  }
  case OP_VAR: {
    if (i->exp.ary == 1) {
      EvaluationNode const* const st = nodeWrite(i - 1, stream);
      lxmWrite(i->exp.op.var.lop, stream);
      lxmWrite(i->exp.op.var.rop, stream);
      return st;
    }
    EvaluationNode const* ops[i->exp.ary];
    ops[0] = nodeWrite(i - 1, NULL);
    for (ux j = 1; j < i->exp.ary; j++) ops[j] = nodeWrite(ops[j - 1], NULL);
    nodeWrite(ops[i->exp.ary - 2], stream);
    lxmWrite(i->exp.op.var.lop, stream);
    for (ux j = i->exp.ary - 2; j > 0; j--) {
      nodeWrite(ops[j - 1], stream);
      lxmWrite(i->exp.op.var.sep, stream);
    }
    nodeWrite(i - 1, stream);
    lxmWrite(i->exp.op.var.rop, stream);
    if (stream) {
      if (i->has) {
        fprintf(stream, "`");
        valWrite(i->type, i->val, stream);
        fprintf(stream, "`");
      }
    }
    return ops[i->exp.ary - 1];
  }
  default: dbgUnexpected("Unknown operator tag!");
  }
}

/* Stream out the evaluation node at the given position and its childeren as
 * string to the given stream. Returns the position after all the childeren of
 * the node. */
static EvaluationNode const*
nodeTree(EvaluationNode const* i, ux const depth, FILE* const stream) {
  fprintf(stream, "%20s   ", opName(i->exp.op));
  for (ux j = 1; j < depth; j++) fprintf(stream, " |  ");
  if (depth > 0) fprintf(stream, " +- ");
  fprintf(stream, "`");
  strWrite(i->exp.val, stream);
  if (i->has) {
    fprintf(stream, "` `");
    valWrite(i->type, i->val, stream);
  }
  fprintf(stream, "`\n");
  ux const ary = i->exp.ary;
  i--;
  for (ux j = 0; j < ary; j++) i = nodeTree(i, depth + 1, stream);
  return i;
}

/* Root node of the given evaluation. */
static EvaluationNode root(Evaluation const evl) {
  return evlAt(evl, evlLen(evl) - 1);
}

Evaluation evlOf(ux const cap) {
  Evaluation res = {0};
  if (cap) reserve(&res, cap);
  return res;
}

void evlFree(Evaluation* const evl) {
  free(evl->bgn);
  evl->bgn = NULL;
  evl->end = NULL;
  evl->all = NULL;
}

ux evlLen(Evaluation const evl) { return evl.end - evl.bgn; }

EvaluationNode evlAt(Evaluation const evl, ux const i) { return evl.bgn[i]; }

void evlAdd(Evaluation* const evl, EvaluationNode const node) {
  reserve(evl, 1);
  *evl->end++ = node;
}

void evlWrite(Evaluation const evl, FILE* const stream) {
  for (EvaluationNode const* i = evl.end - 1; i >= evl.bgn;)
    i = nodeWrite(i, stream);
}

void evlTree(Evaluation const evl, FILE* const stream) {
  for (EvaluationNode const* i = evl.end - 1; i >= evl.bgn;)
    i = nodeTree(i, 0, stream);
}

Type evlType(Evaluation const evl) { return root(evl).type; }

Value evlVal(Evaluation const evl) { return root(evl).val; }

bool evlHas(Evaluation const evl) { return root(evl).has; }
