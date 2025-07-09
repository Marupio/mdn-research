add_executable(sandbox sandbox_main.cpp)
target_include_directories(sandbox PRIVATE ../library)
target_link_directories(sandbox PRIVATE ${CMAKE_BINARY_DIR}/lib)
#target_link_directories(sandbox PRIVATE ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(sandbox PRIVATE mdn)

if(WIN32)
    add_custom_command(TARGET sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/bin/mdn.dll
                $<TARGET_FILE_DIR:sandbox>
        COMMENT "Copying mdn.dll to sandbox output folder (Windows only)")
endif()
