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
    .precedence = precedenceLevel, .inLevel = inLevelIndex               \
  }

/* Create a prenary operator with the given before lexeme. */
#define createPrenaryOperator(precedenceLevel, inLevelIndex, lexemeBefore) \
  {                                                                        \
    .asPrenary = {.before = lexemeBefore}, .tag = OPERATOR_PRENARY,        \
    .precedence = precedenceLevel, .inLevel = inLevelIndex                 \
  }

/* Create a postary operator with the given after lexeme. */
#define createPostaryOperator(precedenceLevel, inLevelIndex, lexemeAfter) \
  {                                                                       \
    .asPostary = {.after = lexemeAfter}, .tag = OPERATOR_POSTARY,         \
    .precedence = precedenceLevel, .inLevel = inLevelIndex                \
  }

/* Create a cirnary operator with the given opening and closing lexemes. */
#define createCirnaryOperator(                                         \
  precedenceLevel, inLevelIndex, openingLexeme, closingLexeme)         \
  {                                                                    \
    .asCirnary = {.opening = openingLexeme, .closing = closingLexeme}, \
    .tag = OPERATOR_CIRNARY, .precedence = precedenceLevel,            \
    .inLevel = inLevelIndex                                            \
  }

/* Create a binary operator with the given lexeme between. */
#define createBinaryOperator(precedenceLevel, inLevelIndex, lexemeBetween) \
  {                                                                        \
    .asBinary = {.between = lexemeBetween}, .tag = OPERATOR_BINARY,        \
    .precedence = precedenceLevel, .inLevel = inLevelIndex                 \
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
    .inLevel = inLevelIndex                                       \
  }

Operator const DECIMAL_LITERAL =
  createNullaryOperator(OPERATOR_PRIMARY, 0, LEXEME_DECIMAL);
Operator const SYMBOL_ACCESS =
  createNullaryOperator(OPERATOR_PRIMARY, 1, LEXEME_IDENTIFIER);
Operator const GROUP = createCirnaryOperator(
  OPERATOR_PRIMARY, 2, LEXEME_OPENING_PARENTHESIS, LEXEME_CLOSING_PARENTHESIS);
Operator const FUNCTION_CALL = createVariaryOperator(
  OPERATOR_PRIMARY, 3, LEXEME_OPENING_PARENTHESIS, LEXEME_COMMA,
  LEXEME_CLOSING_PARENTHESIS);

/* Amount of primary operators. */
#define PRIMARY_OPERATORS 4

Operator const POSATE = createPrenaryOperator(OPERATOR_UNARY, 0, LEXEME_PLUS);
Operator const NEGATE = createPrenaryOperator(OPERATOR_UNARY, 1, LEXEME_MINUS);
Operator const POSTFIX_INCREMENT =
  createPostaryOperator(OPERATOR_UNARY, 2, LEXEME_PLUS_PLUS);
Operator const POSTFIX_DECREMENT =
  createPostaryOperator(OPERATOR_UNARY, 3, LEXEME_MINUS_MINUS);
Operator const PREFIX_INCREMENT =
  createPrenaryOperator(OPERATOR_UNARY, 4, LEXEME_PLUS_PLUS);
Operator const PREFIX_DECREMENT =
  createPrenaryOperator(OPERATOR_UNARY, 5, LEXEME_MINUS_MINUS);
Operator const LOGICAL_NOT =
  createPrenaryOperator(OPERATOR_UNARY, 6, LEXEME_EXCLAMETION);
Operator const COMPLEMENT =
  createPrenaryOperator(OPERATOR_UNARY, 7, LEXEME_TILDE);

/* Amount of unary operators. */
#define UNARY_OPERATORS 8

Operator const MULTIPLICATION =
  createBinaryOperator(OPERATOR_FACTOR, 0, LEXEME_STAR);
Operator const DIVISION =
  createBinaryOperator(OPERATOR_FACTOR, 1, LEXEME_SLASH);
Operator const REMINDER =
  createBinaryOperator(OPERATOR_FACTOR, 2, LEXEME_PERCENT);

/* Amount of factor operators. */
#define FACTOR_OPERATORS 3

Operator const ADDITION = createBinaryOperator(OPERATOR_TERM, 0, LEXEME_PLUS);
Operator const SUBTRACTION =
  createBinaryOperator(OPERATOR_TERM, 1, LEXEME_MINUS);

/* Amount of term operators. */
#define TERM_OPERATORS 2

Operator const LEFT_SHIFT =
  createBinaryOperator(OPERATOR_SHIFT, 0, LEXEME_LEFT_ARROW_LEFT_ARROW);
Operator const RIGHT_SHIFT =
  createBinaryOperator(OPERATOR_SHIFT, 1, LEXEME_RIGHT_ARROW_RIGHT_ARROW);

/* Amount of shift operators. */
#define SHIFT_OPERATORS 2

Operator const BITWISE_AND =
  createBinaryOperator(OPERATOR_BITWISE_AND, 0, LEXEME_AMPERCENT);

/* Amount of bitwise AND operators. */
#define BITWISE_AND_OPERATORS 1

Operator const BITWISE_XOR =
  createBinaryOperator(OPERATOR_BITWISE_XOR, 0, LEXEME_CARET);

/* Amount of bitwise XOR operators. */
#define BITWISE_XOR_OPERATORS 1

Operator const BITWISE_OR =
  createBinaryOperator(OPERATOR_BITWISE_OR, 0, LEXEME_PIPE);

/* Amount of bitwise OR operators. */
#define BITWISE_OR_OPERATORS 1

Operator const SMALLER_THAN =
  createBinaryOperator(OPERATOR_ORDER_COMPARISON, 0, LEXEME_LEFT_ARROW);
Operator const SMALLER_THAN_OR_EQUAL_TO =
  createBinaryOperator(OPERATOR_ORDER_COMPARISON, 1, LEXEME_LEFT_ARROW_EQUAL);
Operator const GREATER_THAN =
  createBinaryOperator(OPERATOR_ORDER_COMPARISON, 2, LEXEME_LEFT_ARROW);
Operator const GREATER_THAN_OR_EQUAL_TO =
  createBinaryOperator(OPERATOR_ORDER_COMPARISON, 3, LEXEME_LEFT_ARROW_EQUAL);

/* Amount of order comparison operators. */
#define ORDER_COMPARISON_OPERATORS 4

Operator const EQUAL_TO =
  createBinaryOperator(OPERATOR_EQUALITY_COMPARISON, 0, LEXEME_EQUAL_EQUAL);
Operator const NOT_EQUAL_TO = createBinaryOperator(
  OPERATOR_EQUALITY_COMPARISON, 1, LEXEME_EXCLAMETION_EQUAL);

/* Amount of equality comparison operators. */
#define EQUALITY_COMPARISON_OPERATORS 2

Operator const LOGICAL_AND =
  createBinaryOperator(OPERATOR_LOGICAL_AND, 0, LEXEME_AMPERCENT_AMPERCENT);

/* Amount of logical AND operators. */
#define LOGICAL_AND_OPERATORS 1

Operator const LOGICAL_OR =
  createBinaryOperator(OPERATOR_LOGICAL_OR, 0, LEXEME_PIPE_PIPE);

/* Amount of logical OR operators. */
#define LOGICAL_OR_OPERATORS 1

Operator const ASSIGNMENT =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 0, LEXEME_EQUAL);
Operator const MUTIPLICATION_ASSIGNMENT =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 1, LEXEME_STAR_EQUAL);
Operator const DIVISION_ASSIGNMENT =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 2, LEXEME_SLASH_EQUAL);
Operator const REMINDER_ASSIGNMENT =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 3, LEXEME_PERCENT_EQUAL);
Operator const ADDITION_ASSIGNMENT =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 4, LEXEME_PLUS_EQUAL);
Operator const SUBTRACTION_ASSIGNMENT =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 5, LEXEME_MINUS_EQUAL);
Operator const LEFT_SHIFT_ASSIGNMENT = createBinaryOperator(
  OPERATOR_ASSIGNMENT, 6, LEXEME_LEFT_ARROW_LEFT_ARROW_EQUAL);
Operator const RIGHT_SHIFT_ASSIGNMENT = createBinaryOperator(
  OPERATOR_ASSIGNMENT, 7, LEXEME_RIGHT_ARROW_RIGHT_ARROW_EQUAL);
Operator const BITWISE_AND_ASSIGNMENT =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 8, LEXEME_AMPERCENT_EQUAL);
Operator const BITWISE_XOR_ASSIGNMENT =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 9, LEXEME_CARET_EQUAL);
Operator const BITWISE_OR_ASSIGNMENT =
  createBinaryOperator(OPERATOR_ASSIGNMENT, 10, LEXEME_PIPE_EQUAL);

/* Amount of assignment operators. */
#define ASSIGNMENT_OPERATORS 11

/* Maximum amount of operators in any level. */
#define MAX_LEVEL_COUNT ASSIGNMENT_OPERATORS

bool compareOperatorEquality(Operator left, Operator right) {
  return left.precedence == right.precedence && left.inLevel == right.inLevel;
}

size_t hashOperator(Operator hashed) {
  return (size_t)hashed.precedence * MAX_LEVEL_COUNT + hashed.inLevel;
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

Operator
getOperatorAt(OperatorPrecedence gottenPrecedence, size_t gottenInLevelIndex) {
  switch (gottenPrecedence) {
  case OPERATOR_ASSIGNMENT:
    switch (gottenInLevelIndex) {
    case 0: return ASSIGNMENT;
    case 1: return MUTIPLICATION_ASSIGNMENT;
    case 2: return DIVISION_ASSIGNMENT;
    case 3: return REMINDER_ASSIGNMENT;
    case 4: return ADDITION_ASSIGNMENT;
    case 5: return SUBTRACTION_ASSIGNMENT;
    case 6: return LEFT_SHIFT_ASSIGNMENT;
    case 7: return RIGHT_SHIFT_ASSIGNMENT;
    case 8: return BITWISE_AND_ASSIGNMENT;
    case 9: return BITWISE_XOR_ASSIGNMENT;
    case 10: return BITWISE_OR_ASSIGNMENT;
    default: unexpected("Unknown assignment level operator!");
    };
  case OPERATOR_LOGICAL_OR:
    switch (gottenInLevelIndex) {
    case 0: return LOGICAL_OR;
    default: unexpected("Unknown logical OR level operator!");
    };
  case OPERATOR_LOGICAL_AND:
    switch (gottenInLevelIndex) {
    case 0: return LOGICAL_AND;
    default: unexpected("Unknown logical AND level operator!");
    };
  case OPERATOR_EQUALITY_COMPARISON:
    switch (gottenInLevelIndex) {
    case 0: return EQUAL_TO;
    case 1: return NOT_EQUAL_TO;
    default: unexpected("Unknown equality comparison level operator!");
    };
  case OPERATOR_ORDER_COMPARISON:
    switch (gottenInLevelIndex) {
    case 0: return SMALLER_THAN;
    case 1: return SMALLER_THAN_OR_EQUAL_TO;
    case 2: return GREATER_THAN;
    case 3: return GREATER_THAN_OR_EQUAL_TO;
    default: unexpected("Unknown order comparison level operator!");
    };
  case OPERATOR_BITWISE_OR:
    switch (gottenInLevelIndex) {
    case 0: return BITWISE_OR;
    default: unexpected("Unknown bitwise OR level operator!");
    };
  case OPERATOR_BITWISE_XOR:
    switch (gottenInLevelIndex) {
    case 0: return BITWISE_XOR;
    default: unexpected("Unknown bitwise XOR level operator!");
    };
  case OPERATOR_BITWISE_AND:
    switch (gottenInLevelIndex) {
    case 0: return BITWISE_AND;
    default: unexpected("Unknown bitwise AND level operator!");
    };
  case OPERATOR_SHIFT:
    switch (gottenInLevelIndex) {
    case 0: return LEFT_SHIFT;
    case 1: return RIGHT_SHIFT;
    default: unexpected("Unknown shift level operator!");
    };
  case OPERATOR_TERM:
    switch (gottenInLevelIndex) {
    case 0: return ADDITION;
    case 1: return SUBTRACTION;
    default: unexpected("Unknown term level operator!");
    };
  case OPERATOR_FACTOR:
    switch (gottenInLevelIndex) {
    case 0: return MULTIPLICATION;
    case 1: return DIVISION;
    case 2: return REMINDER;
    default: unexpected("Unknown factor level operator!");
    };
  case OPERATOR_UNARY:
    switch (gottenInLevelIndex) {
    case 0: return POSATE;
    case 1: return NEGATE;
    case 2: return POSTFIX_INCREMENT;
    case 3: return POSTFIX_DECREMENT;
    case 4: return PREFIX_INCREMENT;
    case 5: return PREFIX_DECREMENT;
    case 6: return LOGICAL_NOT;
    case 7: return COMPLEMENT;
    default: unexpected("Unknown unary level operator!");
    };
  case OPERATOR_PRIMARY:
    switch (gottenInLevelIndex) {
    case 0: return DECIMAL_LITERAL;
    case 1: return SYMBOL_ACCESS;
    case 2: return GROUP;
    case 3: return FUNCTION_CALL;
    default: unexpected("Unknown primary level operator!");
    };
  default: unexpected("Unknown operator precedence!");
  }
}

char const* nameOperator(Operator named) {
  switch (named.precedence) {
  case OPERATOR_ASSIGNMENT:
    switch (named.inLevel) {
    case 0: return "assignment";
    case 1: return "multiplication assignment";
    case 2: return "division assignment";
    case 3: return "reminder assignment";
    case 4: return "addition assignment";
    case 5: return "subtraction assignment";
    case 6: return "left shift assignment";
    case 7: return "right shift assignment";
    case 8: return "bitwise and assignment";
    case 9: return "bitwise xor assignment";
    case 10: return "bitwise or assignment";
    default: unexpected("Unknown assignment level operator!");
    };
  case OPERATOR_LOGICAL_OR:
    switch (named.inLevel) {
    case 0: return "logical or";
    default: unexpected("Unknown logical OR level operator!");
    };
  case OPERATOR_LOGICAL_AND:
    switch (named.inLevel) {
    case 0: return "logical and";
    default: unexpected("Unknown logical AND level operator!");
    };
  case OPERATOR_EQUALITY_COMPARISON:
    switch (named.inLevel) {
    case 0: return "equal to";
    case 1: return "not equal to";
    default: unexpected("Unknown equality comparison level operator!");
    };
  case OPERATOR_ORDER_COMPARISON:
    switch (named.inLevel) {
    case 0: return "smaller than";
    case 1: return "smaller than or equal to";
    case 2: return "greater than";
    case 3: return "greater than or equal to";
    default: unexpected("Unknown order comparison level operator!");
    };
  case OPERATOR_BITWISE_OR:
    switch (named.inLevel) {
    case 0: return "bitwise or";
    default: unexpected("Unknown bitwise OR level operator!");
    };
  case OPERATOR_BITWISE_XOR:
    switch (named.inLevel) {
    case 0: return "bitwise xor";
    default: unexpected("Unknown bitwise XOR level operator!");
    };
  case OPERATOR_BITWISE_AND:
    switch (named.inLevel) {
    case 0: return "bitwise and";
    default: unexpected("Unknown bitwise AND level operator!");
    };
  case OPERATOR_SHIFT:
    switch (named.inLevel) {
    case 0: return "left shift";
    case 1: return "right shift";
    default: unexpected("Unknown shift level operator!");
    };
  case OPERATOR_TERM:
    switch (named.inLevel) {
    case 0: return "addition";
    case 1: return "subtraction";
    default: unexpected("Unknown term level operator!");
    };
  case OPERATOR_FACTOR:
    switch (named.inLevel) {
    case 0: return "multiplication";
    case 1: return "division";
    case 2: return "reminder";
    default: unexpected("Unknown factor level operator!");
    };
  case OPERATOR_UNARY:
    switch (named.inLevel) {
    case 0: return "posate";
    case 1: return "negate";
    case 2: return "postfix increment";
    case 3: return "postfix decrement";
    case 4: return "prefix increment";
    case 5: return "prefix decrement";
    case 6: return "logical not";
    case 7: return "complement";
    default: unexpected("Unknown unary level operator!");
    };
  case OPERATOR_PRIMARY:
    switch (named.inLevel) {
    case 0: return "decimal literal";
    case 1: return "symbol access";
    case 2: return "group";
    case 3: return "function call";
    default: unexpected("Unknown primary level operator!");
    };
  default: unexpected("Unknown operator precedence!");
  }
}
