// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lxr/api.h"
#include "otc/api.h"
#include "psr/api.h"

#include <stddef.h>

/* Empty expression with the given initial capacity. */
Expression createExpression(size_t initialCapacity);
/* Release the memory used by the given expression. */
void       disposeExpression(Expression* disposed);
/* Add the given node to the end of the given expression. */
void       pushExpressionNode(Expression* target, ExpressionNode pushed);
/* Add a binding definition statement with the given name, type and value to the
 * given parse. */
void       pushBindingDefinition(
        Parse* target, String pushedName, Expression pushedType,
        Expression pushedValue);
/* Add an inferred binding definition statement with the given name and
 * value to the given parse. */
void pushInferredBindingDefinition(
  Parse* target, String pushedName, Expression pushedValue);
/* Add a variable definition statement with the given name, type and value to
 * the given parse. */
void pushVariableDefinition(
  Parse* target, String pushedName, Expression pushedType,
  Expression pushedInitialValue);
/* Add an inferred variable definition statement with the given name and
 * value to the given parse. */
void pushInferredVariableDefinition(
  Parse* target, String pushedName, Expression pushedInitialValue);
/* Add a defaulted definition statement with the given name, type and value to
 * the given parse. */
void pushDefaultedVariableDefinition(
  Parse* target, String pushedName, Expression pushedType);
/* Add a discarded expression statement with the given expression to the given
 * parse. */
void pushDiscardedExpression(Parse* target, Expression pushedDiscarded);
/* Remove the last statement from the given parse. */
void popStatement(Parse* target);
/* Parse the given lex into the given parse by reporting to the given source. */
void parseLex(Parse* target, Source* reported, Lex parsed);
