#include "simple-renderer/mesh.hpp"

#include "glsl_definitions.hpp"

#include <utility>

namespace simple {

    using namespace glutils;

    template<auto N, class T, auto P>
    auto appendVertexAttribData(const std::vector<glm::vec<N, T, P>>& from, std::vector<T>& to)
    {
        const auto offset = to.size() * sizeof(T);
        const auto stride = N * sizeof(T);
        for (const auto& v : from)
        {
            to.emplace_back(v.x);
            to.emplace_back(v.y);
            if constexpr (N > 2)
            {
                to.emplace_back(v.z);
                if constexpr (N > 3)
                    to.emplace_back(v.w);
            }
        }
        return std::pair(offset, stride);
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

        std::vector<float> init_data;
        init_data.reserve(positions.size() * 3 + normals.size() * 3 + uvs.size() * 2);
        const auto position_data = appendVertexAttribData(positions, init_data);
        const auto normal_data = appendVertexAttribData(normals, init_data);
        const auto uv_data = appendVertexAttribData(uvs, init_data);

        // *_data: first = byte offset, second = byte stride

        m_vertex_buffer->allocateImmutable(init_data.size() * sizeof(float), static_cast<Buffer::StorageFlags>(0),
                                           init_data.data());

        using ASize = VertexArray::AttribSize;
        using AType = VertexArray::AttribType;

        // positions
        GLuint buffer_binding = 0;
        m_vertex_array->bindVertexBuffer(buffer_binding, *m_vertex_buffer, position_data.first, position_data.second);
        const auto position_location = vertex_position_def.layout.location;
        m_vertex_array->bindAttribute(position_location, buffer_binding);
        m_vertex_array->setAttribFormat(position_location, ASize::three, AType::float_, false, 0);
        m_vertex_array->enableAttribute(position_location);

        // normals
        if (!normals.empty())
        {
            buffer_binding++;
            m_vertex_array->bindVertexBuffer(buffer_binding, *m_vertex_buffer, normal_data.first, normal_data.second);
            const auto normal_location = vertex_normal_def.layout.location;
            m_vertex_array->bindAttribute(normal_location, buffer_binding);
            m_vertex_array->setAttribFormat(normal_location, ASize::three, AType::float_, false, 0);
            m_vertex_array->enableAttribute(normal_location);
        }

        // colors
        if (!uvs.empty())
        {
            buffer_binding++;
            m_vertex_array->bindVertexBuffer(buffer_binding, *m_vertex_buffer, uv_data.first, uv_data.second);
            const auto uv_location = vertex_uv_def.layout.location;
            m_vertex_array->bindAttribute(uv_location, buffer_binding);
            m_vertex_array->setAttribFormat(uv_location, ASize::two, AType::float_, false, 0);
            m_vertex_array->enableAttribute(uv_location);
        }

        // element buffer
        if (!indices.empty())
        {
            m_element_index_offset = 0;
            m_index_count = indices.size();
            m_index_type = GL_UNSIGNED_INT;

            m_element_buffer->allocateImmutable(indices.size() * sizeof(unsigned int), static_cast<Buffer::StorageFlags>(0), indices.data());
            m_vertex_array->bindElementBuffer(*m_element_buffer);
        }
        else
        {
            m_array_index_offset = 0;
            m_index_count = positions.size();
            m_index_type = 0;
        }
    }

    void Mesh::setDrawMode(Mesh::DrawMode draw_mode)
    {
        m_draw_mode = static_cast<GLenum>(draw_mode);
    }

    auto Mesh::getDrawMode() const -> Mesh::DrawMode
    {
        return static_cast<DrawMode>(m_draw_mode);
    }
} // simple