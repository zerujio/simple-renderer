add_library(simple-renderer
        STATIC
        mesh.cpp
        camera.cpp
        renderer.cpp
        shader_program.cpp
        glsl_definitions.cpp
        draw_command.cpp
        command_queue.cpp
        instanced_mesh.cpp
        vertex_buffer.cpp
        buffer_range.cpp
        allocation_registry.cpp
        vertex_attribute_specification.cpp
        texture_2d.cpp
        image_data.cpp
        buffer.cpp
        mesh_descriptor.cpp
        vertex_array.cpp
        render_queue.cpp)

target_include_directories(simple-renderer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(simple-renderer PUBLIC glm glutils PRIVATE stb_image)
target_compile_definitions(simple-renderer PUBLIC SIMPLE_RENDERER_DEBUG=$<CONFIG:Debug>)