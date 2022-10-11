// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>
#include <stdlib.h>

void* allocate(void* target, ptrdiff_t bytes) {
  if (!bytes) {
    free(target);
    return NULL;
  }
  target = realloc(target, bytes);
  expect(target, "Could not allocate!");
  return target;
}

char const* trimPath(char const* fullPath) {
  String      root     = nullTerminated("src");
  String      asString = nullTerminated(fullPath);
  char const* previous = NULL;

  // Last "src" is the source folder; remove the path upto that.
  for (char const* position = asString.first; position < asString.after;
       position++) {
    // If not positioned right after a folder, continue.
    if (*position != '\\' && *position != '/') continue;
    // If there is a previous position after a folder saved, check.
    if (previous) {
      String folder = stringOf(position + 1, previous);
      if (equalStrings(folder, root)) return previous + 1;
    }
    // Save this position if failed to match.
    previous = position;
  }

  return fullPath;
}
