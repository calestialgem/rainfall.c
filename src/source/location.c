// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "source/api.h"
#include "source/mod.h"
#include "utility/api.h"

Location createLocation(Source containing, char const* coresponding) {
  Location result = {
    .source = containing, .position = coresponding, .line = 1, .column = 1};
  // Count the columns by looking at the characters that come before the
  // location.
  for (char const* before = containing.contents.first; before < coresponding;
       before++)
    // Count the line if a newline is found.
    if (*before == '\n') {
      result.line++;
      result.column = 1;
    } else {
      result.column++;
    }
  return result;
}

Location findLineStart(Location inLine) {
  // Calculate the position of the line start by using the column number.
  return (Location){
    .source   = inLine.source,
    .position = inLine.position - inLine.column + 1,
    .line     = inLine.line,
    .column   = 1};
}

Location finLineEnd(Location inLine) {
  // Find the line end by looking  at the characters that come after the
  // location.
  for (char const* after = inLine.position;
       after < inLine.source.contents.after; after++)
    // If a new line is found, the line end is the position before that.
    if (*after == '\n')
      return (Location){
        .source   = inLine.source,
        .position = after - 1,
        .line     = inLine.line,
        .column   = inLine.column + after - 1 - inLine.position};

  // Above loop should find a new line at worst at the end of the file.
  unexpected("File does not end with a new line!");
}
