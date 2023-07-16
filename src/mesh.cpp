#include <iostream>
#include "simple-renderer/mesh.hpp"

#include "simple-renderer/glsl_definitions.hpp"

namespace Simple::Renderer {

Mesh::Mesh(VertexDataInitializer<glm::vec3> positions, VertexDataInitializer<glm::vec3> normals,
           VertexDataInitializer<glm::vec2> uvs, VertexDataInitializer<unsigned int> indices)
        : m_vertex_buffer(positions, normals, uvs, indices),
          m_index_count(indices.size()),
          m_use_index_buffer(indices.size() > 0)
{
    if (!positions.size())
        throw std::logic_error("no position data");

    if (normals.size() != 0 && positions.size() != normals.size())
        throw std::logic_error("different number of positions and normals");

    if (uvs.size() != 0 && positions.size() != uvs.size())
        throw std::logic_error("different number of positions and UVs");

    m_vertex_array.bindVertexBufferAttribute<glm::vec3>(BufferIndex(0),
                                                        m_vertex_buffer.getBufferRange<0>(),
                                                        AttribIndex(vertex_position_def.layout.location));

    if (normals)
    {
        m_vertex_array.bindVertexBufferAttribute<glm::vec3>(BufferIndex(1),
                                                            m_vertex_buffer.getBufferRange<1>(),
                                                            AttribIndex(vertex_normal_def.layout.location));
    }

    if (uvs)
    {
        m_vertex_array.bindVertexBufferAttribute<glm::vec2>(BufferIndex(2),
                                                            m_vertex_buffer.getBufferRange<2>(),
                                                            AttribIndex(vertex_uv_def.layout.location));
    }

    // indices
    if (m_index_count)
    {
        m_vertex_array.bindElementBuffer(m_vertex_buffer.getBuffer());
    }
    else
    {
        m_first_index = 0;
        m_index_count = positions.size();
    }
}

void Mesh::collectDrawCommands(const Drawable::CommandCollector &collector) const
{
    if (isIndexed())
        collector.emplace(m_createDrawElementsCommand(), m_vertex_array.getGLObject());
    else
        collector.emplace(m_createDrawArraysCommand(), m_vertex_array.getGLObject());
}

DrawElementsCommand Mesh::m_createDrawElementsCommand() const
{
    return {m_draw_mode, m_index_count, IndexType::unsigned_int,
            m_vertex_buffer.getTypedRange<3>().offset.get()};
}

DrawArraysCommand Mesh::m_createDrawArraysCommand() const
{
    return {m_draw_mode, m_first_index, m_index_count};
}
} // Simple::Renderer