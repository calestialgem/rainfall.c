// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "otc/api.h"
#include "otc/mod.h"

Location locationOf(Source s, char const* position) {
  Location l = {.source = s, .position = position, .line = 1, .column = 1};
  for (char const* i = s.contents.first; i < position; i++)
    if (*i == '\n') {
      l.line++;
      l.column = 1;
    } else {
      l.column++;
    }
  return l;
}

Location lineStart(Location l) {
  return (Location){
    .source   = l.source,
    .position = l.position - l.column + 1,
    .line     = l.line,
    .column   = 1};
}

Location lineEnd(Location l) {
  for (char const* i = l.position; i < l.source.contents.after; i++)
    if (*i == '\n')
      return (Location){
        .source   = l.source,
        .position = i - 1,
        .line     = l.line,
        .column   = l.column + i - 1 - l.position};

  // Above loop should find a new line at worst at the end of the file.
  unexpected("File does not end with a new line!");
}
