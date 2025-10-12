# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MDN_Tools VERSION 1.1.1)

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
option(Logs "Logging (OFF by default)" OFF)

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

# ---------------- Linux packaging & install rules ----------------
if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)

    # Debian policy prefers /usr; keep overrideable
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "" FORCE)
    endif()

    # ---- Binaries/libraries ----
    install(TARGETS mdn
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # ---- Public headers ----
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/library/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mdn
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )
    install(TARGETS mdn_gui
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # usually /usr/bin
    )

    # ---- Desktop integration (only if files exist) ----
    if(EXISTS "${PROJECT_SOURCE_DIR}/misc/mdn.desktop.in")
        configure_file("${PROJECT_SOURCE_DIR}/misc/mdn.desktop.in"
                "${CMAKE_CURRENT_BINARY_DIR}/mdn.desktop" @ONLY)
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mdn.desktop"
                        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    endif()

    # ---- Desktop file ----
    if(EXISTS "${PROJECT_SOURCE_DIR}/misc/mdn.desktop.in")
        configure_file("${PROJECT_SOURCE_DIR}/misc/mdn.desktop.in"
            "${CMAKE_CURRENT_BINARY_DIR}/mdn.desktop" @ONLY
        )
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mdn.desktop"
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
        )
    endif()

    # ---- Icons (hicolor) ----
    # # Map size -> source file names
    # set(_mdn_icon_map
    #     "16;${PROJECT_SOURCE_DIR}/misc/mdn_icon2_16_clean.png"
    #     "32;${PROJECT_SOURCE_DIR}/misc/mdn_icon2_32_clean.png"
    #     "48;${PROJECT_SOURCE_DIR}/misc/mdn_icon2_48_clean.png"
    #     "64;${PROJECT_SOURCE_DIR}/misc/mdn_icon2_64_clean.png"
    #     "128;${PROJECT_SOURCE_DIR}/misc/mdn_icon2_128_clean.png"
    #     "256;${PROJECT_SOURCE_DIR}/misc/mdn_icon2_256_clean.png"
    #     "512;${PROJECT_SOURCE_DIR}/misc/mdn_icon2_512_clean.png"
    # )
    # foreach(_pair IN LISTS _mdn_icon_map)
    #     list(GET _pair 0 _sz)
    #     list(GET _pair 1 _src)
    #     if(EXISTS "${_src}")
    #         # installs to /usr/share/icons/hicolor/<size>x<size>/apps/mdn.png
    #         install(FILES "${_src}"
    #             DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${_sz}x${_sz}/apps"
    #             RENAME "mdn.png"
    #         )
    #     endif()
    # endforeach()

    # icons live in misc/mdn_icon2_<size>_clean.png
    set(_mdn_icon_sizes 16 32 48 64 128 256 512)

    foreach(_sz IN LISTS _mdn_icon_sizes)
    set(_src "${PROJECT_SOURCE_DIR}/misc/mdn_icon2_${_sz}_clean.png")
    if(EXISTS "${_src}")
        install(FILES "${_src}"
                DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${_sz}x${_sz}/apps"
                RENAME "mdn.png")
    else()
        message(STATUS "Icon not found, skipping: ${_src}")
    endif()
    endforeach()

    # ---- CPack: build a .deb using system Qt deps ----
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "mdn")
    set(CPACK_PACKAGE_VENDOR "David Gaden")
    set(CPACK_PACKAGE_CONTACT "davegaden@hotmail.com")
    # set(CPACK_PACKAGE_CONTACT "https://github.com/Marupio/mdn-research/issues")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MDN: numerical tools with a Qt GUI")

    # --- Derive WORKING_VERSION_STR from git (if available) or a fallback ---
    # Priority:
    #   1) git describe --tags --always --dirty (if in a git repo)
    #   2) MDN_VERSION_STRING (if provided -DMDN_VERSION_STRING=...)
    #   3) "v0.0.1-unknown"

    set(WORKING_VERSION_STR "")

    # 1) Try Git
    find_package(Git QUIET)
    if(GIT_FOUND)
    # Are we inside a git work tree?
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --is-inside-work-tree
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE _git_inside
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _git_inside_res
    )
    if(_git_inside_res EQUAL 0 AND _git_inside STREQUAL "true")
        execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags --always --dirty
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE WORKING_VERSION_STR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _git_desc_res
        )
        if(NOT _git_desc_res EQUAL 0 OR WORKING_VERSION_STR STREQUAL "")
        set(WORKING_VERSION_STR "")
        endif()
    endif()
    endif()

    # 2) Fallback to user-provided -DMDN_VERSION_STRING=...
    if(WORKING_VERSION_STR STREQUAL "" AND DEFINED MDN_VERSION_STRING)
    set(WORKING_VERSION_STR "${MDN_VERSION_STRING}")
    endif()

    # 3) Last resort
    if(WORKING_VERSION_STR STREQUAL "")
    set(WORKING_VERSION_STR "v0.0.1-unknown")
    endif()

    message(STATUS "MDN working version: ${WORKING_VERSION_STR}")

    # ---------------- Debian-friendly split ----------------
    # Convert something like "v1.0.1-8-gae35193" to:
    #   CPACK_PACKAGE_VERSION        = 1.0.1
    #   CPACK_DEBIAN_PACKAGE_RELEASE = 8+gae35193    (no hyphens)
    #
    # If there is no '-' in the string after stripping 'v',
    #   version = entire string, release = "1"

    # Strip leading 'v'
    string(REGEX REPLACE "^v" "" DEB_DESCRIBE "${WORKING_VERSION_STR}")

    # Default split (no dash case)
    set(DEB_VERSION   "${DEB_DESCRIBE}")
    set(DEB_REVISION  "1")

    # If there is a dash, split at the first dash
    string(FIND "${DEB_DESCRIBE}" "-" _dashpos)
    if(_dashpos GREATER -1)
    string(SUBSTRING "${DEB_DESCRIBE}" 0 ${_dashpos} DEB_VERSION)
    math(EXPR _restpos "${_dashpos} + 1")
    string(SUBSTRING "${DEB_DESCRIBE}" ${_restpos} -1 _raw_rev)
    string(REPLACE "-" "+" DEB_REVISION "${_raw_rev}")
    endif()

    # Apply to CPack
    set(CPACK_PACKAGE_VERSION "${DEB_VERSION}")            # must start with a digit
    set(CPACK_DEBIAN_PACKAGE_RELEASE "${DEB_REVISION}")  # no hyphens

    message(STATUS "Debian version: ${DEB_VERSION}, release: ${DEB_REVISION}")


    # Let dpkg-shlibdeps compute deps from your built binaries
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

    # Better file name (mdn_<ver>_amd64.deb)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    include(CPack)
endif()
