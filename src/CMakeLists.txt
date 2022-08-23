add_library(simple-renderer STATIC
        renderer.cpp
        shader_program.cpp shader_variables.cpp)
target_include_directories(simple-renderer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(simple-renderer
        PUBLIC glm glutils)