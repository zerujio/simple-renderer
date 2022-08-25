#include "simple-renderer/renderer.hpp"

#include "glsl_definitions.hpp"

#include <glm/gtc/type_ptr.hpp>

using namespace glutils;

namespace simple {

    Renderer::Renderer()
    {
        glm::vec2 zero_vector;
        m_buffer->allocate(sizeof(glm::vec2), Buffer::Usage::static_read, glm::value_ptr(zero_vector));

        m_va->bindVertexBuffer(0, *m_buffer, 0, 0);
        m_va->bindAttribute(0, 0);
        m_va->setAttribFormat(0, VertexArray::AttribSize::two, VertexArray::AttribType::float_, false,
                              sizeof zero_vector);
        m_va->enableAttribute(0);
    }

    void Renderer::draw(const ShaderProgram &program, const glm::mat4 &model_transform)
    {
        m_draw_calls[&program].emplace_back(model_transform);
    }

    void Renderer::finishFrame(const Camera &camera)
    {
        gl.Clear(GL_COLOR_BUFFER_BIT);
        gl.PointSize(2.5f);
        m_va->bind();

        camera.bindUniformBlock();

        for (auto &program_pair : m_draw_calls)
        {
            auto program = program_pair.first;
            program->m_program->use();
            auto &transforms = program_pair.second;
            for (const auto &tr : transforms)
            {
                gl.UniformMatrix4fv(model_matrix_def.layout.location, 1, false, glm::value_ptr(tr));
                gl.DrawArrays(GL_POINTS, 0, 1);
            }
            transforms.clear();
        }
    }

    void Renderer::setViewport(glm::ivec2 lower_left, glm::ivec2 top_right)
    {
        gl.Viewport(lower_left.x, lower_left.y, top_right.x, top_right.y);
    }

} // simple