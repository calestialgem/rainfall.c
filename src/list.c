// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "list.c"

#include "allocation.c"
#include "test.c"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/* Define a struct that is a dynamic list of the given element type. */
#define List(ElementType)                                  \
  struct ElementType##List {                               \
    /* Pointer to the first element if it exists. */       \
    ElementType* first;                                    \
    /* Pointer to one after the last element. */           \
    ElementType* after;                                    \
    /* Pointer to one after the last allocated element. */ \
    ElementType* bound;                                    \
  }

/* Test defining a list. */
bool TestDefiningList(void) {
  List(int) list = {.first = NULL, .after = NULL, .bound = NULL};
  int value      = 0;
  list.first     = &value;
  list.after     = &value;
  list.bound     = &value;
  return true;
}

/* List of the given type without any elements. */
#define EmptyList(ElementType) \
  ((struct ElementType##List){.first = NULL, .after = NULL, .bound = NULL})

/* Test defining an empty list. */
bool TestDefiningEmptyList(void) {
  List(int) list = EmptyList(int);
  return list.first == NULL && list.after == NULL && list.bound == NULL;
}

/* Free the memory resources of the given list. */
#define DisposeList(disposedList)                                              \
  do {                                                                         \
    free((disposedList).first);                                                \
    (disposedList).first = (disposedList).after = (disposedList).bound = NULL; \
  } while (false)

/* Test disposing a list. */
bool TestDisposingList(void) {
  int* block     = malloc(sizeof(int));
  List(int) list = {.first = block, .after = block, .bound = block + 1};
  DisposeList(list);
  return list.first == NULL;
}

/* Minimum ratio of growth to capacity for a list. */
#define LIST_MIN_GROWTH_FACTOR 0.5

/* Make sure the given amount of space exists at the end of the given list. When
 * necessary, grows by at least half of the current capacity. */
#define ReserveSpace(targetList, reservedSize, ElementType)          \
  do {                                                               \
    /* Find whether growth is necessary. */                          \
    size_t capacity = (targetList).bound - (targetList).first;       \
    size_t count    = (targetList).after - (targetList).first;       \
    size_t space    = capacity - count;                              \
    if (space < (reservedSize)) {                                    \
      /* Grow the dynamic array. */                                  \
      size_t growth    = (reservedSize)-space;                       \
      size_t minGrowth = capacity * LIST_MIN_GROWTH_FACTOR;          \
      capacity += max(growth, minGrowth);                            \
      (targetList).first =                                           \
        realloc((targetList).first, capacity * sizeof(ElementType)); \
      if ((targetList).first == NULL) {                              \
        fprintf(                                                     \
          stderr,                                                    \
          "%s:%u: error: Could not grow " #ElementType               \
          "List from %zu to %zu!\n",                                 \
          __FILE__, __LINE__, count + space, capacity);              \
        abort();                                                     \
      }                                                              \
      (targetList).after = (targetList).first + count;               \
      (targetList).bound = (targetList).first + capacity;            \
    }                                                                \
  } while (false)

/* Test opening new space. */
bool TestOpeningSpace(void) {
  List(int) array = {NULL, NULL, NULL};
  ReserveSpace(array, 1, int);
  *array.after++ = 1;
  DisposeList(array);
  return true;
}

/* Test ensuring existing space. */
bool TestEnsuringSpace(void) {
  int value       = 0;
  List(int) array = {&value, &value, &value + 1};
  ReserveSpace(array, 1, int);
  return array.first == &value;
}

/* Push the given element to the end of the given list. */
#define PushElement(targetList, pushedElement, ElementType) \
  do {                                                      \
    ReserveSpace(targetList, 1, ElementType);               \
    *(targetList).after++ = pushedElement;                  \
  } while (false)

/* Test pushing an element. */
bool TestPushingElement(void) {
  List(int) list = EmptyList(int);
  PushElement(list, 1, int);
  bool result = list.first[0] == 1;
  DisposeList(list);
  return result;
}

/* Pop the last element from the given list. */
#define PopElement(targetList) \
  do { (targetList).after--; } while (false)

/* Test poping an element. */
bool TestPopingElement(void) {
  List(int) list = EmptyList(int);
  PushElement(list, 1, int);
  PushElement(list, 2, int);
  PopElement(list);
  bool result = list.after[-1] == 1;
  DisposeList(list);
  return result;
}

/* Amount of elements in the given list. */
#define CountElements(targetList) ((targetList).after - (targetList).first)

/* Test counting elements. */
bool TestCountingElements(void) {
  List(int) list = EmptyList(int);
  return !CountElements(list);
}

/* Register the tests in the list module. */
void RegisterListModuleTests(void) {
  // Test `List`.
  RegisterUnitTest(TestDefiningList);

  // Test `EmptyList`.
  RegisterUnitTest(TestDefiningEmptyList);

  // Test `DisposeList`.
  RegisterUnitTest(TestDisposingList);

  // Test `ReserveSpace`.
  RegisterUnitTest(TestOpeningSpace);
  RegisterUnitTest(TestEnsuringSpace);

  // Test `PushElement`.
  RegisterUnitTest(TestPushingElement);

  // Test `PopElement`.
  RegisterUnitTest(TestPopingElement);

  // Test `CountElements`.
  RegisterUnitTest(TestCountingElements);
}
