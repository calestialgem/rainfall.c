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
  mapPut(&anr.map, sym.name, tblLen(*anr.tbl));
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

/* Add an evaluation node with the given node and type. */
static void evaluate(ExpressionNode const* const node, Type const type) {
  EvaluationNode const evl = {.exp = *node, .type = type};
  evlAdd(&anr.evl, evl);
}

/* Add an evaluation node with the given node, type and value. */
static void evaluateVal(
  ExpressionNode const* const node, Type const type, Value const val) {
  EvaluationNode const evl = {
    .exp = *node, .type = type, .val = val, .has = true};
  evlAdd(&anr.evl, evl);
}

/* Version of `evaluateNull` that takes `OP_ACS`s. */
static ExpressionNode const* evaluateAcs(
  ExpressionNode const* const node, Type const type, bool const expect) {
  MapEntry const* const e = mapGet(anr.map, node->val);
  if (!e) {
    otcErr(anr.otc, node->val, "Unknown symbol!");
    return NULL;
  }
  Symbol const acs = tblAt(*anr.tbl, e->val);
  if (expect) {
    if (!typeCnv(acs.type, type)) {
      otcErr(
        anr.otc, node->val, "Expected a `%s`, but `%.*s` is a `%s`!",
        typeName(type), (int)strLen(node->val), node->val.bgn,
        typeName(acs.type));
      return NULL;
    }
    if (acs.has) evaluateVal(node, type, valCnv(acs.type, type, acs.val));
    else evaluate(node, type);
  } else {
    if (acs.has) evaluateVal(node, acs.type, acs.val);
    else evaluate(node, acs.type);
  }
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

#define evaluateNeg(TYPE, TYPE_VAL) \
  if (neg) val.TYPE_VAL = -val.TYPE_VAL;

#define errorNeg(TYPE, TYPE_VAL)                                         \
  if (neg && evl.val.TYPE_VAL != 0) {                                    \
    otcErr(                                                              \
      anr.otc, node->val,                                                \
      "Expected unsigned integer type `%s`, but the value is negative!", \
      typeName(type));                                                   \
    return NULL;                                                         \
  }

#define evaluateNum(TYPE, TYPE_VAL, TYPE_INS, NUM_PARSE, NEG_PARSE) \
  do {                                                              \
    Value val = {.TYPE_VAL = (TYPE)NUM_PARSE(num)};                 \
    NEG_PARSE(TYPE, TYPE_VAL);                                      \
    evaluateVal(node, TYPE_INS, val);                               \
    numFree(&num);                                                  \
    return node - 1;                                                \
  } while (false)

  if (!expect) {
    if (numIsInt(num)) {
      int const cmp = numCmp(num, (uint64_t)INT_MAX + 1);
      if (cmp != 1 && (cmp != 0 || neg))
        evaluateNum(int, vint, TYPE_INS_INT, numAsInt, evaluateNeg);
    }
    evaluateNum(double, vdouble, TYPE_INS_DOUBLE, numAsDouble, evaluateNeg);
  }

#define checkInt()                                                    \
  if (!numIsInt(num)) {                                               \
    otcErr(                                                           \
      anr.otc, node->val,                                             \
      "Expected integer type `%s`, but the value is not an integer!", \
      typeName(type));                                                \
    return NULL;                                                      \
  }

#define parseSigned(TYPE, TYPE_VAL, TYPE_INS, MAX)                          \
  if (typeEq(type, TYPE_INS)) {                                             \
    checkInt();                                                             \
    int const cmp = numCmp(num, (uint64_t)(MAX) + 1);                       \
    if (cmp == 1 || (cmp == 0 && !neg)) {                                   \
      otcErr(anr.otc, node->val, "Out of bounds of `%s`!", typeName(type)); \
      return NULL;                                                          \
    }                                                                       \
    evaluateNum(TYPE, TYPE_VAL, TYPE_INS, numAsInt, evaluateNeg);           \
  }

#define parseUnsigned(TYPE, TYPE_VAL, TYPE_INS, MAX)                        \
  if (typeEq(type, TYPE_INS)) {                                             \
    checkInt();                                                             \
    int const cmp = numCmp(num, MAX);                                       \
    if (cmp == 1) {                                                         \
      otcErr(anr.otc, node->val, "Out of bounds of `%s`!", typeName(type)); \
      return NULL;                                                          \
    }                                                                       \
    evaluateNum(TYPE, TYPE_VAL, TYPE_INS, numAsInt, errorNeg);              \
  }

#define parseFloat(TYPE, TYPE_VAL, TYPE_INS, NUM_PARSE) \
  if (typeEq(type, TYPE_INS))                           \
    evaluateNum(TYPE, TYPE_VAL, TYPE_INS, NUM_PARSE, evaluateNeg);

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

#undef parseFloat
#undef parseUnsigned
#undef parseSigned
#undef checkInt
#undef evaluateNum
#undef errorNeg
#undef evaluateNeg

  otcErr(anr.otc, node->val, "Expected a `%s`, not a number!", typeName(type));
  numFree(&num);
  return NULL;
}

/* Version of `evaluateNode` that takes `NullaryOperator`s. */
static ExpressionNode const* evaluateNull(
  ExpressionNode const* const node, Type const type, bool const expect) {
  if (opEq(node->op, OP_ACS)) return evaluateAcs(node, type, expect);
  if (opEq(node->op, OP_DEC)) return evaluateDec(node, type, expect);
  dbgUnexpected("Unknown nullary operator!");
  return NULL;
}

/* Arithmetic representation of the given type with the given name. Reports
 * error if the given type is not arithmetic. */
#define arithmetic(type, name)                                    \
  Arithmetic const name = ariOf(type);                            \
  if (!ariValid(name)) {                                          \
    otcErr(                                                       \
      anr.otc, node->val, "Expected a number, but found a `%s`!", \
      typeName(type));                                            \
    return NULL;                                                  \
  }

/* Checks whether the given source type fits into the given destination type. */
#define arithmeticFits(src, des)                                     \
  if (!ariFits(src, des)) {                                          \
    otcErr(                                                          \
      anr.otc, node->val,                                            \
      "Result of `%s` is a `%s`, which does not fit in a `%s`!",     \
      opName(node->op), typeName((src).type), typeName((des).type)); \
    return NULL;                                                     \
  }

/* Checks whether the given arithmetic is an integer. */
#define arithmeticInt(ari)                                          \
  if (!ariInt(ari)) {                                               \
    otcErr(                                                         \
      anr.otc, node->val, "Expected an integer, but found a `%s`!", \
      typeName((ari).type));                                        \
    return NULL;                                                    \
  }

/* Version of `evaluateNode` that takes `PrenaryOperator`s. */
static ExpressionNode const* evaluatePre(
  ExpressionNode const* const node, Type const type, bool const expect) {
  if (
    opEq(node->op, OP_POS) || opEq(node->op, OP_NEG) ||
    opEq(node->op, OP_NOT) || opEq(node->op, OP_BNT)) {
    ExpressionNode const* const opExp = evaluateNode(node - 1, type, expect);
    if (opExp == NULL) return NULL;
    EvaluationNode const opEvl = root();
    arithmetic(opEvl.type, op);
    Arithmetic const src = ariLarger(op, ARI_INT);
    if (expect) {
      arithmetic(type, des);
      arithmeticFits(src, des);
      evaluate(node, type);
    } else {
      evaluate(node, src.type);
    }
    return opExp;
  }
  if (opEq(node->op, OP_PIN) || opEq(node->op, OP_PDE)) {
    ExpressionNode const* const opExp =
      evaluateNode(node - 1, TYPE_INS_VOID, false);
    if (opExp == NULL) return NULL;
    EvaluationNode const opEvl = root();
    arithmetic(opEvl.type, op);
    if (expect && !typeCnv(type, TYPE_INS_VOID)) {
      otcErr(
        anr.otc, node->val, "Result of `%s` is a `%s`, but expected a `%s`!",
        opName(node->op), typeName(TYPE_INS_VOID), typeName(type));
      return NULL;
    }
    evaluate(node, TYPE_INS_VOID);
    return opExp;
  }
  dbgUnexpected("Unknown prenary operator!");
  return NULL;
}

/* Version of `evaluateNode` that takes `PostaryOperator`s. */
static ExpressionNode const* evaluatePost(
  ExpressionNode const* const node, Type const type, bool const expect) {
  ExpressionNode const* const opExp =
    evaluateNode(node - 1, TYPE_INS_VOID, false);
  if (opExp == NULL) return NULL;
  if (!opEq(node->op, OP_SIN) && !opEq(node->op, OP_SDE)) {
    dbgUnexpected("Unknown postary operator!");
    return NULL;
  }
  EvaluationNode const opEvl = root();
  arithmetic(opEvl.type, op);
  if (expect && !typeEq(type, TYPE_INS_VOID)) {
    otcErr(
      anr.otc, node->val, "Result of `%s` is a `%s`, but expected a `%s`!",
      opName(node->op), typeName(TYPE_INS_VOID), typeName(type));
    return NULL;
  }
  evaluate(node, TYPE_INS_VOID);
  return opExp;
}

/* Version of `evaluateNode` that takes `CirnaryOperator`s. */
static ExpressionNode const* evaluateCir(
  ExpressionNode const* const node, Type const type, bool const expect) {
  ExpressionNode const* const opExp = evaluateNode(node - 1, type, expect);
  if (opExp == NULL) return NULL;
  if (!opEq(node->op, OP_GRP)) {
    dbgUnexpected("Unknown cirnary operator!");
    return NULL;
  }
  EvaluationNode const opEvl = root();
  if (expect) {
    if (!typeCnv(opEvl.type, type)) {
      otcErr(
        anr.otc, node->val,
        "Expected a `%s`, but found `%s`, which is not convertible!",
        typeName(type), typeName(opEvl.type));
      return NULL;
    }
    evaluate(node, type);
  } else {
    evaluate(node, opEvl.type);
  }
  return opExp;
}

/* Check whether the given destination expression is assignable to with the
 * given source type. Reports the given string on error. */
static bool
checkAssignment(String const str, EvaluationNode const des, Type const src) {
  bool res = true;
  if (!typeCnv(src, des.type)) {
    otcErr(
      anr.otc, str, "Cannot assign a `%s` to a `%s`!", typeName(src),
      typeName(des.type));
    res = false;
  }
  if (!opEq(des.exp.op, OP_ACS)) {
    otcErr(anr.otc, des.exp.val, "Assigned expression is not a symbol!");
    res = false;
  } else {
    MapEntry const* const e = mapGet(anr.map, des.exp.val);
    dbgExpect(e, "Access operation was not valid!");
    Symbol const sym = tblAt(*anr.tbl, e->val);
    switch (sym.tag) {
    case SYM_BIND:
      otcErr(anr.otc, des.exp.val, "Cannot reassign a binding!");
      res = false;
      break;
    case SYM_VAR: break;
    case SYM_TYPE:
      otcErr(anr.otc, des.exp.val, "Cannot assign to a type symbol!");
      res = false;
      break;
    default: dbgUnexpected("Unknown symbol tag!");
    }
  }
  return res;
}

/* Version of `evaluateNode` that takes `BinaryOperator`s. */
static ExpressionNode const* evaluateBin(
  ExpressionNode const* const node, Type const type, bool const expect) {
  if (
    opEq(node->op, OP_MUL) || opEq(node->op, OP_DIV) ||
    opEq(node->op, OP_REM) || opEq(node->op, OP_ADD) ||
    opEq(node->op, OP_SUB)) {
    ExpressionNode const* const ropExp = evaluateNode(node - 1, type, expect);
    if (ropExp == NULL) return NULL;
    EvaluationNode const ropEvl = root();
    arithmetic(ropEvl.type, rop);

    ExpressionNode const* const lopExp = evaluateNode(ropExp, type, expect);
    if (lopExp == NULL) return NULL;
    EvaluationNode const lopEvl = root();
    arithmetic(lopEvl.type, lop);

    Arithmetic const rsrc = ariLarger(rop, ARI_INT);
    Arithmetic const lsrc = ariLarger(lop, ARI_INT);
    Arithmetic const src  = ariLarger(lsrc, rsrc);
    if (expect) {
      arithmetic(type, des);
      arithmeticFits(src, des);
      evaluate(node, type);
    } else {
      evaluate(node, src.type);
    }
    return lopExp;
  }
  if (
    opEq(node->op, OP_LSH) || opEq(node->op, OP_RSH) ||
    opEq(node->op, OP_AND) || opEq(node->op, OP_XOR) ||
    opEq(node->op, OP_ORR)) {
    ExpressionNode const* const ropExp = evaluateNode(node - 1, type, expect);
    if (ropExp == NULL) return NULL;
    EvaluationNode const ropEvl = root();
    arithmetic(ropEvl.type, rop);
    arithmeticInt(rop);

    ExpressionNode const* const lopExp = evaluateNode(ropExp, type, expect);
    if (lopExp == NULL) return NULL;
    EvaluationNode const lopEvl = root();
    arithmetic(lopEvl.type, lop);
    arithmeticInt(lop);

    Arithmetic const rsrc = ariLarger(rop, ARI_INT);
    Arithmetic const lsrc = ariLarger(lop, ARI_INT);
    Arithmetic const src  = ariLarger(lsrc, rsrc);
    if (expect) {
      arithmetic(type, des);
      arithmeticFits(src, des);
      evaluate(node, type);
    } else {
      evaluate(node, src.type);
    }
    return lopExp;
  }
  if (
    opEq(node->op, OP_SMT) || opEq(node->op, OP_STE) ||
    opEq(node->op, OP_LGT) || opEq(node->op, OP_LTE) ||
    opEq(node->op, OP_EQU) || opEq(node->op, OP_NEQ) ||
    opEq(node->op, OP_LAN) || opEq(node->op, OP_LOR)) {
    ExpressionNode const* const ropExp = evaluateNode(node - 1, type, expect);
    if (ropExp == NULL) return NULL;
    EvaluationNode const ropEvl = root();
    arithmetic(ropEvl.type, rop);

    ExpressionNode const* const lopExp = evaluateNode(ropExp, type, expect);
    if (lopExp == NULL) return NULL;
    EvaluationNode const lopEvl = root();
    arithmetic(lopEvl.type, lop);

    if (expect && !typeCnv(TYPE_INS_BOOL, type)) {
      otcErr(
        anr.otc, node->val, "Result of `%s` is a `%s`, but expected a `%s`!",
        opName(node->op), typeName(TYPE_INS_BOOL), typeName(type));
      return NULL;
    }
    evaluate(node, TYPE_INS_BOOL);
    return lopExp;
  }
  if (opEq(node->op, OP_ASS)) {
    ExpressionNode const* const ropExp =
      evaluateNode(node - 1, TYPE_INS_VOID, false);
    if (ropExp == NULL) return NULL;
    EvaluationNode const ropEvl = root();

    ExpressionNode const* const lopExp =
      evaluateNode(ropExp, TYPE_INS_VOID, false);
    if (lopExp == NULL) return NULL;
    EvaluationNode const lopEvl = root();

    checkAssignment(node->val, lopEvl, ropEvl.type);

    if (expect && !typeCnv(TYPE_INS_VOID, type)) {
      otcErr(
        anr.otc, node->val, "Result of `%s` is a `%s`, but expected a `%s`!",
        opName(node->op), typeName(TYPE_INS_VOID), typeName(type));
      return NULL;
    }
    evaluate(node, TYPE_INS_VOID);
    return lopExp;
  }
  if (
    opEq(node->op, OP_MLA) || opEq(node->op, OP_DVA) ||
    opEq(node->op, OP_RMA) || opEq(node->op, OP_ADA) ||
    opEq(node->op, OP_SBA)) {
    ExpressionNode const* const ropExp =
      evaluateNode(node - 1, TYPE_INS_VOID, false);
    if (ropExp == NULL) return NULL;
    EvaluationNode const ropEvl = root();
    arithmetic(ropEvl.type, rop);

    ExpressionNode const* const lopExp =
      evaluateNode(ropExp, TYPE_INS_VOID, false);
    if (lopExp == NULL) return NULL;
    EvaluationNode const lopEvl = root();
    arithmetic(lopEvl.type, lop);

    Arithmetic const rsrc = ariLarger(rop, ARI_INT);
    Arithmetic const lsrc = ariLarger(lop, ARI_INT);
    Arithmetic const src  = ariLarger(lsrc, rsrc);

    checkAssignment(node->val, lopEvl, src.type);

    if (expect && !typeCnv(TYPE_INS_VOID, type)) {
      otcErr(
        anr.otc, node->val, "Result of `%s` is a `%s`, but expected a `%s`!",
        opName(node->op), typeName(TYPE_INS_VOID), typeName(type));
      return NULL;
    }
    evaluate(node, TYPE_INS_VOID);
    return lopExp;
  }
  if (
    opEq(node->op, OP_LSA) || opEq(node->op, OP_RSA) ||
    opEq(node->op, OP_ANA) || opEq(node->op, OP_XRA) ||
    opEq(node->op, OP_ORA)) {
    ExpressionNode const* const ropExp =
      evaluateNode(node - 1, TYPE_INS_VOID, false);
    if (ropExp == NULL) return NULL;
    EvaluationNode const ropEvl = root();
    arithmetic(ropEvl.type, rop);
    arithmeticInt(rop);

    ExpressionNode const* const lopExp =
      evaluateNode(ropExp, TYPE_INS_VOID, false);
    if (lopExp == NULL) return NULL;
    EvaluationNode const lopEvl = root();
    arithmetic(lopEvl.type, lop);
    arithmeticInt(lop);

    Arithmetic const rsrc = ariLarger(rop, ARI_INT);
    Arithmetic const lsrc = ariLarger(lop, ARI_INT);
    Arithmetic const src  = ariLarger(lsrc, rsrc);

    checkAssignment(node->val, lopEvl, src.type);

    if (expect && !typeCnv(TYPE_INS_VOID, type)) {
      otcErr(
        anr.otc, node->val, "Result of `%s` is a `%s`, but expected a `%s`!",
        opName(node->op), typeName(TYPE_INS_VOID), typeName(type));
      return NULL;
    }
    evaluate(node, TYPE_INS_VOID);
    return lopExp;
  }
  dbgUnexpected("Unknown binary operator!");
  return NULL;
}

/* Version of `evaluateNode` that takes `VariaryOperator`s. */
static ExpressionNode const* evaluateVar(
  ExpressionNode const* const node, Type const type, bool const expect) {
  if (!opEq(node->op, OP_CLL)) {
    dbgUnexpected("Unknown variary operator!");
    return NULL;
  }
  otcWrn(anr.otc, node->val, "Function call is not yet implemented.");
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
  Type type   = TYPE_INS_VOID;
  bool expect = false;
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
  Type type   = TYPE_INS_VOID;
  bool expect = false;
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
  if (evaluateExp(exp.exp, TYPE_INS_VOID, true)) {
    otcWrn(
      anr.otc, expStr(exp.exp),
      "Expression statement in the global scope is skipped for now. Later it "
      "will not be allowed.");
    Evaluation const evl = get();
    printf("Expression Statement:\n");
    evlTree(evl, stdout);
    printf("\n");
  }
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
