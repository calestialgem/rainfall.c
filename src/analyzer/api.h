// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#pragma once

#include "parser/api.h"

#include <stdbool.h>
#include <stddef.h>

/* Variant of a type. */
typedef enum {
  /* Meta type. */
  TYPE_META,
  /* Void type. */
  TYPE_VOID,
  /* Boolean algebra integer type. */
  TYPE_BOOL,
  /* Smallest addressable integer type. */
  TYPE_BYTE,
  /* Signed, word-size integer type. */
  TYPE_INT,
  /* Unsigned, array element index and array size-size integer type. */
  TYPE_UXS,
  /* Single precision, floating-point real type. */
  TYPE_FLOAT,
  /* Double precision, floating-point real type. */
  TYPE_DOUBLE
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
  Type   asType;
  /* Value of bool type. */
  bool   asBool;
  /* Value of byte type. */
  char   asByte;
  /* Value of int type. */
  int    asInt;
  /* Value of uxs type. */
  size_t asUxs;
  /* Value of float type. */
  float  asFloat;
  /* Value of double type. */
  double asDouble;
} Value;

/* A value with its type and compile-time information. */
typedef struct {
  /* Type. */
  Type  type;
  /* Value if it is known at compile-time. */
  Value value;
  /* Whether the value is known at compile-time. */
  bool  known;
} Object;

/* Semantic evaluation of an expression node. */
typedef struct {
  /* Evaluated expression node. */
  ExpressionNode evaluated;
  /* Evaluated object. */
  Object         object;
} EvaluationNode;

/* Semantic evaluation of an expression. */
typedef struct {
  /* Pointer to the first node if it exists. */
  EvaluationNode* first;
  /* Pointer to one after the last node. */
  EvaluationNode* after;
  /* Pointer to one after the last allocated node. */
  EvaluationNode* bound;
} Evaluation;

/* An evaluation that is bound to a name. */
typedef struct {
  /* Evaluation that is bound. */
  Evaluation bound;
} Binding;

/* A mutable value holder. */
typedef struct {
  /* Evaluation of the variables initial value. Does not exist if the value is
   * defaulted. */
  Evaluation initial;
  /* Whether the variable is a defaulted. */
  bool       defaulted;
} Variable;

/* Variant of a symbol. */
typedef enum {
  /* Binding symbol. */
  SYMBOL_BINDING,
  /* Variable symbol. */
  SYMBOL_VARIABLE,
  /* Type symbol. */
  SYMBOL_TYPE
} SymbolTag;

/* A named semantic object. */
typedef struct {
  union {
    /* Symbol as binding. */
    Binding  asBinding;
    /* Symbol as variable. */
    Variable asVariable;
  };

  /* Variant of the symbol. */
  SymbolTag tag;

  /* Name. */
  String name;
  /* Object. */
  Object object;
  /* Whether the symbol is user-defined. */
  bool   userDefined;
} Symbol;

/* List of symbols. */
typedef struct {
  /* Pointer to the first symbol if it exists. */
  Symbol* first;
  /* Pointer to one after the last symbol. */
  Symbol* after;
  /* Pointer to one after the last allocated symbol. */
  Symbol* bound;
} Table;

/* Analyze the given parse. Reports to the given source. */
Table createTable(Source* reported, Parse analyzed);
/* Release the memory used by the given table. */
void  disposeTable(Table* disposed);
