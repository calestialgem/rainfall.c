// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "api.h"
#include "utl/api.h"

char const* dbgPath(char const* const fullPath) {
  String const ROOT = stringOf("src");
  String const path = stringOf(fullPath);
  char const*  pre  = NULL;

  // Last "src" is the source folder; remove the path upto that.
  for (char const* i = path.end - 1; i >= path.bgn; i--) {
    // If not positioned right after a folder, continue.
    if (*i != '\\' && *i != '/') continue;
    // If there is a previous position after a folder saved, check.
    if (pre) {
      String const folder = {.bgn = i + 1, .end = pre};
      if (stringEqual(folder, ROOT)) return pre + 1;
    }
    // Save this position if failed to match.
    pre = i;
  }

  return fullPath;
}
