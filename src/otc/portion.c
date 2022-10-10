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
static void print(Portion por, FILE* stream, bool skip) {
  Portion line = {.first = lineStart(por.first), .last = lineEnd(por.last)};

  fprintf(stream, "%8s |\n", "");
  fprintf(stream, "%8i | ", line.first.line);

  fwrite(
    line.first.position, sizeof(char),
    line.last.position - line.first.position + 1, stream);

  fprintf(stream, "\n%8s |", skip ? "..." : "");

  for (ptrdiff_t i = 0; i <= por.last.column; i++)
    fputc(i < por.first.column ? ' ' : '~', stream);

  fputc('\n', stream);
}

Portion portionOf(Source source, String section) {
  return (Portion){
    .first = locationAt(source, section.first),
    .last  = locationAt(source, section.after - 1)};
}

void underline(Portion portion, FILE* stream) {
  int span = portion.last.line - portion.first.line + 1;
  // If the portion is contained in a single line.
  if (span == 1) {
    print(portion, stream, false);
  } else {
    // If there are lines skiped between the begining and end of the portion,
    // pass the flag as set.
    print(
      (Portion){.first = portion.first, .last = lineEnd(portion.first)}, stream,
      span > 2);
    print(
      (Portion){.first = lineStart(portion.last), .last = portion.last}, stream,
      false);
  }
  fputc('\n', stream);
}
