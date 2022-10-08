// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "otc/api.h"
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

  // Put the null-terminator as end of file character, and a new line, which
  // makes sure that there is always a line that could be reported to user.
  bfrPut(&con, 0);
  bfrPut(&con, '\n');

  return (Source){.name = name, .con = con};
}

void srcFree(Source* const src) { bfrFree(&src->con); }

iptr srcLen(Source const src) { return bfrLen(src.con); }

char srcAt(Source const src, iptr const i) { return bfrAt(src.con, i); }

char const* srcBgn(Source const src) { return src.con.bgn; }

char const* srcEnd(Source const src) { return src.con.end; }
