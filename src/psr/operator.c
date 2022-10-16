// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "psr/api.h"

#include <stdbool.h>
#include <stddef.h>

/* Create a nullary operator with the given only lexeme. */
#define createNullaryOperator(precedenceLevel, inLevelIndex, onlyLexeme) \
  {                                                                      \
    .asNullary = {.only = onlyLexeme}, .tag = OPERATOR_NULLARY,          \
    .precedence = precedenceLevel, .inLevel = inLevelIndex,              \
    .hash = hashOperator(precedenceLevel, inLevelIndex)                  \
  }

/* Create a prenary operator with the given before lexeme. */
#define createPrenaryOperator(precedenceLevel, inLevelIndex, lexemeBefore) \
  {                                                                        \
    .asPrenary = {.before = lexemeBefore}, .tag = OPERATOR_PRENARY,        \
    .precedence = precedenceLevel, .inLevel = inLevelIndex,                \
    .hash = hashOperator(precedenceLevel, inLevelIndex)                    \
  }

/* Create a postary operator with the given after lexeme. */
#define createPostaryOperator(precedenceLevel, inLevelIndex, lexemeAfter) \
  {                                                                       \
    .asPostary = {.after = lexemeAfter}, .tag = OPERATOR_POSTARY,         \
    .precedence = precedenceLevel, .inLevel = inLevelIndex,               \
    .hash = hashOperator(precedenceLevel, inLevelIndex)                   \
  }

/* Create a cirnary operator with the given opening and closing lexemes. */
#define createCirnaryOperator(                                         \
  precedenceLevel, inLevelIndex, openingLexeme, closingLexeme)         \
  {                                                                    \
    .asCirnary = {.opening = openingLexeme, .closing = closingLexeme}, \
    .tag = OPERATOR_CIRNARY, .precedence = precedenceLevel,            \
    .inLevel = inLevelIndex,                                           \
    .hash    = hashOperator(precedenceLevel, inLevelIndex)             \
  }

/* Create a binary operator with the given lexeme between. */
#define createBinaryOperator(precedenceLevel, inLevelIndex, lexemeBetween) \
  {                                                                        \
    .asBinary = {.between = lexemeBetween}, .tag = OPERATOR_BINARY,        \
    .precedence = precedenceLevel, .inLevel = inLevelIndex,                \
    .hash = hashOperator(precedenceLevel, inLevelIndex)                    \
  }

/* Create a variary operator with the given opening, separating, and closing
 * lexemes. */
#define createVariaryOperator(                                    \
  precedenceLevel, inLevelIndex, openingLexeme, separatingLexeme, \
  closingLexeme)                                                  \
  {                                                               \
    .asVariary =                                                  \
      {.opening    = openingLexeme,                               \
       .separating = separatingLexeme,                            \
       .closing    = closingLexeme},                                 \
    .tag = OPERATOR_VARIARY, .precedence = precedenceLevel,       \
    .inLevel = inLevelIndex,                                      \
    .hash    = hashOperator(precedenceLevel, inLevelIndex)        \
  }

Operator const DECIMAL_LITERAL_OPERATOR =
  createNullaryOperator(OPERATOR_PRIMARY, 0, LEXEME_DECIMAL);
Operator const SYMBOL_ACCESS_OPERATOR =
  createNullaryOperator(OPERATOR_PRIMARY, 1, LEXEME_IDENTIFIER);
Operator const GROUP_OPERATOR = createCirnaryOperator(
  OPERATOR_PRIMARY, 2, LEXEME_OPENING_PARENTHESIS, LEXEME_CLOSING_PARENTHESIS);
Operator const FUNCTION_CALL_OPERATOR = createVariaryOperator(
  OPERATOR_PRIMARY, 3, LEXEME_OPENING_PARENTHESIS, LEXEME_COMMA,
  LEXEME_CLOSING_PARENTHESIS);

/* Amount of primary operators. */
#define PRIMARY_OPERATORS 4

Operator const POSATE_OPERATOR =
  createPrenaryOperator(OPERATOR_UNARY, 0, LEXEME_PLUS);
Operator const NEGATE_OPERATOR =
  createPrenaryOperator(OPERATOR_UNARY, 1, LEXEME_MINUS);
Operator const POSTFIX_INCREMENT_OPERATOR =
  createPostaryOperator(OPERATOR_UNARY, 2, LEXEME_PLUS_PLUS);
Operator const POSTFIX_DECREMENT_OPERATOR =
  createPostaryOperator(OPERATOR_UNARY, 3, LEXEME_MINUS_MINUS);
Operator const PREFIX_INCREMENT_OPERATOR =
  createPrenaryOperator(OPERATOR_UNARY, 4, LEXEME_PLUS_PLUS);
Operator const PREFIX_DECREMENT_OPERATOR =
  createPrenaryOperator(OPERATOR_UNARY, 5, LEXEME_MINUS_MINUS);
Operator const LOGICAL_NOT_OPERATOR =
  createPrenaryOperator(OPERATOR_UNARY, 6, LEXEME_EXCLAMETION);
Operator const COMPLEMENT_OPERATOR =
  createPrenaryOperator(OPERATOR_UNARY, 7, LEXEME_TILDE);

/* Amount of unary operators. */
#define UNARY_OPERATORS 8

Operator const MULTIPLICATION_OPERATOR =
  createBinaryOperator(OPERATOR_FACTOR, 0, LEXEME_STAR);
Operator const DIVISION_OPERATOR =
  createBinaryOperator(OPERATOR_FACTOR, 1, LEXEME_SLASH);
Operator const REMINDER_OPERATOR =
  createBinaryOperator(OPERATOR_FACTOR, 2, LEXEME_PERCENT);

/* Amount of factor operators. */
#define FACTOR_OPERATORS 3

Operator const ADDITION_OPERATOR =
  createBinaryOperator(OPERATOR_TERM, 0, LEXEME_PLUS);
Operator const SUBTRACTION_OPERATOR =
  createBinaryOperator(OPERATOR_TERM, 1, LEXEME_MINUS);

/* Amount of term operators. */
#define TERM_OPERATORS 2

Operator const LEFT_SHIFT_OPERATOR =
  createBinaryOperator(OPERATOR_SHIFT, 0, LEXEME_LEFT_ARROW_LEFT_ARROW);
Operator const RIGHT_SHIFT_OPERATOR =
  createBinaryOperator(OPERATOR_SHIFT, 1, LEXEME_RIGHT_ARROW_RIGHT_ARROW);

/* Amount of shift operators. */
#define SHIFT_OPERATORS 2

Operator const BITWISE_AND_OPERATOR =
  createBinaryOperator(OPERATOR_BITWISE_AND, 0, LEXEME_AMPERCENT);

/* Amount of bitwise AND operators. */
#define BITWISE_AND_OPERATORS 1

Operator const BITWISE_XOR_OPERATOR =
  createBinaryOperator(OPERATOR_BITWISE_XOR, 0, LEXEME_CARET);

/* Amount of bitwise XOR operators. */
#define BITWISE_XOR_OPERATORS 1

Operator const BITWISE_OR_OPERATOR =
  createBinaryOperator(OPERATOR_BITWISE_OR, 0, LEXEME_PIPE);

/* Amount of bitwise OR operators. */
#define BITWISE_OR_OPERATORS 1

Operator const SMALLER_THAN_OPERATOR =
  createBinaryOperator(OPERATOR_ORDER_COMPARISON, 0, LEXEME_LEFT_ARROW);
Operator const SMALLER_THAN_OR_EQUAL_TO_OPERATOR =
  createBinaryOperator(OPERATOR_ORDER_COMPARISON, 1, LEXEME_LEFT_ARROW_EQUAL);
Operator const GREATER_THAN_OPERATOR =
  createBinaryOperator(OPERATOR_ORDER_COMPARISON, 2, LEXEME_RIGHT_ARROW);
Operator const GREATER_THAN_OR_EQUAL_TO_OPERATOR =
  createBinaryOperator(OPERATOR_ORDER_COMPARISON, 3, LEXEME_RIGHT_ARROW_EQUAL);

/* Amount of order comparison operators. */
#define ORDER_COMPARISON_OPERATORS 4

Operator const EQUAL_TO_OPERATOR =
  createBinaryOperator(OPERATOR_EQUALITY_COMPARISON, 0, LEXEME_EQUAL_EQUAL);
Operator const NOT_EQUAL_TO_OPERATOR = createBinaryOperator(
  OPERATOR_EQUALITY_COMPARISON, 1, LEXEME_EXCLAMETION_EQUAL);

/* Amount of equality comparison operators. */
#define EQUALITY_COMPARISON_OPERATORS 2

Operator const LOGICAL_AND_OPERATOR =
  createBinaryOperator(OPERATOR_LOGICAL_AND, 0, LEXEME_AMPERCENT_AMPERCENT);

/* Amount of logical AND operators. */
#define LOGICAL_AND_OPERATORS 1

Operator const LOGICAL_OR_OPERATOR =
  createBinaryOperator(OPERATOR_LOGICAL_OR, 0, LEXEME_PIPE_PIPE);

/* Amount of logical OR operators. */
#define LOGICAL_OR_OPERATORS 1

Operator const ASSIGNMENT_OPERATOR =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 0, LEXEME_EQUAL);
Operator const MUTIPLICATION_ASSIGNMENT_OPERATOR =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 1, LEXEME_STAR_EQUAL);
Operator const DIVISION_ASSIGNMENT_OPERATOR =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 2, LEXEME_SLASH_EQUAL);
Operator const REMINDER_ASSIGNMENT_OPERATOR =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 3, LEXEME_PERCENT_EQUAL);
Operator const ADDITION_ASSIGNMENT_OPERATOR =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 4, LEXEME_PLUS_EQUAL);
Operator const SUBTRACTION_ASSIGNMENT_OPERATOR =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 5, LEXEME_MINUS_EQUAL);
Operator const LEFT_SHIFT_ASSIGNMENT_OPERATOR = createBinaryOperator(
  OPERATOR_ASSIGNMENT, 6, LEXEME_LEFT_ARROW_LEFT_ARROW_EQUAL);
Operator const RIGHT_SHIFT_ASSIGNMENT_OPERATOR = createBinaryOperator(
  OPERATOR_ASSIGNMENT, 7, LEXEME_RIGHT_ARROW_RIGHT_ARROW_EQUAL);
Operator const BITWISE_AND_ASSIGNMENT_OPERATOR =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 8, LEXEME_AMPERCENT_EQUAL);
Operator const BITWISE_XOR_ASSIGNMENT_OPERATOR =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 9, LEXEME_CARET_EQUAL);
Operator const BITWISE_OR_ASSIGNMENT_OPERATOR =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 10, LEXEME_PIPE_EQUAL);

/* Amount of assignment operators. */
#define ASSIGNMENT_OPERATORS 11

bool compareOperatorEquality(Operator left, Operator right) {
  return left.hash == right.hash;
}

size_t countInLevelOperators(OperatorPrecedence counted) {
  switch (counted) {
  case OPERATOR_ASSIGNMENT: return ASSIGNMENT_OPERATORS;
  case OPERATOR_LOGICAL_OR: return LOGICAL_OR_OPERATORS;
  case OPERATOR_LOGICAL_AND: return LOGICAL_AND_OPERATORS;
  case OPERATOR_EQUALITY_COMPARISON: return EQUALITY_COMPARISON_OPERATORS;
  case OPERATOR_ORDER_COMPARISON: return ORDER_COMPARISON_OPERATORS;
  case OPERATOR_BITWISE_OR: return BITWISE_OR_OPERATORS;
  case OPERATOR_BITWISE_XOR: return BITWISE_XOR_OPERATORS;
  case OPERATOR_BITWISE_AND: return BITWISE_AND_OPERATORS;
  case OPERATOR_SHIFT: return SHIFT_OPERATORS;
  case OPERATOR_TERM: return TERM_OPERATORS;
  case OPERATOR_FACTOR: return FACTOR_OPERATORS;
  case OPERATOR_UNARY: return UNARY_OPERATORS;
  case OPERATOR_PRIMARY: return PRIMARY_OPERATORS;
  default: unexpected("Unknown operator precedence!");
  }
}

Operator getOperator(size_t gottenHash) {
  switch (gottenHash) {
  case ASSIGNMENT: return ASSIGNMENT_OPERATOR;
  case MULTIPLICATION_ASSIGNMENT: return MUTIPLICATION_ASSIGNMENT_OPERATOR;
  case DIVISION_ASSIGNMENT: return DIVISION_ASSIGNMENT_OPERATOR;
  case REMINDER_ASSIGNMENT: return REMINDER_ASSIGNMENT_OPERATOR;
  case ADDITION_ASSIGNMENT: return ADDITION_ASSIGNMENT_OPERATOR;
  case SUBTRACTION_ASSIGNMENT: return SUBTRACTION_ASSIGNMENT_OPERATOR;
  case LEFT_SHIFT_ASSIGNMENT: return LEFT_SHIFT_ASSIGNMENT_OPERATOR;
  case RIGHT_SHIFT_ASSIGNMENT: return RIGHT_SHIFT_ASSIGNMENT_OPERATOR;
  case BITWISE_AND_ASSIGNMENT: return BITWISE_AND_ASSIGNMENT_OPERATOR;
  case BITWISE_XOR_ASSIGNMENT: return BITWISE_XOR_ASSIGNMENT_OPERATOR;
  case BITWISE_OR_ASSIGNMENT: return BITWISE_OR_ASSIGNMENT_OPERATOR;
  case LOGICAL_OR: return LOGICAL_OR_OPERATOR;
  case LOGICAL_AND: return LOGICAL_AND_OPERATOR;
  case EQUAL_TO: return EQUAL_TO_OPERATOR;
  case NOT_EQUAL_TO: return NOT_EQUAL_TO_OPERATOR;
  case SMALLER_THAN: return SMALLER_THAN_OPERATOR;
  case SMALLER_THAN_OR_EQUAL_TO: return SMALLER_THAN_OR_EQUAL_TO_OPERATOR;
  case GREATER_THAN: return GREATER_THAN_OPERATOR;
  case GREATER_THAN_OR_EQUAL_TO: return GREATER_THAN_OR_EQUAL_TO_OPERATOR;
  case BITWISE_AND: return BITWISE_AND_OPERATOR;
  case BITWISE_XOR: return BITWISE_XOR_OPERATOR;
  case BITWISE_OR: return BITWISE_OR_OPERATOR;
  case LEFT_SHIFT: return LEFT_SHIFT_OPERATOR;
  case RIGHT_SHIFT: return RIGHT_SHIFT_OPERATOR;
  case ADDITION: return ADDITION_OPERATOR;
  case SUBTRACTION: return SUBTRACTION_OPERATOR;
  case MULTIPLICATION: return MULTIPLICATION_OPERATOR;
  case DIVISION: return DIVISION_OPERATOR;
  case REMINDER: return REMINDER_OPERATOR;
  case POSATE: return POSATE_OPERATOR;
  case NEGATE: return NEGATE_OPERATOR;
  case POSTFIX_INCREMENT: return POSTFIX_INCREMENT_OPERATOR;
  case POSTFIX_DECREMENT: return POSTFIX_DECREMENT_OPERATOR;
  case PREFIX_INCREMENT: return PREFIX_INCREMENT_OPERATOR;
  case PREFIX_DECREMENT: return PREFIX_DECREMENT_OPERATOR;
  case LOGICAL_NOT: return LOGICAL_NOT_OPERATOR;
  case COMPLEMENT: return COMPLEMENT_OPERATOR;
  case DECIMAL_LITERAL: return DECIMAL_LITERAL_OPERATOR;
  case SYMBOL_ACCESS: return SYMBOL_ACCESS_OPERATOR;
  case GROUP: return GROUP_OPERATOR;
  case FUNCTION_CALL: return FUNCTION_CALL_OPERATOR;
  default: unexpected("Unknown operator hash!");
  }
}

Operator
getOperatorAt(OperatorPrecedence gottenPrecedence, size_t gottenInLevelIndex) {
  return getOperator(hashOperator(gottenPrecedence, gottenInLevelIndex));
}

char const* nameOperator(Operator named) {
  switch (named.hash) {
  case ASSIGNMENT: return "assignment";
  case MULTIPLICATION_ASSIGNMENT: return "multiplication assignment";
  case DIVISION_ASSIGNMENT: return "division assignment";
  case REMINDER_ASSIGNMENT: return "reminder assignment";
  case ADDITION_ASSIGNMENT: return "addition assignment";
  case SUBTRACTION_ASSIGNMENT: return "subtraction assignment";
  case LEFT_SHIFT_ASSIGNMENT: return "left shift assignment";
  case RIGHT_SHIFT_ASSIGNMENT: return "right shift assignment";
  case BITWISE_AND_ASSIGNMENT: return "bitwise and assignment";
  case BITWISE_XOR_ASSIGNMENT: return "bitwise xor assignment";
  case BITWISE_OR_ASSIGNMENT: return "bitwise or assignment";
  case LOGICAL_OR: return "logical or";
  case LOGICAL_AND: return "logical and";
  case EQUAL_TO: return "equal to";
  case NOT_EQUAL_TO: return "not equal to";
  case SMALLER_THAN: return "smaller than";
  case SMALLER_THAN_OR_EQUAL_TO: return "smaller than or equal to";
  case GREATER_THAN: return "greater than";
  case GREATER_THAN_OR_EQUAL_TO: return "greater than or equal to";
  case BITWISE_AND: return "bitwise and";
  case BITWISE_XOR: return "bitwise xor";
  case BITWISE_OR: return "bitwise or";
  case LEFT_SHIFT: return "left shift";
  case RIGHT_SHIFT: return "right shift";
  case ADDITION: return "addition";
  case SUBTRACTION: return "subtraction";
  case MULTIPLICATION: return "multiplication";
  case DIVISION: return "division";
  case REMINDER: return "reminder";
  case POSATE: return "posate";
  case NEGATE: return "negate";
  case POSTFIX_INCREMENT: return "postfix increment";
  case POSTFIX_DECREMENT: return "postfix decrement";
  case PREFIX_INCREMENT: return "prefix increment";
  case PREFIX_DECREMENT: return "prefix decrement";
  case LOGICAL_NOT: return "logical not";
  case COMPLEMENT: return "complement";
  case DECIMAL_LITERAL: return "decimal literal";
  case SYMBOL_ACCESS: return "symbol access";
  case GROUP: return "group";
  case FUNCTION_CALL: return "function call";
  default: unexpected("Unknown operator hash!");
  }
}
