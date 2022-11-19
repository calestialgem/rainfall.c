#include "rf_launcher.h"

#include "rf_filesystem.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

/* Creates a package with the given name. */
static void execute_new_command(struct rf_launch_command executed);

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

void rf_launch(struct rf_launch_command launched) {
  switch (launched.variant) {
  case RF_LAUNCH_COMMAND_NEW: execute_new_command(launched); break;
  case RF_LAUNCH_COMMAND_CHECK:
  case RF_LAUNCH_COMMAND_TEST:
  case RF_LAUNCH_COMMAND_BUILD:
  case RF_LAUNCH_COMMAND_RUN:
    fputs("failure: Not implemented yet!", stderr);
    break;
  }
}

static void execute_new_command(struct rf_launch_command executed) {
  if (rf_create_directory(executed.as_new.created_name)) {
    fprintf(stderr,
      "failure: Cannot create package `%.*s`!\n"
      "cause: %s\n",
      (int)executed.as_new.created_name.count,
      executed.as_new.created_name.array, strerror(errno));
    return;
  }
}
