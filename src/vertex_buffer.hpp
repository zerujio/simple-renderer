#ifndef PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP
#define PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP

#include "simple-renderer/allocation_registry.hpp"

#include "glutils/buffer.hpp"
#include "glutils/vertex_attrib_utils.hpp"

#include <vector>
#include <optional>

namespace simple {

/// Specifies the layout of a set of interleaved vertex attributes.
class VertexAttributeSequence final
{
public:
    using size_uint = GLuint;

    /// Specifies the data type and location of an attribute.
    struct AttributeDescriptor final
    {
        GL::VertexAttributeBaseType base_type;    //!< base data type of the attribute (int, float, etc.)
        GL::VertexAttributeLength length;         //!< vector element count (1, 2, 3 or 4).
        size_uint relative_offset;                //!< offset relative to the start of the attribute sequence
    };

    /// Get the stride (i.e. the size) of the whole attribute sequence, including any padding.
    [[nodiscard]] size_uint getStride() const
    { return m_stride; }

    /// Get the number of attributes in the sequence.
    [[nodiscard]] size_uint getAttributeCount() const
    { return m_attributes.size(); }

    /// Read-only access to the underlying container.
    [[nodiscard]] const std::vector<AttributeDescriptor> &getAttributes() const
    { return m_attributes; }

    /// Get the attribute at the specified index, with bounds checking.
    [[nodiscard]] const AttributeDescriptor &getAttribute(size_uint index) const
    { return m_attributes.at(index); }

    /// Access the attribute format and offset at the given index, without bounds checking.
    [[nodiscard]] const AttributeDescriptor &operator[](size_uint index) const
    { return m_attributes[index]; }

    /// constant iterator to the first element of the underlying container.
    [[nodiscard]] auto begin() const
    { return m_attributes.begin(); }

    /// past-the-end const iterator for the underlying container.
    [[nodiscard]] auto end() const
    { return m_attributes.end(); }

    /// Add an attribute to the sequence.
    VertexAttributeSequence &addAttribute(GL::VertexAttributeBaseType base_type,
                                          GL::VertexAttributeLength vector_length);

    /// Add an attribute of type @p T to the sequence.
    template<typename T>
    VertexAttributeSequence &addAttribute()
    { return addAttribute(GL::VertexAttrib::FormatEnum<T>::base_type, GL::VertexAttrib::FormatEnum<T>::length); }

    /// Add padding after the last attribute of the sequence; new attributes will be added AFTER the padding.
    VertexAttributeSequence &addPadding(size_uint bytes);

    /// Remove all attributes and padding.
    void clear();

private:
    size_uint m_stride;
    std::vector<AttributeDescriptor> m_attributes;
};

/// Wraps a (non-resizable) GL buffer object that contains vertex attributes.
class VertexBuffer final
{
public:
    using size_uint = std::uint64_t;

    /// Specifies the contents of a section of the buffer.
    struct SectionDescriptor final
    {
        template<typename Attributes>
        SectionDescriptor(Attributes &&attr, size_uint v_count, size_uint offset)
                : attributes(std::forward<Attributes>(attr)), vertex_count(v_count), buffer_offset(offset)
        {}

        [[nodiscard]] size_uint getSize() const
        { return vertex_count * attributes.getStride(); }

        VertexAttributeSequence attributes; //!< the sequence of attributes that composes each vertex.
        size_uint vertex_count;             //!< number of vertices (instances of the attribute sequence)
        size_uint buffer_offset;            //!< byte offset of the data, relative to the start of the buffer.
    };

    /// Construct a vertex buffer with fixed storage size.
    explicit VertexBuffer(size_uint size);

    /// Return the size of the buffer, in bytes.
    [[nodiscard]] size_uint getBufferSize() const noexcept
    { return m_size; }

    /// Obtain a handle to the underlying GL buffer object.
    [[nodiscard]] GL::BufferHandle getBufferHandle() const noexcept
    { return m_buffer; }

    /// Get the number of sections the buffer's data is divided into.
    [[nodiscard]] size_uint getSectionCount() const noexcept
    { return m_sections.size(); }

    /// Access a container with descriptors for the contents of the buffer.
    [[nodiscard]] const std::vector<SectionDescriptor> &getSectionDescriptors() const noexcept
    { return m_sections; }

    /// Get the descriptor for the buffer section with the given index, with bounds checking.
    [[nodiscard]] const SectionDescriptor &getSectionDescriptor(size_uint index) const noexcept
    { return m_sections.at(index); }

    /// Directly access the descriptor for the buffer section with the given index, without bounds checking.
    [[nodiscard]] const SectionDescriptor &operator[](size_uint index) const noexcept
    { return m_sections[index]; }

    /// Copy vertex data from host memory into the buffer, creating a new section.
    /**
     * This function copies @p vertex_count * @p vertex_attrib_sequence.getStride() bytes from the vertex array pointed
     * to by @p vertex_data.
     *
     * References to section descriptors remain valid. All existing sections retain their format and vertex count, but
     * their offset may change.
     * @param vertex_data A pointer to vertex data to copy into the buffer.
     * @param vertex_count The number of elements in @p vertex_data .
     * @param vertex_attrib_sequence The format of each element in @p vertex_data .
     * @return the SectionDescriptor for the buffer section that contains the data that was copied.
     */
    const SectionDescriptor &addAttributeData(void *vertex_data, size_uint vertex_count,
                                              VertexAttributeSequence vertex_attrib_sequence);

    /// Copy vertex data from a buffer into the vertex buffer, creating a new section.
    /**
     * This function copies @p vertex_count * @p vertex_attribute_sequence.getStride() bytes from @p read_buffer to
     * the vertex buffer, starting at @p read_offset
     *
     * References to section descriptors and the corresponding buffer sections remain valid. All existing sections
     * retain their format and vertex count, but their offset may change.
     * @param read_buffer a object buffer to copy from.
     * @param read_offset a byte offset into @p read_buffer .
     * @param vertex_count number of elements to copy.
     * @param vertex_attribute_sequence format of the data to be copied.
     * @return
     */
    const SectionDescriptor &addAttributeData(GL::BufferHandle read_buffer, size_uint read_offset,
                                              size_uint vertex_count,
                                              VertexAttributeSequence vertex_attribute_sequence);

    /// Copy vertex data from another vertex buffer.
    const SectionDescriptor &addAttributeData(const VertexBuffer &vertex_buffer, size_uint section_index);

    /// Same as addAttributeData() but returns nullptr on failure instead of throwing an exception.
    const SectionDescriptor *tryAddAttributeData(void *vertex_data, size_uint vertex_count,
                                                 VertexAttributeSequence attributes);

    const SectionDescriptor *tryAddAttributeData(GL::BufferHandle read_buffer, size_uint read_offset,
                                                 size_uint vertex_count, VertexAttributeSequence attributes);

    const SectionDescriptor *tryAddAttributeData(const VertexBuffer &vertex_buffer, size_uint section_index);

    /// Update the contents of a data section, without changing the format or number of vertices.
    /**
     * No references to section descriptors are invalidated and the descriptor for the updated section remains
     * unchanged.
     * @param index the index of the section to be updated
     * @param data the data to update the section with. Must have the same size and format as the section being updated.
     */
    void updateAttributeData(size_uint index, void *data) const;

    void updateAttributeData(size_uint index, GL::BufferHandle read_buffer,
                             size_uint read_offset) const;

    /// Discard the data section with the given index.
    /**
     * References to the section descriptor and those with a greater index will be invalidated. The contents of all
     * but the discarded section remain valid.
     * @param index The index of the section to discard.
     */
    void discardAttributeData(size_uint index);

private:
    template<typename Initializer>
    [[nodiscard]]
    const SectionDescriptor *m_tryCreateSection(const Initializer &initializer,
                                                VertexAttributeSequence &&attribute_sequence,
                                                size_uint vertex_count);

    template<typename Initializer>
    [[nodiscard]]
    const SectionDescriptor &m_createSection(const Initializer &initializer,
                                             VertexAttributeSequence &&attribute_sequence,
                                             size_uint vertex_count);

    GL::Buffer m_buffer;
    size_uint m_size;
    AllocationRegistry m_allocator;
    std::vector<SectionDescriptor> m_sections;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP
