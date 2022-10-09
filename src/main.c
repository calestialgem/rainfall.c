// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
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

  Source  source  = srcOf(arguments[1]);
  Outcome outcome = otcOf(source);
  Lex     lex     = lexOf(&outcome, source);
  Parse   parse   = prsOf(&outcome, lex);
  Table   table   = tblOf(&outcome, parse);

  if (outcome.err > 0)
    otcInfoWhole(
      outcome,
      outcome.err > 1 ? "There were %u errors." : "There was an error.",
      outcome.err);
  if (outcome.wrn > 0)
    otcInfoWhole(
      outcome,
      outcome.wrn > 1 ? "There were %u warnings." : "There was a warning.",
      outcome.wrn);

  tblFree(&table);
  prsFree(&parse);
  lexFree(&lex);
  srcFree(&source);
}
