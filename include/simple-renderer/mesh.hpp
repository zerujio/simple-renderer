#ifndef SIMPLERENDERER_MESH_HPP
#define SIMPLERENDERER_MESH_HPP

#include "drawable.hpp"
#include "vertex_buffer.hpp"

#include "glutils/guard.hpp"
#include "glutils/buffer.hpp"
#include "glutils/vertex_array.hpp"

#include "glutils/gl_types.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include <vector>
#include <map>

namespace simple {

/// Base class for renderer-able meshes.
class Mesh : public Drawable
{
public:
    [[nodiscard]] DrawMode getDrawMode() const
    { return m_draw_mode; }

    void setDrawMode(DrawMode mode)
    { m_draw_mode = mode; }

protected:
    /// Bind attributes from a vertex buffer section to the specified attribute locations.
    /**
     *
     * @tparam Array An array-like of signed integers.
     * @param vertex_buffer The vertex buffer to source attributes from.
     * @param section_index The section within the vertex buffer that contains the attribute data.
     * @param attribute_locations array-like specifying the location each attribute in the section should be bound to.
     * If length of the array must be equal to the number of attributes in the vertex buffer section. If the location
     * assigned to a specific location is less than 0 it will be ignored.
     * @param instance_divisor if greater than zero, specifies that these are instanced attributes and every how many
     * instances their values advance.
     */
    template<typename Array>
    void m_bindAttributes(const VertexBuffer& vertex_buffer, const VertexBufferSectionDescriptor& section,
                          Array attribute_locations, GLuint instance_divisor = 0)
    {
        m_bindAttributes(vertex_buffer, section, std::data(attribute_locations), std::size(attribute_locations),
                         instance_divisor);
    }

    /// Mark an attribute as unused.
    void m_unbindAttribute(GLuint attribute_location);

    template<typename Command>
    void emplaceDrawCommand(const Drawable::CommandCollector& collector, Command&& command) const
    {
        collector.emplace(std::forward<Command>(command), m_vertex_array);
    }

private:
    class BufferBindingRef;

    class BufferBinding final
    {
        friend class BufferBindingRef;

    public:
        BufferBinding(GL::BufferHandle buffer, std::uint64_t offset, std::uint32_t stride, std::uint32_t divisor)
                : m_offset(offset), m_stride(stride), m_buffer(buffer), m_divisor(divisor)
        {}

        [[nodiscard]] bool isReferenced() const
        { return m_ref_count; }

        bool operator==(std::tuple<GL::BufferHandle, std::uint64_t, std::uint32_t, std::uint32_t> tuple)
        {
            const auto [buffer, offset, stride, divisor] = tuple;
            return m_buffer == buffer && m_offset == offset && m_stride == stride && m_divisor == divisor;
        }

    private:
        std::uint64_t m_ref_count {0};
        std::uint64_t m_offset;
        std::uint32_t m_stride;
        std::uint32_t m_divisor;
        GL::BufferHandle m_buffer;
    };


    class BufferBindingRef final
    {
    public:
        using iterator = std::vector<BufferBinding>::iterator;
        using const_iterator = std::vector<BufferBinding>::const_iterator;

        BufferBindingRef(iterator buffer_binding_it) : m_it(buffer_binding_it)
        { m_it->m_ref_count++; }

        BufferBindingRef(const BufferBindingRef &other) : m_it(other.m_it)
        { m_it->m_ref_count++; }

        ~BufferBindingRef()
        { m_it->m_ref_count--; }

        BufferBindingRef& operator= (const BufferBindingRef& other)
        {
            m_it->m_ref_count--;
            m_it = other.m_it;
            m_it->m_ref_count++;
        }

        [[nodiscard]] const_iterator getIterator() const
        { return m_it; }

    private:
        iterator m_it;
    };

    using BufferBindingIndex = GLuint;
    using AttributeLocation = GLuint;

    void m_bindAttributes(const VertexBuffer& vertex_buffer, const VertexBufferSectionDescriptor &section,
                          int *locations, std::size_t num_locations, GLuint instance_divisor);

    void m_bindAttribute(AttributeLocation attribute_location, BufferBindingRef binding_point,
                         const VertexAttributeDescriptor& attribute);

    [[nodiscard]] Mesh::BufferBindingRef
    m_assignBufferBinding(GL::BufferHandle buffer, std::uint64_t offset, std::uint32_t stride, std::uint32_t divisor);

    [[nodiscard]] Mesh::BufferBindingRef
    m_bindVertexBuffer(const VertexBuffer &vertex_buffer, const VertexBufferSectionDescriptor &section,
                       GLuint divisor);

    [[nodiscard]] BufferBindingIndex m_getBufferBindingIndex(BufferBindingRef ref) const;

    DrawMode m_draw_mode {DrawMode::points};
    GL::VertexArray m_vertex_array;
    std::vector<BufferBinding> m_binding_points;
    std::vector<std::optional<BufferBindingRef>> m_attribute_bindings;
};


/// A mesh which is drawn by iterating directly over an array of vertices.
class ArrayMesh : public Mesh
{
public:
    template<typename PositionArray, typename NormalArray, typename UVArray>
    ArrayMesh(PositionArray position_array, NormalArray normal_array, UVArray uv_array)
            : ArrayMesh(std::data(position_array), std::size(position_array),
                        std::data(normal_array), std::size(normal_array),
                        std::data(uv_array), std::size(uv_array))
    {}

    template<typename PositionArray, typename NormalArray>
    ArrayMesh(PositionArray position_array, NormalArray normal_array)
            : ArrayMesh(std::data(position_array), std::size(position_array),
                        std::data(normal_array), std::size(normal_array),
                        nullptr, 0)
    {}

    template<typename PositionArray>
    explicit ArrayMesh(PositionArray position_array)
            : ArrayMesh(std::data(position_array), std::size(position_array), nullptr, 0, nullptr, 0)
    {}

    void collectDrawCommands(const CommandCollector &collector) const override;

private:
    ArrayMesh(const glm::vec3 *position_data, std::size_t position_count,
              const glm::vec3 *normal_data, std::size_t normal_count,
              const glm::vec2 *uv_data, std::size_t uv_count);

    VertexBuffer m_vertex_buffer;
    GLint m_vertex_count {0};
};

} // simple

#endif //SIMPLERENDERER_MESH_HPP
