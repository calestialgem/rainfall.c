// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "utl/api.h"

#include <stdio.h>

char const* lxmName(LexemeType const type) {
  switch (type) {
  case LXM_COMMA: return ",";
  case LXM_COLON: return ":";
  case LXM_SEMI: return ";";
  case LXM_OPAREN: return "(";
  case LXM_CPAREN: return ")";
  case LXM_STAR: return "*";
  case LXM_SLASH: return "/";
  case LXM_PERCENT: return "%";
  case LXM_PLUSPLUS: return "++";
  case LXM_PLUS: return "+";
  case LXM_MINUSMINUS: return "--";
  case LXM_MINUS: return "-";
  case LXM_AMPAMP: return "&&";
  case LXM_AMP: return "&";
  case LXM_PIPEPIPE: return "||";
  case LXM_PIPE: return "|";
  case LXM_CARET: return "^";
  case LXM_LARLAR: return "<<";
  case LXM_LAREQ: return "<=";
  case LXM_LARROW: return "<";
  case LXM_RARRAR: return ">>";
  case LXM_RAREQ: return ">=";
  case LXM_RARROW: return ">";
  case LXM_EQEQ: return "==";
  case LXM_EQUAL: return "=";
  case LXM_EXCEQ: return "!=";
  case LXM_EXC: return "!";
  case LXM_TILDE: return "~";
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
