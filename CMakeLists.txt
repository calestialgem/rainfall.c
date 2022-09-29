# SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.0)

# Set the language standard.
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

project(rainfall-cc)

## Main executable of the project.
add_executable(${PROJECT_NAME}
  # Launcher
  src/main.c

  # Debug Module
  src/dbg/mod.c

  # Lexer Module
  src/lxr/lex.c
  src/lxr/lexeme.c
  src/lxr/lexer.c

  # Outcome Module
  src/otc/location.c
  src/otc/outcome.c
  src/otc/portion.c
  src/otc/source.c

  # Parser Module
  src/psr/operator.c
  src/psr/parse.c
  src/psr/parser.c

  # Utility Module
  src/utl/buffer.c
  src/utl/string.c
)

# Add project source root to include directories.
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Enable all warnings.
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
else(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-deprecated-declarations
  )
endif(MSVC)
