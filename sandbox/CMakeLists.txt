add_executable(sandbox sandbox_main.cpp)
# target_include_directories(sandbox PRIVATE ../library)
target_link_directories(sandbox PRIVATE ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(sandbox PRIVATE mdn)
