#include <simple-renderer/renderer.hpp> // The header drags gl.h
#include <glutils/debug.hpp>

#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>

void updateResolution(GLFWwindow *window, int width, int height)
{
    auto renderer = static_cast<simple::Renderer *>(glfwGetWindowUserPointer(window));
    renderer->setViewport(glm::ivec2() , glm::ivec2(width, height));
}

auto main() -> int
{
    glfwInit();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);   // create an OpenGL context with debug capabilities
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

    glutils::enableDebugCallback(); // set a debug callback for the current context.

    {
        simple::Renderer renderer;  // Resources are allocated at construction

        glfwSetWindowUserPointer(window, &renderer);
        glfwSetWindowSizeCallback(window, updateResolution);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            glm::vec2 point {std::cos(glfwGetTime()), std::sin(glfwGetTime())};
            renderer.draw(point);
            renderer.finishFrame();
            glfwSwapBuffers(window);
        }
    }   // The Renderer should be destroyed before destroying the context it operates on!

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}