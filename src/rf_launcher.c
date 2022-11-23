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
  // Check package name.
  if (executed.as_new.created_name.count == 0) {
    fputs("failure: Cannot create package with an empty name!\n", stderr);
    return;
  }
  if (executed.as_new.created_name.array[0] < 'A' ||
      executed.as_new.created_name.array[0] > 'Z') {
    fprintf(stderr,
      "failure: Cannot create package `%.*s`!\n"
      "info: Package name must start with an uppercase English letter.\n",
      (int)executed.as_new.created_name.count,
      executed.as_new.created_name.array);
    return;
  }
  for (size_t i = 0; i < executed.as_new.created_name.count; i++) {
    if ((executed.as_new.created_name.array[0] < 'A' ||
          executed.as_new.created_name.array[0] > 'Z') &&
        (executed.as_new.created_name.array[0] < 'a' ||
          executed.as_new.created_name.array[0] > 'z') &&
        (executed.as_new.created_name.array[0] < '0' ||
          executed.as_new.created_name.array[0] > '9')) {
      fprintf(stderr,
        "failure: Cannot create package `%.*s`!\n"
        "info: Package name must solely consist of English letters and decimal "
        "digits.\n",
        (int)executed.as_new.created_name.count,
        executed.as_new.created_name.array);
      return;
    }
  }

  // Create package directory.
  if (rf_create_directory(executed.as_new.created_name)) {
    fprintf(stderr,
      "failure: Cannot create package `%.*s`!\n"
      "cause: %s\n",
      (int)executed.as_new.created_name.count,
      executed.as_new.created_name.array, strerror(errno));
    return;
  }

  // Create a source file that is inside the package.
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

  // Create a source file with an entry point.
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

  // Report success.
  printf("Successfully created package `%.*s`.\n",
    (int)executed.as_new.created_name.count,
    executed.as_new.created_name.array);
}
