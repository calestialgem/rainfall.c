#include "rf_launcher.h"

#include <stdio.h>

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

void rf_launch(struct rf_launch_command launched) {
  switch (launched.variant) {
  case RF_LAUNCH_COMMAND_NEW:
  case RF_LAUNCH_COMMAND_CHECK:
  case RF_LAUNCH_COMMAND_TEST:
  case RF_LAUNCH_COMMAND_BUILD:
  case RF_LAUNCH_COMMAND_RUN:
    fputs("failure: Not implemented yet!", stderr);
    break;
  }
}
