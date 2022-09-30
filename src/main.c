// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdio.h>

/* Start the program. */
int main(int const argumentCount, char const* const* const arguments) {
  // Check input arguments.
  if (argumentCount != 2) {
    fprintf(
      stderr, "Provide %s Thrice file!\n",
      argumentCount > 2 ? "only one" : "a");
    return -1;
  }

  Source  src = srcOf(arguments[1]);
  Outcome otc = otcOf(src);

  Lex   lex = lexOf(&otc, src);
  Parse prs = prsOf(&otc, lex);

  if (otc.err > 0)
    otcInfoWhole(
      otc, otc.err > 1 ? "There were %u errors." : "There was an error.",
      otc.err);
  if (otc.wrn > 0)
    otcInfoWhole(
      otc, otc.wrn > 1 ? "There were %u warnings." : "There was a warning.",
      otc.wrn);

  printf("\n\nParse Result:\n\n");
  prsWrite(prs, stdout);

  prsFree(&prs);
  lexFree(&lex);
  srcFree(&src);
}
