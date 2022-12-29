#ifndef PROCEDURALPLACEMENTLIB_GPU_BUFFER_HPP
#define PROCEDURALPLACEMENTLIB_GPU_BUFFER_HPP

#include "glutils/guard.hpp"
#include "glutils/buffer.hpp"

namespace simple {

class GPUBuffer
{
public:
    using UsagePattern = glutils::Buffer::Usage;
    using StorageFlags = glutils::Buffer::StorageFlags;
    using MapFlags = glutils::Buffer::AccessFlags;
    using MapMode = glutils::Buffer::AccessMode;

    /// Create a new buffer object with no allocated memory.
    GPUBuffer() = default;

    /// Create a new buffer object, allocate a normal data store. @see allocate()
    GPUBuffer(std::size_t size, UsagePattern usage, const void* data = nullptr)
    {
        allocate(size, usage, data);
    }

    /// Create a new buffer object and allocate a static data store. @see allocateStatic()
    GPUBuffer(std::size_t size, StorageFlags flags, const void* data = nullptr)
    {
        allocateStatic(size, flags, data);
    }

    /// Return the size of the current data store, or 0 if no store has been allocated.
    [[nodiscard]] std::size_t getSize() const {return m_size;}

    /// Allocate storage for the buffer.
    void allocate(std::size_t size, UsagePattern usage_pattern, const void* data = nullptr)
    {
        m_buffer->allocate(size, usage_pattern, data);
        m_size = size;
    }

    /// Allocate static storage for the buffer. Static storage can't be resized or reassigned, but may perform better.
    void allocateStatic(std::size_t size, StorageFlags flags, const void* data = nullptr)
    {
        m_buffer->allocateImmutable(size, flags, data);
        m_size = size;
    }

    /**
     * @brief Write data to buffer.
     * The contents of the memory pointed to by @p data will be copied to the range [@p offset, @p offset + @p size)
     * within the buffer.
     * @param offset Offset into the buffer, in bytes.
     * @param size Amount of the data to copy to the buffer, in bytes.
     * @param data Pointer to the data to be copied.
     */
    void write(std::size_t offset, std::size_t size, const void* data)
    {
        m_buffer->write(static_cast<glutils::GLsizeiptr>(offset), static_cast<glutils::GLsizeiptr>(size), data);
    }

    /**
     * @brief Read data from buffer.
     * The contents of the range [@p offset, @p offset + @p size) within the buffer will be copied to CPU memory at
     * the location pointed to by @p data.
     * @param offset Offset into the buffer, in bytes.
     * @param size Amount of the data to copy from the buffer, in bytes.
     * @param data Pointer to memory to copy read data into.
     */
    void read(std::size_t offset, std::size_t size, void* data) const
    {
        m_buffer->read(static_cast<glutils::GLsizeiptr>(offset), static_cast<glutils::GLsizeiptr>(size), data);
    }

    /// Map the whole memory range of the buffer.
    [[nodiscard]]
    void* map(MapMode mode) const
    {
        return m_buffer->map(mode);
    }

    /// Map the whole memory range of the buffer and cast the resulting pointer to the given type.
    template<typename T>
    [[nodiscard]]
    T* map(MapMode mode) const {return static_cast<T*>(map(mode));}

    template<typename T, MapMode Mode>
    [[nodiscard]]
    auto map() const
    {
        if constexpr (Mode == MapMode::read_only)
            return static_cast<const T*>(map(Mode));
        else
            return static_cast<T*>(map(Mode));
    }

    /// Map a range within the buffer.
    [[nodiscard]]
    void* mapRange(std::size_t offset, std::size_t size, MapFlags flags) const
    {
        return m_buffer->mapRange(static_cast<glutils::GLsizeiptr>(offset), static_cast<glutils::GLsizeiptr>(size), flags);
    }

    /// Map a range within the buffer and cast the resulting pointer T*.
    template<typename T>
    [[nodiscard]]
    T* mapRange(MapMode mode) const {return static_cast<T*>(map(mode));}

    template<typename T, MapMode Mode>
    [[nodiscard]]
    auto mapRange(std::size_t offset, std::size_t size, MapFlags flags) const
    {
        if constexpr (Mode == MapMode::read_only)
            return static_cast<const T*>(map(Mode));
        else
            return static_cast<T*>(map(Mode));
    }

    /// Unmap the buffer, invalidating the pointer obtained through map() or mapRange().
    void unmap() const { m_buffer->unmap(); }

    /// Synchronize attributes with the underlying buffer object, which may cause a stall in the GPU driver.
    glutils::Buffer sync()
    {
        m_size = m_buffer->getSize();
    }

    /// Get a handle to the underlying OpenGL object.
    [[nodiscard]]
    glutils::Buffer getHandle() const { return m_buffer.getHandle(); }

    [[nodiscard]]
    glutils::GLuint getNativeHandle() const { return m_buffer->getName(); }

protected:
    glutils::Guard<glutils::Buffer> m_buffer;
    std::size_t m_size;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_GPU_BUFFER_HPP
