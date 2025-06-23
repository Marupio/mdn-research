add_library(mdn SHARED
    PlaceHolderMdn.cpp
    Mdn2d.cpp
    # Digit.cpp
    # Coord.cpp
    # Add other library source files here
)

set_target_properties(mdn PROPERTIES
    OUTPUT_NAME "mdn"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_include_directories(mdn PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Specify exported headers (optional but improves clarity)
set_target_properties(mdn PROPERTIES
    PUBLIC_HEADER "PlaceHolderMdn.h;Mdn2d.h"
)
