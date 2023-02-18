#ifndef PROCEDURALPLACEMENTLIB_INSTANCED_MESH_HPP
#define PROCEDURALPLACEMENTLIB_INSTANCED_MESH_HPP

#include "mesh.hpp"

#include <vector>
#include <map>

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

    enum class InstanceDataHandle : std::size_t;

    // TODO: allow addInstanceData() and updateInstanceData() to work with a VertexBuffer polymorphic initializer.

    /// Add instanced attributes to the mesh.
    /**
     *
     * @tparam LocationArray An array-like of signed integer values.
     * @param attribute_locations The attribute locations for the attributes specified in @p instanced_attributes .
     * The length of this array-like must match that of @p instanced_attributes .
     * @param instanced_attributes The sequence of attributes that composes the instanced vertices.
     * @param instance_divisor Every how many drawn elements to advance the attributes.
     * @param instance_count Number of instances in the @p instance_data array.
     * @param instance_data The values of the attributes to add to the mesh.
     * @return A handle to the newly added data that can be used to update or delete it.
     */
    template<typename LocationArray>
    InstanceDataHandle addInstanceData(const LocationArray &attribute_locations,
                                       VertexAttributeSequence instanced_attributes,
                                       std::uint32_t instance_divisor, std::uint32_t instance_count,
                                       const void *instance_data)
    {
        return m_addInstanceDataFromArray({std::data(attribute_locations), std::size(attribute_locations)},
                                          instanced_attributes,
                                          instance_divisor, instance_count, instance_data);
    }

    /// Add a single instanced attribute to the mesh.
    /**
     *
     * @tparam DataArray An array-like type.
     * @param attribute_location The values of the instanced attributes.
     * @param instance_divisor Every how many drawn meshes to advance the attribute.
     * @param data An array of attribute values.
     * @return A handle to the newly added attribute data, which can be used to update or delete it.
     */
    template<typename DataArray>
    InstanceDataHandle addInstanceData(std::uint32_t attribute_location, std::uint32_t instance_divisor,
                                       const DataArray &data)
    {
        return m_addInstanceDataFromArray({reinterpret_cast<const std::int32_t *>(&attribute_location), 1},
                                          VertexAttributeSequence().addAttribute<std::decay_t<decltype(data[0])>>(),
                                          instance_divisor,
                                          static_cast<std::uint32_t>(std::size(data)), std::data(data));
    }

    /// Add instanced attributes to the mesh from a buffer.
    template<typename LocationArray>
    InstanceDataHandle addInstanceData(const LocationArray &attribute_locations,
                                       VertexAttributeSequence instanced_attributes,
                                       std::uint32_t instance_divisor, std::uint32_t instance_count,
                                       GL::BufferHandle buffer, std::uintptr_t buffer_offset)
    {
        return m_addInstanceDataFromBuffer({std::data(attribute_locations), std::size(attribute_locations)},
                                           instanced_attributes, instance_count, buffer, buffer_offset,
                                           instance_divisor);
    }

    /// Remove instanced attributes from the mesh.
    void removeInstanceData(InstanceDataHandle handle);

    /// Set new values for previously added instanced attributes.
    void updateInstanceData(InstanceDataHandle handle, std::uint32_t instance_count, const void *new_data);

    void updateInstanceData(InstanceDataHandle handle, std::uint32_t instance_count,
                            GL::BufferHandle read_buffer, std::uintptr_t read_offset);

    [[nodiscard]] bool isHandleValid(InstanceDataHandle handle) const;

private:
    struct DataDescriptor;
    struct AddDataFunctionWrapper;
    struct UpdateDataFunctionWrapper;

    /// Allocate a new buffer of the given size, copy contents of current buffer, and swap them.
    void m_resizeInstanceBuffer(std::size_t new_size);

    /// Ensures that the instance buffer has enough free space to add a new section of the given size, resizing the
    /// buffer if necessary.
    void m_ensureInstanceBufferCapacity(std::size_t section_size);

    /// Create a new unique handle.
    InstanceDataHandle m_createHandle();

    /// Add instance data from a host array.
    [[nodiscard]]
    InstanceDataHandle m_addInstanceDataFromArray(std::pair<const int *, std::size_t> locations,
                                                  VertexAttributeSequence &attributes, std::uint32_t divisor,
                                                  std::uint32_t count, const void *data);

    /// Add instance data from a device buffer.
    [[nodiscard]]
    InstanceDataHandle m_addInstanceDataFromBuffer(std::pair<const int *, std::size_t> locations,
                                                   VertexAttributeSequence &attributes, std::uint32_t count,
                                                   GL::BufferHandle buffer, std::uintptr_t buffer_offset,
                                                   std::uint32_t divisor);

    /// Add instance data using the specified function.
    [[nodiscard]]
    InstanceDataHandle m_addInstanceData(std::pair<const int *, std::size_t> locations,
                                         VertexAttributeSequence &attributes,
                                         std::uint32_t count, const AddDataFunctionWrapper &add_data,
                                         std::uint32_t instance_divisor);

    /// Replace existing data.
    void m_updateInstanceData(InstanceDataHandle handle, std::uint32_t instance_count,
                              const UpdateDataFunctionWrapper& update_data,
                              const AddDataFunctionWrapper& add_data);

    void m_discardBufferSection(std::uintptr_t section_index);

    static constexpr std::size_t s_initial_buffer_size = 1024;

    std::underlying_type_t<InstanceDataHandle> m_next_handle{0};
    std::uint32_t m_instance_count{0};
    VertexBuffer m_instance_buffer{s_initial_buffer_size};
    std::map<InstanceDataHandle, DataDescriptor> m_descriptors;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_INSTANCED_MESH_HPP
