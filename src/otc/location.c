// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "otc/api.h"
#include "otc/mod.h"

Location locationAt(Source source, char const* position) {
  Location res = {
    .source = source, .position = position, .line = 1, .column = 1};
  for (char const* i = source.contents.first; i < position; i++)
    if (*i == '\n') {
      res.line++;
      res.column = 1;
    } else {
      res.column++;
    }
  return res;
}

Location lineStart(Location location) {
  return (Location){
    .source   = location.source,
    .position = location.position - location.column + 1,
    .line     = location.line,
    .column   = 1};
}

Location lineEnd(Location location) {
  for (char const* i = location.position; i < location.source.contents.after;
       i++)
    if (*i == '\n')
      return (Location){
        .source   = location.source,
        .position = i - 1,
        .line     = location.line,
        .column   = location.column + i - 1 - location.position};

  // Above loop should find a new line at worst at the end of the file.
  unexpected("File does not end with a new line!");
}
