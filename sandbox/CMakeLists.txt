# Helper function to add an mdn-based executable
function(add_mdn_app app_name app_source)
    add_executable(${app_name} ${app_source})
    target_include_directories(${app_name} PRIVATE ../library ${PROJECT_BINARY_DIR})
    target_link_directories(${app_name} PRIVATE ${CMAKE_BINARY_DIR}/lib)
    #target_link_directories(${app_name} PRIVATE ${CMAKE_BINARY_DIR}/bin)
    target_link_libraries(${app_name} PRIVATE mdn)

    if(WIN32)
        add_custom_command(TARGET ${app_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/bin/mdn.dll
                    $<TARGET_FILE_DIR:${app_name}>
            COMMENT "Copying mdn.dll to ${app_name} output folder (Windows only)")
    endif()
endfunction()

target_link_libraries(mdn PUBLIC mdn_config)

# Define the two apps using the helper
add_mdn_app(sandbox sandbox_main.cpp)
add_mdn_app(test_library test_library_main.cpp)
add_mdn_app(test_toVecDigits test_toVecDigits_main.cpp)
