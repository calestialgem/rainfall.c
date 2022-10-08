// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#pragma once

#include "anr/api.h"
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

/* List of symbols. */
typedef struct {
  /* Pointer to the first symbol if it exists. */
  Symbol* bgn;
  /* Pointer to one after the last symbol. */
  Symbol* end;
  /* Pointer to one after the last allocated symbol. */
  Symbol* all;
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
/* Amount of arithmetic types. */
#define TYPE_ARITHMETIC_LEN 6
/* Types that could be used in arithmetic operations. */
extern Type const TYPE_ARITHMETIC[TYPE_ARITHMETIC_LEN];

/* Analyze the given parse. Reports to the given outcome. */
Table tblOf(Outcome* otc, Parse prs);
/* Release the memory resources used by the given table. */
void  tblFree(Table* tbl);
/* Stream out the given table as string to the given stream. */
void  tblWrite(Table tbl, FILE* stream);
