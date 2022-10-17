// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lexer/api.h"
#include "lexer/mod.h"
#include "utility/api.h"

#include <stddef.h>

Lex createLex(Source* lexed) {
  Lex result = {.first = NULL, .after = NULL, .bound = NULL};
  lexSource(&result, lexed);
  return result;
}

void disposeLex(Lex* disposed) {
  disposed->first = allocateArray(disposed->first, 0, Lexeme);
  disposed->after = disposed->first;
  disposed->bound = disposed->after;
}

void pushLexeme(Lex* target, Lexeme pushed) {
  reserveArray(target, 1, Lexeme);
  *target->after++ = pushed;
}

void popLexeme(Lex* target) { target->after--; }
