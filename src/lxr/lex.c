// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "lxr/mod.h"
#include "otc/api.h"

#include <stdlib.h>

/* Make sure the given amount of space exists at the end of the given lex.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Lex* const lex, iptr const amount) {
  iptr const cap   = lex->all - lex->bgn;
  iptr const len   = lexLen(*lex);
  iptr const space = cap - len;
  if (space >= amount) return;

  iptr const    growth    = amount - space;
  iptr const    minGrowth = cap / 2;
  iptr const    newCap    = cap + (growth < minGrowth ? minGrowth : growth);
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

iptr lexLen(Lex const lex) { return lex.end - lex.bgn; }

Lexeme lexAt(Lex const lex, iptr const i) { return lex.bgn[i]; }

Lexeme const* lexBgn(Lex const lex) { return lex.bgn; }

Lexeme const* lexEnd(Lex const lex) { return lex.end; }

void lexAdd(Lex* const lex, Lexeme const lxm) {
  reserve(lex, 1);
  *lex->end++ = lxm;
}

void lexPop(Lex* const lex) { lex->end--; }
