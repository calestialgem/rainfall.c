// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "otc/api.h"
#include "otc/mod.h"

Location locOf(Source const src, char const* const pos) {
  Location res = {.src = src, .pos = pos, .ln = 1, .cl = 1};
  for (char const* i = srcBgn(src); i < pos; i++)
    if (*i == '\n') {
      res.ln++;
      res.cl = 1;
    } else {
      res.cl++;
    }
  return res;
}

Location locStart(Location const loc) {
  return (Location){
    .src = loc.src, .pos = loc.pos - loc.cl + 1, .ln = loc.ln, .cl = 1};
}

Location locEnd(Location const loc) {
  for (char const* i = loc.pos; i < srcEnd(loc.src); i++)
    if (*i == '\n')
      return (Location){
        .src = loc.src,
        .pos = i - 1,
        .ln  = loc.ln,
        .cl  = (int)(loc.cl + i - 1 - loc.pos)};

  // Above loop should find a new line at worst at the end of the file.
  dbgUnexpected("File does not end with a new line!");
}
