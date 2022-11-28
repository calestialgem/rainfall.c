#ifndef RF_INTERNAL
#define RF_INTERNAL 1

#include "rainfall.h"

#include <stdio.h>

/* A function type that is a unit test. Runs the test when called, and returns
 * whether the test passed. */
typedef bool (*unit_test_t)(void);

/* Path to a file, which is a null terminated string.*/
struct rf_path {
  /* Pointer to the first character in the path if it exists. Fallowed by a
   * null-termination. */
  char*  array;
  /* Number of characters in the path. Does not include the null-termination. */
  size_t count;
};

/* Implementation defined data for walking over the entries in a directory. */
struct rf_directory_walker;

/* Convience type alias for pointer to the incomplete type
 * `rf_directory_walker`, to be used outside of filesystem implementation. */
typedef struct rf_directory_walker* rf_directory_walker_t;

/* Information about an entry in a directory. */
struct rf_directory_entry {
  /* Path to the entry. */
  struct rf_path path;

  /* Variant of an entry. */
  enum rf_directory_entry_variant {
    /* File. */
    RF_FILE_DIRECTORY_ENTRY,
    /* Subdirectory. */
    RF_SUBDIRECTORY_DIRECTORY_ENTRY,
    /* Not a file or subdirectory. */
    RF_OTHER_DIRECTORY_ENTRY,
  }
  /* Variant of the entry. */
  variant;
};

/* Convenience macro that calls `rf_test_unit` with the name of the given
 * function. */
#define RF_TEST_UNIT(tested_unit) rf_test_unit(tested_unit, #tested_unit)

/* Convience macro that calls `rf_allocate` with the file and line information
 * of the caller. */
#define RF_ALLOCATE(target, size) \
  rf_allocate((void**)target, size, __FILE__, __LINE__)
/* Convenience macro that calls `RF_ALLOCATE` by multiplying the given number of
 * elements with the size of the given element type. */
#define RF_ALLOCATE_ARRAY(target, element_count, element_type) \
  RF_ALLOCATE(target, element_count * sizeof(element_type))
/* Convience macro that calls `rf_reallocate` with the file and line information
 * of the caller. */
#define RF_REALLOCATE(target, new_size) \
  rf_reallocate((void**)target, new_size, __FILE__, __LINE__)
/* Convenience macro that calls `RF_REALLOCATE` by multiplying the given new
 * number of elements with the size of the given element type. */
#define RF_REALLOCATE_ARRAY(target, new_element_count, element_type) \
  RF_REALLOCATE(target, new_element_count * sizeof(element_type))
/* Convience macro that calls `rf_free` with the file and line information
 * of the caller. */
#define RF_FREE(target, size) rf_free((void**)target, size, __FILE__, __LINE__)
/* Convenience macro that calls `RF_FREE` by multiplying the given number of
 * elements with the size of the given element type. */
#define RF_FREE_ARRAY(target, element_count, element_type) \
  RF_FREE(target, element_count * sizeof(element_type))

/* Runs the given unit test and reports the given name if the test fails or
 * takes too long to run. */
void rf_test_unit(unit_test_t tested_unit, char const* reported_name);
/* Starts the timer for all the tests and resets counters for passed and failed
 * unit tests. */
void rf_begin_tests(void);
/* Reports all the tests results and returns whether all the tests passed. */
bool rf_report_tests(void);

/* Allocates the given target block to a new memory block with the given amount
 * of memory. Returns whether the operation failed. Sets `errno` on failure.
 * Reports any memory management errors with the given file and line information
 * of the caller. */
bool rf_allocate(void** target, size_t size, char const* file, unsigned line);
/* Changes the size of the given target block to the given size. Grows or
 * shrinks the existing block inplace, or copies the elements to a new location.
 * Returns whether the operation failed. Sets `errno` on failure. Reports any
 * memory management errors with the given file and line information of the
 * caller. */
bool rf_reallocate(void** target, size_t new_size, char const* file,
  unsigned line);
/* Discards the memory resource used by the given block with the given size, and
 * sets it to a null pointer. Reports any memory management errors with the
 * given file and line information of the caller. */
void rf_free(void** target, size_t size, char const* file, unsigned line);
/* Frees the resources used by the allocator and reports the correctness and
 * performance statistics about the allocations. Might not do anything if the
 * allocations were not recorded. */
void rf_finalize_allocations(void);

/* Creates a path by copying the given string and adding a null-termination
 * character after it. */
struct rf_path rf_convert_to_path(struct rf_string converted);
/* Creates a path by joining the given parts. There must be at least two parts.
 * The last two parts must be the file name and extension, and one of them can
 * be an empty string. Before them, there could be zero or more directory names,
 * which cannot be empty strings. */
struct rf_path rf_join_path(int parts_count,
  struct rf_string const        parts[static parts_count]);
/* Frees the memory resources used by the given path. */
void           rf_free_path(struct rf_path* freed);

/* Changes the current working directory to the given path. */
void rf_change_working_directory(struct rf_path path);
/* Creates a new directory at the given path. Would accept the already existing
 * directory at the given path if the flag is true. */
void rf_create_directory(struct rf_path path, bool accept_existing);

/* Creates a new directory walker for the directory at the given path. */
rf_directory_walker_t rf_walk_directory(struct rf_path path);
/* Frees the memory resources used by the given directory walker. */
void                  rf_free_directory_walker(rf_directory_walker_t* freed);
/* Returns whether there is an entry that is not walked in the directory and if
 * true, sets it to the given target. */
bool                  rf_walk_file(rf_directory_walker_t walker,
                   struct rf_directory_entry*            target);

/* Opens the file at the given path with the given mode and returns the
 * resulting stream. */
FILE* rf_open_file(struct rf_path path, char const* mode);
/* Closes the given stream. */
void  rf_close_file(FILE** closed);

/* Run all the tests for the tester module. */
void rf_test_tester(void);

#endif // RF_INTERNAL
