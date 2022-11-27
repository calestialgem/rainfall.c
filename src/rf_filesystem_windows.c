#include "rf_allocator.h"
#include "rf_filesystem.h"
#include "rf_string.h"

#include <direct.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vadefs.h>

/* Converts the given string to a local null-terminated array. */
#define AS_NULL_TERMINATED(string)                                 \
  char string##_as_null_terminated[string.count + 1];              \
  memcpy(string##_as_null_terminated, string.array, string.count); \
  string##_as_null_terminated[string.count] = 0;

/* Writes the given string to the given buffer at the given index. Moves the
 * index to the position just after the written string. */
#define WRITE_STRING(buffer, index, string)               \
  do {                                                    \
    struct rf_string written = string;                    \
    memcpy(buffer + index, written.array, written.count); \
    index += written.count;                               \
  } while (false)

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

bool rf_open_file(struct rf_file* target, char const* mode, int part_count,
  ...) {
  va_list parts;

  // Count the parts.
  va_start(parts, part_count);
  size_t total_length = 0;

  // Add all the directory name lengths with the separators length.
  for (int i = 0; i < part_count - 2; i++) {
    total_length += va_arg(parts, struct rf_string).count + 1;
  }

  // Add the file name length.
  total_length += va_arg(parts, struct rf_string).count;

  // Add the extension separator and the extension if its there.
  size_t extension_count = va_arg(parts, struct rf_string).count;
  if (extension_count != 0) { total_length += 1 + extension_count; }
  va_end(parts);

  // Add the null-termination character.
  total_length++;

  // Allocate the path buffer.
  if (RF_ALLOCATE_ARRAY(&target->path, total_length, char)) {
    target->stream = NULL;
    return true;
  }

  // Write the parts.
  va_start(parts, part_count);
  size_t current_write_index = 0;

  // Write all the directory names and separators.
  for (int i = 0; i < part_count - 2; i++) {
    WRITE_STRING(target->path, current_write_index,
      va_arg(parts, struct rf_string));
    target->path[current_write_index++] = '\\';
  }

  // Write the file name.
  WRITE_STRING(target->path, current_write_index,
    va_arg(parts, struct rf_string));

  // Write the file extension separator and the file extension if its there.
  if (extension_count != 0) {
    target->path[current_write_index++] = '.';
    WRITE_STRING(target->path, current_write_index,
      va_arg(parts, struct rf_string));
  }
  va_end(parts);

  // Write the null-terminator.
  target->path[current_write_index] = 0;

  // Get the error code; as `fopen` does not guarantee to set `errno`, `fopen_s`
  // is used because it guarantees to return an error code, which could be
  // manually assigned to `errno`.
  int error_code = fopen_s(&target->stream, target->path, mode);
  if (error_code != 0) {
    RF_FREE_ARRAY(target->path, total_length, char);
    target->path   = NULL;
    target->stream = NULL;
    errno          = error_code;
    return true;
  }
  return false;
}

bool rf_close_file(struct rf_file* target) {
  RF_FREE_ARRAY(&target->path, rf_view_null_terminated(target->path).count + 1,
    char);

  // Close the file and return the error status.
  int status     = fclose(target->stream);
  target->stream = NULL;
  return status == EOF;
}
