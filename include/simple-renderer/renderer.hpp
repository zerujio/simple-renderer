#ifndef SIMPLERENDERER_RENDERER_HPP
#define SIMPLERENDERER_RENDERER_HPP

#include <glutils/guard.hpp>
#include <glutils/program.hpp>
#include <glutils/buffer.hpp>
#include <glutils/vertex_array.hpp>

#include <glm/vec2.hpp>

#include <queue>

namespace simple {

    /// Performs rendering operations.
    class Renderer {
    public:

        /// Performs all initialization required for rendering.
        Renderer();

        /// Enqueue a draw command.
        void draw(glm::vec2 position);

        /// Execute queued drawing commands.
        void finishFrame();

        /// Sets the viewport dimensions.
        /**
         * The viewport determines over which portion of the framebuffer the draw commands issued by finishFrame() will
         * draw.
         *
         * The coordinate system has its origin at the lower left corner of the screen.
         *
         * @param lower_left Coordinates of the lower left corner of the viewport.
         * @param top_right Coordinates of the top right corner of the viewport.
         */
        void setViewport(glm::ivec2 lower_left, glm::ivec2 top_right) const;

    private:
        std::vector<glm::vec2> m_cmd_q {};

        glutils::Guard<glutils::VertexArray> m_va {};
        glutils::Guard<glutils::Program> m_program {};
        glutils::Guard<glutils::Buffer> m_buffer {};
    };

} // simple

#endif //SIMPLERENDERER_RENDERER_HPP
