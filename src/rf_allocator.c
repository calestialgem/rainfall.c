#include "rf_allocator.h"

// Conditionally compile the debug or release version depending on the flag.
#ifndef NDEBUG
  #include "rf_allocator_debug.c"
#else
  #include "rf_allocator_release.c"
#endif
