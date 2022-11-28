#ifndef RAINFALL
#define RAINFALL 1

#include <stdbool.h>
#include <stddef.h>

/* Information on the result of an operation. */
enum rf_status {
  /* Operation was completed without any errors. */
  RF_SUCCEEDED,
  /* Operation did not start because some requirements were not met. */
  RF_CANCELED,
  /* Operation started, but it was aborted because of an error. */
  RF_FAILED,
};

/* Immutable view of linearly stored group of UTF-8 encoded characters. The
 * string is NOT guaranteed to be null-terminated. */
struct rf_string {
  /* Immutable pointer to the first character in the string if it exists. Can be
   * an invalid pointer if the count is 0. */
  char const* array;
  /* Number of characters in the string. */
  size_t      count;
};

/* Null-terminated, linearly stored group of UTF-8 encoded characters. */
struct rf_buffer {
  /* Pointer to the first character in the buffer if it exists. Otherwise points
   * to the null-termination character. */
  char*  array;
  /* Number of characters in the buffer excluding the null-termination
   * character. */
  size_t count;
  /* Maximum amount of characters that fits into the array excluding the
   * null-termination character. */
  size_t limit;
};

/* Forest of sources and modules, where the roots are packages. The children
 * come before the parent. Modules are branches, while the sources are
 * leaves. */
struct rf_workspace {
  /* A node in the workspace forest. Might be a source or module. Also it is a
   * package if its at the top level. */
  struct rf_workspace_node {
    /* Name of the source or module. */
    struct rf_string name;

    union {
      /* Identifier of the source model. */
      size_t model_id;
      /* Number of sources and submodules that are directly under the module. */
      size_t children_count;
    };

    /* Variant of the workspace node. */
    enum rf_workspace_node_variant {
      /* Node as source. */
      RF_SOURCE,
      /* Node as module. */
      RF_MODULE,
    }
    /* Variant of the node. */
    variant;
  }
    /* Pointer to the first element of the array of nodes. */
    * array;

  /* Number of nodes stored in the array. */
  size_t count;
  /* Number of allocated node space in the array. */
  size_t limit;
};

/* Model of a source file as an ordered collection of characters. */
struct rf_linear_model {
  /* Immutable view to the characters in the file. */
  struct rf_string contents;
};

/* Helper that loads the contents of source files into memory. */
struct rf_loader {
  struct {
    /* Pointer to the first character of the first loaded file if it exists. */
    char*  array;
    /* Number of characters in the array. */
    size_t count;
    /* Maximum number of characters that could fit in the array. */
    size_t limit;
  }
  /* Combined contents of all the loaded files. */
  file_contents_buffer;

  struct {
    /* Pointer to the first model if it exists. */
    struct rf_linear_model* array;
    /* Number of models in the array. */
    size_t                  count;
    /* Maximum number of models that could fit the array. */
    size_t                  limit;
  }
  /* Linear models of the sources. */
  models;
};

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

/* String without any characters in it. */
extern struct rf_string const RF_EMPTY_STRING;

/* Returns the view of the given null-terminated array as a string. */
struct rf_string rf_view_null_terminated(char const* viewed);
/* Returns whether the characters of the given strings are the same and ordered
 * the same way. */
bool   rf_compare_strings(struct rf_string this, struct rf_string that);
/* Finds whether the given string has the given prefix at its start. */
bool   rf_compare_prefix(struct rf_string compared, struct rf_string prefix);
/* Finds whether the given string has the given suffix at its end. */
bool   rf_compare_suffix(struct rf_string compared, struct rf_string suffix);
/* Removes the given amount of characters from the start of the given string. */
void   rf_skip_prefix(struct rf_string* skipped, size_t amount);
/* Finds the index of the first character in the given source after the last
 * occurrence of the given string. Returns `0` if the string does not exist.*/
size_t rf_find_last_occurrence(struct rf_string source, struct rf_string found);

/* Creates a buffer with just a null-termination character. */
struct rf_buffer rf_create_buffer(void);
/* Frees the memory resources used by the given buffer. */
void             rf_free_buffer(struct rf_buffer* freed);
/* Appends the given UTF-8 encoded character to the end of the given buffer. */
void rf_append_character(struct rf_buffer* destination, int appended);
/* Appends the given string to the end of the given buffer. */
void rf_append_string(struct rf_buffer* destination, struct rf_string appended);

/* Adds the given node to the end of the given workspace. Returns whether the
 * operation failed. Sets `errno` on failure. */
bool rf_push_workspace_node(struct rf_workspace* target,
  struct rf_workspace_node                       pushed);
/* Frees the resources owned by the given workspace. */
void rf_free_workspace(struct rf_workspace* freed);

/* Loads the workspace to the given loader and the workspace. */
enum rf_status rf_load_workspace(struct rf_loader* target_loader,
  struct rf_workspace*                             target_workspace);
/* Frees the memory resources owned by the given loader. */
void           rf_free_loader(struct rf_loader* freed);

/* Launches the compiler with the given command and configuration. */
void rf_launch(struct rf_launch_command launched,
  struct rf_string                      configuration_name);

#endif // RAINFALL
