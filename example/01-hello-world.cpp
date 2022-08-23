#include <simple-renderer/renderer.hpp> // The header drags gl.h
#include <glutils/debug.hpp>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cmath>

void updateResolution(GLFWwindow *window, int width, int height)
{
    // setViewport operates on the OpenGL context; it does not need a Renderer instance.
    simple::Renderer::setViewport(glm::ivec2() , glm::ivec2(width, height));
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

    auto vert_src = R"glsl(
    void main()
    {
        gl_Position = model_tr * vec4(vertex_position, 1.0f);
    }
    )glsl";

    auto frag_src = R"glsl(
    void main()
    {
        frag_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    )glsl";

    glutils::enableDebugCallback(); // set a debug callback for the current context.
    glfwSetWindowSizeCallback(window, updateResolution);

    {
        simple::Renderer renderer;  // Resources are allocated at construction

        simple::ShaderProgram program {vert_src, frag_src}; // compile shaders

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            glm::vec3 point {std::cos(glfwGetTime()), std::sin(glfwGetTime()), 0.0f};
            glm::mat4 transform{glm::translate(glm::mat4(1.0f), point)};
            renderer.draw(program, transform);
            renderer.finishFrame();
            glfwSwapBuffers(window);
        }
    }   // The Renderer should be destroyed before destroying the context it operates on!

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}