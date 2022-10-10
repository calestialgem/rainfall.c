// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "utl/api.h"

/* Contents of a source file. */
typedef struct {
  /* Relative path of the source file without the file extension. */
  char const* name;
  /* Contents of the source file. */
  Buffer      contents;
  /* Amount of errors orginated in the source file. */
  int         errors;
  /* Amount of warnings orginated in the source file. */
  int         warnings;
} Source;

/* Load the source file at the given name. */
Source loadSource(char const* name);
/* Dispose the contents of the given source file. */
void   disposeSource(Source*);
/* Report an error and highlight the given section of the given source file with
 * the given formatted message. */
void   highlightError(Source*, String section, char const* format, ...);
/* Report a warning and highlight the given section of the given source file
 * with the given formatted message. */
void   highlightWarning(Source*, String section, char const* format, ...);
/* Report an information and highlight the given section of the given source
 * file with the given formatted message. */
void   highlightInfo(Source*, String section, char const* format, ...);
/* Report an error at the given source file with the given formatted message. */
void   reportError(Source*, char const* format, ...);
/* Report a warning at the given source file with the given formatted message.
 */
void   reportWarning(Source*, char const* format, ...);
/* Report an information at the given source file with the given formatted
 * message. */
void   reportInfo(Source*, char const* format, ...);
