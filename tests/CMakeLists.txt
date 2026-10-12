cmake_minimum_required(VERSION 3.16)
project(MdnFraxisTests LANGUAGES CXX)

# Standalone backend tests; Qt is not needed to verify the numerical library.
set(MDN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
add_library(mdn_test_backend SHARED
    ${MDN_ROOT}/library/src/Logger.cpp
    ${MDN_ROOT}/library/src/Mdn2d.cpp
    ${MDN_ROOT}/library/src/Mdn2dBase.cpp
    ${MDN_ROOT}/library/src/Mdn2dConfig.cpp
    ${MDN_ROOT}/library/src/Mdn2dIO.cpp
    ${MDN_ROOT}/library/src/Mdn2dRules.cpp
    ${MDN_ROOT}/library/src/TextOptions.cpp
    ${MDN_ROOT}/library/src/Tools.cpp
)
set_target_properties(mdn_test_backend PROPERTIES OUTPUT_NAME mdn)
target_include_directories(mdn_test_backend PUBLIC ${MDN_ROOT}/library/include)
target_compile_features(mdn_test_backend PUBLIC cxx_std_17)
target_compile_definitions(mdn_test_backend PRIVATE
    mdn_EXPORTS $<$<CONFIG:Debug>:MDN_DEBUG>)

add_executable(fraxis_probe fraxis_probe.cpp)
target_link_libraries(fraxis_probe PRIVATE mdn_test_backend)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
enable_testing()
add_test(NAME fraxis_exact
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_fraxis.py
        $<TARGET_FILE:fraxis_probe>)
set_tests_properties(fraxis_exact PROPERTIES TIMEOUT 120)
