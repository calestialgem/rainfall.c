# SPDX-FileCopyrightText: 2022 Cem Geçgel <gecgelcem@outlook.com>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.0)

# Set the language standard.
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

project(rainfall-c)

# Setup the given target.
function(SetupTarget setTarget)
  # Add project source root to include directories.
  target_include_directories(${setTarget} PRIVATE src)

  # Enable all warnings.
  if(MSVC)
    target_compile_options(${setTarget} PRIVATE /Wall)
  else(MSVC)
    target_compile_options(${setTarget} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wno-deprecated-declarations
      -Wno-gnu-empty-struct
      -Wno-pragma-once-outside-header
      -Wno-empty-translation-unit
    )
  endif(MSVC)
endfunction(SetupTarget setTarget)

# Main executable of the project.
add_executable(${PROJECT_NAME} src/main.c)
SetupTarget(${PROJECT_NAME})

# Header files in the project.
set(headerFiles src/lib.c)

# Add compile commands for the header files, but leave them as object files.
# Compile commands helps clangd to understand the headers.
add_library(headerObjects OBJECT ${headerFiles})
SetupTarget(headerObjects)
