find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(mdn_gui
    main.cpp
    MainWindow.cpp
    MainWindow.h
    NumberDisplayWidget.cpp
    NumberDisplayWidget.h
)

target_link_libraries(mdn_gui PRIVATE Qt6::Widgets mdn)

include_directories("../library")

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
