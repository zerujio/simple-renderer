#ifndef SIMPLERENDERER_BUFFER_HPP
#define SIMPLERENDERER_BUFFER_HPP

#include "glutils/buffer.hpp"
#include "typed_offset.hpp"
#include "typed_range.hpp"

namespace Simple::Renderer {

template<typename>
class BufferRange;

template<typename T>
constexpr bool operator==(const BufferRange<T> &, const BufferRange<T> &) noexcept;

template<typename T>
constexpr bool operator!=(const BufferRange<T> &, const BufferRange<T> &) noexcept;

/**
 * @brief A TypedRange paired with the buffer it belongs to.
 * @tparam T The type of the values contained within the buffer range. If const-qualified, indicates a read-only
 * reference to the buffer range.
 */
template<typename T = std::byte>
class BufferRange final
{
public:
    template<typename> friend
    class BufferRange;

    friend class Buffer;

    friend class VertexArray;

    using Offset = TypedOffset<std::remove_const_t<T>>;
    using Range = TypedRange<std::remove_const_t<T>>;
    using size_t = typename Offset::size_t;

    /// Constructs a range with zero size, zero offset and no buffer.
    constexpr BufferRange() noexcept = default;

    /// Construct from a native buffer handle and a typed range.
    constexpr BufferRange(GL::BufferHandle handle, Range range) noexcept: m_range(range), m_buffer(handle)
    {}

    constexpr BufferRange(const BufferRange &) noexcept = default;

    /// BufferRange<Y> can be implicitly converted to BufferRange<T> if their respective TypedRange types are
    /// implicitly convertible and the const qualifiers are compatible.
    template<typename Y,
            std::enable_if_t<
                    std::is_convertible_v<typename BufferRange<Y>::Range, Range>
                    && (!std::is_const_v<Y> || std::is_const_v<T>),
                    int> = 0>
    constexpr BufferRange(const BufferRange<Y> &other) noexcept:
            m_range(other.getTypedRange()), m_buffer(other.m_buffer)
    {}

    /// explicit conversion if the typed ranges are convertible and the const qualifiers are compatible
    template<typename Y, std::enable_if_t<!std::is_const_v<Y> || std::is_const_v<T>, int> = 0>
    constexpr explicit BufferRange(const BufferRange<Y> &other) noexcept:
            m_range(static_cast<Range>(other.getTypedRange())), m_buffer(other.m_buffer)
    {}

    /// Get this buffer range's TypedRange object.
    [[nodiscard]]
    constexpr const Range &getTypedRange() const
    { return m_range; }

    /// The number of values of type @p T contained within the range described by *this.
    [[nodiscard]] constexpr size_t getSize() const
    { return m_range.size; }

    /// The byte offset of the range, relative to the start of the buffer.
    [[nodiscard]] constexpr size_t getOffset() const
    { return m_range.size; }

    /// A buffer range is considered valid if it references a valid (non-zero) buffer, even if the memory range itself is empty.
    [[nodiscard]]
    constexpr bool isValid() const noexcept
    { return !m_buffer.isZero(); }

    /// Same as isValid()
    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    { return isValid(); }

    /// Construct a sub range within the same buffer.
    [[nodiscard]]
    constexpr BufferRange sub(Offset relative_offset, size_t new_size) const noexcept
    {
        const Range result = m_range.sub(relative_offset, new_size);

        if (result)
            return {m_buffer, result};
        else
            return {};
    }

    /**
     * @brief Joins two overlapping buffer ranges.
     * @return The union of @p l and @p r, or an empty range if they don't point to the same buffer or don't overlap.
     */
    static constexpr BufferRange join(const BufferRange &l, const BufferRange &r) noexcept
    {
        if (l.m_buffer != r.m_buffer)
            return {};

        const TypedRange mem_range = l.getTypedRange() + r.getTypedRange();

        if (!mem_range)
            return {};

        return {l.m_buffer, mem_range};
    }

    friend constexpr bool operator==<T>(const BufferRange &, const BufferRange &) noexcept;

    friend constexpr bool operator!=<T>(const BufferRange &, const BufferRange &) noexcept;

private:
    Range m_range;
    GL::BufferHandle m_buffer;
};

/// Two buffer ranges are equal if they reference the same buffer and memory range (offset and size are equal).
template<typename T>
constexpr bool operator==(const BufferRange<T> &l, const BufferRange<T> &r) noexcept
{
    return l.m_buffer == r.m_buffer && l.m_range == r.m_range;
}

template<typename T>
constexpr bool operator!=(const BufferRange<T> &l, const BufferRange<T> &r) noexcept
{
    return !operator==(l, r);
}

/// alias for BufferRange\<const T>
template<typename T>
using ConstBufferRange = BufferRange<const T>;

/// Manages a GPU memory buffer of fixed size.
class Buffer final
{
public:
    using size_t = std::size_t;

    /// Creates an invalid Buffer object, which is not associated with any GPU buffer.
    constexpr Buffer() = default;

    /**
     * @brief Construct a buffer of the specified size.
     * @param size The size of the buffer, in bytes.
     * @param data The data to initialize the buffer with. If null, the buffer's contents will be left uninitialized.
     * If buffer allocation fails an empty buffer object will be created.
     */
    explicit Buffer(size_t size, void *data = nullptr);

    /**
     * @brief Retrieve the size of the underlying buffer object.
     * @return Size of the buffer data store, in bytes, or zero if *this has no associated GPU buffer.
     */
    [[nodiscard]]
    constexpr size_t getSize() const noexcept
    { return m_size; }

    /// Returns true if *this has an associated GPU buffer.
    [[nodiscard]] constexpr bool isValid() const
    { return !m_buffer.isZero(); }

    [[nodiscard]] constexpr explicit operator bool() const
    { return isValid(); }

    /// Get range within the buffer; returns an invalid range if offset + size * sizeof(T) exceeds the buffer size.
    template<typename T = std::byte>
    [[nodiscard]]
    constexpr BufferRange<T> makeRange(TypedOffset<T> offset, size_t size) const noexcept
    {
        return makeRange<T>({offset, size});
    }

    template<typename T = std::byte>
    [[nodiscard]]
    constexpr BufferRange<T> makeRange(TypedRange<T> range) const noexcept
    {
        if (range.offset.get() + range.size * sizeof(T) > m_size)
            return {};

        return {m_buffer, range};
    }

    template<typename T = std::byte>
    [[nodiscard]]
    constexpr BufferRange<const T> makeConstRange(TypedOffset<T> offset, size_t size) const noexcept
    {
        return makeRange<const T>(offset, size);
    }

    template<typename T = std::byte>
    [[nodiscard]]
    constexpr BufferRange<const T> makeConstRange(TypedRange<T> range) const noexcept
    {
        return makeRange<const T>(range);
    }

    /**
     * @brief Copies data between buffer ranges.
     * @param from source buffer range.
     * @param to destination buffer range.
     * The source and destination ranges must be valid and have the same size (which may be zero). If both ranges
     * correspond to the same buffer, they must not overlap. Failure to follow these requirements
     * will result in a no-op and possibly an exception for DEBUG builds; otherwise behavior is undefined.
     */
    template<typename T>
    static void copy(const ConstBufferRange<T> &from, const BufferRange<T> &to)
    {
        copy(static_cast<BufferRange<const std::byte>>(from),
             static_cast<BufferRange<std::byte>>(to));
    }

    /// Returns the OpenGL handle for the GPU buffer.
    [[nodiscard]] GL::BufferHandle getGLHandle() const
    { return m_buffer; }

private:
    GL::Buffer m_buffer{GL::BufferHandle()};
    size_t m_size{0};   ///< size of the buffer in bytes
};

template<>
void Buffer::copy(const ConstBufferRange<std::byte> &from, const BufferRange<std::byte> &to);

} // Simple::Renderer

#endif //SIMPLERENDERER_BUFFER_HPP
