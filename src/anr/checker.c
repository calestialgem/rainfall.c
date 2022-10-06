// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"
#include "psr/api.h"

// Prototype for recursion in evaluations.
static ExpressionNode const* nodeAny(ExpressionNode const* node, Type type);

/* Root of the current evaluation. */
static EvaluationNode root() { return evlRoot(anr.evl); }

/* Check whether the type of the given access nullary expression node is the
 * same as the given type. Returns the node that comes after all the childeren
 * of the given one, or null if the type does not match. */
static ExpressionNode const*
nodeAcs(ExpressionNode const* const node, Type const type) {
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
      EvaluationNode evl = evlRoot(i->evl);
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
nodeDec(ExpressionNode const* const node, Type const type) {
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
nodeNull(ExpressionNode const* const node, Type const type) {
  if (opEq(node->op, OP_ACS)) return nodeAcs(node, type);
  if (opEq(node->op, OP_DEC)) return nodeDec(node, type);
  dbgUnexpected("Unknown nullary operator!");
  return NULL;
}

/* Check whether the type of the given prenary expression node is the same as
 * the given type. Returns the node that comes after all the childeren of the
 * given one, or null if the type does not match. */
static ExpressionNode const*
nodePre(ExpressionNode const* const node, Type const type) {
  ExpressionNode const* const operand = nodeAny(node - 1, type);
  if (operand == NULL) return NULL;
  if (opEq(node->op, OP_POS)) {
    EvaluationNode evl = evlAt(anr.evl, evlLen(anr.evl) - 1);
    evl.exp            = *node;
    evlAdd(&anr.evl, evl);
    return node - 1;
  }
  return operand;
}

/* Check whether the type of the given postary expression node is the same as
 * the given type. Returns the node that comes after all the childeren of the
 * given one, or null if the type does not match. */
static ExpressionNode const*
nodePost(ExpressionNode const* const node, Type const type) {}

/* Check whether the type of the given cirnary expression node is the same as
 * the given type. Returns the node that comes after all the childeren of the
 * given one, or null if the type does not match. */
static ExpressionNode const*
nodeCir(ExpressionNode const* const node, Type const type) {}

/* Check whether the type of the given binary expression node is the same as the
 * given type. Returns the node that comes after all the childeren of the given
 * one, or null if the type does not match. */
static ExpressionNode const*
nodeBin(ExpressionNode const* const node, Type const type) {}

/* Check whether the type of the given variary expression node is the same as
 * the given type. Returns the node that comes after all the childeren of the
 * given one, or null if the type does not match. */
static ExpressionNode const*
nodeVar(ExpressionNode const* const node, Type const type) {}

/* Check whether the type of the given expression node is the same as the given
 * type. Returns the node that comes after all the childeren of the given one,
 * or null if the type does not match. */
static ExpressionNode const*
nodeAny(ExpressionNode const* const node, Type const type) {
  switch (node->op.tag) {
  case OP_NULL: return nodeNull(node, type);
  case OP_PRE: return nodePre(node, type);
  case OP_POST: return nodePost(node, type);
  case OP_CIR: return nodeCir(node, type);
  case OP_BIN: return nodeBin(node, type);
  case OP_VAR: return nodeVar(node, type);
  default: dbgUnexpected("Unknown operator tag!");
  }
}

/* Check whether the type of the given expression is the same as the given type.
 * Returns true if the type matched. */
static bool exp(Expression const exp, Type const type) {
  return nodeAny(exp.end - 1, type);
}

/* Last analyzed evaluation. */
static Evaluation evlGet() {
  Evaluation res = anr.evl;
  anr.evl        = evlOf(0);
  return res;
}

/* Check the types of the expressions in the given let definition statement. */
static void sttLet(LetDefinition const let) {}

/* Check the types of the expressions in the given var definition statement. */
static void sttVar(VarDefinition const var) {}

/* Check the type of the expression in the given assignment statement. */
static void sttAss(Assignment const ass) {}

/* Check the types of the expression in the given compound assignment statement.
 */
static void sttCas(CompoundAssignment const cas) {}

/* Check the types of the expression in the given expression statement. */
static void sttExp(ExpressionStatement const exp) {}

void anrCheck() {
  for (Statement const* i = anr.prs.bgn; i < anr.prs.end; i++) {
    switch (i->tag) {
    case STT_LET: sttLet(i->let); break;
    case STT_VAR: sttVar(i->var); break;
    case STT_ASS: sttAss(i->ass); break;
    case STT_CAS: sttCas(i->cas); break;
    case STT_EXP: sttExp(i->exp); break;
    default: dbgUnexpected("Unknown statement tag!");
    }
  }
}
