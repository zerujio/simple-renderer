#ifndef PROCEDURALPLACEMENTLIB_VERTEX_ATTRIBUTE_SPECIFICATION_HPP
#define PROCEDURALPLACEMENTLIB_VERTEX_ATTRIBUTE_SPECIFICATION_HPP

#include "drawable.hpp"

#include "glutils/buffer.hpp"
#include "glutils/vertex_array.hpp"

#include <cstdint>
#include <vector>
#include <tuple>

namespace Simple {

class VertexBuffer;
class VertexBufferSectionDescriptor;
class VertexAttributeDescriptor;

/// Groups together attributes from one or more vertex buffers and specifies how they should be drawn.
class VertexAttributeSpecification final
{
public:
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
    void bindAttributes(const VertexBuffer &vertex_buffer, const VertexBufferSectionDescriptor &section,
                          const Array& attribute_locations, std::uint32_t instance_divisor = 0)
    {
        m_bindAttributes(vertex_buffer, section, std::data(attribute_locations), std::size(attribute_locations),
                         instance_divisor);
    }

    /// Mark an attribute as unused.
    void unbindAttribute(GLuint attribute_location);

    /// Bind an index buffer; only one index buffer may be bound at a time.
    void bindIndexBuffer(const VertexBuffer &vertex_buffer);
    void unbindIndexBuffer();

    /// Emplace a command using this vertex batch.
    template<typename Command>
    void emplaceDrawCommand(const Drawable::CommandCollector &collector, Command &&command) const
    { collector.emplace(std::forward<Command>(command), m_vertex_array); }

private:
    void m_bindAttributes(const VertexBuffer &vertex_buffer, const VertexBufferSectionDescriptor &section,
                          const int *locations, std::size_t num_locations, GLuint instance_divisor);

    void m_bindAttribute(GLuint attribute_location, GLuint buffer_binding, const VertexAttributeDescriptor &attribute);

    [[nodiscard]] std::uint32_t m_bindVertexBuffer(const VertexBuffer &vertex_buffer,
                                                   const VertexBufferSectionDescriptor &section, std::uint32_t divisor);

    [[nodiscard]] std::uint32_t m_assignBufferBinding(std::uint64_t offset, std::uint32_t stride, std::uint32_t divisor,
                                                      GL::BufferHandle buffer);

    struct VertexBufferBinding final
    {
        std::uint64_t offset{0};
        std::uint32_t stride{0};
        std::uint32_t divisor{0};
        GL::BufferHandle buffer{};

        template<typename TupleType>
        VertexBufferBinding& operator=(const TupleType& tuple)
        { std::tie(offset, stride, divisor, buffer) = tuple; return *this; }

        template<typename TupleType>
        [[nodiscard]] bool operator==(const TupleType& tuple) const
        { return std::make_tuple(offset, stride, divisor, buffer) == tuple; }
    };

    static constexpr GLuint s_no_binding_index = -1u;

    GL::VertexArray m_vertex_array;
    std::vector<VertexBufferBinding> m_vertex_buffer_bindings;
    std::vector<std::uint32_t> m_attribute_bindings;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_VERTEX_ATTRIBUTE_SPECIFICATION_HPP
