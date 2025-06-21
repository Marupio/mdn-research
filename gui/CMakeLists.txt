find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(mdn_gui
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

target_link_libraries(mdn_gui PRIVATE Qt6::Widgets library)

include_directories("../library")

if(IS_VSCODE_BUILD_BOOL)
    add_custom_command(TARGET mdn_gui POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt6.exe" "$<TARGET_FILE:mdn_gui>"
        COMMENT "Running windeployqt6 to bundle Qt dependencies for VS Code build..."
    )
endif()
