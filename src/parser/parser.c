// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lexer/api.h"
#include "parser/api.h"
#include "parser/mod.h"
#include "source/api.h"
#include "utility/api.h"

#include <stdbool.h>
#include <stddef.h>

/* Context of the parsing process. */
typedef struct {
  /* Target parse to add the parsed statements into. */
  Parse*        target;
  /* Source to report to. */
  Source*       reported;
  /* Parsed lex. */
  Lex           parsed;
  /* Position of the currently parsed lexeme. */
  Lexeme const* current;
  /* Start of an unexpected group of lexemes. Null if it does not exist. */
  Lexeme const* firstUnexpected;
  /* Expression that is currently being built-up. */
  Expression    built;
} Context;

/* Whether there is a lexeme to parse. */
static bool checkLexemeExistance(Context* context) {
  // EOF should not be parsed; thus, subtract by 1;
  return context->current < context->parsed.after - 1;
}

/* Lexeme that is parsed. */
static Lexeme getCurrentLexeme(Context* context) { return *context->current; }

/* Whether there is a lexeme to lex and it is of the given variant. */
static bool compareCurrent(Context* context, LexemeTag compared) {
  return checkLexemeExistance(context) &&
         getCurrentLexeme(context).tag == compared;
}

/* Go to the next lexeme. */
static void advanceOnce(Context* context) { context->current++; }

/* Return the lexeme that was parsed and go to the next lexeme. */
static Lexeme takeOnce(Context* context) {
  // Store the result; then, advance.
  Lexeme taken = getCurrentLexeme(context);
  advanceOnce(context);
  return taken;
}

/* Whether the current lexeme is of the given type. Consumes the lexeme if true.
 */
static bool consumeOnce(Context* context, LexemeTag consumed) {
  // Store the result; then, advance.
  bool wasTaken = compareCurrent(context, consumed);
  if (wasTaken) advanceOnce(context);
  return wasTaken;
}

/* Section of the source file starting from the given section upto the current
 * lexeme. */
static String createSectionFromSection(Context* context, String start) {
  // Ends at the end of the previous lexeme, rather than the start of the
  // current one. This way any characters that were skipped by the lexer between
  // them is not included in the created section.
  return createString(start.first, context->current[-1].section.after);
}

/* Call `createSectionFromSection` with the section of the given lexeme. */
static String createSectionFromLexeme(Context* context, Lexeme start) {
  return createSectionFromSection(context, start.section);
}

/* Call `createSectionFromSection` with the section of the given node. */
static String createSectionFromNode(Context* context, ExpressionNode node) {
  return createSectionFromSection(context, node.section);
}

/* Built a new node with the given operator, arity and section. */
static void buildNode(
  Context* context, Operator builtOperator, size_t builtArity,
  String builtSection) {
  pushExpressionNode(
    &context->built, (ExpressionNode){
                       .operator= builtOperator.hash,
                       .arity   = builtArity,
                       .section = builtSection});
}

/* Last built node. */
static ExpressionNode getLastBuiltNode(Context* context) {
  return context->built.after[-1];
}

/* Built expression. Clears the built expression field in context. */
static Expression getBuiltExpression(Context* context) {
  Expression built = context->built;
  context->built   = createExpression(0);
  return built;
}

/* Result of parsing an syntactic object. */
typedef enum {
  /* Object was successfull parsed. */
  SUCCESS,
  /* No parsing happened because the object does not exist. If a parent object
   * needed the result of this parse it should report an error. */
  NOT_THERE,
  /* Object was there, but it was not successfully parsed. The parent object
   * that needed the result of this one, should not give an error as the
   * child object is responsible for reporting when it returns a failure. */
  FAILURE
} Result;

// Prototype for recursivly parsing expressions.
static Result parseExpression(Context* context, OperatorPrecedence parsedLevel);

/* Try to parse a nullary expression node. */
static Result
parseNullaryNode(Context* context, Operator parsed, bool cleanParse) {
  Lexeme start = getCurrentLexeme(context);
  // As the nullary operators do not have any operands that come before them,
  // if its not a clean parse it cannot be a nullary expression. If the operator
  // lexeme is not there, the nullary expression is not there.
  if (!cleanParse || !consumeOnce(context, parsed.asNullary.only))
    return NOT_THERE;

  // Build the node without any operands.
  buildNode(context, parsed, 0, createSectionFromLexeme(context, start));
  return SUCCESS;
}

/* Try to parse a prenary expression node. */
static Result
parsePrenaryNode(Context* context, Operator parsed, bool cleanParse) {
  Lexeme start = getCurrentLexeme(context);
  // As the prenary operators do not have any operands that come before them,
  // if its not a clean parse it cannot be a prenary expression. If the operator
  // lexeme is not there, the prenary expression is not there.
  if (!cleanParse || !consumeOnce(context, parsed.asPrenary.before))
    return NOT_THERE;

  // Parse the operand and build the node if successfull. The operand can be at
  // the same level since the prenary operators are ordered right to left.
  switch (parseExpression(context, parsed.precedence)) {
  case SUCCESS:
    buildNode(context, parsed, 1, createSectionFromLexeme(context, start));
    return SUCCESS;
  case NOT_THERE:
    highlightError(
      context->reported, createSectionFromLexeme(context, start),
      "Expected an operand after the operator `%s`!",
      nameLexeme(parsed.asPrenary.before));
  case FAILURE: return FAILURE;
  default: unexpected("Unknown parse result!");
  }
}

/* Try to parse a postary expression node. */
static Result
parsePostaryNode(Context* context, Operator parsed, bool cleanParse) {
  // If its a clean parse, there is no operand. And, if the operator lexeme
  // variant is not there, the postary expression is not there.
  if (cleanParse || !consumeOnce(context, parsed.asPostary.after))
    return NOT_THERE;
  buildNode(
    context, parsed, 1,
    createSectionFromNode(context, getLastBuiltNode(context)));
  return SUCCESS;
}

/* Try to parse a cirnary expression node. */
static Result
parseCirnaryNode(Context* context, Operator parsed, bool cleanParse) {
  Lexeme start = getCurrentLexeme(context);
  // As the cirnary operators do not have any operands that come before them,
  // if its not a clean parse it cannot be a cirnary expression. If the opening
  // lexeme is not there, the cirnary expression is not there.
  if (!cleanParse || !consumeOnce(context, parsed.asCirnary.opening))
    return NOT_THERE;

  // Parse the operand between the opening and closing lexemes. The precedence
  // of the surrounded operand can be as low as possible since it is clearly
  // marked with the opening and closing lexemes of the cirnary operator. The
  // expression inside cannot be an assignment per the rules of the language.
  switch (parseExpression(context, OPERATOR_ASSIGNMENT + 1)) {
  case SUCCESS: break;
  case NOT_THERE:
    highlightError(
      context->reported, createSectionFromLexeme(context, start),
      "Expected an operand after the opening `%s`!",
      nameLexeme(parsed.asCirnary.opening));
  case FAILURE: return FAILURE;
  default: unexpected("Unknown parse result!");
  }

  // Check the closing lexeme.
  if (!consumeOnce(context, parsed.asCirnary.closing)) {
    highlightError(
      context->reported, createSectionFromLexeme(context, start),
      "Expected a closing `%s` for the opening `%s`!",
      nameLexeme(parsed.asCirnary.closing),
      nameLexeme(parsed.asCirnary.opening));
    highlightInfo(
      context->reported, start.section, "Opening `%s` was here.",
      nameLexeme(parsed.asCirnary.opening));
    return FAILURE;
  }

  buildNode(context, parsed, 1, createSectionFromLexeme(context, start));
  return SUCCESS;
}

/* Try to parse a binary expression node. */
static Result
parseBinaryNode(Context* context, Operator parsed, bool cleanParse) {
  // If its a clean parse, there is no left operand. And, if the operator lexeme
  // variant is not there, the binary expression is not there.
  if (cleanParse || !consumeOnce(context, parsed.asBinary.between))
    return NOT_THERE;
  ExpressionNode leftOperand = getLastBuiltNode(context);

  // Parse the right operand that comes after the operator. The operand can only
  // be something that binds thighter than the parsed operator since the binary
  // operators are ordered left to right.
  switch (parseExpression(context, parsed.precedence + 1)) {
  case SUCCESS:
    buildNode(context, parsed, 2, createSectionFromNode(context, leftOperand));
    return SUCCESS;
  case NOT_THERE:
    highlightError(
      context->reported, createSectionFromNode(context, leftOperand),
      "Expected an operand after the operator `%s`!",
      nameLexeme(parsed.asBinary.between));
  case FAILURE: return FAILURE;
  default: unexpected("Unknown parse result!");
  }
}

/* Try to parse a variary expression node. */
static Result
parseVariaryNode(Context* context, Operator parsed, bool cleanParse) {
  Lexeme start = getCurrentLexeme(context);
  // As the variary operators do not have any operands that come before them,
  // if its not a clean parse it cannot be a variary expression. If the opening
  // lexeme is not there, the variary expression is not there.
  if (!cleanParse || !consumeOnce(context, parsed.asVariary.opening))
    return NOT_THERE;

  // Try to parse the first operand. Operands of the variary operators can be
  // any level like the cirnary operators, because they are all marked by the
  // opening, separating and closing lexemes clearly.
  switch (parseExpression(context, OPERATOR_ASSIGNMENT + 1)) {
  case SUCCESS: break;
  case NOT_THERE:
    // If the closing lexeme is found, the expression is complete.
    if (consumeOnce(context, parsed.asVariary.closing)) {
      buildNode(context, parsed, 0, createSectionFromLexeme(context, start));
      return SUCCESS;
    }

    // If there is not an operator or a closing lexeme, there is an error.
    highlightError(
      context->reported, createSectionFromLexeme(context, start),
      "Expected a closing `%s` for the opening `%s`!",
      nameLexeme(parsed.asVariary.closing),
      nameLexeme(parsed.asVariary.opening));
    highlightInfo(
      context->reported, start.section, "Opening `%s` was here.",
      nameLexeme(parsed.asVariary.opening));
  case FAILURE: return FAILURE;
  default: unexpected("Unknown parse result!");
  }

  // The first operand was parsed just now; thus, there is 1 operand at the
  // moment.
  size_t arity = 1;

  while (true) {
    // If the closing lexeme is found, the expression is over.
    if (consumeOnce(context, parsed.asVariary.closing)) break;

    // If the closing lexeme is not there, the separating one must be.
    if (!consumeOnce(context, parsed.asVariary.separating)) {
      highlightError(
        context->reported, createSectionFromLexeme(context, start),
        "Expected a closing `%s` for the opening `%s`!",
        nameLexeme(parsed.asVariary.closing),
        nameLexeme(parsed.asVariary.opening));
      highlightInfo(
        context->reported, start.section, "Opening `%s` was here.",
        nameLexeme(parsed.asVariary.opening));
      return FAILURE;
    }

    // After the separating lexeme, there should be another operand. As the
    // operand is parsed successfuly the arity should be increased.
    switch (parseExpression(context, OPERATOR_ASSIGNMENT + 1)) {
    case SUCCESS: arity++; break;
    case NOT_THERE:
      highlightError(
        context->reported, createSectionFromLexeme(context, start),
        "Expected an operand after the separating `%s`!",
        nameLexeme(parsed.asVariary.separating));
    case FAILURE: return FAILURE;
    default: unexpected("Unknown parse result!");
    }
  }

  buildNode(context, parsed, arity, createSectionFromLexeme(context, start));
  return SUCCESS;
}

/* Try to parse a multary expression node. */
static Result
parseMultaryNode(Context* context, Operator parsed, bool cleanParse) {
  Lexeme start = getCurrentLexeme(context);
  // If its a clean parse, there is no first operand. And, if the opening lexeme
  // variant is not there, the multary expression is not there.
  if (cleanParse || !consumeOnce(context, parsed.asMultary.opening))
    return NOT_THERE;
  ExpressionNode firstOperand = getLastBuiltNode(context);

  // Try to parse the second operand. Operands of the multary operators after
  // the first one can be any level like the cirnary operators, because they are
  // all marked by the opening, separating and closing lexemes clearly.
  switch (parseExpression(context, OPERATOR_ASSIGNMENT + 1)) {
  case SUCCESS: break;
  case NOT_THERE:
    // If the closing lexeme is found, the expression is complete.
    if (consumeOnce(context, parsed.asMultary.closing)) {
      buildNode(
        context, parsed, 1, createSectionFromNode(context, firstOperand));
      return SUCCESS;
    }

    // If there is not an operator or a closing lexeme, there is an error.
    highlightError(
      context->reported, createSectionFromNode(context, firstOperand),
      "Expected a closing `%s` for the opening `%s`!",
      nameLexeme(parsed.asMultary.closing),
      nameLexeme(parsed.asMultary.opening));
    highlightInfo(
      context->reported, start.section, "Opening `%s` was here.",
      nameLexeme(parsed.asMultary.opening));
  case FAILURE: return FAILURE;
  default: unexpected("Unknown parse result!");
  }

  // The first operand was there, and the second one was parsed just now; thus,
  // there are 2 operands at the moment.
  size_t arity = 2;

  while (true) {
    // If the closing lexeme is found, the expression is over.
    if (consumeOnce(context, parsed.asMultary.closing)) break;

    // If the closing lexeme is not there, the separating one must be.
    if (!consumeOnce(context, parsed.asMultary.separating)) {
      highlightError(
        context->reported, createSectionFromNode(context, firstOperand),
        "Expected a closing `%s` for the opening `%s`!",
        nameLexeme(parsed.asMultary.closing),
        nameLexeme(parsed.asMultary.opening));
      highlightInfo(
        context->reported, start.section, "Opening `%s` was here.",
        nameLexeme(parsed.asMultary.opening));
      return FAILURE;
    }

    // After the separating lexeme, there should be another operand. As the
    // operand is parsed successfuly the arity should be increased.
    switch (parseExpression(context, OPERATOR_ASSIGNMENT + 1)) {
    case SUCCESS: arity++; break;
    case NOT_THERE:
      highlightError(
        context->reported, createSectionFromNode(context, firstOperand),
        "Expected an operand after the separating `%s`!",
        nameLexeme(parsed.asMultary.separating));
    case FAILURE: return FAILURE;
    default: unexpected("Unknown parse result!");
    }
  }

  buildNode(
    context, parsed, arity, createSectionFromNode(context, firstOperand));
  return SUCCESS;
}

/* Try to parse an expression node. */
static Result parseNode(
  Context* context, OperatorPrecedence parsedLevel, size_t parsedInLevelIndex,
  bool cleanParse) {
  Operator parsed = getOperatorAt(parsedLevel, parsedInLevelIndex);
  switch (parsed.tag) {
  case OPERATOR_NULLARY: return parseNullaryNode(context, parsed, cleanParse);
  case OPERATOR_PRENARY: return parsePrenaryNode(context, parsed, cleanParse);
  case OPERATOR_POSTARY: return parsePostaryNode(context, parsed, cleanParse);
  case OPERATOR_CIRNARY: return parseCirnaryNode(context, parsed, cleanParse);
  case OPERATOR_BINARY: return parseBinaryNode(context, parsed, cleanParse);
  case OPERATOR_VARIARY: return parseVariaryNode(context, parsed, cleanParse);
  case OPERATOR_MULTARY: return parseMultaryNode(context, parsed, cleanParse);
  default: unexpected("Unknown operator variant!");
  }
}

/* Try to parse an expression. */
static Result parseExpression(Context* context, OperatorPrecedence parsed) {
  Result result = NOT_THERE;

  // Parse an expression on the given precedence level or higher.
  for (OperatorPrecedence level = parsed; level < OPERATOR_LEVELS; level++) {
    for (size_t inLevel = 0; inLevel < countInLevelOperators(level);
         inLevel++) {
      // Pass the clean parse flag depending on whether there is not a result.
      switch (parseNode(context, level, inLevel, result == NOT_THERE)) {
      case SUCCESS:
        // Iterate starting from the parsed precedence level on success. This
        // way, operators are correctly ordered from left to right. The ones
        // that are associative from right to left, which is the prenary
        // operators, handle it themselves by parsing an expression of the same
        // level as their operand. Others, which are associative left to right,
        // only parse operands with higher precedence levels than themselves.
        // Since the loop variable is incremented at the end, subtract one to
        // get the correct target level in the next iteration of the outer loop.
        level  = parsed - 1;
        result = SUCCESS;
        break;
      case NOT_THERE: continue;
      case FAILURE: return FAILURE;
      default: unexpected("Unknown parse result!");
      }

      // Break from the inner loop on the success case.
      break;
    }
  }

  return result;
}

/* Try to parse a binding definition. */
static Result parseBindingDefinition(Context* context) {
  Lexeme start = getCurrentLexeme(context);

  // If the lexeme is not keyword `let`, there is no binding definition.
  if (!consumeOnce(context, LEXEME_LET)) return NOT_THERE;

  // There must be an identifier for the definition.
  if (!compareCurrent(context, LEXEME_IDENTIFIER)) {
    highlightError(
      context->reported, createSectionFromLexeme(context, start),
      "Expected a name in the binding!");
    return FAILURE;
  }
  Lexeme identifier = takeOnce(context);

  // The type expression is optional, create an empty one. Empty means it was
  // not there. If there is a colon, the type expression must follow it.
  Expression type = createExpression(0);
  if (consumeOnce(context, LEXEME_COLON)) {
    // Assignment expression should not be parsed because it consumes the value
    // expression as well. Also it is not allowed by the language rules.
    switch (parseExpression(context, OPERATOR_ASSIGNMENT + 1)) {
    case SUCCESS: break;
    case NOT_THERE:
      highlightError(
        context->reported, createSectionFromLexeme(context, start),
        "Expected a type in the binding definition after `:`!");
    case FAILURE: return FAILURE;
    default: unexpected("Unknown parse result!");
    }

    type = getBuiltExpression(context);
  }

  // After the type, the equal sign leads the value.
  if (!consumeOnce(context, LEXEME_EQUAL)) {
    highlightError(
      context->reported, createSectionFromLexeme(context, start),
      "Expected a `=` and a value in the binding definition.");
    return FAILURE;
  }

  // The value is not optional in a binding compared to a value.
  switch (parseExpression(context, OPERATOR_ASSIGNMENT + 1)) {
  case SUCCESS: break;
  case NOT_THERE:
    highlightError(
      context->reported, createSectionFromLexeme(context, start),
      "Expected a value in the binding definition after `=`!");
  case FAILURE: return FAILURE;
  default: unexpected("Unknown parse result!");
  }
  Expression value = getBuiltExpression(context);

  // If there are not any nodes in the type expression, it is inferred.
  if (countExpressionNodes(type))
    pushBindingDefinition(context->target, identifier.section, type, value);
  else
    pushInferredBindingDefinition(context->target, identifier.section, value);
  return SUCCESS;
}

/* Try to parse a variable definition. */
static Result parseVariableDefinition(Context* context) {
  Lexeme start = getCurrentLexeme(context);

  // If the lexeme is not keyword `var`, there is no variable definition.
  if (!consumeOnce(context, LEXEME_VAR)) return NOT_THERE;

  // There must be an identifier for the definition.
  if (!compareCurrent(context, LEXEME_IDENTIFIER)) {
    highlightError(
      context->reported, createSectionFromLexeme(context, start),
      "Expected a name in the variable definition!");
    return FAILURE;
  }
  Lexeme identifier = takeOnce(context);

  // The type expression is optional, create an empty one. Empty means it was
  // not there. If there is a colon, the type expression must follow it.
  Expression type = createExpression(0);
  if (consumeOnce(context, LEXEME_COLON)) {
    // Assignment expression should not be parsed because it consumes the
    // initial value expression as well. Also it is not allowed by the language
    // rules.
    switch (parseExpression(context, OPERATOR_ASSIGNMENT + 1)) {
    case SUCCESS: break;
    case NOT_THERE:
      highlightError(
        context->reported, createSectionFromLexeme(context, start),
        "Expected a type in the variable definition after `:`!");
    case FAILURE: return FAILURE;
    default: unexpected("Unknown parse result!");
    }

    type = getBuiltExpression(context);
  }

  // The initial value expression is optional, create an empty one. Empty means
  // it was not there. If there is an equal sign, the initial value expression
  // must follow it.
  Expression initialValue = createExpression(0);
  if (consumeOnce(context, LEXEME_EQUAL)) {
    // Assignment expression is not allowed by the language rules.
    switch (parseExpression(context, OPERATOR_ASSIGNMENT + 1)) {
    case SUCCESS: break;
    case NOT_THERE:
      highlightError(
        context->reported, createSectionFromLexeme(context, start),
        "Expected an initial value in the variable definition after `=`!");
    case FAILURE: return FAILURE;
    default: unexpected("Unknown parse result!");
    }

    initialValue = getBuiltExpression(context);
  }

  // If there are not any nodes in the initial value expression, it is
  // defaulted.
  if (countExpressionNodes(initialValue)) {
    // If there are not any nodes in the type expression, it is inferred.
    if (countExpressionNodes(type))
      pushVariableDefinition(
        context->target, identifier.section, type, initialValue);
    else
      pushInferredVariableDefinition(
        context->target, identifier.section, initialValue);
  } else {
    // If the initial value is defaulted there must be a type expression.
    // Otherwise it is impossible to define a variable.
    if (countExpressionNodes(type)) {
      pushDefaultedVariableDefinition(
        context->target, identifier.section, type);
    } else {
      highlightError(
        context->reported, createSectionFromLexeme(context, start),
        "Expected a type or an initial value in the variable definition!");
      return FAILURE;
    }
  }
  return SUCCESS;
}

/* Try to parse a discarded expression. */
static Result parseDiscardedExpression(Context* context) {
  // Parse a free expression, including the assignment binary expressions. This
  // is the only place an assignment expression is allowed.
  switch (parseExpression(context, OPERATOR_ASSIGNMENT)) {
  case SUCCESS:
    pushDiscardedExpression(context->target, getBuiltExpression(context));
    return SUCCESS;
  case NOT_THERE: return NOT_THERE;
  case FAILURE: return FAILURE;
  default: unexpected("Unknown parse result!");
  }
}

/* Try to parse a statement. */
static Result parseStatement(Context* context) {
  Result result = parseBindingDefinition(context);
  if (result != NOT_THERE) return result;

  result = parseVariableDefinition(context);
  if (result != NOT_THERE) return result;

  // If everything fails, try to parse a discarded expression.
  return parseDiscardedExpression(context);
}

/* Report the unexpected group of lexemes. */
static void reportUnexpected(Context* context, Lexeme const* afterUnexpected) {
  if (!context->firstUnexpected) return;

  // Similar to `createSectionFromSection`, create a section from the first
  // unexpected lexeme to the beginin of the lexeme after the unexpected group.
  String unexpected = {
    .first = context->firstUnexpected->section.first,
    .after = afterUnexpected[-1].section.after};

  highlightError(
    context->reported, unexpected, "Expected a statement instead of %s!",
    countCharacters(unexpected) > 1 ? "these characters" : "this character");

  // Clear the unexpected start to indicate it is handled.
  context->firstUnexpected = NULL;
}

/* Run the parser with the context. */
static void parse(Context* context) {
  while (checkLexemeExistance(context)) {
    Lexeme const* start  = context->current;
    Result        result = parseStatement(context);

    // If nothing was parsed, the lexeme is unexpected.
    if (result == NOT_THERE) {
      if (!context->firstUnexpected) context->firstUnexpected = start;
      advanceOnce(context);
      continue;
    }

    // If there is a result, show the previously skipped unexpected group of
    // lexemes.
    reportUnexpected(context, start);

    if (result == SUCCESS) {
      // Statements end with a semicolon.
      if (!consumeOnce(context, LEXEME_SEMICOLON)) {
        highlightError(
          context->reported, createSectionFromLexeme(context, *start),
          "Expected a `;` after the statement!");
        popStatement(context->target);
      }
      continue;
    }

    // Result was a failure, skip until a synchronization lexeme.
    if (!checkLexemeExistance(context)) break;

    // Skip the lexemes until the synchronization lexeme.
    Lexeme skipStart = getCurrentLexeme(context);
    while (checkLexemeExistance(context) &&
           !compareCurrent(context, LEXEME_SEMICOLON))
      advanceOnce(context);

    // Skip the synchronization lexeme as well.
    advanceOnce(context);

    highlightInfo(
      context->reported, createSectionFromLexeme(context, skipStart),
      "Skipped because of the previous error.");
  }

  // Show any unexpected characters at the end, which did not find an
  // oppurtinity to be reported because there might not have been a successful
  // parse after them.
  reportUnexpected(context, context->current);

  // Cannot use `compareCurrent` because that checks for existance, which does
  // not consider the last lexeme.
  expect(
    getCurrentLexeme(context).tag == LEXEME_EOF,
    "Lex does not end with an EOF!");
}

void parseLex(Parse* target, Source* reported, Lex parsed) {
  // Create a context and pass its pointer, because all functions take a context
  // pointer this removes the need for taking the address of the context.
  parse(&(Context){
    .target          = target,
    .reported        = reported,
    .parsed          = parsed,
    .current         = parsed.first,
    .firstUnexpected = NULL,
    .built           = createExpression(0)});
}
