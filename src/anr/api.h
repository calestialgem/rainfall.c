// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#pragma once

#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>

/* Type of a type symbol. */
typedef struct {
} Meta;

/* Signed, 1-byte integer. */
typedef struct {
} I1;

/* Signed, 2-byte integer. */
typedef struct {
} I2;

/* Signed, 4-byte integer. */
typedef struct {
} I4;

/* Signed, 8-byte integer. */
typedef struct {
} I8;

/* Signed, pointer-size integer. */
typedef struct {
} Ix;

/* Unsigned, 1-byte integer. */
typedef struct {
} U1;

/* Unsigned, 2-byte integer. */
typedef struct {
} U2;

/* Unsigned, 4-byte integer. */
typedef struct {
} U4;

/* Unsigned, 8-byte integer. */
typedef struct {
} U8;

/* Unsigned, pointer-size integer. */
typedef struct {
} Ux;

/* Floating-point, 4-byte real. */
typedef struct {
} F4;

/* Floating-point, 8-byte real. */
typedef struct {
} F8;

/* Variant of type. */
typedef enum {
  /* Meta type. */
  TYPE_META,
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
  union {
    /* Type as a meta. */
    Meta meta;
    /* Type as a i1. */
    I1   i1;
    /* Type as a i2. */
    I2   i2;
    /* Type as a i4. */
    I4   i4;
    /* Type as a i8. */
    I8   i8;
    /* Type as a ix. */
    Ix   ix;
    /* Type as a u1. */
    U1   u1;
    /* Type as a u2. */
    U2   u2;
    /* Type as a u4. */
    U4   u4;
    /* Type as a u8. */
    U8   u8;
    /* Type as a ux. */
    Ux   ux;
    /* Type as a f4. */
    F4   f4;
    /* Type as a f8. */
    F8   f8;
  };

  /* Variant of the type. */
  TypeTag tag;
} Type;

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
extern Type const TYPE_UNS_U1;
/* Instance of u2 type. */
extern Type const TYPE_UNS_U2;
/* Instance of u4 type. */
extern Type const TYPE_UNS_U4;
/* Instance of u8 type. */
extern Type const TYPE_UNS_U8;
/* Instance of ux type. */
extern Type const TYPE_UNS_UX;
/* Instance of f4 type. */
extern Type const TYPE_FNS_F4;
/* Instance of f8 type. */
extern Type const TYPE_FNS_F8;

/* Analyze the given parse. Reports to the given outcome. */
Table tblOf(Outcome* otc, Parse prs);
/* Release the memory resources used by the given table. */
void  tblFree(Table* tbl);
