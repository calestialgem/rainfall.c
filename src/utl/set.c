// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>

Set createSet(size_t initialCapacity) {
  Set created   = {.first = NULL, .after = NULL, .members = 0};
  created.first = allocateArray(created.first, initialCapacity, String);
  created.after = created.first + initialCapacity;
  return created;
}

void disposeSet(Set* disposed) {
  disposed->first   = allocateArray(disposed->first, 0, String);
  disposed->after   = disposed->first;
  disposed->members = 0;
}

/* Lowest ratio of used capacity to total capacity thay is allowed. */
#define MIN_USED_CAPACITY_RATIO 0.5
/* Factor to scale the capacity in each growth. */
#define GROWTH_MULTIPLIER       16

void insertMember(Set* target, String inserted) {
  size_t capacity = target->after - target->first;

  // Grow if necessary.
  if (capacity == 0 || target->members >= capacity * MIN_USED_CAPACITY_RATIO) {
    // If capacity is zero, make it finite.
    if (!capacity) capacity = 1;
    capacity *= GROWTH_MULTIPLIER;

    // Rehash by creating a new set and inserting all buckets that have a finite
    // member again.
    Set new = createSet(capacity);
    for (String const* bucket = target->first; bucket < target->after; bucket++)
      if (countCharacters(*bucket)) insertMember(&new, *bucket);

    // Swap the new set with the old one.
    disposeSet(target);
    *target = new;
  }

  // Start at the bucket at pointed by the hashcode, end at the bucket one
  // before that by wrapping. The new member should be inserted at the first
  // empty bucket found.
  size_t hash = calculateHashcode(inserted);
  for (size_t index = 0; index < capacity; index++) {
    String* bucket = target->first + (hash + index) % capacity;
    if (!countCharacters(*bucket)) {
      *bucket = inserted;
      target->members++;
      return;
    }
  }

  unexpected("Could not find an empty place in the set!");
}

String const* accessMember(Set source, String accessed) {
  // Start at the bucket at pointed by the hashcode, end at the bucket one
  // before that by wrapping. The accessed member should be at the first
  // bucket with an equal member.
  size_t capacity = source.after - source.first;
  size_t hash     = calculateHashcode(accessed);
  for (size_t index = 0; index < capacity; index++) {
    String const* bucket = source.first + (hash + index) % capacity;
    if (compareStringEquality(*bucket, accessed)) return bucket;
  }

  // If none was equal, the accessed member does not exist.
  return NULL;
}
