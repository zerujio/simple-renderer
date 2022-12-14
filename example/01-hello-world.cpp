#include "simple-renderer/renderer.hpp"
#include "glutils/debug.hpp"
#include "glutils/gl.hpp" // this header drags gl.h

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cmath>

struct Cube {
    static const std::vector<glm::vec3> vertex_positions;
    static const std::vector<glm::vec3> vertex_normals;
    static const std::vector<glm::vec2> vertex_uvs;
    static const std::vector<unsigned int> indices;
};

struct UserData
{
    float camera_fov;
    float camera_near;
    float camera_far;
    simple::Camera &camera;
};

void updateResolution(GLFWwindow *window, int width, int height)
{
    // setViewport operates on the OpenGL context; it does not need a Renderer instance.
    simple::Renderer::setViewport(glm::ivec2(), glm::ivec2(width, height));

    auto data = static_cast<const UserData *>(glfwGetWindowUserPointer(window));
    data->camera.setProjectionMatrix(glm::perspectiveFov(data->camera_fov,
                                                         float(width), float(height),
                                                         data->camera_near, data->camera_far));
}

auto main() -> int
{
    glfwInit();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);   // create an OpenGL context with debug capabilities
    glm::ivec2 window_size {1024, 768};
    auto window = glfwCreateWindow(window_size.x, window_size.y, "Hello world!", nullptr, nullptr);
    if (!window) {
        std::cerr << "window creation failed" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!GL::loadGLContext(glfwGetProcAddress)) {
        std::cerr << "GL function loading failed" << std::endl;
        return -2;
    }

    GL::enableDebugCallback(); // set a debug callback for the current context.
    glfwSetWindowSizeCallback(window, updateResolution);

    // Shaders are GLSL code.
    // proj_matrix, view_matrix and model_matrix are built-in uniforms (See ShaderProgram).
    auto vert_src = R"glsl(
    void main()
    {
        gl_Position = proj_matrix * view_matrix * model_matrix * vec4(vertex_position, 1.0f);
    }
    )glsl";

    auto frag_src = R"glsl(
    void main()
    {
        frag_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    )glsl";

    {
        simple::Renderer renderer;
        simple::Camera camera;

        UserData callback_data {glm::pi<float>() / 2.0f, 0.01f, 100.0f, camera};
        glfwSetWindowUserPointer(window, &callback_data);

        camera.setViewMatrix(glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
        camera.setProjectionMatrix(glm::perspectiveFov(callback_data.camera_fov,
                                                       float(window_size.x), float(window_size.y),
                                                       callback_data.camera_near, callback_data.camera_far));

        simple::ShaderProgram program {vert_src, frag_src}; // compile shaders

        simple::Mesh mesh {Cube::vertex_positions, Cube::vertex_normals, Cube::vertex_uvs, Cube::indices};
        mesh.setDrawMode(simple::DrawMode::triangles);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            glm::mat4 transform{glm::rotate(glm::mat4(1.0f), float(glfwGetTime()), {0.0f, 1.0f, 0.0f})};
            renderer.draw(mesh, program, transform);
            renderer.finishFrame(camera);
            glfwSwapBuffers(window);
        }
        // WARNING:
        // Renderer and Camera should be destroyed before destroying the context they operate on
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

const std::vector<glm::vec3> Cube::vertex_positions
{
        // face 0
        { .5, .5,-.5},
        { .5,-.5,-.5},
        {-.5, .5,-.5},
        {-.5,-.5,-.5},
        // face 1
        { .5, .5, .5},
        { .5,-.5, .5},
        {-.5, .5, .5},
        {-.5,-.5, .5},
        // face 2
        { .5, .5, .5},
        { .5, .5,-.5},
        {-.5, .5, .5},
        {-.5, .5,-.5},
        // face 3
        { .5,-.5, .5},
        { .5,-.5,-.5},
        {-.5,-.5, .5},
        {-.5,-.5,-.5},
        // face 4
        { .5, .5, .5},
        { .5, .5,-.5},
        { .5,-.5, .5},
        { .5,-.5,-.5},
        // face 5
        {-.5, .5, .5},
        {-.5, .5,-.5},
        {-.5,-.5, .5},
        {-.5,-.5,-.5},
};

const std::vector<glm::vec3> Cube::vertex_normals
{
        { .0, .0,-1.},
        { .0, .0,-1.},
        { .0, .0,-1.},
        { .0, .0,-1.},
        { .0, .0, 1.},
        { .0, .0, 1.},
        { .0, .0, 1.},
        { .0, .0, 1.},
        { 0., 1., 0.},
        { 0., 1., 0.},
        { 0., 1., 0.},
        { 0., 1., 0.},
        { 0.,-1., 0.},
        { 0.,-1., 0.},
        { 0.,-1., 0.},
        { 0.,-1., 0.},
        { 1., 0., 0.},
        { 1., 0., 0.},
        { 1., 0., 0.},
        { 1., 0., 0.},
        {-1., 0., 0.},
        {-1., 0., 0.},
        {-1., 0., 0.},
        {-1., 0., 0.},
};

const std::vector<glm::vec2> Cube::vertex_uvs
{
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
};

const std::vector<unsigned int> Cube::indices
{
        1, 3, 0,    0, 3, 2,    // face 0
        6, 5, 4,    5, 6, 7,    // face 1
        9, 10, 8,   10, 9, 11,  // face 2
        14, 13, 12, 13, 14, 15, // face 3
        19, 16, 18, 16, 19, 17, // face 4
        22, 21, 23, 21, 22, 20  // face 5
};