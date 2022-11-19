#include "rf_filesystem.h"

#include <direct.h>
#include <errno.h>
#include <string.h>

/* Converts the given string to a local null-terminated array. */
#define AS_NULL_TERMINATED(string)                                 \
  char string##_as_null_terminated[string.count + 1];              \
  memcpy(string##_as_null_terminated, string.array, string.count); \
  string##_as_null_terminated[string.count] = 0;

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

bool rf_change_working_directory(struct rf_string path) {
  AS_NULL_TERMINATED(path);
  return _chdir(path_as_null_terminated);
}

bool rf_create_directory(struct rf_string path) {
  AS_NULL_TERMINATED(path);
  return _mkdir(path_as_null_terminated);
}
