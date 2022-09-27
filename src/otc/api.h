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

/* Load the source file at the given name. */
Source sourceOf(char const* name);
/* Dispose the contents of the given source file. */
void   sourceFree(Source* src);
/* Amount of characters in the given source file. */
ux     sourceLength(Source src);
/* Character at the given index in the given source file. */
char   sourceAt(Source src, ux i);
