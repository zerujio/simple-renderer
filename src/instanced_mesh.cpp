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

    for (auto &[handle, data_descriptor]: m_descriptors)
    {
        const auto &section_descriptor = new_buffer.addAttributeData(m_instance_buffer,
                                                                     data_descriptor.section_index);
        data_descriptor.section_index = new_buffer.getSectionCount() - 1;
        m_getVertexAttributes().bindAttributes(new_buffer, section_descriptor, data_descriptor.attribute_locations,
                                               data_descriptor.divisor);
    }

    m_instance_buffer = std::move(new_buffer);
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

auto InstancedMesh::m_addInstanceDataFromArray(std::pair<const int *, std::size_t> locations,
                                               VertexAttributeSequence &attributes, std::uint32_t divisor,
                                               std::uint32_t count, const void *data)
-> InstanceDataHandle
{
    return m_addInstanceData(locations, attributes, count,
                             VertexBuffer::makeSectionInitializerFromPointer(data, count, attributes), divisor);
}

auto InstancedMesh::m_addInstanceDataFromBuffer(std::pair<const int *, std::size_t> locations,
                                                VertexAttributeSequence &attributes, std::uint32_t count,
                                                GL::BufferHandle buffer, std::uintptr_t buffer_offset,
                                                std::uint32_t divisor)
-> InstanceDataHandle
{
    return m_addInstanceData(locations, attributes, count,
                             VertexBuffer::makeSectionInitializerFromBuffer(buffer, buffer_offset, count, attributes),
                             divisor);
}

void InstancedMesh::m_discardBufferSection(std::uintptr_t section_index)
{
    m_instance_buffer.discardAttributeData(section_index);
    for (auto &[_, descriptor]: m_descriptors)
        if (descriptor.section_index > section_index)
            descriptor.section_index--;
}

auto InstancedMesh::m_addInstanceData(std::pair<const int *, std::size_t> locations,
                                      VertexAttributeSequence &attributes,
                                      std::uint32_t count, const std::function<void(WBufferRef)> &initializer,
                                      std::uint32_t instance_divisor)
-> InstanceDataHandle
{
    if (locations.second != attributes.getAttributeCount())
        throw std::logic_error("provided a different number of attributes and attribute locations");

    m_ensureInstanceBufferCapacity(count * attributes.getStride());

    const auto &section_descriptor = m_instance_buffer.addAttributeData(initializer, count, std::move(attributes));

    const auto [iter, _] = m_descriptors.try_emplace(m_createHandle(), locations,
                                                     m_instance_buffer.getSectionCount() - 1, instance_divisor);
    const auto &[handle, descriptor] = *iter;

    m_getVertexAttributes().bindAttributes(m_instance_buffer, section_descriptor, descriptor.attribute_locations,
                                           descriptor.divisor);

    return handle;
}

void InstancedMesh::removeInstanceData(InstancedMesh::InstanceDataHandle handle)
{
    const auto iter = m_descriptors.find(handle);

    if (iter == m_descriptors.end())
        return;

    const auto &descriptor = iter->second;

    m_discardBufferSection(descriptor.section_index);

    for (auto location: descriptor.attribute_locations)
        m_getVertexAttributes().unbindAttribute(location);

    m_descriptors.erase(iter);
}

bool InstancedMesh::isHandleValid(InstanceDataHandle handle) const
{
    return m_descriptors.find(handle) != m_descriptors.end();
}

auto InstancedMesh::m_createHandle() -> InstanceDataHandle
{
    return static_cast<InstanceDataHandle>(m_next_handle++);
}

void InstancedMesh::updateInstanceData(InstanceDataHandle handle, std::uint32_t instance_count,
                                       const void *new_data)
{
    const auto& attributes = m_instance_buffer.getSectionDescriptor(m_descriptors.at(handle).section_index).attributes;
    updateInstanceData(handle, instance_count, VertexBuffer::makeSectionInitializerFromPointer(new_data, instance_count,
                                                                                               attributes));
}

void
InstancedMesh::updateInstanceData(InstanceDataHandle handle, std::uint32_t instance_count, GL::BufferHandle read_buffer,
                                  std::uintptr_t read_offset)
{
    const auto& attributes = m_instance_buffer.getSectionDescriptor(m_descriptors.at(handle).section_index).attributes;
    updateInstanceData(handle, instance_count, VertexBuffer::makeSectionInitializerFromBuffer(read_buffer, read_offset,
                                                                                              instance_count,
                                                                                              attributes));
}

void InstancedMesh::updateInstanceData(InstancedMesh::InstanceDataHandle handle, std::uint32_t instance_count,
                                       const std::function<void(WBufferRef)> &initializer)
{
    const auto iter = m_descriptors.find(handle);

    if (iter == m_descriptors.end())
        throw std::logic_error("invalid handle");

    auto &descriptor = iter->second;

    if (instance_count == m_instance_buffer.getSectionDescriptor(descriptor.section_index).vertex_count)
    {
        m_instance_buffer.updateAttributeData(descriptor.section_index, initializer);
        return;
    }

    auto old_section_descriptor = m_instance_buffer[descriptor.section_index];

    m_discardBufferSection(descriptor.section_index);

    if (instance_count > old_section_descriptor.vertex_count)
        m_ensureInstanceBufferCapacity(instance_count * old_section_descriptor.attributes.getStride());

    m_instance_buffer.addAttributeData(initializer, instance_count, std::move(old_section_descriptor.attributes));
    descriptor.section_index = m_instance_buffer.getSectionCount() - 1;
}

} // simple