// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "otc/api.h"
#include "utl/api.h"

#include <stdio.h>

/* Type of a lexeme. */
typedef enum {
  /* Mark ",". */
  LXM_COMMA,
  /* Mark ":". */
  LXM_COLON,
  /* Mark ";". */
  LXM_SEMI,
  /* Mark "(". */
  LXM_OPAREN,
  /* Mark ")". */
  LXM_CPAREN,
  /* Mark "*=". */
  LXM_STAREQ,
  /* Mark "*". */
  LXM_STAR,
  /* Mark "/=". */
  LXM_SLASHEQ,
  /* Mark "/". */
  LXM_SLASH,
  /* Mark "%=". */
  LXM_PERCEQ,
  /* Mark "%". */
  LXM_PERCENT,
  /* Mark "+=". */
  LXM_PLUSEQ,
  /* Mark "++". */
  LXM_PLUSPLUS,
  /* Mark "+". */
  LXM_PLUS,
  /* Mark "-=". */
  LXM_MINUSEQ,
  /* Mark "--". */
  LXM_MINUSMINUS,
  /* Mark "-". */
  LXM_MINUS,
  /* Mark "&=". */
  LXM_AMPEQ,
  /* Mark "&&". */
  LXM_AMPAMP,
  /* Mark "&". */
  LXM_AMP,
  /* Mark "|=". */
  LXM_PIPEEQ,
  /* Mark "||". */
  LXM_PIPEPIPE,
  /* Mark "|". */
  LXM_PIPE,
  /* Mark "^=". */
  LXM_CARETEQ,
  /* Mark "^". */
  LXM_CARET,
  /* Mark "<<=". */
  LXM_LARLAREQ,
  /* Mark "<<". */
  LXM_LARLAR,
  /* Mark "<=". */
  LXM_LAREQ,
  /* Mark "<". */
  LXM_LARROW,
  /* Mark ">>=". */
  LXM_RARRAREQ,
  /* Mark ">>". */
  LXM_RARRAR,
  /* Mark ">=". */
  LXM_RAREQ,
  /* Mark ">". */
  LXM_RARROW,
  /* Mark "==". */
  LXM_EQEQ,
  /* Mark "=". */
  LXM_EQUAL,
  /* Mark "!=". */
  LXM_EXCEQ,
  /* Mark "!". */
  LXM_EXC,
  /* Mark "~". */
  LXM_TILDE,
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
/* Stream out the name of the given lexeme type as string to the given stream.
 * Does not do anything if the stream is null. */
void        lxmWrite(LexemeType type, FILE* stream);
/* Print the given lexeme. */
void        lxmPrint(Lexeme lxm);

/* Lex the given source file. Reports to the given outcome. */
Lex           lexOf(Outcome* otc, Source src);
/* Release the memory resources used by the given lex. */
void          lexFree(Lex* lex);
/* Amount of lexemes in the given lex. */
iptr          lexLen(Lex lex);
/* Lexeme at the given index in the given lex. */
Lexeme        lexAt(Lex lex, iptr i);
/* Pointer to the first lexeme of the given lex if it exits. */
Lexeme const* lexBgn(Lex lex);
/* Pointer to one after the last lexeme of the given lex. */
Lexeme const* lexEnd(Lex lex);
