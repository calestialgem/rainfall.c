#include "rf_filesystem.h"

#include <direct.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Converts the given string to a local null-terminated array. */
#define AS_NULL_TERMINATED(string)                                 \
  char string##_as_null_terminated[string.count + 1];              \
  memcpy(string##_as_null_terminated, string.array, string.count); \
  string##_as_null_terminated[string.count] = 0;

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

enum rf_filesystem_result rf_change_working_directory(struct rf_string path) {
  AS_NULL_TERMINATED(path);

  if (_chdir(path_as_null_terminated) != 0) {
    switch (errno) {
    case ENOENT: return RF_FILESYSTEM_NONEXISTING_PATH;
    default:
      fprintf(stderr, "Unknown error code: %d!\n", errno);
      return RF_FILESYSTEM_UNKNOWN_ERROR;
    }
  }

  return RF_FILESYSTEM_SUCCESSFUL;
}

enum rf_filesystem_result rf_create_directory(struct rf_string path) {
  AS_NULL_TERMINATED(path);

  if (_mkdir(path_as_null_terminated) != 0) {
    switch (errno) {
    case ENOENT: return RF_FILESYSTEM_INVALID_PATH;
    case EEXIST: return RF_FILESYSTEM_EXISTING_PATH;
    default:
      fprintf(stderr, "Unknown error code: %d!\n", errno);
      return RF_FILESYSTEM_UNKNOWN_ERROR;
    }
  }

  return RF_FILESYSTEM_SUCCESSFUL;
}
