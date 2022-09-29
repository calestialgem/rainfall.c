// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "otc/api.h"
#include "utl/api.h"

/* Type of a lexeme. */
typedef enum {
  /* Mark "=". */
  LXM_EQUAL,
  /* Mark ":". */
  LXM_COLON,
  /* Mark ";". */
  LXM_SEMI,
  /* Mark "(". */
  LXM_OPAREN,
  /* Mark ")". */
  LXM_CPAREN,
  /* Mark "+". */
  LXM_PLUS,
  /* Mark "-". */
  LXM_MINUS,
  /* Mark "*". */
  LXM_STAR,
  /* Mark "/". */
  LXM_SLASH,
  /* Mark "%". */
  LXM_PERCENT,
  /* Marks the end of the file. */
  LXM_EOF,
  /* Keyword "let". */
  LXM_LET,
  /* Keyword "var". */
  LXM_VAR,
  /* Identifier. */
  LXM_ID,
  /* Decimal number literal. */
  LXM_DEC,
  /* Characters that could not be lexed. */
  LXM_ERR
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
Lex           lexOf(Outcome* otc, Source src);
/* Release the memory resources used by the given lex. */
void          lexFree(Lex* lex);
/* Amount of lexemes in the given lex. */
ux            lexLen(Lex lex);
/* Lexeme at the given index in the given lex. */
Lexeme        lexAt(Lex lex, ux i);
/* Pointer to the first lexeme of the given lex if it exits. */
Lexeme const* lexBgn(Lex lex);
/* Pointer to one after the last lexeme of the given lex. */
Lexeme const* lexEnd(Lex lex);
