#ifndef SIMPLERENDERER_CAMERA_HPP
#define SIMPLERENDERER_CAMERA_HPP

#include <glutils/buffer.hpp>
#include <glutils/guard.hpp>

#include <glm/mat4x4.hpp>

namespace Simple {

/// Encapsulates camera related data.
class Camera
{
    friend class Renderer;

public:
    Camera();

    /// Set the view transform, which is accesible as 'view_matrix' in shaders.
    void setViewMatrix(const glm::mat4 &matrix) const;

    /// Set the projection transform, which is accesible as 'proj_matrix' in shaders.
    void setProjectionMatrix(const glm::mat4 &matrix) const;

private:
    void bindUniformBlock() const;

    GL::Buffer m_buffer;
};

} // simple

#endif //SIMPLERENDERER_CAMERA_HPP
