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
#include <stdio.h>
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

/* Go to the previous lexeme. */
static void prev() { psr.cur--; }

/* Return the lexeme that was parsed and go to the next lexeme. */
static Lexeme take() {
  Lexeme const old = get();
  next();
  return old;
}

/* Whether the current lexeme is of the given type. */
static bool check(LexemeType const type) { return has() && get().type == type; }

/* Return whether the current lexeme is of the given type. Goes to the next
 * lexeme if true. */
static bool consume(LexemeType const type) {
  bool const res = check(type);
  if (res) next();
  return res;
}

/* String starting from the given string upto the current lexeme. */
static String strJoin(String const old) {
  return (String){.bgn = old.bgn, .end = (psr.cur - 1)->val.end};
}

/* String starting from the given lexeme upto the current lexeme. */
static String lxmJoin(Lexeme const old) { return strJoin(old.val); }

/* String starting from the given expression node upto the current lexeme. */
static String expJoin(ExpressionNode const node) { return strJoin(node.val); }

/* Add a new expression node with the given operator, arity and value. */
static void expNodeAdd(Operator const op, ux const ary, String const val) {
  expAdd(&psr.exp, (ExpressionNode){.op = op, .ary = ary, .val = val});
}

/* Last parsed expression node. */
static ExpressionNode expNodeGet() {
  return expAt(psr.exp, expLen(psr.exp) - 1);
}

/* Last parsed expression. */
static Expression expGet() {
  Expression res = psr.exp;
  psr.exp        = expOf(0);
  printf("\n\nExpression Breakdown:\n\n");
  expTree(res, stdout);
  return res;
}

// Prototype to recursivly parse expressions.
static Result exp(ux lvl);

/* Try to parse a nullary expression node. */
static Result expNodeNull(NullaryOperator const null, bool const has) {
  Lexeme const old = get();
  if (has || !consume(null.op)) return NO;
  expNodeAdd(opOfNull(null), 0, lxmJoin(old));
  return YES;
}

/* Try to parse a prenary expression node. */
static Result
expNodePre(PrenaryOperator const pre, ux const lvl, bool const has) {
  Lexeme const old = get();
  if (has || !consume(pre.op)) return NO;
  switch (exp(lvl)) {
  case YES: expNodeAdd(opOfPre(pre), 1, lxmJoin(old)); return YES;
  case NO:
    otcErr(
      psr.otc, lxmJoin(old), "Expected an operand after the operator `%s`!",
      lxmName(pre.op));
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
}

/* Try to parse a postary expression node. */
static Result expNodePost(PostaryOperator const post, bool const has) {
  if (!has || !consume(post.op)) return NO;
  expNodeAdd(opOfPost(post), 1, expJoin(expNodeGet()));
  return YES;
}

/* Try to parse a cirnary expression node. */
static Result expNodeCir(CirnaryOperator const cir, bool const has) {
  Lexeme const old = get();
  if (has || !consume(cir.lop)) return NO;
  switch (exp(0)) {
  case YES: break;
  case NO:
    otcErr(
      psr.otc, lxmJoin(old), "Expected an operand after the opening `%s`!",
      lxmName(cir.lop));
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
  if (!consume(cir.rop)) {
    otcErr(
      psr.otc, lxmJoin(old), "Expected a closing `%s` for the opening `%s`!",
      lxmName(cir.rop), lxmName(cir.lop));
    otcInfo(*psr.otc, old.val, "Opened here.");
    return ERR;
  }
  expNodeAdd(opOfCir(cir), 1, lxmJoin(old));
  return YES;
}

/* Try to parse a binary expression node. */
static Result
expNodeBin(BinaryOperator const bin, ux const lvl, bool const has) {
  if (!has || !consume(bin.op)) return NO;
  ExpressionNode const old = expNodeGet();
  switch (exp(lvl + 1)) {
  case YES: expNodeAdd(opOfBin(bin), 2, expJoin(old)); return YES;
  case NO:
    otcErr(
      psr.otc, expJoin(old), "Expected an operand after the operator `%s`!",
      lxmName(bin.op));
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
}

/* Try to parse a variary expression node. */
static Result expNodeVar(VariaryOperator const var, bool const has) {
  Lexeme const open = get();
  if (!has || !consume(var.lop)) return NO;
  ExpressionNode const old = expNodeGet();
  switch (exp(0)) {
  case YES: break;
  case NO:
    if (consume(var.rop)) {
      expNodeAdd(opOfVar(var), 1, expJoin(old));
      return YES;
    }
    otcErr(
      psr.otc, expJoin(old), "Expected a closing `%s` for the opening `%s`!",
      lxmName(var.rop), lxmName(var.lop));
    otcInfo(*psr.otc, open.val, "Opened here.");
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
  ux ary = 2;
  while (true) {
    if (consume(var.rop)) break;
    if (!consume(var.sep)) {
      otcErr(
        psr.otc, expJoin(old), "Expected a closing `%s` for the opening `%s`!",
        lxmName(var.rop), lxmName(var.lop));
      otcInfo(*psr.otc, open.val, "Opened here.");
      return ERR;
    }
    switch (exp(0)) {
    case YES: ary++; continue;
    case NO:
      otcErr(
        psr.otc, expJoin(old), "Expected an operand after the separator `%s`!",
        lxmName(var.sep));
    case ERR: return ERR;
    default: dbgUnexpected("Unknown parse result!");
    }
  }
  expNodeAdd(opOfVar(var), ary, expJoin(old));
  return YES;
}

/* Try to parse an expression node. */
static Result expNode(ux const lvl, ux const i, bool const has) {
  Operator const op = OP_ORDER[lvl][i];
  switch (op.tag) {
  case OP_NULL: return expNodeNull(op.null, has);
  case OP_PRE: return expNodePre(op.pre, lvl, has);
  case OP_POST: return expNodePost(op.post, has);
  case OP_CIR: return expNodeCir(op.cir, has);
  case OP_BIN: return expNodeBin(op.bin, lvl, has);
  case OP_VAR: return expNodeVar(op.var, has);
  default: dbgUnexpected("Unknown operator tag!");
  }
}

/* Try to parse an expression. */
static Result exp(ux const lvl) {
  Result parsed = NO;
  for (ux i = lvl; i < OP_ORDER_LEN; i++) {
    for (ux j = 0; j < OP_LEVEL_LEN[i]; j++) {
      switch (expNode(i, j, parsed == YES)) {
      case YES:
        parsed = YES;
        // Recurse in the same precedence level. Since the loop variable is
        // incremented at the end subtract one to get the correct target level
        // in the next iteration of the outer loop.
        i      = lvl - 1;
        break;
      case NO: continue;
      case ERR: return ERR;
      default: dbgUnexpected("Unknown parse result!");
      }
      // Break on case `YES` from the inner loop.
      break;
    }
  }
  return parsed;
}

/* Try to parse a let definition statement. */
static Result let() {
  Lexeme const old = get();

  if (!consume(LXM_LET)) return NO;

  if (!check(LXM_ID)) {
    otcErr(psr.otc, lxmJoin(old), "Expected a name in definition!");
    return ERR;
  }
  Lexeme const name = take();

  if (!consume(LXM_COLON)) {
    otcErr(psr.otc, lxmJoin(old), "Expected a `:` in the definition!");
    return ERR;
  }

  switch (exp(0)) {
  case YES: break;
  case NO:
    otcErr(
      psr.otc, lxmJoin(old), "Expected a type expression in the definition!");
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
  Expression const type = expGet();

  if (!consume(LXM_EQUAL)) {
    otcErr(psr.otc, lxmJoin(old), "Expected a `=` in the definition!");
    return ERR;
  }

  switch (exp(0)) {
  case YES: break;
  case NO:
    otcErr(
      psr.otc, lxmJoin(old), "Expected a value expression in the definition!");
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
  Expression const val = expGet();

  prsAdd(
    psr.prs,
    (Statement){
      .let = {.name = name, .type = type, .val = val},
        .tag = STT_LET
  });
  return YES;
}

/* Try to parse a var definition statement. */
static Result var() {
  Lexeme const old = get();

  if (!consume(LXM_VAR)) return NO;

  if (!check(LXM_ID)) {
    otcErr(psr.otc, lxmJoin(old), "Expected a name in definition!");
    return ERR;
  }
  Lexeme const name = take();

  if (!consume(LXM_COLON)) {
    otcErr(psr.otc, lxmJoin(old), "Expected a `:` in the definition!");
    return ERR;
  }

  switch (exp(0)) {
  case YES: break;
  case NO:
    otcErr(
      psr.otc, lxmJoin(old), "Expected a type expression in the definition!");
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
  Expression const type = expGet();

  if (!consume(LXM_EQUAL)) {
    prsAdd(
      psr.prs,
      (Statement){
        .var = {.name = name, .type = type, .val = expOf(0)},
          .tag = STT_VAR
    });
    return YES;
  }

  switch (exp(0)) {
  case YES: break;
  case NO:
    otcErr(
      psr.otc, lxmJoin(old), "Expected a value expression in the definition!");
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
  Expression const val = expGet();

  prsAdd(
    psr.prs,
    (Statement){
      .var = {.name = name, .type = type, .val = val},
        .tag = STT_VAR
  });
  return YES;
}

/* Try to parse a assignment statement. */
static Result ass() {
  Lexeme const old = get();

  if (!check(LXM_ID)) return NO;
  Lexeme const name = take();

  if (!consume(LXM_EQUAL)) {
    prev(); // Rollback name.
    return NO;
  }

  switch (exp(0)) {
  case YES: break;
  case NO:
    otcErr(
      psr.otc, lxmJoin(old), "Expected a value expression in the assignment!");
  case ERR: return ERR;
  default: dbgUnexpected("Unknown parse result!");
  }
  Expression const val = expGet();

  prsAdd(
    psr.prs, (Statement){
               .ass = {.name = name, .val = val},
                 .tag = STT_ASS
  });
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

/* Report the unknown lexemes in the given range. */
static void unknown(Lexeme const unk, Lexeme const bgn) {
  if (!strLen(unk.val)) return;
  String const val = {.bgn = unk.val.bgn, .end = bgn.val.bgn};
  otcErr(
    psr.otc, val, "Expected a statement instead of %s!",
    strLen(val) > 1 ? "these characters" : "this character");
}

void parserParse(Parse* const prs, Outcome* const otc, Lex const lex) {
  psr.prs = prs;
  psr.otc = otc;
  psr.lex = lex;
  psr.cur = lexBgn(lex);
  psr.exp = expOf(0);

  Lexeme unk = {0};

  while (has()) {
    // End of the last lexeme, stored for error reporting.
    Lexeme const end = get();
    Result const res = statement();
    if (res == NO) {
      if (strLen(unk.val)) unk = end;
      next();
      continue;
    }
    // Skip until a semicolon. This is used for synchronizing over errors.
    if (res == ERR) {
      Lexeme const old = get();
      while (has() && !check(LXM_SEMI)) next();
      next(); // Consume the synchronization lexeme.
      String const skipped = lxmJoin(old);
      otcInfo(*psr.otc, skipped, "Skipped because of the previous error.");
    } else {
      // Remove the parsed statement if there is no semicolon after it.
      if (!consume(LXM_SEMI)) {
        otcErr(psr.otc, lxmJoin(end), "Expected a `;` after the statement!");
        prsPop(psr.prs);
      }
    }
    unknown(unk, end);
    unk = (Lexeme){0};
  }
  dbgExpect(get().type == LXM_EOF, "Lex does not end with EOF!");
  unknown(unk, get());
  unk = (Lexeme){0};
}
