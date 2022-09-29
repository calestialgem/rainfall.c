// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lxr/api.h"
#include "utl/api.h"

#include <stdbool.h>

/* Operators without operands. */
typedef struct {
  /* Lexeme that is the only operator. */
  LexemeType op;
} NullaryOperator;

/* Operators with a operator that comes before the only operand. */
typedef struct {
  /* Lexeme that comes before the operand. */
  LexemeType op;
} PrenaryOperator;

/* Operators with a operator that comes after the only operand. */
typedef struct {
  /* Lexeme that comes after the operand. */
  LexemeType op;
} PostaryOperator;

/* Operators with two operators that surround the only operand. */
typedef struct {
  /* Lexeme that comes before the operand. */
  LexemeType lop;
  /* Lexeme that comes after the operand. */
  LexemeType rop;
} CirnaryOperator;

/* Operators with an operator that comes between two operands. */
typedef struct {
  /* Lexeme that comes between the operands. */
  LexemeType op;
} BinaryOperator;

/* Operators with two or more operators and one or more operands where the
 * total amount can vary. All operands are separated by an operator. */
typedef struct {
  /* Lexeme that comes after the first operand. */
  LexemeType lop;
  /* Lexeme that comes between the intermediary operands. */
  LexemeType sep;
  /* Lexeme that comes after the last operand. */
  LexemeType rop;
} VariaryOperator;

/* Type of an opertor. */
typedef enum {
  /* Nullary operator. */
  OP_NULL,
  /* Prenary operator. */
  OP_PRE,
  /* Postary operator. */
  OP_POST,
  /* Cirnary operator. */
  OP_CIR,
  /* Binary operator. */
  OP_BIN,
  /* Variary operator. */
  OP_VAR
} OperatorTag;

/* Rules of an operation that result in calculation of a value. */
typedef struct {
  union {
    /* Operator as nullary operator. */
    NullaryOperator null;
    /* Operator as prenary operator. */
    PrenaryOperator pre;
    /* Operator as postary operator. */
    PostaryOperator post;
    /* Operator as cirnary operator. */
    CirnaryOperator cir;
    /* Operator as binary operator. */
    BinaryOperator  bin;
    /* Operator as variary operator. */
    VariaryOperator var;
  };

  /* Type of the operator. */
  OperatorTag tag;
} Operator;

/* Instantiation of an operator. */
typedef struct {
  /* Operator. */
  Operator op;
  /* Amount of operands, which are the nodes that come after this one in the
   * expression's array. */
  ux       ary;
  /* Combined value of all the lexemes of the expression. */
  String   val;
} ExpressionNode;

/* Operations that result in calculation of a value. */
typedef struct {
  /* Pointer to the first node if it exists. */
  ExpressionNode* bgn;
  /* Pointer to one after the last node. */
  ExpressionNode* end;
  /* Pointer to one after the last allocated node. */
  ExpressionNode* all;
} Expression;

/* Creation of an immutable variable with its type and value. */
typedef struct {
  /* Identifier that is the name of the defined variable. */
  Lexeme     name;
  /* Expression that gives the type of the defined variable. */
  Expression type;
  /* Expression that gives the value of the defined variable. */
  Expression val;
} LetDefinition;

/* Creation of a mutable vairable with its type and initial value. */
typedef struct {
  /* Identifier that is the name of the defined variable. */
  Lexeme     name;
  /* Expression that gives the type of the defined variable. */
  Expression type;
  /* Expression that gives the initial value of the defined variable. */
  Expression val;
} VarDefinition;

/* Changing the value of a mutable variable. */
typedef struct {
  /* Identifier that is the name of the assigned variable. */
  Lexeme     name;
  /* Expression that gives the assigned value. */
  Expression val;
} Assignment;

/* Type of a statement. */
typedef enum {
  /* Let definition statement. */
  STT_LET,
  /* Var definition statement. */
  STT_VAR,
  /* Assignment statement. */
  STT_ASS
} StatementTag;

/* Directives that are given for the computer to execute. */
typedef struct {
  union {
    /* Statement as let definition statement. */
    LetDefinition let;
    /* Statement as var definition statement. */
    VarDefinition var;
    /* Statement as assignment statement. */
    Assignment    ass;
  };

  /* Type of the statement. */
  StatementTag tag;
} Statement;

/* Result of parsing a lex. */
typedef struct {
  /* Pointer to the first statement if it exists. */
  Statement* bgn;
  /* Pointer to one after the last statement. */
  Statement* end;
  /* Pointer to one after the last allocated statement. */
  Statement* all;
} Parse;
