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
  BindingDefinition pushed = {
    .name = pushedName, .type = pushedType, .value = pushedValue};
  pushStatement(
    target,
    (Statement){
      .asBindingDefinition = pushed, .tag = STATEMENT_BINDING_DEFINITION});
}

void pushInferredBindingDefinition(
  Parse* target, String pushedName, Expression pushedValue) {
  InferredBindingDefinition pushed = {.name = pushedName, .value = pushedValue};
  pushStatement(
    target, (Statement){
              .asInferredBindingDefinition = pushed,
              .tag = STATEMENT_INFERRED_BINDING_DEFINITION});
}

void pushVariableDefinition(
  Parse* target, String pushedName, Expression pushedType,
  Expression pushedInitialValue) {
  VariableDefinition pushed = {
    .name = pushedName, .type = pushedType, .initialValue = pushedInitialValue};
  pushStatement(
    target,
    (Statement){
      .asVariableDefinition = pushed, .tag = STATEMENT_VARIABLE_DEFINITION});
}

void pushInferredVariableDefinition(
  Parse* target, String pushedName, Expression pushedInitialValue) {
  InferredVariableDefinition pushed = {
    .name = pushedName, .initialValue = pushedInitialValue};
  pushStatement(
    target, (Statement){
              .asInferredVariableDefinition = pushed,
              .tag = STATEMENT_INFERRED_VARIABLE_DEFINITION});
}

void pushDefaultedVariableDefinition(
  Parse* target, String pushedName, Expression pushedType) {
  DefaultedVariableDefinition pushed = {.name = pushedName, .type = pushedType};
  pushStatement(
    target, (Statement){
              .asDefaultedVariableDefinition = pushed,
              .tag = STATEMENT_DEFAULTED_VARIABLE_DEFINITION});
}

void pushDiscardedExpression(Parse* target, Expression pushedDiscarded) {
  DiscardedExpression pushed = {.discarded = pushedDiscarded};
  pushStatement(
    target,
    (Statement){
      .asDiscardedExpression = pushed, .tag = STATEMENT_DISCARDED_EXPRESSION});
}

void popStatement(Parse* target) { target->after--; }
