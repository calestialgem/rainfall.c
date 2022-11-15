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

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

// =====================================================
//    ***   Q U A L I T Y   A S S E S S M E N T   ***
// =====================================================

/* Run all the tests for the string module. */
void rf_string_test(void);

#endif // RF_STRING_H
