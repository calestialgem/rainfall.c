// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Context of the analysis process. */
struct {
  /* Table to add the symbols into. */
  Table*     tbl;
  /* Outcome to report to. */
  Outcome*   otc;
  /* Analyzed parse. */
  Parse      prs;
  /* Map of symbol indicies on the table from the symbol names. */
  Map        map;
  /* Evaluation that is currently analyzed. */
  Evaluation evl;
} anr;

/* Root of the current evaluation. */
static EvaluationNode root() { return evlRoot(anr.evl); }

/* Last analyzed evaluation. Clears the currently analyzed evaluation before
 * returning. */
static Evaluation get() {
  Evaluation const res = anr.evl;
  anr.evl              = evlOf(0);
  return res;
}

/* Add the given symbol to the table and the map. */
static void add(Symbol const sym) {
  iptr const len = tblLen(*anr.tbl);
  printf("[%lli] %.*s\n", len, (int)strLen(sym.name), sym.name.bgn);
  mapPut(&anr.map, sym.name, len);
  tblAdd(anr.tbl, sym);
}

/* Add the given type as a built-in. */
static void prepareType(Type const type) {
  String const name = strOf(typeName(type));
  Symbol const sym  = {
     .tag  = SYM_TYPE,
     .name = name,
     .type = TYPE_INS_META,
     .val  = {.meta = type},
     .has  = true};
  add(sym);
}

/* Add the built-in symbols. */
static void prepare() {
  for (iptr i = 0; i < TYPE_BUILT_LEN; i++) prepareType(TYPE_BUILT[i]);
}

// Prototype for recursive evaluation of expressions.
static ExpressionNode const*
evaluateNode(ExpressionNode const* node, Type type, bool expect);

/* Version of `evaluateNull` that takes `OP_ACS`s. */
static ExpressionNode const* evaluateAcs(
  ExpressionNode const* const node, Type const type, bool const expect) {
  MapEntry const* const e = mapGet(anr.map, node->val);
  if (!e) {
    otcErr(anr.otc, node->val, "Unknown symbol!");
    return NULL;
  }
  Symbol const acs = tblAt(*anr.tbl, e->val);
  if (expect && !typeEq(acs.type, type)) {
    otcErr(
      anr.otc, node->val, "Expected a `%s`, but `%.*s` is a `%s`!",
      typeName(type), (int)strLen(node->val), node->val.bgn,
      typeName(acs.type));
    return NULL;
  }
  EvaluationNode const evl = {
    .exp = *node, .type = type, .val = acs.val, .has = acs.has};
  evlAdd(&anr.evl, evl);
  return node - 1;
}

/* Version of `evaluateNull` that takes `OP_DEC`s. */
static ExpressionNode const* evaluateDec(
  ExpressionNode const* const node, Type const type, bool const expect) {
  bool const neg = strAt(node->val, 0) == '-';
  Number     num = numOfDec((String){
        .bgn = (neg || strAt(node->val, 0) == '+') + node->val.bgn,
        .end = node->val.end});
  if (num.flag == NUM_TOO_PRECISE) {
    otcErr(anr.otc, node->val, "The number is too long!");
    return NULL;
  }
  if (!expect) {
    if (numIsInt(num)) {
      int const cmp = numCmp(num, (uint64_t)INT_MAX + 1);
      if (cmp != 1 && (cmp != 0 || neg)) {
        EvaluationNode evl = {
          .exp  = *node,
          .type = TYPE_INS_INT,
          .val  = {.vint = (int)numAsInt(num)},
          .has  = true};
        if (neg) evl.val.vint = -evl.val.vint;
        evlAdd(&anr.evl, evl);
        goto success;
      }
    }
    EvaluationNode evl = {
      .exp  = *node,
      .type = TYPE_INS_DOUBLE,
      .val  = {.vdouble = numAsDouble(num)},
      .has  = true};
    if (neg) evl.val.vdouble = -evl.val.vdouble;
    evlAdd(&anr.evl, evl);
    goto success;
  }

#define parseSigned(TYPE, TYPE_VAL, TYPE_INS, MAX)                          \
  if (typeEq(type, TYPE_INS)) {                                             \
    if (!numIsInt(num)) {                                                   \
      otcErr(                                                               \
        anr.otc, node->val,                                                 \
        "Expected integer type `%s`, but the value is not an integer!",     \
        typeName(type));                                                    \
      return NULL;                                                          \
    }                                                                       \
    int const cmp = numCmp(num, (uint64_t)(MAX) + 1);                       \
    if (cmp == 1 || (cmp == 0 && !neg)) {                                   \
      otcErr(anr.otc, node->val, "Out of bounds of `%s`!", typeName(type)); \
      return NULL;                                                          \
    }                                                                       \
    EvaluationNode evl = {                                                  \
      .exp  = *node,                                                        \
      .type = type,                                                         \
      .val  = {.TYPE_VAL = (TYPE)numAsInt(num)},                            \
      .has  = true};                                                         \
    if (neg) evl.val.TYPE_VAL = (TYPE)-evl.val.TYPE_VAL;                    \
    evlAdd(&anr.evl, evl);                                                  \
    goto success;                                                           \
  }

#define parseUnsigned(TYPE, TYPE_VAL, TYPE_INS, MAX)                        \
  if (typeEq(type, TYPE_INS)) {                                             \
    if (!numInt(num)) {                                                     \
      otcErr(                                                               \
        anr.otc, node->val,                                                 \
        "Expected integer type `%s`, but the value is not an integer!",     \
        typeName(type));                                                    \
      return NULL;                                                          \
    }                                                                       \
    int const cmp = numCmp(num, MAX);                                       \
    if (cmp == 1) {                                                         \
      otcErr(anr.otc, node->val, "Out of bounds of `%s`!", typeName(type)); \
      return NULL;                                                          \
    }                                                                       \
    EvaluationNode evl = {                                                  \
      .exp  = *node,                                                        \
      .type = type,                                                         \
      .val  = {.TYPE_VAL = (TYPE)numU8(num)},                               \
      .has  = true};                                                         \
    if (neg && evl.val.TYPE_VAL != 0) {                                     \
      otcErr(                                                               \
        anr.otc, node->val,                                                 \
        "Expected unsigned integer type `%s`, but the value is negative!",  \
        typeName(type));                                                    \
      return NULL;                                                          \
    }                                                                       \
    evlAdd(&anr.evl, evl);                                                  \
    goto success;                                                           \
  }

#define parseFloat(TYPE, TYPE_VAL, TYPE_INS, NUM_PARSE) \
  if (typeEq(type, TYPE_INS)) {                         \
    EvaluationNode evl = {                              \
      .exp  = *node,                                    \
      .type = type,                                     \
      .val  = {.TYPE_VAL = NUM_PARSE(num)},             \
      .has  = true};                                     \
    if (neg) evl.val.TYPE_VAL = -evl.val.TYPE_VAL;      \
    evlAdd(&anr.evl, evl);                              \
    goto success;                                       \
  }

  if (typeEq(type, TYPE_INS_BOOL)) {
    EvaluationNode evl = {
      .exp = *node, .type = type, .val = {.vbool = numAsInt(num)}, .has = true};
    evlAdd(&anr.evl, evl);
  }

  parseSigned(char, byte, TYPE_INS_BYTE, INT8_MAX);
  parseSigned(int, vint, TYPE_INS_INT, INT_MAX);
  parseSigned(iptr, iptr, TYPE_INS_IPTR, PTRDIFF_MAX);
  parseFloat(float, vfloat, TYPE_INS_FLOAT, numAsFloat);
  parseFloat(double, vdouble, TYPE_INS_DOUBLE, numAsDouble);

  otcErr(anr.otc, node->val, "Expected a `%s`, not a number!", typeName(type));
  numFree(&num);
  return NULL;

#undef parseSigned
#undef parseUnsigned
#undef parseFloat

success:

  numFree(&num);
  return node - 1;
}

/* Version of `evaluateNode` that takes `NullaryOperator`s. */
static ExpressionNode const* evaluateNull(
  ExpressionNode const* const node, Type const type, bool const expect) {
  if (opEq(node->op, OP_ACS)) return evaluateAcs(node, type, expect);
  if (opEq(node->op, OP_DEC)) return evaluateDec(node, type, expect);
  dbgUnexpected("Unknown nullary operator!");
  return NULL;
}

/* Version of `evaluatePre` that takes `OP_POS`s. */
static ExpressionNode const* checkPos(
  ExpressionNode const* const node, ExpressionNode const* const operand,
  Type const type, bool const expect) {
  EvaluationNode evl = root();
  evl.exp            = *node;
  evlAdd(&anr.evl, evl);
  return operand;
}

/* Version of `evaluateNode` that takes `PrenaryOperator`s. */
static ExpressionNode const* evaluatePre(
  ExpressionNode const* const node, Type const type, bool const expect) {
  ExpressionNode const* const operand = evaluateNode(node - 1, type, expect);
  if (operand == NULL) return NULL;
  if (opEq(node->op, OP_POS)) return checkPos(node, operand, type, expect);
  dbgUnexpected("Unknown prenary operator!");
  return NULL;
}

/* Version of `evaluateNode` that takes `PostaryOperator`s. */
static ExpressionNode const* evaluatePost(
  ExpressionNode const* const node, Type const type, bool const expect) {
  return NULL;
}

/* Version of `evaluateNode` that takes `CirnaryOperator`s. */
static ExpressionNode const* evaluateCir(
  ExpressionNode const* const node, Type const type, bool const expect) {
  return NULL;
}

/* Version of `evaluateNode` that takes `BinaryOperator`s. */
static ExpressionNode const* evaluateBin(
  ExpressionNode const* const node, Type const type, bool const expect) {
  return NULL;
}

/* Version of `evaluateNode` that takes `VariaryOperator`s. */
static ExpressionNode const* evaluateVar(
  ExpressionNode const* const node, Type const type, bool const expect) {
  return NULL;
}

/* Evaluate the given node. Returns the node that comes after the given one and
 * its childeren or null if the type does not match. */
static ExpressionNode const* evaluateNode(
  ExpressionNode const* const node, Type const type, bool const expect) {
  switch (node->op.tag) {
  case OP_NULL: return evaluateNull(node, type, expect);
  case OP_PRE: return evaluatePre(node, type, expect);
  case OP_POST: return evaluatePost(node, type, expect);
  case OP_CIR: return evaluateCir(node, type, expect);
  case OP_BIN: return evaluateBin(node, type, expect);
  case OP_VAR: return evaluateVar(node, type, expect);
  default: dbgUnexpected("Unknown operator tag!");
  }
}

/* Evaluate the given expression. Returns whether the type of the given
 * expression is the same as the given type if it is expected to be. */
static bool
evaluateExp(Expression const exp, Type const type, bool const expect) {
  return evaluateNode(exp.end - 1, type, expect);
}

/* Whether the definition of the symbol with the given name is valid. */
static bool assesDef(String const name) {
  MapEntry const* const e = mapGet(anr.map, name);
  if (!e) return true;
  Symbol const prev = tblAt(*anr.tbl, e->val);
  if (prev.usr) {
    otcErr(anr.otc, name, "Name clashes with a previously defined symbol!");
    otcInfo(*anr.otc, prev.name, "Previous definition was here.");
  } else {
    otcErr(anr.otc, name, "Name clashes with a built-in symbol!");
  }
  return false;
}

/* Resolve the type of a definiton from the given expression. */
static void
resolveType(Expression const exp, Type* const type, bool* const expect) {
  if (expLen(exp)) {
    if (!evaluateExp(exp, TYPE_INS_META, true)) return;
    Evaluation const evl = get();
    if (!evlHas(evl)) {
      otcErr(anr.otc, expStr(exp), "Type must be known at compile-time!");
      return;
    }
    *type = evlVal(evl).meta;
    if (typeEq(*type, TYPE_INS_VOID)) {
      otcErr(anr.otc, expStr(exp), "Type cannot be void!");
      return;
    }
    *expect = true;
  }
}

/* Resolve the given let definition statement. */
static void resolveLet(LetDefinition const let) {
  if (!assesDef(let.name)) return;
  bool expect = false;
  Type type   = TYPE_INS_VOID;
  resolveType(let.type, &type, &expect);

  if (!evaluateExp(let.val, type, expect)) return;

  Evaluation const evl = get();
  Symbol const     sym = {
        .bind = {.evl = evl},
        .tag  = SYM_BIND,
        .name = let.name,
        .type = evlType(evl),
        .val  = evlVal(evl),
        .has  = evlHas(evl),
        .usr  = true};
  add(sym);
}

/* Resolve the given var definition statement. */
static void resolveVar(VarDefinition const var) {
  if (!assesDef(var.name)) return;
  bool expect = false;
  Type type   = TYPE_INS_VOID;
  resolveType(var.type, &type, &expect);

  if (expLen(var.val)) {
    if (!evaluateExp(var.val, type, expect)) return;
    Evaluation const evl = get();
    Symbol const     sym = {
          .var  = {.evl = evl},
          .tag  = SYM_VAR,
          .name = var.name,
          .type = evlType(evl),
          .val  = evlVal(evl),
          .has  = evlHas(evl),
          .usr  = true};
    add(sym);
    return;
  }

  Symbol const sym = {
    .tag  = SYM_VAR,
    .name = var.name,
    .type = type,
    .val  = valDefault(type),
    .has  = true,
    .usr  = true};
  add(sym);
}

/* Resolve the given expression statement. */
static void resolveExp(ExpressionStatement const exp) {
  if (!evaluateExp(exp.exp, TYPE_INS_VOID, true))
    otcErr(anr.otc, expStr(exp.exp), "Value of the expression is lost!");
}

/* Check the name accesses and types of expressions in the given statement. */
static void resolve(Statement const stt) {
  switch (stt.tag) {
  case STT_LET: resolveLet(stt.let); break;
  case STT_VAR: resolveVar(stt.var); break;
  case STT_EXP: resolveExp(stt.exp); break;
  default: dbgUnexpected("Unknown statement tag!");
  }
}

void analyze(Table* const tbl, Outcome* const otc, Parse const prs) {
  anr.tbl = tbl;
  anr.otc = otc;
  anr.prs = prs;
  anr.map = mapOf(0);
  anr.evl = evlOf(0);

  prepare();
  for (Statement const* i = anr.prs.bgn; i < anr.prs.end; i++) resolve(*i);
}
