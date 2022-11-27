#include "rf_loader.h"

#include "rf_allocator.h"

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

enum rf_status rf_load_workspace(struct rf_loader* loader,
  struct rf_workspace*                             target) {}

void rf_free_loader(struct rf_loader* freed) {
  // Free the file contents.
  RF_FREE(&freed->file_contents_buffer.array);
  freed->file_contents_buffer.count = 0;
  freed->file_contents_buffer.limit = 0;

  // Free the models.
  RF_FREE(&freed->models.array);
  freed->models.count = 0;
  freed->models.limit = 0;
}
