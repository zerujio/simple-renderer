#ifndef PROCEDURALPLACEMENTLIB_GPU_BUFFER_HPP
#define PROCEDURALPLACEMENTLIB_GPU_BUFFER_HPP

#include "glutils/guard.hpp"
#include "glutils/buffer.hpp"

namespace simple {

class GPUBuffer
{
public:
    using MapFlags = glutils::Buffer::AccessFlags;
    using MapMode = glutils::Buffer::AccessMode;

    /**
     * @brief Write data to buffer.
     * The contents of the memory pointed to by @p data will be copied to the range [@p offset, @p offset + @p size)
     * within the buffer.
     * @param offset Offset into the buffer, in bytes.
     * @param size Amount of the data to copy to the buffer, in bytes.
     * @param data Pointer to the data to be copied.
     */
    void write(std::size_t offset, std::size_t size, const void* data) const
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

    /// Get the native handle of the buffer.
    [[nodiscard]]
    glutils::Buffer getHandle() const { return m_buffer.getHandle(); }

    [[nodiscard]]
    glutils::GLuint getNativeHandle() const { return m_buffer->getName(); }

protected:
    glutils::Guard<glutils::Buffer> m_buffer;
};

/**
 * @brief Wraps a non-resizable buffer in the graphics API.
 */
class StaticGPUBuffer final : public GPUBuffer
{
public:
    using StorageFlags = glutils::Buffer::StorageFlags;

    /**
     * @brief Create a new non-resizable buffer.
     * @param size Size in bytes to allocate for the buffer.
     * @param flags Storage options.
     * @param data Pointer to data to initialize the buffer with. May be nullptr.
     */
    explicit StaticGPUBuffer(std::size_t size, StorageFlags flags = StorageFlags::none, const void* data = nullptr)
    {
        m_buffer->allocateImmutable(static_cast<glutils::GLsizeiptr>(size), flags, data);
    }

    [[nodiscard]]
    StorageFlags getStorageFlags() const
    {
        return m_buffer->getStorageFlags();
    }
};

/**
 * @brief Wraps a resizable buffer in the graphics API.
 */
class DynamicGPUBuffer final : public GPUBuffer
{
    using Usage = glutils::Buffer::Usage;

    /**
     * @brief Create a new, resizable buffer.
     * @param size Size in bytes of the initial capacity. May be zero.
     * @param usage Usage pattern for the buffer.
     * @param initial_data Pointer to data to initialize the buffer with. May be nullptr.
     */
    explicit DynamicGPUBuffer(std::size_t size, Usage usage, const void* initial_data = nullptr)
    {
        resize(size, usage, initial_data);
    }

     /**
      * @brief Change the size of the buffer.
      * Data contained within the buffer is lost when resized.
      * @param size Size in bytes of the initial capacity. May be zero.
      * @param usage Usage pattern for the buffer.
      * @param initial_data Pointer to data to initialize the buffer with. May be nullptr.
      */
     void resize(std::size_t size, Usage usage, const void* initial_data = nullptr) const
     {
         m_buffer->allocate(static_cast<glutils::GLsizeiptr>(size), usage, initial_data);
     }
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_GPU_BUFFER_HPP
