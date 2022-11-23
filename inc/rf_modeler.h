/* Represents the entire workspace with a single type of model. */
#ifndef RF_MODELER_H
#define RF_MODELER_H 1

#include "rf_string.h"

#include <stddef.h>

enum {
  /* Maximum number of source files that could be modeled by the compiler. */
  RF_SOURCE_MODELS_LIMIT = 1024,
};

/* A node in the workspace forest. Might be a source or module. Also it is a
 * package if its at the top level. */
struct rf_workspace_node {
  /* Name of the source or module. */
  struct rf_string name;

  union {
    /* Identifier of the source model. */
    int model_id;
    /* Number of sources and submodules that are directly under the module. */
    int children_count;
  };

  /* Variant of the workspace node. */
  enum rf_workspace_node_variant {
    /* Node as source. */
    RF_SOURCE,
    /* Node as module. */
    RF_MODULE,
  }
  /* Variant of the node. */
  variant;
};

/* Forest of sources and modules, where the roots are packages. The children
 * come before the parent. Modules are branches, while the sources are
 * leaves. */
struct rf_workspace {
  /* Pointer to the first element of the array of nodes. */
  struct rf_workspace_node* array;
  /* Number of nodes stored in the array. */
  size_t                    count;
  /* Number of allocated node space in the array. */
  size_t                    limit;
};

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Adds the given node to the end of the given workspace. Returns whether the
 * operation failed. Sets `errno` on failure. */
bool rf_push_workspace_node(struct rf_workspace* target,
  struct rf_workspace_node                       pushed);
/* Frees the resources owned by the given workspace. */
void rf_free_workspace(struct rf_workspace* freed);

#endif // RF_MODELER_H
