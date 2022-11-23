/* Loads the module directories and source files into memory as linear
 * collection of characters. Thus, linearly models the sources. */
#ifndef RF_LOADER_H
#define RF_LOADER_H 1

#include "rf_modeler.h"
#include "rf_status.h"
#include "rf_string.h"

/* Model of a source file as an ordered collection of characters. */
struct rf_linear_model {
  /* Immutable view to the characters in the file. */
  struct rf_string contents;
};

/* Helper that loads the contents of source files into memory. */
struct rf_loader {
  struct {
    /* Pointer to the first character of the first loaded file if it exists. */
    char*  array;
    /* Number of characters in the array. */
    size_t count;
    /* Maximum number of characters that could fit in the array. */
    size_t limit;
  }
  /* Combined contents of all the loaded files. */
  file_contents_buffer;

  struct {
    /* Fixed array of models. */
    struct rf_linear_model array[RF_SOURCE_MODELS_LIMIT];
    /* Number of models in the array. */
    int                    count;
  }
  /* Linear models of the sources. */
  models;
};

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Loads the workspace using the given loader to the given target. */
enum rf_status rf_load_workspace(struct rf_loader* loader,
  struct rf_workspace*                             target);
/* Frees the memory resources owned by the given loader. */
void           rf_free_loader(struct rf_loader* freed);

#endif // RF_LOADER_H
