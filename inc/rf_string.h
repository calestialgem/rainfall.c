/* Represents strings and provides functions for manipulating them. */
#ifndef RF_STRING_H
#define RF_STRING_H 1

#include <stdbool.h>
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

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Returns the view of the given null-terminated array as a string. */
struct rf_string rf_view_null_terminated(char const* viewed);
/* Returns whether the characters of the given strings are the same and ordered
 * the same way. */
bool rf_compare_strings(struct rf_string this, struct rf_string that);
/* Finds whether the given string has the given prefix at its start. */
bool rf_compare_prefix(struct rf_string compared, struct rf_string prefix);
/* Removes the given amount of characters from the start of the given string. */
void rf_skip_prefix(struct rf_string* skipped, size_t amount);

#endif // RF_STRING_H
