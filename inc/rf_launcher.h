/* Launches the compiler and executes the asked commands with the options that
 * were given. */
#ifndef RF_LAUNCHER_H
#define RF_LAUNCHER_H 1

#include "rf_string.h"

#include <stddef.h>

/* An instruction that can be executed by the compiler. */
struct rf_launch_command {
  union {
    /* Command for creating a new package in the workspace. */
    struct rf_new_command {
      /* Name of the package that will be created. */
      struct rf_string created_name;
    }
    /* Command as a new command. */
    as_new;

    /* Command for syntactically and semantically checking the packages in the
     * workspace. */
    struct rf_check_command {
      struct {
        /* Immutable pointer to the first name that will be checked if it
         * exists. Can be an invalid pointer if the count is 0. */
        struct rf_string const* array;
        /* Number of packages that will be checked. */
        size_t                  count;
      }
      /* Names of packages that will be checked. If the view is empty, all the
       * packages in the workspace will be checked. */
      checked_names;
    }
    /* Command as a check command. */
    as_check;

    /* Command for running the tests of the packages in the workspace. */
    struct rf_test_command {
      struct {
        /* Immutable pointer to the first name that will be tested if it exists.
         * Can be an invalid pointer if the count is 0. */
        struct rf_string const* array;
        /* Number of packages that will be tested. */
        size_t                  count;
      }
      /* Names of packages that will be tested. If the view is empty, all the
       * packages in the workspace will be tested. */
      tested_names;
    }
    /* Command as a test command. */
    as_test;

    /* Command for building an executable package in the workspace to a C
     * file. */
    struct rf_build_command {
      /* Name of the package that will be built. */
      struct rf_string built_name;
    }
    /* Command as a build command. */
    as_build;

    /* Command for building an running an executable package in the
     * workspace. */
    struct rf_run_command {
      /* Name of the package that will be run. */
      struct rf_string run_name;

      struct {
        /* Immutable pointer to the first argument that will be passed if it
         * exists. Can be an invalid pointer if the count is 0. */
        struct rf_string const* array;
        /* Number of arguments that will be passed. */
        size_t                  count;
      }
      /* Arguments that will be passed to the run package from the
       * command-line. */
      passed_arguments;
    }
    /* Command as a run command. */
    as_run;
  };

  /* Value that indicates the variant of a launch command. */
  enum rf_launch_command_variant {
    RF_NEW_COMMAND,
    RF_CHECK_COMMAND,
    RF_TEST_COMMAND,
    RF_BUILD_COMMAND,
    RF_RUN_COMMAND,
  }
  /* Variant of the command. */
  variant;
};

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Launches the compiler with the given command and configuration. */
void rf_launch(struct rf_launch_command launched,
  struct rf_string                      configuration_name);

#endif // RF_LAUNCHER_H
