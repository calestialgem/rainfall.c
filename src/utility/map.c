// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility/api.h"

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

/* Insert the given entry to the given map. */
static void insertEntryWhole(Map* target, MapEntry inserted) {
  insertBucket(
    target, inserted, entries, insertEntryWhole, createMap, disposeMap, Map,
    MapEntry);
}

void insertEntry(Map* target, String insertedKey, size_t insertedValue) {
  MapEntry inserted = {.key = insertedKey, .value = insertedValue};
  insertEntryWhole(target, inserted);
}

MapEntry const* accessEntry(Map source, String accessedKey) {
  accessBucket(source, accessedKey, MapEntry);
}

String const* accessKey(Map source, String accessedKey) {
  return &accessEntry(source, accessedKey)->key;
}

size_t const* accessValue(Map source, String accessedKey) {
  return &accessEntry(source, accessedKey)->value;
}
