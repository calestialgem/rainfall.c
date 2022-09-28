// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "otc/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdio.h>

/* Location of a character in a source file. */
typedef struct {
  /* Source file the character is in. */
  Source      src;
  /* Pointer to the character. */
  char const* pos;
  /* Line number the character is at. */
  u4          ln;
  /* Column number the character is at. */
  u4          cl;
} Location;

/* Location of a part of a source file. */
typedef struct {
  /* Location of the first character of the part. */
  Location bgn;
  /* Location of the last character of the part. */
  Location end;
} Portion;

/* Location of the character at the given position in the given source file. */
Location locOf(Source src, char const* pos);
/* Location of the start of the line of the given location. */
Location locStart(Location loc);
/* Location of the end of the line of the given location. */
Location locEnd(Location loc);

/* Location of the given part of the given source file. */
Portion porOf(Source src, String part);
/* Print and underline the given portion to the given stream. */
void    porUnderline(Portion por, FILE* stream);
