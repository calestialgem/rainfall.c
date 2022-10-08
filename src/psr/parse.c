// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "psr/api.h"
#include "psr/mod.h"
#include "utl/api.h"

#include <stdio.h>
#include <stdlib.h>

/* Make sure the given amount of space exists at the end of the given parse.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Parse* const prs, iptr const amount) {
  iptr const cap   = prs->all - prs->bgn;
  iptr const len   = prsLen(*prs);
  iptr const space = cap - len;
  if (space >= amount) return;

  iptr const       growth    = amount - space;
  iptr const       minGrowth = cap / 2;
  iptr const       newCap    = cap + (growth < minGrowth ? minGrowth : growth);
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
  for (Statement* i = prs->bgn; i < prs->end; i++) {
    switch (i->tag) {
    case STT_LET:
      expFree(&i->let.type);
      expFree(&i->let.val);
      break;
    case STT_VAR:
      expFree(&i->var.type);
      expFree(&i->var.val);
      break;
    case STT_ASS: expFree(&i->ass.val); break;
    case STT_CAS: expFree(&i->cas.rhs); break;
    case STT_EXP: expFree(&i->exp.exp); break;
    default: dbgUnexpected("Unknown statement tag!");
    }
  }
  free(prs->bgn);
  prs->bgn = NULL;
  prs->end = NULL;
  prs->all = NULL;
}

iptr prsLen(Parse const prs) { return prs.end - prs.bgn; }

Statement prsAt(Parse const prs, iptr const i) { return prs.bgn[i]; }

void prsWrite(Parse const prs, FILE* const stream) {
  for (Statement const* i = prs.bgn; i < prs.end; i++) {
    switch (i->tag) {
    case STT_LET:
      fprintf(stream, "let ");
      strWrite(i->let.name, stream);
      fprintf(stream, ": ");
      expWrite(i->let.type, stream);
      fprintf(stream, " = ");
      expWrite(i->let.val, stream);
      fprintf(stream, ";");
      break;
    case STT_VAR:
      fprintf(stream, "var ");
      strWrite(i->var.name, stream);
      fprintf(stream, ": ");
      expWrite(i->var.type, stream);
      if (expLen(i->var.val)) {
        fprintf(stream, " = ");
        expWrite(i->var.val, stream);
      }
      fprintf(stream, ";");
      break;
    case STT_ASS:
      strWrite(i->ass.name, stream);
      fprintf(stream, " = ");
      expWrite(i->ass.val, stream);
      fprintf(stream, ";");
      break;
    case STT_CAS:
      strWrite(i->cas.name, stream);
      fprintf(stream, " %s= ", lxmName(i->cas.op.bin.op));
      expWrite(i->cas.rhs, stream);
      fprintf(stream, ";");
      break;
    case STT_EXP:
      expWrite(i->exp.exp, stream);
      fprintf(stream, ";");
      break;
    default: dbgUnexpected("Unknown statement tag!");
    }
    fprintf(stream, "\n");
  }
}

void prsAdd(Parse* const prs, Statement const stt) {
  reserve(prs, 1);
  *prs->end++ = stt;
}

void prsPop(Parse* const prs) { prs->end--; }
