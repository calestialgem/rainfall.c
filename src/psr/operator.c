// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lxr/api.h"
#include "psr/api.h"
#include "utl/api.h"

#define dec \
  { .null = {.op = LXM_DEC}, .tag = OP_NULL }
#define acs \
  { .null = {.op = LXM_DEC}, .tag = OP_NULL }
#define grp \
  { .cir = {.lop = LXM_OPAREN, .rop = LXM_CPAREN}, .tag = OP_CIR }

#define pos \
  { .pre = {.op = LXM_PLUS}, .tag = OP_PRE }
#define neg \
  { .pre = {.op = LXM_MINUS}, .tag = OP_PRE }

#define mul \
  { .pre = {.op = LXM_STAR}, .tag = OP_BIN }
#define div \
  { .pre = {.op = LXM_SLASH}, .tag = OP_BIN }
#define rem \
  { .pre = {.op = LXM_PERCENT}, .tag = OP_BIN }

#define add \
  { .pre = {.op = LXM_PLUS}, .tag = OP_BIN }
#define sub \
  { .pre = {.op = LXM_MINUS}, .tag = OP_BIN }

Operator const OP_DEC = dec;
Operator const OP_ACS = acs;
Operator const OP_GRP = grp;

Operator const OP_POS = pos;
Operator const OP_NEG = neg;

Operator const OP_MUL = mul;
Operator const OP_DIV = div;
Operator const OP_REM = rem;

Operator const OP_ADD = add;
Operator const OP_SUB = sub;

Operator const OP_PRIMARY[OP_PRIMARY_LEN] = {dec, acs, grp};
Operator const OP_UNARY[OP_UNARY_LEN]     = {pos, neg};
Operator const OP_FACTOR[OP_FACTOR_LEN]   = {mul, div, rem};
Operator const OP_TERM[OP_TERM_LEN]       = {add, sub};

ux const OP_LEVEL_LEN[OP_ORDER_LEN] = {
  OP_PRIMARY_LEN, OP_UNARY_LEN, OP_FACTOR_LEN, OP_TERM_LEN};
Operator const* const OP_ORDER[OP_ORDER_LEN] = {
  OP_PRIMARY, OP_UNARY, OP_FACTOR, OP_TERM};
