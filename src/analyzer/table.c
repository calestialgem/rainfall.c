// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "analyzer/api.h"
#include "analyzer/mod.h"
#include "parser/api.h"
#include "source/api.h"
#include "utility/api.h"

Table createTable(Source* reported, Parse analyzed) {
  Table created = {.first = NULL, .after = NULL, .bound = NULL};
  analyzeParse(&created, reported, analyzed);
  return created;
}

void disposeTable(Table* disposed) {
  for (Symbol* symbol = disposed->first; symbol < disposed->after; symbol++) {
    switch (symbol->tag) {
    case SYMBOL_BINDING: disposeEvaluation(&symbol->asBinding.bound); break;
    case SYMBOL_VARIABLE: disposeEvaluation(&symbol->asVariable.initial); break;
    case SYMBOL_TYPE: break;
    default: unexpected("Unknown symbol variant!");
    }
  }
  disposed->first = allocateArray(disposed->first, 0, Symbol);
  disposed->after = disposed->first;
  disposed->bound = disposed->first;
}

void pushSymbol(Table* target, Symbol pushed) {
  reserveArray(target, 1, Symbol);
  *target->after++ = pushed;
}

void popSymbol(Table* target) { target->after--; }
