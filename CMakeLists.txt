cmake_minimum_required(VERSION 3.16)
project(MDNToolkit)

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
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
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
