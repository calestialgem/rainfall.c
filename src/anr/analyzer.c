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
  Evaluation const res = anr.evl;
  anr.evl              = evlOf(0);
  return res;
}

/* Root of the current evaluation. */
static EvaluationNode root() { return evlRoot(anr.evl); }

/* Add a new symbol with the given name with the current evaluation. */
static void add(String const name) {
  mapPut(&anr.map, name, tblLen(*anr.tbl));
  tblAdd(anr.tbl, (Symbol){.name = name, .evl = evlGet()});
}

/* Add the given type as a built-in. */
static void prepareType(Type const type) {
  String const name = strOf(typeName(type));
  evlAdd(
    &anr.evl, (EvaluationNode){
                .type = TYPE_INS_META, .val = {.meta = type}, .has = true});
  add(name);
}

/* Add an operation for the given prenary operator with the given output type
 * and the given input type. */
static void preparePre(Operator const op, Type const out, Type const in) {
  tblOpnAdd(anr.tbl, (Operation){.pre = {.in = in}, .op = op, .out = out});
}

/* Add an operation for the given postary operator with the given output type
 * and the given input type. */
static void preparePost(Operator const op, Type const out, Type const in) {
  tblOpnAdd(anr.tbl, (Operation){.post = {.in = in}, .op = op, .out = out});
}

/* Add an operation for the given cirnary operator with the given output type
 * and the given input type. */
static void prepareCir(Operator const op, Type const out, Type const in) {
  tblOpnAdd(anr.tbl, (Operation){.cir = {.in = in}, .op = op, .out = out});
}

/* Add an operation for the given binary operator with the given output type and
 * the given input types. */
static void
prepareBin(Operator const op, Type const out, Type const lin, Type const rin) {
  tblOpnAdd(
    anr.tbl, (Operation){
               .bin = {.lin = lin, .rin = rin},
                 .op = op, .out = out
  });
}

/* Add an operation for the given variary operator with the given output type
 * and the given input types. */
static void prepareVar(
  Operator const op, Type const out, Type const fin, Type* const rin,
  iptr const ary) {
  tblOpnAdd(
    anr.tbl,
    (Operation){
      .var = {.fin = fin, .rin = rin, .ary = ary},
        .op = op, .out = out
  });
}

/* Add a conversion from the given source type to the given destination type.
 */
static void prepareCnv(Type const src, Type const des) {
  tblCnvAdd(anr.tbl, (TypeConversion){.from = src, .to = des});
}

/* Add the built-in symbols. */
static void prepare() {
  for (iptr i = 0; i < TYPE_BUILT_LEN; i++) prepareType(TYPE_BUILT[i]);

  // Skips meta and void, for all scalars.
  for (iptr i = 2; i < TYPE_BUILT_LEN; i++)
    prepareCir(OP_GRP, TYPE_BUILT[i], TYPE_BUILT[i]);

  // Skips meta and void, for all scalars.
  for (iptr i = 2; i < TYPE_BUILT_LEN; i++)
    prepareCnv(TYPE_BUILT[i], TYPE_INS_VOID);

  prepareCnv(TYPE_INS_BOOL, TYPE_INS_BYTE);
  prepareCnv(TYPE_INS_BOOL, TYPE_INS_INT);
  prepareCnv(TYPE_INS_BOOL, TYPE_INS_IPTR);
  prepareCnv(TYPE_INS_BOOL, TYPE_INS_FLOAT);
  prepareCnv(TYPE_INS_BOOL, TYPE_INS_DOUBLE);

  prepareCnv(TYPE_INS_BYTE, TYPE_INS_INT);
  prepareCnv(TYPE_INS_BYTE, TYPE_INS_IPTR);
  prepareCnv(TYPE_INS_BYTE, TYPE_INS_FLOAT);
  prepareCnv(TYPE_INS_BYTE, TYPE_INS_DOUBLE);

  prepareCnv(TYPE_INS_INT, TYPE_INS_IPTR);
  prepareCnv(TYPE_INS_INT, TYPE_INS_FLOAT);
  prepareCnv(TYPE_INS_INT, TYPE_INS_DOUBLE);

  prepareCnv(TYPE_INS_IPTR, TYPE_INS_FLOAT);
  prepareCnv(TYPE_INS_IPTR, TYPE_INS_DOUBLE);

  prepareCnv(TYPE_INS_FLOAT, TYPE_INS_DOUBLE);
}

// Prototype for recursive check of expressions.
static ExpressionNode const* checkNode(ExpressionNode const* node, Type type);

/* Whether the given source type can be converted to the given destination type.
 */
static bool checkType(Type const src, Type const des) {
  return tblCnv(*anr.tbl, src, des);
}

/* Version of `checkNull` that takes `OP_ACS`s. */
static ExpressionNode const*
checkAcs(ExpressionNode const* const node, Type const type) {
  MapEntry const* const e = mapGet(anr.map, node->val);
  if (!e) {
    otcErr(anr.otc, node->val, "Unknown symbol!");
    return NULL;
  }
  EvaluationNode const acs = evlRoot(tblAt(*anr.tbl, e->val).evl);
  if (!checkType(acs.type, type)) {
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
  bool const neg = strAt(node->val, 0) == '-';
  Number     num = numOfZero();
  if (numSetDec(
        &num, (String){
                .bgn = (neg || strAt(node->val, 0) == '+') + node->val.bgn,
                .end = node->val.end})) {
    otcErr(anr.otc, node->val, "Exponent is too long!");
    return NULL;
  }

#define parseSigned(TYPE, TYPE_VAL, TYPE_INS, MAX)                          \
  if (typeEq(type, TYPE_INS)) {                                             \
    if (!numIsInt(num)) {                                                   \
      otcErr(                                                               \
        anr.otc, node->val, "Not an integer for integer type `%s`!",        \
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

#define parseUnsigned(TYPE, TYPE_VAL, TYPE_INS, MAX)                          \
  if (typeEq(type, TYPE_INS)) {                                               \
    if (!numInt(num)) {                                                       \
      otcErr(                                                                 \
        anr.otc, node->val, "Not an integer for integer type `%s`!",          \
        typeName(type));                                                      \
      return NULL;                                                            \
    }                                                                         \
    int const cmp = numCmp(num, MAX);                                         \
    if (cmp == 1) {                                                           \
      otcErr(anr.otc, node->val, "Out of bounds of `%s`!", typeName(type));   \
      return NULL;                                                            \
    }                                                                         \
    EvaluationNode evl = {                                                    \
      .exp  = *node,                                                          \
      .type = type,                                                           \
      .val  = {.TYPE_VAL = (TYPE)numU8(num)},                                 \
      .has  = true};                                                           \
    if (neg && evl.val.TYPE_VAL != 0) {                                       \
      otcErr(                                                                 \
        anr.otc, node->val, "Negative value for unsigned integer type `%s`!", \
        typeName(type));                                                      \
      return NULL;                                                            \
    }                                                                         \
    evlAdd(&anr.evl, evl);                                                    \
    goto success;                                                             \
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

/* Version of `checkNode` that takes `NullaryOperator`s. */
static ExpressionNode const*
checkNull(ExpressionNode const* const node, Type const type) {
  if (opEq(node->op, OP_ACS)) return checkAcs(node, type);
  if (opEq(node->op, OP_DEC)) return checkDec(node, type);
  dbgUnexpected("Unknown nullary operator!");
  return NULL;
}

/* Version of `checkPre` that takes `OP_POS`s. */
static ExpressionNode const* checkPos(
  ExpressionNode const* const node, ExpressionNode const* const operand,
  Type const type) {
  EvaluationNode evl = root();
  evl.exp            = *node;
  evlAdd(&anr.evl, evl);
  return operand;
}

/* Version of `checkNode` that takes `PrenaryOperator`s. */
static ExpressionNode const*
checkPre(ExpressionNode const* const node, Type const type) {
  ExpressionNode const* const operand = checkNode(node - 1, type);
  if (operand == NULL) return NULL;
  if (opEq(node->op, OP_POS)) return checkPos(node, operand, type);
  dbgUnexpected("Unknown prenary operator!");
  return NULL;
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
