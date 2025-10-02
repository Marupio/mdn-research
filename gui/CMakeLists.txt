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

# ---- Bundle feature toggles (Windows) ----
option(MDN_BUNDLE_WITH_SVG       "Bundle Qt SVG support (Qt6Svg.dll, qsvg*.dll)"              OFF)
option(MDN_BUNDLE_WITH_NETWORK   "Bundle Qt Network (Qt6Network.dll, TLS backends, qNLM)"     OFF)
option(MDN_BUNDLE_WITH_GIF       "Bundle GIF image plugin (qgif.dll)"                          OFF)
option(MDN_BUNDLE_WITH_TOUCH     "Bundle TUIO/Touch generic plugin (qtuiotouchplugin.dll)"     OFF)
option(MDN_BUNDLE_WITH_MODERNUI  "Bundle qmodernwindowsstyle.dll style plugin"                 OFF)
option(MDN_BUNDLE_WITH_DX_RHI    "Bundle dxcompiler.dll/dxil.dll (D3D12 shader compiler)"      OFF)
# usually ON
option(MDN_BUNDLE_WITH_SW_OPENGL "Bundle opengl32sw.dll software fall-back renderer"           ON)
# usually OFF for fully portable
option(MDN_BUNDLE_INCLUDE_VCREDIST "Include vc_redist.x64.exe in bundle"                       OFF)

# --- Windows-only bundle target (portable zip) ---
if(WIN32)
    # Try to find a windeployqt matching our Qt major
    if(QT_VERSION_MAJOR EQUAL 6)
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
    set(BUNDLE_DIR "${CMAKE_BINARY_DIR}/MDN-${MDN_VERSION_STRING}-win64-$<CONFIG>")
    set(BUNDLE_NAME "MDN-${MDN_VERSION_STRING}-win64-$<CONFIG>")
    set(BUNDLE_OUT_ZIP "${MDN_DIST_DIR}/${BUNDLE_NAME}.zip")

    # Where we keep tiny helper scripts for bundling
    set(MDN_CMAKE_HELPER_DIR "${CMAKE_CURRENT_BINARY_DIR}/cmake")
    file(MAKE_DIRECTORY "${MDN_CMAKE_HELPER_DIR}")

    # ---- Generate helper scripts at configure time ----
    file(WRITE "${MDN_CMAKE_HELPER_DIR}/trim_svg.cmake" [=[
if(NOT MDN_BUNDLE_WITH_SVG)
  file(REMOVE
    "${BUNDLE_DIR}/Qt6Svg.dll"
    "${BUNDLE_DIR}/qsvg.dll"
    "${BUNDLE_DIR}/qsvgicon.dll"
  )
endif()
]=])

    file(WRITE "${MDN_CMAKE_HELPER_DIR}/trim_network.cmake" [=[
if(NOT MDN_BUNDLE_WITH_NETWORK)
  file(REMOVE
    "${BUNDLE_DIR}/Qt6Network.dll"
    "${BUNDLE_DIR}/qnetworklistmanager.dll"
    "${BUNDLE_DIR}/qcertonlybackend.dll"
    "${BUNDLE_DIR}/qschannelbackend.dll"
  )
endif()
]=])

    file(WRITE "${MDN_CMAKE_HELPER_DIR}/trim_gif.cmake" [=[
if(NOT MDN_BUNDLE_WITH_GIF)
  file(REMOVE "${BUNDLE_DIR}/qgif.dll")
endif()
]=])

    file(WRITE "${MDN_CMAKE_HELPER_DIR}/trim_touch.cmake" [=[
if(NOT MDN_BUNDLE_WITH_TOUCH)
  file(REMOVE "${BUNDLE_DIR}/qtuiotouchplugin.dll")
endif()
]=])

    file(WRITE "${MDN_CMAKE_HELPER_DIR}/trim_modernui.cmake" [=[
if(NOT MDN_BUNDLE_WITH_MODERNUI)
  file(REMOVE "${BUNDLE_DIR}/qmodernwindowsstyle.dll")
endif()
]=])

    file(WRITE "${MDN_CMAKE_HELPER_DIR}/trim_dx.cmake" [=[
if(NOT MDN_BUNDLE_WITH_DX_RHI)
  file(REMOVE
    "${BUNDLE_DIR}/dxcompiler.dll"
    "${BUNDLE_DIR}/dxil.dll"
  )
endif()
]=])

    file(WRITE "${MDN_CMAKE_HELPER_DIR}/trim_swgl.cmake" [=[
if(NOT MDN_BUNDLE_WITH_SW_OPENGL)
  file(REMOVE "${BUNDLE_DIR}/opengl32sw.dll")
endif()
]=])

    file(WRITE "${MDN_CMAKE_HELPER_DIR}/include_vcredist.cmake" [=[
if(MDN_BUNDLE_INCLUDE_VCREDIST)
  if(DEFINED VCREDIST_SRC AND EXISTS "${VCREDIST_SRC}")
    file(COPY "${VCREDIST_SRC}" DESTINATION "${BUNDLE_DIR}")
  endif()
endif()
]=])

    add_custom_target(bundle_win
        COMMENT "Create a portable Windows bundle (zip) with mdn_gui, mdn.dll, and Qt runtime"
        DEPENDS mdn_gui
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${BUNDLE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_DIR}"

        # 1) Copy the GUI exe
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:mdn_gui>" "${BUNDLE_DIR}/"

        # 2) Copy mdn.dll next to the exe
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:mdn>" "${BUNDLE_DIR}/"

        # 3) Run windeployqt to pull Qt runtime directly into the bundle dir
        COMMAND "${WINDEPLOYQT}"
            $<IF:$<CONFIG:Debug>,--debug,--release>
            --no-translations
            --no-quick-import
            --dir "${BUNDLE_DIR}"
            "$<TARGET_FILE:mdn_gui>"

        # 4) Trim optional stuff based on toggles (ONE set; pass flags explicitly)
        COMMAND ${CMAKE_COMMAND} -D BUNDLE_DIR="${BUNDLE_DIR}" -D MDN_BUNDLE_WITH_SVG:BOOL=${MDN_BUNDLE_WITH_SVG}         -P "${MDN_CMAKE_HELPER_DIR}/trim_svg.cmake"
        COMMAND ${CMAKE_COMMAND} -D BUNDLE_DIR="${BUNDLE_DIR}" -D MDN_BUNDLE_WITH_NETWORK:BOOL=${MDN_BUNDLE_WITH_NETWORK} -P "${MDN_CMAKE_HELPER_DIR}/trim_network.cmake"
        COMMAND ${CMAKE_COMMAND} -D BUNDLE_DIR="${BUNDLE_DIR}" -D MDN_BUNDLE_WITH_GIF:BOOL=${MDN_BUNDLE_WITH_GIF}         -P "${MDN_CMAKE_HELPER_DIR}/trim_gif.cmake"
        COMMAND ${CMAKE_COMMAND} -D BUNDLE_DIR="${BUNDLE_DIR}" -D MDN_BUNDLE_WITH_TOUCH:BOOL=${MDN_BUNDLE_WITH_TOUCH}     -P "${MDN_CMAKE_HELPER_DIR}/trim_touch.cmake"
        COMMAND ${CMAKE_COMMAND} -D BUNDLE_DIR="${BUNDLE_DIR}" -D MDN_BUNDLE_WITH_MODERNUI:BOOL=${MDN_BUNDLE_WITH_MODERNUI} -P "${MDN_CMAKE_HELPER_DIR}/trim_modernui.cmake"
        COMMAND ${CMAKE_COMMAND} -D BUNDLE_DIR="${BUNDLE_DIR}" -D MDN_BUNDLE_WITH_DX_RHI:BOOL=${MDN_BUNDLE_WITH_DX_RHI}   -P "${MDN_CMAKE_HELPER_DIR}/trim_dx.cmake"
        COMMAND ${CMAKE_COMMAND} -D BUNDLE_DIR="${BUNDLE_DIR}" -D MDN_BUNDLE_WITH_SW_OPENGL:BOOL=${MDN_BUNDLE_WITH_SW_OPENGL} -P "${MDN_CMAKE_HELPER_DIR}/trim_swgl.cmake"
        COMMAND ${CMAKE_COMMAND} -D BUNDLE_DIR="${BUNDLE_DIR}" -D MDN_BUNDLE_INCLUDE_VCREDIST:BOOL=${MDN_BUNDLE_INCLUDE_VCREDIST} -D VCREDIST_SRC="${VCREDIST_SRC}" -P "${MDN_CMAKE_HELPER_DIR}/include_vcredist.cmake"

        # 5) Create versioned zip
        COMMAND ${CMAKE_COMMAND} -E echo "Zipping -> ${BUNDLE_OUT_ZIP}"
        COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${BUNDLE_OUT_ZIP}" --format=zip -- "${BUNDLE_DIR}"
        VERBATIM
    )
endif()
