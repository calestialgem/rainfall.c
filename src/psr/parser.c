// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "lxr/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "psr/mod.h"
#include "utl/api.h"

#include <stdarg.h>
#include <stdbool.h>
#include <vadefs.h>

/* Context of the parsing process. */
static struct {
  /* Parse to add the statements into. */
  Parse*        prs;
  /* Outcome to report to. */
  Outcome*      otc;
  /* Parsed lex. */
  Lex           lex;
  /* Position of the currently parsed lexeme. */
  Lexeme const* cur;
  /* Expression that is currently parsed. */
  Expression    exp;
} psr;

/* Result of a parsing operation. */
typedef enum {
  /* Successfull parsing. */
  YES,
  /* Does not exist. */
  NO,
  /* Found the initial pattern but had error later down the line. */
  ERR
} Result;

/* Parsing operation. */
typedef Result (*operation)();

/* Whether there is a lexeme to parse. */
static bool has() {
  // EOF should not be parsed; thus, subtract by 1.
  return psr.cur < lexEnd(psr.lex) - 1;
}

/* Lexeme that is parsed. */
static Lexeme get() { return *psr.cur; }

/* Go to the next lexeme. */
static void next() { psr.cur++; }

/* Return the lexeme that was parsed and go to the next lexeme. */
static Lexeme consume() {
  Lexeme const lxm = get();
  next();
  return lxm;
}

/* String starting from the given position upto the current lexeme. */
static String val(Lexeme const* old) {
  return (String){.bgn = old->val.bgn, .end = psr.cur->val.bgn};
}

/* Calls the given log function with the given outcome. */
#define logArgs(log, otc)      \
  do {                         \
    va_list args = NULL;       \
    va_start(args, fmt);       \
    log(otc, part, fmt, args); \
    va_end(args);              \
  } while (false)

/* Calls the given log function with the given outcome. */
#define logWholeArgs(log, otc) \
  do {                         \
    va_list args = NULL;       \
    va_start(args, fmt);       \
    log(otc, fmt, args);       \
    va_end(args);              \
  } while (false)

/* Report an error at the given part of the source file with the given
 * formatted message. */
static void err(String const part, char const* const fmt, ...) {
  logArgs(otcErr, psr.otc);
}

/* Report a warning at the given part of the source file with the given
 * formatted message. */
static void wrn(String const part, char const* const fmt, ...) {
  logArgs(otcWrn, psr.otc);
}

/* Report an information at the given part of the source file with the given
 * formatted message. */
static void info(String const part, char const* const fmt, ...) {
  logArgs(otcInfo, *psr.otc);
}

/* Whether the current lexeme is of the given type. */
static bool check(LexemeType const type) { return has() && get().type == type; }

/* Add a new expression node with the given operator, arity and value. */
static void expNodeAdd(Operator const op, ux const ary, String const val) {
  expAdd(&psr.exp, (ExpressionNode){.op = op, .ary = ary, .val = val});
}

/* Put a new expression node at the given index with the given operator, arity
 * and value. */
static void
expNodePut(ux const i, Operator const op, ux const ary, String const val) {
  expPut(&psr.exp, i, (ExpressionNode){.op = op, .ary = ary, .val = val});
}

/* Whether there is a nodes in the expression. */
static bool expNodeHas() { return expLen(psr.exp) >= 1; }

/* Last expression node. */
static ExpressionNode expNodeGet() {
  // How to know which node to stop?
  dbgUnexpected("Not implemented!");
}

// Prototype to recursivly parse expressions.
static Result exp(ux lvl);

/* Try to parse a nullary expression node. */
static Result expNodeNull(NullaryOperator const null) {
  if (!check(null.op)) return NO;
  expNodeAdd(opOfNull(null), 0, consume().val);
  return YES;
}

/* Try to parse a prenary expression node. */
static Result expNodePre(PrenaryOperator const pre, ux const lvl) {
  if (!check(pre.op)) return NO;
  next();
  Lexeme const* const old = psr.cur;
  ux const            i   = expLen(psr.exp);
  switch (exp(lvl)) {
  case YES: expNodePut(i, opOfPre(pre), 1, val(old)); return YES;
  case NO:
    err(
      val(old), "Expected an operand after the `%s` in the prenary operation!",
      lxmName(pre.op));
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
}

/* Try to parse a postary expression node. */
static Result expNodePost(PostaryOperator const post) {
  if (!check(post.op)) return NO;
  if (!expNodeHas()) return ERR;
  Lexeme const* const old = psr.cur;
  switch (exp(0)) {
  case YES: expNodeAdd(opOfPost(post), 1, val(old)); return YES;
  case NO:
    err(
      val(old), "Expected an operand after the `%s` in the postary operation!",
      lxmName(post.op));
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
}

/* Try to parse a cirnary expression node. */
static Result expNodeCir(CirnaryOperator const op) {}

/* Try to parse a binary expression node. */
static Result expNodeBin(BinaryOperator const op, ux const lvl) {}

/* Try to parse a variary expression node. */
static Result expNodeVar(VariaryOperator const op) {}

/* Try to parse an expression node. */
static Result expNode(ux const lvl, ux const i) {
  Operator const op = OP_ORDER[lvl][i];
  switch (op.tag) {
  case OP_NULL: return expNodeNull(op.null);
  case OP_PRE: return expNodePre(op.pre, lvl);
  case OP_POST: return expNodePost(op.post);
  case OP_CIR: return expNodeCir(op.cir);
  case OP_BIN: return expNodeBin(op.bin, lvl);
  case OP_VAR: return expNodeVar(op.var);
  default: dbgUnexpected("Unknown operator tag!");
  }
}

/* Try to parse an expression. */
static Result exp(ux const lvl) {
  Result parsed = NO;
  for (ux i = lvl; i < OP_ORDER_LEN; i++) {
    for (ux j = 0; j < OP_LEVEL_LEN[i]; j++) {
      switch (expNode(i, j)) {
      case YES:
        parsed = YES;
        // Recurse in the lower precedence level.
        i      = lvl - 1;
        break;
      case NO: continue;
      case ERR: return ERR;
      default: dbgUnexpected("Unknown parse result!");
      }
    }
  }
  return parsed;
}

/* Try to parse a let definition statement. */
static Result let() {
  if (!check(LXM_LET)) return NO;
  next();
  return YES;
}

/* Try to parse a var definition statement. */
static Result var() {
  if (!check(LXM_VAR)) return NO;
  next();
  return YES;
}

/* Try to parse a assignment statement. */
static Result ass() {
  if (!check(LXM_ID)) return NO;
  next();
  return YES;
}

/* Try to parse a statement. */
static Result statement() {
#define OPERATIONS_LEN 3
#define OPERATIONS \
  (operation[OPERATIONS_LEN]) { &let, &var, &ass }

  for (ux i = 0; i < OPERATIONS_LEN; i++) {
    Result const res = OPERATIONS[i]();
    if (res != NO) return res;
  }

#undef OPERATIONS
#undef OPERATIONS_LEN

  return NO;
}

/* Report the unknown lexemes. */
static void unknown(Lexeme const** const unk, Lexeme const* const bgn) {
  if (*unk == NULL) return;
  String const val = {.bgn = (*unk)->val.bgn, .end = bgn->val.bgn};
  err(
    val, "Expected a statement instead of %s!",
    strLen(val) > 1 ? "these characters" : "this character");
  *unk = NULL;
}

void parserParse(Parse* const prs, Outcome* const otc, Lex const lex) {
  psr.prs = prs;
  psr.otc = otc;
  psr.lex = lex;
  psr.cur = lexBgn(lex);
  psr.exp = expOf(0);

  Lexeme const* unk = NULL;

  while (has()) {
    // End of the last lexeme, stored for error reporting.
    Lexeme const* const end = psr.cur;
    Result const        res = statement();
    if (res == NO) {
      if (unk == NULL) unk = end;
      next();
      continue;
    }
    // Skip until a semicolon. This is used for synchronizing over errors.
    if (res == ERR) {
      Lexeme const* const old = psr.cur;
      while (has() && !check(LXM_SEMI)) next();
      next(); // Consume the synchronization lexeme.
      String const skipped = val(old);
      info(skipped, "Skipped because of the previous error.");
    }
    unknown(&unk, end);
  }
  dbgExpect(check(LXM_EOF), "Lex does not end with EOF!");
  unknown(&unk, psr.cur);
}
