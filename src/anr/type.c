// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"

/* Definition of a type without member data. */
#define type(t) \
  { .tag = t }

Type const TYP_INS_META = type(TYP_META);
Type const TYP_INS_I1   = type(TYP_I1);
Type const TYP_INS_I2   = type(TYP_I2);
Type const TYP_INS_I4   = type(TYP_I4);
Type const TYP_INS_I8   = type(TYP_I8);
Type const TYP_INS_IX   = type(TYP_IX);
Type const TYP_UNS_U1   = type(TYP_U1);
Type const TYP_UNS_U2   = type(TYP_U2);
Type const TYP_UNS_U4   = type(TYP_U4);
Type const TYP_UNS_U8   = type(TYP_U8);
Type const TYP_UNS_UX   = type(TYP_UX);
Type const TYP_FNS_F4   = type(TYP_F4);
Type const TYP_FNS_F8   = type(TYP_F8);
