// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdlib.h>

/* Amount of allocated pairs in the given map. */
static iptr mapCap(Map const map) { return map.end - map.bgn; }

/* Whether the given map should grow before adding a new pair. */
static bool shouldGrow(Map const map) {
  double const MIN_RATIO = 0.5;
  iptr const   cap       = mapCap(map);
  return cap == 0 || (double)map.len / (double)cap >= MIN_RATIO;
}

/* Multiply the capacity of the given map by 16. */
static void grow(Map* const map) {
  iptr const MULTIPLIER = 16;
  iptr const cap        = mapCap(*map);
  Map new = mapOf(cap < MULTIPLIER ? MULTIPLIER : cap * MULTIPLIER);
  for (MapEntry const* i = map->bgn; i < map->end; i++)
    if (strLen(i->key)) mapPut(&new, i->key, i->val);
  mapFree(map);
  *map = new;
}

Map mapOf(iptr const cap) {
  Map res = {0};
  if (cap) {
    res.bgn = calloc(cap, sizeof(MapEntry));
    dbgExpect(res.bgn, "Could not allocate!");
    res.end = res.bgn + cap;
  }
  return res;
}

void mapFree(Map* const map) {
  free(map->bgn);
  map->bgn = NULL;
  map->end = NULL;
  map->len = 0;
}

void mapPut(Map* const map, String const key, iptr const val) {
  if (shouldGrow(*map)) grow(map);
  iptr const cap  = mapCap(*map);
  iptr const hash = strHash(key);
  for (iptr i = 0; i < cap; i++) {
    iptr const index = (hash + i) % cap;
    if (!strLen(map->bgn[index].key)) {
      map->bgn[index].key = key;
      map->bgn[index].val = val;
      map->len++;
      return;
    }
  }
  dbgUnexpected("Could not find empty place in map!");
}

MapEntry const* mapGet(Map const map, String const key) {
  iptr const cap  = mapCap(map);
  iptr const hash = strHash(key);
  for (iptr i = 0; i < cap; i++) {
    iptr const index = (hash + i) % cap;
    if (strEq(map.bgn[index].key, key)) return map.bgn + index;
  }
  return NULL;
}

String const* mapGetKey(Map const map, String const key) {
  return &mapGet(map, key)->key;
}

iptr const* mapGetVal(Map const map, String const key) {
  return &mapGet(map, key)->val;
}
