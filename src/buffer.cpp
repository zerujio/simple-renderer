#include "simple_renderer/buffer.hpp"

#include <stdexcept>

namespace Simple::Renderer {

Buffer::Buffer(Buffer::size_t size, const void *data) : m_buffer(), m_size(m_buffer ? size : 0)
{
    if (!m_buffer)
        throw std::runtime_error("GL buffer object creation failed");

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
                     static_cast<GLintptr>(from.getOffset().get()),
                     static_cast<GLintptr>(to.getOffset().get()),
                     static_cast<GLsizeiptr>(from.getSize()));
}

} // Simple::Renderer
