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

/* Log the given formatted message at the given level for the given part of the
 * given source file to the given stream. */
static void highlight(
  Source s, FILE* f, char const* level, String section, char const* format,
  va_list v) {
  Portion p = portionOf(s, section);
  fprintf(
    f, "%s:%u:%u:%u:%u: %s: ", s.name, p.first.line, p.first.column,
    p.last.line, p.last.column, level);

  vfprintf(f, format, v);
  fputc('\n', f);

  underline(p, f);
}

/* Log the given formatted message at the given level for the given source file
 * to the given stream. */
static void
report(Source s, FILE* f, char const* level, char const* format, va_list v) {
  fprintf(f, "%s: %s: ", s.name, level);

  vfprintf(f, format, v);
  fputc('\n', f);
}

/* Passes the varargs to `highlight` with the given stream and level. */
#define highlightArgs(stream, level)                     \
  do {                                                   \
    va_list args = NULL;                                 \
    va_start(args, format);                              \
    highlight(*s, stream, level, section, format, args); \
    va_end(args);                                        \
  } while (false)

/* Passes the varargs to `report` with the given stream and level. */
#define reportArgs(stream, level)            \
  do {                                       \
    va_list args = NULL;                     \
    va_start(args, format);                  \
    report(*s, stream, level, format, args); \
    va_end(args);                            \
  } while (false)

Source loadSource(char const* name) {
  // Join the name with the extension.
  Buffer path = emptyBuffer();
  append(&path, nullTerminated(name));
  put(&path, '.');
  append(&path, nullTerminated("tr"));
  put(&path, 0);

  FILE* f = fopen(path.first, "r");
  disposeBuffer(&path);
  expect(f, "Could not open file!");

  Buffer contents = emptyBuffer();
  read(&contents, f);

  // Put the null-terminator as end of file character, and a new line, which
  // makes sure that there is always a line that could be reported to user.
  put(&contents, 0);
  put(&contents, '\n');

  return (Source){
    .name = name, .contents = contents, .errors = 0, .warnings = 0};
}

void disposeSource(Source* s) { disposeBuffer(&s->contents); }

void highlightError(Source* s, String section, char const* format, ...) {
  highlightArgs(stderr, "error");
  s->errors++;
}

void highlightWarning(Source* s, String section, char const* format, ...) {
  highlightArgs(stdout, "warning");
  s->warnings++;
}

void highlightInfo(Source* s, String section, char const* format, ...) {
  highlightArgs(stdout, "info");
}

void reportError(Source* s, char const* format, ...) {
  reportArgs(stderr, "error");
  s->errors++;
}

void reportWarning(Source* s, char const* format, ...) {
  reportArgs(stdout, "warning");
  s->warnings++;
}

void reportInfo(Source* s, char const* format, ...) {
  reportArgs(stdout, "info");
}
