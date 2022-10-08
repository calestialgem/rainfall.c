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
  /* Boolean algebra integer type. */
  TYPE_BOOL,
  /* Smallest addressable integer type. */
  TYPE_BYTE,
  /* Signed, word integer type. */
  TYPE_INT,
  /* Signed, index and size integer type. */
  TYPE_IPTR,
  /* Single precision floating-point real type. */
  TYPE_FLOAT,
  /* Double precision floating-point real type. */
  TYPE_DOUBLE,
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
  /* Value of meta type. */
  Type   meta;
  /* Value of bool type. */
  bool   vbool;
  /* Value of byte type. */
  char   byte;
  /* Value of int type. */
  int    vint;
  /* Value of iptr type. */
  iptr   iptr;
  /* Value of float type. */
  float  vfloat;
  /* Value of double type. */
  double vdouble;
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
  iptr  ary;
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
/* Instance of bool type. */
extern Type const TYPE_INS_BOOL;
/* Instance of byte type. */
extern Type const TYPE_INS_BYTE;
/* Instance of int type. */
extern Type const TYPE_INS_INT;
/* Instance of iptr type. */
extern Type const TYPE_INS_IPTR;
/* Instance of float type. */
extern Type const TYPE_INS_FLOAT;
/* Instance of double type. */
extern Type const TYPE_INS_DOUBLE;

/* Amount of built-in types. */
#define TYPE_BUILT_LEN 8
/* Array of built-in types. */
extern Type const TYPE_BUILT[TYPE_BUILT_LEN];

/* Analyze the given parse. Reports to the given outcome. */
Table tblOf(Outcome* otc, Parse prs);
/* Release the memory resources used by the given table. */
void  tblFree(Table* tbl);
/* Stream out the given table as string to the given stream. */
void  tblWrite(Table tbl, FILE* stream);
