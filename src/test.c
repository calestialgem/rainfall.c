// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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
  // Check the capacity.
  if (RegisteredUnitTests == UNIT_TEST_CAPACITY) {
    fputs("Out of unit test capacity!\n", stderr);
    abort();
    return;
  }

  // Push to the end of the array.
  UnitTestArray[RegisteredUnitTests++] = registered;
}

/* Run all the unit tests. */
void RunUnitTests(void) {
  // Run all the tests and count the failed ones.
  size_t failedTests = 0;
  for (size_t test = 0; test < RegisteredUnitTests; test++) {
    UnitTestInfo const tested = UnitTestArray[test];
    if (tested.callable()) {
      printf("[PASS] %s\n", tested.name);
    } else {
      fprintf(stderr, "[FAIL] %s\n", tested.name);
      failedTests++;
    }
  }

  // Print whether there were failed tests.
  if (RegisteredUnitTests == 0) puts("\nThere are no registered tests!\n");
  else if (failedTests == 0) puts("\nAll tests passed.\n");
  else if (failedTests == 1) fputs("\nA test failed!\n", stderr);
  else fprintf(stderr, "\n%zu tests failed!\n", failedTests);
}

/* Always passes to check the test infrastructure. */
bool TestRegisteringUnitTests(void) { return true; }

/* Register the tests in the test module. */
void RegisterTestModuleTests(void) {
  RegisterUnitTest(TestRegisteringUnitTests);
}
