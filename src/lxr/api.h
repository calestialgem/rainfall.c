// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "otc/api.h"
#include "utl/api.h"

/* Type of a lexeme. */
typedef enum {
  /* Mark "=". */
  LXR_EQUAL,
  /* Mark ":". */
  LXR_COLON,
  /* Mark ";". */
  LXR_SEMI,
  /* Mark "(". */
  LXR_OPAREN,
  /* Mark ")". */
  LXR_CPAREN,
  /* Mark "+". */
  LXR_PLUS,
  /* Mark "-". */
  LXR_MINUS,
  /* Mark "*". */
  LXR_STAR,
  /* Mark "/". */
  LXR_SLASH,
  /* Mark "%". */
  LXR_PERCENT,
  /* Marks the end of the file. */
  LXR_EOF,
  /* Keyword "let". */
  LXR_LET,
  /* Keyword "var". */
  LXR_VAR,
  /* Identifier. */
  LXR_ID,
  /* Decimal number literal. */
  LXR_DEC,
  /* Characters that could not be lexed. */
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

/* Name of the given lexeme type. */
char const* lxmName(LexemeType type);
/* Print the given lexeme. */
void        lxmPrint(Lexeme lxm);

/* Lex the given source file. Reports to the given outcome. */
Lex    lexOf(Outcome* otc, Source src);
/* Release the memory resources used by the given lex. */
void   lexFree(Lex* lex);
/* Amount of lexemes in the given lex. */
ux     lexLen(Lex lex);
/* Lexeme at the given index in the given buffer. */
Lexeme lexAt(Lex lex, ux i);
