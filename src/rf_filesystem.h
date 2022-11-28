/* Abstracts away the communication with the filesystem, such that the compiler
 * can be run on different platforms. */
#ifndef RF_FILESYSTEM_H
#define RF_FILESYSTEM_H 1

#include "rf_string.h"

#include <stdbool.h>
#include <stdio.h>

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

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Create a path by joining the given parts. There must be at least two parts.
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

#endif // RF_FILESYSTEM_H
