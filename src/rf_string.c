#include "rf_string.h"

#include <string.h>

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

struct rf_string rf_view_null_terminated(char const* viewed) {
  return (struct rf_string){.array = viewed, .count = strlen(viewed)};
}

bool rf_compare_strings(struct rf_string this, struct rf_string that) {
  if (this.count != that.count) { return false; }
  if (this.array == that.array) { return true; }
  for (size_t i = 0; i < this.count; i++) {
    if (this.array[i] != that.array[i]) { return false; }
  }
  return true;
}

bool rf_compare_prefix(struct rf_string compared, struct rf_string prefix) {
  if (compared.count < prefix.count) { return false; }
  if (compared.array == prefix.array) { return true; }
  for (size_t i = 0; i < prefix.count; i++) {
    if (compared.array[i] != prefix.array[i]) { return false; }
  }
  return true;
}

void rf_skip_prefix(struct rf_string* skipped, size_t amount) {
  skipped->array += amount;
  skipped->count -= amount;
}
