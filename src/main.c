// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "analyzer/api.h"
#include "lexer/api.h"
#include "parser/api.h"
#include "source/api.h"
#include "utility/api.h"

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

  initLexer();

  Source source = createSource(arguments[1]);
  Lex    lex    = createLex(&source);
  Parse  parse  = createParse(&source, lex);
  Table  table  = createTable(&source, parse);

  if (source.errors > 0)
    reportInfo(
      &source,
      source.errors > 1 ? "There were %u errors." : "There was an error.",
      source.errors);
  if (source.warnings > 0)
    reportInfo(
      &source,
      source.warnings > 1 ? "There were %u warnings." : "There was a warning.",
      source.warnings);

  disposeTable(&table);
  disposeParse(&parse);
  disposeLex(&lex);
  disposeSource(&source);
}
