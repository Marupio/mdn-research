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
    BinaryOperationDialog.hpp BinaryOperationDialog.cpp
    CellLineEdit.cpp CellLineEdit.hpp
    Clipboard.cpp Clipboard.hpp
    CommandWidget.hpp CommandWidget.cpp
    HelpDialog.hpp HelpDialog.cpp
    HoverPeekTabBar.hpp HoverPeekTabBar.cpp
    HoverPeekTabWidget.hpp HoverPeekTabWidget.cpp
    LoggerConfigurator.hpp LoggerConfigurator.cpp
    main.cpp
    MainWindow.cpp MainWindow.hpp
    MarkerWidget.hpp
    NumberDisplayWidget.cpp NumberDisplayWidget.hpp
    OperationHistory.ui
    OperationPlan.hpp
    OperationStrip.hpp OperationStrip.cpp
    OpsController.hpp OpsController.cpp
    Project.hpp Project.cpp
    ProjectPropertiesDialog.hpp ProjectPropertiesDialog.cpp
    QtLoggingBridge.hpp QtLoggingBridge.cpp
    StatusDisplayWidget.hpp StatusDisplayWidget.cpp
    WelcomeDialog.hpp
)

target_link_libraries(mdn_gui PRIVATE ${QT_LIBS} mdn)

target_sources(mdn_gui PRIVATE help.qrc)

include_directories("../library" ${PROJECT_BINARY_DIR})

# Console test app that exercises Project (no windows shown)
add_executable(guiTest
    guiTest.cpp
    LoggerConfigurator.hpp LoggerConfigurator.cpp
    Project.cpp Project.hpp
    Clipboard.cpp Clipboard.hpp
    QtLoggingBridge.cpp QtLoggingBridge.hpp
)

target_link_libraries(mdn PUBLIC mdn_config)
target_link_libraries(guiTest PRIVATE ${QT_LIBS} mdn)
target_include_directories(guiTest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "../library")

if(IS_VSCODE_BUILD_BOOL)
    message(STATUS "Adding command to copy Qt DLLs and library for VS Code build")
    # Use --debug for Debug, --release otherwise
    add_custom_command(TARGET mdn_gui POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt6.exe"
                $<$<CONFIG:Debug>:--debug>
                $<$<NOT:$<CONFIG:Debug>>:--release>
                "$<TARGET_FILE:mdn_gui>"
        COMMENT "Running windeployqt6 to bundle Qt dependencies for VS Code build..."
    )
    add_custom_command(TARGET mdn_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:mdn>                # the DLL
            $<TARGET_FILE_DIR:mdn_gui>        # next to the EXE
    )
endif()

# --- Windows-only bundle target (portable zip) ---
if(WIN32)
    # Try to find a windeployqt matching our Qt major
    if(QT_VERSION_MAJOR EQUAL 6)
        # Prefer windeployqt from Qt6; allow environment/Qt path
        find_program(WINDEPLOYQT windeployqt6.exe HINTS "$ENV{Qt6_DIR}/../../../bin" "${Qt6_DIR}/../../../bin")
        if(NOT WINDEPLOYQT)
            find_program(WINDEPLOYQT windeployqt.exe)  # fallback name
        endif()
    else()
        find_program(WINDEPLOYQT windeployqt.exe)
    endif()

    if(NOT WINDEPLOYQT)
        message(FATAL_ERROR "Could not find windeployqt(.exe). Ensure Qt is installed and in PATH or set Qt*_DIR.")
    endif()

    # Bundle staging directory: one folder per config (Debug/Release)
    set(BUNDLE_DIR "${CMAKE_BINARY_DIR}/bundle/$<CONFIG>")
    set(BUNDLE_NAME "MDN-${MDN_VERSION_STRING}-win64-$<CONFIG>")  # e.g., MDN-v1.2.3-win64-Release
    set(BUNDLE_OUT_ZIP "${MDN_DIST_DIR}/${BUNDLE_NAME}.zip")

    add_custom_target(bundle_win
        COMMENT "Create a portable Windows bundle (zip) with mdn_gui, mdn.dll, and Qt runtime"
        DEPENDS mdn_gui
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${BUNDLE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_DIR}"

        # 1) Copy the GUI exe
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:mdn_gui>" "${BUNDLE_DIR}/"

        # 2) Copy mdn.dll next to the exe
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:mdn>" "${BUNDLE_DIR}/"

        # 3) Run windeployqt to pull in the Qt runtime DLLs
        #    Use --release for Release, --debug for Debug
        COMMAND "${WINDEPLOYQT}"
            $<$<CONFIG:Debug>:--debug>
            $<$<NOT:$<CONFIG:Debug>>:--release>
            --no-translations
            --no-quick-import
            "$<TARGET_FILE:mdn_gui>"

        # 4) windeployqt drops files next to the EXE; move them into BUNDLE_DIR if needed
        #   (Some Qt setups deploy right beside the exe already; this step is cheap if not
        #   required.)
        COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:mdn_gui>" "${BUNDLE_DIR}"

        # 5) Create versioned zip
        COMMAND ${CMAKE_COMMAND} -E echo "Zipping -> ${BUNDLE_OUT_ZIP}"
        COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${BUNDLE_OUT_ZIP}" --format=zip -- "${BUNDLE_DIR}"
        VERBATIM
    )
endif()
