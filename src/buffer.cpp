#include "simple-renderer/buffer.hpp"

#include <stdexcept>

namespace Simple::Renderer {

Buffer::Buffer(Buffer::size_t size, void *data) : m_size(m_buffer ? size : 0)
{
    if (!m_buffer)
        return;

    m_buffer.allocateImmutable(static_cast<GLsizeiptr>(size), GL::Buffer::StorageFlags::none, data);
}

void Buffer::copy(RBufferRange from, WBufferRange to)
{
#if SIMPLE_RENDERER_DEBUG
    if (from.m_range.size != to.m_range.size)
        throw std::logic_error("copy between buffer ranges of different size");
#endif
    GL::Buffer::copy(from.m_buffer, to.m_buffer,
                     static_cast<GLintptr>(from.m_range.offset),
                     static_cast<GLintptr>(to.m_range.offset),
                     static_cast<GLsizeiptr>(from.m_range.size));
}

} // Simple::Renderer
