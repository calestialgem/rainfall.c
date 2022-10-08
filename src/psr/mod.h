// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lxr/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

/* Expression with the given initial node capacity. */
Expression     expOf(iptr cap);
/* Release the memory resources used by the given expression. */
void           expFree(Expression* exp);
/* Node at the given index in the given expression. */
ExpressionNode expAt(Expression exp, iptr i);
/* Add the given node to the end of the given expression. */
void           expAdd(Expression* exp, ExpressionNode node);
/* Stream out the given expression as string to the given stream. */
void           expWrite(Expression exp, FILE* stream);
/* Stream out the given expression tree as string to the given stream. */
void           expTree(Expression exp, FILE* stream);

/* Add the given statement to the end of the given parse. */
void prsAdd(Parse* prs, Statement stt);
/* Remove the last added statement from the given parse. */
void prsPop(Parse* prs);

/* Parse the given lex into the given parse by reporting to the given outcome.
 */
void parserParse(Parse* prs, Outcome* otc, Lex lex);
