// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* A function that returns the result of a unit test. */
typedef bool (*UnitTest)(void);

/* Information on a unit test. */
typedef struct {
  /* Callable of the test. */
  UnitTest    callable;
  /* Name of the test. */
  char const* name;
  /* File the test is in. */
  char const* file;
  /* Line number in the file the test is at. */
  unsigned    line;
} UnitTestInfo;

/* Maximum amount of unit tests that can be stored. */
#define UNIT_TEST_CAPACITY 1024

/* Array of unit tests that are stored. */
UnitTestInfo UnitTestArray[UNIT_TEST_CAPACITY];

/* Amount of unit tests that are registered to the array. */
size_t RegisteredUnitTests = 0;

/* Register the unit test of the given callable to the array with the calling
 * file and line information. */
#define RegisterUnitTest(registeredUnitTest) \
  RegisterUnitTestInfo((UnitTestInfo){       \
    registeredUnitTest, #registeredUnitTest, __FILE__, __LINE__})

/* Register the given unit test information. */
void RegisterUnitTestInfo(UnitTestInfo const registered) {
  // Check existance.
  for (size_t test = 0; test < RegisteredUnitTests; test++) {
    if (registered.callable == UnitTestArray[test].callable) {
      fprintf(stderr, "Test `%s` is already registered!\n", registered.name);
      abort();
      return;
    }
  }

  // Check the capacity.
  if (RegisteredUnitTests == UNIT_TEST_CAPACITY) {
    fprintf(stderr, "No space to add test `%s`!\n", registered.name);
    abort();
    return;
  }

  // Push to the end of the array.
  UnitTestArray[RegisteredUnitTests++] = registered;
}

/* Current UTC time in seconds. */
long double GetTimeInSeconds() {
  // Get the time and join the seconds with the nanoseconds.
  struct timespec now;
  timespec_get(&now, TIME_UTC);
  long double const nanoScale = 1e9;
  return (long double)now.tv_sec + now.tv_nsec / nanoScale;
}

/* Run all the unit tests. */
void RunUnitTests(void) {
  // Store the time for measuring the total time taken.
  long double const start = GetTimeInSeconds();

  // Run all the tests and count the failed ones.
  size_t failedTests = 0;
  for (size_t test = 0; test < RegisteredUnitTests; test++) {
    // Store the time for measuring the time taken by currently tested unit.
    long double const unitStart = GetTimeInSeconds();

    // Test and report.
    UnitTestInfo const tested = UnitTestArray[test];
    if (tested.callable()) {
      printf("[PASS]");
    } else {
      printf("[FAIL]");
      failedTests++;
    }

    // Find the elapsed time for the currently tested unit.
    long double const milliScale          = 1e3;
    long double const elapsedSeconds      = GetTimeInSeconds() - unitStart;
    long double const elapsedMilliseconds = elapsedSeconds * milliScale;
    printf(" %s (%.3Lf ms)\n", tested.name, elapsedMilliseconds);
  }

  // Print whether there were failed tests.
  if (RegisteredUnitTests == 0) printf("\nThere are no registered tests!");
  else if (failedTests == 0) printf("\nAll tests passed.");
  else if (failedTests == 1) printf("\nA test failed!");
  else printf("\n%zu tests failed!", failedTests);

  // Find the total elapsed time.
  long double const milliScale          = 1e3;
  long double const elapsedSeconds      = GetTimeInSeconds() - start;
  long double const elapsedMilliseconds = elapsedSeconds * milliScale;
  printf(" (%.3Lf ms)\n", elapsedMilliseconds);
}

/* Always passes to check the test infrastructure. */
bool TestRegisteringUnitTests(void) { return true; }

/* Register the tests in the test module. */
void RegisterTestModuleTests(void) {
  RegisterUnitTest(TestRegisteringUnitTests);
}
