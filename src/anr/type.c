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

#define tmeta type(TYPE_META)
#define ti1   type(TYPE_I1)
#define ti2   type(TYPE_I2)
#define ti4   type(TYPE_I4)
#define ti8   type(TYPE_I8)
#define tix   type(TYPE_IX)
#define tu1   type(TYPE_U1)
#define tu2   type(TYPE_U2)
#define tu4   type(TYPE_U4)
#define tu8   type(TYPE_U8)
#define tux   type(TYPE_UX)
#define tf4   type(TYPE_F4)
#define tf8   type(TYPE_F8)

Type const TYPE_INS_META = tmeta;
Type const TYPE_INS_I1   = ti1;
Type const TYPE_INS_I2   = ti2;
Type const TYPE_INS_I4   = ti4;
Type const TYPE_INS_I8   = ti8;
Type const TYPE_INS_IX   = tix;
Type const TYPE_INS_U1   = tu1;
Type const TYPE_INS_U2   = tu2;
Type const TYPE_INS_U4   = tu4;
Type const TYPE_INS_U8   = tu8;
Type const TYPE_INS_UX   = tux;
Type const TYPE_INS_F4   = tf4;
Type const TYPE_INS_F8   = tf8;

Type const TYPE_BUILT[TYPE_BUILT_LEN] = {tmeta, ti1, ti2, ti4, ti8, tix, tu1,
                                         tu2,   tu4, tu8, tux, tf4, tf8};

bool typeEq(Type const lhs, Type const rhs) { return lhs.tag == rhs.tag; }

bool typeSigned(Type const type) {
  return typeEq(type, TYPE_INS_I1) || typeEq(type, TYPE_INS_I2) ||
         typeEq(type, TYPE_INS_I4) || typeEq(type, TYPE_INS_I8) ||
         typeEq(type, TYPE_INS_IX);
}

bool typeUnsigned(Type const type) {
  return typeEq(type, TYPE_INS_U1) || typeEq(type, TYPE_INS_U2) ||
         typeEq(type, TYPE_INS_U4) || typeEq(type, TYPE_INS_U8) ||
         typeEq(type, TYPE_INS_UX);
}

bool typeFloat(Type const type) {
  return typeEq(type, TYPE_INS_F4) || typeEq(type, TYPE_INS_F8);
}

bool typeScalar(Type const type) {
  return typeSigned(type) || typeUnsigned(type) || typeFloat(type);
}

char const* typeName(Type const type) {
  switch (type.tag) {
  case TYPE_META: return "type";
  case TYPE_I1: return "i1";
  case TYPE_I2: return "i2";
  case TYPE_I4: return "i4";
  case TYPE_I8: return "i8";
  case TYPE_IX: return "ix";
  case TYPE_U1: return "u1";
  case TYPE_U2: return "u2";
  case TYPE_U4: return "u4";
  case TYPE_U8: return "u8";
  case TYPE_UX: return "ux";
  case TYPE_F4: return "f4";
  case TYPE_F8: return "f8";
  default: dbgUnexpected("Unknown type!");
  }
}

void typeWrite(Type const type, FILE* const stream) {
  fprintf(stream, "%s", typeName(type));
}

void valWrite(Type const type, Value const val, FILE* const stream) {
  switch (type.tag) {
  case TYPE_META: typeWrite(val.meta, stream); return;
  case TYPE_I1: fprintf(stream, "%hhi", val.i1); return;
  case TYPE_I2: fprintf(stream, "%hi", val.i2); return;
  case TYPE_I4: fprintf(stream, "%i", val.i4); return;
  case TYPE_I8: fprintf(stream, "%lli", val.i8); return;
  case TYPE_IX: fprintf(stream, "%lli", val.ix); return;
  case TYPE_U1: fprintf(stream, "%hhu", val.u1); return;
  case TYPE_U2: fprintf(stream, "%hu", val.u2); return;
  case TYPE_U4: fprintf(stream, "%u", val.u4); return;
  case TYPE_U8: fprintf(stream, "%llu", val.u8); return;
  case TYPE_UX: fprintf(stream, "%llu", val.ux); return;
  case TYPE_F4: fprintf(stream, "%f", val.f4); return;
  case TYPE_F8: fprintf(stream, "%f", val.f8); return;
  default: dbgUnexpected("Unknown type!");
  }
}
