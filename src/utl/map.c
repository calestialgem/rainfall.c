// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>

Map emptyMap(void) { return (Map){.first = NULL, .after = NULL, .entries = 0}; }

void disposeMap(Map* map) {
  map->first   = allocateArray(map->first, 0, MapEntry);
  map->after   = map->first;
  map->entries = 0;
}

/* Lowest ratio of used capacity to total capacity thay is allowed. */
#define MIN_RATIO  0.5
/* Factor to scale the capacity in each growth. */
#define MULTIPLIER 16

void insertEntry(Map* map, String key, ptrdiff_t value) {
  ptrdiff_t capacity = map->after - map->first;

  // Grow if necessary.
  if (capacity == 0 || (double)map->entries / capacity >= MIN_RATIO) {
    ptrdiff_t newCapacity =
      capacity < MULTIPLIER ? MULTIPLIER : capacity * MULTIPLIER;

    Map new   = emptyMap();
    new.first = allocateArray(new.first, newCapacity, MapEntry);
    new.after = new.first + newCapacity;

    for (MapEntry const* i = map->first; i < map->after; i++)
      if (characters(i->key)) insertEntry(&new, i->key, i->value);

    disposeMap(map);
    *map = new;
  }

  ptrdiff_t hash = hashcode(key);
  for (ptrdiff_t i = 0; i < capacity; i++) {
    ptrdiff_t index = (hash + i) % capacity;
    if (!characters(map->first[index].key)) {
      map->first[index].key   = key;
      map->first[index].value = value;
      map->entries++;
      return;
    }
  }

  unexpected("Could not find an empty place in the map!");
}

MapEntry const* accessEntry(Map map, String key) {
  ptrdiff_t capacity = map.after - map.first;
  ptrdiff_t hash     = hashcode(key);
  for (ptrdiff_t i = 0; i < capacity; i++) {
    ptrdiff_t index = (hash + i) % capacity;
    if (equalStrings(map.first[index].key, key)) return map.first + index;
  }
  return NULL;
}

String const* accessKey(Map map, String key) {
  return &accessEntry(map, key)->key;
}

ptrdiff_t const* accessValue(Map map, String key) {
  return &accessEntry(map, key)->value;
}
