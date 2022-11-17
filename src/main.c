#include "rf_launcher.h"
#include "rf_tester.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Command-line arguments that are given to the compiler. The first argument is
 * skipped because it is assumed to be the path to the executable. */
struct arguments {
  /* Immutable pointer to the first argument after the path to the executable if
   * it exists. */
  char const* const* array;
  /* Number of arguments after the path to the executable. */
  int                count;
};

/* Runs all the unit tests for the compiler and returns whether all of them
 * passed. */
static bool run_tests(void);
/* Parse the given command-line arguments to the given launch command. Returns
 * whether it successfully parsed. */
static bool
parse_arguments(struct arguments parsed, struct rf_launch_command* result);

/* Parse the command-line arguments and run the compiler, if all the unit tests
 * pass. */
int main(int arguments_count, char const* const* arguments_array) {
  // Stop if there were failed tests.
  if (!run_tests()) {
    (void)fputs("Stopping the compiler because of the failed tests.", stderr);
    return EXIT_FAILURE;
  }

  // Create the arguments by skipping the first one, which is assumed to be the
  // path to the executable.
  struct arguments arguments = {
    .array = arguments_array + 1, .count = arguments_count - 1};

  // Parse the arguments.
  struct rf_launch_command launched;
  if (!parse_arguments(arguments, &launched)) { return EXIT_FAILURE; }

  // Launch the parsed command.
  rf_launch(launched);
  return EXIT_SUCCESS;
}

static bool run_tests(void) {
  rf_begin_tests();
  rf_test_tester();
  return rf_report_tests();
}

static bool
parse_arguments(struct arguments parsed, struct rf_launch_command* result) {
  return true;
}
