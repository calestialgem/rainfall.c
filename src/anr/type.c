// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"

#include <stdio.h>

/* Definition of a type without member data. */
#define type(t) \
  { .tag = t }

Type const TYPE_INS_META = type(TYPE_META);
Type const TYPE_INS_I1   = type(TYPE_I1);
Type const TYPE_INS_I2   = type(TYPE_I2);
Type const TYPE_INS_I4   = type(TYPE_I4);
Type const TYPE_INS_I8   = type(TYPE_I8);
Type const TYPE_INS_IX   = type(TYPE_IX);
Type const TYPE_UNS_U1   = type(TYPE_U1);
Type const TYPE_UNS_U2   = type(TYPE_U2);
Type const TYPE_UNS_U4   = type(TYPE_U4);
Type const TYPE_UNS_U8   = type(TYPE_U8);
Type const TYPE_UNS_UX   = type(TYPE_UX);
Type const TYPE_FNS_F4   = type(TYPE_F4);
Type const TYPE_FNS_F8   = type(TYPE_F8);

void typeWrite(Type const type, FILE* const stream) {
  switch (type.tag) {
  case TYPE_META: fprintf(stream, "type"); return;
  case TYPE_I1: fprintf(stream, "i1"); return;
  case TYPE_I2: fprintf(stream, "i2"); return;
  case TYPE_I4: fprintf(stream, "i4"); return;
  case TYPE_I8: fprintf(stream, "i8"); return;
  case TYPE_IX: fprintf(stream, "ix"); return;
  case TYPE_U1: fprintf(stream, "u1"); return;
  case TYPE_U2: fprintf(stream, "u2"); return;
  case TYPE_U4: fprintf(stream, "u4"); return;
  case TYPE_U8: fprintf(stream, "u8"); return;
  case TYPE_UX: fprintf(stream, "ux"); return;
  case TYPE_F4: fprintf(stream, "f4"); return;
  case TYPE_F8: fprintf(stream, "f8"); return;
  default: dbgUnexpected("Unknown type!");
  }
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
