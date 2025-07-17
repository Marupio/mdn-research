add_library(mdn SHARED
    Logger.cpp
    Mdn2dBase.cpp
    Mdn2dRules.cpp
    Mdn2d.cpp
    Tools.cpp
)

set_target_properties(mdn PROPERTIES
    OUTPUT_NAME "mdn"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER
        "Carryover.h;Constants.h;Coord.h;CoordSet.h;Digit.h;\
Fraxis.h;Logger.h;Mdn2dBase.h;Mdn2dConfig.h;Mdn2d.h;\
Mdn2dRules.h;MdnException.h;PrecisionStatus.h;SignConvention.h;Tools.h"
)

target_include_directories(mdn PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(mdn PRIVATE mdn_EXPORTS)

