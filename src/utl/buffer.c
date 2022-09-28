// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Makes sure the given amount of space exists at the end of the given buffer.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Buffer* const bfr, ux const amount) {
  ux const cap   = bfrCap(*bfr);
  ux const len   = bfrLen(*bfr);
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

Buffer bfrOf(ux const cap) {
  Buffer res = {0};
  if (cap) reserve(&res, cap);
  return res;
}

void bfrFree(Buffer* const bfr) {
  free(bfr->bgn);
  bfr->bgn = NULL;
  bfr->end = NULL;
  bfr->all = NULL;
}

ux bfrLen(Buffer const bfr) { return bfr.end - bfr.bgn; }

ux bfrCap(Buffer const bfr) { return bfr.all - bfr.bgn; }

char bfrAt(Buffer const bfr, ux const i) { return bfr.bgn[i]; }

String bfrView(Buffer const bfr) {
  return (String){.bgn = bfr.bgn, .end = bfr.end};
}

void bfrAppend(Buffer* const bfr, String const str) {
  ux const len = strLen(str);
  reserve(bfr, len);
  memmove(bfr->end, str.bgn, len);
  bfr->end += len;
}

void bfrPut(Buffer* const bfr, char const c) {
  reserve(bfr, 1);
  *bfr->end++ = c;
}

void bfrRead(Buffer* const bfr, FILE* const stream) {
  ux const CHUNK = 1024;
  for (ux written = CHUNK; written == CHUNK; bfr->end += written) {
    reserve(bfr, CHUNK);
    written = fread(bfr->end, sizeof(char), CHUNK, stream);
  }
  dbgExpect(feof(stream), "Error reading stream!");
}

void bfrWrite(Buffer const bfr, FILE* const stream) {
  fwrite(bfr.bgn, sizeof(char), bfrLen(bfr), stream);
}
