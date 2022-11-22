/* Manages memory allocation in the compiler. */
#ifndef RF_ALLOCATOR_H
#define RF_ALLOCATOR_H

#include <stdbool.h>
#include <stddef.h>

/* Convenience macro that calls `rf_allocate` by multiplying the given amount of
 * elements with the size of the given element type. */
#define RF_ALLOCATE_ARRAY(target, element_amount, element_type) \
  rf_allocate(target, element_amount * sizeof(element_type))
/* Convience macro that calls `rf_allocate` with the file and line information
 * of the caller. */
#define RF_ALLOCATE(target, amount) \
  rf_allocate(target, amount, __FILE__, __LINE__)
/* Convience macro that calls `rf_free` with the file and line information
 * of the caller. */
#define RF_FREE(target) rf_free(target, __FILE__, __LINE__)

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Allocates the given target block to a new memory block with the given amount
 * of memory. If the target is already pointing to an allocated block, grows the
 * existing block or copies the elements to a new location. Returns whether the
 * operation failed. Sets `errno` on failure. Reports any memory management
 * errors with the given file and line information of the caller. */
bool rf_allocate(void** target, size_t amount, char const* file, unsigned line);
/* Discards the memory resource used by the given block, and sets it to a null
 * pointer. Reports any memory management errors with the given file and line
 * information of the caller. */
void rf_free(void** target, char const* file, unsigned line);
/* Reports the correctness and performance statistics about the allocations.
 * Might not do anything if the allocations were not recorded. */
void rf_report_allocations(void);

#endif // RF_ALLOCATOR_H
