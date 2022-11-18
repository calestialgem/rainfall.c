/* Abstracts away the communication with the filesystem, such that the compiler
 * can be run on different platforms. */
#ifndef RF_FILESYSTEM_H
#define RF_FILESYSTEM_H 1

#include "rf_string.h"

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Changes the current working directory to the given path. */
void rf_change_working_directory(struct rf_string path);

#endif // RF_FILESYSTEM_H
