// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "api.h"
#include "dbg/api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Makes sure the given amount of space exists at the end of the given buffer.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Buffer* const bfr, ux const amount) {
  ux const cap   = bufferCapacity(*bfr);
  ux const len   = bufferLength(*bfr);
  ux const space = cap - len;
  if (space >= amount) return;

  ux const    growth    = amount - space;
  ux const    minGrowth = cap / 2;
  ux const    newCap    = cap + (growth < minGrowth ? minGrowth : growth);
  char* const mem       = realloc(bfr->bgn, newCap);
  dbgExpect(mem, "Could not reallocate!");

  bfr->bgn = mem;
  bfr->end = mem + len;
  bfr->all = mem + newCap;
}

Buffer bufferOf(ux const cap) {
  Buffer res = {.bgn = NULL, .end = NULL, .all = NULL};
  if (cap) reserve(&res, cap);
  return res;
}

void bufferFree(Buffer* const bfr) {
  free(bfr->bgn);
  bfr->bgn = NULL;
  bfr->end = NULL;
  bfr->all = NULL;
}

ux bufferLength(Buffer const bfr) { return bfr.end - bfr.bgn; }

ux bufferCapacity(Buffer const bfr) { return bfr.all - bfr.bgn; }

char bufferAt(Buffer const bfr, ux const i) { return bfr.bgn[i]; }

String bufferView(Buffer const bfr) {
  return (String){.bgn = bfr.bgn, .end = bfr.end};
}

void bufferAppend(Buffer* const bfr, String const str) {
  ux const len = stringLength(str);
  reserve(bfr, len);
  memmove(bfr->end, str.bgn, len);
  bfr->end += len;
}

void bufferPut(Buffer* const bfr, char const c) {
  reserve(bfr, 1);
  *bfr->end++ = c;
}

void bufferRead(Buffer* const bfr, FILE* const stream) {
  ux const CHUNK = 1024;
  for (ux written = CHUNK; written == CHUNK; bfr->end += written) {
    reserve(bfr, CHUNK);
    written = fread(bfr->end, sizeof(char), CHUNK, stream);
  }
  dbgExpect(feof(stream), "Error reading stream!");
}

void bufferWrite(Buffer const bfr, FILE* const stream) {
  fwrite(bfr.bgn, sizeof(char), bufferLength(bfr), stream);
}
