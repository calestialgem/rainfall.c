// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "utl/api.h"

/* Contents of a source file. */
typedef struct {
  /* Relative path of the source file without the file extension. */
  char const* name;
  /* Contents of the source file. */
  Buffer      con;
} Source;

/* Compilation result of a source file. */
typedef struct {
  /* Source file that results belongs to. */
  Source src;
  /* Amount of errors orginated in the source file. */
  u4     err;
  /* Amount of warnings orginated in the source file. */
  u4     wrn;
} Outcome;

/* Load the source file at the given name. */
Source      srcOf(char const* name);
/* Dispose the contents of the given source file. */
void        srcFree(Source* src);
/* Amount of characters in the given source file. */
ux          srcLen(Source src);
/* Character at the given index in the given source file. */
char        srcAt(Source src, ux i);
/* Pointer to the first character of the contents of the given source file if it
 * exits. */
char const* srcBgn(Source src);
/* Pointer to one after the last character of the contents of the given source
 * file. */
char const* srcEnd(Source src);

/* Clean outcome of the given source file. */
Outcome otcOf(Source src);
/* Report an error at the given part of the source file with the given formatted
 * message. */
void    otcErr(Outcome* otc, String part, char const* fmt, ...);
/* Report a warning at the given part of the source file with the given
 * formatted message. */
void    otcWrn(Outcome* otc, String part, char const* fmt, ...);
/* Report an information at the given part of the source file with the given
 * formatted message. */
void    otcInfo(Outcome otc, String part, char const* fmt, ...);
/* Report an error for the whole source file with the given formatted message.
 */
void    otcErrWhole(Outcome* otc, char const* fmt, ...);
/* Report a warning for the whole source file with the given formatted message.
 */
void    otcWrnWhole(Outcome* otc, char const* fmt, ...);
/* Report an information for the whole source file with the given formatted
 * message. */
void    otcInfoWhole(Outcome otc, char const* fmt, ...);
