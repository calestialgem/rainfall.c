// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility/api.h"

#include <stddef.h>
#include <stdlib.h>

void* allocate(void* reallocatedBlock, size_t allocatedSize) {
  // Free the allocation if the size is zero.
  if (!allocatedSize) {
    free(reallocatedBlock);
    return NULL;
  }

  // Allocate and check before returning.
  reallocatedBlock = realloc(reallocatedBlock, allocatedSize);
  expect(reallocatedBlock, "Could not allocate!");
  return reallocatedBlock;
}

char const* trimRainfallSourcePath(char const* fullPath) {
  String      rootFolder       = viewTerminated("src");
  String      fullPathAsString = viewTerminated(fullPath);
  char const* previousPosition = NULL;

  // Last "src" is the source folder; remove the path upto that.
  for (char const* position = fullPathAsString.after - 1;
       position >= fullPathAsString.first; position--) {
    // If not positioned right after a folder, continue.
    if (*position != '\\' && *position != '/') continue;
    // If there is a previous position after a folder saved, check.
    if (previousPosition) {
      String folder = createString(position + 1, previousPosition);
      if (compareStringEquality(folder, rootFolder))
        return previousPosition + 1;
    }
    // Save this position if failed to match.
    previousPosition = position;
  }

  return fullPath;
}
