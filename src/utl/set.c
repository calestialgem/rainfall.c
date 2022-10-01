// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stdbool.h>
#include <stdlib.h>

/* Amount of allocated strings in the given set. */
static ux setCap(Set const set) { return set.end - set.bgn; }

/* Whether the given should grow before adding a new string. */
static bool shouldGrow(Set const set) {
  f8 const MIN_RATIO = 0.5;
  ux const cap       = setCap(set);
  return cap == 0 || (f8)set.len / (f8)cap >= MIN_RATIO;
}

/* Multiply the capacity of the given set by 16. */
static void grow(Set* const set) {
  ux const MULTIPLIER = 16;
  ux const cap        = setCap(*set);
  Set new             = setOf(cap < MULTIPLIER ? MULTIPLIER : cap * MULTIPLIER);
  for (String const* i = set->bgn; i < set->end; i++)
    if (strLen(*i)) setPut(&new, *i);
  setFree(set);
  *set = new;
}

/* Hashcode of the given string. */
static ux hashcode(String const str) {
  ux const PRIME = 53;
  ux       hash  = 0;
  for (char const* i = str.bgn; i < str.end; i++) {
    hash *= PRIME;
    hash += *i;
  }
  return hash;
}

Set setOf(ux const cap) {
  Set res = {0};
  if (cap) {
    res.bgn = calloc(cap, sizeof(String));
    dbgExpect(res.bgn, "Could not allocate!");
    res.end = res.bgn + cap;
  }
  return res;
}

void setFree(Set* const set) {
  free(set->bgn);
  set->bgn = NULL;
  set->end = NULL;
  set->len = 0;
}

void setPut(Set* const set, String const str) {
  if (shouldGrow(*set)) grow(set);
  ux const cap  = setCap(*set);
  ux const hash = hashcode(str);
  for (ux i = 0; i < cap; i++) {
    ux const index = (hash + i) % cap;
    if (!strLen(set->bgn[index])) {
      set->bgn[index] = str;
      set->len++;
      return;
    }
  }
  dbgUnexpected("Could not find empty place in set!");
}

String const* setGet(Set const set, String const str) {
  ux const cap  = setCap(set);
  ux const hash = hashcode(str);
  for (ux i = 0; i < cap; i++) {
    ux const index = (hash + i) % cap;
    if (strEq(set.bgn[index], str)) return set.bgn + index;
  }
  return NULL;
}
