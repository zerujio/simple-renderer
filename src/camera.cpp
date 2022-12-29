#include <simple-renderer/camera.hpp>

#include "simple-renderer/glsl_definitions.hpp"
#include "glutils/gl.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <array>

namespace simple {

    constexpr auto mat4_size = sizeof(glm::mat4);

    using GL::gl;

    Camera::Camera()
    {
        std::array init_data {glm::mat4(1.0f), glm::mat4(1.0f)};
        m_buffer.allocateImmutable(2 * mat4_size, GL::BufferHandle::StorageFlags::dynamic_storage, init_data.data());
    }

    void Camera::setViewMatrix(const glm::mat4 &matrix) const
    {
        m_buffer.write(view_matrix_block_index * mat4_size, mat4_size, glm::value_ptr(matrix));
    }

    void Camera::setProjectionMatrix(const glm::mat4 &matrix) const
    {
        m_buffer.write(proj_matrix_block_index * mat4_size, mat4_size, glm::value_ptr(matrix));
    }

    void Camera::bindUniformBlock() const
    {
        gl.BindBufferBase(GL_UNIFORM_BUFFER, camera_uniform_block_def.layout.binding, m_buffer.getName());
    }

} // simple