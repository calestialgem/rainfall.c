/* Manages memory allocation in the compiler. */
#ifndef RF_ALLOCATOR_H
#define RF_ALLOCATOR_H 1

#include <stdbool.h>
#include <stddef.h>

/* Convience macro that calls `rf_allocate` with the file and line information
 * of the caller. */
#define RF_ALLOCATE(target, size) \
  rf_allocate((void**)target, size, __FILE__, __LINE__)
/* Convenience macro that calls `RF_ALLOCATE` by multiplying the given number of
 * elements with the size of the given element type. */
#define RF_ALLOCATE_ARRAY(target, element_count, element_type) \
  RF_ALLOCATE(target, element_count * sizeof(element_type))
/* Convience macro that calls `rf_reallocate` with the file and line information
 * of the caller. */
#define RF_REALLOCATE(target, new_size) \
  rf_reallocate((void**)target, new_size, __FILE__, __LINE__)
/* Convenience macro that calls `RF_REALLOCATE` by multiplying the given new
 * number of elements with the size of the given element type. */
#define RF_REALLOCATE_ARRAY(target, new_element_count, element_type) \
  RF_REALLOCATE(target, new_element_count * sizeof(element_type))
/* Convience macro that calls `rf_free` with the file and line information
 * of the caller. */
#define RF_FREE(target, size) rf_free((void**)target, size, __FILE__, __LINE__)
/* Convenience macro that calls `RF_FREE` by multiplying the given number of
 * elements with the size of the given element type. */
#define RF_FREE_ARRAY(target, element_count, element_type) \
  RF_FREE(target, element_count * sizeof(element_type))

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Allocates the given target block to a new memory block with the given amount
 * of memory. Returns whether the operation failed. Sets `errno` on failure.
 * Reports any memory management errors with the given file and line information
 * of the caller. */
bool rf_allocate(void** target, size_t size, char const* file, unsigned line);
/* Changes the size of the given target block to the given size. Grows or
 * shrinks the existing block inplace, or copies the elements to a new location.
 * Returns whether the operation failed. Sets `errno` on failure. Reports any
 * memory management errors with the given file and line information of the
 * caller. */
bool rf_reallocate(void** target, size_t new_size, char const* file,
  unsigned line);
/* Discards the memory resource used by the given block with the given size, and
 * sets it to a null pointer. Reports any memory management errors with the
 * given file and line information of the caller. */
void rf_free(void** target, size_t size, char const* file, unsigned line);
/* Frees the resources used by the allocator and reports the correctness and
 * performance statistics about the allocations. Might not do anything if the
 * allocations were not recorded. */
void rf_finalize_allocations(void);

#endif // RF_ALLOCATOR_H
