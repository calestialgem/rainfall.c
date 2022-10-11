// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "otc/api.h"
#include "otc/mod.h"

Location locationAt(Source source, char const* position) {
  Location result = {
    .source = source, .position = position, .line = 1, .column = 1};
  for (char const* before = source.contents.first; before < position; before++)
    if (*before == '\n') {
      result.line++;
      result.column = 1;
    } else {
      result.column++;
    }
  return result;
}

Location lineStart(Location source) {
  return (Location){
    .source   = source.source,
    .position = source.position - source.column + 1,
    .line     = source.line,
    .column   = 1};
}

Location lineEnd(Location source) {
  for (char const* position = source.position;
       position < source.source.contents.after; position++)
    if (*position == '\n')
      return (Location){
        .source   = source.source,
        .position = position - 1,
        .line     = source.line,
        .column   = source.column + position - 1 - source.position};

  // Above loop should find a new line at worst at the end of the file.
  unexpected("File does not end with a new line!");
}
