// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "test.c"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Allocate a memory block with the given amount of bytes. If a previous memory
 * block is given tries to change its size. If the bytes are zero, frees the
 * previous block. */
void* Allocate(void* relocated, size_t new) {
  // Free the allocation if the size is zero.
  if (!new) {
    free(relocated);
    return NULL;
  }

  // Allocate and check before returning.
  relocated = realloc(relocated, new);
  if (!relocated) {
    fprintf(stderr, "Could not allocate!\n");
    abort();
    return NULL;
  }
  return relocated;
}

/* Test allocation freeing a block. */
bool TestFreeingWithAllocation(void) {
  size_t size  = 1;
  void*  block = malloc(size);
  memset(block, 0, size);
  return !Allocate(block, 0);
}

/* Test allocation without a block. */
bool TestAllocationWithoutABlock(void) {
  size_t size  = 1;
  void*  block = Allocate(NULL, size);
  memset(block, 0, size);
  free(block);
  return true;
}

/* Test reallocation using allocation. */
bool TestReallocationWithAllocate(void) {
  size_t size  = 1;
  void*  block = malloc(size);
  memset(block, 0, size);
  void* moved = Allocate(block, size * 2);
  memset(moved, 0, size * 2);
  free(moved);
  return true;
}

/* Call `allocate` with amount of bytes enough to have the given amount of
 * elements of the given type. */
#define AllocateArray(                                           \
  reallocatedArray, allocatedElementCount, AllocatedElementType) \
  (AllocatedElementType*)Allocate(                               \
    reallocatedArray, (allocatedElementCount) * sizeof(AllocatedElementType))

/* Test array allocating. */
bool TestArrayAllocation(void) {
  int* array = AllocateArray(NULL, 1, int);
  array[0]   = 1;
  array      = AllocateArray(array, 2, int);
  array[1]   = 2;
  array      = AllocateArray(array, 0, int);
  return true;
}

/* Make sure the given amount of space exists at the end of the given dynamic
 * array. When necessary, grows by at least half of the current capacity. */
#define ReserveSpace(targetArray, reservedSize, ReservedElementType)        \
  do {                                                                      \
    /* Find whether growth is necessary. */                                 \
    size_t capacity = (targetArray)->bound - (targetArray)->first;          \
    size_t count    = (targetArray)->after - (targetArray)->first;          \
    size_t space    = capacity - count;                                     \
    if (space < (reservedSize)) {                                           \
      /* Grow the dynamic array. */                                         \
      capacity += max((reservedSize)-space, capacity / 2);                  \
      (targetArray)->first =                                                \
        AllocateArray((targetArray)->first, capacity, ReservedElementType); \
      (targetArray)->after = (targetArray)->first + count;                  \
      (targetArray)->bound = (targetArray)->first + capacity;               \
    }                                                                       \
  } while (false)

/* Test opening new space. */
bool TestOpeningSpace(void) {
  struct {
    int* first;
    int* after;
    int* bound;
  } array = {NULL, NULL, NULL};

  ReserveSpace(&array, 1, int);
  *array.after++ = 1;
  array.first    = AllocateArray(array.first, 0, int);
  return true;
}

/* Test ensuring existing space. */
bool TestEnsuringSpace(void) {
  int* block = AllocateArray(NULL, 1, int);

  struct {
    int* first;
    int* after;
    int* bound;
  } array = {block, block, block + 1};

  ReserveSpace(&array, 1, int);
  AllocateArray(array.first, 0, int);
  return array.first == block;
}

/* Register the tests in the allocation module. */
void RegisterAllocationModuleTests(void) {
  // Test `Allocate`.
  RegisterUnitTest(TestFreeingWithAllocation);
  RegisterUnitTest(TestAllocationWithoutABlock);
  RegisterUnitTest(TestReallocationWithAllocate);

  // Test `AllocateArray`.
  RegisterUnitTest(TestArrayAllocation);

  // Test `ReserveSpace`.
  RegisterUnitTest(TestOpeningSpace);
  RegisterUnitTest(TestEnsuringSpace);
}
