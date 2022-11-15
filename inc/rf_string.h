/* Represents strings and provides functions for manipulating them. */
#ifndef RF_STRING_H
#define RF_STRING_H 1

#include <stddef.h>

/* Immutable view of linearly stored group of characters. The string is NOT
 * guaranteed to be null-terminated. */
struct rf_string {
  /* Immutable pointer to the first character in the string if it exists. Can be
   * an invalid pointer if the count is 0. */
  char const* array;
  /* Number of characters in the string. */
  size_t      count;
};

#endif // RF_STRING_H
