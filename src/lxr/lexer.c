// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lxr/api.h"
#include "lxr/mod.h"
#include "otc/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stddef.h>

/* Context of the lexing process. */
typedef struct {
  /* Lex to add the lexemes into. */
  Lex*        lex;
  /* Lexed source. */
  Source*     source;
  /* Position of the currently lexed character. */
  char const* current;
} Lexer;

String markNames[MARK_COUNT];
String keywordNames[KEYWORD_COUNT];

/* Add a lexeme with the given value and type. */
#define push(section_, tag_) \
  pushLexeme(l->lex, (Lexeme){.section = (section_), .tag = (tag_)})

/* Whether there is a character to lex. */
#define has() (l->current < l->source->contents.after)

/* Character that is lexed. */
#define get() (*l->current)

/* Go to the next character. */
#define next() (l->current++)

/* Go to the previous character. */
#define previous() (l->current--)

/* Go back to the given position. */
#define back(old) \
  do { l->current = (old); } while (false)

/* Section of the source file starting from the given position upto the current
 * character. */
#define section(old) stringOf(old, l->current)

/* Whether the current character exists and it equals to the given one. Consumes
 * the character if true. */
static bool take(Lexer* l, char c) {
  bool res = has() && get() == c;
  if (res) next();
  return res;
}

/* Whether the next characters are the same as the given string. Consumes the
 * characters if true. */
static bool check(Lexer* l, String s) {
  for (size_t i = 0; i < characters(s); i++)
    if (!has() || l->current[i] != s.first[i]) return false;
  l->current += characters(s);
  return true;
}

/* Consume the character if the first one fits the given initial predicate and
 * the remaining fit the given rest predicate. */
static bool consume(Lexer* l, bool (*init)(char), bool (*rest)(char)) {
  if (!has() || !init(get())) return false;
  next();
  while (has() && rest(get())) next();
  return true;
}

/* Whether the given character is in the English alphabet. */
static bool alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/* Whether the given character is a decimal digit. */
static bool digit10(char c) { return c >= '0' && c <= '9'; }

/* Whether the given character is whitespace. */
static bool space(char c) { return c == ' ' || c == '\t' || c == '\n'; }

/* Try to skip a whitespace. */
static bool whitespace(Lexer* l) { return consume(l, &space, &space); }

/* Try to skip a comment. */
static bool comment(Lexer* l) {
  if (!take(l, '/')) return false;
  if (!take(l, '/')) {
    previous(); // Roll back the first '/'.
    return false;
  }
  // Consume until a new line.
  while (!take(l, '\n')) next();
  return true;
}

/* Try to lex a mark. */
static bool mark(Lexer* l) {
  char const* old = l->current;

  // Check whether there is a mark.
  for (size_t i = 0; i < MARK_COUNT; i++) {
    if (check(l, markNames[i])) {
      push(section(old), MARK_FIRST + i);
      return true;
    }
  }

  // EOF mark.
  if (take(l, 0)) {
    push(section(old), LEXEME_EOF);
    return true;
  }

  return false;
}

/* Whether the given character can be start of a word. */
static bool wordInit(char c) { return alpha(c) || c == '_'; }

/* Whether the given character can be rest of a word. */
static bool wordRest(char c) { return alpha(c) || digit10(c) || c == '_'; }

/* Try to lex a word. */
static bool word(Lexer* l) {
  char const* old = l->current;

  // Identifier.
  if (!consume(l, &wordInit, &wordRest)) return false;
  String word = section(old);

  // Check whether it is a reserved identifier.
  for (size_t i = 0; i < KEYWORD_COUNT; i++) {
    if (equalStrings(word, keywordNames[i])) {
      push(word, KEYWORD_FIRST + i);
      return true;
    }
  }
#undef LENGTH

  push(word, LEXEME_IDENTIFIER);
  return true;
}

/* Whether the given character can be start of a decimal. */
static bool decimalInit(char c) { return digit10(c); }

/* Whether the given character can be rest of a decimal. */
static bool decimalRest(char c) { return digit10(c) || c == '_'; }

/* Try to lex a decimal literal. */
static bool decimal(Lexer* l) {
  char const* old = l->current;

  // Whole part.
  // Optional sign.
  take(l, '+') || take(l, '-');
  if (!consume(l, &decimalInit, &decimalRest)) {
    back(old);
    return false;
  }

  // Fraction.
  char const* frac = l->current;
  if (take(l, '.') && !consume(l, &decimalInit, &decimalRest)) back(frac);

  // Exponent.
  char const* exp = l->current;
  if (take(l, 'e') || take(l, 'E')) {
    // Optional sign.
    take(l, '+') || take(l, '-');
    if (!consume(l, &decimalInit, &decimalRest)) back(exp);
  }

  push(section(old), LEXEME_DECIMAL);
  return true;
}

/* Try to skip or lex a separator. */
static bool separator(Lexer* l) {
  return whitespace(l) || comment(l) || mark(l);
}

/* Run the given lexer context. */
static void run(Lexer* l) {
  while (has()) {
    char const* old = l->current;

    if (word(l) || decimal(l)) {
      if (separator(l)) continue;
      // Roll back the word or decimal that was lexed because it is not
      // separated from what comes after it.
      back(old);
      popLexeme(l->lex);
    }

    if (separator(l)) continue;

    // Unknown character! Mark all characters until a separator is found.
    next();
    String err = section(old);
    while (!separator(l)) {
      next();
      err.after++;
    }
    highlightError(
      l->source, err, "Could not recognize %s!",
      characters(err) > 1 ? "these characters" : "this character");
    push(err, LEXEME_ERROR);
  }
}

void lex(Lex* x, Source* s) {
  Lexer l = {.lex = x, .source = s, .current = s->contents.first};
  run(&l);
}

void initLexer() {
  for (size_t i = 0; i < MARK_COUNT; i++)
    markNames[i] = nullTerminated(lexemeName(MARK_FIRST + i));
  for (size_t i = 0; i < KEYWORD_COUNT; i++)
    keywordNames[i] = nullTerminated(lexemeName(KEYWORD_FIRST + i));
}
