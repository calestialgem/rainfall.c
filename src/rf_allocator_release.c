#include "rf_allocator.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

bool rf_allocate(void** target, size_t amount, char const*, unsigned) {
  // Try reallocating and report error if it fails.
  void* allocated_block = realloc(*target, amount);
  if (allocated_block == NULL) { return true; }

  // Set the new block and report success.
  *target = allocated_block;
  return false;
}

void rf_free(void** target, char const*, unsigned) {
  // Free the target and clear it.
  free(*target);
  *target = NULL;
}

void rf_report_allocations(void) {
  // Release allocator does not record or check allocations for maximum
  // performance.
}
