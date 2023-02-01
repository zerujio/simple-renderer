#include "simple-renderer/buffer_range.hpp"

#include <stdexcept>

namespace simple {
void BufferRange::MappedPtrDeleter::operator()(const std::byte *ptr) const
{
    if (ptr)
        m_buffer.unmap();
}

std::unique_ptr<std::byte[], BufferRange::MappedPtrDeleter> BufferRange::m_map(GL::Buffer::AccessFlags access_flags) const
{
    return {static_cast<std::byte*>(m_buffer.mapRange(getOffset(), getSize(), access_flags)),
            MappedPtrDeleter(m_buffer)};
}

void BufferRange::copy(RBufferRange read_range, WBufferRange write_range)
{
    if (read_range.getSize() != write_range.getSize())
        throw std::logic_error("can't copy between ranges of different size");

    GL::Buffer::copy(read_range.m_buffer, write_range.m_buffer,
                     read_range.m_offset, write_range.m_offset,
                     read_range.m_size);
}

void BufferRange::copy(RBufferRange read_range, GL::BufferHandle write_buffer, BufferRange::offset_t write_offset)
{
    GL::Buffer::copy(read_range.m_buffer, write_buffer, read_range.m_offset, write_offset, read_range.m_size);
}

void BufferRange::copy(GL::BufferHandle read_buffer, BufferRange::offset_t read_offset, WBufferRange write_range)
{
    GL::Buffer::copy(read_buffer, write_range.m_buffer, read_offset, write_range.m_offset, write_range.m_size);
}

std::unique_ptr<const std::byte[], BufferRange::MappedPtrDeleter> RBufferRange::mapForRead() const
{
    return m_map(GL::Buffer::AccessFlags::read);
}

void RBufferRange::read(void *data)
{
    m_getBuffer().read(getOffset(), getSize(), data);
}

std::unique_ptr<std::byte[], BufferRange::MappedPtrDeleter> WBufferRange::mapForWrite() const
{
    return m_map(GL::Buffer::AccessFlags::write);
}

void WBufferRange::write(const void *data)
{
    m_getBuffer().write(getOffset(), getSize(), data);
}

std::unique_ptr<std::byte[], BufferRange::MappedPtrDeleter> RWBufferRange::map() const
{
    return m_map(GL::Buffer::AccessFlags::read | GL::Buffer::AccessFlags::write);
}
} // simple