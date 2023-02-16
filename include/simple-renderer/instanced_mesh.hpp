#ifndef PROCEDURALPLACEMENTLIB_INSTANCED_MESH_HPP
#define PROCEDURALPLACEMENTLIB_INSTANCED_MESH_HPP

#include "mesh.hpp"

#include <vector>

namespace simple {

class InstancedMesh : public Mesh
{
public:
    using Mesh::Mesh;

    void collectDrawCommands(const CommandCollector &collector) const override;

    /// Adjust how many instances of this mesh are drawn.
    void setInstanceCount(std::uint32_t instance_count)
    { m_instance_count = instance_count; }

    [[nodiscard]] std::uint32_t getInstanceCount() const
    { return m_instance_count; }

    template<typename LocationArray>
    void addInstanceData(const LocationArray &attribute_locations, const VertexAttributeSequence &instanced_attributes,
                         std::uint32_t instance_divisor, std::uint32_t instance_count, const void *instance_data)
    {
        m_addInstanceDataFromArray({std::data(attribute_locations), std::size(attribute_locations)},
                                   instanced_attributes,
                                   instance_divisor, instance_count, instance_data);
    }

    template<typename DataArray>
    void addInstanceData(std::uint32_t attribute_location, std::uint32_t instance_divisor, const DataArray &data)
    {
        m_addInstanceDataFromArray({reinterpret_cast<const std::int32_t *>(&attribute_location), 1},
                                   VertexAttributeSequence().addAttribute<std::decay_t<decltype(data[0])>>(),
                                   instance_divisor,
                                   static_cast<std::uint32_t>(std::size(data)), std::data(data));
    }

    template<typename LocationArray>
    void addInstanceData(const LocationArray &attribute_locations, const VertexAttributeSequence &instanced_attributes,
                         std::uint32_t instance_divisor, std::uint32_t instance_count,
                         GL::BufferHandle buffer, std::uintptr_t buffer_offset)
    {
        m_addInstanceDataFromBuffer({std::data(attribute_locations), std::size(attribute_locations)},
                                    instanced_attributes,
                                    instance_divisor, instance_count, buffer, buffer_offset);
    }

private:
    /// Allocate a new buffer of the given size, copy contents of current buffer, and swap them.
    void m_resizeInstanceBuffer(std::size_t new_size);

    /// Ensures that the instance buffer has enough free space to add a new section of the given size, resizing the
    /// buffer if necessary.
    void m_ensureInstanceBufferCapacity(std::size_t section_size);

    const std::pair<std::vector<std::int32_t>, std::uint32_t> &
    m_setAttributeBinding(std::size_t section_index, std::pair<const int *, std::size_t> locations,
                          std::uint32_t instance_divisor);

    void
    m_addInstanceDataFromArray(std::pair<const int *, std::size_t> locations, const VertexAttributeSequence &attributes,
                               std::uint32_t divisor, std::uint32_t count, const void *data);

    void m_addInstanceDataFromBuffer(std::pair<const int *, std::size_t> locations,
                                     const VertexAttributeSequence &attributes,
                                     std::uint32_t divisor, std::uint32_t count, GL::BufferHandle buffer,
                                     std::uintptr_t buffer_offset);

    template<typename CopyFunction>
    void m_addInstanceData(std::pair<const int *, std::size_t> locations, const VertexAttributeSequence &attributes,
                           std::uint32_t instance_divisor, std::uint32_t instance_count,
                           const CopyFunction &copy_instance_data);

    static constexpr std::size_t s_initial_buffer_size = 1024;

    std::uint32_t m_instance_count{0};
    VertexBuffer m_instance_buffer{s_initial_buffer_size};
    std::vector<std::pair<std::vector<std::int32_t>, std::uint32_t>> m_attribute_bindings;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_INSTANCED_MESH_HPP
