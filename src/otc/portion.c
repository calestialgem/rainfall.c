// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "otc/api.h"
#include "otc/mod.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* Portion of the line at the given position. */
static Portion linePortion(Location source) {
  return (Portion){.first = lineStart(source), .last = lineEnd(source)};
}

/* Print the given portion, which is contained in a singe line, to the given
 * stream. Prints "..." as continuation marks according to given skip flag. */
static void print(Portion source, FILE* target, bool skip) {
  Portion line = linePortion(source.first);

  fprintf(target, "%8s |\n", "");
  fprintf(target, "%8i | ", line.first.line);

  fwrite(
    line.first.position, sizeof(char),
    line.last.position - line.first.position + 1, target);

  fprintf(target, "\n%8s |", skip ? "..." : "");

  for (int i = 0; i <= source.last.column; i++)
    fputc(i < source.first.column ? ' ' : '~', target);

  fputc('\n', target);
}

Portion portionAt(Source source, String section) {
  return (Portion){
    .first = locationAt(source, section.first),
    .last  = locationAt(source, section.after - 1)};
}

void underline(Portion source, FILE* target) {
  int span = source.last.line - source.first.line + 1;
  // If the portion is contained in a single line.
  if (span == 1) {
    print(source, target, false);
  } else {
    // If there are lines skiped between the begining and end of the portion,
    // pass the flag as set.
    print(linePortion(source.first), target, span > 2);
    print(linePortion(source.last), target, false);
  }
  fputc('\n', target);
}
