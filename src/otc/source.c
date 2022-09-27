// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "api.h"
#include "dbg/api.h"
#include "utl/api.h"

#include <stdio.h>

Source srcOf(char const* const name) {
  // Join the name with the extension.
  String const EXTENSION = strOf("tr");
  String const str       = strOf(name);
  Buffer       path      = bfrOf(0);
  bfrAppend(&path, str);
  bfrPut(&path, '.');
  bfrAppend(&path, EXTENSION);
  bfrPut(&path, 0);

  FILE* const stream = fopen(path.bgn, "r");
  bfrFree(&path);
  dbgExpect(stream, "Could not open file!");

  Buffer con = bfrOf(0);
  bfrRead(&con, stream);
  return (Source){.name = name, .con = con};
}

void srcFree(Source* const src) { bfrFree(&src->con); }

ux srcLen(Source const src) { return bfrLen(src.con); }

char srcAt(Source const src, ux const i) { return bfrAt(src.con, i); }
