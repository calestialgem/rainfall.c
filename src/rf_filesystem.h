/* Abstracts away the communication with the filesystem, such that the compiler
 * can be run on different platforms. */
#ifndef RF_FILESYSTEM_H
#define RF_FILESYSTEM_H 1

#include "rf_string.h"

#include <stdbool.h>
#include <stdio.h>

/* Information on a file with a stream to it. */
struct rf_file {
  /* Null-terminated array of characters that holds the path to the file. Might
   * be relative or absolute. */
  char* path;
  /* Owned stream to and from the file. The stream might be opened with any
   * mode. */
  FILE* stream;

  /* Variant of a file. */
  enum rf_file_variant {
    /* A file that might be read from or written to as bytes. */
    RF_FILE_DATA,
    /* A file that contains other files. */
    RF_FILE_DIRECTORY,
    /* Any other form of file. */
    RF_FILE_OTHER,
  }
  /* Variant of the file. */
  variant;
};

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Changes the current working directory to the given path. Returns whether the
 * operation was not successful. Populates the `errno` on failure. */
bool rf_change_working_directory(struct rf_string path);
/* Creates a new directory at the given path. Returns whether the
 * operation was not successful. Populates the `errno` on failure. */
bool rf_create_directory(struct rf_string path);
/* Opens the file at the path that is created by joining the given part strings,
 * whose count must be given, with the given mode and writes the information on
 * the file to the given target. The parts must end with the file name and file
 * extension, which might be empty strings. Parts can start with zero or more of
 * directory names, which cannot be empty strings. Returns whether the operation
 * was not successful. Populates the `errno` on failure. */
bool rf_open_file(struct rf_file* target, char const* mode, int part_count,
  ...);
/* Closes the given file and sets its pointers to `NULL`. Might fail; for
 * example, because closing a file flushes the buffered output. Returns whether
 * the operation was not successful. Populates the `errno` on failure. */
bool rf_close_file(struct rf_file* target);

#endif // RF_FILESYSTEM_H
