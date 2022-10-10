// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>

Set emptySet(void) { return (Set){.first = NULL, .after = NULL, .members = 0}; }

void disposeSet(Set* set) {
  set->first   = allocateArray(set->first, 0, String);
  set->after   = set->first;
  set->members = 0;
}

/* Lowest ratio of used capacity to total capacity thay is allowed. */
#define MIN_RATIO  0.5
/* Factor to scale the capacity in each growth. */
#define MULTIPLIER 16

void insertMember(Set* set, String member) {
  ptrdiff_t capacity = set->after - set->first;

  // Grow if necessary.
  if (capacity == 0 || (double)set->members / capacity >= MIN_RATIO) {
    ptrdiff_t newCapacity =
      capacity < MULTIPLIER ? MULTIPLIER : capacity * MULTIPLIER;

    Set new   = emptySet();
    new.first = allocateArray(new.first, newCapacity, String);
    new.after = new.first + newCapacity;

    for (String const* i = set->first; i < set->after; i++)
      if (characters(*i)) insertMember(&new, *i);

    disposeSet(set);
    *set = new;
  }

  ptrdiff_t hash = hashcode(member);
  for (ptrdiff_t i = 0; i < capacity; i++) {
    ptrdiff_t index = (hash + i) % capacity;
    if (!characters(set->first[index])) {
      set->first[index] = member;
      set->members++;
      return;
    }
  }

  unexpected("Could not find an empty place in the set!");
}

String const* accessMember(Set set, String member) {
  ptrdiff_t capacity = set.after - set.first;
  ptrdiff_t hash     = hashcode(member);
  for (ptrdiff_t i = 0; i < capacity; i++) {
    ptrdiff_t index = (hash + i) % capacity;
    if (equalStrings(set.first[index], member)) return set.first + index;
  }
  return NULL;
}
