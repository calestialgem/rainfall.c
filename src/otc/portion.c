// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "otc/api.h"
#include "otc/mod.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdio.h>

/* Print the given portion, which is contained in a singe line, to the given
 * stream. Prints "..." as continuation marks according to given skip flag. */
static void print(Portion const por, FILE* const stream, bool const skip) {
  Portion const line = {.bgn = locStart(por.bgn), .end = locEnd(por.end)};

  fprintf(stream, "%8s |\n", "");
  fprintf(stream, "%8i | ", line.bgn.ln);

  // `String` is exclusive, meaning it does not include the character pointed by
  // its `end` member; while, `Portion` is inclusive. Thus, add one to the `end`
  // of `Portion` to get the `end` of `String`.
  strWrite((String){.bgn = line.bgn.pos, .end = line.end.pos + 1}, stream);

  fprintf(stream, "\n%8s |", skip ? "..." : "");

  for (iptr i = 0; i <= por.end.cl; i++)
    fprintf(stream, "%c", i < por.bgn.cl ? ' ' : '~');

  fprintf(stream, "\n");
}

Portion porOf(Source const src, String const part) {
  // `String` is exclusive, meaning it does not include the character pointed by
  // its `end` member; while, `Portion` is inclusive. Thus, subtract one from
  // the `end` of `String` to get the `end` of `Portion`.
  return (Portion){
    .bgn = locOf(src, part.bgn), .end = locOf(src, part.end - 1)};
}

void porUnderline(Portion const por, FILE* const stream) {
  int const span = por.end.ln - por.bgn.ln + 1;
  // If the portion is contained in a single line.
  if (span == 1) {
    print(por, stream, false);
  } else {
    // If there are lines skiped between the begining and end of the portion,
    // pass the flag as set.
    print((Portion){.bgn = por.bgn, .end = locEnd(por.bgn)}, stream, span > 2);
    print((Portion){.bgn = locStart(por.end), .end = por.end}, stream, false);
  }
  fprintf(stream, "\n");
}
