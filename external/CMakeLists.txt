# glfw
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
add_subdirectory(glfw)

# GL (includes glad and glm)
add_subdirectory(glutils-cpp)

# stb_image
add_subdirectory(stb)

# imgui
add_library(imgui STATIC
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PUBLIC glfw)