/* Launches the compiler and executes the asked commands with the options that
 * were given. */
#ifndef RF_LAUNCHER_H
#define RF_LAUNCHER_H 1

#include "rf_string.h"

#include <stddef.h>

/* Value that indicates the variant of a launch command. */
enum rf_launch_command_variant {
  /* Command for creating a new package in the workspace. */
  RF_LAUNCH_COMMAND_NEW,
  /* Command for syntactically and semantically checking the packages in the
     workspace. */
  RF_LAUNCH_COMMAND_CHECK,
  /* Command for running the tests of the packages in the workspace. */
  RF_LAUNCH_COMMAND_TEST,
  /* Command for building an executable package in the workspace to a C file. */
  RF_LAUNCH_COMMAND_BUILD,
  /* Command for building an running an executable package in the workspace.. */
  RF_LAUNCH_COMMAND_RUN,
};

/* An instruction that can be executed by the Rainfall compiler. */
struct rf_launch_command {
  union {
    /* Command as a new command. */
    struct {
      /* Name of the package that will be created. */
      struct rf_string created_name;
    } as_new;

    /* Command as a check command. */
    struct {
      /* Names of packages that will be checked. If the view is empty, all the
       * packages in the workspace will be checked. */
      struct {
        /* Immutable pointer to the first name that will be checked if it
         * exists. Can be an invalid pointer if the count is 0. */
        struct rf_string const* array;
        /* Number of packages that will be checked. */
        size_t                  count;
      } checked_names;
    } as_check;

    /* Command as a test command. */
    struct {
      /* Names of packages that will be tested. If the view is empty, all the
       * packages in the workspace will be tested. */
      struct {
        /* Immutable pointer to the first name that will be tested if it exists.
         * Can be an invalid pointer if the count is 0. */
        struct rf_string const* array;
        /* Number of packages that will be tested. */
        size_t                  count;
      } tested_names;
    } as_test;

    /* Command as a build command. */
    struct {
      /* Name of the package that will be built. */
      struct rf_string built_name;
    } as_build;

    /* Command as a run command. */
    struct {
      /* Name of the package that will be run. */
      struct rf_string run_name;

      /* Arguments that will be passed to the run package from the
       * command-line. */
      struct {
        /* Immutable pointer to the first argument that will be passed if it
         * exists. Can be an invalid pointer if the count is 0. */
        struct rf_string const* array;
        /* Number of arguments that will be passed. */
        size_t                  count;
      } passed_arguments;
    } as_run;
  };

  /* Variant of the command. */
  enum rf_launch_command_variant variant;
};

#endif // RF_LAUNCHER_H
