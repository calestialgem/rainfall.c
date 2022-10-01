// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lxr/api.h"
#include "lxr/mod.h"
#include "otc/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdio.h>

/* Context of the lexing process. */
static struct {
  /* Lex to add the lexemes into. */
  Lex*        lex;
  /* Outcome to report to. */
  Outcome*    otc;
  /* Lexed source. */
  Source      src;
  /* Position of the currently lexed character. */
  char const* cur;
} lxr;

/* Add a lexeme with the given value and type. */
static void add(String const val, LexemeType const type) {
  lexAdd(lxr.lex, (Lexeme){.val = val, .type = type});
}

/* Whether there is a character to lex. */
static bool has() { return lxr.cur < srcEnd(lxr.src); }

/* Character that is lexed. */
static char get() { return *lxr.cur; }

/* Go to the next character. */
static void next() { lxr.cur++; }

/* Go to the previous character. */
static void prev() { lxr.cur--; }

/* Go back to the given position. */
static void back(char const* const old) { lxr.cur = old; }

/* String starting from the given position upto the current character. */
static String val(char const* const old) {
  return (String){.bgn = old, .end = lxr.cur};
}

/* Whether the current character exists and it equals to the given one. Consumes
 * the character if true. */
static bool take(char const c) {
  bool const res = has() && get() == c;
  if (res) next();
  return res;
}

/* Whether the next characters are the same as the given string. Consumes the
 * characters if true. */
static bool check(String const str) {
  for (ux i = 0; i < strLen(str); i++)
    if (!has() || lxr.cur[i] != strAt(str, i)) return false;
  lxr.cur += strLen(str);
  return true;
}

/* Consume the character if the first one fits the given initial predicate and
 * the remaining fit the given rest predicate. */
static bool consume(bool (*const init)(char), bool (*const rest)(char)) {
  if (!has() || !init(get())) return false;
  next();
  while (has() && rest(get())) next();
  return true;
}

/* Whether the given character is in the English alphabet. */
static bool alpha(char const c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/* Whether the given character is a decimal digit. */
static bool digit(char const c) { return c >= '0' && c <= '9'; }

/* Whether the given character is whitespace. */
static bool space(char const c) { return c == ' ' || c == '\t' || c == '\n'; }

/* Try to skip a whitespace. */
static bool whitespace() { return consume(&space, &space); }

/* Try to skip a comment. */
static bool comment() {
  if (!take('/')) return false;
  if (!take('/')) {
    prev(); // Roll back the first '/'.
    return false;
  }
  // Consume until a new line.
  while (!take('\n')) next();
  return true;
}

/* Try to lex a mark. */
static bool mark() {
  char const* const old = lxr.cur;

// Check whether there is a mark.
#define LENGTH 28
  String const MARKS[LENGTH] = {
    strOf(","),  strOf(":"),  strOf(";"),  strOf("("),  strOf(")"),
    strOf("*"),  strOf("/"),  strOf("%"),  strOf("++"), strOf("+"),
    strOf("--"), strOf("-"),  strOf("&&"), strOf("&"),  strOf("||"),
    strOf("|"),  strOf("^"),  strOf("<<"), strOf("<="), strOf("<"),
    strOf(">>"), strOf(">="), strOf(">"),  strOf("=="), strOf("="),
    strOf("!="), strOf("!"),  strOf("~")};

  for (ux i = 0; i < LENGTH; i++) {
    if (check(MARKS[i])) {
      add(val(old), LXM_COMMA + i);
      return true;
    }
  }
#undef LENGTH

  // EOF mark.
  if (take(0)) {
    add(val(old), LXM_EOF);
    return true;
  }

  return false;
}

/* Whether the given character can be start of a word. */
static bool wordInit(char const c) { return alpha(c) || c == '_'; }

/* Whether the given character can be rest of a word. */
static bool wordRest(char const c) { return alpha(c) || digit(c) || c == '_'; }

/* Try to lex a word. */
static bool word() {
  char const* const old = lxr.cur;

  // Identifier.
  if (!consume(&wordInit, &wordRest)) return false;
  String const word = val(old);

// Check whether it is a reserved identifier.
#define LENGTH 2
  String const KEYWORDS[LENGTH] = {strOf("let"), strOf("var")};

  for (ux i = 0; i < LENGTH; i++) {
    if (strEq(word, KEYWORDS[i])) {
      add(word, LXM_LET + i);
      return true;
    }
  }
#undef LENGTH

  add(word, LXM_ID);
  return true;
}

/* Whether the given character can be start of a decimal. */
static bool decimalInit(char const c) { return digit(c); }

/* Whether the given character can be rest of a decimal. */
static bool decimalRest(char const c) { return digit(c) || c == '_'; }

/* Try to lex a decimal literal. */
static bool decimal() {
  char const* const old = lxr.cur;

  // Whole part.
  // Optional sign.
  take('+') || take('-');
  if (!consume(&decimalInit, &decimalRest)) {
    back(old);
    return false;
  }

  // Fraction.
  char const* const frac = lxr.cur;
  if (take('.') && !consume(&decimalInit, &decimalRest)) back(frac);

  // Exponent.
  char const* const exp = lxr.cur;
  if (take('e') || take('E')) {
    // Optional sign.
    take('+') || take('-');
    if (!consume(&decimalInit, &decimalRest)) back(exp);
  }

  add(val(old), LXM_DEC);
  return true;
}

/* Try to skip or lex a separator. */
static bool separator() { return whitespace() || comment() || mark(); }

void lexerLex(Lex* const lex, Outcome* const otc, Source const src) {
  lxr.lex = lex;
  lxr.otc = otc;
  lxr.src = src;
  lxr.cur = srcBgn(src);

  while (has()) {
    char const* const old = lxr.cur;

    if (word() || decimal()) {
      if (separator()) continue;
      // Roll back the word or decimal that was lexed because it is not
      // separated from what comes after it.
      lxr.cur = old;
      lexPop(lxr.lex);
    }

    if (separator()) continue;

    // Unknown character! Mark all characters until a separator is found.
    next();
    String err = val(old);
    while (!separator()) {
      next();
      err.end++;
    }
    otcErr(
      lxr.otc, err, "Could not recognize %s!",
      strLen(err) > 1 ? "these characters" : "this character");
    lexAdd(lxr.lex, (Lexeme){.val = err, .type = LXM_ERR});
  }
}
