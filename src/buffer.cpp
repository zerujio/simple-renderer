#include "simple-renderer/buffer.hpp"

#include <stdexcept>

namespace Simple::Renderer {

Buffer::Buffer(Buffer::size_t size, void *data) : m_size(m_buffer ? size : 0)
{
    if (!m_buffer)
        return;

    m_buffer.allocateImmutable(static_cast<GLsizeiptr>(size), GL::Buffer::StorageFlags::none, data);
}

template<>
void Buffer::copy<std::byte>(const ConstBufferRange<std::byte>& from, const BufferRange<std::byte>& to)
{
#if SIMPLE_RENDERER_DEBUG
    if (from.getSize() != to.getSize())
        throw std::logic_error("copy between buffer ranges of different size");
#endif
    GL::Buffer::copy(from.m_buffer, to.m_buffer,
                     static_cast<GLintptr>(from.getOffset()),
                     static_cast<GLintptr>(to.getOffset()),
                     static_cast<GLsizeiptr>(from.getSize()));
}

} // Simple::Renderer
