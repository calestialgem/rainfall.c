#include "rainfall.h"
#include "rf_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

struct rf_string const RF_EMPTY_STRING = {.array = NULL, .count = 0};

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

struct rf_string rf_view_null_terminated(char const* viewed) {
  return (struct rf_string){.array = viewed, .count = strlen(viewed)};
}

bool rf_compare_strings(struct rf_string this, struct rf_string that) {
  if (this.count != that.count) { return false; }
  for (size_t i = 0; i < this.count; i++) {
    if (this.array[i] != that.array[i]) { return false; }
  }
  return true;
}

bool rf_compare_prefix(struct rf_string compared, struct rf_string prefix) {
  if (compared.count < prefix.count) { return false; }
  for (size_t i = 0; i < prefix.count; i++) {
    if (compared.array[i] != prefix.array[i]) { return false; }
  }
  return true;
}

bool rf_compare_suffix(struct rf_string compared, struct rf_string suffix) {
  if (compared.count < suffix.count) { return false; }
  for (size_t i = 0; i < suffix.count; i++) {
    if (compared.array[compared.count - suffix.count + i] != suffix.array[i]) {
      return false;
    }
  }
  return true;
}

void rf_skip_prefix(struct rf_string* skipped, size_t amount) {
  skipped->array += amount;
  skipped->count -= amount;
}

size_t rf_find_last_occurrence(struct rf_string source,
  struct rf_string                              found) {
  // Test suffix after removing the last character one by one.
  while (source.count >= found.count && !rf_compare_suffix(source, found)) {
    source.count--;
  }
  return source.count;
}
