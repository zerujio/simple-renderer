#ifndef SIMPLERENDERER_RENDERER_HPP
#define SIMPLERENDERER_RENDERER_HPP

#include "shader_program.hpp"
#include "mesh.hpp"
#include "camera.hpp"

#include <glutils/guard.hpp>
#include <glutils/program.hpp>
#include <glutils/buffer.hpp>
#include <glutils/vertex_array.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <queue>

namespace simple {

    /// Performs rendering operations.
    class Renderer
    {
    public:
        /**
         * @brief enqueue a draw command.
         * @param program The shader program to draw with. The reference must remain valid until finishFrame is called.
         * @param mesh The mesh to draw.
         * @param model_transform The transformation matrix, accessible in the shader as 'model_matrix'.
         */
        void draw(const ShaderProgram& program, const Mesh& mesh, const glm::mat4& model_transform);
        void draw(const ShaderProgram& program, const Mesh& mesh, glm::mat4&& model_transform);

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
        struct DrawCall
        {
            const ShaderProgram* program;
            const Mesh* mesh;
            glm::mat4 transform;

            DrawCall(const ShaderProgram& p, const Mesh& m, const glm::mat4& t);
            DrawCall(const ShaderProgram& p, const Mesh& m, glm::mat4&& t);
        };

        struct DrawCallCompare
        {
            auto operator() (const DrawCall* l, const DrawCall* r) const -> bool;
        };

        std::priority_queue<const DrawCall*, std::vector<const DrawCall*>, DrawCallCompare> m_draw_queue;
        std::vector<DrawCall> m_draw_calls;
    };

} // simple

#endif //SIMPLERENDERER_RENDERER_HPP
