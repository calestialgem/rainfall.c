// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Make sure the given amount of space exists at the end of the given buffer.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Buffer* expanded, size_t reserved) {
  size_t capacity = expanded->bound - expanded->first;
  size_t bytes    = expanded->after - expanded->first;
  size_t space    = capacity - bytes;
  if (space >= reserved) return;

  size_t growth    = reserved - space;
  size_t minGrowth = capacity / 2;
  if (growth < minGrowth) growth = minGrowth;
  capacity += growth;

  expanded->first = allocateArray(expanded->first, capacity, char);
  expanded->after = expanded->first + bytes;
  expanded->bound = expanded->first + capacity;
}

Buffer emptyBuffer(void) {
  return (Buffer){.first = NULL, .after = NULL, .bound = NULL};
}

Buffer copyBuffer(Buffer source) {
  size_t bytes  = source.after - source.first;
  Buffer result = emptyBuffer();
  reserve(&result, bytes);
  memcpy(result.first, source.first, bytes);
  result.after += bytes;
  return result;
}

void disposeBuffer(Buffer* target) {
  target->first = allocateArray(target->first, 0, char);
  target->after = target->first;
  target->bound = target->first;
}

size_t bytes(Buffer source) { return source.after - source.first; }

void put(Buffer* target, char putted) {
  reserve(target, 1);
  *target->after++ = putted;
}

void append(Buffer* target, String appended) {
  size_t written = characters(appended);
  reserve(target, written);
  memmove(target->after, appended.first, written * sizeof(char));
  target->after += written;
}

/* Amount of bytes to read from a stream at every step. */
#define CHUNK 1024

void read(Buffer* target, FILE* read) {
  for (size_t written = CHUNK; written == CHUNK;) {
    reserve(target, CHUNK);
    written = fread(target->after, sizeof(char), CHUNK, read);
    target->after += written;
  }
  expect(feof(read), "Could not read the stream!");
}
