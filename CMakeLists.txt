cmake_minimum_required(VERSION 3.16)
project(MDN_Tools VERSION 1.0.0)

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/generated")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${PROJECT_BINARY_DIR}/generated/mdn_config.h @ONLY)

# Small interface target that carries the include path to everyone
add_library(mdn_config INTERFACE)
target_include_directories(mdn_config INTERFACE "${PROJECT_BINARY_DIR}/generated")


# Detect if this is a VS Code build (based on presence of 'out' in the build path)
string(FIND "${CMAKE_BINARY_DIR}" "/out/" IS_VSCODE_BUILD)
set(IS_VSCODE_BUILD_BOOL FALSE)
if(NOT IS_VSCODE_BUILD EQUAL -1)
    message(STATUS "Detected VS Code build")
    set(IS_VSCODE_BUILD_BOOL TRUE)
else(NOT IS_VSCODE_BUILD EQUAL -1)
    message(STATUS "Did not detect VS Code build")
endif()

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions($<$<CONFIG:Debug>:MDN_DEBUG>)

# Per-compiler debug options (compile + link), guarded properly
# Use these globally, or put them per-target (recommended) – see below.

# GLOBAL (applies to all targets)
add_compile_options(
  $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<CONFIG:Debug>>:-g>           # GCC/Clang Debug => -g
  $<$<CXX_COMPILER_ID:MSVC>:/utf-8>                                     # nice to have on MSVC always
  $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Zi>                 # MSVC Debug => PDB info
)
add_link_options(
  $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/DEBUG>              # MSVC link: emit PDB
)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    message(STATUS "No build type set, setting Debug.")
endif()

# Add NDEBUG in Release mode (disables assert)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(NDEBUG)
endif()

# Add MDN_DEBUG define in Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(MDN_DEBUG)
    # set(CMAKE_CXX_FLAGS_DEBUG "-g")
    message(STATUS "Debug enabled, MDN_DEBUG is defined.")
else(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Release build enabled, no debug defined and stuff")
endif()

# Logging option
option(Logs "Logging (ON by default)" ON)

if(Logs)
    add_compile_definitions(MDN_LOGS)
    message(STATUS "Logging is ON (enabled) in this configuration")
else(Logs)
    message(STATUS "Logging is OFF (disabled) in this configuration")
endif(Logs)


# Top-level CMakeLists.txt

# Choose which one to include by commenting out the other
add_subdirectory(library)   # Builds the MDN library
add_subdirectory(sandbox)   # Builds the sandbox tester
add_subdirectory(gui)       # Builds the GUI application
