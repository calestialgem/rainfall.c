// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "analyzer/api.h"
#include "parser/api.h"
#include "utility/api.h"

#include <stddef.h>
#include <stdlib.h>

Evaluation createEvaluation(size_t initialCapacity) {
  Evaluation created = {.first = NULL, .after = NULL, .bound = NULL};
  reserveArray(&created, initialCapacity, EvaluationNode);
  return created;
}

void disposeEvaluation(Evaluation* disposed) {
  disposed->first = allocateArray(disposed->first, 0, EvaluationNode);
  disposed->after = disposed->first;
  disposed->bound = disposed->first;
}

void pushEvaluationNode(Evaluation* target, EvaluationNode pushed) {
  reserveArray(target, 1, EvaluationNode);
  *target->after++ = pushed;
}
