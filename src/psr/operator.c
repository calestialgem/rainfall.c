// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
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
#define cll var(LXM_OPAREN, LXM_COMMA, LXM_CPAREN)
#define sin post(LXM_PLUSPLUS)
#define sde post(LXM_MINUSMINUS)
#define pin pre(LXM_PLUSPLUS)
#define pde pre(LXM_MINUSMINUS)

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
Operator const OP_CLL = cll;
Operator const OP_SIN = sin;
Operator const OP_SDE = sde;
Operator const OP_PIN = pin;
Operator const OP_PDE = pde;

Operator const OP_POS = pos;
Operator const OP_NEG = neg;

Operator const OP_MUL = mul;
Operator const OP_DIV = div;
Operator const OP_REM = rem;

Operator const OP_ADD = add;
Operator const OP_SUB = sub;

Operator const OP_PRIMARY[OP_PRIMARY_LEN] = {dec, acs, grp, cll,
                                             sin, sde, pin, pde};
Operator const OP_UNARY[OP_UNARY_LEN]     = {pos, neg};
Operator const OP_FACTOR[OP_FACTOR_LEN]   = {mul, div, rem};
Operator const OP_TERM[OP_TERM_LEN]       = {add, sub};

ux const OP_LEVEL_LEN[OP_ORDER_LEN] = {
  OP_TERM_LEN, OP_FACTOR_LEN, OP_UNARY_LEN, OP_PRIMARY_LEN};
Operator const* const OP_ORDER[OP_ORDER_LEN] = {
  OP_TERM, OP_FACTOR, OP_UNARY, OP_PRIMARY};

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

bool opEq(Operator const lhs, Operator const rhs) {
  if (lhs.tag != rhs.tag) return false;
  switch (lhs.tag) {
  case OP_NULL: return lhs.null.op == rhs.null.op;
  case OP_PRE: return lhs.pre.op == rhs.pre.op;
  case OP_POST: return lhs.post.op == rhs.post.op;
  case OP_CIR: return lhs.cir.lop == rhs.cir.lop;
  case OP_BIN: return lhs.bin.op == rhs.bin.op;
  case OP_VAR: return lhs.var.lop == rhs.var.lop;
  default: dbgUnexpected("Unknown operator tag!");
  }
}

char const* opName(Operator const op) {
  if (opEq(op, OP_DEC)) return "decimal literal";
  if (opEq(op, OP_ACS)) return "symbol access";
  if (opEq(op, OP_GRP)) return "group";
  if (opEq(op, OP_CLL)) return "function call";
  if (opEq(op, OP_SIN)) return "suffix increment";
  if (opEq(op, OP_SDE)) return "suffix decrement";
  if (opEq(op, OP_PIN)) return "prefix increment";
  if (opEq(op, OP_PDE)) return "prefix decrement";
  if (opEq(op, OP_POS)) return "posate";
  if (opEq(op, OP_NEG)) return "negate";
  if (opEq(op, OP_MUL)) return "multiply";
  if (opEq(op, OP_DIV)) return "divide";
  if (opEq(op, OP_REM)) return "reminder";
  if (opEq(op, OP_ADD)) return "add";
  if (opEq(op, OP_SUB)) return "subtract";
  dbgUnexpected("Unknown operator!");
}
