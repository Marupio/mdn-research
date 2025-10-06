# library/CMakeLists.txt
add_library(mdn SHARED
    src/Logger.cpp
    src/Mdn2d.cpp
    src/Mdn2dBase.cpp
    src/Mdn2dConfig.cpp
    src/Mdn2dIO.cpp
    src/Mdn2dRules.cpp
    src/TextOptions.cpp
    src/Tools.cpp
)

# Output dirs
set_target_properties(mdn PROPERTIES
  OUTPUT_NAME "mdn"
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
  PUBLIC_HEADER ""  # we use directory-based install; silence PUBLIC_HEADER warnings
)

# Public headers come from library/include during build; from <prefix>/include after install
target_include_directories(mdn
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/library/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${PROJECT_SOURCE_DIR}/library # for any private headers near sources
    ${PROJECT_BINARY_DIR} # generated config headers here
)

if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)

    # Install the library artifacts
    install(TARGETS mdn
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Install all public headers under include/mdn/
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/library/include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mdn
            FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
endif()

target_link_libraries(mdn PUBLIC mdn_config)
target_compile_definitions(mdn PRIVATE mdn_EXPORTS)
