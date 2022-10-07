// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#pragma once

#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdio.h>

/* Variant of type. */
typedef enum {
  /* Meta type. */
  TYPE_META,
  /* Void type. */
  TYPE_VOID,
  /* I1 type. */
  TYPE_I1,
  /* I2 type. */
  TYPE_I2,
  /* I4 type. */
  TYPE_I4,
  /* I8 type. */
  TYPE_I8,
  /* IX type. */
  TYPE_IX,
  /* U1 type. */
  TYPE_U1,
  /* U2 type. */
  TYPE_U2,
  /* U4 type. */
  TYPE_U4,
  /* U8 type. */
  TYPE_U8,
  /* UX type. */
  TYPE_UX,
  /* F4 type. */
  TYPE_F4,
  /* F8 type. */
  TYPE_F8
} TypeTag;

/* Meaning of a value. */
typedef struct {
  union {};

  /* Variant of the type. */
  TypeTag tag;
} Type;

/* Semantic rule of converting a type to another one. */
typedef struct {
  /* Source type. */
  Type to;
  /* Destination type. */
  Type from;
} TypeConversion;

/* Value. */
typedef union {
  /* Value of a meta type. */
  Type meta;
  /* Value of a i1 type. */
  i1   i1;
  /* Value of a i2 type. */
  i2   i2;
  /* Value of a i4 type. */
  i4   i4;
  /* Value of a i8 type. */
  i8   i8;
  /* Value of a ix type. */
  ix   ix;
  /* Value of a u1 type. */
  u1   u1;
  /* Value of a u2 type. */
  u2   u2;
  /* Value of a u4 type. */
  u4   u4;
  /* Value of a u8 type. */
  u8   u8;
  /* Value of a ux type. */
  ux   ux;
  /* Value of a f4 type. */
  f4   f4;
  /* Value of a f8 type. */
  f8   f8;
} Value;

/* Semantic representation of an expression node. */
typedef struct {
  /* Expression. */
  ExpressionNode exp;
  /* Type. */
  Type           type;
  /* Value if it is known at compile-time. */
  Value          val;
  /* Whether the value is known at compile-time. */
  bool           has;
} EvaluationNode;

/* Semantic representation of an expression. */
typedef struct {
  /* Pointer to the first node if it exists. */
  EvaluationNode* bgn;
  /* Pointer to one after the last node. */
  EvaluationNode* end;
  /* Pointer to one after the last allocated node. */
  EvaluationNode* all;
} Evaluation;

/* Semantic object with a name. */
typedef struct {
  /* Identifier. */
  String     name;
  /* Evaluation. */
  Evaluation evl;
} Symbol;

/* `Operation` with `NullaryOperator`. */
typedef struct {
} NullaryOperation;

/* `Operation` with `PrenaryOperator`. */
typedef struct {
  /* Type of the operand that comes after the operator. */
  Type in;
} PrenaryOperation;

/* `Operation` with `PostaryOperator`. */
typedef struct {
  /* Type of the operand that comes before the operator. */
  Type in;
} PostaryOperation;

/* `Operation` with `CirnaryOperator`. */
typedef struct {
  /* Type of the operand that comes between the operators. */
  Type in;
} CirnaryOperation;

/* `Operation` with `BinaryOperator`. */
typedef struct {
  /* Type of the operand that comes before the operator. */
  Type lin;
  /* Type of the operand that comes after the operator. */
  Type rin;
} BinaryOperation;

/* `Operation` with `VariaryOperator`. */
typedef struct {
  /* Type of the first operand. */
  Type  fin;
  /* Types of the remaining operands. */
  Type* rin;
  /* Amound of operands. */
  ux    ary;
} VariaryOperation;

/* Semantic rule of using an operator on types. */
typedef struct {
  union {
    /* Operation as nullary operation. */
    NullaryOperation null;
    /* Operation as prenary operation. */
    PrenaryOperation pre;
    /* Operation as postary operation. */
    PostaryOperation post;
    /* Operation as cirnary operation. */
    CirnaryOperation cir;
    /* Operation as binary operation. */
    BinaryOperation  bin;
    /* Operation as variary operation. */
    VariaryOperation var;
  };

  /* Operator. */
  Operator op;
  /* Type of the output of the operation. */
  Type     out;
} Operation;

/* List of symbols. */
typedef struct {
  /* Dynamicly allocated array of symbols. */
  struct {
    /* Pointer to the first symbol if it exists. */
    Symbol* bgn;
    /* Pointer to one after the last symbol. */
    Symbol* end;
    /* Pointer to one after the last allocated symbol. */
    Symbol* all;
  } sym;

  /* Dynamicly allocated array of operations. */
  struct {
    /* Pointer to the first operation if it exists. */
    Operation* bgn;
    /* Pointer to one after the last operation. */
    Operation* end;
    /* Pointer to one after the last allocated operation. */
    Operation* all;
  } opn;

  /* Dynamicly allocated array of type conversions. */
  struct {
    /* Pointer to the first conversion if it exists. */
    TypeConversion* bgn;
    /* Pointer to one after the last conversion. */
    TypeConversion* end;
    /* Pointer to one after the last allocated conversion. */
    TypeConversion* all;
  } cnv;
} Table;

/* Instance of meta type. */
extern Type const TYPE_INS_META;
/* Instance of void type. */
extern Type const TYPE_INS_VOID;
/* Instance of i1 type. */
extern Type const TYPE_INS_I1;
/* Instance of i2 type. */
extern Type const TYPE_INS_I2;
/* Instance of i4 type. */
extern Type const TYPE_INS_I4;
/* Instance of i8 type. */
extern Type const TYPE_INS_I8;
/* Instance of ix type. */
extern Type const TYPE_INS_IX;
/* Instance of u1 type. */
extern Type const TYPE_INS_U1;
/* Instance of u2 type. */
extern Type const TYPE_INS_U2;
/* Instance of u4 type. */
extern Type const TYPE_INS_U4;
/* Instance of u8 type. */
extern Type const TYPE_INS_U8;
/* Instance of ux type. */
extern Type const TYPE_INS_UX;
/* Instance of f4 type. */
extern Type const TYPE_INS_F4;
/* Instance of f8 type. */
extern Type const TYPE_INS_F8;

/* Amount of built-in types. */
#define TYPE_BUILT_LEN 14
/* Array of built-in types. */
extern Type const TYPE_BUILT[TYPE_BUILT_LEN];

/* Analyze the given parse. Reports to the given outcome. */
Table tblOf(Outcome* otc, Parse prs);
/* Release the memory resources used by the given table. */
void  tblFree(Table* tbl);
/* Stream out the given table as string to the given stream. */
void  tblWrite(Table tbl, FILE* stream);
