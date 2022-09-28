// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "otc/api.h"
#include "utl/api.h"

/* Type of a lexeme. */
typedef enum {
  LXR_EQUAL,
  LXR_COLON,
  LXR_SEMI,
  LXR_OPAREN,
  LXR_CPAREN,
  LXR_PLUS,
  LXR_MINUS,
  LXR_STAR,
  LXR_SLASH,
  LXR_PERCENT,
  LXR_EOF,
  LXR_LET,
  LXR_VAR,
  LXR_DEC,
  LXR_ID,
  LXR_ERR
} LexemeType;

/* Indivisible group of characters from the source file. */
typedef struct {
  /* View of the characters. */
  String     val;
  /* Type of the lexeme. */
  LexemeType type;
} Lexeme;

/* Result of lexing a source file. */
typedef struct {
  /* Pointer to the first lexeme if it exists. */
  Lexeme* bgn;
  /* Pointer to one after the last lexeme. */
  Lexeme* end;
  /* Pointer to one after the last allocated lexeme. */
  Lexeme* all;
} Lex;

/* Name of the given token type. */
char const* lxmTypeName(LexemeType type);

/* Print the given token. */
void lxmPrint(Lexeme lxm);

/* Lex the given source file. Reports to the given outcome. */
Lex    lexOf(Outcome* otc, Source src);
/* Release the memory resources used by the given lex. */
void   lexFree(Lex* lex);
/* Amount of lexemes in the given lex. */
ux     lexLen(Lex lex);
/* Lexeme at the given index in the given buffer. */
Lexeme lexAt(Lex lex, ux i);
