#include "rf_allocator.h"
#include "rf_filesystem.h"
#include "rf_launcher.h"
#include "rf_string.h"
#include "rf_tester.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Data of the command-line argument parsing proccess. */
struct parse_context {
  /* Command-line arguments that are given to the compiler.
   * The first argument is skipped because it is assumed to
   * be the path to the executable. */
  struct {
    /* Immutable pointer to the first argument after the path to the executable
     * if it exists. */
    struct rf_string const* array;
    /* Number of arguments after the path to the executable. */
    int                     count;
  } arguments;

  /* Index the argument that is going to be parsed next. Can be equal to the
   * number of arguments, which means there is no more arguments to parse. */
  int                      next_index;
  /* Launch command that is the result of parsing the command-line arguments. */
  struct rf_launch_command result;
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
/* Runs the given context. Returns the result. */
static enum parse_result parse_arguments(struct parse_context* context);
/* Parses the next option from the given context. Moves the context after the
 * parsed option. Returns the result. */
static enum parse_result parse_option(struct parse_context* context);
/* Parses the next argument of the directory option from the given context.
 * Moves the context after the parsed argument. Returns the result. */
static enum parse_result parse_directory_option(struct parse_context* context);
/* Parses the next argument of the configuration option from the given context.
 * Moves the context after the parsed argument. Returns the result. */
static enum parse_result parse_configuration_option(
  struct parse_context* context);
/* Parses the next command from the given context. Moves the context after the
 * parsed command. Returns the result. */
static enum parse_result parse_command(struct parse_context* context);
/* Parses the next argument of the new command from the given context.
 * Moves the context after the parsed argument. Returns the result. */
static enum parse_result parse_new_command(struct parse_context* context);
/* Parses the next arguments of the check command from the given context.
 * Moves the context after the parsed arguments. Returns the result. */
static enum parse_result parse_check_command(struct parse_context* context);
/* Parses the next arguments of the test command from the given context.
 * Moves the context after the parsed arguments. Returns the result. */
static enum parse_result parse_test_command(struct parse_context* context);
/* Parses the next argument of the new command from the given context.
 * Moves the context after the parsed argument. Returns the result. */
static enum parse_result parse_build_command(struct parse_context* context);
/* Parses the next arguments of the run command from the given context.
 * Moves the context after the parsed arguments. Returns the result. */
static enum parse_result parse_run_command(struct parse_context* context);
/* Stores the next argument from the given context to the given target. Moves
 * the context after the stored argument. Returns the result. */
static enum parse_result store_next_argument(struct parse_context* context,
  struct rf_string*                                                target);
/* Stores the all remaining argument from the given context to the given target.
 * Moves the context to the end of the arguments. */
static void store_remaining_arguments(struct parse_context* context,
  struct rf_string const** target_array, size_t* target_count);

/* Parse the command-line arguments and run the compiler, if all the unit tests
 * pass. */
int main(int arguments_count, char const* const* arguments_array) {
  // Stop if there were failed tests.
  if (!run_tests()) {
    fputs("info: Stopping the compiler because of the failed tests.\n", stderr);
    return EXIT_FAILURE;
  }

  // Check whether there are no arguments, which means the assumption that the
  // first argument is the path to the executable of the compiler does not hold.
  if (arguments_count < 1) {
    fputs(
      "failure: Command line arguments do not start with the path to the "
      "executable!",
      stderr);
    return EXIT_FAILURE;
  }

  // Create the arguments by skipping the first one, which is assumed to be the
  // path to the executable.
  struct {
    struct rf_string* array;
    int               count;
  } arguments_as_string;

  arguments_as_string.count = arguments_count - 1;
  if (RF_ALLOCATE_ARRAY(&arguments_as_string.array, arguments_as_string.count,
        struct rf_string)) {
    fprintf(stderr,
      "failure: Could not allocate memory for processing command-line "
      "arguments!\n"
      "cause: %s\n",
      strerror(errno));
    return EXIT_FAILURE;
  }
  for (int i = 0; i < arguments_as_string.count; i++) {
    arguments_as_string.array[i] =
      rf_view_null_terminated(arguments_array[i + 1]);
  }

  // Parse the arguments and launch if it was successful.
  struct parse_context context;
  context.arguments.array = arguments_as_string.array;
  context.arguments.count = arguments_as_string.count;
  context.next_index      = 0;
  int exit_code           = EXIT_SUCCESS;
  switch (parse_arguments(&context)) {
  case PARSE_CANCELED: break;
  case PARSE_FAILED: exit_code = EXIT_FAILURE; break;
  case PARSE_SUCCEEDED: rf_launch(context.result); break;
  }
  RF_FREE(&arguments_as_string.array);

  // Report allocations and exit.
  rf_report_allocations();
  return exit_code;
}

static bool run_tests(void) {
  rf_begin_tests();
  rf_test_tester();
  return rf_report_tests();
}

static enum parse_result parse_arguments(struct parse_context* context) {
  // If no arguments are given, print the version and help message.
  if (context->arguments.count == 0) {
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
  context->result.configuration_name = RF_EMPTY_STRING;

  // Try parsing options until cannot.
  for (;;) {
    switch (parse_option(context)) {
    case PARSE_CANCELED: break;
    case PARSE_FAILED: return PARSE_FAILED;
    case PARSE_SUCCEEDED: continue;
    }
    break;
  }

  switch (parse_command(context)) {
  case PARSE_CANCELED: fprintf(stderr, "failure: Expected a command!\n");
  case PARSE_FAILED: return PARSE_FAILED;
  case PARSE_SUCCEEDED: break;
  }

  // Check whether there are unused arguments.
  if (context->next_index < context->arguments.count) {
    fputs("failure: There are extra arguments provided to the command: `",
      stderr);

    // Print the extra arguments as a comma-separated list. To that end, the
    // first extra argument is printed first, then the rest is leaded by a
    // comma.
    for (fputs(context->arguments.array[context->next_index++].array, stderr);
         context->next_index < context->arguments.count;
         context->next_index++) {
      fputs("`, `", stderr);
      fputs(context->arguments.array[context->next_index].array, stderr);
    }

    fputs(
      "`!\n"
      "info: Run the compiler without arguments to see the usage.\n",
      stderr);
    return PARSE_FAILED;
  }

  return PARSE_SUCCEEDED;
}

static enum parse_result parse_option(struct parse_context* context) {
  // Check whether there is an argument left.
  if (context->next_index == context->arguments.count) {
    return PARSE_CANCELED;
  }

  // Get the first argument, which should be the option name or shortcut.
  struct rf_string option = context->arguments.array[context->next_index++];

  // Check whether it is a name, which starts with a '--'.
  if (option.count >= 2 && option.array[0] == '-' && option.array[1] == '-') {
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
      return parse_directory_option(context);
    }
    if (rf_compare_strings(option, rf_view_null_terminated("configuration"))) {
      return parse_configuration_option(context);
    }

    fprintf(stderr,
      "failure: Unknown option name: `%.*s`!\n"
      "info: Run the compiler without arguments to see the usage.\n",
      (int)option.count, option.array);
    return PARSE_FAILED;
  }

  // Check whether it is a shortcut, which starts with a '-'.
  if (option.count >= 1 && option.array[0] == '-') {
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
    case 'd': return parse_directory_option(context);
    case 'c': return parse_configuration_option(context);
    default:
      fprintf(stderr,
        "failure: Unknown option shortcut: `%c`!\n"
        "info: Run the compiler without arguments to see the usage.\n",
        option.array[1]);
      return PARSE_FAILED;
    }
  }

  // If it was not an option name or shortcut, cancel the parse. Retreat from
  // the previously skipped over argument that was thought to be an option name
  // or shortcut.
  context->next_index--;
  return PARSE_CANCELED;
}

static enum parse_result parse_directory_option(struct parse_context* context) {
  // Make sure the option is provided once.
  static bool already_parsed = false;
  if (already_parsed) {
    fprintf(stderr, "failure: Multiple directory options are provided!");
    return PARSE_FAILED;
  }
  already_parsed = true;

  // Check whether there is an argument left.
  if (context->next_index == context->arguments.count) {
    fprintf(stderr, "failure: Path to the workspace is not provided!");
    return PARSE_FAILED;
  }

  // Set the path and advance over it.
  struct rf_string workspace_directory =
    context->arguments.array[context->next_index++];
  if (rf_change_working_directory(workspace_directory)) {
    fprintf(stderr,
      "failure: Cannot set the workspace directory to `%.*s`!\n"
      "cause: %s\n",
      (int)workspace_directory.count, workspace_directory.array,
      strerror(errno));
    return PARSE_FAILED;
  }

  return PARSE_SUCCEEDED;
}

static enum parse_result parse_configuration_option(
  struct parse_context* context) {
  // Make sure the option is provided once.
  static bool already_parsed = false;
  if (already_parsed) {
    fprintf(stderr, "failure: Multiple configuration options are provided!");
    return PARSE_FAILED;
  }
  already_parsed = true;

  // Check whether there is an argument left.
  if (context->next_index == context->arguments.count) {
    fprintf(stderr, "failure: Name of the configuration is not provided!");
    return PARSE_FAILED;
  }

  // Set the name and advance over it.
  context->result.configuration_name =
    context->arguments.array[context->next_index++];
  return PARSE_SUCCEEDED;
}

static enum parse_result parse_command(struct parse_context* context) {
  // Check whether there is an argument left.
  if (context->next_index == context->arguments.count) {
    return PARSE_CANCELED;
  }

  // Get the command name or shortcut.
  struct rf_string command = context->arguments.array[context->next_index++];

  // Check whether the command is a shortcut.
  if (command.count == 1) {
    switch (command.array[0]) {
    case 'n': return parse_new_command(context);
    case 'c': return parse_check_command(context);
    case 't': return parse_test_command(context);
    case 'b': return parse_build_command(context);
    case 'r': return parse_run_command(context);
    default:
      fprintf(stderr,
        "failure: Unknown command shortcut `%c`!\n"
        "info: Run the compiler without arguments to see the usage.\n",
        command.array[0]);
      return PARSE_FAILED;
    }
  } else {
    if (rf_compare_strings(command, rf_view_null_terminated("new"))) {
      return parse_new_command(context);
    }
    if (rf_compare_strings(command, rf_view_null_terminated("check"))) {
      return parse_check_command(context);
    }
    if (rf_compare_strings(command, rf_view_null_terminated("test"))) {
      return parse_test_command(context);
    }
    if (rf_compare_strings(command, rf_view_null_terminated("build"))) {
      return parse_build_command(context);
    }
    if (rf_compare_strings(command, rf_view_null_terminated("run"))) {
      return parse_run_command(context);
    }

    fprintf(stderr,
      "failure: Unknown command name `%.*s`!\n"
      "info: Run the compiler without arguments to see the usage.\n",
      (int)command.count, command.array);
    return PARSE_FAILED;
  }
}

static enum parse_result parse_new_command(struct parse_context* context) {
  switch (store_next_argument(context, &context->result.as_run.run_name)) {
  case PARSE_CANCELED:
    fputs(
      "failure: Name for the package that will be created is not provided!"
      "info: Run the compiler without arguments to see the usage.\n",
      stderr);
  case PARSE_FAILED: return PARSE_FAILED;
  case PARSE_SUCCEEDED: break;
  }

  // Set the variant.
  context->result.variant = RF_LAUNCH_COMMAND_NEW;
  return PARSE_SUCCEEDED;
}

static enum parse_result parse_check_command(struct parse_context* context) {
  store_remaining_arguments(context,
    &context->result.as_check.checked_names.array,
    &context->result.as_check.checked_names.count);

  // Set the variant.
  context->result.variant = RF_LAUNCH_COMMAND_CHECK;
  return PARSE_SUCCEEDED;
}

static enum parse_result parse_test_command(struct parse_context* context) {
  store_remaining_arguments(context,
    &context->result.as_test.tested_names.array,
    &context->result.as_test.tested_names.count);

  // Set the variant.
  context->result.variant = RF_LAUNCH_COMMAND_TEST;
  return PARSE_SUCCEEDED;
}

static enum parse_result parse_build_command(struct parse_context* context) {
  switch (store_next_argument(context, &context->result.as_run.run_name)) {
  case PARSE_CANCELED:
    fputs(
      "failure: Name for the package that will be built is not provided!"
      "info: Run the compiler without arguments to see the usage.\n",
      stderr);
  case PARSE_FAILED: return PARSE_FAILED;
  case PARSE_SUCCEEDED: break;
  }

  // Set the variant.
  context->result.variant = RF_LAUNCH_COMMAND_BUILD;
  return PARSE_SUCCEEDED;
}

static enum parse_result parse_run_command(struct parse_context* context) {
  switch (store_next_argument(context, &context->result.as_run.run_name)) {
  case PARSE_CANCELED:
    fputs(
      "failure: Name for the package that will be run is not provided!"
      "info: Run the compiler without arguments to see the usage.\n",
      stderr);
  case PARSE_FAILED: return PARSE_FAILED;
  case PARSE_SUCCEEDED: break;
  }

  store_remaining_arguments(context,
    &context->result.as_run.passed_arguments.array,
    &context->result.as_run.passed_arguments.count);

  // Set the variant.
  context->result.variant = RF_LAUNCH_COMMAND_RUN;
  return PARSE_SUCCEEDED;
}

static enum parse_result store_next_argument(struct parse_context* context,
  struct rf_string*                                                target) {
  // Check whether there is an argument left.
  if (context->next_index == context->arguments.count) {
    return PARSE_CANCELED;
  }

  // Set the name and advance over it.
  *target = context->arguments.array[context->next_index++];
  return PARSE_SUCCEEDED;
}

static void store_remaining_arguments(struct parse_context* context,
  struct rf_string const** target_array, size_t* target_count) {
  *target_array       = context->arguments.array + context->next_index;
  *target_count       = context->arguments.count - context->next_index;
  context->next_index = context->arguments.count;
}
