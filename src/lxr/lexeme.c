// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lxr/api.h"
#include "utl/api.h"

#include <stdio.h>

char const* lxmName(LexemeType const type) {
  switch (type) {
  case LXR_EQUAL: return "=";
  case LXR_COLON: return ":";
  case LXR_SEMI: return ";";
  case LXR_OPAREN: return "(";
  case LXR_CPAREN: return ")";
  case LXR_PLUS: return "+";
  case LXR_MINUS: return "-";
  case LXR_STAR: return "*";
  case LXR_SLASH: return "/";
  case LXR_PERCENT: return "%";
  case LXR_EOF: return "eof";
  case LXR_LET: return "let";
  case LXR_VAR: return "var";
  case LXR_DEC: return "decimal";
  case LXR_ID: return "id";
  case LXR_ERR: return "error";
  default: return "unknown";
  }
}

void lxmPrint(Lexeme const lxm) {
  printf("%s `", lxmName(lxm.type));
  strWrite(lxm.val, stdout);
  printf("`\n");
}
