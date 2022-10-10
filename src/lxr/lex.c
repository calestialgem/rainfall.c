// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "lxr/mod.h"

#include <stddef.h>

/* Make sure the given amount of space exists at the end of the given lex.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Lex* l, ptrdiff_t amount) {
  ptrdiff_t capacity = l->bound - l->first;
  ptrdiff_t elements = l->after - l->first;
  ptrdiff_t space    = capacity - elements;
  if (space >= amount) return;

  ptrdiff_t growth    = amount - space;
  ptrdiff_t minGrowth = capacity / 2;
  if (growth < minGrowth) growth = minGrowth;
  capacity += growth;

  l->first = allocateArray(l->first, capacity, Lexeme);
  l->after = l->first + elements;
  l->bound = l->first + capacity;
}

Lex lexOf(Source* s) {
  Lex l = {.first = NULL, .after = NULL, .bound = NULL};
  lex(&l, s);
  return l;
}

void disposeLex(Lex* l) {
  l->first = allocateArray(l->first, 0, Lexeme);
  l->after = l->first;
  l->bound = l->after;
}

void pushLexeme(Lex* l, Lexeme x) {
  reserve(l, 1);
  *l->after++ = x;
}

void popLexeme(Lex* l) { l->after--; }
