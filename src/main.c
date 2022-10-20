// SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lib.c"

#include <stdio.h>

/* Start the program. */
int main(int const argumentCount, char const* const* const arguments) {
  printf("Running with arguments:\n");
  for (int argument = 0; argument < argumentCount; argument++)
    printf("[%i] %s\n", argument, arguments[argument]);

  printf("%s\n", GetMessage());
}
