// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "lxr/mod.h"

#include <stddef.h>

/* Make sure the given amount of space exists at the end of the given lex.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Lex* targetLex, size_t reservedAmount) {
  size_t capacity = targetLex->bound - targetLex->first;
  size_t elements = targetLex->after - targetLex->first;
  size_t space    = capacity - elements;
  if (space >= reservedAmount) return;

  size_t growth    = reservedAmount - space;
  size_t minGrowth = capacity / 2;
  if (growth < minGrowth) growth = minGrowth;
  capacity += growth;

  targetLex->first = allocateArray(targetLex->first, capacity, Lexeme);
  targetLex->after = targetLex->first + elements;
  targetLex->bound = targetLex->first + capacity;
}

Lex lexOf(Source* source) {
  Lex result = {.first = NULL, .after = NULL, .bound = NULL};
  lex(&result, source);
  return result;
}

void disposeLex(Lex* targetLex) {
  targetLex->first = allocateArray(targetLex->first, 0, Lexeme);
  targetLex->after = targetLex->first;
  targetLex->bound = targetLex->after;
}

void pushLexeme(Lex* targetLex, Lexeme pushedLexeme) {
  reserve(targetLex, 1);
  *targetLex->after++ = pushedLexeme;
}

void popLexeme(Lex* targetLex) { targetLex->after--; }
