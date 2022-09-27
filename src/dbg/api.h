// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Path to the Rainfall source file relative to the root source folder. Returns
 * the full path if it cannot find the root. */
char const* dbgPath(char const* fullPath);

/* Aborts if the given condition does not hold, after printing the given
 * message with the file and line information of the Rainfall source file. */
#define dbgExpect(condition, message)                                        \
  do {                                                                       \
    if (!(condition)) {                                                      \
      fprintf(                                                               \
        stderr, "%s:%u: debug: %s\n", dbgPath(__FILE__), __LINE__, message); \
      abort();                                                               \
    }                                                                        \
  } while (false)

/* Calls `dbgExpect` with a condition that always fails. */
#define dbgUnexpected(message) dbgExpect(false, message)
