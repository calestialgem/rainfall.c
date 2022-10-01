// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>

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

/* Convert the given decimal digit string with optional underscore separators
 * into the given number. Returns true if it owerflows. */
static bool checkDecimal(String const str, ux* const res) {
  u8 const BASE = 10;
  *res          = 0;
  for (char const* i = str.bgn; i < str.end; i++) {
    if (*i == '_') continue;
    ux pre = *res;
    *res *= BASE;
    if (pre > *res) return true;
    pre = *res;
    *res += *i - '0';
    if (pre > *res) return true;
  }
  return false;
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

  String whole    = node->val;
  String fraction = {.bgn = node->val.end, .end = node->val.end};
  String exponent = {.bgn = node->val.end, .end = node->val.end};

  for (char const* i = node->val.end - 1; i >= node->val.bgn; i--) {
    switch (*i) {
    case '.':
      whole.end    = i;
      fraction.bgn = i + 1;
      break;
    case 'e':
    case 'E':
      whole.end    = i;
      fraction.bgn = i;
      fraction.end = i;
      exponent.bgn = i + 1;
      break;
    default: break;
    }
  }

  // Consume the sign character.
  bool negative = strAt(whole, 0) == '-';
  if (negative || strAt(whole, 0) == '+') whole.bgn++;

  // TODO: Handle negative zero.
  if (
    negative && typeEq(type, TYPE_INS_U1) && typeEq(type, TYPE_INS_U2) &&
    typeEq(type, TYPE_INS_U4) && typeEq(type, TYPE_INS_U8) &&
    typeEq(type, TYPE_INS_UX)) {
    otcErr(
      anr.otc, node->val,
      "Expected a `%s`, which is unsigned, but the number is negative!",
      typeName(type));
    return NULL;
  }

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
