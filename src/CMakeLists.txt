add_library(simple-renderer STATIC
        renderer.cpp)
target_include_directories(simple-renderer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(simple-renderer
        PUBLIC glm glutils)