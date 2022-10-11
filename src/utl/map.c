// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>

Map createMap(size_t initialCapacity) {
  Map new   = {.first = NULL, .after = NULL, .entries = 0};
  new.first = allocateArray(new.first, initialCapacity, MapEntry);
  new.after = new.first + initialCapacity;
  return new;
}

void disposeMap(Map* disposed) {
  disposed->first   = allocateArray(disposed->first, 0, MapEntry);
  disposed->after   = disposed->first;
  disposed->entries = 0;
}

/* Lowest ratio of used capacity to total capacity thay is allowed. */
#define MIN_USED_CAPACITY_RATIO 0.5
/* Factor to scale the capacity in each growth. */
#define GROWTH_MULTIPLIER       16

void insertEntry(Map* target, String insertedKey, size_t insertedValue) {
  size_t capacity = target->after - target->first;

  // Grow if necessary.
  if (capacity == 0 || target->entries >= capacity * MIN_USED_CAPACITY_RATIO) {
    // If capacity is zero, make it finite.
    if (!capacity) capacity = 1;
    capacity *= GROWTH_MULTIPLIER;

    // Rehash by creating a new map and inserting all buckets that have a finite
    // key again.
    Map new = createMap(capacity);
    for (MapEntry const* bucket = target->first; bucket < target->after;
         bucket++)
      if (countCharacters(bucket->key))
        insertEntry(&new, bucket->key, bucket->value);

    // Swap the new map with the old one.
    disposeMap(target);
    *target = new;
  }

  // Start at the bucket at pointed by the hashcode, end at the bucket one
  // before that by wrapping. The new entry should be inserted at the first
  // empty bucket found.
  size_t hash = calculateHashcode(insertedKey);
  for (size_t index = 0; index < capacity; index++) {
    MapEntry* bucket = target->first + (hash + index) % capacity;
    if (!countCharacters(bucket->key)) {
      bucket->key   = insertedKey;
      bucket->value = insertedValue;
      target->entries++;
      return;
    }
  }

  unexpected("Could not find an empty place in the map!");
}

MapEntry const* accessEntry(Map source, String accessedKey) {
  // Start at the bucket at pointed by the hashcode, end at the bucket one
  // before that by wrapping. The accessed entry should be at the first
  // bucket with an equal key.
  size_t capacity = source.after - source.first;
  size_t hash     = calculateHashcode(accessedKey);
  for (size_t index = 0; index < capacity; index++) {
    MapEntry const* bucket = source.first + (hash + index) % capacity;
    if (compareStringEquality(bucket->key, accessedKey)) return bucket;
  }

  // If none was equal, the accesed entry does not exist.
  return NULL;
}

String const* accessKey(Map source, String accessedKey) {
  return &accessEntry(source, accessedKey)->key;
}

size_t const* accessValue(Map source, String accessedKey) {
  return &accessEntry(source, accessedKey)->value;
}
