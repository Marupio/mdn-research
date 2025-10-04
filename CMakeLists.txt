cmake_minimum_required(VERSION 3.16)
project(MDN_Tools VERSION 1.1.0)

execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE GIT_DESCRIBE_OK
    OUTPUT_VARIABLE GIT_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(GIT_DESCRIBE_OK EQUAL 0)
    set(MDN_VERSION_STRING "${GIT_DESCRIBE}")
else()
    set(MDN_VERSION_STRING "${PROJECT_VERSION}")
endif()


# A place to drop packaged artefacts
set(MDN_DIST_DIR "${CMAKE_BINARY_DIR}/dist")
file(MAKE_DIRECTORY "${MDN_DIST_DIR}")

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
add_compile_definitions(MDN_RELEASE_TYPE="$<CONFIG>")

# Per-compiler debug options (compile + link), guarded properly
# Use these globally, or put them per-target (recommended) – see below.

# GLOBAL (applies to all targets)
add_compile_options(
  # GCC/Clang Debug => -g
  $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<CONFIG:Debug>>:-g>
  # nice to have on MSVC always
  $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
  # MSVC Debug => PDB info
  $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Zi>
)
add_link_options(
  # MSVC link: emit PDB
  $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/DEBUG>
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


# --- Projects ---
add_subdirectory(library)   # mdn (SHARED)

# Make sandbox/test apps opt-in. They are excluded from ALL even when present.
option(BUILD_SANDBOX "Build sandbox/test helper apps" OFF)
if(BUILD_SANDBOX)
    add_subdirectory(sandbox EXCLUDE_FROM_ALL)
else()
    # Still add to permit IDE discovery, but never in default ALL.
    add_subdirectory(sandbox EXCLUDE_FROM_ALL)
endif()

add_subdirectory(gui)       # mdn_gui (EXE)

# add_subdirectory(library)   # Builds the MDN library
# add_subdirectory(sandbox)   # Builds the sandbox tester
# add_subdirectory(gui)       # Builds the GUI application
# --- Bundle target: build only the shared library and the GUI ---
add_custom_target(bundle
    DEPENDS mdn mdn_gui
    COMMENT "Build MDN shared library (mdn) and GUI (mdn_gui) only"
)

# # ---- choose a FHS-friendly install prefix on Linux ----
# if(UNIX AND NOT APPLE)
#   # let packagers override, but default to /usr (Debian policy)
#   if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#     set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix" FORCE)
#   endif()
# endif()
#
# # ---- install your binaries ----
# install(TARGETS mdn_gui
#   RUNTIME DESTINATION bin
# )
# install(TARGETS mdn
#   LIBRARY DESTINATION lib
#   ARCHIVE DESTINATION lib
#   RUNTIME DESTINATION bin # if mdn.dll-equivalent produces a runtime on Linux, usually not
# )
#
# # ---- (optional) install shared data/help/resources ----
# # install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION share/mdn)
#
# # ---- desktop integration (icon + .desktop entry) ----
# # Put an app icon at: misc/icons/mdn.png (256x256 recommended)
# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/misc/icons/mdn.png
#         DESTINATION share/icons/hicolor/256x256/apps)
#
# # Template at misc/mdn.desktop.in
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/misc/mdn.desktop.in
#                ${CMAKE_CURRENT_BINARY_DIR}/mdn.desktop @ONLY)
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mdn.desktop
#         DESTINATION share/applications)
#