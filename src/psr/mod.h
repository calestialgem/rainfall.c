// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lxr/api.h"
#include "otc/api.h"
#include "psr/api.h"

/* Add the given statement to the end of the given parse. */
void prsAdd(Parse* prs, Statement stt);

/* Parse the given lex into the given parse by reporting to the given outcome.
 */
void parserParse(Parse* prs, Outcome* otc, Lex lex);
