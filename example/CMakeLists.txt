add_executable(demo demo.cpp)
target_link_libraries(demo PUBLIC glfw simple-renderer)
target_include_directories(demo PUBLIC ${PROJECT_SOURCE_DIR})