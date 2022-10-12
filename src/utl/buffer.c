// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

Buffer createBuffer(size_t initialCapacity) {
  Buffer created = {.first = NULL, .after = NULL, .bound = NULL};
  reserveArray(&created, initialCapacity, char);
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
  reserveArray(target, 1, char);
  *target->after++ = appended;
}

void appendString(Buffer* target, String appended) {
  // Reserve necessary space and use `memmove` on them, because the string might
  // point into the target buffer, which would break `memcpy`.
  size_t written = countCharacters(appended);
  reserveArray(target, written, char);
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
    reserveArray(target, CHUNK, char);
    written = fread(target->after, sizeof(char), CHUNK, appended);
    target->after += written;
  }
  expect(feof(appended), "Could not read the stream!");
}
