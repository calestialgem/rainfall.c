// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Make sure the given amount of space exists at the end of the given buffer.
 * When necessary, grows by at least half of the current capacity. */
static void reserve(Buffer* buffer, ptrdiff_t amount) {
  ptrdiff_t capacity = buffer->bound - buffer->first;
  ptrdiff_t bytes    = buffer->after - buffer->first;
  ptrdiff_t space    = capacity - bytes;
  if (space >= amount) return;

  ptrdiff_t growth    = amount - space;
  ptrdiff_t minGrowth = capacity / 2;
  if (growth < minGrowth) growth = minGrowth;
  capacity += growth;

  buffer->first = allocateArray(buffer->first, capacity, char);
  buffer->after = buffer->first + bytes;
  buffer->bound = buffer->first + capacity;
}

Buffer emptyBuffer(void) {
  return (Buffer){.first = NULL, .after = NULL, .bound = NULL};
}

Buffer copyBuffer(Buffer buffer) {
  ptrdiff_t bytes = buffer.after - buffer.first;
  Buffer    copy  = emptyBuffer();
  reserve(&copy, bytes);
  memcpy(copy.first, buffer.first, bytes);
  copy.after += bytes;
  return copy;
}

void disposeBuffer(Buffer* buffer) {
  buffer->first = allocateArray(buffer->first, 0, char);
  buffer->after = buffer->first;
  buffer->bound = buffer->first;
}

ptrdiff_t bytes(Buffer buffer) { return buffer.after - buffer.first; }

void append(Buffer* buffer, char character) {
  reserve(buffer, 1);
  *buffer->after++ = character;
}

/* Amount of bytes to read from a stream at every step. */
#define CHUNK 1024

void read(Buffer* buffer, FILE* stream) {
  for (ptrdiff_t written = CHUNK; written == CHUNK;) {
    reserve(buffer, CHUNK);
    written = fread(buffer->after, sizeof(char), CHUNK, stream);
    buffer->after += written;
  }
  expect(feof(stream), "Could not read the stream!");
}
