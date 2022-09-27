# SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.0)

# Set the language standard.
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

project(rainfall-cc)

## List of header files. Compile commands will be generated for these. Otherwise
## clangd struggles to do any analysis on them.
set(headers
  src/lib.c
)

## Sets the compile options of the given target.
function(setup target)
  # Add project source root to include directories.
  target_include_directories(${target} PRIVATE src)

  # Enable all warnings.
  if(MSVC)
    target_compile_options(${target} PRIVATE /Wall)
  else(MSVC)
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wno-pragma-once-outside-header
    )
  endif(MSVC)
endfunction(setup target)

## Main executable of the project.
add_executable(${PROJECT_NAME} src/main.c)
setup(${PROJECT_NAME})

# Create compile commands for the header files.
add_library(header_objects OBJECT ${headers})
setup(header_objects)
