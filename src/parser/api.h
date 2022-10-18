// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lexer/api.h"
#include "source/api.h"
#include "utility/api.h"

#include <stdbool.h>
#include <stddef.h>

/* Operators without operands. */
typedef struct {
  /* Lexeme that is the only operator. */
  LexemeTag only;
} NullaryOperator;

/* Operators with a operator that comes before the only operand. */
typedef struct {
  /* Lexeme that comes before the operand. */
  LexemeTag before;
} PrenaryOperator;

/* Operators with a operator that comes after the only operand. */
typedef struct {
  /* Lexeme that comes after the operand. */
  LexemeTag after;
} PostaryOperator;

/* Operators with two operators that surround the only operand. */
typedef struct {
  /* Lexeme that comes before the operand. */
  LexemeTag opening;
  /* Lexeme that comes after the operand. */
  LexemeTag closing;
} CirnaryOperator;

/* Operators with an operator that comes between two operands. */
typedef struct {
  /* Lexeme that comes between the operands. */
  LexemeTag between;
} BinaryOperator;

/* Operators with any amount of operands that are surrounded and separated by
 * operators. */
typedef struct {
  /* Lexeme that comes before the first operand. */
  LexemeTag opening;
  /* Lexeme that comes between the intermediary operands. */
  LexemeTag separating;
  /* Lexeme that comes after the last operand. */
  LexemeTag closing;
} VariaryOperator;

/* `VariaryOperator` that comes after an operand. */
typedef struct {
  /* Lexeme that comes after the first operand. */
  LexemeTag opening;
  /* Lexeme that comes between the intermediary operands. */
  LexemeTag separating;
  /* Lexeme that comes after the last operand. */
  LexemeTag closing;
} MultaryOperator;

/* Variant of an opertor. */
typedef enum {
  /* Nullary operator. */
  OPERATOR_NULLARY,
  /* Prenary operator. */
  OPERATOR_PRENARY,
  /* Postary operator. */
  OPERATOR_POSTARY,
  /* Cirnary operator. */
  OPERATOR_CIRNARY,
  /* Binary operator. */
  OPERATOR_BINARY,
  /* Variary operator. */
  OPERATOR_VARIARY,
  /* Multary operator. */
  OPERATOR_MULTARY
} OperatorTag;

/* Levels of operators that decide which operator can have operands of which.
 * Higher level operators bind thighter; thus, they cannot have operands that
 * are formed of lower level operators. This is not the case for cirnary
 * operators because they have clear start and end, and for the operands of the
 * variary operators after the first one, those also have a clear start,
 * separator and end. */
typedef enum {
  /* Operators that change a variable. */
  OPERATOR_ASSIGNMENT,
  /* Operators that OR Booleans and short circuit. */
  OPERATOR_LOGICAL_OR,
  /* Operators that AND Booleans and short circuit. */
  OPERATOR_LOGICAL_AND,
  /* Operators that compare equality. */
  OPERATOR_EQUALITY_COMPARISON,
  /* Operators that compare order. */
  OPERATOR_ORDER_COMPARISON,
  /* Operators that OR the bits of an integer. */
  OPERATOR_BITWISE_OR,
  /* Operators that XOR the bits of an integer. */
  OPERATOR_BITWISE_XOR,
  /* Operators that AND the bits of an integer. */
  OPERATOR_BITWISE_AND,
  /* Operators that shift the bits of an integer. */
  OPERATOR_SHIFT,
  /* Operators that combine the terms in a calculation. */
  OPERATOR_TERM,
  /* Operators that combine the factors in a calculation. */
  OPERATOR_FACTOR,
  /* Operators that bound thightly to a single operand. */
  OPERATOR_UNARY,
  /* Operators that are indivisable building blocks of expressions. */
  OPERATOR_PRIMARY,
  /* Operators that list expressions. */
  OPERATOR_LIST,
  /* Amount of operator precedence levels. */
  OPERATOR_LEVELS
} OperatorPrecedence;

/* Rules of an operation that result in calculation of a value. */
typedef struct {
  union {
    /* Operator as nullary operator. */
    NullaryOperator asNullary;
    /* Operator as prenary operator. */
    PrenaryOperator asPrenary;
    /* Operator as postary operator. */
    PostaryOperator asPostary;
    /* Operator as cirnary operator. */
    CirnaryOperator asCirnary;
    /* Operator as binary operator. */
    BinaryOperator  asBinary;
    /* Operator as variary operator. */
    VariaryOperator asVariary;
    /* Operator as multary operator. */
    MultaryOperator asMultary;
  };

  /* Variant of the operator. */
  OperatorTag        tag;
  /* Precedence of the operator. */
  OperatorPrecedence precedence;
  /* Index of the operator in its own precedence level. */
  size_t             inLevel;
  /* Hashcode of the operator. */
  size_t             hash;
} Operator;

/* Maximum amount of operators in any level. */
#define MAX_OPERATOR_LEVEL_COUNT 11
/* Hashcode of an operator with the given precedence and index. */
#define hashOperator(precedenceLevel, inLevelIndex) \
  ((size_t)(precedenceLevel)*MAX_OPERATOR_LEVEL_COUNT + (inLevelIndex))

/* Comma separated list. */
#define LIST hashOperator(OPERATOR_LIST, 0)

/* Number literal with decimal digits. */
#define DECIMAL_LITERAL hashOperator(OPERATOR_PRIMARY, 0)
/* Access to a symbol with its id. */
#define SYMBOL_ACCESS   hashOperator(OPERATOR_PRIMARY, 1)
/* Expression grouped with parentheses. */
#define GROUP           hashOperator(OPERATOR_PRIMARY, 2)
/* Call to a function. */
#define FUNCTION_CALL   hashOperator(OPERATOR_PRIMARY, 3)
/* Function arrow. */
#define FUNCTION_ARROW  hashOperator(OPERATOR_PRIMARY, 4)

/* Posate. */
#define POSATE            hashOperator(OPERATOR_UNARY, 0)
/* Negate. */
#define NEGATE            hashOperator(OPERATOR_UNARY, 1)
/* Increment after returning. */
#define POSTFIX_INCREMENT hashOperator(OPERATOR_UNARY, 2)
/* Decrement after returning. */
#define POSTFIX_DECREMENT hashOperator(OPERATOR_UNARY, 3)
/* Increment before returning. */
#define PREFIX_INCREMENT  hashOperator(OPERATOR_UNARY, 4)
/* Decrement before returning. */
#define PREFIX_DECREMENT  hashOperator(OPERATOR_UNARY, 5)
/* Invert truthiness value. */
#define LOGICAL_NOT       hashOperator(OPERATOR_UNARY, 6)
/* Invert bits. */
#define COMPLEMENT        hashOperator(OPERATOR_UNARY, 7)

/* Multiply. */
#define MULTIPLICATION hashOperator(OPERATOR_FACTOR, 0)
/* Divide. */
#define DIVISION       hashOperator(OPERATOR_FACTOR, 1)
/* Reminder after division. */
#define REMINDER       hashOperator(OPERATOR_FACTOR, 2)

/* Add. */
#define ADDITION    hashOperator(OPERATOR_TERM, 0)
/* Subtract. */
#define SUBTRACTION hashOperator(OPERATOR_TERM, 1)

/* Shift bits to left. */
#define LEFT_SHIFT  hashOperator(OPERATOR_SHIFT, 0)
/* Shift bits to right. */
#define RIGHT_SHIFT hashOperator(OPERATOR_SHIFT, 1)

/* Bitwise AND. */
#define BITWISE_AND hashOperator(OPERATOR_BITWISE_AND, 0)

/* Bitwise XOR */
#define BITWISE_XOR hashOperator(OPERATOR_BITWISE_XOR, 0)

/* Bitwise OR. */
#define BITWISE_OR hashOperator(OPERATOR_BITWISE_OR, 0)

/* Whether the left is smaller than the right. */
#define SMALLER_THAN             hashOperator(OPERATOR_ORDER_COMPARISON, 0)
/* Whether the left is smaller than or equal to the right. */
#define SMALLER_THAN_OR_EQUAL_TO hashOperator(OPERATOR_ORDER_COMPARISON, 1)
/* Whether the left is grater than the right. */
#define GREATER_THAN             hashOperator(OPERATOR_ORDER_COMPARISON, 2)
/* Whether the left is grater than or equal to the right. */
#define GREATER_THAN_OR_EQUAL_TO hashOperator(OPERATOR_ORDER_COMPARISON, 3)

/* Whether the left is equal to right. */
#define EQUAL_TO     hashOperator(OPERATOR_EQUALITY_COMPARISON, 0)
/* Wherhet the left is not equal to right. */
#define NOT_EQUAL_TO hashOperator(OPERATOR_EQUALITY_COMPARISON, 1)

/* Logical AND. */
#define LOGICAL_AND hashOperator(OPERATOR_LOGICAL_AND, 0)

/* Logical OR. */
#define LOGICAL_OR hashOperator(OPERATOR_LOGICAL_OR, 0)

/* Assignment. */
#define ASSIGNMENT                hashOperator(OPERATOR_ASSIGNMENT, 0)
/* Assignment after multiplication. */
#define MULTIPLICATION_ASSIGNMENT hashOperator(OPERATOR_ASSIGNMENT, 1)
/* Assignment after division. */
#define DIVISION_ASSIGNMENT       hashOperator(OPERATOR_ASSIGNMENT, 2)
/* Assignment after reminder. */
#define REMINDER_ASSIGNMENT       hashOperator(OPERATOR_ASSIGNMENT, 3)
/* Assignment after addition. */
#define ADDITION_ASSIGNMENT       hashOperator(OPERATOR_ASSIGNMENT, 4)
/* Assignment after subtraction. */
#define SUBTRACTION_ASSIGNMENT    hashOperator(OPERATOR_ASSIGNMENT, 5)
/* Assignment after left shift. */
#define LEFT_SHIFT_ASSIGNMENT     hashOperator(OPERATOR_ASSIGNMENT, 6)
/* Assignment after right shift. */
#define RIGHT_SHIFT_ASSIGNMENT    hashOperator(OPERATOR_ASSIGNMENT, 7)
/* Assignment after bitwise and. */
#define BITWISE_AND_ASSIGNMENT    hashOperator(OPERATOR_ASSIGNMENT, 8)
/* Assignment after bitwise xor. */
#define BITWISE_XOR_ASSIGNMENT    hashOperator(OPERATOR_ASSIGNMENT, 9)
/* Assignment after bitwise or. */
#define BITWISE_OR_ASSIGNMENT     hashOperator(OPERATOR_ASSIGNMENT, 10)

/* Whether the given operators are the same. */
bool     compareOperatorEquality(Operator left, Operator right);
/* Amount of operators in the given precedence and given index in the precedence
 * level. */
size_t   countInLevelOperators(OperatorPrecedence counted);
/* Operator with the given hash. */
Operator getOperator(size_t gottenHash);
/* Operator at the given prececence and index. */
Operator
getOperatorAt(OperatorPrecedence gottenPrecedence, size_t gottenInLevelIndex);
/* Name of the given operator. */
char const* nameOperator(Operator named);

/* Instantiation of an operator. */
typedef struct {
  /* Hash of the instantiated operator. */
  size_t operator;
  /* Amount of operands, which are the nodes that come before this one in the
   * expression's array. */
  size_t arity;
  /* Combined source section of all the lexemes of the expression. */
  String section;
} ExpressionNode;

/* Operations that result in calculation of a value. */
typedef struct {
  /* Pointer to the first node if it exists. */
  ExpressionNode* first;
  /* Pointer to one after the last node. */
  ExpressionNode* after;
  /* Pointer to one after the last allocated node. */
  ExpressionNode* bound;
} Expression;

/* Amount of nodes in the given expression. */
size_t countExpressionNodes(Expression counted);
/* Section of the source file that has the given expression. */
String getExpressionSection(Expression gotten);

/* Creation of an immutable binding with its type and value. */
typedef struct {
  /* Identifier of the defined binding. */
  String     name;
  /* Expression that gives the type of the defined binding. */
  Expression type;
  /* Expression that gives the value of the defined binding. */
  Expression value;
} BindingDefinition;

/* Binding definition with the type not provided. The type is inferred from the
 * bound value. */
typedef struct {
  /* Identifier of the defined binding. */
  String     name;
  /* Expression that gives the value of the defined binding. */
  Expression value;
} InferredBindingDefinition;

/* Creation of a mutable vairable with its type and initial value. */
typedef struct {
  /* Identifier of the defined variable. */
  String     name;
  /* Expression that gives the type of the defined variable. */
  Expression type;
  /* Expression that gives the initial value of the defined variable. */
  Expression initialValue;
} VariableDefinition;

/* Variable definition with the type not provided. The type is inferred from the
 * initial value. */
typedef struct {
  /* Identifier of the defined variable. */
  String     name;
  /* Expression that gives the initial value of the defined variable. */
  Expression initialValue;
} InferredVariableDefinition;

/* Variable definition with the initial value not provided. The initial value
 * becomes the default value of the type. */
typedef struct {
  /* Identifier of the defined variable. */
  String     name;
  /* Expression that gives the type of the defined variable. */
  Expression type;
} DefaultedVariableDefinition;

/* Expression whose resulting value is discarded. These are calculated for the
 * side effects. */
typedef struct {
  /* Expression that is calculated, but discarded. */
  Expression discarded;
} DiscardedExpression;

/* Variant of a statement. */
typedef enum {
  /* Binding definition statement. */
  STATEMENT_BINDING_DEFINITION,
  /* Inferred binding definition statement. */
  STATEMENT_INFERRED_BINDING_DEFINITION,
  /* Variable definition statement. */
  STATEMENT_VARIABLE_DEFINITION,
  /* Inferred variable definition statement. */
  STATEMENT_INFERRED_VARIABLE_DEFINITION,
  /* Defaulted variable definition statement. */
  STATEMENT_DEFAULTED_VARIABLE_DEFINITION,
  /* Discarded expression statement. */
  STATEMENT_DISCARDED_EXPRESSION
} StatementTag;

/* Directives that are given for the computer to execute. */
typedef struct {
  union {
    /* Statement as binding definition. */
    BindingDefinition           asBindingDefinition;
    /* Statement as inferred binding definition. */
    InferredBindingDefinition   asInferredBindingDefinition;
    /* Statement as variable definition. */
    VariableDefinition          asVariableDefinition;
    /* Statement as inferred variable definition. */
    InferredVariableDefinition  asInferredVariableDefinition;
    /* Statement as defaulted variable definition. */
    DefaultedVariableDefinition asDefaultedVariableDefinition;
    /* Statement as discarded expression. */
    DiscardedExpression         asDiscardedExpression;
  };

  /* Variant of the statement. */
  StatementTag tag;
} Statement;

/* Result of parsing a lex. */
typedef struct {
  /* Pointer to the first statement if it exists. */
  Statement* first;
  /* Pointer to one after the last statement. */
  Statement* after;
  /* Pointer to one after the last allocated statement. */
  Statement* bound;
} Parse;

/* Parse the given lex. Reports to the given outcome. */
Parse createParse(Source* reported, Lex parsed);
/* Release the memory resources used by the given parse. */
void  disposeParse(Parse* disposed);
