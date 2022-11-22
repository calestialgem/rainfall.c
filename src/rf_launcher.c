#include "rf_launcher.h"

#include "rf_filesystem.h"
#include "rf_string.h"

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
  case RF_LAUNCH_COMMAND_CHECK: [[fallthrough]];
  case RF_LAUNCH_COMMAND_TEST: [[fallthrough]];
  case RF_LAUNCH_COMMAND_BUILD: [[fallthrough]];
  case RF_LAUNCH_COMMAND_RUN:
    fputs("failure: Not implemented yet!", stderr);
    break;
  }
}

// ===================================================
//    [+]   P R I V A T E   F U N C T I O N S   [+]
// ===================================================

static void execute_new_command(struct rf_launch_command executed) {
  if (rf_create_directory(executed.as_new.created_name)) {
    fprintf(stderr,
      "failure: Cannot create package `%.*s`!\n"
      "cause: %s\n",
      (int)executed.as_new.created_name.count,
      executed.as_new.created_name.array, strerror(errno));
    return;
  }

  struct rf_file library_file;
  if (rf_open_file(&library_file, "wx", ".tr", 2, executed.as_new.created_name,
        rf_view_null_terminated("Prelude"))) {
    fprintf(stderr,
      "failure: Cannot open prelude source `%.*s/Prelude.tr`!\n"
      "cause: %s\n",
      (int)executed.as_new.created_name.count,
      executed.as_new.created_name.array, strerror(errno));
    return;
  }

  fprintf(library_file.stream,
    "/* Generated prelude of %.*s package. */\n"
    "\n"
    "/* Returns a value for debugging the setup. */\n"
    "public function value(): int {\n"
    "  return 7;\n"
    "}\n",
    (int)executed.as_new.created_name.count,
    executed.as_new.created_name.array);

  if (rf_close_file(&library_file)) {
    fprintf(stderr,
      "failure: Cannot close prelude source `%.*s/Prelude.tr`!\n"
      "cause: %s\n",
      (int)executed.as_new.created_name.count,
      executed.as_new.created_name.array, strerror(errno));
    return;
  }

  struct rf_file main_file;
  if (rf_open_file(&main_file, "wx", ".tr", 2, executed.as_new.created_name,
        rf_view_null_terminated("Main"))) {
    fprintf(stderr,
      "failure: Cannot open main source `%.*s/Main.tr`!\n"
      "cause: %s\n",
      (int)executed.as_new.created_name.count,
      executed.as_new.created_name.array, strerror(errno));
    return;
  }

  fprintf(main_file.stream,
    "/* Access the prelude of %.*s for debugging the setup. */\n"
    "import %.*s;\n"
    "\n"
    "entrypoint {\n"
    "  return %.*s.value;\n"
    "}\n",
    (int)executed.as_new.created_name.count, executed.as_new.created_name.array,
    (int)executed.as_new.created_name.count, executed.as_new.created_name.array,
    (int)executed.as_new.created_name.count,
    executed.as_new.created_name.array);

  if (rf_close_file(&main_file)) {
    fprintf(stderr,
      "failure: Cannot close main source `%.*s/Main.tr`!\n"
      "cause: %s\n",
      (int)executed.as_new.created_name.count,
      executed.as_new.created_name.array, strerror(errno));
    return;
  }

  printf("Successfully created package `%.*s`.\n",
    (int)executed.as_new.created_name.count,
    executed.as_new.created_name.array);
}
