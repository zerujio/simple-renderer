#ifndef PROCEDURALPLACEMENTLIB_INSTANCED_MESH_HPP
#define PROCEDURALPLACEMENTLIB_INSTANCED_MESH_HPP

#include "mesh.hpp"

#include <vector>
#include <functional>

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
    void addInstanceData(const LocationArray &attribute_locations, const VertexAttributeSequence &attributes,
                         std::uint32_t instance_divisor, std::uint32_t instance_count, const void *instance_data)
    {
        m_addInstanceData(std::data(attribute_locations), std::count(attribute_locations), attributes,
                          instance_divisor, instance_count, instance_data);
    }

    template<typename DataArray>
    void addInstanceData(std::uint32_t attribute_location, std::uint32_t instance_divisor, const DataArray &data)
    {
        using AttributeType = std::decay_t<decltype(data[0])>;
        m_addInstanceData(reinterpret_cast<const std::int32_t*>(&attribute_location), 1,
                          VertexAttributeSequence().addAttribute<AttributeType>(), instance_divisor,
                          static_cast<std::uint32_t>(std::size(data)), std::data(data));
    }

private:
    void m_resizeInstanceBuffer(std::size_t new_size);

    void m_addInstanceData(const int *locations, std::size_t location_count, const VertexAttributeSequence &attribs,
                           std::uint32_t divisor, std::uint32_t instance_count, const void *instance_data);

    static constexpr std::size_t s_initial_buffer_size = 1024;

    std::uint32_t m_instance_count{0};
    VertexBuffer m_instance_buffer{s_initial_buffer_size};
    std::vector<std::pair<std::vector<std::int32_t>, std::uint32_t>> m_attribute_bindings;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_INSTANCED_MESH_HPP
