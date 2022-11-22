cmake_minimum_required(VERSION 3.0)

# Set the language standard.
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED true)

project(rainfall-c)

# Main executable of the project.
add_executable(${PROJECT_NAME}
  src/main.c
  src/rf_allocator.c
  src/rf_tester.c
  src/rf_string.c
  src/rf_filesystem.c
  src/rf_modeler.c
  src/rf_launcher.c
)

# Add project source root to include directories.
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PUBLIC inc)

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
