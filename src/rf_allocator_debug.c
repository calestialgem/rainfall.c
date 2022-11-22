#include "rf_allocator.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Recorded information about an allocation. */
struct allocation {
  /* Location of the allocation. */
  void*       block;
  /* Name of the file the allocation was done in. */
  char const* file;
  /* Number of the line the allocation was done at. */
  unsigned    line;
  /* Number of times the allocation was reallocated with a different size. */
  unsigned    reallocation_count;
  /* Number of times the allocation was copied to a different location to
   * facilitate reallocation. */
  unsigned    relocation_count;
  /* Whether the allocation is already freed. */
  bool        is_freed;
};

enum {
  /* Maximum number of allocations that could be contained in the list of
     allocations. */
  ALLOCATION_LIST_LIMIT = 8
};

struct {
  /* Staticly stored array of elements in the list. */
  struct allocation elements[ALLOCATION_LIST_LIMIT];
  /* Number of elements that are in the list. */
  size_t            count;
}
/* Staticly stored list of allocations. */
allocation_list;

/* Returns the pointer to the allocation with the given block. Returns null
 * pointer when there is not an allocation with the given block. */
static struct allocation* find_allocation(void* found_block);
/* Inserts the given allocation to the end of the list. Aborts if there are no
 * space left in the list. */
static void               push_allocation(struct allocation pushed);

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

bool rf_allocate(void** target, size_t amount, char const* file,
  unsigned line) {
  // Try reallocating and report error if it fails.
  void* allocated_block = realloc(*target, amount);
  if (allocated_block == NULL) { return true; }

  // Find the recorded information about the target block, and update it. Create
  // new information if its not there.
  struct allocation* previous = find_allocation(*target);
  if (previous != NULL) {
    previous->reallocation_count++;
    previous->relocation_count += previous->block != allocated_block;
    previous->block = allocated_block;
  } else {
    push_allocation((struct allocation){.block = allocated_block,
      .file                                    = file,
      .line                                    = line,
      .is_freed                                = false,
      .reallocation_count                      = 0,
      .relocation_count                        = 0});
  }

  // Set the new block and report success.
  *target = allocated_block;
  return false;
}

void rf_free(void** target, char const* file, unsigned line) {
  // Get the information on the target block.
  struct allocation* previous = find_allocation(*target);

  // If there is no record of it, report and abort.
  if (previous == NULL) {
    fprintf(stderr,
      "failure: Trying to free a block that was not allocated at %s:%u!", file,
      line);
    abort();
  }

  // If it is already freed, report and abort.
  if (previous->is_freed) {
    fprintf(stderr,
      "failure: Trying to free a block that was already freed at %s:%u!", file,
      line);
    abort();
  }

  // Mark it as freed, free it and clear it.
  previous->is_freed = true;
  free(*target);
  *target = NULL;
}

void rf_report_allocations(void) {
  puts("\nAllocations:");
  for (size_t i = 0; i < allocation_list.count; i++) {
    struct allocation allocation = allocation_list.elements[i];
    printf("[%zu] %s:%u: %u (%u)%s\n", i, allocation.file, allocation.line,
      allocation.reallocation_count, allocation.relocation_count,
      allocation.is_freed ? "" : "LEAKED");
  }
  putc('\n', stdout);
}

// ===================================================
//    [+]   P R I V A T E   F U N C T I O N S   [+]
// ===================================================

static struct allocation* find_allocation(void* found_block) {
  for (size_t i = 0; i < allocation_list.count; i++) {
    if (allocation_list.elements[i].block == found_block) {
      return allocation_list.elements + i;
    }
  }
  return NULL;
}

static void push_allocation(struct allocation pushed) {
  if (allocation_list.count == ALLOCATION_LIST_LIMIT) {
    fprintf(stderr,
      "failure: There are not any more space to track the allocation at "
      "%s:%u!\n",
      pushed.file, pushed.line);
    abort();
  }
  allocation_list.elements[allocation_list.count++] = pushed;
}
