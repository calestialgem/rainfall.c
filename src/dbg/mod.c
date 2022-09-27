// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "api.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* Whether the given range of characters matches the root source folder. */
bool static root(char const* begin, char const* end) {
  char const   FOLDER[] = "src";
  size_t const LENGTH   = strlen(FOLDER);

  for (size_t i = 0; i < LENGTH; i++, begin++)
    if (begin == end || *begin != FOLDER[i]) return false;

  // Matched if all the input is consumed.
  return begin == end;
}

char const* dbgPath(char const* fullPath) {
  // Last "src" is the source folder; remove the path upto that.
  for (char const *previous = NULL, *i = fullPath + strlen(fullPath) - 1;
       i >= fullPath; i--) {
    // If not positioned right after a folder, continue.
    if (*i != '\\' && *i != '/') continue;
    // If there is a previous position after a folder saved, check.
    if (previous && root(i + 1, previous)) return previous + 1;
    // Save this position if failed to match.
    previous = i;
  }

  return fullPath;
}
