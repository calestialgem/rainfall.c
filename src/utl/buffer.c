// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Make sure the given amount of space exists at the end of the given buffer.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Buffer* target, size_t reserved) {
  size_t capacity = target->bound - target->first;
  size_t bytes    = target->after - target->first;
  size_t space    = capacity - bytes;
  if (space >= reserved) return;

  size_t growth    = reserved - space;
  size_t minGrowth = capacity / 2;
  if (growth < minGrowth) growth = minGrowth;
  capacity += growth;

  target->first = allocateArray(target->first, capacity, char);
  target->after = target->first + bytes;
  target->bound = target->first + capacity;
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

void put(Buffer* target, char source) {
  reserve(target, 1);
  *target->after++ = source;
}

void append(Buffer* target, String source) {
  size_t written = characters(source);
  reserve(target, written);
  memmove(target->after, source.first, written * sizeof(char));
  target->after += written;
}

/* Amount of bytes to read from a stream at every step. */
#define CHUNK 1024

void read(Buffer* target, FILE* source) {
  for (size_t written = CHUNK; written == CHUNK;) {
    reserve(target, CHUNK);
    written = fread(target->after, sizeof(char), CHUNK, source);
    target->after += written;
  }
  expect(feof(source), "Could not read the stream!");
}
