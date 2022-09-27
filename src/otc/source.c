// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "api.h"
#include "dbg/api.h"
#include "utl/api.h"

#include <stdio.h>

Source sourceOf(char const* const name) {
  // Join the name with the extension.
  String const EXTENSION = stringOf("tr");
  String const str       = stringOf(name);
  Buffer       path      = bufferOf(0);
  bufferAppend(&path, str);
  bufferPut(&path, '.');
  bufferAppend(&path, EXTENSION);
  bufferPut(&path, 0);

  FILE* const stream = fopen(path.bgn, "r");
  bufferFree(&path);
  dbgExpect(stream, "Could not open file!");

  Buffer con = bufferOf(0);
  bufferRead(&con, stream);
  return (Source){.name = name, .con = con};
}

void sourceFree(Source* const src) { bufferFree(&src->con); }

ux sourceLength(Source const src) { return bufferLength(src.con); }

char sourceAt(Source const src, ux const i) { return bufferAt(src.con, i); }
