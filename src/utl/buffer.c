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
  // Find whether growth is necessary.
  size_t capacity = target->bound - target->first;
  size_t count    = countBytes(*target);
  size_t space    = capacity - count;
  if (space >= reserved) return;

  // Calculate necessary growth amount.
  size_t growth    = reserved - space;
  size_t minGrowth = capacity / 2;
  if (growth < minGrowth) growth = minGrowth;
  capacity += growth;

  // Grow the underlying dynamic array.
  target->first = allocateArray(target->first, capacity, char);
  target->after = target->first + count;
  target->bound = target->first + capacity;
}

Buffer createBuffer(size_t initialCapacity) {
  Buffer created = {.first = NULL, .after = NULL, .bound = NULL};
  reserve(&created, initialCapacity);
  return created;
}

Buffer copyBuffer(Buffer copied) {
  // Create a new buffer with necessary capacity and use `memcpy` on them. They
  // are guaranteed to be different buffers; then, `memcpy` should be selected
  // over `memmove` because it is faster.
  size_t count = countBytes(copied);
  Buffer copy  = createBuffer(count);
  memcpy(copy.first, copied.first, count);
  copy.after += count;
  return copy;
}

void disposeBuffer(Buffer* disposed) {
  disposed->first = allocateArray(disposed->first, 0, char);
  disposed->after = disposed->first;
  disposed->bound = disposed->first;
}

size_t countBytes(Buffer counted) { return counted.after - counted.first; }

void appendCharacter(Buffer* target, char appended) {
  reserve(target, 1);
  *target->after++ = appended;
}

void appendString(Buffer* target, String appended) {
  // Reserve necessary space and use `memmove` on them, because the string might
  // point into the target buffer, which would break `memcpy`.
  size_t written = countCharacters(appended);
  reserve(target, written);
  memmove(target->after, appended.first, written * sizeof(char));
  target->after += written;
}

/* Amount of bytes to read from a stream at every step. */
#define CHUNK 1024

void read(Buffer* target, FILE* appended) {
  // Reserve space and read from the stream in chunks until the written amount
  // is less than a chunk (not equal to chunk as it cannot be bigger,) which
  // indicates the stream is completely read.
  for (size_t written = CHUNK; written == CHUNK;) {
    reserve(target, CHUNK);
    written = fread(target->after, sizeof(char), CHUNK, appended);
    target->after += written;
  }
  expect(feof(appended), "Could not read the stream!");
}
