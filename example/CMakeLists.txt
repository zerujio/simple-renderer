add_custom_target(simple-renderer-examples)

function(add_renderer_example name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PUBLIC glfw simple-renderer)
    add_dependencies(simple-renderer-examples ${name})
endfunction()

add_renderer_example(01-hello-world)