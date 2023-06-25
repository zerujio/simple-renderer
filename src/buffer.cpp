#include "buffer.hpp"

#include <stdexcept>

namespace Simple::Renderer {

Buffer::Buffer(Buffer::size_t size, void *data) : m_size(size)
{
    if (!m_buffer)
        throw std::runtime_error("failed to create OpenGL buffer");

    m_buffer.allocateImmutable(size, GL::Buffer::StorageFlags::none, data);

}

} // Simple::Renderer
