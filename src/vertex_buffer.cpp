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

const VertexBufferSectionDescriptor *
VertexBuffer::tryAddAttributeData(const std::function<void(WBufferRef)> &initializer,
                                  size_uint vertex_count, VertexAttributeSequence attribute_sequence)
{
    const size_uint size = attribute_sequence.getStride() * vertex_count;

    const auto offset_opt = m_allocator.tryAllocate(size);
    if (!offset_opt.has_value())
        return nullptr;

    initializer({m_buffer, *offset_opt, size});

    return &m_sections.emplace_back(attribute_sequence, vertex_count, *offset_opt);
}

const VertexBufferSectionDescriptor *
VertexBuffer::tryAddAttributeData(const void *vertex_data, VertexBuffer::size_uint vertex_count,
                                  VertexAttributeSequence attributes)
{
    const auto initializer = makeSectionInitializerFromPointer(vertex_data, vertex_count, attributes);
    return tryAddAttributeData(initializer, vertex_count, std::move(attributes));
}

const VertexBufferSectionDescriptor *
VertexBuffer::tryAddAttributeData(GL::BufferHandle read_buffer, VertexBuffer::size_uint read_offset,
                                  VertexBuffer::size_uint vertex_count, VertexAttributeSequence attributes)
{
    const auto initializer = makeSectionInitializerFromBuffer(read_buffer, read_offset, vertex_count,
                                                               attributes);
    return tryAddAttributeData(initializer, vertex_count, std::move(attributes));
}

const VertexBufferSectionDescriptor *
VertexBuffer::tryAddAttributeData(const VertexBuffer &vertex_buffer, VertexBuffer::size_uint section_index)
{
    const auto &descriptor = getSectionDescriptor(section_index);
    return tryAddAttributeData(vertex_buffer.m_buffer, descriptor.buffer_offset, descriptor.vertex_count,
                               descriptor.attributes);
}

const VertexBufferSectionDescriptor &checkSectionCreationSuccess(const VertexBufferSectionDescriptor* new_descriptor)
{
    if (!new_descriptor)
        throw std::logic_error("buffer allocation failure");

    return *new_descriptor;
}

const VertexBufferSectionDescriptor &
VertexBuffer::addAttributeData(const std::function<void(WBufferRef)> &initializer, size_uint vertex_count,
                               VertexAttributeSequence attributes)
{
    return checkSectionCreationSuccess(tryAddAttributeData(initializer, vertex_count, std::move(attributes)));
}

const VertexBufferSectionDescriptor &VertexBuffer::addAttributeData(const void *vertex_data,
                                                                    VertexBuffer::size_uint vertex_count,
                                                                    VertexAttributeSequence sequence)
{
    return checkSectionCreationSuccess(tryAddAttributeData(vertex_data, vertex_count, std::move(sequence)));
}

const VertexBufferSectionDescriptor &
VertexBuffer::addAttributeData(GL::BufferHandle read_buffer, VertexBuffer::size_uint read_offset,
                               VertexBuffer::size_uint vertex_count, VertexAttributeSequence sequence)
{
    return checkSectionCreationSuccess(tryAddAttributeData(read_buffer, read_offset, vertex_count,
                                                           std::move(sequence)));
}

const VertexBufferSectionDescriptor &
VertexBuffer::addAttributeData(const VertexBuffer &vertex_buffer, VertexBuffer::size_uint section_index)
{
    const auto &descriptor = vertex_buffer.getSectionDescriptor(section_index);
    return addAttributeData(vertex_buffer.m_buffer, descriptor.buffer_offset, descriptor.vertex_count,
                            descriptor.attributes);
}

void VertexBuffer::updateAttributeData(VertexBuffer::size_uint index, const void *data) const
{
    const auto &descriptor = getSectionDescriptor(index);
    m_buffer.write(descriptor.buffer_offset, descriptor.getSize(), data);
}

void VertexBuffer::updateAttributeData(VertexBuffer::size_uint index, GL::BufferHandle read_buffer,
                                       VertexBuffer::size_uint read_offset) const
{
    const auto &descriptor = getSectionDescriptor(index);
    GL::Buffer::copy(read_buffer, m_buffer, read_offset, descriptor.buffer_offset, descriptor.getSize());
}

void
VertexBuffer::updateAttributeData(VertexBuffer::size_uint index, const std::function<void(WBufferRef)> &initializer)
{
    const auto& descriptor = getSectionDescriptor(index);
    initializer({m_buffer, descriptor.buffer_offset, descriptor.vertex_count * descriptor.attributes.getStride()});
}

void VertexBuffer::discardAttributeData(VertexBuffer::size_uint index)
{
    if (index >= m_sections.size())
        throw std::logic_error("section index out of range");

    m_allocator.deallocate(m_sections[index].buffer_offset);
    m_sections.erase(m_sections.begin() + index);
}

std::function<void (WBufferRef)>
VertexBuffer::makeSectionInitializerFromBuffer(GL::BufferHandle buffer, std::uintptr_t offset, std::size_t vertex_count,
                                               const VertexAttributeSequence& attributes)
{
    const auto size = vertex_count * attributes.getStride();
    return [buffer, offset, size](WBufferRef ref) { ref.copyFrom({buffer, offset, size}); };
}

std::function<void(WBufferRef)>
VertexBuffer::makeSectionInitializerFromPointer(const void *data_pointer, std::size_t vertex_count,
                                                const VertexAttributeSequence &attributes)
{
    const auto size = vertex_count * attributes.getStride();

    if (!data_pointer)
        throw std::logic_error("can't create vertex buffer section initializer from null pointer");

    return [data_pointer, size] (WBufferRef ref)
    {
        if (ref.getSize() != size)
            throw std::logic_error("size of contained data does not match the provided buffer range size");
        ref.write(data_pointer);
    };
}

} // simple