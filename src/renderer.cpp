#include "simple-renderer/renderer.hpp"

#include "glutils/gl.hpp"

namespace Simple::Renderer {

void loadGL(glLoader loader)
{
    GL::loadContext(loader);
}

void setViewport(glm::ivec2 lower_left, glm::ivec2 top_right)
{
    glViewport(lower_left.x, lower_left.y, top_right.x, top_right.y);
}

} // Simple::Renderer