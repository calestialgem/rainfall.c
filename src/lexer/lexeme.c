// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lexer/api.h"
#include "utility/api.h"

char const* nameLexeme(LexemeTag named) {
  switch (named) {
  case LEXEME_COMMA: return ",";
  case LEXEME_COLON: return ":";
  case LEXEME_SEMICOLON: return ";";
  case LEXEME_OPENING_PARENTHESIS: return "(";
  case LEXEME_CLOSING_PARENTHESIS: return ")";
  case LEXEME_STAR_EQUAL: return "*=";
  case LEXEME_STAR: return "*";
  case LEXEME_SLASH_EQUAL: return "/=";
  case LEXEME_SLASH: return "/";
  case LEXEME_PERCENT_EQUAL: return "%=";
  case LEXEME_PERCENT: return "%";
  case LEXEME_PLUS_EQUAL: return "+=";
  case LEXEME_PLUS_PLUS: return "++";
  case LEXEME_PLUS: return "+";
  case LEXEME_MINUS_EQUAL: return "-=";
  case LEXEME_MINUS_MINUS: return "--";
  case LEXEME_MINUS: return "-";
  case LEXEME_AMPERCENT_EQUAL: return "&=";
  case LEXEME_AMPERCENT_AMPERCENT: return "&&";
  case LEXEME_AMPERCENT: return "&";
  case LEXEME_PIPE_EQUAL: return "|=";
  case LEXEME_PIPE_PIPE: return "||";
  case LEXEME_PIPE: return "|";
  case LEXEME_CARET_EQUAL: return "^=";
  case LEXEME_CARET: return "^";
  case LEXEME_LEFT_ARROW_LEFT_ARROW_EQUAL: return "<<=";
  case LEXEME_LEFT_ARROW_LEFT_ARROW: return "<<";
  case LEXEME_LEFT_ARROW_EQUAL: return "<=";
  case LEXEME_LEFT_ARROW: return "<";
  case LEXEME_RIGHT_ARROW_RIGHT_ARROW_EQUAL: return ">>=";
  case LEXEME_RIGHT_ARROW_RIGHT_ARROW: return ">>";
  case LEXEME_RIGHT_ARROW_EQUAL: return ">=";
  case LEXEME_RIGHT_ARROW: return ">";
  case LEXEME_EQUAL_EQUAL: return "==";
  case LEXEME_EQUAL: return "=";
  case LEXEME_EXCLAMETION_EQUAL: return "!=";
  case LEXEME_EXCLAMETION: return "!";
  case LEXEME_TILDE: return "~";
  case LEXEME_EOF: return "eof";
  case LEXEME_LET: return "let";
  case LEXEME_VAR: return "var";
  case LEXEME_IDENTIFIER: return "identifier";
  case LEXEME_DECIMAL: return "decimal";
  case LEXEME_ERROR: return "error";
  default: unexpected("Unknown lexeme variant!");
  };
}
