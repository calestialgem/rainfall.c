cmake_minimum_required(VERSION 3.0)

# Set the language standard.
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

project(rainfall-c)

# Setup the given target.
function(setup_target target_to_setup)
  # Add project source root to include directories.
  target_include_directories(${target_to_setup} PRIVATE src)

  # Enable all warnings.
  if(MSVC)
    target_compile_options(${target_to_setup} PRIVATE /Wall)
  else(MSVC)
    target_compile_options(${target_to_setup} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wno-deprecated-declarations
      -Wno-gnu-empty-struct
      -Wno-pragma-once-outside-header
      -Wno-empty-translation-unit
    )
  endif(MSVC)
endfunction(setup_target target_to_setup)

# Main executable of the project.
add_executable(${PROJECT_NAME} src/main.c)
setup_target(${PROJECT_NAME})

# Header files in the project.
set(header_files
)

# Add compile commands for the header files, but leave them as object files.
# Compile commands helps clangd to understand the headers.
if(header_files)
  add_library(headerObjects OBJECT ${header_files})
  setup_target(headerObjects)
endif(header_files)
