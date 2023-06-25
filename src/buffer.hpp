#ifndef SIMPLERENDERER_BUFFER_HPP
#define SIMPLERENDERER_BUFFER_HPP

#include <cstdint>

#include "glutils/buffer.hpp"

namespace Simple::Renderer {

/// A GPU memory buffer of fixed size.
class Buffer final
{
public:
    using size_t = std::uint64_t;

    /**
     * @brief Construct a buffer of the specified size.
     * @param size The size of the buffer, in bytes.
     * @param data The data to initialize the buffer with. If null, the buffer's contents will be uninitialized.
     */
    explicit Buffer(size_t size, void* data = nullptr);

    /// Returns the size of the buffer in bytes.
    [[nodiscard]] size_t getSize() const { return m_size; }

private:
    GL::Buffer m_buffer;
    size_t m_size;
};

} // Simple::Renderer

#endif //SIMPLERENDERER_BUFFER_HPP
