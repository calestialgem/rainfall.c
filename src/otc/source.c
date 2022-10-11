// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "otc/api.h"
#include "otc/mod.h"
#include "utl/api.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <vadefs.h>

/* Log the formatted message at the given level for the given source file to the
 * given stream. */
#define reportArguments(source, target, level)         \
  do {                                                 \
    va_list arguments = NULL;                          \
    va_start(arguments, format);                       \
    fprintf(target, "%s: %s: ", (source).name, level); \
    vfprintf(target, format, arguments);               \
    fputc('\n', target);                               \
    va_end(arguments);                                 \
  } while (false)

/* Log the formatted message at the given level for the given part of the given
 * source file and highlight it to the given stream. */
#define highlightArguments(source, target, level)                           \
  do {                                                                      \
    va_list arguments = NULL;                                               \
    va_start(arguments, format);                                            \
    Portion portion = portionAt(source, section);                           \
    fprintf(                                                                \
      target, "%s:%u:%u:%u:%u: %s: ", (source).name, portion.first.line,    \
      portion.first.column, portion.last.line, portion.last.column, level); \
    vfprintf(target, format, arguments);                                    \
    fputc('\n', target);                                                    \
    underline(portion, target);                                             \
    va_end(arguments);                                                      \
  } while (false)

Source loadSource(char const* name) {
  // Join the name with the extension.
  Buffer path = emptyBuffer();
  append(&path, nullTerminated(name));
  put(&path, '.');
  append(&path, nullTerminated("tr"));
  put(&path, 0);

  FILE* source = fopen(path.first, "r");
  disposeBuffer(&path);
  expect(source, "Could not open file!");

  Buffer contents = emptyBuffer();
  read(&contents, source);

  // Put the null-terminator as end of file character, and a new line, which
  // makes sure that there is always a line that could be reported to user.
  put(&contents, 0);
  put(&contents, '\n');

  return (Source){
    .name = name, .contents = contents, .errors = 0, .warnings = 0};
}

void disposeSource(Source* target) { disposeBuffer(&target->contents); }

void reportError(Source* target, char const* format, ...) {
  reportArguments(*target, stderr, "error");
  target->errors++;
}

void reportWarning(Source* target, char const* format, ...) {
  reportArguments(*target, stdout, "warning");
  target->warnings++;
}

void reportInfo(Source* target, char const* format, ...) {
  reportArguments(*target, stdout, "info");
}

void highlightError(Source* target, String section, char const* format, ...) {
  highlightArguments(*target, stderr, "error");
  target->errors++;
}

void highlightWarning(Source* target, String section, char const* format, ...) {
  highlightArguments(*target, stdout, "warning");
  target->warnings++;
}

void highlightInfo(Source* target, String section, char const* format, ...) {
  highlightArguments(*target, stdout, "info");
}
