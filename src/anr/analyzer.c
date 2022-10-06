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

/* Last analyzed evaluation. Clears the currently analyzed evaluation before
 * returning. */
static Evaluation evlGet() {
  Evaluation res = anr.evl;
  anr.evl        = evlOf(0);
  return res;
}

/* Root of the current evaluation. */
static EvaluationNode root() { return evlRoot(anr.evl); }

/* Add a new symbol with the given name with the current evaluation. */
static void add(String const name) {
  mapPut(&anr.map, name, tblLen(*anr.tbl));
  tblAdd(anr.tbl, (Symbol){.name = name, .evl = evlGet()});
}

/* Add the built-in symbols. */
static void prepare() {
  for (ux i = 0; i < TYPE_BUILT_LEN; i++) {
    String const name = strOf(typeName(TYPE_BUILT[i]));
    evlAdd(
      &anr.evl,
      (EvaluationNode){
        .type = TYPE_INS_META, .val = {.meta = TYPE_BUILT[i]}, .has = true});
    add(name);
  }
}

// Prototype for recursive check of expressions.
static ExpressionNode const* checkNode(ExpressionNode const* node, Type type);

/* Version of `checkNull` that takes `OP_ACS`s. */
static ExpressionNode const*
checkAcs(ExpressionNode const* const node, Type const type) {
  MapEntry const* entry = mapGet(anr.map, node->val);
  if (!entry) {
    otcErr(anr.otc, node->val, "Unknown symbol!");
    return NULL;
  }
  EvaluationNode const acs = evlRoot(tblAt(*anr.tbl, entry->val).evl);
  if (!typeEq(acs.type, type)) {
    otcErr(
      anr.otc, node->val, "Expected a `%s`, but `%.*s` is a `%s`!",
      typeName(type), (int)strLen(node->val), node->val.bgn,
      typeName(acs.type));
    return NULL;
  }
  EvaluationNode evl = acs;
  evl.exp            = *node;
  evlAdd(&anr.evl, evl);
  return node - 1;
}

/* Version of `checkNull` that takes `OP_DEC`s. */
static ExpressionNode const*
checkDec(ExpressionNode const* const node, Type const type) {
  if (!(typeEq(type, TYPE_INS_I1) && typeEq(type, TYPE_INS_I2) &&
        typeEq(type, TYPE_INS_I4) && typeEq(type, TYPE_INS_I8) &&
        typeEq(type, TYPE_INS_IX) && typeEq(type, TYPE_INS_U1) &&
        typeEq(type, TYPE_INS_U2) && typeEq(type, TYPE_INS_U4) &&
        typeEq(type, TYPE_INS_U8) && typeEq(type, TYPE_INS_UX) &&
        typeEq(type, TYPE_INS_F4) && typeEq(type, TYPE_INS_F8))) {
    otcErr(
      anr.otc, node->val, "Expected a `%s`, not a number!", typeName(type));
    return NULL;
  }

  bool const neg = strAt(node->val, 0) == '-';
  Number     num = numOfZero();
  if (numSetDec(
        &num, (String){
                .bgn = (neg || strAt(node->val, 0) == '+') + node->val.bgn,
                .end = node->val.end})) {
    otcErr(anr.otc, node->val, "Exponent is too long!");
    return NULL;
  }

#define parseSigned(TYPE, TYPE_INS, MAX)                                    \
  if (typeEq(type, TYPE_INS)) {                                             \
    if (!numInt(num)) {                                                     \
      otcErr(                                                               \
        anr.otc, node->val, "Not an integer for integer type `%s`!",        \
        typeName(type));                                                    \
      return NULL;                                                          \
    }                                                                       \
    i8 const cmp = numCmp(num, (u8)(MAX) + 1);                              \
    if (cmp == 1 || (cmp == 0 && !neg)) {                                   \
      otcErr(anr.otc, node->val, "Out of bounds of `%s`!", typeName(type)); \
      return NULL;                                                          \
    }                                                                       \
    EvaluationNode evl = {                                                  \
      .exp  = *node,                                                        \
      .type = type,                                                         \
      .val  = {.TYPE = (TYPE)numU8(num)},                                   \
      .has  = true};                                                         \
    if (neg) evl.val.TYPE = (TYPE)-evl.val.TYPE;                            \
    evlAdd(&anr.evl, evl);                                                  \
    goto success;                                                           \
  }

#define parseUnsigned(TYPE, TYPE_INS, MAX)                                    \
  if (typeEq(type, TYPE_INS)) {                                               \
    if (!numInt(num)) {                                                       \
      otcErr(                                                                 \
        anr.otc, node->val, "Not an integer for integer type `%s`!",          \
        typeName(type));                                                      \
      return NULL;                                                            \
    }                                                                         \
    i8 const cmp = numCmp(num, MAX);                                          \
    if (cmp == 1) {                                                           \
      otcErr(anr.otc, node->val, "Out of bounds of `%s`!", typeName(type));   \
      return NULL;                                                            \
    }                                                                         \
    EvaluationNode evl = {                                                    \
      .exp  = *node,                                                          \
      .type = type,                                                           \
      .val  = {.TYPE = (TYPE)numU8(num)},                                     \
      .has  = true};                                                           \
    if (neg && evl.val.TYPE != 0) {                                           \
      otcErr(                                                                 \
        anr.otc, node->val, "Negative value for unsigned integer type `%s`!", \
        typeName(type));                                                      \
      return NULL;                                                            \
    }                                                                         \
    evlAdd(&anr.evl, evl);                                                    \
    goto success;                                                             \
  }

#define parseFloat(TYPE, TYPE_INS)         \
  if (typeEq(type, TYPE_INS)) {            \
    EvaluationNode evl = {                 \
      .exp  = *node,                       \
      .type = type,                        \
      .val  = {.TYPE = (TYPE)numF8(num)},  \
      .has  = true};                        \
    if (neg) evl.val.TYPE = -evl.val.TYPE; \
    evlAdd(&anr.evl, evl);                 \
    goto success;                          \
  }

  parseSigned(i1, TYPE_INS_I1, INT8_MAX);
  parseSigned(i2, TYPE_INS_I2, INT16_MAX);
  parseSigned(i4, TYPE_INS_I4, INT32_MAX);
  parseSigned(i8, TYPE_INS_I8, INT64_MAX);
  parseSigned(ix, TYPE_INS_IX, INTPTR_MAX);

  parseUnsigned(u1, TYPE_INS_U1, UINT8_MAX);
  parseUnsigned(u2, TYPE_INS_U2, UINT16_MAX);
  parseUnsigned(u4, TYPE_INS_U4, UINT32_MAX);
  parseUnsigned(u8, TYPE_INS_U8, UINT64_MAX);
  parseUnsigned(ux, TYPE_INS_UX, UINTPTR_MAX);

  parseFloat(f4, TYPE_INS_F4);
  parseFloat(f8, TYPE_INS_F8);

#undef parseSigned
#undef parseUnsigned
#undef parseFloat

success:

  numFree(&num);
  return node - 1;
}

/* Version of `checkNode` that takes `NullaryOperator`s. */
static ExpressionNode const*
checkNull(ExpressionNode const* const node, Type const type) {
  if (opEq(node->op, OP_ACS)) return checkAcs(node, type);
  if (opEq(node->op, OP_DEC)) return checkDec(node, type);
  dbgUnexpected("Unknown nullary operator!");
  return NULL;
}

/* Version of `checkNode` that takes `PrenaryOperator`s. */
static ExpressionNode const*
checkPre(ExpressionNode const* const node, Type const type) {
  ExpressionNode const* const operand = checkNode(node - 1, type);
  if (operand == NULL) return NULL;
  if (opEq(node->op, OP_POS)) {
    EvaluationNode evl = root();
    evl.exp            = *node;
    evlAdd(&anr.evl, evl);
    return node - 1;
  }
  return operand;
}

/* Version of `checkNode` that takes `PostaryOperator`s. */
static ExpressionNode const*
checkPost(ExpressionNode const* const node, Type const type) {}

/* Version of `checkNode` that takes `CirnaryOperator`s. */
static ExpressionNode const*
checkCir(ExpressionNode const* const node, Type const type) {}

/* Version of `checkNode` that takes `BinaryOperator`s. */
static ExpressionNode const*
checkBin(ExpressionNode const* const node, Type const type) {}

/* Version of `checkNode` that takes `VariaryOperator`s. */
static ExpressionNode const*
checkVar(ExpressionNode const* const node, Type const type) {}

/* Check the given node agains the given type. Returns the node that comes
 * after the given one and its childeren or null if the type does not match. */
static ExpressionNode const*
checkNode(ExpressionNode const* const node, Type const type) {
  switch (node->op.tag) {
  case OP_NULL: return checkNull(node, type);
  case OP_PRE: return checkPre(node, type);
  case OP_POST: return checkPost(node, type);
  case OP_CIR: return checkCir(node, type);
  case OP_BIN: return checkBin(node, type);
  case OP_VAR: return checkVar(node, type);
  default: dbgUnexpected("Unknown operator tag!");
  }
}

/* Whether the type of the given expression is the same as the given type. */
static bool checkExp(Expression const exp, Type const type) {
  return checkNode(exp.end - 1, type);
}

/* Whether the definition of the symbol with the given name is valid. */
static bool assesDef(String const name) {
  MapEntry const* const e = mapGet(anr.map, name);
  if (!e) return true;
  Symbol const prev = tblAt(*anr.tbl, e->val);
  if (symUsr(prev)) {
    otcErr(anr.otc, name, "Name clashes with a previously defined symbol!");
    otcInfo(*anr.otc, prev.name, "Previous definition was here.");
  } else {
    otcErr(anr.otc, name, "Name clashes with a built-in symbol!");
  }
  return false;
}

/* Resolve the given let definition statement. */
static void resolveLet(LetDefinition const let) {
  if (!assesDef(let.name)) return;
}

/* Resolve the given var definition statement. */
static void resolveVar(VarDefinition const var) {
  if (!assesDef(var.name)) return;
}

/* Resolve the given assignment statement. */
static void resolveAss(Assignment const ass) {}

/* Resolve the given compound assignment statement. */
static void resolveCas(CompoundAssignment const cas) {}

/* Resolve the given expression statement. */
static void resolveExp(ExpressionStatement const exp) {}

/* Check the name accesses and types of expressions in the given statement. */
static void resolve(Statement const stt) {
  switch (stt.tag) {
  case STT_LET: resolveLet(stt.let); break;
  case STT_VAR: resolveVar(stt.var); break;
  case STT_ASS: resolveAss(stt.ass); break;
  case STT_CAS: resolveCas(stt.cas); break;
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
