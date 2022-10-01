// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

/* Context of the analysis process. */
static struct {
  /* Table to add the symbols into. */
  Table*   tbl;
  /* Outcome to report to. */
  Outcome* otc;
  /* Analyzed parse. */
  Parse    prs;
  /* Set of built-in symbols. */
  Set      bld;
  /* Set of user-defined symbols. */
  Set      usr;
  /* Set of user-defined, resolved but not checked symbols. */
  Set      res;
} anr;

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

/* Resolve the definition with the given name. */
static void resolveDef(String const name) {
  if (setGet(anr.bld, name)) {
    otcErr(anr.otc, name, "Name clashes with a built-in symbol!");
    return;
  }
  String const* const userDef = setGet(anr.res, name);
  if (userDef) {
    otcErr(anr.otc, name, "Name clashes with a previously defined symbol!");
    otcInfo(*anr.otc, *userDef, "Previous definition was here.");
    return;
  }
  setPut(&anr.res, name);
}

/* Resolve name clashes in the definitions. */
static void resolve() {
  for (Statement const* i = anr.prs.bgn; i < anr.prs.end; i++) {
    switch (i->tag) {
    case STT_LET: resolveDef(i->let.name); break;
    case STT_VAR: resolveDef(i->var.name); break;
    default: break;
    }
  }
}

void analyzerAnalyze(Table* const tbl, Outcome* const otc, Parse const prs) {
  anr.tbl = tbl;
  anr.otc = otc;
  anr.prs = prs;
  anr.bld = setOf(0);
  anr.usr = setOf(0);
  anr.res = setOf(0);

  prepare();
  resolve();
}
