add_library(mdn SHARED
    Logger.cpp
    Mdn2d.cpp
    Mdn2dBase.cpp
    Mdn2dConfig.cpp
    Mdn2dIO.cpp
    Mdn2dRules.cpp
    TextOptions.cpp
    Tools.cpp
)

set_target_properties(mdn PROPERTIES
    OUTPUT_NAME "mdn"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER
        "Carryover.hpp;Constants.hpp;Coord.hpp;CoordSet.hpp;Digit.hpp;Fraxis.hpp;GlobalConfig.hpp;\
Logger.hpp;Mdn2d.hpp;Mdn2dBase.hpp;Mdn2dConfig.hpp;Mdn2dConfigImpact.hpp;Mdn2dFramework.hpp;\
Mdn2dIO.hpp;Mdn2dRules.hpp;MdnException.hpp;MdnObserver.hpp;PrecisionStatus.hpp;\
Rect.hpp;SignConvention.hpp;TextOptions.hpp;Tools.hpp"
)

target_include_directories(mdn PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(mdn PRIVATE mdn_EXPORTS)

