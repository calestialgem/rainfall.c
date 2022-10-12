// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lxr/api.h"
#include "otc/api.h"
#include "utl/api.h"

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

/* Operators with two or more operators and one or more operands where the
 * total amount can vary. All operands are separated by an operator. */
typedef struct {
  /* Lexeme that comes after the first operand. */
  LexemeTag opening;
  /* Lexeme that comes between the intermediary operands. */
  LexemeTag separating;
  /* Lexeme that comes after the last operand. */
  LexemeTag closing;
} VariaryOperator;

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
  OPERATOR_VARIARY
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
  };

  /* Variant of the operator. */
  OperatorTag        tag;
  /* Precedence of the operator. */
  OperatorPrecedence precedence;
  /* Index of the operator in its own precedence level. */
  size_t             inLevel;
} Operator;

/* Number literal with decimal digits. */
extern Operator const DECIMAL_LITERAL;
/* Access to a symbol with its id. */
extern Operator const SYMBOL_ACCESS;
/* Expression grouped with parentheses. */
extern Operator const GROUP;
/* Call to a function. */
extern Operator const FUNCTION_CALL;

/* Posate. */
extern Operator const POSATE;
/* Negate. */
extern Operator const NEGATE;
/* Increment after returning. */
extern Operator const POSTFIX_INCREMENT;
/* Decrement after returning. */
extern Operator const POSTFIX_DECREMENT;
/* Increment before returning. */
extern Operator const PREFIX_INCREMENT;
/* Decrement before returning. */
extern Operator const PREFIX_DECREMENT;
/* Invert truthiness value. */
extern Operator const LOGICAL_NOT;
/* Invert bits. */
extern Operator const COMPLEMENT;

/* Multiply. */
extern Operator const MULTIPLICATION;
/* Divide. */
extern Operator const DIVISION;
/* Reminder after division. */
extern Operator const REMINDER;

/* Add. */
extern Operator const ADDITION;
/* Subtract. */
extern Operator const SUBTRACTION;

/* Shift bits to left. */
extern Operator const LEFT_SHIFT;
/* Shift bits to right. */
extern Operator const RIGHT_SHIFT;

/* Bitwise AND. */
extern Operator const BITWISE_AND;

/* Bitwise XOR */
extern Operator const BITWISE_XOR;

/* Bitwise OR. */
extern Operator const BITWISE_OR;

/* Whether the left is smaller than the right. */
extern Operator const SMALLER_THAN;
/* Whether the left is smaller than or equal to the right. */
extern Operator const SMALLER_THAN_OR_EQUAL_TO;
/* Whether the left is grater than the right. */
extern Operator const GREATER_THAN;
/* Whether the left is grater than or equal to the right. */
extern Operator const GREATER_THAN_OR_EQUAL_TO;

/* Whether the left is equal to right. */
extern Operator const EQUAL_TO;
/* Wherhet the left is not equal to right. */
extern Operator const NOT_EQUAL_TO;

/* Logical AND. */
extern Operator const LOGICAL_AND;

/* Logical OR. */
extern Operator const LOGICAL_OR;

/* Assignment. */
extern Operator const ASSIGNMENT;
/* Assignment after multiplication. */
extern Operator const MUTIPLICATION_ASSIGNMENT;
/* Assignment after division. */
extern Operator const DIVISION_ASSIGNMENT;
/* Assignment after reminder. */
extern Operator const REMINDER_ASSIGNMENT;
/* Assignment after addition. */
extern Operator const ADDITION_ASSIGNMENT;
/* Assignment after subtraction. */
extern Operator const SUBTRACTION_ASSIGNMENT;
/* Assignment after left shift. */
extern Operator const LEFT_SHIFT_ASSIGNMENT;
/* Assignment after right shift. */
extern Operator const RIGHT_SHIFT_ASSIGNMENT;
/* Assignment after bitwise and. */
extern Operator const BITWISE_AND_ASSIGNMENT;
/* Assignment after bitwise xor. */
extern Operator const BITWISE_XOR_ASSIGNMENT;
/* Assignment after bitwise or. */
extern Operator const BITWISE_OR_ASSIGNMENT;

/* Whether the given operators are the same. */
bool   compareOperatorEquality(Operator left, Operator right);
/* Hashcode of the given operator. */
size_t hashOperator(Operator hashed);
/* Amount of operators in the given precedence and given index in the precedence
 * level. */
size_t countInLevelOperators(OperatorPrecedence counted);
/* Operator at the given prececence and index. */
Operator
getOperatorAt(OperatorPrecedence gottenPrecedence, size_t gottenInLevelIndex);
/* Name of the given operator. */
char const* nameOperator(Operator named);

/* Instantiation of an operator. */
typedef struct {
  /* Operator. */
  Operator operator;
  /* Amount of operands, which are the nodes that come before this one in the
   * expression's array. */
  size_t   arity;
  /* Combined source section of all the lexemes of the expression. */
  String   section;
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
