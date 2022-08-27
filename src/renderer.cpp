#include "simple-renderer/renderer.hpp"

#include "glsl_definitions.hpp"

#include <glm/gtc/type_ptr.hpp>

using namespace glutils;

namespace simple {


    void Renderer::setViewport(glm::ivec2 lower_left, glm::ivec2 top_right)
    {
        gl.Viewport(lower_left.x, lower_left.y, top_right.x, top_right.y);
    }


    void Renderer::draw(const ShaderProgram &program, const Mesh &mesh, const glm::mat4 &model_transform)
    {
        const auto& draw_call = m_draw_calls.emplace_back(program, mesh, model_transform);
        m_draw_queue.push(&draw_call);
    }
    void Renderer::draw(const ShaderProgram &program, const Mesh &mesh, glm::mat4 &&model_transform)
    {
        m_draw_queue.push(&m_draw_calls.emplace_back(program, mesh, model_transform));
    }


    void Renderer::finishFrame(const Camera &camera)
    {
        gl.Clear(GL_COLOR_BUFFER_BIT);
        gl.PointSize(2.5f);

        camera.bindUniformBlock();

        glutils::Program current_program;
        glutils::VertexArray current_vertex_array;
        while (!m_draw_queue.empty())
        {
            const auto draw_call = m_draw_queue.top();

            // Check if shader program changed
            {
                const auto next_program = draw_call->program->m_program.getHandle();
                if (current_program.getName() != next_program.getName())
                {
                    next_program.use();
                    current_program = next_program;
                }
            }

            // Check if vertex array changed
            {
                const auto next_vertex_array = draw_call->mesh->m_vertex_array.getHandle();
                if (current_vertex_array.getName() != next_vertex_array.getName())
                {
                    next_vertex_array.bind();
                    current_vertex_array = next_vertex_array;
                }
            }

            gl.UniformMatrix4fv(model_matrix_def.layout.location, 1, false, glm::value_ptr(draw_call->transform));

            const auto mesh = draw_call->mesh;
            if (mesh->m_index_type)
                gl.DrawElements(mesh->m_draw_mode,
                                mesh->m_index_count,
                                mesh->m_index_type,
                                reinterpret_cast<const void *>(mesh->m_element_index_offset));
            else
                gl.DrawArrays(mesh->m_draw_mode,
                              mesh->m_array_index_offset,
                              mesh->m_index_count);

            m_draw_queue.pop();
        }
    }

    Renderer::DrawCall::DrawCall(const ShaderProgram &p, const Mesh &m, const glm::mat4 &t)
    : program(&p), mesh(&m), transform(t)
    {}

    Renderer::DrawCall::DrawCall(const ShaderProgram &p, const Mesh &m, glm::mat4 &&t)
    : program(&p), mesh(&m), transform(t)
    {}

    auto Renderer::DrawCallCompare::operator()(const Renderer::DrawCall *l, const Renderer::DrawCall *r) const -> bool
    {
        const auto l_program = l->program->m_program->getName();
        const auto r_program = r->program->m_program->getName();

        if (l_program < r_program)
            return true;

        const auto l_mesh = l->mesh->m_vertex_array->getName();
        const auto r_mesh = r->mesh->m_vertex_array->getName();

        return l_program == r_program && l_mesh < r_mesh;
    }
} // simple