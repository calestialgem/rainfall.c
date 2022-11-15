#include "rf_tester.h"

#include <stdio.h>  // fputs, printf, stderr
#include <stdlib.h> // abort
#include <time.h>   // timespec, timespec_get, TIME_UTC

/* Point at time when the tester starts. */
static double   tester_start_milliseconds;
/* Number of tests that were run. */
static unsigned run_test_count;
/* Number of tests that failed. */
static unsigned failed_test_count;

/* Current point at time in milliseconds. */
static double current_milliseconds(void);
/* Elapsed duration starting from the given point at time in milliseconds. */
static double elapsed_milliseconds(double start_milliseconds);

void rf_test_unit(unit_test_t tested_unit, char const* reported_name) {
  // Run the test while measuring the elapsed time.
  double start_milliseconds    = current_milliseconds();
  bool   outcome               = tested_unit();
  double duration_milliseconds = elapsed_milliseconds(start_milliseconds);

  // Return if there are no problems.
  double const ACCEPTABLE_DURATION_MILLISECONDS = 10.0;
  if (outcome && duration_milliseconds <= ACCEPTABLE_DURATION_MILLISECONDS) {
    return;
  }

  if (!outcome) { failed_test_count++; }
  printf(
    "[%s] %s (%.3f ms)\n", outcome ? "TOO LONG" : "FAILED", reported_name,
    duration_milliseconds);
}

void rf_tester_start(void) {
  tester_start_milliseconds = current_milliseconds();
  run_test_count            = 0;
  failed_test_count         = 0;
}

bool rf_tester_report(void) {
  double duration_milliseconds =
    elapsed_milliseconds(tester_start_milliseconds);

  // Report the results.
  if (run_test_count == 0) {
    printf("No tests were run.");
  } else if (failed_test_count == 0) {
    printf("All %d test have passed.", run_test_count);
  } else {
    printf("%d/%d of the tets failed!", failed_test_count, run_test_count);
  }
  printf(" (%.3f ms)\n", duration_milliseconds);

  return failed_test_count == 0;
}

static double current_milliseconds(void) {
  // Get the current time.
  struct timespec current_time;
  if (timespec_get(&current_time, TIME_UTC) == 0) {
    (void)fputs("Could not get the time!\n", stderr);
    abort();
  }

  // Find the contributions to milliseconds from the seconds and nanoseconds
  // parts.
  double const MILLI_SCALE  = 1E3;
  double const NANO_SCALE   = 1E9;
  double       from_seconds = (double)current_time.tv_sec * MILLI_SCALE;
  double from_nanoseconds   = current_time.tv_nsec * MILLI_SCALE / NANO_SCALE;
  return from_seconds + from_nanoseconds;
}

static double elapsed_milliseconds(double start_milliseconds) {
  return current_milliseconds() - start_milliseconds;
}
