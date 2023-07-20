#include "simple_renderer/renderer.hpp"
#include "simple_renderer/render_queue.hpp"

#include "glutils/gl.hpp" // this header drags gl.h

#include "GLFW/glfw3.h"

#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <vector>

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
    Simple::Renderer::Camera &camera;
};

void updateResolution(GLFWwindow *window, int width, int height)
{
    // setViewport operates on the OpenGL context; it does not need a Renderer instance.
    Simple::Renderer::setViewport(glm::ivec2(), glm::ivec2(width, height));

    auto data = static_cast<const UserData *>(glfwGetWindowUserPointer(window));
    data->camera.setProjectionMatrix(glm::perspectiveFov(data->camera_fov,
                                                         float(width), float(height),
                                                         data->camera_near, data->camera_far));
}

void glfwErrorCallback(int error_code, const char* signature)
{
    std::cout << "GLFW Error " << error_code << ": " << signature << "\n";
}

int main()
{
    glfwSetErrorCallback(glfwErrorCallback);

    glfwInit();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);   // create an OpenGL context with debug capabilities

    glm::ivec2 window_size {1024, 768};
    const auto window = glfwCreateWindow(window_size.x, window_size.y, "Hello world!", nullptr, nullptr);
    if (!window) {
        std::cerr << "window creation failed" << std::endl;
    }

    glfwMakeContextCurrent(window);

    GL::loadContext(glfwGetProcAddress);
    GL::enableDebugMessages();

    glfwSetWindowSizeCallback(window, updateResolution);

    // Shaders are GLSL code.
    // proj_matrix, view_matrix and model_matrix are built-in uniforms (See ShaderProgram).
    constexpr auto vert_src = R"glsl(
    out vec3 f_normal;
    out vec3 f_position;

    void main()
    {
        gl_Position = proj_matrix * view_matrix * model_matrix * vec4(vertex_position, 1.0f);
        f_position = vec3(model_matrix * vec4(vertex_position, 1.0f));
        f_normal = mat3(transpose(inverse(model_matrix))) * vertex_normal;
    }
    )glsl";

    constexpr auto frag_src = R"glsl(
    in vec3 f_normal;
    in vec3 f_position;

    const vec3 light_color      = {1., 1., 1.};
    const vec3 light_direction  = {-1., -1., 0.};
    const vec3 view_position    = {1., 1., 1.};
    const float ambient_light_intensity     = 0.1f;
    const float specular_light_intensity    = 0.5f;

    void main()
    {
        const vec3 normal = normalize(-f_normal);
        const float diffuse_light_intensity = max(dot(normal, light_direction), 0.f);

        const vec3 view_direction = normalize(view_position - f_position);
        const vec3 reflect_direction = reflect(light_direction, normal);
        const float spec = pow(max(dot(view_direction, reflect_direction), 0.f), 32);

        const vec3 color_sum = (ambient_light_intensity + diffuse_light_intensity + specular_light_intensity * spec)
                                * light_color;
        frag_color = vec4(color_sum, 1.0f);
    }
    )glsl";

    {
        using namespace Simple::Renderer;

        RenderQueue render_queue;
        Camera camera;

        UserData callback_data {glm::pi<float>() / 2.0f, 0.01f, 100.0f, camera};
        glfwSetWindowUserPointer(window, &callback_data);

        camera.setViewMatrix(glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
        camera.setProjectionMatrix(glm::perspectiveFov(callback_data.camera_fov,
                                                       float(window_size.x), float(window_size.y),
                                                       callback_data.camera_near, callback_data.camera_far));

        ShaderProgram program {vert_src, frag_src}; // compile shaders

        Mesh mesh
        {
                Cube::vertex_positions,
                Cube::vertex_normals,
                Cube::vertex_uvs,
                Cube::indices
        };

        enable(Capability::depth_test);
        enable(Capability::cull_face);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            constexpr float angular_v = 0.75f;
            glm::mat4 transform{glm::rotate(glm::mat4(1.0f), float(glfwGetTime()) * angular_v, {0.0f, 1.0f, 0.0f})};

            render_queue.draw(mesh, program, transform);
            render_queue.finishFrame(camera);

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
        // face 0
        { .0, .0,-1.},
        { .0, .0,-1.},
        { .0, .0,-1.},
        { .0, .0,-1.},
        // face 1
        { .0, .0, 1.},
        { .0, .0, 1.},
        { .0, .0, 1.},
        { .0, .0, 1.},
        // face 2
        { 0., 1., 0.},
        { 0., 1., 0.},
        { 0., 1., 0.},
        { 0., 1., 0.},
        // face 3
        { 0.,-1., 0.},
        { 0.,-1., 0.},
        { 0.,-1., 0.},
        { 0.,-1., 0.},
        // face 4
        { 1., 0., 0.},
        { 1., 0., 0.},
        { 1., 0., 0.},
        { 1., 0., 0.},
        // face 5
        {-1., 0., 0.},
        {-1., 0., 0.},
        {-1., 0., 0.},
        {-1., 0., 0.},
};

const std::vector<glm::vec2> Cube::vertex_uvs
{
        // face 0
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        // face 1
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        // face 2
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        // face 3
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        // face 4
        {1., 0.},
        {1., 1.},
        {0., 0.},
        {0., 1.},
        // face 5
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
