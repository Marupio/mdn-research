# Try Qt6 first
find_package(Qt6 COMPONENTS Widgets Core Gui QUIET)

if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBS Qt6::Widgets Qt6::Core Qt6::Gui)
else()
    # Fallback to Qt5
    find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)
    set(QT_VERSION_MAJOR 5)
    set(QT_LIBS Qt5::Widgets Qt5::Core Qt5::Gui)
endif()

# Enable Qt meta-object compiler (MOC), UI compiler (UIC), resource compiler (RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Choose your C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mdn_gui
    Clipboard.cpp Clipboard.hpp
    main.cpp
    MainWindow.cpp MainWindow.hpp
    MdnMainWindow.hpp MdnMainWindow.cpp MdnMainWindow.ui
    NumberDisplayWidget.cpp NumberDisplayWidget.hpp
    OperationHistory.ui
    Project.hpp Project.cpp
)

target_link_libraries(mdn_gui PRIVATE ${QT_LIBS} mdn)

include_directories("../library")

# target_compile_definitions(mdn_gui PRIVATE mdn_EXPORTS)

if(IS_VSCODE_BUILD_BOOL)
    message(STATUS "Adding command to copy Qt DLLs and library for VS Code build")
    add_custom_command(TARGET mdn_gui POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt6.exe" "$<TARGET_FILE:mdn_gui>"
        COMMENT "Running windeployqt6 to bundle Qt dependencies for VS Code build..."
    )
    add_custom_command(TARGET mdn_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:mdn>                # the DLL
            $<TARGET_FILE_DIR:mdn_gui>        # next to the EXE
    )
endif()
