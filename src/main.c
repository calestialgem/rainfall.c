// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "allocation.c"
#include "string.c"
#include "test.c"

/* Start the program. */
int main(void) {
  RegisterTestModuleTests();
  RegisterStringModuleTests();
  RegisterAllocationModuleTests();
  RunUnitTests();
}
