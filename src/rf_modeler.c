#include "rainfall.h"
#include "rf_internal.h"

#include <stdlib.h>

/* Ensures that the given amount of space for nodes exists at the end of the
 * given workspace forest. Does amortized growth if necessary. Returns whether
 * the operation failed. Sets `errno` on failure. */
static bool ensure_nodes(struct rf_workspace* target, size_t amount);

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

bool rf_push_workspace_node(struct rf_workspace* target,
  struct rf_workspace_node                       pushed) {
  if (ensure_nodes(target, 1)) { return true; }
  target->array[target->count++] = pushed;
  return false;
}

void rf_free_workspace(struct rf_workspace* freed) {
  RF_FREE_ARRAY(&freed->array, freed->limit, struct rf_workspace_node);
  freed->count = 0;
  freed->limit = 0;
}

// ===================================================
//    [+]   P R I V A T E   F U N C T I O N S   [+]
// ===================================================

static bool ensure_nodes(struct rf_workspace* target, size_t amount) {
  // Check whether there is a need for growth.
  size_t space = target->limit - target->count;
  if (space >= amount) { return false; }

  // If the list did not exist previously, allocate it.
  if (target->array == NULL) {
    if (RF_ALLOCATE_ARRAY(&target->array, amount, struct rf_workspace_node)) {
      return true;
    }
    target->limit = amount;
    return false;
  }

  // Grow at least by the half of the current limit to amortize allocations.
  size_t new_limit = target->limit + max(amount, target->limit / 2);
  if (RF_REALLOCATE_ARRAY(&target->array, new_limit,
        struct rf_workspace_node)) {
    return true;
  }
  target->limit = new_limit;
  return false;
}
