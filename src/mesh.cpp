#include "simple-renderer/mesh.hpp"

#include "simple-renderer/glsl_definitions.hpp"
#include "simple-renderer/vertex_buffer.hpp"

#include <utility>
#include <type_traits>
#include <algorithm>
#include <array>

namespace simple {

Mesh::BufferBindingRef
Mesh::m_assignBufferBinding(GL::BufferHandle buffer, std::uint64_t offset, std::uint32_t stride, std::uint32_t divisor)
{
    const auto unused_binding_it = std::find_if(m_binding_points.begin(), m_binding_points.end(),
                                                [](const BufferBinding &binding)
                                                { return !binding.isReferenced(); });

    if (unused_binding_it != m_binding_points.end())
        return unused_binding_it;

    return m_binding_points.insert(unused_binding_it, {buffer, offset, stride, divisor});
}

Mesh::BufferBindingRef
Mesh::m_bindVertexBuffer(const VertexBuffer &vertex_buffer, const VertexBufferSectionDescriptor &section,
                         GLuint divisor)
{
    // check for an existing binding for this buffer and section
    {
        const auto existing_binding_it = std::find(m_binding_points.begin(), m_binding_points.end(),
                                                   std::make_tuple(vertex_buffer.getBufferHandle(),
                                                                   section.buffer_offset,
                                                                   section.attributes.getStride(),
                                                                   divisor));

        if (existing_binding_it != m_binding_points.end())
            return existing_binding_it;
    }

    // assign a new binding
    const auto new_binding_ref = m_assignBufferBinding(vertex_buffer.getBufferHandle(), section.buffer_offset,
                                                       section.attributes.getStride(), divisor);
    const auto new_binding_index = m_getBufferBindingIndex(new_binding_ref);

    m_vertex_array.bindVertexBuffer(new_binding_index, vertex_buffer.getBufferHandle(), section.buffer_offset,
                                    section.attributes.getStride());
    m_vertex_array.setBindingDivisor(new_binding_index, divisor);

    return new_binding_ref;
}

Mesh::BufferBindingIndex Mesh::m_getBufferBindingIndex(BufferBindingRef ref) const
{
    return ref.getIterator() - m_binding_points.begin();
}

void Mesh::m_bindAttribute(std::uint32_t attribute_location, BufferBindingRef binding_point,
                           const VertexAttributeDescriptor& attribute)
{
    const auto binding_index = m_getBufferBindingIndex(binding_point);

    m_vertex_array.bindAttribute(attribute_location, binding_index);

    using BaseType = GL::VertexAttributeBaseType;
    switch (attribute.base_type)
    {
        case BaseType::_double:
            if (!attribute.float_cast)
            {
                m_vertex_array.setAttribLFormat(attribute_location, attribute.length, attribute.base_type,
                                                attribute.relative_offset);
                break;
            }
        case BaseType::_float:
        case BaseType::_half_float:
        case BaseType::_fixed:
            m_vertex_array.setAttribFormat(attribute_location, attribute.length, attribute.base_type,
                                           false, attribute.relative_offset);
            break;
        default:
            if (attribute.float_cast)
                m_vertex_array.setAttribFormat(attribute_location, attribute.length, attribute.base_type,
                                               attribute.normalized, attribute.relative_offset);
            else
                m_vertex_array.setAttribIFormat(attribute_location, attribute.length, attribute.base_type,
                                                attribute.relative_offset);
            break;
    }

    m_vertex_array.enableAttribute(attribute_location);

    if (attribute_location >= m_attribute_bindings.size())
        m_attribute_bindings.resize(attribute_location + 1);

    m_attribute_bindings[attribute_location] = binding_point;
}

void Mesh::m_unbindAttribute(std::uint32_t attribute_location)
{
    if (attribute_location >= m_attribute_bindings.size())
        return;

    auto& opt = m_attribute_bindings[attribute_location];
    if (!opt)
        return;

    m_vertex_array.disableAttribute(attribute_location);
    opt.reset();
}

void Mesh::m_bindAttributes(const VertexBuffer &vertex_buffer, const VertexBufferSectionDescriptor &section,
                            int *locations, std::size_t num_locations, GLuint instance_divisor)
{
    if (section.attributes.getAttributeCount() != num_locations)
        throw std::logic_error("number of locations specified doesn't match number of attributes in vertex buffer");

    const auto binding_ref = m_bindVertexBuffer(vertex_buffer, section, instance_divisor);

    for (const auto &attribute_descriptor: section.attributes)
        m_bindAttribute(*locations++, binding_ref, attribute_descriptor);
}

template<typename T>
std::size_t getVectorByteSize(const std::vector<T> &vector)
{
    return vector.size() * sizeof(T);
}

template<typename T>
GLintptr copyToByteArray(const std::vector<T> &data, std::vector<std::byte> &byte_array)
{
    const auto offset = byte_array.size();
    const auto size = getVectorByteSize(data);

    if (size)
    {
        byte_array.resize(byte_array.size() + size);
        auto ptr = reinterpret_cast<T *>(&byte_array[offset]);
        for (const auto &value: data)
            *ptr++ = value;
    }

    return static_cast<GLintptr>(offset);
}

ArrayMesh::ArrayMesh(const glm::vec3 *position_data, std::size_t position_count, const glm::vec3 *normal_data,
                     std::size_t normal_count, const glm::vec2 *uv_data, std::size_t uv_count)
        : m_vertex_buffer((position_count + normal_count) * sizeof(glm::vec3) + uv_count * sizeof(glm::vec2)),
          m_vertex_count(static_cast<GLint>(position_count))
{
    if (!position_data or !position_count)
        throw std::logic_error("no vertex position data");

    m_bindAttributes(m_vertex_buffer,
                     m_vertex_buffer.addAttributeData(position_data, position_count,
                                                      VertexAttributeSequence().addAttribute<glm::vec3>()),
                     std::array<GLint, 1>{vertex_position_def.layout.location});

    if (normal_data)
    {
        if (normal_count != position_count)
            throw std::logic_error("position and normal vertex count mismatch");

        const auto & section_descriptor = m_vertex_buffer.addAttributeData(normal_data, normal_count,
                                                                           VertexAttributeSequence()
                                                                           .addAttribute<glm::vec3>());
        m_bindAttributes(m_vertex_buffer, section_descriptor, std::array<GLint, 1>{vertex_normal_def.layout.location});
    }

    if (uv_data)
    {
        if (uv_count != position_count)
            throw std::logic_error("position and uv vertex count mismatch");

        const auto &section_descriptor = m_vertex_buffer.addAttributeData(uv_data, uv_count,
                                                                          VertexAttributeSequence()
                                                                          .addAttribute<glm::vec2>());
        m_bindAttributes(m_vertex_buffer, section_descriptor, std::array<GLint, 1>{vertex_uv_def.layout.location});
    }
}

void ArrayMesh::collectDrawCommands(const Drawable::CommandCollector &collector) const
{
    emplaceDrawCommand(collector, DrawArraysCommand(getDrawMode(), 0, m_vertex_count));
}

} // simple