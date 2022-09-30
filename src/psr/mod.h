// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lxr/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

/* Expression with the given initial node capacity. */
Expression     expOf(ux cap);
/* Release the memory resources used by the given expression. */
void           expFree(Expression* exp);
/* Amount of nodes in the given expression. */
ux             expLen(Expression exp);
/* Node at the given index in the given expression. */
ExpressionNode expAt(Expression exp, ux i);
/* Add the given node to the end of the given expression. */
void           expAdd(Expression* exp, ExpressionNode node);

/* Add the given statement to the end of the given parse. */
void prsAdd(Parse* prs, Statement stt);

/* Parse the given lex into the given parse by reporting to the given outcome.
 */
void parserParse(Parse* prs, Outcome* otc, Lex lex);
