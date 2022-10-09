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

Arithmetic const ARI_INT = {.type = tint, .rank = 2};

bool typeEq(Type const lhs, Type const rhs) { return lhs.tag == rhs.tag; }

iptr typeRank(Type const type) {
  for (iptr i = 0; i < TYPE_ARITHMETIC_LEN; i++)
    if (typeEq(type, TYPE_ARITHMETIC[i])) return i;
  return -1;
}

bool typeCnv(Type const src, Type const des) {
  if (typeEq(src, des)) return true;
  Arithmetic const ariSrc = ariOf(src);
  Arithmetic const ariDes = ariOf(des);
  return ariValid(ariSrc) && ariValid(ariDes) && ariFits(ariSrc, ariDes);
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

Arithmetic ariOf(Type const type) {
  return (Arithmetic){.type = type, .rank = typeRank(type)};
}

bool ariValid(Arithmetic const ari) { return ari.rank != -1; }

bool ariInt(Arithmetic const ari) { return ari.rank < TYPE_ARITHMETIC_LEN - 2; }

bool ariFits(Arithmetic const src, Arithmetic const des) {
  return src.rank <= des.rank;
}

Arithmetic ariLarger(Arithmetic const lhs, Arithmetic const rhs) {
  return !ariFits(lhs, rhs) ? lhs : rhs;
}

Value ariCnv(Arithmetic const src, Arithmetic const des, Value const val) {
  switch (src.type.tag) {
  case TYPE_BOOL:
    switch (des.type.tag) {
    case TYPE_BOOL: return val;
    case TYPE_BYTE: return (Value){.byte = val.vbool};
    case TYPE_INT: return (Value){.vint = val.vbool};
    case TYPE_IPTR: return (Value){.iptr = val.vbool};
    case TYPE_FLOAT: return (Value){.vfloat = val.vbool};
    case TYPE_DOUBLE: return (Value){.vdouble = val.vbool};
    default: dbgUnexpected("Unknown arithmetic type!");
    }
    break;
  case TYPE_BYTE:
    switch (des.type.tag) {
    case TYPE_BYTE: return val;
    case TYPE_INT: return (Value){.vint = val.byte};
    case TYPE_IPTR: return (Value){.iptr = val.byte};
    case TYPE_FLOAT: return (Value){.vfloat = val.byte};
    case TYPE_DOUBLE: return (Value){.vdouble = val.byte};
    default: dbgUnexpected("Unknown arithmetic type!");
    }
    break;
  case TYPE_INT:
    switch (des.type.tag) {
    case TYPE_INT: return val;
    case TYPE_IPTR: return (Value){.iptr = val.vint};
    case TYPE_FLOAT: return (Value){.vfloat = val.vint};
    case TYPE_DOUBLE: return (Value){.vdouble = val.vint};
    default: dbgUnexpected("Unknown arithmetic type!");
    }
    break;
  case TYPE_IPTR:
    switch (des.type.tag) {
    case TYPE_IPTR: return val;
    case TYPE_FLOAT: return (Value){.vfloat = val.iptr};
    case TYPE_DOUBLE: return (Value){.vdouble = val.iptr};
    default: dbgUnexpected("Unknown arithmetic type!");
    }
    break;
  case TYPE_FLOAT:
    switch (des.type.tag) {
    case TYPE_FLOAT: return val;
    case TYPE_DOUBLE: return (Value){.vdouble = val.vfloat};
    default: dbgUnexpected("Unknown arithmetic type!");
    }
    break;
  case TYPE_DOUBLE:
    switch (des.type.tag) {
    case TYPE_DOUBLE: return val;
    default: dbgUnexpected("Unknown arithmetic type!");
    }
    break;
  default: dbgUnexpected("Unknown arithmetic type!");
  }
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

Value valCnv(Type src, Type des, Value val) {
  if (typeEq(src, des)) return val;
  Arithmetic const ariSrc = ariOf(src);
  Arithmetic const ariDes = ariOf(des);
  return ariCnv(ariSrc, ariDes, val);
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
