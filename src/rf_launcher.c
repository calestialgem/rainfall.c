#include "rf_launcher.h"

#include "rf_filesystem.h"

#include <stdio.h>
#include <string.h>

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

void rf_launch(struct rf_launch_command launched) {
  int error = 0;
  switch (launched.variant) {
  case RF_LAUNCH_COMMAND_NEW:
    error = rf_create_directory(launched.as_new.created_name);
    if (error != 0) {
      fprintf(stderr,
        "failure: Cannot create package `%.*s`!\n"
        "cause: %s\n",
        (int)launched.as_new.created_name.count,
        launched.as_new.created_name.array, strerror(error));
      return;
    }
    break;
  case RF_LAUNCH_COMMAND_CHECK:
  case RF_LAUNCH_COMMAND_TEST:
  case RF_LAUNCH_COMMAND_BUILD:
  case RF_LAUNCH_COMMAND_RUN:
    fputs("failure: Not implemented yet!", stderr);
    break;
  }
}
