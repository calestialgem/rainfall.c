// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "utl/api.h"

#include <stdio.h>

char const* lxmName(LexemeType const type) {
  switch (type) {
  case LXM_EQUAL: return "=";
  case LXM_COLON: return ":";
  case LXM_SEMI: return ";";
  case LXM_OPAREN: return "(";
  case LXM_CPAREN: return ")";
  case LXM_PLUS: return "+";
  case LXM_MINUS: return "-";
  case LXM_STAR: return "*";
  case LXM_SLASH: return "/";
  case LXM_PERCENT: return "%";
  case LXM_COMMA: return ",";
  case LXM_EOF: return "eof";
  case LXM_LET: return "let";
  case LXM_VAR: return "var";
  case LXM_DEC: return "decimal";
  case LXM_ID: return "id";
  case LXM_ERR: return "error";
  default: dbgUnexpected("Unknown lexeme type!");
  }
}

void lxmWrite(LexemeType const type, FILE* const stream) {
  if (stream) fprintf(stream, "%s", lxmName(type));
}

void lxmPrint(Lexeme const lxm) {
  printf("%s `", lxmName(lxm.type));
  strWrite(lxm.val, stdout);
  printf("`\n");
}
