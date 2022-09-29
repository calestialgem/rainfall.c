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

/* Go the the next lexeme. */
static void next() { psr.cur++; }

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
static bool check(LexemeType const type) { return get().type == type; }

/* Consume the current lexeme if it exists and it is of the given type. Returns
 * whether the lexeme was consumed. */
static bool consume(LexemeType const type) {
  if (!(has() && check(type))) return false;
  next();
  return true;
}

/* Try to parse a let definition statement. */
static Result let() {
  if (!consume(LXM_LET)) return NO;
  return YES;
}

/* Try to parse a var definition statement. */
static Result var() {
  if (!consume(LXM_VAR)) return NO;
  return YES;
}

/* Try to parse a assignment statement. */
static Result ass() {
  if (!consume(LXM_ID)) return NO;
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
