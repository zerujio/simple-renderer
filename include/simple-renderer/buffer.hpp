#ifndef SIMPLERENDERER_BUFFER_HPP
#define SIMPLERENDERER_BUFFER_HPP

#include "glutils/buffer.hpp"

#include <cstdint>
#include <type_traits>
#include <string>
#include <stdexcept>

namespace Simple::Renderer {

/// Represents a memory range, usually within a GPU buffer.
struct MemoryRange
{
    using size_t = std::size_t;

    size_t offset {0};  ///< byte offset of the memory range, usually relative to the start of a buffer.
    size_t size {0};    ///< size of the range in bytes.

    /// Constructs an empty range (zero size and offset).
    constexpr MemoryRange() noexcept = default;

    /// Construct a range with specified offset and size.
    constexpr MemoryRange(size_t offset, size_t size) noexcept : offset(offset), size(size) {}

    /**
     * @brief Creates a new object representing a subrange within *this.
     * @param relative_offset byte offset relative to the start of the current range.
     * @param sub_size size of the new range.
     * @return A new range with the specified offset and size, or an empty range if relative_offset + sub_size would
     * exceed the size of *this.
     */
    [[nodiscard]]
    constexpr MemoryRange subRange(size_t relative_offset, size_t sub_size) const noexcept
    {
        if (relative_offset + sub_size > size)
            return {};

        return {offset + relative_offset, size};
    }

    /// Joins two overlapping ranges; returns an empty range if @p l and @p r do not overlap.
    [[nodiscard]]
    static constexpr MemoryRange join(MemoryRange l, MemoryRange r) noexcept
    {
        const size_t start = std::min(l.offset, r.offset);
        const size_t end = std::max(l.offset + l.size, r.offset + r.size);
        const size_t size = end - start;

        if (l.size + r.size < size)
            return {};
        else
            return {start, size};
    }

    /// Checks if the size is zero.
    [[nodiscard]] constexpr bool isEmpty() const noexcept { return size == 0; }

    /// true if the range is not empty, false otherwise.
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return !isEmpty(); }
};

/// same as MemoryRange::join(l, r)
constexpr MemoryRange operator+(MemoryRange l, MemoryRange r) noexcept
{
    return MemoryRange::join(l, r);
}

/// A reference to a memory range within a buffer.
template<bool Read = true, bool Write = true>
class BufferRange final
{
    static_assert(Read || Write);

public:
    friend class Buffer;

    template<bool R, bool W>
    friend class BufferRange;

    using size_t = std::size_t;

    /// Constructs a range with zero size, zero offset and no buffer.
    constexpr BufferRange() noexcept = default;

    /// Construct from another range with compatible access specifiers.
    template<bool R, bool W, typename = std::enable_if_t<(!Read || R) && (!Write || W), void>>
    explicit constexpr BufferRange(const BufferRange<R, W> &other) noexcept
        : m_range(other.getRange()), m_buffer(other.m_buffer)
    {}

    [[nodiscard]]
    constexpr const MemoryRange& getMemoryRange() const { return m_range; }

    [[nodiscard]] constexpr size_t getSize() const { return m_range.size; }
    [[nodiscard]] constexpr size_t getOffset() const { return m_range.size; }

    /// A buffer range is considered valid if it references a valid (non-zero) buffer, even if the memory range itself is empty.
    [[nodiscard]]
    constexpr bool isValid() const noexcept { return !m_buffer.isZero(); }

    /// Same as isValid()
    [[nodiscard]]
    constexpr explicit operator bool() const noexcept { return isValid(); }

    /// Construct a sub range within the same buffer.
    [[nodiscard]]
    constexpr BufferRange subRange(size_t relative_offset, size_t new_size) const noexcept
    {
        const MemoryRange result = m_range.subRange(relative_offset, new_size);

        if (result)
            return {m_buffer, result};
        else
            return {};
    }

    /// Joins two overlapping ranges; returns an invalid range if @p l and @p r do not belong to the same buffer or do not overlap.
    static constexpr BufferRange join(const BufferRange& l, const BufferRange& r) noexcept
    {
        if (l.m_buffer != r.m_buffer)
            return {};

        const MemoryRange mem_range = l.getMemoryRange() + r.getMemoryRange();

        if (!mem_range)
            return {};

        return {l.m_buffer, mem_range};
    }

private:
    constexpr BufferRange(GL::BufferHandle handle, MemoryRange range) noexcept : m_range(range), m_buffer(handle) {}

    MemoryRange m_range;
    GL::BufferHandle m_buffer;
};

/// Buffer range with read and write access.
using RWBufferRange = BufferRange<true, true>;

/// Buffer range with read-only access.
using RBufferRange = BufferRange<true, false>;

/// Buffer range with write-only access.
using WBufferRange = BufferRange<false, true>;

/// A GPU memory buffer of fixed size.
class Buffer final
{
public:
    using size_t = std::size_t;

    /// Creates an empty Buffer object, which is not associated with an OpenGL buffer.
    constexpr Buffer() = default;

    /**
     * @brief Construct a buffer of the specified size.
     * @param size The size of the buffer, in bytes.
     * @param data The data to initialize the buffer with. If null, the buffer's contents will be uninitialized.
     * If buffer allocation fails an empty buffer object will be created.
     */
    explicit Buffer(size_t size, void *data = nullptr);

    /// Returns the size of the buffer in bytes.
    [[nodiscard]]
    constexpr size_t getSize() const noexcept
    {
        return m_size;
    }

    /// Get range within the buffer; returns an invalid range if offset + size exceeds the buffer size.
    template<bool R, bool W>
    [[nodiscard]]
    constexpr BufferRange<R, W> makeRange(size_t offset, size_t size) const noexcept
    {
        if (offset + size > m_size)
            return {};

        return {m_buffer, {offset, size}};
    }

    /// Create a BufferRange which contains the whole buffer.
    template<bool R, bool W>
    [[nodiscard]]
    constexpr BufferRange<R, W> makeRange() const noexcept { return {m_buffer, {0, m_size}}; }

    [[nodiscard]]
    constexpr RBufferRange makeRRange(size_t offset, size_t size) const
    {
        return makeRange<true, false>(offset, size);
    }

    [[nodiscard]]
    constexpr RBufferRange makeRRange() const { return makeRange<true, false>(); }

    [[nodiscard]]
    constexpr WBufferRange makeWRange(size_t offset, size_t size) const
    {
        return makeRange<false, true>(offset, size);
    }

    [[nodiscard]]
    constexpr WBufferRange makeWRange() const { return makeRange<false, true>(); }

    [[nodiscard]]
    constexpr RWBufferRange makeRWRange(size_t offset, size_t size) const
    {
        return makeRange<true, true>(offset, size);
    }

    [[nodiscard]]
    constexpr RWBufferRange makeRWRange() const { return makeRange<true, true>(); }

    /**
     * @brief Copies data between buffer ranges.
     * @param from source buffer range.
     * @param to destination buffer range.
     * The source and destination ranges must be valid and have the same size (which may be zero). If both ranges
     * correspond to the same buffer, they must not overlap. Failure to follow these requirements
     * will result in a no-op and possibly an exception for DEBUG builds; otherwise behavior is undefined.
     */
    static void copy(RBufferRange from, WBufferRange to);

private:
    GL::Buffer m_buffer {};
    size_t m_size {0};
};

} // Simple::Renderer

#endif //SIMPLERENDERER_BUFFER_HPP
