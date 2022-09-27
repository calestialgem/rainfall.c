// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lib.c"

#include <stdio.h>

/* Start the program. */
int main(int argumentCount, char const* const* arguments) {
  if (argumentCount != 2) {
    fprintf(stderr, "Provide a Thrice file!\n");
    return -1;
  }
  doSomething(arguments[1]);
}
