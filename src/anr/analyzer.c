// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdint.h>

Analyzer anr;

/* Add the built-in symbols. */
static void prepare() {
  for (ux i = 0; i < TYPE_BUILT_LEN; i++) {
    String const name = strOf(typeName(TYPE_BUILT[i]));
    Evaluation   evl  = evlOf(1);
    evl.bgn[0]        = (EvaluationNode){
             .type = TYPE_INS_META, .val = {.meta = TYPE_BUILT[i]}, .has = true};
    setPut(&anr.bld, name);
    tblAdd(anr.tbl, (Symbol){.name = name, .evl = evl});
  }
}

void analyzerAnalyze(Table* const tbl, Outcome* const otc, Parse const prs) {
  anr.tbl = tbl;
  anr.otc = otc;
  anr.prs = prs;
  anr.bld = setOf(0);
  anr.usr = setOf(0);
  anr.res = setOf(0);
  anr.evl = evlOf(0);

  prepare();
  anrResolve();
  anrCheck();
}
