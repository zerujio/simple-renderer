#include "simple-renderer/instanced_mesh.hpp"

namespace simple {

void InstancedMesh::m_resizeInstanceBuffer(std::size_t new_size)
{
    VertexBuffer new_buffer{new_size};

    for (std::size_t section_index = 0; section_index < m_instance_buffer.getSectionCount(); section_index++)
    {
        const auto &section_descriptor = new_buffer.addAttributeData(m_instance_buffer, section_index);
        const auto& [attribute_locations, instance_divisor] = m_attribute_bindings[section_index];
        m_getVertexAttributes().bindAttributes(new_buffer, section_descriptor, attribute_locations, instance_divisor);
    }

    m_instance_buffer = std::move(new_buffer);
}

void InstancedMesh::m_addInstanceData(const int *locations, std::size_t location_count,
                                      const VertexAttributeSequence &attribs, std::uint32_t divisor,
                                      std::uint32_t instance_count, const void *instance_data)
{
    const auto required_section_size = instance_count * attribs.getStride();

    if (m_instance_buffer.getMaxNewSectionSize() < required_section_size)
    {
        auto new_size = m_instance_buffer.getBufferSize() * 2;
        while (new_size - m_instance_buffer.getBufferSize() < required_section_size)
            new_size *= 2;
        m_resizeInstanceBuffer(new_size);
    }

    const auto& section_descriptor = m_instance_buffer.addAttributeData(instance_data, instance_count, attribs);

    auto & [new_locations, new_divisor] = m_attribute_bindings.emplace_back();
    new_divisor = divisor;
    new_locations.insert(new_locations.end(), locations, locations + location_count);

    m_getVertexAttributes().bindAttributes(m_instance_buffer, section_descriptor, new_locations, new_divisor);
}

void InstancedMesh::collectDrawCommands(const Drawable::CommandCollector &collector) const
{
    if (isIndexed())
        m_emplaceDrawCommand(collector,
                             DrawElementsInstancedCommand(m_createDrawElementsCommand(), m_instance_count));
    else
        m_emplaceDrawCommand(collector,
                             DrawArraysInstancedCommand(m_createDrawArraysCommand(),m_instance_count));
}

} // simple