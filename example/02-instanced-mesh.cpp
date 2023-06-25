#include "renderer-example-common.hpp"

#include "simple-renderer/renderer.hpp"
#include "simple-renderer/instanced_mesh.hpp"

#include "glutils/gl.hpp"
#include "GLFW/glfw3.h"

#include <iostream>
#include <array>

struct {
    float fov = glm::pi<float>() / 2.0f;
    float far = 100.0f;
    float near = 0.01f;
    Simple::Camera* ptr = nullptr;
} s_camera;

void updateResolution(GLFWwindow* window, int width, int height)
{
    Simple::Renderer::setViewport(glm::ivec2(), glm::ivec2(width, height));

    if (s_camera.ptr)
        s_camera.ptr->setProjectionMatrix(glm::perspectiveFov(s_camera.fov, float(width), float(height),
                                                              s_camera.near, s_camera.far));
}

constexpr auto vertex_shader = R"glsl(
layout(location = 4) in vec3 a_position_offset;

out vec3 f_normal;
out vec3 f_position;

void main()
{
    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(vertex_position + a_position_offset, 1.0f);
    f_position = vec3(model_matrix * vec4(vertex_position, 1.0f));
    f_normal = mat3(transpose(inverse(model_matrix))) * vertex_normal;
}
)glsl";

constexpr auto fragment_shader = R"glsl(
in vec3 f_normal;
in vec3 f_position;

const vec3 light_color      = {1., 1., 1.};
const vec3 light_direction  = {-1., -1., 0.};
const vec3 view_position    = {5., 5., 5.};
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

void renderLoop(GLFWwindow* window)
{
    Simple::Renderer renderer;

    Simple::Camera camera;
    s_camera.ptr = &camera;
    camera.setViewMatrix(glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
    {
        glm::ivec2 window_size;
        glfwGetFramebufferSize(window, &window_size.x, &window_size.y);
        camera.setProjectionMatrix(glm::perspectiveFov(s_camera.fov, float(window_size.x), float(window_size.y),
                                                       s_camera.near, s_camera.far));
    }

    Simple::ShaderProgram shader_program{vertex_shader, fragment_shader};

    Simple::InstancedMesh mesh{Cube::vertex_positions, Cube::vertex_normals, Cube::vertex_uvs, Cube::indices};

    std::array<glm::vec3, 27> instance_offsets {};
    {
        std::size_t i = 0;
        std::array<float, 3> offsets {-2.f, 0.f, 2.f};

        for (float x : offsets)
            for (float y : offsets)
                for (float z : offsets)
                    instance_offsets[i++] = {x, y, z};
    }

    mesh.addInstanceData(4, 1, instance_offsets);
    mesh.setInstanceCount(instance_offsets.size());

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        constexpr float angular_v = 0.75f;
        glm::mat4 transform{glm::rotate(glm::mat4(1.0f), float(glfwGetTime()) * angular_v, {0.0f, 1.0f, 0.0f})};

        renderer.draw(mesh, shader_program, transform);
        renderer.finishFrame(camera);

        glfwSwapBuffers(window);
    }
}

int main()
{
    glfwSetErrorCallback([](int error_code, const char* error_msg)
                         { std::cerr << "GLFW Error " << error_code << " : " << error_msg << "\n"; });

    glfwInit();

#if GLUTILS_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1024, 768, "02 Instanced Mesh", nullptr, nullptr);
    if (!window)
        return -1;

    glfwMakeContextCurrent(window);

    GL::loadContext(glfwGetProcAddress);

    renderLoop(window);

    glfwTerminate();
}