// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "utility/api.h"

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
Source createSource(char const* loadedFileName);
/* Dispose the contents of the given source file. */
void   disposeSource(Source* disposed);
/* Report an error at the given source file with the given formatted message. */
void   reportError(Source* reported, char const* format, ...);
/* Report a warning at the given source file with the given formatted message.
 */
void   reportWarning(Source* reported, char const* format, ...);
/* Report an information at the given source file with the given formatted
 * message. */
void   reportInfo(Source* reported, char const* format, ...);
/* Report an error and highlight the given section of the given source file with
 * the given formatted message. */
void   highlightError(
    Source* reported, String highlighted, char const* format, ...);
/* Report a warning and highlight the given section of the given source file
 * with the given formatted message. */
void highlightWarning(
  Source* reported, String highlighted, char const* format, ...);
/* Report an information and highlight the given section of the given source
 * file with the given formatted message. */
void highlightInfo(
  Source* reported, String highlighted, char const* format, ...);
