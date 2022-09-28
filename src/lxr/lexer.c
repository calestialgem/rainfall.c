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

/* String starting from the given position upto the current character. */
static String val(char const* old) {
  return (String){.bgn = old, .end = lxr.cur};
}

/* Consume the character if the first one fits the given initial predicate and
 * the remaining fit the given rest predicate. */
static bool consume(bool (*const init)(char), bool (*const rest)(char)) {
  if (!init(get())) return false;
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
  if (get() != '/') return false;
  next();
  if (!(has() && get() == '/')) {
    prev(); // Roll back the first '/'.
    return false;
  }

  while (has() && get() != '\n') next();
  next(); // Consume the new line aswell.
  return true;
}

/* Try to lex a mark. */
static bool mark() {
  char const* const old = lxr.cur;

#define MARKS  "=:;()+-*/%"
#define LENGTH 11 // Null-terminator is lexed as EOF

  char const c = get();

  for (ux i = 0; i < LENGTH; i++) {
    if (c == MARKS[i]) {
      next();
      add(val(old), LXR_EQUAL + i);
      return true;
    }
  }

#undef MARKS
#undef LENGTH

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
      add(word, LXR_LET + i);
      return true;
    }
  }
#undef LENGTH

  add(word, LXR_ID);
  return true;
}

/* Whether the given character can be start of a decimal. */
static bool decimalInit(char const c) { return digit(c); }

/* Whether the given character can be rest of a decimal. */
static bool decimalRest(char const c) { return digit(c) || c == '_'; }

/* Try to lex a word. */
static bool decimal() {
  char const* const old = lxr.cur;

  // Whole part.
  if (!consume(&decimalInit, &decimalRest)) return false;

  // Fraction.
  if (has() && get() == '.') {
    next();
    // Roll back '.' if cannot consume the fraction.
    if (!(has() && consume(&decimalInit, &decimalRest))) prev();
  }

  // Exponent.
  if (has() && (get() == 'e' || get() == 'E')) {
    next();
    if (!has()) {
      prev(); // Roll back 'e' or 'E'.
    } else {
      if (get() == '+' || get() == '-') {
        next();
        if (!(has() && consume(&decimalInit, &decimalRest))) {
          prev(); // Roll back 'e' or 'E'.
          prev(); // Roll back '+' or '-'.
        }
      } else if (!consume(&decimalInit, &decimalRest)) {
        prev(); // Roll back 'e' or 'E'.
      }
    }
  }

  add(val(old), LXR_DEC);
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

    if (separator()) continue;

    if (word() || decimal()) {
      if (separator()) continue;
      // Roll back the word or decimal that was lexed because it is not
      // separated from what comes after it.
      lxr.cur = old;
      lexPop(lxr.lex);
    }

    // Unknown character! Mark all characters until a separator is found.
    next();
    String err = val(old);
    while (!separator()) {
      next();
      err.end++;
    }
    otcErr(
      lxr.otc, err, "Coult not recognize %s!",
      strLen(err) > 1 ? "these characters" : "this character");
    lexAdd(lxr.lex, (Lexeme){.val = err, .type = LXR_ERR});
  }
}
