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
static struct {
  /* Table to add the symbols into. */
  Table*     tbl;
  /* Outcome to report to. */
  Outcome*   otc;
  /* Analyzed parse. */
  Parse      prs;
  /* Set of built-in symbols. */
  Set        bld;
  /* Set of user-defined symbols. */
  Set        usr;
  /* Set of user-defined, resolved but not checked symbols. */
  Set        res;
  /* Evaluation that is currently analyzed. */
  Evaluation evl;
} anr;

/* Add the built-in symbols. */
static void prepare() {
  for (ux i = 0; i < TYPE_BUILT_LEN; i++) {
    String const name = strOf(typeName(TYPE_BUILT[i]));
    Evaluation   evl  = evlOf(1);
    evl.bgn[0]        = (EvaluationNode){
             .type = TYPE_INS_META, .val = {.meta = TYPE_BUILT[i]}, .has = true};
    setPut(&anr.bld, name);
    tblAdd(anr.tbl, (Symbol){.name = name, .evl = evl});
  }
}

/* Resolve the definition with the given name. */
static void resolveDef(String const name) {
  if (setGet(anr.bld, name)) {
    otcErr(anr.otc, name, "Name clashes with a built-in symbol!");
    return;
  }
  String const* const userDef = setGet(anr.res, name);
  if (userDef) {
    otcErr(anr.otc, name, "Name clashes with a previously defined symbol!");
    otcInfo(*anr.otc, *userDef, "Previous definition was here.");
    return;
  }
  setPut(&anr.res, name);
}

/* Resolve name clashes in the definitions. */
static void resolve() {
  for (Statement const* i = anr.prs.bgn; i < anr.prs.end; i++) {
    switch (i->tag) {
    case STT_LET: resolveDef(i->let.name); break;
    case STT_VAR: resolveDef(i->var.name); break;
    default: break;
    }
  }
}

/* Check whether the type of the given access nullary expression node is the
 * same as the given type. Returns the node that comes after all the childeren
 * of the given one, or null if the type does not match. */
static ExpressionNode const*
checkExpNodeNullaryAccess(ExpressionNode const* const node, Type const type) {
  if (!setGet(anr.bld, node->val) && !setGet(anr.usr, node->val)) {
    otcErr(anr.otc, node->val, "Could not find the accessed symbol!");
    String const* const unchecked = setGet(anr.res, node->val);
    if (unchecked)
      otcInfo(*anr.otc, *unchecked, "Symbol is defined later here.");
    return NULL;
  }
  for (Symbol const* i = anr.tbl->bgn; i < anr.tbl->end; i++) {
    if (strEq(node->val, i->name)) {
      if (!typeEq(evlType(i->evl), type)) {
        otcErr(
          anr.otc, node->val, "Expected a `%s`, but `%.*s` is a `%s`!",
          typeName(type), (int)strLen(i->name), i->name.bgn,
          typeName(evlType(i->evl)));
        return NULL;
      }
      EvaluationNode evl = evlAt(i->evl, evlLen(i->evl) - 1);
      evl.exp            = *node;
      evlAdd(&anr.evl, evl);
      return node - 1;
    }
  }
  dbgUnexpected("Symbol is in the sets but not in the table!");
  return NULL;
}

/* Check whether the type of the given decimal nullary expression node is the
 * same as the given type. Returns the node that comes after all the childeren
 * of the given one, or null if the type does not match. */
static ExpressionNode const*
checkExpNodeNullaryDecimal(ExpressionNode const* const node, Type const type) {
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

/* Check whether the type of the given nullary expression node is the same as
 * the given type. Returns the node that comes after all the childeren of the
 * given one, or null if the type does not match. */
static ExpressionNode const*
checkExpNodeNullary(ExpressionNode const* const node, Type const type) {
  if (opEq(node->op, OP_ACS)) return checkExpNodeNullaryAccess(node, type);
  if (opEq(node->op, OP_DEC)) return checkExpNodeNullaryDecimal(node, type);
  dbgUnexpected("Unknown nullary operator!");
  return NULL;
}

/* Check whether the type of the given prenary expression node is the same as
 * the given type. Returns the node that comes after all the childeren of the
 * given one, or null if the type does not match. */
static ExpressionNode const*
checkExpNodePrenary(ExpressionNode const* const node, Type const type) {}

/* Check whether the type of the given postary expression node is the same as
 * the given type. Returns the node that comes after all the childeren of the
 * given one, or null if the type does not match. */
static ExpressionNode const*
checkExpNodePostary(ExpressionNode const* const node, Type const type) {}

/* Check whether the type of the given cirnary expression node is the same as
 * the given type. Returns the node that comes after all the childeren of the
 * given one, or null if the type does not match. */
static ExpressionNode const*
checkExpNodeCirnary(ExpressionNode const* const node, Type const type) {}

/* Check whether the type of the given binary expression node is the same as the
 * given type. Returns the node that comes after all the childeren of the given
 * one, or null if the type does not match. */
static ExpressionNode const*
checkExpNodeBinary(ExpressionNode const* const node, Type const type) {}

/* Check whether the type of the given variary expression node is the same as
 * the given type. Returns the node that comes after all the childeren of the
 * given one, or null if the type does not match. */
static ExpressionNode const*
checkExpNodeVariary(ExpressionNode const* const node, Type const type) {}

/* Check whether the type of the given expression node is the same as the given
 * type. Returns the node that comes after all the childeren of the given one,
 * or null if the type does not match. */
static ExpressionNode const*
checkExpNode(ExpressionNode const* const node, Type const type) {
  switch (node->op.tag) {
  case OP_NULL: return checkExpNodeNullary(node, type);
  case OP_PRE: return checkExpNodePrenary(node, type);
  case OP_POST: return checkExpNodePostary(node, type);
  case OP_CIR: return checkExpNodeCirnary(node, type);
  case OP_BIN: return checkExpNodeBinary(node, type);
  case OP_VAR: return checkExpNodeVariary(node, type);
  default: dbgUnexpected("Unknown operator tag!");
  }
}

/* Check whether the type of the given expression is the same as the given type.
 * Returns true if the type matched. */
static bool checkExp(Expression const exp, Type const type) {
  return checkExpNode(exp.end - 1, type);
}

/* Last analyzed evaluation. */
static Evaluation evlGet() {
  Evaluation res = anr.evl;
  anr.evl        = evlOf(0);
  return res;
}

/* Check the types of the expressions in the given let definition statement. */
static void checkSttLet(LetDefinition const let) {}

/* Check the types of the expressions in the given var definition statement. */
static void checkSttVar(VarDefinition const var) {}

/* Check the type of the expression in the given assignment statement. */
static void checkSttAss(Assignment const ass) {}

/* Check the types of the expression in the given compound assignment statement.
 */
static void checkSttCas(CompoundAssignment const cas) {}

/* Check the types of the expression in the given expression statement. */
static void checkSttExp(ExpressionStatement const exp) {}

/* Check the types of the expressions. */
static void check() {
  for (Statement const* i = anr.prs.bgn; i < anr.prs.end; i++) {
    switch (i->tag) {
    case STT_LET: checkSttLet(i->let); break;
    case STT_VAR: checkSttVar(i->var); break;
    case STT_ASS: checkSttAss(i->ass); break;
    case STT_CAS: checkSttCas(i->cas); break;
    case STT_EXP: checkSttExp(i->exp); break;
    default: dbgUnexpected("Unknown statement tag!");
    }
  }
}

void analyzerAnalyze(Table* const tbl, Outcome* const otc, Parse const prs) {
  anr.tbl = tbl;
  anr.otc = otc;
  anr.prs = prs;
  anr.bld = setOf(0);
  anr.usr = setOf(0);
  anr.res = setOf(0);
  anr.evl = evlOf(0);

  prepare();
  resolve();
  check();
}
