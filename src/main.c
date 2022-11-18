#include "rf_launcher.h"
#include "rf_string.h"
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

/* Information on the result of parsing command-line arguments. */
enum parse_result {
  /* Parse did not start because some requirements were not met. */
  PARSE_CANCELED,
  /* Parse started, but it was aborted because of an error. */
  PARSE_FAILED,
  /* Parse finished without any errors. */
  PARSE_SUCCEEDED,
};

/* Runs all the unit tests for the compiler and returns whether all of them
 * passed. */
static bool              run_tests(void);
/* Parses the given command-line arguments to the given launch command. Returns
 * the result. */
static enum parse_result parse_arguments(struct arguments parsed,
  struct rf_launch_command*                               result);
/* Parses the first option in the given arguments starting at the given
 * index to the given launch command. Moves the given index to the position
 * after the parsed arguments. Returns the result. */
static enum parse_result parse_option(struct arguments parsed,
  int* parsed_index, struct rf_launch_command* result);
/* Parses the directory option argument starting at the given index to the given
 * launch command. Moves the given index to the position after the parsed
 * argument. Returns the result. */
static enum parse_result parse_directory_option(struct arguments parsed,
  int* parsed_index, struct rf_launch_command* result);
/* Parses the configuration option argument starting at the given index to the
 * given launch command. Moves the given index to the position after the parsed
 * argument. Returns the result. */
static enum parse_result parse_configuration_option(struct arguments parsed,
  int* parsed_index, struct rf_launch_command* result);

/* Parse the command-line arguments and run the compiler, if all the unit tests
 * pass. */
int main(int arguments_count, char const* const* arguments_array) {
  // Stop if there were failed tests.
  if (!run_tests()) {
    fputs("info: Stopping the compiler because of the failed tests.\n", stderr);
    return EXIT_FAILURE;
  }

  // Create the arguments by skipping the first one, which is assumed to be the
  // path to the executable.
  struct arguments arguments = {.array = arguments_array + 1,
    .count                             = arguments_count - 1};

  // Parse the arguments and launch if it was successful.
  struct rf_launch_command launched;
  switch (parse_arguments(arguments, &launched)) {
  case PARSE_FAILED: return EXIT_FAILURE;
  case PARSE_SUCCEEDED: rf_launch(launched);
  case PARSE_CANCELED: return EXIT_SUCCESS;
  }
}

static bool run_tests(void) {
  rf_begin_tests();
  rf_test_tester();
  return rf_report_tests();
}

static enum parse_result parse_arguments(struct arguments parsed,
  struct rf_launch_command*                               result) {
  // If no arguments are given, print the version and help message.
  if (parsed.count == 0) {
    printf(
      "Rainfall Thrice to C Transpiler Version 0.0.1\n"
      "Usage: rainfall [options] <command> [arguments]\n"
      "\n"
      "Commands:\n"
      " (n) new <name>: Creates a new package with the given name.\n"
      " (c) check [names]: Checks the given packages or all the packages "
      "in "
      "the workspace if none is given.\n"
      " (t) test [names]: Tests the given packages or all the packages in "
      "the workspace if none is given.\n"
      " (b) build <name>: Builds the executable package with the given "
      "name.\n"
      " (r) run <name> [arguments]: Runs the executable package with the "
      "given name by passing the given command-line arguments to it.\n"
      "\n"
      "Options:\n"
      " (-d) --directory <path>: Runs the compiler in the workspace directory "
      "at the given path. Defaults to the current working directory.\n"
      " (-c) --configuration <name>: Runs the compiler with the configuration "
      "that has the given name. Defaults to the default configuration if it "
      "exists. Otherwise, runs the compiler without setting a "
      "configuration.\n");
    return PARSE_CANCELED;
  }

  // Set the default option values.
  result->workspace_path     = rf_view_null_terminated(".");
  result->configuration_name = RF_EMPTY_STRING;

  // Try parsing options until cannot.
  int parsed_index = 0;
  for (;;) {
    switch (parse_option(parsed, &parsed_index, result)) {
    case PARSE_CANCELED: break;
    case PARSE_FAILED: return PARSE_FAILED;
    case PARSE_SUCCEEDED: continue;
    }
    break;
  }

  return PARSE_SUCCEEDED;
}

static enum parse_result parse_option(struct arguments parsed,
  int* parsed_index, struct rf_launch_command* result) {
  // Check whether there is an argument left.
  if (parsed.count <= *parsed_index) { return PARSE_CANCELED; }

  // Get the first argument, which should be the option name or shortcut.
  struct rf_string option =
    rf_view_null_terminated(parsed.array[*parsed_index]);

  // Check whether it is a name, which starts with a '--'.
  if (option.count >= 2 && option.array[0] == '-' && option.array[1] == '-') {
    // Advance the index over the option.
    (*parsed_index)++;

    // Remove the '--'.
    rf_skip_prefix(&option, 2);

    // Check the name length.
    if (option.count == 0) {
      fprintf(stderr, "failure: Expected an option name after `--`!\n");
      return PARSE_FAILED;
    }
    if (option.count == 1) {
      fprintf(stderr,
        "failure: Option name must be longer than a single character!\n"
        "info: Use `-%c` for providing only the shortcut.\n",
        option.array[0]);
      return PARSE_FAILED;
    }

    // Dispatch to option.
    if (rf_compare_strings(option, rf_view_null_terminated("directory"))) {
      return parse_directory_option(parsed, parsed_index, result);
    }
    if (rf_compare_strings(option, rf_view_null_terminated("configuration"))) {
      return parse_configuration_option(parsed, parsed_index, result);
    }

    fprintf(stderr,
      "failure: Unknown option name: `%.*s`!\n"
      "info: Run the compiler without arguments to see the usage.\n",
      (int)option.count, option.array);
    return PARSE_FAILED;
  }

  // Check whether it is a shortcut, which starts with a '-'.
  if (option.count >= 1 && option.array[0] == '-') {
    // Advance the index over the option.
    (*parsed_index)++;

    // Check the shortcut length.
    if (option.count < 2) {
      fprintf(stderr, "failure: Expected an option shortcut after `-`!\n");
      return PARSE_FAILED;
    }
    if (option.count > 2) {
      fprintf(stderr,
        "failure: Option shortcut must be a single character!\n"
        "info: Use `-%.*s` for providing the full name.\n",
        (int)option.count, option.array);
      return PARSE_FAILED;
    }

    // Dispatch to option.
    switch (option.array[1]) {
    case 'd': return parse_directory_option(parsed, parsed_index, result);
    case 'c': return parse_configuration_option(parsed, parsed_index, result);
    default:
      fprintf(stderr,
        "failure: Unknown option shortcut: `%c`!\n"
        "info: Run the compiler without arguments to see the usage.\n",
        option.array[1]);
      return PARSE_FAILED;
    }
  }

  // If it was not an option name or shortcut, cancel the parse.
  return PARSE_CANCELED;
}

static enum parse_result parse_directory_option(struct arguments parsed,
  int* parsed_index, struct rf_launch_command* result) {
  // Make sure the option is provided once.
  static bool already_parsed = false;
  if (already_parsed) {
    fprintf(stderr, "failure: Multiple directory options are provided!");
    return PARSE_FAILED;
  }
  already_parsed = true;

  // Check whether there is an argument left.
  if (parsed.count <= *parsed_index) {
    fprintf(stderr, "failure: Path to the workspace is not provided!");
    return PARSE_FAILED;
  }

  // Set the path and advance over it.
  result->workspace_path = rf_view_null_terminated(parsed.array[*parsed_index]);
  (*parsed_index)++;
  return PARSE_SUCCEEDED;
}

static enum parse_result parse_configuration_option(struct arguments parsed,
  int* parsed_index, struct rf_launch_command* result) {
  // Make sure the option is provided once.
  static bool already_parsed = false;
  if (already_parsed) {
    fprintf(stderr, "failure: Multiple configuration options are provided!");
    return PARSE_FAILED;
  }
  already_parsed = true;

  // Check whether there is an argument left.
  if (parsed.count <= *parsed_index) {
    fprintf(stderr, "failure: Name of the configuration is not provided!");
    return PARSE_FAILED;
  }

  // Set the name and advance over it.
  result->configuration_name =
    rf_view_null_terminated(parsed.array[*parsed_index]);
  (*parsed_index)++;
  return PARSE_SUCCEEDED;
}
