#ifndef RF_LAUNCHER_H
#define RF_LAUNCHER_H 1

/* Value that indicates the variant of a launch command. */
enum rf_launch_command_variant {
  /* Command for creating a new package in the workspace. */
  RF_LAUNCH_COMMAND_NEW,
  /* Command for syntactically and semantically checking the packages in the
     workspace. */
  RF_LAUNCH_COMMAND_CHECK,
  /* Command for running the tests of the packages in the workspace. */
  RF_LAUNCH_COMMAND_TEST,
  /* Command for building an executable package in the workspace to a C file. */
  RF_LAUNCH_COMMAND_BUILD,
  /* Command for building an running an executable package in the workspace.. */
  RF_LAUNCH_COMMAND_RUN,
};

/* An instruction that can be executed by the Rainfall compiler. */
struct rf_launch_command {
  /* Variant of the command. */
  enum rf_launch_command_variant variant;
};

#endif // RF_LAUNCHER_H
