#include "simple_renderer/vertex_attribute_specification.hpp"
#include "simple_renderer/vertex_buffer.hpp"

#include <algorithm>

namespace Simple {

void VertexAttributeSpecification::unbindAttribute(std::uint32_t attribute_location)
{
    if (attribute_location >= m_attribute_bindings.size()
        || m_attribute_bindings[attribute_location] == s_no_binding_index)
        return;

    m_vertex_array.disableAttribute(attribute_location);
    m_attribute_bindings[attribute_location] = s_no_binding_index;
}

void VertexAttributeSpecification::bindIndexBuffer(const VertexBuffer &vertex_buffer)
{
    m_vertex_array.bindElementBuffer(vertex_buffer.getBufferHandle());
}

void VertexAttributeSpecification::unbindIndexBuffer()
{
    m_vertex_array.bindElementBuffer({}); // bind zero buffer
}

std::uint32_t VertexAttributeSpecification::m_assignBufferBinding(std::uint64_t offset, std::uint32_t stride, std::uint32_t divisor,
                                                                  GL::BufferHandle buffer)
{
    std::size_t binding_index = 0;

    for (;binding_index < m_vertex_buffer_bindings.size(); binding_index++)
    {
        if (std::count(m_attribute_bindings.begin(), m_attribute_bindings.end(), binding_index) == 0)
        {
            m_vertex_buffer_bindings[binding_index] = std::make_tuple(offset, stride, divisor, buffer);
            return binding_index;
        }
    }

    m_vertex_buffer_bindings.push_back({offset, stride, divisor, buffer});

    return binding_index;
}

std::uint32_t VertexAttributeSpecification::m_bindVertexBuffer(const VertexBuffer &vertex_buffer,
                                                               const VertexBufferSectionDescriptor &section, std::uint32_t divisor)
{
    // check for an existing binding for this buffer and section
    {
        const auto tuple = std::make_tuple(section.buffer_offset, section.attributes.getStride(), divisor,
                                           vertex_buffer.getBufferHandle());
        const auto it = std::find(m_vertex_buffer_bindings.begin(), m_vertex_buffer_bindings.end(), tuple);

        if (it != m_vertex_buffer_bindings.end())
            return it - m_vertex_buffer_bindings.begin();
    }

    // assign a new binding
    const auto new_binding = m_assignBufferBinding(section.buffer_offset, section.attributes.getStride(), divisor,
                                                   vertex_buffer.getBufferHandle());

    m_vertex_array.bindVertexBuffer(new_binding, vertex_buffer.getBufferHandle(), section.buffer_offset,
                                    section.attributes.getStride());
    m_vertex_array.setBindingDivisor(new_binding, divisor);

    return new_binding;
}

void VertexAttributeSpecification::m_bindAttribute(std::uint32_t attribute_location, GLuint buffer_binding,
                                                   const VertexAttributeDescriptor &attribute)
{
    m_vertex_array.bindAttribute(attribute_location, buffer_binding);

    using BaseType = GL::VertexAttributeType;
    switch (attribute.base_type)
    {
        case BaseType::_double:
            if (!attribute.float_cast)
            {
                m_vertex_array.setAttribLFormat(attribute_location, attribute.length, attribute.base_type,
                                                attribute.relative_offset);
                break;
            } // else: treat as float
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

    if (attribute_location >= m_attribute_bindings.size())
        m_attribute_bindings.resize(attribute_location + 1, s_no_binding_index);

    if (m_attribute_bindings[attribute_location] == s_no_binding_index)
        m_vertex_array.enableAttribute(attribute_location);

    m_attribute_bindings[attribute_location] = buffer_binding;
}

void VertexAttributeSpecification::m_bindAttributes(const VertexBuffer &vertex_buffer, const VertexBufferSectionDescriptor &section,
                                                    const int *locations, std::size_t num_locations, GLuint instance_divisor)
{
    if (section.attributes.getAttributeCount() != num_locations)
        throw std::logic_error("number of locations specified doesn't match number of attributes in vertex buffer");

    const auto binding_index = m_bindVertexBuffer(vertex_buffer, section, instance_divisor);

    for (const auto &attribute_descriptor : section.attributes)
        m_bindAttribute(*locations++, binding_index, attribute_descriptor);
}

} // simple