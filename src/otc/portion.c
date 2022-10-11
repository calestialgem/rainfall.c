// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "otc/api.h"
#include "otc/mod.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* Portion of the line at the given position. */
static Portion findLine(Location inLine) {
  return (Portion){.first = findLineStart(inLine), .last = finLineEnd(inLine)};
}

/* Print the given portion, which is contained in a singe line, to the given
 * stream. Prints "..." as continuation marks according to given skip flag. */
static void underlineLine(Portion underlined, FILE* target, bool skippedLines) {
  // Print the line number and bars.
  Portion containingLine = findLine(underlined.first);
  fprintf(target, "%8s |\n", "");
  fprintf(target, "%8i | ", containingLine.first.line);

  // Print the line that contains the portion.
  fwrite(
    containingLine.first.position, sizeof(char),
    containingLine.last.position - containingLine.first.position + 1, target);

  // Print continuation characters if there is skipping.
  fprintf(target, "\n%8s |", skippedLines ? "..." : "");

  // Print the underline by printing spaces upto the start of the underlined
  // portion. Use '~' after that, up to the end of the underlined portion.
  for (int i = 0; i <= underlined.last.column; i++)
    fputc(i < underlined.first.column ? ' ' : '~', target);

  fputc('\n', target);
}

Portion createPortion(Source containing, String coresponding) {
  // Conver the both ends of the string to locations. The `after` pointer in the
  // string is excluded while the `last` location of the portion is included.
  // Thus, a 1 is subtracted from the `after` pointer.
  return (Portion){
    .first = createLocation(containing, coresponding.first),
    .last  = createLocation(containing, coresponding.after - 1)};
}

void underlinePortion(Portion underlined, FILE* target) {
  int span = underlined.last.line - underlined.first.line + 1;
  // If the portion is contained in a single line.
  if (span == 1) {
    underlineLine(underlined, target, false);
  } else {
    // If there are lines skiped between the begining and end of the portion,
    // pass the flag as set.
    underlineLine(findLine(underlined.first), target, span > 2);
    underlineLine(findLine(underlined.last), target, false);
  }
  fputc('\n', target);
}
