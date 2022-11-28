#include "rainfall.h"
#include "rf_internal.h"

// Compile the implementation for the correct platform.
#ifdef _WIN32
  #include "rf_filesystem_windows.c"
#else
  // No provided implementation fits for the current platform; thus, compiler
  // cannot be compiled.
  #error Filesystem only supports the fallowing platforms: Windows.
#endif
