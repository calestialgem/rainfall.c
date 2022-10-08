// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdlib.h>

/* Make sure the given amount of symbol space exists at the end of the given
 * table. When necessary, grows by at least half of the current capacity. */
static void symReserve(Table* const tbl, iptr const amount) {
  iptr const cap   = tbl->all - tbl->bgn;
  iptr const len   = tblLen(*tbl);
  iptr const space = cap - len;
  if (space >= amount) return;

  iptr const    growth    = amount - space;
  iptr const    minGrowth = cap / 2;
  iptr const    newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  Symbol* const mem       = realloc(tbl->bgn, newCap * sizeof(Symbol));
  dbgExpect(mem, "Could not reallocate!");

  tbl->bgn = mem;
  tbl->end = mem + len;
  tbl->all = mem + newCap;
}

Table tblOf(Outcome* const otc, Parse const prs) {
  Table res = {0};
  analyze(&res, otc, prs);
  return res;
}

void tblFree(Table* const tbl) {
  for (Symbol* i = tbl->bgn; i < tbl->end; i++) {
    switch (i->tag) {
    case SYM_BIND: evlFree(&i->bind.evl); break;
    case SYM_VAR: evlFree(&i->var.evl); break;
    default: break;
    }
  }
  free(tbl->bgn);
  tbl->bgn = NULL;
  tbl->end = NULL;
  tbl->all = NULL;
}

iptr tblLen(Table const tbl) { return tbl.end - tbl.bgn; }

Symbol tblAt(Table const tbl, iptr const i) { return tbl.bgn[i]; }

void tblWrite(Table const tbl, FILE* const stream) {
  for (Symbol const* i = tbl.bgn; i < tbl.end; i++) {
    if (!i->usr) continue;
    strWrite(i->name, stream);
    fprintf(stream, ": ");
    typeWrite(i->type, stream);
    if (i->has) {
      fprintf(stream, " = ");
      valWrite(i->type, i->val, stream);
    }
    fprintf(stream, "\n");
    switch (i->tag) {
    case SYM_BIND:
      evlTree(i->bind.evl, stream);
      fprintf(stream, "\n");
      break;
    case SYM_VAR:
      if (evlLen(i->var.evl)) {
        evlTree(i->var.evl, stream);
        fprintf(stream, "\n");
      }
      break;
    default: break;
    }
  }
}

void tblAdd(Table* const tbl, Symbol const sym) {
  symReserve(tbl, 1);
  *tbl->end++ = sym;
}

void tblPop(Table* const tbl) { tbl->end--; }
