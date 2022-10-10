// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "otc/api.h"
#include "otc/mod.h"
#include "utl/api.h"

#include <stdarg.h>
#include <stdio.h>
#include <vadefs.h>

/* Log the given formatted message at the given level for the given part of the
 * given source file to the given stream. */
static void log(
  Source source, FILE* stream, char const* level, String section,
  char const* format, va_list args) {
  Portion por = portionOf(source, section);
  fprintf(
    stream, "%s:%u:%u:%u:%u: %s: ", source.name, por.first.line, por.first.cl,
    por.end.line, por.end.cl, level);

  vfprintf(stream, format, args);
  fprintf(stream, "\n");

  underline(por, stream);
}

/* Log the given formatted message at the given level for the given source file
 * to the given stream. */
static void logWhole(
  Source source, FILE* stream, char const* level, char const* format,
  va_list args) {
  fprintf(stream, "%s: %s: ", source.name, level);

  vfprintf(stream, format, args);
  fprintf(stream, "\n");
}

/* Calls `log` with the given source, stream and level. */
#define logArgs(stream, level)                          \
  do {                                                  \
    va_list args = NULL;                                \
    va_start(args, format);                             \
    log(*source, stream, level, section, format, args); \
    va_end(args);                                       \
  } while (false)

/* Calls `logWhole` with the given source, stream and level. */
#define logWholeArgs(stream, level)                 \
  do {                                              \
    va_list args = NULL;                            \
    va_start(args, format);                         \
    logWhole(*source, stream, level, format, args); \
    va_end(args);                                   \
  } while (false)

Source loadSource(char const* name) {
  // Join the name with the extension.
  Buffer path = emptyBuffer();
  append(&path, nullTerminated(name));
  put(&path, '.');
  append(&path, nullTerminated("tr"));
  put(&path, 0);

  FILE* stream = fopen(path.first, "r");
  disposeBuffer(&path);
  expect(stream, "Could not open file!");

  Buffer contents = emptyBuffer();
  read(&contents, stream);

  // Put the null-terminator as end of file character, and a new line, which
  // makes sure that there is always a line that could be reported to user.
  put(&contents, 0);
  put(&contents, '\n');

  return (Source){
    .name = name, .contents = contents, .errors = 0, .warnings = 0};
}

void disposeSource(Source* source) { disposeBuffer(&source->contents); }

void highlightError(Source* source, String section, char const* format, ...) {
  logArgs(stderr, "error");
  source->errors++;
}

void highlightWarning(Source* source, String section, char const* format, ...) {
  logArgs(stdout, "warning");
  source->warnings++;
}

void highlightInformation(
  Source* source, String section, char const* format, ...) {
  logArgs(stdout, "info");
}

void reportError(Source* source, char const* format, ...) {
  logWholeArgs(stderr, "error");
  source->errors++;
}

void reportWarning(Source* source, char const* format, ...) {
  logWholeArgs(stdout, "warning");
  source->warnings++;
}

void reportInformation(Source* source, char const* format, ...) {
  logWholeArgs(stdout, "info");
}
