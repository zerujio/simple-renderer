#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "glutils/gl.hpp"
#include "GLFW/glfw3.h"

#include <iostream>

struct ContextInitListener : Catch::TestEventListenerBase
{
    using TestEventListenerBase::TestEventListenerBase;

    void testRunStarting(const Catch::TestRunInfo& test_info) override
    {
        constexpr auto glfw_error_callback = [](int err_code, const char* msg)
        {
            std::cerr << "GLFW Error " << err_code << ": " << msg << std::endl;
        };

        glfwSetErrorCallback(glfw_error_callback);
        glfwInit();

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        window = glfwCreateWindow(10, 10, "Test", nullptr, nullptr);

        glfwMakeContextCurrent(window);

        GL::loadContext(glfwGetProcAddress);
        GL::enableDebugMessages();
    }

    void testRunEnded(const Catch::TestRunStats& test_stats) override
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    GLFWwindow* window {nullptr};
};

CATCH_REGISTER_LISTENER(ContextInitListener)