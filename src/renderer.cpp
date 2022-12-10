#include "simple-renderer/renderer.hpp"

#include "glsl_definitions.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

using namespace glutils;

namespace simple {


    void Renderer::setViewport(glm::ivec2 lower_left, glm::ivec2 top_right)
    {
        gl.Viewport(lower_left.x, lower_left.y, top_right.x, top_right.y);
    }


    void Renderer::draw(const ShaderProgram &program, const Mesh &mesh, const glm::mat4 &model_transform)
    {
        m_draw_call_data.push_back({&program, &mesh, model_transform});
    }

    void Renderer::draw(const ShaderProgram &program, const Mesh &mesh, glm::mat4 &&model_transform)
    {
        m_draw_call_data.push_back({&program, &mesh, model_transform});
    }


    void Renderer::finishFrame(const Camera &camera)
    {
        gl.Clear(GL_COLOR_BUFFER_BIT);
        //gl.PointSize(2.5f);

        camera.bindUniformBlock();

        for (const DrawCall& draw_call : m_draw_call_data)
            m_scratch_buffer.push_back(&draw_call);

        std::sort(m_scratch_buffer.begin(), m_scratch_buffer.end(),
                  [](const DrawCall* l, const DrawCall* r)
                  {
                      const auto l_program = l->program->m_program->getName();
                      const auto r_program = r->program->m_program->getName();

                      if (l_program < r_program)
                          return true;

                      const auto l_mesh = l->mesh->m_vertex_array->getName();
                      const auto r_mesh = r->mesh->m_vertex_array->getName();

                      return l_program == r_program && l_mesh < r_mesh;
                  });

        glutils::Program current_program;
        glutils::VertexArray current_vertex_array;

        for (const DrawCall* draw_call : m_scratch_buffer)
        {
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
            {
                if (mesh->usesInstancedDrawing())
                    gl.DrawElementsInstanced(mesh->m_draw_mode,
                                             mesh->getElementCount(),
                                             mesh->getIndexType(),
                                             reinterpret_cast<void*>(mesh->getElementBufferOffset()),
                                             mesh->getInstanceCount());
                else
                    gl.DrawElements(mesh->m_draw_mode,
                                    mesh->m_index_count,
                                    mesh->m_index_type,
                                    reinterpret_cast<const void *>(mesh->m_element_index_offset));
            }
            else
                gl.DrawArrays(mesh->m_draw_mode,
                              mesh->m_array_index_offset,
                              mesh->m_index_count);
        }

        m_draw_call_data.clear();
        m_scratch_buffer.clear();
    }

} // simple