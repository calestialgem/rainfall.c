// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>

Map emptyMap(void) { return (Map){.first = NULL, .after = NULL, .entries = 0}; }

void disposeMap(Map* target) {
  target->first   = allocateArray(target->first, 0, MapEntry);
  target->after   = target->first;
  target->entries = 0;
}

/* Lowest ratio of used capacity to total capacity thay is allowed. */
#define MIN_RATIO  0.5
/* Factor to scale the capacity in each growth. */
#define MULTIPLIER 16

void insertEntry(Map* target, String insertedKey, size_t insertedValue) {
  size_t capacity = target->after - target->first;

  // Grow if necessary.
  if (capacity == 0 || (double)target->entries / capacity >= MIN_RATIO) {
    size_t newCapacity =
      capacity < MULTIPLIER ? MULTIPLIER : capacity * MULTIPLIER;

    Map new   = emptyMap();
    new.first = allocateArray(new.first, newCapacity, MapEntry);
    new.after = new.first + newCapacity;

    for (MapEntry const* i = target->first; i < target->after; i++)
      if (characters(i->key)) insertEntry(&new, i->key, i->value);

    disposeMap(target);
    *target = new;
  }

  size_t hash = hashcode(insertedKey);
  for (size_t i = 0; i < capacity; i++) {
    size_t index = (hash + i) % capacity;
    if (!characters(target->first[index].key)) {
      target->first[index].key   = insertedKey;
      target->first[index].value = insertedValue;
      target->entries++;
      return;
    }
  }

  unexpected("Could not find an empty place in the map!");
}

MapEntry const* accessEntry(Map source, String accessedKey) {
  size_t capacity = source.after - source.first;
  size_t hash     = hashcode(accessedKey);
  for (size_t i = 0; i < capacity; i++) {
    size_t index = (hash + i) % capacity;
    if (equalStrings(source.first[index].key, accessedKey))
      return source.first + index;
  }
  return NULL;
}

String const* accessKey(Map source, String accessedKey) {
  return &accessEntry(source, accessedKey)->key;
}

size_t const* accessValue(Map source, String accessedKey) {
  return &accessEntry(source, accessedKey)->value;
}
