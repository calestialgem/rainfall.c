// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "otc/api.h"
#include "otc/mod.h"
#include "utl/api.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <vadefs.h>

/* Log the formatted message at the given level for the given source file to the
 * given stream. Prints the message by passing the variable arguments and the
 * format string to `fprintf`. */
#define reportArguments(reportedSource, targetStream, logLevelString)      \
  do {                                                                     \
    /* Transfer the variable amount of arguments to `fprintf`. */          \
    va_list arguments = NULL;                                              \
    va_start(arguments, format);                                           \
    /* First print the file information; then, the message. */             \
    fprintf(                                                               \
      targetStream, "%s.tr: %s: ", (reportedSource).name, logLevelString); \
    vfprintf(targetStream, format, arguments);                             \
    fputc('\n', targetStream);                                             \
    va_end(arguments);                                                     \
  } while (false)

/* Log the formatted message at the given level for the given part of the given
 * source file and highlight it to the given stream. Prints the message by
 * passing the variable arguments and the format string to `fprintf`. */
#define highlightArguments(reportedSource, targetStream, logLevelString)      \
  do {                                                                        \
    /* Transfer the variable amount of arguments to `fprintf`. */             \
    va_list arguments = NULL;                                                 \
    va_start(arguments, format);                                              \
    Portion portion = createPortion(reportedSource, highlighted);             \
    /* First print the file and line information; then, the message. */       \
    fprintf(                                                                  \
      targetStream, "%s.tr:%u:%u:%u:%u: %s: ", (reportedSource).name,         \
      portion.first.line, portion.first.column, portion.last.line,            \
      portion.last.column + 1, logLevelString);                               \
    vfprintf(targetStream, format, arguments);                                \
    fputc('\n', targetStream);                                                \
    /* Underline the portion that should be highlighted after the message. */ \
    underlinePortion(portion, targetStream);                                  \
    va_end(arguments);                                                        \
  } while (false)

Source createSource(char const* name) {
  // Join the name with the extension.
  Buffer path = createBuffer(strlen(name) + 4);
  appendString(&path, viewTerminated(name));
  appendCharacter(&path, '.');
  appendString(&path, viewTerminated("tr"));
  appendCharacter(&path, 0);

  // Open and buffer the source file contents.
  FILE* source = fopen(path.first, "r");
  disposeBuffer(&path);
  expect(source, "Could not open file!");

  Buffer contents = createBuffer(0);
  appendStream(&contents, source);

  // Put the null-terminator as end of file character, and a new line, which
  // makes sure that there is always a line that could be reported to user.
  appendCharacter(&contents, 0);
  appendCharacter(&contents, '\n');

  return (Source){
    .name = name, .contents = contents, .errors = 0, .warnings = 0};
}

void disposeSource(Source* disposed) { disposeBuffer(&disposed->contents); }

void reportError(Source* reported, char const* format, ...) {
  reportArguments(*reported, stderr, "error");
  reported->errors++;
}

void reportWarning(Source* reported, char const* format, ...) {
  reportArguments(*reported, stdout, "warning");
  reported->warnings++;
}

void reportInfo(Source* reported, char const* format, ...) {
  reportArguments(*reported, stdout, "info");
}

void highlightError(
  Source* reported, String highlighted, char const* format, ...) {
  highlightArguments(*reported, stderr, "error");
  reported->errors++;
}

void highlightWarning(
  Source* reported, String highlighted, char const* format, ...) {
  highlightArguments(*reported, stdout, "warning");
  reported->warnings++;
}

void highlightInfo(
  Source* reported, String highlighted, char const* format, ...) {
  highlightArguments(*reported, stdout, "info");
}
