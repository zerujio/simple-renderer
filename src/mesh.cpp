#include "simple-renderer/mesh.hpp"

#include "simple-renderer/glsl_definitions.hpp"

#include <utility>
#include <type_traits>

namespace simple {

using namespace glutils;

template<typename T>
std::size_t getVectorByteSize(const std::vector<T>& vector)
{
    return vector.size() * sizeof(T);
}

template<typename T>
GLintptr copyToByteArray(const std::vector<T>& data, std::vector<std::byte>& byte_array)
{
    const auto offset = byte_array.size();
    const auto size = getVectorByteSize(data);

    if (size)
    {
        byte_array.resize(byte_array.size() + size);
        auto ptr = reinterpret_cast<T*>(&byte_array[offset]);
        for (const auto& value : data)
            *ptr++ = value;
    }

    return static_cast<GLintptr>(offset);
}

Mesh::Mesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals,
           const std::vector<glm::vec2>& uvs, const std::vector<GLuint>& indices)
{
    if (positions.empty())
        throw std::runtime_error("no vertex position data");

    if (!normals.empty() && normals.size() != positions.size())
        throw std::runtime_error("position and normal data size mismatch");

    if (!uvs.empty() && uvs.size() != positions.size())
        throw std::runtime_error("position and uv data size mismatch");

    std::vector<std::byte> init_data;

    init_data.reserve(getVectorByteSize(positions) + getVectorByteSize(normals) + getVectorByteSize(uvs)
                    + getVectorByteSize(indices));
    const auto position_offset = copyToByteArray(positions, init_data);
    const auto normal_offset = copyToByteArray(normals, init_data);
    const auto uv_offset = copyToByteArray(uvs, init_data);
    const auto index_offset = copyToByteArray(indices, init_data);

    m_buffer->allocateImmutable(init_data.size(), Buffer::StorageFlags::none, init_data.data());

    using ASize = VertexArray::AttribSize;
    using AType = VertexArray::AttribType;

    // positions
    GLuint buffer_binding = 0;
    m_vertex_array->bindVertexBuffer(buffer_binding, *m_buffer, position_offset, sizeof(glm::vec3));
    const auto position_location = vertex_position_def.layout.location;
    m_vertex_array->bindAttribute(position_location, buffer_binding);
    m_vertex_array->setAttribFormat(position_location, ASize::three, AType::float_, false, 0);
    m_vertex_array->enableAttribute(position_location);

    // normals
    if (!normals.empty())
    {
        buffer_binding++;
        m_vertex_array->bindVertexBuffer(buffer_binding, *m_buffer, normal_offset, sizeof(glm::vec3));
        const auto normal_location = vertex_normal_def.layout.location;
        m_vertex_array->bindAttribute(normal_location, buffer_binding);
        m_vertex_array->setAttribFormat(normal_location, ASize::three, AType::float_, false, 0);
        m_vertex_array->enableAttribute(normal_location);
    }

    // uvs
    if (!uvs.empty())
    {
        buffer_binding++;
        m_vertex_array->bindVertexBuffer(buffer_binding, *m_buffer, uv_offset, sizeof(glm::vec2));
        const auto uv_location = vertex_uv_def.layout.location;
        m_vertex_array->bindAttribute(uv_location, buffer_binding);
        m_vertex_array->setAttribFormat(uv_location, ASize::two, AType::float_, false, 0);
        m_vertex_array->enableAttribute(uv_location);
    }

    m_draw_indexed = !indices.empty();

    // element buffer
    if (m_draw_indexed)
    {
        m_index_count = indices.size();
        m_index_buffer.type = IndexType::unsigned_int;
        m_index_buffer.offset = index_offset;
        m_vertex_array->bindElementBuffer(*m_buffer);
    }
    else
    {
        m_first_index = 0;
        m_index_count = positions.size();
    }
}

void Mesh::collectDrawCommands(const CommandCollector& collector) const
{
    if (m_draw_indexed)
    {
        if (m_instance_count)
            collector.emplace(DrawElementsInstancedCommand(m_draw_mode,
                                                           m_index_count,
                                                           m_index_buffer.type,
                                                           m_index_buffer.offset,
                                                           m_instance_count),
                              m_vertex_array.getHandle());
        else
            collector.emplace(DrawElementsCommand(m_draw_mode, m_index_count, m_index_buffer.type, m_index_buffer.offset),
                              m_vertex_array.getHandle());
    }
    else
    {
        if (m_instance_count)
            collector.emplace(DrawArraysInstancedCommand(m_draw_mode, m_first_index, m_index_count, m_instance_count),
                              m_vertex_array.getHandle());
        else
            collector.emplace(DrawArraysCommand(m_draw_mode, m_first_index, m_index_count),
                              m_vertex_array.getHandle());
    }
}

} // simple