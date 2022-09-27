// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
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

  char path[1024];
  sprintf(path, "%s.tr", arguments[1]);
  Buffer bfr = bufferOf(0);
  bufferRead(&bfr, fopen(path, "r"));
  bufferWrite(bfr, fopen("copy.tr", "w"));
}
