// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lxr/api.h"
#include "otc/api.h"

/* Add the given lexeme to the end of the given lex. */
void lexAdd(Lex* lex, Lexeme lxm);
/* Remove the last added lexeme from the given lex. */
void lexPop(Lex* lex);

/* Lex the given source file into the given lex by reporting to the given
 * outcome. */
void lexerLex(Lex* lex, Outcome* otc, Source src);
