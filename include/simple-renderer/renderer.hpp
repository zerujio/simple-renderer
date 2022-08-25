#ifndef SIMPLERENDERER_RENDERER_HPP
#define SIMPLERENDERER_RENDERER_HPP

#include "shader_program.hpp"
#include "camera.hpp"

#include <glutils/guard.hpp>
#include <glutils/program.hpp>
#include <glutils/buffer.hpp>
#include <glutils/vertex_array.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <map>
#include <vector>

namespace simple {

    /// Performs rendering operations.
    class Renderer
    {
    public:
        /// Performs all initialization required for rendering.
        Renderer();

        /// Enqueue a draw command.
        void draw(const ShaderProgram &program, const glm::mat4 &model_transform);

        /// Execute queued drawing commands.
        void finishFrame(const Camera& camera);

        /// Sets the viewport dimensions.
        /**
         * The viewport determines over which portion of the framebuffer the draw commands issued by finishFrame() will
         * draw. The coordinate system has its origin at the lower left corner of the screen.
         *
         * This function operates over the OpenGL context, so its effects will be seen by all Renderer objects operating
         * in the thread it was called on.
         *
         * @param lower_left Coordinates of the lower left corner of the viewport.
         * @param top_right Coordinates of the top right corner of the viewport.
         */
        static void setViewport(glm::ivec2 lower_left, glm::ivec2 top_right);

    private:
        std::map<const ShaderProgram *, std::vector<glm::mat4>> m_draw_calls;

        glutils::Guard<glutils::VertexArray> m_va {};
        glutils::Guard<glutils::Buffer> m_buffer {};
    };

} // simple

#endif //SIMPLERENDERER_RENDERER_HPP
