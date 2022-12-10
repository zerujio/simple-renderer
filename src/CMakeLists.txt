add_library(simple-renderer STATIC
        renderer.cpp
        shader_program.cpp
        mesh.cpp
        glsl_definitions.cpp
        camera.cpp
        draw_command.cpp
        command_queue.cpp)
target_include_directories(simple-renderer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(simple-renderer
        PUBLIC glm glutils)