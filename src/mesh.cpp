#include <iostream>
#include "simple-renderer/mesh.hpp"

#include "simple-renderer/glsl_definitions.hpp"

namespace Simple {

// old implementation

Mesh::Mesh(VertexDataInitializer<glm::vec3> positions, VertexDataInitializer<glm::vec3> normals,
           VertexDataInitializer<glm::vec2> uvs, VertexDataInitializer<unsigned int> indices)
        : m_vertex_buffer(positions.sizeBytes() + normals.sizeBytes() + uvs.sizeBytes() + indices.sizeBytes()),
          m_index_count(positions.size), m_use_index_buffer(indices.size > 0)
{
    if (!positions)
        throw std::logic_error("no position data");

    if (normals.size != 0 && positions.size != normals.size)
        throw std::logic_error("different number of positions and normals");

    if (uvs.size != 0 && positions.size != uvs.size)
        throw std::logic_error("different number of positions and UVs");

    // indices
    if (indices.size)
    {
        const auto uint_attribute = VertexAttributeSequence().addAttribute<GLuint>();
        const auto &descriptor = m_vertex_buffer.addAttributeData(indices.data, indices.size, uint_attribute);
        m_index_count = indices.size;
        m_index_buffer_offset = descriptor.buffer_offset;
        m_vertex_specification.bindIndexBuffer(m_vertex_buffer);
    }
    else
    {
        m_first_index = 0;
        m_index_count = positions.size;
    }

    // vertex positions
    {
        const auto attribute = VertexAttributeSequence().addAttribute<glm::vec3>();
        const auto &descriptor = m_vertex_buffer.addAttributeData(positions.data, positions.size, attribute);
        m_vertex_specification.bindAttributes(m_vertex_buffer, descriptor,
                                              std::array{vertex_position_def.layout.location});
    }

    // vertex normals
    if (normals)
    {
        const auto attribute = VertexAttributeSequence().addAttribute<glm::vec3>();
        const auto &descriptor = m_vertex_buffer.addAttributeData(normals.data, normals.size, attribute);
        m_vertex_specification.bindAttributes(m_vertex_buffer, descriptor,
                                              std::array{vertex_normal_def.layout.location});
    }

    // vertex texture coordinates
    if (uvs)
    {
        const auto attribute = VertexAttributeSequence().addAttribute<glm::vec2>();
        const auto &descriptor = m_vertex_buffer.addAttributeData(uvs.data, uvs.size, attribute);
        m_vertex_specification.bindAttributes(m_vertex_buffer, descriptor,
                                              std::array{vertex_uv_def.layout.location});
    }
}

void Mesh::collectDrawCommands(const Drawable::CommandCollector &collector) const
{
    if (isIndexed())
        m_emplaceDrawCommand(collector, m_createDrawElementsCommand());
    else
        m_emplaceDrawCommand(collector, m_createDrawArraysCommand());
}

DrawElementsCommand Mesh::m_createDrawElementsCommand() const
{
    return {m_draw_mode, m_index_count, IndexType::unsigned_int, m_index_buffer_offset};
}

DrawArraysCommand Mesh::m_createDrawArraysCommand() const
{
    return {m_draw_mode, m_first_index, m_index_count};
}

namespace Renderer {

template<typename AttributeType, std::size_t SectionIndex>
void Mesh::m_bindAttribute(uint attrib_index)
{
    constexpr BufferIndex buffer_index {SectionIndex};
    m_vertex_array.bindVertexBufferAttribute<AttributeType>(buffer_index,
                                                            m_vertex_buffer.getBufferRange<SectionIndex>(),
                                                            AttribIndex(attrib_index));
    //m_vertex_array.setVertexBufferInstanceDivisor(buffer_index, 0);
}

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

    m_bindAttribute<glm::vec3, 0>(vertex_position_def.layout.location);

    if (normals)
    {
        m_bindAttribute<glm::vec3, 1>(vertex_normal_def.layout.location);
    }

    if (uvs)
    {
        m_bindAttribute<glm::vec2, 2>(vertex_uv_def.layout.location);
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
    return {draw_mode, m_index_count, IndexType::unsigned_int,
            m_vertex_buffer.getTypedRange<3>().offset.get()};
}

DrawArraysCommand Mesh::m_createDrawArraysCommand() const
{
    return {draw_mode, m_first_index, m_index_count};
}

}// namespace Renderer

} // namespace Simple