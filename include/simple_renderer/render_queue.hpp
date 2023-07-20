#ifndef SIMPLEENGINE_RENDER_QUEUE_HPP
#define SIMPLEENGINE_RENDER_QUEUE_HPP

#include "simple_renderer/shader_program.hpp"
#include "simple_renderer/mesh.hpp"
#include "simple_renderer/camera.hpp"
#include "simple_renderer/command_queue.hpp"
#include "simple_renderer/draw_command.hpp"

#include "glutils/guard.hpp"
#include "glutils/program.hpp"
#include "glutils/buffer.hpp"
#include "glutils/vertex_array.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#include <vector>

namespace Simple::Renderer {

/// Performs rendering operations.
class RenderQueue
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

private:
    using UniformData = glm::mat4;
    std::vector<UniformData> m_uniform_data;

    /// stores commands and arguments
    using RendererCommandQueue = RendererCommandSet::Instantiate<CommandQueue>;

    RendererCommandQueue m_command_queue;

    /// holds commands in the order they will be executed
    std::vector<std::tuple<GL::ProgramHandle, GL::VertexArrayHandle, const UniformData*, const DrawCommand*>>
            m_command_sequence;

    struct CommandSequenceBuilder;
};

} // Simple::Renderer

#endif //SIMPLEENGINE_RENDER_QUEUE_HPP
