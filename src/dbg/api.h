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

/* Make sure the given amount of space exists at the end of the given dynamic
 * array. When necessary, grows by at least half of the current capacity. */
#define reserveArray(targetArray, reservedSize, ReservedElementType)      \
  /* Find whether growth is necessary. */                                 \
  size_t capacity = (targetArray)->bound - (targetArray)->first;          \
  size_t count    = (targetArray)->after - (targetArray)->first;          \
  size_t space    = capacity - count;                                     \
  if (space < (reservedSize)) {                                           \
    /* Grow the dynamic array. */                                         \
    capacity += max((reservedSize)-space, capacity / 2);                  \
    (targetArray)->first =                                                \
      allocateArray((targetArray)->first, capacity, ReservedElementType); \
    (targetArray)->after = (targetArray)->first + count;                  \
    (targetArray)->bound = (targetArray)->first + capacity;               \
  }

/* Lowest ratio of used capacity to total capacity thay is allowed on bucket
 * arrays. */
#define MIN_USED_CAPACITY_RATIO 0.5
/* Factor to scale the capacity of bucket arrays in each growth. */
#define GROWTH_MULTIPLIER       16

/* Insert the given bucket to the one of the given string. Starts at the bucket
 * that is pointed by the hashcode of the string, and ends at the bucket one
 * before that by wrapping at the array boundary. Bucket or its first member
 * should be a string. */
#define insertBucket(                                                          \
  targetArray, insertedBucket, bucketCount, inserterFunction, creatorFunction, \
  disposerFunction, TargetArrayType, AccessedBucketType)                       \
  size_t capacity = (targetArray)->after - (targetArray)->first;               \
  /* Grow if necessary. */                                                     \
  if (                                                                         \
    capacity == 0 ||                                                           \
    (targetArray)->bucketCount >= capacity * MIN_USED_CAPACITY_RATIO) {        \
    /* If capacity is zero, make it finite. */                                 \
    if (!capacity) capacity = 1;                                               \
    capacity *= GROWTH_MULTIPLIER;                                             \
    /* Rehash by inserting all buckets that have a finite member again */      \
    TargetArrayType new = creatorFunction(capacity);                           \
    for (AccessedBucketType const* bucket = (targetArray)->first;              \
         bucket < (targetArray)->after; bucket++)                              \
      if (countCharacters(*(String const*)bucket))                             \
        inserterFunction(&new, *bucket);                                       \
    /* Swap the new array with the old one. */                                 \
    disposerFunction(target);                                                  \
    *target = new;                                                             \
  }                                                                            \
  size_t hash = calculateHashcode(*(String const*)&(insertedBucket));          \
  for (size_t index = 0; index < capacity; index++) {                          \
    AccessedBucketType* bucket =                                               \
      (targetArray)->first + (hash + index) % capacity;                        \
    if (!countCharacters(*(String const*)bucket)) {                            \
      *bucket = insertedBucket;                                                \
      (targetArray)->bucketCount++;                                            \
      return;                                                                  \
    }                                                                          \
  }                                                                            \
  unexpected("Could not find an empty bucket!");

/* Access the bucket of the given string at the given source array of the given
 * bucket type. Starts at the bucket that is pointed by the hashcode of the
 * string, and ends at the bucket one before that by wrapping at the array
 * boundary. Returns the bucket that has a string that equals to the given one.
 * Returns null if cannot find any equal bucket. Bucket or its first member
 * should be a string. */
#define accessBucket(sourceArray, accessedString, AccessedBucketType)  \
  size_t capacity = (sourceArray).after - (sourceArray).first;         \
  size_t hash     = calculateHashcode(accessedString);                 \
  for (size_t index = 0; index < capacity; index++) {                  \
    AccessedBucketType const* bucket =                                 \
      (sourceArray).first + (hash + index) % capacity;                 \
    if (compareStringEquality(*(String const*)bucket, accessedString)) \
      return bucket;                                                   \
  }                                                                    \
  /* If none was equal, the accessed bucket does not exist. */         \
  return NULL

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
