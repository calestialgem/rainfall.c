// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Allocate a memory block with the given amount of bytes. If a previous memory
 * block is given tries to change its size. If the bytes are zero, frees the
 * previous block. */
void*       allocate(void* reallocatedBlock, size_t allocatedSize);
/* Path to the Rainfall source file at the given path relative to the root
 * source folder. Returns the full path if it cannot find the root. */
char const* trimRainfallSourcePath(char const* fullPath);

/* Call `allocate` with amount of bytes enough to have the given amount of
 * elements of the given type. */
#define allocateArray(                                           \
  reallocatedArray, allocatedElementCount, AllocatedElementType) \
  (AllocatedElementType*)allocate(                               \
    reallocatedArray, (allocatedElementCount) * sizeof(AllocatedElementType))

/* Aborts if the given condition does not hold, after printing the given
 * message with the file and line information of the Rainfall source file. */
#define expect(expectedCondition, unexpectedMessage)                    \
  do {                                                                  \
    if (!(expectedCondition)) {                                         \
      fprintf(                                                          \
        stderr, "%s:%u: debug: %s\n", trimRainfallSourcePath(__FILE__), \
        __LINE__, unexpectedMessage);                                   \
      abort();                                                          \
    }                                                                   \
  } while (false)

/* Calls `expect` with a condition that always fails. */
#define unexpected(unexpectedMessage) expect(false, unexpectedMessage)
