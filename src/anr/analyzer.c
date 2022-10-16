// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "anr/api.h"
#include "anr/mod.h"
#include "dbg/api.h"
#include "otc/api.h"
#include "psr/api.h"
#include "utl/api.h"

#include <stdbool.h>

/* Context of the analysis process. */
typedef struct {
  /* Table to add the symbols into. */
  Table*  target;
  /* Source to report to. */
  Source* reported;
  /* Analyzed parse. */
  Parse   analyzed;
  /* Map from the symbol name to the symbol's index on the table. */
  Map     namesToIndices;
  /* Symbol names that were not analyzed because of failure but was seen. */
  Set     failedNames;
} Context;

/* Push to table and insert to map the given symbol. */
static void addSymbol(Context* context, Symbol added) {
  // The index of the pushed symbol would be the current amount of symbols in
  // the table, which can be found from the difference of the pointers.
  size_t index = context->target->after - context->target->first;
  insertEntry(&context->namesToIndices, added.name, index);
  pushSymbol(context->target, added);
}

/* Prepare the given built-in type. */
static void prepareType(Context* context, Type prepared) {
  Value  value  = {.asType = prepared};
  Object object = {.type = META_TYPE_INSTANCE, .value = value, .known = true};
  addSymbol(
    context, (Symbol){
               .tag         = SYMBOL_TYPE,
               .name        = viewTerminated(nameType(prepared)),
               .object      = object,
               .userDefined = false});
}

/* Prepare the given built-in binding. */
static void prepareBinding(
  Context* context, char const* preparedName, Type preparedType,
  Value preparedValue) {
  Object object = {.type = preparedType, .value = preparedValue, .known = true};
  addSymbol(
    context, (Symbol){
               .tag         = SYMBOL_BINDING,
               .name        = viewTerminated(preparedName),
               .object      = object,
               .userDefined = false});
}

/* Setup the built-int symbols. */
static void prepare(Context* context) {
  prepareType(context, META_TYPE_INSTANCE);
  prepareType(context, VOID_TYPE_INSTANCE);
  prepareType(context, BOOL_TYPE_INSTANCE);
  prepareType(context, BYTE_TYPE_INSTANCE);
  prepareType(context, INT_TYPE_INSTANCE);
  prepareType(context, UXS_TYPE_INSTANCE);
  prepareType(context, FLOAT_TYPE_INSTANCE);
  prepareType(context, DOUBLE_TYPE_INSTANCE);

  prepareBinding(context, "true", BOOL_TYPE_INSTANCE, (Value){.asBool = true});
  prepareBinding(
    context, "false", BOOL_TYPE_INSTANCE, (Value){.asBool = false});
}

/* Whether the definition of a new symbol with the given name is valid. */
static bool checkDefinedName(Context* context, String checked) {
  // Check the successfully defined symbols.
  MapEntry const* entry = accessEntry(context->namesToIndices, checked);
  if (entry) {
    Symbol previous = context->target->first[entry->value];
    if (previous.userDefined) {
      highlightError(
        context->reported, checked,
        "Name clashes with a previously defined symbol!");
      highlightInfo(
        context->reported, previous.name, "Previous definition was here.");
    } else {
      highlightError(
        context->reported, checked, "Name clashes with a built-in symbol!");
    }
    return false;
  }

  // Check the failed symbols.
  String const* failed = accessMember(context->failedNames, checked);
  if (failed) {
    highlightError(
      context->reported, checked,
      "Name clashes with a previously defined symbol!");
    highlightInfo(context->reported, *failed, "Previous definition was here.");
    return false;
  }

  return true;
}

/* Whether the expected type can be the result of an arithmetic operation.
 * Errors about the source object are reported at the given section. */
static bool checkExpectedArithmetic(
  Context* context, String checkedSection, Type expected) {
  // Check whether the type is arithmetic.
  if (!checkArithmeticity(expected)) {
    highlightError(
      context->reported, checkedSection, "Expected a `%s`, but found a number!",
      nameType(expected));
    return false;
  }

  // Check whether the type can fit at least an integer, which is the bare
  // minimum for any arithmetic operation because of integer promotion rules of
  // C language.
  if (!checkArithmeticConvertability(INT_TYPE_INSTANCE, expected)) {
    highlightError(
      context->reported, checkedSection,
      "Due to integer promotion, the result of operation is `%s`, which "
      "does not fit inside a `%s`!",
      nameType(INT_TYPE_INSTANCE), nameType(expected));
    return false;
  }

  return true;
}

/* Whether the expected type can be converted from the given source object from
 * the given section. Errors about the source object are reported at the given
 * section. */
static bool checkExpectedType(
  Context* context, String checkedSection, Type checkedType, Type expected) {
  // Check whether the type can be converted to the result
  if (!checkConvertability(checkedType, expected)) {
    highlightError(
      context->reported, checkedSection, "Expected a `%s`, found a `%s`!",
      nameType(expected), nameType(checkedType));
    return false;
  }

  return true;
}

/* Check whether the given destination evaluation is assignable from the given
 * source object from the given section. Errors about the source object are
 * reported at the given section. */
static bool checkAssignment(
  Context* context, String sourceSection, Object sourceObject,
  EvaluationNode destination) {
  // Keep a result flag, such that there is no early exiting, and all errors are
  // reported at the same time.
  bool result = true;

  // Check type convertability.
  if (!checkConvertability(sourceObject.type, destination.object.type)) {
    highlightError(
      context->reported, sourceSection, "Cannot assign a `%s` to a `%s`!",
      nameType(sourceObject.type), nameType(destination.object.type));
    result = false;
  }

  // Check the destination expression.
  switch (destination.evaluated.operator) {
  // Check whether the assigned symbol is a variable.
  case SYMBOL_ACCESS: {
    MapEntry const* entry =
      accessEntry(context->namesToIndices, destination.evaluated.section);
    expect(entry, "Access operation was not checked correctly!");
    switch (context->target->first[entry->value].tag) {
    case SYMBOL_VARIABLE: break;
    default:
      highlightError(
        context->reported, sourceSection, "Can only assign to a variable!");
      result = false;
    }
    break;
  }
  default:
    highlightError(
      context->reported, destination.evaluated.section,
      "Cannot assign to this expression!");
    result = false;
  }

  return result;
}

// Prototype for recursive checking of expressions.
static bool checkNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer,
  Type expected);
// Prototype for recursive evaluation of expressions.
static bool evaluateNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer);

/* Call `evaluateNode` and check whether the result is an arithmetic type. */
static bool evaluateArithmeticNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer) {
  if (!evaluateNode(context, built, pointer)) return false;

  // Check whether the type is arithmetic.
  EvaluationNode result = built->after[-1];
  if (!checkArithmeticity(result.object.type)) {
    highlightError(
      context->reported, result.evaluated.section,
      "Expected a number, but found a `%s`!", nameType(result.object.type));
    return false;
  }

  return true;
}

/* Call `evaluateNode` and check whether the result is an integer type. */
static bool evaluateIntegerNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer) {
  if (!evaluateNode(context, built, pointer)) return false;

  // Check whether the type is integer.
  EvaluationNode result = built->after[-1];
  if (!checkIntegerness(result.object.type)) {
    highlightError(
      context->reported, result.evaluated.section,
      "Expected an integer, but found a `%s`!", nameType(result.object.type));
    return false;
  }

  return true;
}

/* Convert the object of the last built one to the given type. */
static void convertLastBuiltNode(Evaluation* built, Type destination) {
  // If known, convert the value.
  if (built->after[-1].object.known) {
    built->after[-1].object.value = convertValue(
      built->after[-1].object.type, destination, built->after[-1].object.value);
  }
  built->after[-1].object.type = destination;
}

/* Call `checkNode` with it if it is an integer, otherwise calls
 * `evaluateIntegerNode`. */
static bool checkIntegerNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer,
  Type expected) {
  // Even if the expected type is not an integer, the checked node should be.
  if (!checkIntegerness(expected))
    return evaluateIntegerNode(context, built, pointer);

  return checkNode(context, built, pointer, expected);
}

/* Version of `checkNode` with a nullary operator. */
static bool checkNullaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer,
  Type expected) {
  ExpressionNode checked = **pointer;
  (*pointer)--;
  switch (checked.operator) {
  // Nullary operator that results in the accessed type.
  case SYMBOL_ACCESS: {
    // Check wheter the accessed symbol is defined.
    MapEntry const* entry =
      accessEntry(context->namesToIndices, checked.section);
    if (!entry) {
      // Check whether it was a failed symbol; then, fail silently.
      if (!accessMember(context->failedNames, checked.section))
        highlightError(context->reported, checked.section, "Unknown symbol!");
      return false;
    }
    Symbol accessed = context->target->first[entry->value];

    // Check the type.
    if (!checkConvertability(accessed.object.type, expected)) {
      highlightError(
        context->reported, checked.section,
        "Expected a `%s`, but `%.s` is a `%s`!", nameType(expected),
        (int)countCharacters(accessed.name), accessed.name.first,
        nameType(accessed.object.type));
      return false;
    }

    // If known, convert the value.
    Object object = {.type = expected};
    if (accessed.object.known) {
      object.value =
        convertValue(accessed.object.type, expected, accessed.object.value);
      object.known = true;
    }
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = checked, .object = object});
    return true;
  }

  // Nullary operator that results in an arithmetic type.
  case DECIMAL_LITERAL: {
    // Check and consume the sign.
    bool   negative = *checked.section.first == '-';
    String parsed   = createString(
        (negative || *checked.section.first == '+') + checked.section.first,
        checked.section.after);

    // Store the result for reporting multiple errors.
    bool result = true;

    // Parse the string to a decimal number.
    Number decimal = parseDecimal(parsed);
    if (decimal.flag == NUMBER_TOO_PRECISE) {
      highlightError(
        context->reported, checked.section, "The number is too long!");
      result = false;
    }

    Object object = {.known = true};

    // Check whether the expected type is an arithmetic type.
    if (!checkArithmeticity(expected)) {
      highlightError(
        context->reported, checked.section,
        "Expected a `%s`, but found a number!", nameType(expected));
      result = false;
    }

    if (!result) {
      disposeNumber(&decimal);
      return false;
    }

    // Try to convert to the expeced arithmetic type.
    switch (
      convertNumberToArithmetic(expected, &object.value, decimal, negative)) {
    case NUMBER_CONVERSION_SUCCESS:
      pushEvaluationNode(
        built, (EvaluationNode){.evaluated = checked, .object = object});
      disposeNumber(&decimal);
      return true;
    case NUMBER_CONVERSION_NOT_INTEGER:
      highlightError(
        context->reported, checked.section,
        "Expected a `%s`, but the number is not an integer!",
        nameType(expected));
      disposeNumber(&decimal);
      return false;
    case NUMBER_CONVERSION_NOT_UNSIGNED:
      highlightError(
        context->reported, checked.section,
        "Expected a `%s`, but the number is negative!", nameType(expected));
      disposeNumber(&decimal);
      return false;
    case NUMBER_CONVERSION_OUT_OUF_BOUNDS:
      highlightError(
        context->reported, checked.section, "Number is out of bounds of `%s`!",
        nameType(expected));
      disposeNumber(&decimal);
      return false;
    default: unexpected("Unknown number conversion result!");
    }
  }
  default: unexpected("Unknown nullary operator!");
  }
}

/* Version of `checkNode` with a prenary operator. */
static bool checkPrenaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer,
  Type expected) {
  ExpressionNode checked = **pointer;
  (*pointer)--;
  switch (checked.operator) {
  // Prenary operators taking any arithmetic and returning it.
  case POSATE:
  case NEGATE: {
    if (
      !checkExpectedArithmetic(context, checked.section, expected) ||
      !checkNode(context, built, pointer, expected))
      return false;
    Object object = {.type = expected};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = checked, .object = object});
    return true;
  }

  // Prenary operator taking any integer and returning it.
  case COMPLEMENT: {
    if (
      !checkExpectedArithmetic(context, checked.section, expected) ||
      !checkIntegerNode(context, built, pointer, expected))
      return false;
    Object object = {.type = expected};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = checked, .object = object});
    return true;
  }

  // Binary operators delegating to the unchecked version that returns bool.
  case LOGICAL_NOT: {
    // Roll back the consumed expression node.
    (*pointer)++;

    // Check the expected type.
    // Store the result for reporting multiple errors.
    bool result =
      checkExpectedType(context, checked.section, BOOL_TYPE_INSTANCE, expected);

    // Delegate to the unchecked version.
    if (!evaluateNode(context, built, pointer)) return false;

    // If the expected type failed, remove the last built node.
    if (!result) {
      built->after--;
      return false;
    }

    // Convert the result of the unchecked version.
    convertLastBuiltNode(built, expected);
    return true;
  }

  // Binary operators delegating to the unchecked version that returns void.
  case PREFIX_INCREMENT:
  case PREFIX_DECREMENT: {
    // Roll back the consumed expression node.
    (*pointer)++;

    // Check the expected type.
    // Store the result for reporting multiple errors.
    bool result =
      checkExpectedType(context, checked.section, VOID_TYPE_INSTANCE, expected);

    // Delegate to the unchecked version.
    if (!evaluateNode(context, built, pointer)) return false;

    // If the expected type failed, remove the last built node.
    if (!result) {
      built->after--;
      return false;
    }

    // Convert the result of the unchecked version.
    convertLastBuiltNode(built, expected);
    return true;
  }
  default: unexpected("Unknown prenary operator!");
  }
}

/* Version of `checkNode` with a postary operator. */
static bool checkPostaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer,
  Type expected) {
  ExpressionNode checked = **pointer;
  (*pointer)--;
  switch (checked.operator) {
  // Binary operators delegating to the unchecked version.
  case POSTFIX_INCREMENT:
  case POSTFIX_DECREMENT: {
    // Roll back the consumed expression node.
    (*pointer)++;

    // Check the expected type.
    // Store the result for reporting multiple errors.
    bool result =
      checkExpectedType(context, checked.section, VOID_TYPE_INSTANCE, expected);

    // Delegate to the unchecked version.
    if (!evaluateNode(context, built, pointer)) return false;

    // If the expected type failed, remove the last built node.
    if (!result) {
      built->after--;
      return false;
    }

    // Convert the result of the unchecked version.
    convertLastBuiltNode(built, expected);
    return true;
  }
  default: unexpected("Unknown postary operator!");
  }
}

/* Version of `checkNode` with a cirnary operator. */
static bool checkCirnaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer,
  Type expected) {
  ExpressionNode checked = **pointer;
  (*pointer)--;
  switch (checked.operator) {
  // Cirnary operator taking any type and returning it.
  case GROUP: {
    if (!checkNode(context, built, pointer, expected)) return false;
    Object object = {.type = expected};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = checked, .object = object});
    return true;
  }
  default: unexpected("Unknown cirnary operator!");
  }
}

/* Version of `checkNode` with a binary operator. */
static bool checkBinaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer,
  Type expected) {
  ExpressionNode checked = **pointer;
  (*pointer)--;
  switch (checked.operator) {
  // Binary operators taking any arithmetic and returning it.
  case MULTIPLICATION:
  case DIVISION:
  case REMINDER:
  case ADDITION:
  case SUBTRACTION: {
    // Bitwise OR is used to report multiple errors.
    if (
      !checkExpectedArithmetic(context, checked.section, expected) ||
      ((int)!checkNode(context, built, pointer, expected) | // Right operand.
       !checkNode(context, built, pointer, expected)))      // Left operand.
      return false;
    Object object = {.type = expected};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = checked, .object = object});
    return true;
  }

  // Binary operators taking any integer and returning it.
  case LEFT_SHIFT:
  case RIGHT_SHIFT:
  case BITWISE_AND:
  case BITWISE_XOR:
  case BITWISE_OR: {
    // Bitwise OR is used to report multiple errors.
    if (
      !checkExpectedArithmetic(context, checked.section, expected) ||
      ((int)!checkIntegerNode(
         context, built, pointer, expected) |                 // Right operand.
       !checkIntegerNode(context, built, pointer, expected))) // Left operand.
      return false;
    Object object = {.type = expected};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = checked, .object = object});
    return true;
  }

  // Binary operators taking any arithmetic and returning bool.
  case SMALLER_THAN:
  case SMALLER_THAN_OR_EQUAL_TO:
  case GREATER_THAN:
  case GREATER_THAN_OR_EQUAL_TO:
  case EQUAL_TO:
  case NOT_EQUAL_TO:
  case LOGICAL_AND:
  case LOGICAL_OR: {
    // Bitwise OR is used to report multiple errors.
    if (
      (int)!checkExpectedType(
        context, checked.section, BOOL_TYPE_INSTANCE, expected) |
      !evaluateArithmeticNode(context, built, pointer) | // Right operand.
      !evaluateArithmeticNode(context, built, pointer))  // Left operand.
      return false;
    Object object = {.type = expected};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = checked, .object = object});
    return true;
  }

  // Binary operators delegating to the unchecked version.
  case ASSIGNMENT:
  case MULTIPLICATION_ASSIGNMENT:
  case DIVISION_ASSIGNMENT:
  case REMINDER_ASSIGNMENT:
  case ADDITION_ASSIGNMENT:
  case SUBTRACTION_ASSIGNMENT:
  case LEFT_SHIFT_ASSIGNMENT:
  case RIGHT_SHIFT_ASSIGNMENT:
  case BITWISE_AND_ASSIGNMENT:
  case BITWISE_XOR_ASSIGNMENT:
  case BITWISE_OR_ASSIGNMENT: {
    // Roll back the consumed expression node.
    (*pointer)++;

    // Check the expected type.
    // Store the result for reporting multiple errors.
    bool result =
      checkExpectedType(context, checked.section, VOID_TYPE_INSTANCE, expected);

    // Delegate to the unchecked version.
    if (!evaluateNode(context, built, pointer)) return false;

    // If the expected type failed, remove the last built node.
    if (!result) {
      built->after--;
      return false;
    }

    // Convert the result of the unchecked version.
    convertLastBuiltNode(built, expected);
    return true;
  }
  default: unexpected("Unknown binary operator!");
  }
}

/* Version of `checkNode` with a variary operator. */
static bool checkVariaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer,
  Type expected) {
  ExpressionNode checked = **pointer;
  (*pointer)--;
  switch (checked.operator) {
  case FUNCTION_CALL:
    highlightWarning(
      context->reported, checked.section,
      "Function call is not yet implemented.");
    return false;
  default: unexpected("Unknown variary operator!");
  }
}

/* Check the given expression node. Returns the node that comes after the
 * given one and its childeren. Returns null if the evaluation failed or the
 * node did not have the expected type. */
static bool checkNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer,
  Type expected) {
  ExpressionNode checked = **pointer;
  switch (getOperator(checked.operator).tag) {
  case OPERATOR_NULLARY:
    return checkNullaryNode(context, built, pointer, expected);
  case OPERATOR_PRENARY:
    return checkPrenaryNode(context, built, pointer, expected);
  case OPERATOR_POSTARY:
    return checkPostaryNode(context, built, pointer, expected);
  case OPERATOR_CIRNARY:
    return checkCirnaryNode(context, built, pointer, expected);
  case OPERATOR_BINARY:
    return checkBinaryNode(context, built, pointer, expected);
  case OPERATOR_VARIARY:
    return checkVariaryNode(context, built, pointer, expected);
  default: unexpected("Unknown operator variant!");
  }
}

/* Evaluate the given expression and build it into the given evaluation; and
 * check its type. Returns whether the expression was successful and the
 * expression had the expected type. */
static bool checkExpression(
  Context* context, Evaluation* built, Expression checked, Type expected) {
  // Check starting from the root node.
  ExpressionNode const* start = checked.after - 1;
  return checkNode(context, built, &start, expected);
}

/* Version of `evaluateNode` with a nullary operator. */
static bool evaluateNullaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer) {
  ExpressionNode evaluated = **pointer;
  (*pointer)--;
  switch (evaluated.operator) {
  // Nullary operator that results in the accessed type.
  case SYMBOL_ACCESS: {
    // Check wheter the accessed symbol is defined.
    MapEntry const* entry =
      accessEntry(context->namesToIndices, evaluated.section);
    if (!entry) {
      // Check whether it was a failed symbol; then, fail silently.
      if (!accessMember(context->failedNames, evaluated.section))
        highlightError(context->reported, evaluated.section, "Unknown symbol!");
      return false;
    }

    // Create with the the accessed object.
    Symbol accessed = context->target->first[entry->value];
    pushEvaluationNode(
      built,
      (EvaluationNode){.evaluated = evaluated, .object = accessed.object});
    return true;
  }

  // Nullary operator that results in an arithmetic type.
  case DECIMAL_LITERAL: {
    // Check and consume the sign.
    bool   negative = *evaluated.section.first == '-';
    String parsed   = createString(
        (negative || *evaluated.section.first == '+') + evaluated.section.first,
        evaluated.section.after);

    // Parse the string to a decimal number.
    Number decimal = parseDecimal(parsed);
    if (decimal.flag == NUMBER_TOO_PRECISE) {
      highlightError(
        context->reported, evaluated.section, "The number is too long!");
      disposeNumber(&decimal);
      return false;
    }

    Object object = {.known = true};

    // Default to an int.
    if (
      convertNumberToArithmetic(
        INT_TYPE_INSTANCE, &object.value, decimal, negative) !=
      NUMBER_CONVERSION_SUCCESS) {
      object.type = INT_TYPE_INSTANCE;
    } else {
      // Otherwise, it is inferred as double, which is always possible.
      expect(
        convertNumberToArithmetic(
          DOUBLE_TYPE_INSTANCE, &object.value, decimal, negative) ==
          NUMBER_CONVERSION_SUCCESS,
        "Failed to convert to a double!");
      object.type = DOUBLE_TYPE_INSTANCE;
    }
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    disposeNumber(&decimal);
    return true;
  }
  default: unexpected("Unknown nullary operator!");
  }
}

/* Version of `evaluateNode` with a prenary operator. */
static bool evaluatePrenaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer) {
  ExpressionNode evaluated = **pointer;
  (*pointer)--;
  switch (evaluated.operator) {
  // Prenary operators taking any arithmetic and returning it.
  case POSATE:
  case NEGATE: {
    // Check the operand.
    if (!evaluateArithmeticNode(context, built, pointer)) return false;
    EvaluationNode operand = built->after[-1];

    // Do integer promotion.
    Object object = {
      .type = findCombination(operand.object.type, INT_TYPE_INSTANCE)};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }

  // Prenary operator taking any arithmetic and returning bool.
  case LOGICAL_NOT: {
    // Check the operand.
    if (!evaluateArithmeticNode(context, built, pointer)) return false;

    // Result is always bool.
    Object object = {.type = BOOL_TYPE_INSTANCE};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }

  // Prenary operator taking any integer and returning it.
  case COMPLEMENT: {
    // Check the operand.
    if (!evaluateIntegerNode(context, built, pointer)) return false;
    EvaluationNode operand = built->after[-1];

    // Do integer promotion.
    Object object = {
      .type = findCombination(operand.object.type, INT_TYPE_INSTANCE)};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }

  // Prenary operators taking any arithmetic and returning void.
  case PREFIX_INCREMENT:
  case PREFIX_DECREMENT: {
    // Check the operand.
    if (!evaluateArithmeticNode(context, built, pointer)) return false;

    // Result is always void.
    Object object = {.type = VOID_TYPE_INSTANCE};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }
  default: unexpected("Unknown prenary operator!");
  }
}

/* Version of `evaluateNode` with a postary operator. */
static bool evaluatePostaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer) {
  ExpressionNode evaluated = **pointer;
  (*pointer)--;
  switch (evaluated.operator) {
  // Postary operators taking any arithmetic and returning void.
  case POSTFIX_INCREMENT:
  case POSTFIX_DECREMENT: {
    // Check the operand.
    if (!evaluateArithmeticNode(context, built, pointer)) return false;

    // Result is always void.
    Object object = {.type = VOID_TYPE_INSTANCE};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }
  default: unexpected("Unknown postary operator!");
  }
}

/* Version of `evaluateNode` with a cirnary operator. */
static bool evaluateCirnaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer) {
  ExpressionNode evaluated = **pointer;
  (*pointer)--;
  switch (evaluated.operator) {
  // Cirnary operator taking any type and returning it.
  case GROUP: {
    // Check the operand.
    if (!evaluateNode(context, built, pointer)) return false;
    EvaluationNode operand = built->after[-1];

    // Create with the the surrounded object.
    pushEvaluationNode(
      built,
      (EvaluationNode){.evaluated = evaluated, .object = operand.object});
    return true;
  }
  default: unexpected("Unknown cirnary operator!");
  }
}

/* Version of `evaluateNode` with a binary operator. */
static bool evaluateBinaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer) {
  ExpressionNode evaluated = **pointer;
  (*pointer)--;
  switch (evaluated.operator) {
  // Binary operators taking any arithmetic and returning it.
  case MULTIPLICATION:
  case DIVISION:
  case REMINDER:
  case ADDITION:
  case SUBTRACTION: {
    // Store the result for reporting multiple errors.
    bool result = true;

    // Check the operands.
    EvaluationNode leftOperand;
    EvaluationNode rightOperand;
    if (!evaluateArithmeticNode(context, built, pointer)) result = false;
    else rightOperand = built->after[-1];
    if (!evaluateArithmeticNode(context, built, pointer)) result = false;
    else leftOperand = built->after[-1];
    if (!result) return false;

    // Find the resultant type.
    Object object = {
      .type =
        findCombination(leftOperand.object.type, rightOperand.object.type)};

    // Do integer promotion.
    object.type = findCombination(object.type, INT_TYPE_INSTANCE);
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }

  // Binary operators taking any integer and returning it.
  case LEFT_SHIFT:
  case RIGHT_SHIFT:
  case BITWISE_AND:
  case BITWISE_XOR:
  case BITWISE_OR: {
    // Store the result for reporting multiple errors.
    bool result = true;

    // Check the operands.
    EvaluationNode leftOperand;
    EvaluationNode rightOperand;
    if (!evaluateIntegerNode(context, built, pointer)) result = false;
    else rightOperand = built->after[-1];
    if (!evaluateIntegerNode(context, built, pointer)) result = false;
    else leftOperand = built->after[-1];
    if (!result) return false;

    // Find the resultant type.
    Object object = {
      .type =
        findCombination(leftOperand.object.type, rightOperand.object.type)};

    // Do integer promotion.
    object.type = findCombination(object.type, INT_TYPE_INSTANCE);
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }

  // Binary operators taking any arithmetic and returning bool.
  case SMALLER_THAN:
  case SMALLER_THAN_OR_EQUAL_TO:
  case GREATER_THAN:
  case GREATER_THAN_OR_EQUAL_TO:
  case EQUAL_TO:
  case NOT_EQUAL_TO:
  case LOGICAL_AND:
  case LOGICAL_OR: {
    // Check the operands.
    // Bitwise OR to report multiple errors.
    if (
      (int)!evaluateArithmeticNode(context, built, pointer) |
      !evaluateArithmeticNode(context, built, pointer))
      return false;

    // Result is always bool.
    Object object = {.type = BOOL_TYPE_INSTANCE};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }

  // Binary operator taking any type and returning void.
  case ASSIGNMENT: {
    // Store the result for reporting multiple errors.
    bool result = true;

    // Check the operands.
    EvaluationNode leftOperand;
    EvaluationNode rightOperand;
    if (!evaluateNode(context, built, pointer)) result = false;
    else rightOperand = built->after[-1];
    if (!evaluateNode(context, built, pointer)) result = false;
    else leftOperand = built->after[-1];
    if (!result) return false;

    // Check assignment.
    if (!checkAssignment(
          context, rightOperand.evaluated.section, rightOperand.object,
          leftOperand))
      return false;

    // Result is always void.
    Object object = {.type = VOID_TYPE_INSTANCE};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }

  // Binary operators taking any arithmetic and returning void.
  case MULTIPLICATION_ASSIGNMENT:
  case DIVISION_ASSIGNMENT:
  case REMINDER_ASSIGNMENT:
  case ADDITION_ASSIGNMENT:
  case SUBTRACTION_ASSIGNMENT: {
    // Store the result for reporting multiple errors.
    bool result = true;

    // Check the operands.
    EvaluationNode leftOperand;
    EvaluationNode rightOperand;
    if (!evaluateArithmeticNode(context, built, pointer)) result = false;
    else rightOperand = built->after[-1];
    if (!evaluateArithmeticNode(context, built, pointer)) result = false;
    else leftOperand = built->after[-1];
    if (!result) return false;

    // Build assigned object and check.
    Object assigned = {
      .type =
        findCombination(leftOperand.object.type, rightOperand.object.type)};

    // Do integer promotion.
    assigned.type = findCombination(assigned.type, INT_TYPE_INSTANCE);
    if (!checkAssignment(
          context, rightOperand.evaluated.section, assigned, leftOperand))
      return false;

    // Result is always void.
    Object object = {.type = VOID_TYPE_INSTANCE};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }

  // Binary operators taking any integer and returning void.
  case LEFT_SHIFT_ASSIGNMENT:
  case RIGHT_SHIFT_ASSIGNMENT:
  case BITWISE_AND_ASSIGNMENT:
  case BITWISE_XOR_ASSIGNMENT:
  case BITWISE_OR_ASSIGNMENT: {
    // Store the result for reporting multiple errors.
    bool result = true;

    // Check the operands.
    EvaluationNode leftOperand;
    EvaluationNode rightOperand;
    if (!evaluateIntegerNode(context, built, pointer)) result = false;
    else rightOperand = built->after[-1];
    if (!evaluateIntegerNode(context, built, pointer)) result = false;
    else leftOperand = built->after[-1];
    if (!result) return false;

    // Build assigned object and check.
    Object assigned = {
      .type =
        findCombination(leftOperand.object.type, rightOperand.object.type)};

    // Do integer promotion.
    assigned.type = findCombination(assigned.type, INT_TYPE_INSTANCE);
    if (!checkAssignment(
          context, rightOperand.evaluated.section, assigned, leftOperand))
      return false;

    // Result is always void.
    Object object = {.type = VOID_TYPE_INSTANCE};
    pushEvaluationNode(
      built, (EvaluationNode){.evaluated = evaluated, .object = object});
    return true;
  }
  default: unexpected("Unknown binary operator!");
  }
}

/* Version of `evaluateNode` with a variary operator. */
static bool evaluateVariaryNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer) {
  ExpressionNode evaluated = **pointer;
  (*pointer)--;
  switch (evaluated.operator) {
  case FUNCTION_CALL:
    highlightWarning(
      context->reported, evaluated.section,
      "Function call is not yet implemented.");
    return false;
  default: unexpected("Unknown variary operator!");
  }
}

/* Evaluate the given expression node and move it to the node that comes after
 * the given one and its childeren. Returns whether the evaluation failed. */
static bool evaluateNode(
  Context* context, Evaluation* built, ExpressionNode const** pointer) {
  ExpressionNode evaluated = **pointer;
  printf(
    "Evaluating: [%s] `%.*s`\n", nameOperator(getOperator(evaluated.operator)),
    (int)countCharacters(evaluated.section), evaluated.section.first);
  switch (getOperator(evaluated.operator).tag) {
  case OPERATOR_NULLARY: return evaluateNullaryNode(context, built, pointer);
  case OPERATOR_PRENARY: return evaluatePrenaryNode(context, built, pointer);
  case OPERATOR_POSTARY: return evaluatePostaryNode(context, built, pointer);
  case OPERATOR_CIRNARY: return evaluateCirnaryNode(context, built, pointer);
  case OPERATOR_BINARY: return evaluateBinaryNode(context, built, pointer);
  case OPERATOR_VARIARY: return evaluateVariaryNode(context, built, pointer);
  default: unexpected("Unknown operator variant!");
  }
}

/* Evaluate the given expression and build it into the given evaluation. Returns
 * whether the expression was successful. */
static bool
evaluateExpression(Context* context, Evaluation* built, Expression evaluated) {
  // Evaluate starting from the root node.
  ExpressionNode const* start = evaluated.after - 1;
  return evaluateNode(context, built, &start);
}

/* Evaluate the type in the given expression and build it into the given type.
 * Returns whether the expression is correct and the type of the given
 * expression is the meta type. */
static bool evaluateType(Context* context, Type* built, Expression resolved) {
  // Evaluate the type expression and expect the meta type.
  Evaluation evaluation = createEvaluation(0);
  if (!checkExpression(context, &evaluation, resolved, META_TYPE_INSTANCE))
    return false;

  // Check whether the value, which is the type, is known.
  Object evaluated = evaluation.after[-1].object;
  if (!evaluated.known) {
    highlightError(
      context->reported, getExpressionSection(resolved),
      "Type must be known at compile-time!");
    return false;
  }

  *built = evaluated.value.asType;
  return true;
}

/* Whether the given type is a valid binding type. Reports the given source
 * section on error. */
static bool
checkBindingType(Context* context, Type checked, String highlighted) {
  // Binding must be of a non-void type.
  if (compareTypeEquality(checked, VOID_TYPE_INSTANCE)) {
    highlightError(
      context->reported, highlighted, "Type of a binding cannot be `%s`!",
      nameType(checked));
    return false;
  }

  return true;
}

/* Whether the given type is a valid variable type. Reports the given source
 * section on error. */
static bool
checkVariableType(Context* context, Type checked, String highlighted) {
  // Variable must be of a non-void and non-meta type.
  if (
    compareTypeEquality(checked, VOID_TYPE_INSTANCE) ||
    compareTypeEquality(checked, META_TYPE_INSTANCE)) {
    highlightError(
      context->reported, highlighted, "Type of a variable cannot be `%s`!",
      nameType(checked));
    return false;
  }

  return true;
}

/* Add the given name to the failed definitions. */
static void recordFailedDefinition(Context* context, String name) {
  insertMember(&context->failedNames, name);
}

/* Resolve the given binding definition. */
static void
resolveBindingDefinition(Context* context, BindingDefinition resolved) {
  // Check name.
  if (!checkDefinedName(context, resolved.name)) return;

  // Check type.
  Type definedType;
  if (
    !evaluateType(context, &definedType, resolved.type) ||
    !checkBindingType(
      context, definedType, getExpressionSection(resolved.type))) {
    recordFailedDefinition(context, resolved.name);
    return;
  }

  // Check value.
  Evaluation definedValue = createEvaluation(0);
  if (!checkExpression(context, &definedValue, resolved.value, definedType)) {
    recordFailedDefinition(context, resolved.name);
    return;
  }

  // Create a symbol with the given type and value.
  Binding        binding = {.bound = definedValue};
  EvaluationNode root    = definedValue.after[-1];
  Object         object  = {
             .type  = definedType,
             .value = root.object.value,
             .known = root.object.known};
  addSymbol(
    context, (Symbol){
               .asBinding   = binding,
               .tag         = SYMBOL_BINDING,
               .name        = resolved.name,
               .object      = object,
               .userDefined = true});
}

/* Resolve the given inferred binding definition. */
static void resolveInferredBindingDefinition(
  Context* context, InferredBindingDefinition resolved) {
  // Check name.
  if (!checkDefinedName(context, resolved.name)) return;

  // Check value.
  Evaluation definedValue = createEvaluation(0);
  if (!evaluateExpression(context, &definedValue, resolved.value)) {
    recordFailedDefinition(context, resolved.name);
    return;
  }

  // Create a symbol with the given value.
  Binding binding = {.bound = definedValue};
  addSymbol(
    context, (Symbol){
               .asBinding   = binding,
               .tag         = SYMBOL_BINDING,
               .name        = resolved.name,
               .object      = definedValue.after[-1].object,
               .userDefined = true});
}

/* Resolve the given variable definition. */
static void
resolveVariableDefinition(Context* context, VariableDefinition resolved) {
  // Check name.
  if (!checkDefinedName(context, resolved.name)) return;

  // Check type.
  Type definedType;
  if (
    !evaluateType(context, &definedType, resolved.type) ||
    !checkVariableType(
      context, definedType, getExpressionSection(resolved.type))) {
    recordFailedDefinition(context, resolved.name);
    return;
  }

  // Check value.
  Evaluation definedValue = createEvaluation(0);
  if (!checkExpression(
        context, &definedValue, resolved.initialValue, definedType)) {
    recordFailedDefinition(context, resolved.name);
    return;
  }

  // Create a symbol with the given type and value.
  Variable       variable = {.initial = definedValue, .defaulted = false};
  EvaluationNode root     = definedValue.after[-1];
  Object         object   = {
              .type  = definedType,
              .value = root.object.value,
              .known = root.object.known};
  addSymbol(
    context, (Symbol){
               .asVariable  = variable,
               .tag         = SYMBOL_VARIABLE,
               .name        = resolved.name,
               .object      = object,
               .userDefined = true});
}

/* Resolve the given inferred variable definition. */
static void resolveInferredVariableDefinition(
  Context* context, InferredVariableDefinition resolved) {
  // Check name.
  if (!checkDefinedName(context, resolved.name)) return;

  // Check value.
  Evaluation definedValue = createEvaluation(0);
  if (!evaluateExpression(context, &definedValue, resolved.initialValue)) {
    recordFailedDefinition(context, resolved.name);
    return;
  }

  // Create a symbol with the given value.
  Variable variable = {.initial = definedValue, .defaulted = false};
  addSymbol(
    context, (Symbol){
               .asVariable  = variable,
               .tag         = SYMBOL_VARIABLE,
               .name        = resolved.name,
               .object      = definedValue.after[-1].object,
               .userDefined = true});
}

/* Resolve the given defaulted variable definition. */
static void resolveDefaultedVariableDefinition(
  Context* context, DefaultedVariableDefinition resolved) {
  // Check name.
  if (!checkDefinedName(context, resolved.name)) return;

  // Check type.
  Type definedType;
  if (
    !evaluateType(context, &definedType, resolved.type) ||
    !checkVariableType(
      context, definedType, getExpressionSection(resolved.type)) ||
    !checkDefaultability(definedType)) {
    recordFailedDefinition(context, resolved.name);
    return;
  }

  // Create a symbol with the given type and default value of the type.
  Variable variable = {.defaulted = true};
  Object   object   = {
        .type = definedType, .value = defaultValue(definedType), .known = true};
  addSymbol(
    context, (Symbol){
               .asVariable  = variable,
               .tag         = SYMBOL_VARIABLE,
               .name        = resolved.name,
               .object      = object,
               .userDefined = true});
}

/* Resolve the given discarded expression. */
static void
resolveDiscardedExpression(Context* context, DiscardedExpression resolved) {
  // Check the discarded expression.
  Evaluation discarded = createEvaluation(0);
  if (!checkExpression(
        context, &discarded, resolved.discarded, VOID_TYPE_INSTANCE))
    return;

  highlightWarning(
    context->reported, getExpressionSection(resolved.discarded),
    "Expression statement in the global scope is skipped for now. Later it "
    "will not be allowed.");
}

/* Check the symbol accesses and types of the expressions in the user-defined
 * symbols. */
static void resolve(Context* context) {
  for (Statement* statement = context->analyzed.first;
       statement < context->analyzed.after; statement++) {
    switch (statement->tag) {
    case STATEMENT_BINDING_DEFINITION:
      resolveBindingDefinition(context, statement->asBindingDefinition);
      break;
    case STATEMENT_INFERRED_BINDING_DEFINITION:
      resolveInferredBindingDefinition(
        context, statement->asInferredBindingDefinition);
      break;
    case STATEMENT_VARIABLE_DEFINITION:
      resolveVariableDefinition(context, statement->asVariableDefinition);
      break;
    case STATEMENT_INFERRED_VARIABLE_DEFINITION:
      resolveInferredVariableDefinition(
        context, statement->asInferredVariableDefinition);
      break;
    case STATEMENT_DEFAULTED_VARIABLE_DEFINITION:
      resolveDefaultedVariableDefinition(
        context, statement->asDefaultedVariableDefinition);
      break;
    case STATEMENT_DISCARDED_EXPRESSION:
      resolveDiscardedExpression(context, statement->asDiscardedExpression);
      break;
    default: unexpected("Unknown statement variant!");
    }
  }
}

/* Run the analyzer with the given context. */
void analyze(Context* context) {
  prepare(context);
  resolve(context);
}

void analyzeParse(Table* target, Source* reported, Parse analyzed) {
  // Create a context and pass its pointer, because all functions take a context
  // pointer and this removes the need for taking the adress of the context in
  // the main anaylsis function.
  analyze(&(Context){
    .target         = target,
    .reported       = reported,
    .analyzed       = analyzed,
    .namesToIndices = createMap(0),
    .failedNames    = createSet(0)});
}
