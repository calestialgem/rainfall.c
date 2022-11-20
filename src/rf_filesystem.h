/* Abstracts away the communication with the filesystem, such that the compiler
 * can be run on different platforms. */
#ifndef RF_FILESYSTEM_H
#define RF_FILESYSTEM_H 1

#include "rf_string.h"

#include <stdbool.h>
#include <stdio.h>

/* Combination of path and stream to a file. This is necessary, because the file
 * paths must be null-terminated. `rf_string`s are not null-terminated and they
 * might be required to be joined together to create a path. Thus, the path is
 * dynamicly allocated and it is managed together with the path. */
struct rf_file {
  /* Owned, null-terminated character array as the path to the file. The path
   * might be relative or absolute. */
  char* path;
  /* Owned stream to and from the file. The stream might be opened with any
   * mode. */
  FILE* stream;
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
 * whose count must be given, with directory separators in between and the given
 * extension juxtaposed at the end. Hence, the final path to the opened file
 * would be `<part1>/<part2>/.../<partN><extension>`. Writes its information to
 * the given target. Atleast a single part must be given and the number of
 * `rf_string`s must be equal to the given count. Returns whether the operation
 * was not successful. Populates the `errno` on failure. */
bool rf_open_file(struct rf_file* target, char const* mode,
  char const* extension, int part_count, ...);
/* Closes the given file and sets its pointers to `NULL`. Might fail; for
 * example, because closing a file flushes the buffered output. Returns whether
 * the operation was not successful. Populates the `errno` on failure. */
bool rf_close_file(struct rf_file* target);

#endif // RF_FILESYSTEM_H
