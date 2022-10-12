// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "otc/api.h"
#include "utl/api.h"

/* Variant of a lexeme. */
typedef enum {
  /* Mark ",". */
  LEXEME_COMMA,
  /* Mark ":". */
  LEXEME_COLON,
  /* Mark ";". */
  LEXEME_SEMICOLON,
  /* Mark "(". */
  LEXEME_OPENING_PARENTHESIS,
  /* Mark ")". */
  LEXEME_CLOSING_PARENTHESIS,
  /* Mark "*=". */
  LEXEME_STAR_EQUAL,
  /* Mark "*". */
  LEXEME_STAR,
  /* Mark "/=". */
  LEXEME_SLASH_EQUAL,
  /* Mark "/". */
  LEXEME_SLASH,
  /* Mark "%=". */
  LEXEME_PERCENT_EQUAL,
  /* Mark "%". */
  LEXEME_PERCENT,
  /* Mark "+=". */
  LEXEME_PLUS_EQUAL,
  /* Mark "++". */
  LEXEME_PLUS_PLUS,
  /* Mark "+". */
  LEXEME_PLUS,
  /* Mark "-=". */
  LEXEME_MINUS_EQUAL,
  /* Mark "--". */
  LEXEME_MINUS_MINUS,
  /* Mark "-". */
  LEXEME_MINUS,
  /* Mark "&=". */
  LEXEME_AMPERCENT_EQUAL,
  /* Mark "&&". */
  LEXEME_AMPERCENT_AMPERCENT,
  /* Mark "&". */
  LEXEME_AMPERCENT,
  /* Mark "|=". */
  LEXEME_PIPE_EQUAL,
  /* Mark "||". */
  LEXEME_PIPE_PIPE,
  /* Mark "|". */
  LEXEME_PIPE,
  /* Mark "^=". */
  LEXEME_CARET_EQUAL,
  /* Mark "^". */
  LEXEME_CARET,
  /* Mark "<<=". */
  LEXEME_LEFT_ARROW_LEFT_ARROW_EQUAL,
  /* Mark "<<". */
  LEXEME_LEFT_ARROW_LEFT_ARROW,
  /* Mark "<=". */
  LEXEME_LEFT_ARROW_EQUAL,
  /* Mark "<". */
  LEXEME_LEFT_ARROW,
  /* Mark ">>=". */
  LEXEME_RIGHT_ARROW_RIGHT_ARROW_EQUAL,
  /* Mark ">>". */
  LEXEME_RIGHT_ARROW_RIGHT_ARROW,
  /* Mark ">=". */
  LEXEME_RIGHT_ARROW_EQUAL,
  /* Mark ">". */
  LEXEME_RIGHT_ARROW,
  /* Mark "==". */
  LEXEME_EQUAL_EQUAL,
  /* Mark "=". */
  LEXEME_EQUAL,
  /* Mark "!=". */
  LEXEME_EXCLAMETION_EQUAL,
  /* Mark "!". */
  LEXEME_EXCLAMETION,
  /* Mark "~". */
  LEXEME_TILDE,
  /* Marks the end of the file. */
  LEXEME_EOF,
  /* Keyword "let". */
  LEXEME_LET,
  /* Keyword "var". */
  LEXEME_VAR,
  /* Identifier. */
  LEXEME_IDENTIFIER,
  /* Decimal number literal. */
  LEXEME_DECIMAL,
  /* Characters that could not be lexed. */
  LEXEME_ERROR
} LexemeTag;

/* Indivisible group of characters from the source file. */
typedef struct {
  /* Section of the source the lexeme occupies. */
  String    section;
  /* Variant of the lexeme. */
  LexemeTag tag;
} Lexeme;

/* Result of lexing a source file. */
typedef struct {
  /* Pointer to the first lexeme if it exists. */
  Lexeme* first;
  /* Pointer to one after the last lexeme. */
  Lexeme* after;
  /* Pointer to one after the last allocated lexeme. */
  Lexeme* bound;
} Lex;

/* Initialize the module. */
void        initLexer(void);
/* Lex the given source file. */
Lex         createLex(Source* lexed);
/* Release the memory resources used by the given lex. */
void        disposeLex(Lex* disposed);
/* Name of the given lexeme variant. */
char const* nameLexemeTag(LexemeTag named);
