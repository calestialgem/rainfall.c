// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lxr/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdio.h>

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
  /* Amount of operands, which are the nodes that come before this one in the
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

/* Changing the value of a mutable variable by applying a binary operator where
 * its on the left. */
typedef struct {
  /* Identifier that is the name of the assigned variable. */
  Lexeme     name;
  /* Operator. */
  Operator   op;
  /* Expression that gives the right hand side of the binary expression. */
  Expression rhs;
} CompoundAssignment;

/* Expression whose resulting value is discarded. These are calculated for the
 * side effects. */
typedef struct {
  /* Expression that is calculated, but discarded. */
  Expression exp;
} ExpressionStatement;

/* Type of a statement. */
typedef enum {
  /* Let definition statement. */
  STT_LET,
  /* Var definition statement. */
  STT_VAR,
  /* Assignment statement. */
  STT_ASS,
  /* Compound assignment statement. */
  STT_CAS,
  /* Expression statement. */
  STT_EXP
} StatementTag;

/* Directives that are given for the computer to execute. */
typedef struct {
  union {
    /* Statement as let definition statement. */
    LetDefinition       let;
    /* Statement as var definition statement. */
    VarDefinition       var;
    /* Statement as assignment statement. */
    Assignment          ass;
    /* Statement as compound assignment statement. */
    CompoundAssignment  cas;
    /* Statement as expression statement. */
    ExpressionStatement exp;
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

/* Number literal with decimal digits. */
extern Operator const OP_DEC;
/* Access to a symbol with its id. */
extern Operator const OP_ACS;
/* Expression grouped with parentheses. */
extern Operator const OP_GRP;
/* Call to a function. */
extern Operator const OP_CLL;

/* Posate. */
extern Operator const OP_POS;
/* Negate. */
extern Operator const OP_NEG;
/* Increment after returning. */
extern Operator const OP_SIN;
/* Decrement after returning. */
extern Operator const OP_SDE;
/* Increment before returning. */
extern Operator const OP_PIN;
/* Decrement before returning. */
extern Operator const OP_PDE;
/* Invert truthiness value. */
extern Operator const OP_NOT;
/* Invert bits. */
extern Operator const OP_BNT;

/* Multiply. */
extern Operator const OP_MUL;
/* Divide. */
extern Operator const OP_DIV;
/* Reminder after division. */
extern Operator const OP_REM;

/* Add. */
extern Operator const OP_ADD;
/* Subtract. */
extern Operator const OP_SUB;

/* Shift bits to left. */
extern Operator const OP_LSH;
/* Shift bits to right. */
extern Operator const OP_RSH;

/* Bitwise AND. */
extern Operator const OP_AND;
/* Bitwise XOR */
extern Operator const OP_XOR;
/* Bitwise OR. */
extern Operator const OP_ORR;

/* Short circuit AND. */
extern Operator const OP_SAN;
/* Short circuit OR. */
extern Operator const OP_SOR;

/* Whether the left is smaller than the right. */
extern Operator const OP_SMT;
/* Whether the left is smaller than or equal to the right. */
extern Operator const OP_STE;
/* Whether the left is larger than the right. */
extern Operator const OP_LGT;
/* Whether the left is larger than or equal to the right. */
extern Operator const OP_LTE;

/* Whether the left is equal to right. */
extern Operator const OP_EQU;
/* Wherhet the left is not equal to right. */
extern Operator const OP_NEQ;

/* Amount of primary operators. */
#define OP_PRIMARY_LEN 4
/* Operators that are the indivisible parts of an expression. */
extern Operator const OP_PRIMARY[OP_PRIMARY_LEN];

/* Amount of unary operators. */
#define OP_UNARY_LEN 8
/* Operators that are bound strongly to a single operand. */
extern Operator const OP_UNARY[OP_UNARY_LEN];

/* Amount of factor operators. */
#define OP_FACTOR_LEN 3
/* Operators between the factors in calculations. */
extern Operator const OP_FACTOR[OP_FACTOR_LEN];

/* Amount of term operators. */
#define OP_TERM_LEN 2
/* Operators between the terms in calculations. */
extern Operator const OP_TERM[OP_TERM_LEN];

/* Amount of shift operators. */
#define OP_SHIFT_LEN 2
/* Operators that shift bits. */
extern Operator const OP_SHIFT[OP_SHIFT_LEN];

/* Amount of bitwise operators. */
#define OP_BITWISE_LEN 3
/* Operators that compare bits. */
extern Operator const OP_BITWISE[OP_BITWISE_LEN];

/* Amount of short circuit operators. */
#define OP_SHORT_LEN 2
/* Operators that compare with short circuiting. */
extern Operator const OP_SHORT[OP_SHORT_LEN];

/* Amount of comparison operators. */
#define OP_COMPARISON_LEN 4
/* Operators that compare order. */
extern Operator const OP_COMPARISON[OP_COMPARISON_LEN];

/* Amount of equality operators. */
#define OP_EQUALITY_LEN 4
/* Operators that compare equality. */
extern Operator const OP_EQUALITY[OP_EQUALITY_LEN];

/* Amount of operator precedence levels. */
#define OP_ORDER_LEN 9
/* Amounts of operators in precedence levels. */
extern ux const              OP_LEVEL_LEN[OP_ORDER_LEN];
/* Precedence levels of operators from low to high. */
extern Operator const* const OP_ORDER[OP_ORDER_LEN];

/* Amount of binary operators that could be used in compound assignments. */
#define OP_COMPOUND_LEN 10
/* Binary operators that could be used in compound assignments. */
extern Operator const OP_COMPOUND[OP_COMPOUND_LEN];

/* Flatten the given nullary operator. */
Operator opOfNull(NullaryOperator null);
/* Flatten the given prenary operator. */
Operator opOfPre(PrenaryOperator pre);
/* Flatten the given postary operator. */
Operator opOfPost(PostaryOperator post);
/* Flatten the given cirnary operator. */
Operator opOfCir(CirnaryOperator cir);
/* Flatten the given binary operator. */
Operator opOfBin(BinaryOperator bin);
/* Flatten the given variary operator. */
Operator opOfVar(VariaryOperator var);
/* Whether the given operators are the same. */
bool     opEq(Operator lhs, Operator rhs);

/* Parse the given lex. Reports to the given outcome. */
Parse     prsOf(Outcome* otc, Lex lex);
/* Release the memory resources used by the given parse. */
void      prsFree(Parse* prs);
/* Amount of statements in the given parse. */
ux        prsLen(Parse prs);
/* Statement at the given index in the given parse. */
Statement prsAt(Parse prs, ux i);
/* Stream out the given parse as string to the given stream. */
void      prsWrite(Parse prs, FILE* stream);
