/* Provides null-terminated, dynamicly allocated, linearly stored buffers. */
#ifndef RF_BUFFER_H
#define RF_BUFFER_H 1

#include "rf_string.h"

#include <stddef.h>

/* Null-terminated, linearly stored group of UTF-8 encoded characters. */
struct rf_buffer {
  /* Pointer to the first character in the buffer if it exists. Otherwise points
   * to the null-termination character. */
  char*  array;
  /* Number of characters in the buffer excluding the null-termination
   * character. */
  size_t count;
  /* Maximum amount of characters that fits into the array excluding the
   * null-termination character. */
  size_t limit;
};

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Creates a buffer with just a null-termination character. */
struct rf_buffer rf_create_buffer(void);
/* Frees the memory resources used by the given buffer. */
void             rf_free_buffer(struct rf_buffer* freed);
/* Appends the given UTF-8 encoded character to the end of the given buffer. */
void rf_append_character(struct rf_buffer* destination, int appended);
/* Appends the given string to the end of the given buffer. */
void rf_append_string(struct rf_buffer* destination, struct rf_string appended);

#endif // RF_BUFFER_H
