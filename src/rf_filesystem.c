#include "rf_filesystem.h"

#include "rf_string.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compile the implementation for the correct platform.
#ifdef _WIN32
  #include "rf_filesystem_windows.c"
#else
  // No provided implementation fits for the current platform; thus, compiler
  // cannot be compiled.
  #error Filesystem only supports the fallowing platforms: Windows.
#endif

/* Writes the given string to the given buffer at the given index. Moves the
 * index to the position just after the written string. */
#define WRITE_STRING(buffer, index, string)             \
  do {                                                  \
    memcpy(buffer + index, string.array, string.count); \
    index += string.count;                              \
  } while (false)

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

bool rf_open_file(struct rf_file* target, char const* mode,
  char const* extension, int part_count, ...) {
  // Convert variable length part array to a real array.
  va_list variable_length_string_array;
  va_start(variable_length_string_array, part_count);
  struct rf_string parts[part_count];
  for (int i = 0; i < part_count; i++) {
    parts[i] = va_arg(variable_length_string_array, struct rf_string);
  }
  va_end(variable_length_string_array);

  // Get the total length of parts, the extension and the directory separators.
  struct rf_string extension_as_string = rf_view_null_terminated(extension);
  size_t total_length = extension_as_string.count + max(0, part_count - 1);
  for (int i = 0; i < part_count; i++) { total_length += parts[i].count; }

  // Allocate an extra byte for the null-termination character.
  target->path = malloc(total_length + 1);

  // Write the parts, the directory separators, the extension and the
  // null-termination character.
  size_t current_write_index = 0;
  WRITE_STRING(target->path, current_write_index, parts[0]);
  for (int i = 1; i < part_count; i++) {
    target->path[current_write_index++] = '/';
    WRITE_STRING(target->path, current_write_index, parts[i]);
  }
  WRITE_STRING(target->path, current_write_index, extension_as_string);
  target->path[current_write_index] = 0;

  // Get the error code; as `fopen` does not guarantee to set `errno`, `fopen_s`
  // is used because it guarantees to return an error code, which could be
  // manually assigned to `errno`.
  int error_code = fopen_s(&target->stream, target->path, mode);
  if (error_code != 0) {
    errno = error_code;
    return true;
  }
  return false;
}

bool rf_close_file(struct rf_file* target) {
  free(target->path);
  target->path = NULL;

  // Close the file and return the error status.
  int status     = fclose(target->stream);
  target->stream = NULL;
  return status == EOF;
}
