// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "lxr/mod.h"
#include "otc/api.h"

#include <stdlib.h>

/* Makes sure the given amount of space exists at the end of the given lex.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Lex* const lex, ux const amount) {
  ux const cap   = lex->all - lex->bgn;
  ux const len   = lexLen(*lex);
  ux const space = cap - len;
  if (space >= amount) return;

  ux const      growth    = amount - space;
  ux const      minGrowth = cap / 2;
  ux const      newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  Lexeme* const mem       = realloc(lex->bgn, newCap * sizeof(Lexeme));
  dbgExpect(mem, "Could not reallocate!");

  lex->bgn = mem;
  lex->end = mem + len;
  lex->all = mem + newCap;
}

Lex lexOf(Outcome* const otc, Source const src) {
  Lex res = {0};
  lexerLex(&res, otc, src);
  return res;
}

void lexFree(Lex* const lex) {
  free(lex->bgn);
  lex->bgn = NULL;
  lex->end = NULL;
  lex->all = NULL;
}

ux lexLen(Lex const lex) { return lex.end - lex.bgn; }

Lexeme lexAt(Lex const lex, ux const i) { return lex.bgn[i]; }

void lexAdd(Lex* lex, Lexeme lxm) {
  reserve(lex, 1);
  *lex->end++ = lxm;
}

void lexPop(Lex* lex) { lex->end--; }
