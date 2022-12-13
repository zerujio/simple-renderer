#ifndef SIMPLERENDERER_RENDERER_HPP
#define SIMPLERENDERER_RENDERER_HPP

#include "shader_program.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "command_queue.hpp"
#include "draw_command.hpp"

#include "glutils/guard.hpp"
#include "glutils/program.hpp"
#include "glutils/buffer.hpp"
#include "glutils/vertex_array.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#include <vector>

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
    void draw(const Drawable& drawable, const ShaderProgram& program, const glm::mat4& model_transform);

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
    using UniformData = glm::mat4;
    std::vector<UniformData> m_uniform_data;

    /// stores commands and arguments
    using RendererCommandQueue = RendererCommandSet::Instantiate<CommandQueue>;

    RendererCommandQueue m_command_queue;

    /// holds commands in the order they will be executed
    std::vector<std::tuple<glutils::Program, glutils::VertexArray, const UniformData*, const DrawCommand*>>
            m_command_sequence;

    struct CommandSequenceBuilder;
};

} // simple

#endif //SIMPLERENDERER_RENDERER_HPP
