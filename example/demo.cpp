#include <simple-renderer/renderer.hpp>
#include <glutils/debug.hpp>

#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>

auto main() -> int
{
    glfwInit();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    auto window = glfwCreateWindow(1024, 768, "Hello world!", nullptr, nullptr);
    if (!window) {
        std::cerr << "window creation failed" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!glutils::loadGLContext(glfwGetProcAddress)) {
        std::cerr << "GL function loading failed" << std::endl;
        return -2;
    }

    glutils::enableDebugCallback();

    simple::Renderer renderer;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glm::vec2 point {std::cos(glfwGetTime()), std::sin(glfwGetTime())};
        renderer.draw(point);
        renderer.finishFrame();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}