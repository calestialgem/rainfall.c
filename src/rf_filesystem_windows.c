#include "rf_filesystem.h"

#include <direct.h>
#include <stdlib.h>
#include <string.h>

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

void rf_change_working_directory(struct rf_string path) {
  // First, convert the string to a null-terminated character array. One extra
  // capacity is for the null termination character.
  char path_as_null_terminated[path.count + 1];
  memcpy(path_as_null_terminated, path.array, path.count);
  path_as_null_terminated[path.count] = 0;

  // Change the current working directory to the given path.
  _chdir(path_as_null_terminated);
}
