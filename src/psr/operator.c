// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lxr/api.h"
#include "psr/api.h"
#include "utl/api.h"

/* Definition of a nullary operator with the given operator. */
#define null(lxm) \
  { .null = {.op = lxm}, .tag = OP_NULL }
/* Definition of a prenary operator with the given operator. */
#define pre(lxm) \
  { .pre = {.op = lxm}, .tag = OP_PRE }
/* Definition of a postary operator with the given operator. */
#define post(lxm) \
  { .post = {.op = lxm}, .tag = OP_POST }
/* Definition of a cirnary operator with the given operators. */
#define cir(llxm, rlxm) \
  { .cir = {.lop = llxm, .rop = rlxm}, .tag = OP_CIR }
/* Definition of a binary operator with the given operator. */
#define bin(lxm) \
  { .pre = {.op = lxm}, .tag = OP_BIN }
/* Definition of a variary operator with the given operators. */
#define var(llxm, slxm, rlxm) \
  { .var = {.lop = llxm, .sep = slxm, .rop = rlxm}, .tag = OP_VAR }

// Definitions are in macros because macros can be used inside array
// initializers as well.
#define dec null(LXM_DEC)
#define acs null(LXM_ID)
#define grp cir(LXM_OPAREN, LXM_CPAREN)

#define pos pre(LXM_PLUS)
#define neg pre(LXM_MINUS)

#define mul bin(LXM_STAR)
#define div bin(LXM_SLASH)
#define rem bin(LXM_PERCENT)

#define add bin(LXM_PLUS)
#define sub bin(LXM_MINUS)

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

Operator opOfNull(NullaryOperator const null) {
  return (Operator){.null = null, .tag = OP_NULL};
}

Operator opOfPre(PrenaryOperator const pre) {
  return (Operator){.pre = pre, .tag = OP_PRE};
}

Operator opOfPost(PostaryOperator const post) {
  return (Operator){.post = post, .tag = OP_POST};
}

Operator opOfCir(CirnaryOperator const cir) {
  return (Operator){.cir = cir, .tag = OP_CIR};
}

Operator opOfBin(BinaryOperator const bin) {
  return (Operator){.bin = bin, .tag = OP_BIN};
}

Operator opOfVar(VariaryOperator const var) {
  return (Operator){.var = var, .tag = OP_VAR};
}
