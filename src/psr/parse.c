// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "psr/api.h"
#include "psr/mod.h"

#include <stdlib.h>

/* Make sure the given amount of space exists at the end of the given parse.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Parse* const prs, ux const amount) {
  ux const cap   = prs->all - prs->bgn;
  ux const len   = prsLen(*prs);
  ux const space = cap - len;
  if (space >= amount) return;

  ux const         growth    = amount - space;
  ux const         minGrowth = cap / 2;
  ux const         newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  Statement* const mem       = realloc(prs->bgn, newCap * sizeof(Statement));
  dbgExpect(mem, "Could not reallocate!");

  prs->bgn = mem;
  prs->end = mem + len;
  prs->all = mem + newCap;
}

Parse prsOf(Outcome* const otc, Lex const lex) {
  Parse res = {0};
  parserParse(&res, otc, lex);
  return res;
}

void prsFree(Parse* const prs) {
  free(prs->bgn);
  prs->bgn = NULL;
  prs->end = NULL;
  prs->all = NULL;
}

ux prsLen(Parse const prs) { return prs.end - prs.bgn; }

Statement prsAt(Parse const prs, ux const i) { return prs.bgn[i]; }

void prsAdd(Parse* const prs, Statement const stt) {
  reserve(prs, 1);
  *prs->end++ = stt;
}
