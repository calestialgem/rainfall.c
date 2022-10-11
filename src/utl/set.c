// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbg/api.h"
#include "utl/api.h"

#include <stddef.h>

Set emptySet(void) { return (Set){.first = NULL, .after = NULL, .members = 0}; }

void disposeSet(Set* target) {
  target->first   = allocateArray(target->first, 0, String);
  target->after   = target->first;
  target->members = 0;
}

/* Lowest ratio of used capacity to total capacity thay is allowed. */
#define MIN_RATIO  0.5
/* Factor to scale the capacity in each growth. */
#define MULTIPLIER 16

void insertMember(Set* target, String inserted) {
  size_t capacity = target->after - target->first;

  // Grow if necessary.
  if (capacity == 0 || (double)target->members / capacity >= MIN_RATIO) {
    size_t newCapacity =
      capacity < MULTIPLIER ? MULTIPLIER : capacity * MULTIPLIER;

    Set new   = emptySet();
    new.first = allocateArray(new.first, newCapacity, String);
    new.after = new.first + newCapacity;

    for (String const* i = target->first; i < target->after; i++)
      if (characters(*i)) insertMember(&new, *i);

    disposeSet(target);
    *target = new;
  }

  size_t hash = hashcode(inserted);
  for (size_t i = 0; i < capacity; i++) {
    size_t index = (hash + i) % capacity;
    if (!characters(target->first[index])) {
      target->first[index] = inserted;
      target->members++;
      return;
    }
  }

  unexpected("Could not find an empty place in the set!");
}

String const* accessMember(Set source, String accessed) {
  size_t capacity = source.after - source.first;
  size_t hash     = hashcode(accessed);
  for (size_t i = 0; i < capacity; i++) {
    size_t index = (hash + i) % capacity;
    if (equalStrings(source.first[index], accessed))
      return source.first + index;
  }
  return NULL;
}
