// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lxr/api.h"
#include "lxr/mod.h"
#include "otc/api.h"

/* First mark tag. */
#define MARK_FIRST LEXEME_COMMA
/* Tag after the last mark tag. */
#define MARK_AFTER LEXEME_EOF
/* Amount of marks. */
#define MARKS      (MARK_AFTER - MARK_FIRST)
/* Names of mark lexeme variants. */
extern String marks[MARKS];

/* First keyword tag. */
#define KEYWORD_FIRST LEXEME_LET
/* Tag after the last keyword tag. */
#define KEYWORD_AFTER LEXEME_IDENTIFIER
/* Amount of keywords. */
#define KEYWORDS      (KEYWORD_AFTER - KEYWORD_FIRST)
/* Names of keyword lexeme variants. */
extern String keywords[KEYWORDS];

/* Add the given lexeme to the end of the given lex. */
void pushLexeme(Lex*, Lexeme);
/* Remove the last lexeme from the given lex. */
void popLexeme(Lex*);
/* Lex the given source file into the given lex. */
void lex(Lex*, Source*);
