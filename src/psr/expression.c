// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "psr/api.h"
#include "psr/mod.h"
#include "utl/api.h"

#include <stddef.h>

size_t countExpressionNodes(Expression counted) {
  return counted.after - counted.first;
}

String getExpressionSection(Expression gotten) {
  return gotten.after[-1].section;
}

Expression createExpression(size_t initialCapacity) {
  Expression created = {.first = NULL, .after = NULL, .bound = NULL};
  reserveArray(&created, initialCapacity, ExpressionNode);
  return created;
}

void disposeExpression(Expression* disposed) {
  disposed->first = allocateArray(disposed->first, 0, ExpressionNode);
  disposed->after = disposed->first;
  disposed->bound = disposed->first;
}

void pushExpressionNode(Expression* target, ExpressionNode pushed) {
  reserveArray(target, 1, ExpressionNode);
  *target->after++ = pushed;
}
