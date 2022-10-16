// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "dbg/api.h"
#include "psr/api.h"

#include <stddef.h>
#include <stdlib.h>

Evaluation createEvaluation(size_t initialCapacity) {
  Evaluation created = {.first = NULL, .after = NULL, .bound = NULL};
  reserveArray(&created, initialCapacity, EvaluationNode);
  return created;
}

void disposeExpression(Expression* disposed) {
  disposed->first = allocateArray(disposed->first, 0, ExpressionNode);
  disposed->after = disposed->first;
  disposed->bound = disposed->first;
}

void pushNode(Expression* target, ExpressionNode pushed) {
  reserveArray(target, 1, ExpressionNode);
  *target->after++ = pushed;
}
