// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "psr/mod.h"

Parse createParse(Source* reported, Lex parsed) {
  Parse created = {.first = NULL, .after = NULL, .bound = NULL};
  parseLex(&created, reported, parsed);
  return created;
}

void disposeParse(Parse* disposed) {
  for (Statement* statement = disposed->first; statement < disposed->after;
       statement++) {
    switch (statement->tag) {
    case STATEMENT_BINDING_DEFINITION:
      disposeExpression(&statement->asBindingDefinition.type);
      disposeExpression(&statement->asBindingDefinition.value);
      break;
    case STATEMENT_INFERRED_BINDING_DEFINITION:
      disposeExpression(&statement->asInferredBindingDefinition.value);
      break;
    case STATEMENT_VARIABLE_DEFINITION:
      disposeExpression(&statement->asVariableDefinition.type);
      disposeExpression(&statement->asVariableDefinition.initialValue);
      break;
    case STATEMENT_INFERRED_VARIABLE_DEFINITION:
      disposeExpression(&statement->asInferredVariableDefinition.initialValue);
      break;
    case STATEMENT_DEFAULTED_VARIABLE_DEFINITION:
      disposeExpression(&statement->asDefaultedVariableDefinition.type);
      break;
    case STATEMENT_DISCARDED_EXPRESSION:
      disposeExpression(&statement->asDiscardedExpression.discarded);
      break;
    default: unexpected("Unknown statement variant!");
    }
  }
  disposed->first = allocateArray(disposed->first, 0, Statement);
  disposed->after = disposed->first;
  disposed->bound = disposed->first;
}

static void pushStatement(Parse* target, Statement pushed) {
  reserveArray(target, 1, Statement);
  *target->after++ = pushed;
}

void pushBindingDefinition(
  Parse* target, String pushedName, Expression pushedType,
  Expression pushedValue) {
  pushStatement(
    target, (Statement){
              .asBindingDefinition =
                {.name = pushedName, .type = pushedType, .value = pushedValue},
              .tag = STATEMENT_BINDING_DEFINITION
  });
}

void pushInferredBindingDefinition(
  Parse* target, String pushedName, Expression pushedValue) {
  pushStatement(
    target,
    (Statement){
      .asInferredBindingDefinition = {.name = pushedName, .value = pushedValue},
      .tag                         = STATEMENT_INFERRED_BINDING_DEFINITION
  });
}

void pushVariableDefinition(
  Parse* target, String pushedName, Expression pushedType,
  Expression pushedInitialValue) {
  pushStatement(
    target, (Statement){
              .asVariableDefinition =
                {.name         = pushedName,
                                       .type         = pushedType,
                                       .initialValue = pushedInitialValue},
              .tag = STATEMENT_VARIABLE_DEFINITION
  });
}

void pushInferredVariableDefinition(
  Parse* target, String pushedName, Expression pushedInitialValue) {
  pushStatement(
    target, (Statement){
              .asInferredVariableDefinition =
                {.name = pushedName, .initialValue = pushedInitialValue},
              .tag = STATEMENT_VARIABLE_DEFINITION
  });
}

void pushDefaultedVariableDefinition(
  Parse* target, String pushedName, Expression pushedType) {
  pushStatement(
    target,
    (Statement){
      .asDefaultedVariableDefinition = {.name = pushedName, .type = pushedType},
      .tag                           = STATEMENT_VARIABLE_DEFINITION
  });
}

void pushDiscardedExpression(Parse* target, Expression pushedDiscarded) {
  pushStatement(
    target, (Statement){
              .asDiscardedExpression = {.discarded = pushedDiscarded},
              .tag                   = STATEMENT_DISCARDED_EXPRESSION});
}

void popStatement(Parse* target) { target->after--; }
