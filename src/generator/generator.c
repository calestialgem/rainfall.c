// FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// License-Identifier: GPL-3.0-or-later

#include "analyzer/api.h"
#include "analyzer/mod.h"
#include "generator/api.h"
#include "parser/api.h"
#include "utility/api.h"

#include <float.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

/* Context of the generation process. */
typedef struct {
  /* Stream to write the symbols into. */
  FILE* target;
  /* Generated table. */
  Table generated;
  /* Amount of indentation to generate. */
  int   indentation;
} Context;

/* Generate a new line. */
static void generateNewLine(Context* context) {
  fputc('\n', context->target);
  for (int level = 0; level < context->indentation; level++)
    fputs("  ", context->target);
}

/* Generate the include for the given library name. */
static void generateInclude(Context* context, char const* generated) {
  generateNewLine(context);
  fprintf(context->target, "#include <%s.h>", generated);
}

/* Generate the definition of the given type. */
static void generateType(Context* context, Type generated) {
  switch (generated.tag) {
  case TYPE_BYTE:
    generateNewLine(context);
    fprintf(context->target, "typedef char %s;", nameType(generated));
    break;
  case TYPE_UXS:
    generateNewLine(context);
    fprintf(context->target, "typedef size_t %s;", nameType(generated));
    break;
  case TYPE_BOOL:
  case TYPE_META:
  case TYPE_VOID:
  case TYPE_INT:
  case TYPE_FLOAT:
  case TYPE_DOUBLE: break;
  default: unexpected("Unknown type symbol!");
  }
}

/* Generate a usecase for the given type. */
static void generateTypeUsage(Context* context, Type used) {
  switch (used.tag) {
  case TYPE_VOID:
  case TYPE_BOOL:
  case TYPE_BYTE:
  case TYPE_INT:
  case TYPE_UXS:
  case TYPE_FLOAT:
  case TYPE_DOUBLE: fputs(nameType(used), context->target); break;
  case TYPE_META: unexpected("Could not use meta type!");
  default: unexpected("Unknown type symbol!");
  }
}

/* Generate the given value of the given type as a literal. */
static void
generateValue(Context* context, Type generatedType, Value generatedValue) {
  switch (generatedType.tag) {
  case TYPE_META: generateTypeUsage(context, generatedValue.asType); break;
  case TYPE_BOOL:
    fputs(generatedValue.asBool ? "true" : "false", context->target);
    break;
  case TYPE_BYTE:
    fprintf(context->target, "'%c'", generatedValue.asByte);
    break;
  case TYPE_INT: fprintf(context->target, "%i", generatedValue.asInt); break;
  case TYPE_UXS: fprintf(context->target, "%zu", generatedValue.asUxs); break;
  case TYPE_FLOAT:
    fprintf(context->target, "%.*g", FLT_DECIMAL_DIG, generatedValue.asFloat);
    break;
  case TYPE_DOUBLE:
    fprintf(context->target, "%.*g", DBL_DECIMAL_DIG, generatedValue.asDouble);
    break;
  case TYPE_VOID: unexpected("Could not generate a value of type void!");
  default: unexpected("Unknown type symbol!");
  }
}

/* Generate the given evaluation node and move the pointer to the node after all
 * the childeren of this one. Does not generate but moves the pointer if the
 * flag is true. */
static void
generateNode(Context* context, EvaluationNode const** pointer, bool justPass) {
  if (!justPass) fputc('(', context->target);
  EvaluationNode generated = **pointer;
  (*pointer)--;
  switch (getOperator(generated.evaluated.operator).tag) {
  case OPERATOR_NULLARY:
    switch (generated.evaluated.operator) {
    case SYMBOL_ACCESS:
      if (!justPass)
        fprintf(
          context->target, "%.*s",
          (int)countCharacters(generated.evaluated.section),
          generated.evaluated.section.first);
      break;
    case DECIMAL_LITERAL:
      if (!justPass)
        generateValue(context, generated.object.type, generated.object.value);
      break;
    default: unexpected("Unknown nullary operator!");
    }
    break;
  case OPERATOR_PRENARY:
    if (!justPass) switch (generated.evaluated.operator) {
      case POSATE: fputc('+', context->target); break;
      case NEGATE: fputc('-', context->target); break;
      case LOGICAL_NOT: fputc('!', context->target); break;
      case COMPLEMENT: fputc('~', context->target); break;
      case PREFIX_INCREMENT: fputs("++", context->target); break;
      case PREFIX_DECREMENT: fputs("--", context->target); break;
      default: unexpected("Unknown prenary operator!");
      }
    generateNode(context, pointer, justPass);
    break;
  case OPERATOR_POSTARY:
    generateNode(context, pointer, justPass);
    if (!justPass) switch (generated.evaluated.operator) {
      case POSTFIX_INCREMENT: fputs("++", context->target); break;
      case POSTFIX_DECREMENT: fputs("--", context->target); break;
      default: unexpected("Unknown postary operator!");
      }
    break;
  case OPERATOR_CIRNARY:
    switch (generated.evaluated.operator) {
    case GROUP:
      if (!justPass) fputc('(', context->target);
      generateNode(context, pointer, justPass);
      if (!justPass) fputc(')', context->target);
      break;
    default: unexpected("Unknown cirnary operator!");
    }
    break;
  case OPERATOR_BINARY:
    generateNode(context, pointer, justPass);
    if (!justPass) switch (generated.evaluated.operator) {
      case MULTIPLICATION: fputc('*', context->target); break;
      case DIVISION: fputc('/', context->target); break;
      case REMINDER: fputc('%', context->target); break;
      case ADDITION: fputc('+', context->target); break;
      case SUBTRACTION: fputc('-', context->target); break;
      case LEFT_SHIFT: fputs("<<", context->target); break;
      case RIGHT_SHIFT: fputs(">>", context->target); break;
      case BITWISE_AND: fputc('&', context->target); break;
      case BITWISE_XOR: fputc('^', context->target); break;
      case BITWISE_OR: fputc('|', context->target); break;
      case SMALLER_THAN: fputc('<', context->target); break;
      case SMALLER_THAN_OR_EQUAL_TO: fputs("<=", context->target); break;
      case GREATER_THAN: fputc('>', context->target); break;
      case GREATER_THAN_OR_EQUAL_TO: fputs(">=", context->target); break;
      case EQUAL_TO: fputs("==", context->target); break;
      case NOT_EQUAL_TO: fputs("!=", context->target); break;
      case LOGICAL_AND: fputs("&&", context->target); break;
      case LOGICAL_OR: fputs("||", context->target); break;
      case ASSIGNMENT: fputc('=', context->target); break;
      case MULTIPLICATION_ASSIGNMENT: fputs("*=", context->target); break;
      case DIVISION_ASSIGNMENT: fputs("=/", context->target); break;
      case REMINDER_ASSIGNMENT: fputs("%=", context->target); break;
      case ADDITION_ASSIGNMENT: fputs("+=", context->target); break;
      case SUBTRACTION_ASSIGNMENT: fputs("-=", context->target); break;
      case LEFT_SHIFT_ASSIGNMENT: fputs("<<=", context->target); break;
      case RIGHT_SHIFT_ASSIGNMENT: fputs(">>=", context->target); break;
      case BITWISE_AND_ASSIGNMENT: fputs("&=", context->target); break;
      case BITWISE_XOR_ASSIGNMENT: fputs("^=", context->target); break;
      case BITWISE_OR_ASSIGNMENT: fputs("|=", context->target); break;
      default: unexpected("Unknown binary operator!");
      }
    generateNode(context, pointer, justPass);
    break;
  case OPERATOR_VARIARY:
    switch (generated.evaluated.operator) {
    case FUNCTION_CALL: unexpected("Not implemented!");
    default: unexpected("Unknown variary operator!");
    }
    break;
  default: unexpected("Unknown operator variant!");
  }
  if (!justPass) fputc(')', context->target);
}

/* Generate the given evaluation. */
static void generateEvaluation(Context* context, Evaluation generated) {
  EvaluationNode const* start = generated.after - 1;
  generateNode(context, &start, false);
}

/* Generate the given symbol. */
static void generateSymbol(Context* context, Symbol generated) {
  if (!generated.userDefined) return;
  generateNewLine(context);
  switch (generated.tag) {
  case SYMBOL_BINDING:
    generateTypeUsage(context, generated.object.type);
    fprintf(
      context->target, " const %.*s = ", (int)countCharacters(generated.name),
      generated.name.first);
    generateEvaluation(context, generated.asBinding.bound);
    fputc(';', context->target);
    break;
  case SYMBOL_VARIABLE:
    generateTypeUsage(context, generated.object.type);
    fprintf(
      context->target, " %.*s = ", (int)countCharacters(generated.name),
      generated.name.first);
    if (generated.asVariable.defaulted)
      generateValue(context, generated.object.type, generated.object.value);
    else generateEvaluation(context, generated.asVariable.initial);
    fputc(';', context->target);
    break;
  case SYMBOL_TYPE: unexpected("Could not generate a type symbol!");
  default: unexpected("Unknown symbol variant!");
  }
}

/* Year that the year of local date starts counting. */
#define YEAR_START 1900

/* Run the generator with the given context. */
static void generate(Context* context) {
  time_t    now           = time(NULL);
  struct tm localDateTime = *localtime(&now);
  fprintf(
    context->target,
    "// Generated by Rainfall.c on %i.%02i.%02i at %02i.%02i.%02i.",
    localDateTime.tm_year + YEAR_START, localDateTime.tm_mon + 1,
    localDateTime.tm_mday, localDateTime.tm_hour, localDateTime.tm_min,
    localDateTime.tm_sec);
  generateNewLine(context);

  generateInclude(context, "stddef");  // size_t
  generateInclude(context, "stdbool"); // bool, true, false
  generateNewLine(context);

  // First generate all type symbols.
  for (Symbol const* symbol = context->generated.first;
       symbol < context->generated.after; symbol++)
    if (symbol->tag == SYMBOL_TYPE)
      generateType(context, symbol->object.value.asType);
  generateNewLine(context);

  generateNewLine(context);
  fprintf(
    context->target,
    "int main(int argumentCount, char const* const* argumentArray) {");
  context->indentation++;

  // Then, generate non-type symbols.
  for (Symbol const* symbol = context->generated.first;
       symbol < context->generated.after; symbol++)
    if (symbol->tag != SYMBOL_TYPE) generateSymbol(context, *symbol);

  context->indentation--;
  generateNewLine(context);
  fputc('}', context->target);
  generateNewLine(context);
}

void generateTable(Table generated) {
  // Open the target file that will be generated.
  FILE* target = fopen("build.c", "w");
  expect(target, "Could not open the generated file!");

  // Create a context and pass its pointer, because all functions take a context
  // pointer and this removes the need for taking the address of the context in
  // the main generation function.
  generate(
    &(Context){.target = target, .generated = generated, .indentation = 0});

  expect(fclose(target) == 0, "Could not close the generated file!");
}
