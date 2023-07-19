#ifndef SIMPLERENDERER_RENDERER_HPP
#define SIMPLERENDERER_RENDERER_HPP

#include "glm/vec2.hpp"

namespace Simple::Renderer {

using glProc = void(*)();
using glLoader = glProc(*)(const char*);

/// Loads OpenGL functions for the current thread. This is all the initialization the renderer requires.
void loadGL(glLoader loader);

/**
 * @brief Sets the viewport dimensions for the calling thread's rendering context.
 * The viewport determines over which portion of the framebuffer the draw commands issued by a RenderQueue will
 * draw. The coordinate system has its origin at the lower left corner of the screen.
 *
 * @param lower_left Coordinates of the lower left corner of the viewport.
 * @param top_right Coordinates of the top right corner of the viewport.
 */
void setViewport(glm::ivec2 lower_left, glm::ivec2 top_right);

}//Simple::Renderer

#endif //SIMPLERENDERER_RENDERER_HPP
