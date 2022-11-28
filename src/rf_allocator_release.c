#include "rainfall.h"
#include "rf_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

bool rf_allocate(void** target, size_t size, char const*, unsigned) {
  // Try allocating and report error if it fails.
  void* allocated_block = malloc(size);
  if (allocated_block == NULL) { return true; }

  // Set the new block and report success.
  *target = allocated_block;
  return false;
}

bool rf_reallocate(void** target, size_t new_size, char const*, unsigned) {
  // Try reallocating and report error if it fails.
  void* reallocated_block = realloc(*target, new_size);
  if (reallocated_block == NULL) { return true; }

  // Set the might be relocated block and report success.
  *target = reallocated_block;
  return false;
}

void rf_free(void** target, size_t, char const*, unsigned) {
  // If the target is null, return.
  if (*target == NULL) { return; }

  // Free it and clear it.
  free(*target);
  *target = NULL;
}

void rf_finalize_allocations(void) {
  // Release allocator does not record or check allocations for maximum
  // performance.
}
