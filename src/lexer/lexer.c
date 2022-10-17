// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lexer/api.h"
#include "lexer/mod.h"
#include "source/api.h"
#include "utility/api.h"

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
  /* Start of an unknown portion. Null if there is no unknown portion. */
  char const* unknownStart;
} Lexer;

String markNames[MARK_COUNT];
String keywordNames[KEYWORD_COUNT];

/* Section of the source file starting from the given position upto the current
 * character. */
#define createSection(startPosition) \
  createString(startPosition, context->current)

/* Add a lexeme with the given value and type. If there was an error, reports
 * that first and clears the error start. */
#define pushSectionAndTag(pushedSection, pushedTag)                          \
  if (context->unknownStart) {                                               \
    String unknown = createSection(context->unknownStart);                   \
    highlightError(                                                          \
      context->source, unknown, "Could not recognize %s!",                   \
      countCharacters(unknown) > 1 ? "these characters" : "this character"); \
    pushLexeme(                                                              \
      context->lex, (Lexeme){.section = unknown, .tag = LEXEME_ERROR});      \
    context->unknownStart = NULL;                                            \
  }                                                                          \
  pushLexeme(                                                                \
    context->lex, (Lexeme){.section = (pushedSection), .tag = (pushedTag)})

/* Whether there is a character to lex. */
#define checkCharacterExistance() \
  (context->current < context->source->contents.after)

/* Character that is lexed. */
#define getCurrentCharacter() (*context->current)

/* Whether there is a character to lex and it equals to the given one. */
#define compareCurrent(comparedCharacter) \
  (checkCharacterExistance() && getCurrentCharacter() == (comparedCharacter))

/* Whether there is a character to lex and it passes the given predicate. */
#define checkCurrent(checkedPredicate) \
  (checkCharacterExistance() && checkedPredicate(getCurrentCharacter()))

/* Go to the next character. */
#define advanceOnce() (context->current++)

/* Go to the previous character. */
#define retreatOnce() (context->current--)

/* Go back to the given position. */
#define retreatToPosition(oldPosition) \
  do { context->current = oldPosition; } while (false)

/* Whether the current character exists and it equals to the given one. Consumes
 * the character if true. */
static bool takeOnce(Lexer* context, char taken) {
  // Store the result; then, go to the next one.
  bool wasTaken = compareCurrent(taken);
  if (wasTaken) advanceOnce();
  return wasTaken;
}

/* Whether the next characters are the same as the given string. Consumes the
 * characters if true. */
static bool takeFixed(Lexer* context, String taken) {
  char const* start = context->current;
  size_t      count = countCharacters(taken);

  // Return `false` if any character is mismatched.
  for (size_t character = 0; character < count; character++) {
    if (!compareCurrent(taken.first[character])) {
      retreatToPosition(start);
      return false;
    }
    advanceOnce();
  }
  return true;
}

/* Predicate for checking a character. */
typedef bool (*Check)(char);

/* Consume the characters if the first one fits the given leading check and
 * the remaining fit the given traling check. */
static bool
takeVarying(Lexer* context, Check takenLeading, Check takenTrailing) {
  // Check the first character.
  if (!checkCurrent(takenLeading)) return false;
  advanceOnce();

  // Then, consume all the characters until there is a missmatch.
  while (checkCurrent(takenTrailing)) advanceOnce();
  return true;
}

/* Whether the given character is in the English alphabet. */
static bool compareToAlpha(char compared) {
  return (compared >= 'a' && compared <= 'z') ||
         (compared >= 'A' && compared <= 'Z');
}

/* Whether the given character is a decimal digit. */
static bool compareToDecimalDigit(char compared) {
  return compared >= '0' && compared <= '9';
}

/* Whether the given character is whitespace. */
static bool compareToWhitespace(char compared) {
  return compared == ' ' || compared == '\t' || compared == '\n';
}

/* Try to skip a whitespace. */
static bool lexWhitespace(Lexer* context) {
  return takeVarying(context, &compareToWhitespace, &compareToWhitespace);
}

/* Whether the given character is the start of a comment. */
static bool compareToCommentLead(char compared) { return compared == '/'; }

/* Whether the given character is the trailing characters of a comment. */
static bool compareToCommentTrail(char compared) { return compared != '\n'; }

/* Try to skip a comment. */
static bool lexComment(Lexer* context) {
  // Handle the first slash manually, leave the second one to the helper.
  if (!takeOnce(context, '/')) return false;
  if (!takeVarying(context, &compareToCommentLead, &compareToCommentTrail)) {
    retreatOnce(); // Roll back the first '/'.
    return false;
  }
  return true;
}

/* Try to lex a mark. */
static bool lexMark(Lexer* context) {
  char const* start = context->current;

  // Check whether there is a mark.
  for (size_t mark = 0; mark < MARK_COUNT; mark++) {
    if (takeFixed(context, markNames[mark])) {
      pushSectionAndTag(createSection(start), MARK_FIRST + mark);
      return true;
    }
  }

  // EOF mark.
  if (takeOnce(context, 0)) {
    pushSectionAndTag(createSection(start), LEXEME_EOF);
    return true;
  }

  return false;
}

/* Whether the given character can be start of a word. */
static bool compareToWordLead(char compared) {
  return compareToAlpha(compared) || compared == '_';
}

/* Whether the given character can be rest of a word. */
static bool compareToWordTrail(char compared) {
  return compareToAlpha(compared) || compareToDecimalDigit(compared) ||
         compared == '_';
}

/* Try to lex a word. */
static bool lexWord(Lexer* context) {
  char const* start = context->current;

  // Identifier.
  if (!takeVarying(context, &compareToWordLead, &compareToWordTrail))
    return false;
  String word = createSection(start);

  // Check whether it is a reserved identifier.
  for (size_t keyword = 0; keyword < KEYWORD_COUNT; keyword++) {
    if (compareStringEquality(word, keywordNames[keyword])) {
      pushSectionAndTag(word, KEYWORD_FIRST + keyword);
      return true;
    }
  }

  pushSectionAndTag(word, LEXEME_IDENTIFIER);
  return true;
}

/* Whether the given character can be start of a decimal. */
static bool compareToDecimalLead(char compared) {
  return compareToDecimalDigit(compared);
}

/* Whether the given character can be rest of a decimal. */
static bool compareToDecimalTrail(char compared) {
  return compareToDecimalDigit(compared) || compared == '_';
}

/* Try to lex a decimal literal. */
static bool lexDecimal(Lexer* context) {
  char const* start = context->current;

  // Whole part.
  // Optional sign.
  takeOnce(context, '+') || takeOnce(context, '-');
  if (!takeVarying(context, &compareToDecimalLead, &compareToDecimalTrail)) {
    retreatToPosition(start);
    return false;
  }

  // Fraction.
  char const* fractionStart = context->current;
  if (
    takeOnce(context, '.') &&
    !takeVarying(context, &compareToDecimalLead, &compareToDecimalTrail))
    retreatToPosition(fractionStart);

  // Exponent.
  char const* exponentStart = context->current;
  if (takeOnce(context, 'e') || takeOnce(context, 'E')) {
    // Optional sign.
    takeOnce(context, '+') || takeOnce(context, '-');
    if (!takeVarying(context, &compareToDecimalLead, &compareToDecimalTrail))
      retreatToPosition(exponentStart);
  }

  pushSectionAndTag(createSection(start), LEXEME_DECIMAL);
  return true;
}

/* Try to skip or lex a separator. */
static bool lexSeparator(Lexer* context) {
  return lexWhitespace(context) || lexComment(context) || lexMark(context);
}

/* Run the given lexer context. */
static void lex(Lexer* context) {
  while (checkCharacterExistance()) {
    char const* start = context->current;

    if (lexWord(context) || lexDecimal(context)) {
      if (lexSeparator(context)) continue;
      // Roll back the word or decimal that was lexed because it is not
      // separated from what comes after it.
      retreatToPosition(start);
      popLexeme(context->lex);
    }

    if (lexSeparator(context)) continue;

    // Unknown character! Mark all characters until a separator is found.
    context->unknownStart = start;
    do { advanceOnce(); } while (!lexSeparator(context));
  }
}

void lexSource(Lex* target, Source* lexed) {
  // Create a lexer and pass its pointer, because all the macros that are used
  // require a lexer in the local scope. This makes the code easier to fallow
  // by reducing function parameters without using a global context variable.
  lex(&(Lexer){
    .lex          = target,
    .source       = lexed,
    .current      = lexed->contents.first,
    .unknownStart = NULL});
}

void initLexer() {
  // Cache lexeme tag names of lexemes that equal to a fixed pattern of
  // characters like marks and keywords.
  for (size_t i = 0; i < MARK_COUNT; i++)
    markNames[i] = viewTerminated(nameLexeme(MARK_FIRST + i));
  for (size_t i = 0; i < KEYWORD_COUNT; i++)
    keywordNames[i] = viewTerminated(nameLexeme(KEYWORD_FIRST + i));
}
