add_custom_target(simple-renderer-examples)

add_library(simple-renderer-examples-common STATIC renderer-example-common.cpp)
target_link_libraries(simple-renderer-examples-common PUBLIC glm)

function(add_renderer_example name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PUBLIC glfw simple-renderer simple-renderer-examples-common)
    add_dependencies(simple-renderer-examples ${name})
endfunction()

add_renderer_example(01-hello-world)
add_renderer_example(02-instanced-mesh)
