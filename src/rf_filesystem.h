/* Abstracts away the communication with the filesystem, such that the compiler
 * can be run on different platforms. */
#ifndef RF_FILESYSTEM_H
#define RF_FILESYSTEM_H 1

#include "rf_string.h"

/* Codes for errors that might arise in a filesystem operation. */
enum rf_filesystem_result {
  /* Operation was done without any errors. */
  RF_FILESYSTEM_SUCCESSFUL,
  /* An unknown error occurred. */
  RF_FILESYSTEM_UNKNOWN_ERROR,
  /* The given path already existed. */
  RF_FILESYSTEM_EXISTING_PATH,
  /* The given path did not exist. */
  RF_FILESYSTEM_NONEXISTING_PATH,
  /* The given path was invalid. */
  RF_FILESYSTEM_INVALID_PATH,
};

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Changes the current working directory to the given path. Returns the error
 * code of the operation. */
[[nodiscard]] enum rf_filesystem_result rf_change_working_directory(
  struct rf_string path);
/* Creates a new directory at the given path. Returns the error code of the
 * operation. */
[[nodiscard]] enum rf_filesystem_result rf_create_directory(
  struct rf_string path);

#endif // RF_FILESYSTEM_H
