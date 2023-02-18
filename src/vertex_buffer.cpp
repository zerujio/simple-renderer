#include "simple-renderer/vertex_buffer.hpp"

namespace simple {

///////////////////////////////////////////////// VertexAttributeSequence //////////////////////////////////////////////

VertexAttributeSequence &VertexAttributeSequence::addAttribute(GL::VertexAttributeBaseType base_type,
                                                               GL::VertexAttributeLength vector_length)
{
    m_attributes.push_back({base_type, vector_length, m_stride});
    m_stride += GL::VertexAttrib::getSizeOfBaseType(base_type) * GL::VertexAttrib::toLengthValue(vector_length);
    return *this;
}

VertexAttributeSequence &VertexAttributeSequence::addPadding(size_uint size_bytes)
{
    m_stride += size_bytes;
    return *this;
}

void VertexAttributeSequence::clear()
{
    m_stride = 0;
    m_attributes.clear();
}

///////////////////////////////////////////////// Vertex buffer ////////////////////////////////////////////////////////

VertexBuffer::VertexBuffer(VertexBuffer::size_uint size) : m_allocator(size), m_size(size)
{
    m_buffer.allocateImmutable(size, GL::Buffer::StorageFlags::dynamic_storage);
}

template<typename Initializer>
const VertexBufferSectionDescriptor *
VertexBuffer::m_tryCreateSection(const Initializer &initializer, VertexAttributeSequence &&attribute_sequence,
                                 VertexBuffer::size_uint vertex_count)
{
    const size_uint size = attribute_sequence.getStride() * vertex_count;

    const auto offset_opt = m_allocator.tryAllocate(size);
    if (!offset_opt.has_value())
        return nullptr;

    initializer(m_buffer, *offset_opt, size);

    return &m_sections.emplace_back(attribute_sequence, vertex_count, *offset_opt);
}

template<typename Initializer>
const VertexBufferSectionDescriptor& VertexBuffer::m_createSection(const Initializer &initializer,
                                                                     VertexAttributeSequence &&attribute_sequence,
                                                                     VertexBuffer::size_uint vertex_count)
{
    const auto section_descriptor_ptr = m_tryCreateSection(initializer, std::move(attribute_sequence), vertex_count);
    if (!section_descriptor_ptr)
        throw std::logic_error("vertex buffer out of memory");
    return *section_descriptor_ptr;
}

// TODO: make the InitializeFromDevice and InitializeFromHost functions public and use the new RBufferRef class.
struct InitializeFromDevice final
{
    using size_uint = VertexBuffer::size_uint;

    GL::BufferHandle read_buffer;
    size_uint read_offset {0};

    void operator() (GL::BufferHandle write_buffer, size_uint write_offset, size_uint size) const
    {
        GL::Buffer::copy(read_buffer, write_buffer, read_offset, write_offset, size);
    }
};

struct InitializeFromHost final
{
    using size_uint = VertexBuffer::size_uint;

    const void* data;

    void operator() (GL::BufferHandle write_buffer, size_uint write_offset, size_uint size) const
    {
        write_buffer.write(write_offset, size, data);
    }
};

const VertexBufferSectionDescriptor &VertexBuffer::addAttributeData(const void *vertex_data,
                                                                    VertexBuffer::size_uint vertex_count,
                                                                    VertexAttributeSequence sequence)
{
    return m_createSection(InitializeFromHost{vertex_data}, std::move(sequence), vertex_count);
}

const VertexBufferSectionDescriptor &
VertexBuffer::addAttributeData(GL::BufferHandle read_buffer, VertexBuffer::size_uint read_offset,
                               VertexBuffer::size_uint vertex_count, VertexAttributeSequence sequence)
{
    return m_createSection(InitializeFromDevice{read_buffer, read_offset}, std::move(sequence), vertex_count);
}

const VertexBufferSectionDescriptor &
VertexBuffer::addAttributeData(const VertexBuffer &vertex_buffer, VertexBuffer::size_uint section_index)
{
    const auto& descriptor = vertex_buffer.getSectionDescriptor(section_index);
    return addAttributeData(vertex_buffer.m_buffer, descriptor.buffer_offset, descriptor.vertex_count,
                            descriptor.attributes);
}

const VertexBufferSectionDescriptor *
VertexBuffer::tryAddAttributeData(const void *vertex_data, VertexBuffer::size_uint vertex_count,
                                  VertexAttributeSequence attributes)
{
    return m_tryCreateSection(InitializeFromHost{vertex_data}, std::move(attributes), vertex_count);
}

const VertexBufferSectionDescriptor *
VertexBuffer::tryAddAttributeData(GL::BufferHandle read_buffer, VertexBuffer::size_uint read_offset,
                                  VertexBuffer::size_uint vertex_count, VertexAttributeSequence attributes)
{
    return m_tryCreateSection(InitializeFromDevice{read_buffer, read_offset}, std::move(attributes), vertex_count);
}

const VertexBufferSectionDescriptor *
VertexBuffer::tryAddAttributeData(const VertexBuffer &vertex_buffer, VertexBuffer::size_uint section_index)
{
    const auto& descriptor = getSectionDescriptor(section_index);
    return tryAddAttributeData(vertex_buffer.m_buffer, descriptor.buffer_offset, descriptor.vertex_count,
                               descriptor.attributes);
}

void VertexBuffer::updateAttributeData(VertexBuffer::size_uint index, const void *data) const
{
    const auto& descriptor = getSectionDescriptor(index);
    m_buffer.write(descriptor.buffer_offset, descriptor.getSize(), data);
}

void VertexBuffer::updateAttributeData(VertexBuffer::size_uint index, GL::BufferHandle read_buffer,
                                       VertexBuffer::size_uint read_offset) const
{
    const auto& descriptor = getSectionDescriptor(index);
    GL::Buffer::copy(read_buffer, m_buffer, read_offset, descriptor.buffer_offset, descriptor.getSize());
}

void VertexBuffer::discardAttributeData(VertexBuffer::size_uint index)
{
    m_sections.erase(m_sections.begin() + index);
}

} // simple