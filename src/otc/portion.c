// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "otc/api.h"
#include "otc/mod.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* Print the given portion, which is contained in a singe line, to the given
 * stream. Prints "..." as continuation marks according to given skip flag. */
static void print(Portion p, FILE* f, bool skip) {
  Portion line = {.first = lineStart(p.first), .last = lineEnd(p.last)};

  fprintf(f, "%8s |\n", "");
  fprintf(f, "%8i | ", line.first.line);

  fwrite(
    line.first.position, sizeof(char),
    line.last.position - line.first.position + 1, f);

  fprintf(f, "\n%8s |", skip ? "..." : "");

  for (ptrdiff_t i = 0; i <= p.last.column; i++)
    fputc(i < p.first.column ? ' ' : '~', f);

  fputc('\n', f);
}

Portion portionOf(Source s, String section) {
  return (Portion){
    .first = locationOf(s, section.first),
    .last  = locationOf(s, section.after - 1)};
}

void underline(Portion p, FILE* f) {
  int span = p.last.line - p.first.line + 1;
  // If the portion is contained in a single line.
  if (span == 1) {
    print(p, f, false);
  } else {
    // If there are lines skiped between the begining and end of the portion,
    // pass the flag as set.
    print((Portion){.first = p.first, .last = lineEnd(p.first)}, f, span > 2);
    print((Portion){.first = lineStart(p.last), .last = p.last}, f, false);
  }
  fputc('\n', f);
}
