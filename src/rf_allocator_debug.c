#include "rf_allocator.h"
#include "rf_string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Location in the project source files. */
struct location {
  /* Path of the source file the allocation was done in relative to the
   * project's source directory. */
  struct rf_string path;
  /* Number of the line the allocation was done at. */
  unsigned         line;
};

/* Recorded information about an allocation. */
struct allocation {
  /* Location of the allocation. */
  void*           block;
  /* Current size of the block. */
  size_t          size;
  /* Maximum size ever reached by the block. */
  size_t          max_size;
  /* Location the block is allocated at. */
  struct location allocated_at;
  /* Location the block is freed at. */
  struct location freed_at;
  /* Number of times the allocation was reallocated with a different size. */
  unsigned        times_grown;
  /* Number of times the allocation was copied to a different location to
   * facilitate reallocation. */
  unsigned        times_moved;
  /* Whether the allocation is already freed. */
  bool            is_freed;
};

static struct {
  /* Pointer to the first allocation if it exists. */
  struct allocation* array;
  /* Number of elements that are in the list. */
  size_t             count;
  /* Maximum number of elements that fit to the array. */
  size_t             limit;
}
/* Staticly stored list of allocations. */
allocation_list;

/* Reduces the path relative to the project's source directory. */
static struct rf_string   reduce_path(char const* file);
/* Returns the pointer to the allocation with the given block. Returns null
 * pointer when there is not an allocation with the given block. */
static struct allocation* find_allocation(void* found_block);
/* Inserts the given allocation to the end of the list. Aborts if there are no
 * space left in the list. */
static void               push_allocation(struct allocation pushed);

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

bool rf_allocate(void** target, size_t size, char const* file, unsigned line) {
  // Get the information on the target block.
  struct allocation* previous = find_allocation(*target);

  struct rf_string path = reduce_path(file);

  if (previous != NULL) {
    fprintf(stderr,
      "%.*s:%u: failure: Trying to allocate a block that is already "
      "allocated!\n"
      "%.*s:%u: info: Previously block was allocated here.\n.",
      (int)path.count, path.array, line, (int)previous->allocated_at.path.count,
      previous->allocated_at.path.array, previous->allocated_at.line);
    abort();
  }

  // Try allocating and report error if it fails.
  void* allocated_block = malloc(size);
  if (allocated_block == NULL) { return true; }

  // Record the allocation.
  push_allocation((struct allocation){
    .block        = allocated_block,
    .size         = size,
    .max_size     = size,
    .allocated_at = {.path = path, .line = line}
  });

  // Set the new block and report success.
  *target = allocated_block;
  return false;
}

bool rf_reallocate(void** target, size_t new_size, char const* file,
  unsigned line) {
  // Get the information on the target block.
  struct allocation* previous = find_allocation(*target);

  struct rf_string path = reduce_path(file);

  // If there is no record of it, report and abort.
  if (previous == NULL) {
    fprintf(stderr,
      "%.*s:%u: failure: Trying to reallocate a block that was not "
      "allocated!\n",
      (int)path.count, path.array, line);
    abort();
  }

  // If the block is already freed, report and abort.
  if (previous->is_freed) {
    fprintf(stderr,
      "%.*s:%u: failure: Trying to reallocate a block that was freed!\n",
      (int)path.count, path.array, line);
    abort();
  }

  // Try reallocating and report error if it fails.
  void* reallocated_block = realloc(*target, new_size);
  if (reallocated_block == NULL) { return true; }

  // Update the records.
  previous->times_grown++;
  previous->times_moved += previous->block != reallocated_block;
  previous->block    = reallocated_block;
  previous->size     = new_size;
  previous->max_size = max(previous->max_size, new_size);

  // Set the might be relocated block and report success.
  *target = reallocated_block;
  return false;
}

void rf_free(void** target, size_t size, char const* file, unsigned line) {
  // If the target is null, return.
  if (*target == NULL) { return; }

  // Get the information on the target block.
  struct allocation* previous = find_allocation(*target);

  struct rf_string path = reduce_path(file);

  // If there is no record of it, report and abort.
  if (previous == NULL) {
    fprintf(stderr,
      "%.*s:%u: failure: Trying to free a block that was not allocated!\n",
      (int)path.count, path.array, line);
    abort();
  }

  // If it is already freed, report and abort.
  if (previous->is_freed) {
    fprintf(stderr,
      "%.*s:%u: failure: Trying to free a block that was already freed!\n"
      "%.*s:%u: info: Previously block was freed here.",
      (int)path.count, path.array, line, (int)previous->freed_at.path.count,
      previous->freed_at.path.array, previous->freed_at.line);
    abort();
  }

  // If the sizes do not match, report and abort.
  if (previous->size != size) {
    fprintf(stderr,
      "%.*s:%u: failure: Trying to free a block with size %zu as size %zu!\n",
      (int)path.count, path.array, line, previous->size, size);
    abort();
  }

  // Mark it as freed, free it and clear it.
  previous->is_freed      = true;
  previous->size          = 0;
  previous->freed_at.path = path;
  previous->freed_at.line = line;
  free(*target);
  *target = NULL;
}

void rf_finalize_allocations(void) {
  puts("\nAllocations:");
  for (size_t i = 0; i < allocation_list.count; i++) {
    struct allocation allocation = allocation_list.array[i];

    printf("%.*s:%u: max %zu grown %u moved %u",
      (int)allocation.allocated_at.path.count,
      allocation.allocated_at.path.array, allocation.allocated_at.line,
      allocation.max_size, allocation.times_grown, allocation.times_moved);

    if (!allocation.is_freed) {
      printf(" LEAKED %zu", allocation.size);
      free(allocation.block);
    }
    putc('\n', stdout);
  }
  putc('\n', stdout);
  free(allocation_list.array);
  allocation_list.array = NULL;
  allocation_list.count = 0;
  allocation_list.limit = 0;
}

// ===================================================
//    [+]   P R I V A T E   F U N C T I O N S   [+]
// ===================================================

static struct rf_string reduce_path(char const* file) {
  // Remove the parent directories upto the source folder.
  struct rf_string path = rf_view_null_terminated(file);
  rf_skip_prefix(&path,
    rf_find_last_occurrence(path, rf_view_null_terminated("/src/")));
  return path;
}

static struct allocation* find_allocation(void* found_block) {
  for (size_t i = 0; i < allocation_list.count; i++) {
    if (allocation_list.array[i].block == found_block) {
      return allocation_list.array + i;
    }
  }
  return NULL;
}

static void push_allocation(struct allocation pushed) {
  // Check whether the list has space for the allocation, grow if needed.
  if (allocation_list.count == allocation_list.limit) {
    if (allocation_list.limit == 0) {
      // Start allocating with a limit for a single allocation.
      allocation_list.limit = 1;
      allocation_list.array =
        malloc(allocation_list.limit * sizeof(struct allocation));
    } else {
      // Amortize allocations by growing limit to its double.
      allocation_list.limit *= 2;
      allocation_list.array = realloc(allocation_list.array,
        allocation_list.limit * sizeof(struct allocation));
    }

    // Check the growth.
    if (allocation_list.array == NULL) {
      fprintf(stderr,
        "%.*s:%u: failure: Cannot allocate memory to track allocation!\n",
        (int)pushed.allocated_at.path.count, pushed.allocated_at.path.array,
        pushed.allocated_at.line);
      abort();
    }
  }

  // Put the allocation to end.
  allocation_list.array[allocation_list.count++] = pushed;
}
