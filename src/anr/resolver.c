// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/mod.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

/* Resolve the definition with the given name. */
static void def(String const name) {
  if (setGet(anr.bld, name)) {
    otcErr(anr.otc, name, "Name clashes with a built-in symbol!");
    return;
  }
  String const* const prev = setGet(anr.usr, name);
  if (prev) {
    otcErr(anr.otc, name, "Name clashes with a previously defined symbol!");
    otcInfo(*anr.otc, *prev, "Previous definition was here.");
    return;
  }
  setPut(&anr.usr, name);
}

void anrResolve() {
  for (Statement const* i = anr.prs.bgn; i < anr.prs.end; i++) {
    switch (i->tag) {
    case STT_LET: def(i->let.name); break;
    case STT_VAR: def(i->var.name); break;
    default: break;
    }
  }
}
