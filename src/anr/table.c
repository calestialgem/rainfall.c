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
static void symReserve(Table* const tbl, ux const amount) {
  ux const cap   = tbl->sym.all - tbl->sym.bgn;
  ux const len   = tblLen(*tbl);
  ux const space = cap - len;
  if (space >= amount) return;

  ux const      growth    = amount - space;
  ux const      minGrowth = cap / 2;
  ux const      newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  Symbol* const mem       = realloc(tbl->sym.bgn, newCap * sizeof(Symbol));
  dbgExpect(mem, "Could not reallocate!");

  tbl->sym.bgn = mem;
  tbl->sym.end = mem + len;
  tbl->sym.all = mem + newCap;
}

/* Make sure the given amount of operation space exists at the end of the given
 * table. When necessary, grows by at least half of the current capacity. */
static void opnReserve(Table* const tbl, ux const amount) {
  ux const cap   = tbl->opn.all - tbl->opn.bgn;
  ux const len   = tbl->opn.end - tbl->opn.bgn;
  ux const space = cap - len;
  if (space >= amount) return;

  ux const         growth    = amount - space;
  ux const         minGrowth = cap / 2;
  ux const         newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  Operation* const mem = realloc(tbl->opn.bgn, newCap * sizeof(Operation));
  dbgExpect(mem, "Could not reallocate!");

  tbl->opn.bgn = mem;
  tbl->opn.end = mem + len;
  tbl->opn.all = mem + newCap;
}

/* Make sure the given amount of conversion space exists at the end of the given
 * table. When necessary, grows by at least half of the current capacity. */
static void cnvReserve(Table* const tbl, ux const amount) {
  ux const cap   = tbl->cnv.all - tbl->cnv.bgn;
  ux const len   = tbl->cnv.end - tbl->cnv.bgn;
  ux const space = cap - len;
  if (space >= amount) return;

  ux const growth    = amount - space;
  ux const minGrowth = cap / 2;
  ux const newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  TypeConversion* const mem =
    realloc(tbl->cnv.bgn, newCap * sizeof(TypeConversion));
  dbgExpect(mem, "Could not reallocate!");

  tbl->cnv.bgn = mem;
  tbl->cnv.end = mem + len;
  tbl->cnv.all = mem + newCap;
}

Table tblOf(Outcome* const otc, Parse const prs) {
  Table res = {0};
  analyze(&res, otc, prs);
  return res;
}

void tblFree(Table* const tbl) {
  for (Symbol* i = tbl->sym.bgn; i < tbl->sym.end; i++) evlFree(&i->evl);
  free(tbl->sym.bgn);
  tbl->sym.bgn = NULL;
  tbl->sym.end = NULL;
  tbl->sym.all = NULL;
}

ux tblLen(Table const tbl) { return tbl.sym.end - tbl.sym.bgn; }

Symbol tblAt(Table const tbl, ux const i) { return tbl.sym.bgn[i]; }

void tblWrite(Table const tbl, FILE* const stream) {
  for (Symbol const* i = tbl.sym.bgn; i < tbl.sym.end; i++) {
    if (!symUsr(*i)) continue;
    strWrite(i->name, stream);
    fprintf(stream, ": ");
    typeWrite(evlType(i->evl), stream);
    if (evlHas(i->evl)) {
      fprintf(stream, " = ");
      valWrite(evlType(i->evl), evlVal(i->evl), stream);
    }
    fprintf(stream, "\n");
    evlTree(i->evl, stream);
    fprintf(stream, "\n");
  }
}

void tblAdd(Table* const tbl, Symbol const sym) {
  symReserve(tbl, 1);
  *tbl->sym.end++ = sym;
}

void tblPop(Table* const tbl) { tbl->sym.end--; }

void tblOpnAdd(Table* const tbl, Operation const opn) {
  opnReserve(tbl, 1);
  *tbl->opn.end++ = opn;
}

void tblCnvAdd(Table* const tbl, TypeConversion const cnv) {
  cnvReserve(tbl, 1);
  *tbl->cnv.end++ = cnv;
}

bool tblCnv(Table const tbl, Type const src, Type const des) {
  if (typeEq(src, des)) return true;
  for (TypeConversion const* i = tbl.cnv.bgn; i < tbl.cnv.end; i++)
    if (typeEq(i->from, src) && typeEq(i->to, des)) return true;
  return false;
}
