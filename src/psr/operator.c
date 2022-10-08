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
#define not pre(LXM_EXC)
#define bnt pre(LXM_TILDE)

#define pos pre(LXM_PLUS)
#define neg pre(LXM_MINUS)

#define mul bin(LXM_STAR)
#define div bin(LXM_SLASH)
#define rem bin(LXM_PERCENT)

#define add bin(LXM_PLUS)
#define sub bin(LXM_MINUS)

#define lsh bin(LXM_LARLAR)
#define rsh bin(LXM_RARRAR)

#define and bin(LXM_AMP)

#define xor bin(LXM_CARET)

#define orr bin(LXM_PIPE)

#define smt bin(LXM_LARROW)
#define ste bin(LXM_LAREQ)
#define lgt bin(LXM_RARROW)
#define lte bin(LXM_RAREQ)

#define equ bin(LXM_EQEQ)
#define neq bin(LXM_EXCEQ)

#define lan bin(LXM_AMPAMP)

#define lor bin(LXM_PIPEPIPE)

#define ass bin(LXM_EQUAL)
#define mla bin(LXM_STAREQ)
#define dva bin(LXM_SLASHEQ)
#define rma bin(LXM_PERCEQ)
#define ada bin(LXM_PLUSEQ)
#define sba bin(LXM_MINUSEQ)
#define lsa bin(LXM_LARLAREQ)
#define rsa bin(LXM_RARRAREQ)
#define ana bin(LXM_AMPEQ)
#define xra bin(LXM_CARETEQ)
#define ora bin(LXM_PIPEEQ)

Operator const OP_DEC = dec;
Operator const OP_ACS = acs;
Operator const OP_GRP = grp;
Operator const OP_CLL = cll;

Operator const OP_POS = pos;
Operator const OP_NEG = neg;
Operator const OP_SIN = sin;
Operator const OP_SDE = sde;
Operator const OP_PIN = pin;
Operator const OP_PDE = pde;
Operator const OP_NOT = not ;
Operator const OP_BNT = bnt;

Operator const OP_MUL = mul;
Operator const OP_DIV = div;
Operator const OP_REM = rem;

Operator const OP_ADD = add;
Operator const OP_SUB = sub;

Operator const OP_LSH = lsh;
Operator const OP_RSH = rsh;

Operator const OP_AND = and;

Operator const OP_XOR = xor;

Operator const OP_ORR = orr;

Operator const OP_SMT = smt;
Operator const OP_STE = ste;
Operator const OP_LGT = lgt;
Operator const OP_LTE = lte;

Operator const OP_EQU = equ;
Operator const OP_NEQ = neq;

Operator const OP_LAN = lan;

Operator const OP_LOR = lor;

Operator const OP_ASS = ass;
Operator const OP_MLA = mla;
Operator const OP_DVA = dva;
Operator const OP_RMA = rma;
Operator const OP_ADA = ada;
Operator const OP_SBA = sba;
Operator const OP_LSA = lsa;
Operator const OP_RSA = rsa;
Operator const OP_ANA = ana;
Operator const OP_XRA = xra;
Operator const OP_ORA = ora;

Operator const OP_PRIMARY[OP_PRIMARY_LEN]       = {dec, acs, grp, cll};
Operator const OP_UNARY[OP_UNARY_LEN]           = {pos, neg, sin, sde,
                                                   pin, pde, not, bnt};
Operator const OP_FACTOR[OP_FACTOR_LEN]         = {mul, div, rem};
Operator const OP_TERM[OP_TERM_LEN]             = {add, sub};
Operator const OP_SHIFT[OP_SHIFT_LEN]           = {lsh, rsh};
Operator const OP_BITAND[OP_BITAND_LEN]         = {and};
Operator const OP_BITXOR[OP_BITXOR_LEN]         = {xor};
Operator const OP_BITOR[OP_BITOR_LEN]           = {orr};
Operator const OP_COMPARISON[OP_COMPARISON_LEN] = {smt, ste, lgt, lte};
Operator const OP_EQUALITY[OP_EQUALITY_LEN]     = {equ, neq};
Operator const OP_LOGAND[OP_LOGAND_LEN]         = {lan};
Operator const OP_LOGOR[OP_LOGOR_LEN]           = {lor};
Operator const OP_ASSIGNMENT[OP_ASSIGNMENT_LEN] = {ass, mla, dva, rma, ada, sba,
                                                   lsa, rsa, ana, xra, ora};

iptr const OP_LEVEL_LEN[OP_ORDER_LEN] = {
  OP_ASSIGNMENT_LEN, OP_LOGOR_LEN, OP_LOGAND_LEN, OP_EQUALITY_LEN,
  OP_COMPARISON_LEN, OP_BITOR_LEN, OP_BITXOR_LEN, OP_BITAND_LEN,
  OP_SHIFT_LEN,      OP_TERM_LEN,  OP_FACTOR_LEN, OP_UNARY_LEN,
  OP_PRIMARY_LEN};
Operator const* const OP_ORDER[OP_ORDER_LEN] = {
  OP_ASSIGNMENT, OP_LOGOR,  OP_LOGAND, OP_EQUALITY, OP_COMPARISON,
  OP_BITOR,      OP_BITXOR, OP_BITAND, OP_SHIFT,    OP_TERM,
  OP_FACTOR,     OP_UNARY,  OP_PRIMARY};

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
  if (opEq(op, OP_POS)) return "posate";
  if (opEq(op, OP_NEG)) return "negate";
  if (opEq(op, OP_SIN)) return "suffix increment";
  if (opEq(op, OP_SDE)) return "suffix decrement";
  if (opEq(op, OP_PIN)) return "prefix increment";
  if (opEq(op, OP_PDE)) return "prefix decrement";
  if (opEq(op, OP_NOT)) return "not";
  if (opEq(op, OP_BNT)) return "bitwise not";
  if (opEq(op, OP_MUL)) return "multiply";
  if (opEq(op, OP_DIV)) return "divide";
  if (opEq(op, OP_REM)) return "reminder";
  if (opEq(op, OP_ADD)) return "add";
  if (opEq(op, OP_SUB)) return "subtract";
  if (opEq(op, OP_LSH)) return "left shift";
  if (opEq(op, OP_RSH)) return "right shift";
  if (opEq(op, OP_AND)) return "bitwise and";
  if (opEq(op, OP_XOR)) return "bitwise xor";
  if (opEq(op, OP_ORR)) return "bitwise or";
  if (opEq(op, OP_SMT)) return "smaller";
  if (opEq(op, OP_STE)) return "smaller or equal";
  if (opEq(op, OP_LGT)) return "larger";
  if (opEq(op, OP_LTE)) return "larger or equal";
  if (opEq(op, OP_EQU)) return "equal";
  if (opEq(op, OP_NEQ)) return "not equal";
  if (opEq(op, OP_LAN)) return "and";
  if (opEq(op, OP_LOR)) return "or";
  if (opEq(op, OP_ASS)) return "assignment";
  if (opEq(op, OP_MLA)) return "multiply assign";
  if (opEq(op, OP_DVA)) return "divide assign";
  if (opEq(op, OP_RMA)) return "reminder assign";
  if (opEq(op, OP_ADA)) return "add assign";
  if (opEq(op, OP_SBA)) return "subtract assign";
  if (opEq(op, OP_LSA)) return "left shift assign";
  if (opEq(op, OP_RSA)) return "right shift assign";
  if (opEq(op, OP_ANA)) return "bitwise and assign";
  if (opEq(op, OP_XRA)) return "bitwise xor assign";
  if (opEq(op, OP_ORA)) return "bitwise or assign";
  dbgUnexpected("Unknown operator!");
}
