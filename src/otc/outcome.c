// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "otc/api.h"
#include "otc/mod.h"
#include "utl/api.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <vadefs.h>

/* Log the given formatted message at the given level for the given part of the
 * given source file to the given stream. */
static void log(
  Source const src, FILE* stream, char const* lvl, String const part,
  char const* const fmt, va_list const args) {
  Portion const por = porOf(src, part);
  fprintf(
    stream, "%s:%u:%u:%u:%u: %s: ", src.name, por.bgn.ln, por.bgn.cl,
    por.end.ln, por.end.cl, lvl);

  vfprintf(stream, fmt, args);
  fprintf(stream, "\n");

  porUnderline(por, stream);
}

/* Log the given formatted message at the given level for the given source file
 * to the given stream. */
static void logWhole(
  Source const src, FILE* stream, char const* lvl, char const* const fmt,
  va_list const args) {
  fprintf(stream, "%s: %s: ", src.name, lvl);

  vfprintf(stream, fmt, args);
  fprintf(stream, "\n");
}

/* Calls `log` with the given source, stream and level. */
#define logArgs(src, stream, lvl)           \
  do {                                      \
    va_list args = NULL;                    \
    va_start(args, fmt);                    \
    log(src, stream, lvl, part, fmt, args); \
    va_end(args);                           \
  } while (false)

/* Calls `logWhole` with the given source, stream and level. */
#define logWholeArgs(src, stream, lvl)     \
  do {                                     \
    va_list args = NULL;                   \
    va_start(args, fmt);                   \
    logWhole(src, stream, lvl, fmt, args); \
    va_end(args);                          \
  } while (false)

Outcome otcOf(Source const src) { return (Outcome){.src = src}; }

void otcErr(Outcome* const otc, String const part, char const* const fmt, ...) {
  logArgs(otc->src, stderr, "error");
  otc->err++;
}

void otcWrn(Outcome* const otc, String const part, char const* const fmt, ...) {
  logArgs(otc->src, stdout, "warning");
  otc->wrn++;
}

void otcInfo(Outcome const otc, String const part, char const* const fmt, ...) {
  logArgs(otc.src, stdout, "info");
}

void otcErrWhole(Outcome* const otc, char const* const fmt, ...) {
  logWholeArgs(otc->src, stderr, "error");
  otc->err++;
}

void otcWrnWhole(Outcome* const otc, char const* const fmt, ...) {
  logWholeArgs(otc->src, stdout, "warning");
  otc->wrn++;
}

void otcInfoWhole(Outcome const otc, char const* const fmt, ...) {
  logWholeArgs(otc.src, stdout, "info");
}
