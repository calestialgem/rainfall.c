// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"

#include <stdio.h>

/* Definition of a type without member data. */
#define type(t) \
  { .tag = t }

// Definitions are in macros because macros can be used inside array
// initializers as well.
#define tmeta   type(TYPE_META)
#define tvoid   type(TYPE_VOID)
#define tbool   type(TYPE_BOOL)
#define tbyte   type(TYPE_BYTE)
#define tint    type(TYPE_INT)
#define tptr    type(TYPE_IPTR)
#define tfloat  type(TYPE_FLOAT)
#define tdouble type(TYPE_DOUBLE)

Type const TYPE_INS_META   = tmeta;
Type const TYPE_INS_VOID   = tvoid;
Type const TYPE_INS_BOOL   = tbool;
Type const TYPE_INS_BYTE   = tbyte;
Type const TYPE_INS_INT    = tint;
Type const TYPE_INS_IPTR   = tptr;
Type const TYPE_INS_FLOAT  = tfloat;
Type const TYPE_INS_DOUBLE = tdouble;

Type const TYPE_BUILT[TYPE_BUILT_LEN]           = {tmeta, tvoid, tbool,  tbyte,
                                                   tint,  tptr,  tfloat, tdouble};
Type const TYPE_ARITHMETIC[TYPE_ARITHMETIC_LEN] = {tbool, tbyte,  tint,
                                                   tptr,  tfloat, tdouble};

bool typeEq(Type const lhs, Type const rhs) { return lhs.tag == rhs.tag; }

iptr typeRank(Type const type) {
  for (iptr i = 0; i < TYPE_ARITHMETIC_LEN; i++)
    if (typeEq(type, TYPE_ARITHMETIC[i])) return i;
  return -1;
}

char const* typeName(Type const type) {
  switch (type.tag) {
  case TYPE_META: return "type";
  case TYPE_VOID: return "void";
  case TYPE_BOOL: return "bool";
  case TYPE_BYTE: return "byte";
  case TYPE_INT: return "int";
  case TYPE_IPTR: return "iptr";
  case TYPE_FLOAT: return "float";
  case TYPE_DOUBLE: return "double";
  default: dbgUnexpected("Unknown type!");
  }
}

void typeWrite(Type const type, FILE* const stream) {
  fprintf(stream, "%s", typeName(type));
}

Value valDefault(Type const type) {
  switch (type.tag) {
  case TYPE_META: return (Value){.meta = TYPE_INS_VOID};
  case TYPE_VOID: return (Value){.vint = 0};
  case TYPE_BOOL: return (Value){.vbool = false};
  case TYPE_BYTE: return (Value){.byte = 0};
  case TYPE_INT: return (Value){.vint = 0};
  case TYPE_IPTR: return (Value){.iptr = 0};
  case TYPE_FLOAT: return (Value){.vfloat = 0};
  case TYPE_DOUBLE: return (Value){.vdouble = 0};
  default: dbgUnexpected("Unknown type!");
  }
}

void valWrite(Type const type, Value const val, FILE* const stream) {
  switch (type.tag) {
  case TYPE_META: typeWrite(val.meta, stream); return;
  case TYPE_VOID: return;
  case TYPE_BOOL: fprintf(stream, "%s", val.vbool ? "true" : "false"); return;
  case TYPE_BYTE: fprintf(stream, "%hhi", val.byte); return;
  case TYPE_INT: fprintf(stream, "%i", val.vint); return;
  case TYPE_IPTR: fprintf(stream, "%lli", val.iptr); return;
  case TYPE_FLOAT: fprintf(stream, "%g", val.vfloat); return;
  case TYPE_DOUBLE: fprintf(stream, "%g", val.vdouble); return;
  default: dbgUnexpected("Unknown type!");
  }
}
