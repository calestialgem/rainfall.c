# SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.0)

# Set the language standard.
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

project(rainfall-c)

## Main executable of the project.
add_executable(${PROJECT_NAME}
  # Launcher
  src/main.c

  # Analyzer Module
  src/analyzer/analyzer.c
  src/analyzer/evaluation.c
  src/analyzer/number.c
  src/analyzer/table.c
  src/analyzer/type.c

  # Lexer Module
  src/lexer/lex.c
  src/lexer/lexeme.c
  src/lexer/lexer.c

  # Source Module
  src/source/location.c
  src/source/portion.c
  src/source/source.c

  # Parser Module
  src/parser/expression.c
  src/parser/operator.c
  src/parser/parse.c
  src/parser/parser.c

  # Utility Module
  src/utility/buffer.c
  src/utility/map.c
  src/utility/mod.c
  src/utility/set.c
  src/utility/string.c
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
    -Wno-gnu-empty-struct
  )
endif(MSVC)
