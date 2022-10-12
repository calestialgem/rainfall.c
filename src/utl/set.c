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

void insertMember(Set* target, String inserted) {
  insertBucket(
    target, inserted, members, insertMember, createSet, disposeSet, Set,
    String);
}

String const* accessMember(Set source, String accessed) {
  accessBucket(source, accessed, String);
}
