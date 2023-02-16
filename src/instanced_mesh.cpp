#include "simple-renderer/instanced_mesh.hpp"

namespace simple {

void InstancedMesh::collectDrawCommands(const Drawable::CommandCollector &collector) const
{
    if (isIndexed())
        m_emplaceDrawCommand(collector,
                             DrawElementsInstancedCommand(m_createDrawElementsCommand(), m_instance_count));
    else
        m_emplaceDrawCommand(collector,
                             DrawArraysInstancedCommand(m_createDrawArraysCommand(), m_instance_count));
}

void InstancedMesh::m_resizeInstanceBuffer(std::size_t new_size)
{
    VertexBuffer new_buffer{new_size};

    for (std::size_t section_index = 0; section_index < m_instance_buffer.getSectionCount(); section_index++)
    {
        const auto &section_descriptor = new_buffer.addAttributeData(m_instance_buffer, section_index);
        const auto &[attribute_locations, instance_divisor] = m_attribute_bindings[section_index];
        m_getVertexAttributes().bindAttributes(new_buffer, section_descriptor, attribute_locations, instance_divisor);
    }

    m_instance_buffer = std::move(new_buffer);
}

const std::pair<std::vector<std::int32_t>, std::uint32_t> &
InstancedMesh::m_setAttributeBinding(std::size_t section_index, std::pair<const int *, std::size_t> locations,
                                     std::uint32_t instance_divisor)
{
    if (section_index >= m_attribute_bindings.size())
        m_attribute_bindings.resize(section_index + 1);

    auto &pair = m_attribute_bindings[section_index];
    auto &[current_locations, current_divisor] = pair;

    current_locations.clear();
    current_locations.insert(current_locations.end(), locations.first, locations.first + locations.second);

    current_divisor = instance_divisor;

    return pair;
}

void InstancedMesh::m_ensureInstanceBufferCapacity(std::size_t section_size)
{
    if (m_instance_buffer.getMaxNewSectionSize() >= section_size)
        return;

    auto new_size = m_instance_buffer.getBufferSize() * 2;
    while (new_size < m_instance_buffer.getBufferSize() + section_size)
        new_size *= 2;
    m_resizeInstanceBuffer(new_size);
}

void InstancedMesh::m_addInstanceDataFromArray(std::pair<const int *, std::size_t> locations,
                                               const VertexAttributeSequence &attributes, std::uint32_t divisor,
                                               std::uint32_t count, const void *data)
{
    m_addInstanceData(locations, attributes, divisor, count,
                      [data, count, &attributes](VertexBuffer& vertex_buffer)
                      { return vertex_buffer.addAttributeData(data, count, attributes); });
}

void InstancedMesh::m_addInstanceDataFromBuffer(std::pair<const int *, std::size_t> locations,
                                                const VertexAttributeSequence &attributes, std::uint32_t divisor,
                                                std::uint32_t count, GL::BufferHandle buffer,
                                                std::uintptr_t buffer_offset)
{
    m_addInstanceData(locations, attributes, divisor, count,
                      [buffer, buffer_offset, count, &attributes](VertexBuffer &vertex_buffer)
                      { return vertex_buffer.addAttributeData(buffer, buffer_offset, count, attributes); });
}

template<typename CopyFunction>
void InstancedMesh::m_addInstanceData(std::pair<const int *, std::size_t> locations,
                                      const VertexAttributeSequence &attributes, std::uint32_t instance_divisor,
                                      std::uint32_t instance_count, const CopyFunction &copy_function)
{
    if (locations.second != attributes.getAttributeCount())
        throw std::logic_error("provided a different number of attributes and attribute locations");

    m_ensureInstanceBufferCapacity(instance_count * attributes.getStride());

    const auto &section_descriptor = copy_function(m_instance_buffer);

    const auto &[location_vector, _] = m_setAttributeBinding(m_instance_buffer.getSectionCount() - 1,
                                                             locations, instance_count);

    m_getVertexAttributes().bindAttributes(m_instance_buffer, section_descriptor, location_vector, instance_divisor);
}

} // simple