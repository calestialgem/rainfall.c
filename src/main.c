#include "rf_tester.h"

#include <stdbool.h> // bool
#include <stdio.h>   // puts, fputs, stderr
#include <stdlib.h>  // EXIT_FAILURE

/* Runs all the unit tests for the compiler and returns whether all of them
 * passed. */
static bool run_tests(void);

/* Parse the command-line arguments and run the compiler, if all the unit tests
 * pass. */
int main(void) {
  // Stop if there were failed tests.
  if (!run_tests()) {
    (void)fputs("Stopping the compiler because of the failed tests.", stderr);
    return EXIT_FAILURE;
  }
  puts("Hello, world!");
}

static bool run_tests(void) {
  rf_tester_start();
  return rf_tester_report();
}
