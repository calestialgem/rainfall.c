// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "otc/api.h"
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

  otcErrWhole(&otc, "Some error happened!");
  otcWrnWhole(&otc, "And a warning...");

  otcErr(
    &otc,
    (String){
      .bgn = src.con.bgn + 118,
      .end = src.con.bgn + 119,
    },
    "There are magic numbers in the source!");

  otcWrn(
    &otc,
    (String){
      .bgn = src.con.bgn + 190,
      .end = src.con.bgn + 250,
    },
    "Spans multiple lines!");

  otcInfo(
    otc,
    (String){
      .bgn = src.con.bgn + 225,
      .end = src.con.bgn + 245,
    },
    "Without skipping.");

  if (otc.err > 0)
    otcInfoWhole(
      otc, otc.err > 1 ? "There were %u errors." : "There was an error.",
      otc.err);
  if (otc.wrn > 0)
    otcInfoWhole(
      otc, otc.wrn > 1 ? "There were %u warnings." : "There was a warning.",
      otc.wrn);
  srcFree(&src);
}
