#include "simple-renderer/renderer.hpp"

#include "simple-renderer/glsl_definitions.hpp"

#include "glutils/gl.hpp"

#include "glm/gtc/type_ptr.hpp"

#include <algorithm>
#include <utility>

using namespace GL;

namespace Simple::Renderer {


Context::Context()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}


void Context::setViewport(glm::ivec2 lower_left, glm::ivec2 top_right)
{
    glViewport(lower_left.x, lower_left.y, top_right.x, top_right.y);
}

void Context::draw(const Drawable &drawable, const ShaderProgram &program, const glm::mat4 &model_transform)
{
    const std::size_t uniform_data_index = m_uniform_data.size();
    m_uniform_data.emplace_back(model_transform);

    drawable.collectDrawCommands(CommandCollector(m_command_queue, uniform_data_index, program.m_program));
}

struct Context::CommandSequenceBuilder
{
    CommandSequenceBuilder(Context &renderer) : renderer(renderer)
    {}

    template<typename Command>
    void operator()(const RendererCommandQueue::CommandVector <Command> &command_vector) const
    {
        for (const auto &[command, args]: command_vector)
        {
            const auto [uniform_index, program, vertex_array] = args;
            renderer.m_command_sequence.emplace_back(program,
                                                     vertex_array,
                                                     &renderer.m_uniform_data[uniform_index],
                                                     &command);
        }
    }

    Context &renderer;
};

void Context::finishFrame(const Camera &camera)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //gl.PointSize(2.5f);

    m_command_queue.forEachCommandType(CommandSequenceBuilder(*this));

    std::sort(m_command_sequence.begin(), m_command_sequence.end());

    camera.bindUniformBlock();

    ProgramHandle bound_program{};
    VertexArrayHandle bound_vertex_array{};
    const UniformData *bound_uniform{nullptr};

    // iterate over commands in sequence, changing gl state when necessary
    for (const auto &[program, vertex_array, uniform_data, command]: m_command_sequence)
    {
        if (program != bound_program)
        {
            program.use();
            bound_program = program;
        }

        if (vertex_array != bound_vertex_array)
        {
            vertex_array.bind();
            bound_vertex_array = vertex_array;
        }

        if (uniform_data != bound_uniform)
        {
            glUniformMatrix4fv(model_matrix_def.layout.location, 1, false, glm::value_ptr(*uniform_data));
            bound_uniform = uniform_data;
        }

        (*command)();
    }

    m_command_queue.clear();
    m_command_sequence.clear();
    m_uniform_data.clear();
}

} // Simple::Renderer