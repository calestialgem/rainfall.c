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
  Source      source;
  /* Pointer to the character. */
  char const* position;
  /* Line number the character is at. */
  int         line;
  /* Column number the character is at. */
  int         column;
} Location;

/* Location of a part of a source file. */
typedef struct {
  /* Location of the first character of the part. */
  Location first;
  /* Location of the last character of the part. */
  Location last;
} Portion;

/* Location of the character at the given position in the given source file. */
Location createLocation(Source containing, char const* coresponding);
/* Location of the start of the line of the given location. */
Location findLineStart(Location inLine);
/* Location of the end of the line of the given location. */
Location finLineEnd(Location inLine);
/* Location of the given section of the given source file. */
Portion  createPortion(Source containing, String coresponding);
/* Print and underline the given portion to the given stream. */
void     underlinePortion(Portion underlined, FILE* target);
