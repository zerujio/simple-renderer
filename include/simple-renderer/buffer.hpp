#ifndef SIMPLERENDERER_BUFFER_HPP
#define SIMPLERENDERER_BUFFER_HPP

#include "glutils/buffer.hpp"

#include <cstdint>
#include <type_traits>

namespace Simple::Renderer {

/// True if @p T is a type that can be read by the GPU when stored in a buffer, false otherwise.
template<typename T>
constexpr bool is_gpu_compatible = std::is_standard_layout_v<T>
                && !(std::is_pointer_v<T> || std::is_member_pointer_v<T>)
                && alignof(T) >= 4;

template<typename T>
constexpr std::size_t stride_of = std::max(sizeof(T), alignof(T));

template<typename T>
class TypedOffsetBase
{
public:
    using size_t = std::uintptr_t;
    using ssize_t = std::intptr_t;
    using offset_t = size_t;
    using difference_t = ssize_t;

    // same as alignof(T)
    static constexpr size_t alignment = alignof(T);

    // separation between the start of consecutive elements of type @p T in a buffer
    static constexpr size_t stride = stride_of<T>;

    /// Creates a zero offset.
    constexpr TypedOffsetBase() noexcept = default;

    /**
     * @brief Initialize from an unsigned integer.
     * @param value The byte offset. If not a multiple of alignof(T), it will be rounded away from zero to ensure
     * proper alignment.
     */
    constexpr explicit TypedOffsetBase(offset_t value) noexcept : m_value(align_offset(value)) {}

    /// Retrieve the byte offset as an unsigned integer.
    [[nodiscard]] constexpr offset_t get() const noexcept { return m_value; }
    [[nodiscard]] constexpr explicit operator offset_t() const noexcept { return get(); }

    constexpr TypedOffsetBase operator+(ssize_t n) const noexcept
    {
        return TypedOffsetBase(m_value + stride * n);
    }

    constexpr TypedOffsetBase operator+(size_t n) const noexcept
    {
        // this is OK because stride is at least 4, meaning that overflow becomes a problem earlier than
        // out of range unsigned-to-signed conversion does.
        return operator+(static_cast<ssize_t>(n));
    }

    constexpr TypedOffsetBase& operator+=(ssize_t n) noexcept
    {
        m_value += stride * n;
        return *this;
    }

    constexpr TypedOffsetBase& operator+=(size_t n) noexcept
    {
        return operator+=(static_cast<ssize_t>(n));
    }

    constexpr TypedOffsetBase operator-(ssize_t n) noexcept
    {
        return operator+(-n);
    }

    constexpr TypedOffsetBase operator-(size_t n) noexcept
    {
        return operator-(static_cast<ssize_t>(n));
    }

    constexpr TypedOffsetBase operator-=(ssize_t n) noexcept
    {
        return operator+=(-n);
    }

    constexpr TypedOffsetBase operator-=(size_t n) noexcept
    {
        return operator-=(static_cast<ssize_t>(n));
    }

    constexpr TypedOffsetBase& operator++() noexcept
    {
        m_value += stride;
        return *this;
    }

    constexpr TypedOffsetBase operator++(int) noexcept
    {
        auto pre = *this;
        operator++();
        return pre;
    }

    constexpr TypedOffsetBase operator--() noexcept
    {
        m_value -= stride;
        return *this;
    }

    constexpr TypedOffsetBase operator--(int) noexcept
    {
        auto pre = *this;
        operator--();
        return pre;
    }

    constexpr difference_t operator-(TypedOffsetBase other) const noexcept
    {
        constexpr size_t bitshift = log2u(alignment);
        return (static_cast<difference_t>(m_value) - static_cast<difference_t>(other)) >> bitshift;
    }

    /// aligns an offset value to alignof(T)
    static constexpr offset_t align_offset(offset_t offset)
    {
        constexpr size_t alignment_log2 = log2u(alignment);
        offset_t aligned = (offset >> alignment_log2) << alignment_log2;
        return aligned + (aligned < offset) * alignment;
    }

private:
    static constexpr size_t log2u(size_t value)
    {
        size_t result = 0;
        while (value != 0)
        {
            value >>= 1;
            result++;
        }
        return result;
    }

    offset_t m_value {0};
};


/**
 * @brief An unsigned integer representing an offset into a GPU buffer.
 * @tparam T The data type of the element found at the offset.
 * Meant to be analogous to a pointer to @p T. All operators behave as if this were the case.
 */
template<typename T>
class TypedOffset;

template<typename T>
class TypedOffset<const T> : public TypedOffsetBase<T>
{
public:
    static_assert(is_gpu_compatible<T>);

    using TypedOffsetBase<T>::TypedOffsetBase;
};

/// non-const TypedOffset. Inherits from TypedOffset<const T> to allow implicit conversion from non-const to const.
template<typename T>
class TypedOffset : public TypedOffset<const T>
{
    using TypedOffset<const T>::TypedOffset;
};

/// Special typed offset which represents a byte offset.
template<>
class TypedOffset<const std::byte> : public TypedOffsetBase<std::byte>
{
    using TypedOffsetBase<std::byte>::TypedOffsetBase;
};

template<>
class TypedOffset<std::byte> : public TypedOffset<const std::byte>
{
    using TypedOffset<const std::byte>::TypedOffset;
};

using ByteOffset = TypedOffset<std::byte>;
using ConstByteOffset = TypedOffset<const std::byte>;


/// Represents a memory range, usually within a GPU buffer.
template<typename T>
struct TypedRangeBase
{
    using Offset = TypedOffset<T>;
    using size_t = typename Offset::size_t;
    using ssize_t = typename Offset::ssize_t;

    static constexpr size_t alignment = Offset::alignment;
    static constexpr size_t stride = Offset::stride;

    Offset offset {0};  ///< byte offset of the memory range, usually relative to the start of a buffer.
    size_t size {0};    ///< number of elements of type @p T contained within the range.

    /// Constructs an empty range (zero size and offset).
    constexpr TypedRangeBase() noexcept = default;

    /// Construct a range with specified offset and size.
    constexpr TypedRangeBase(Offset offset, size_t size) noexcept : offset(offset), size(size) {}
    constexpr TypedRangeBase(size_t offset, size_t size) noexcept : offset(offset), size(size) {}

    /**
     * @brief Creates a new object representing a subrange within *this.
     * @param relative_offset byte offset relative to the start of the current range.
     * @param sub_size size of the new range.
     * @return A new range with the specified offset and size, or an empty range if relative_offset + sub_size would
     * exceed the size of *this.
     */
    [[nodiscard]]
    constexpr TypedRangeBase subRange(Offset relative_offset, size_t sub_size) const noexcept
    {
        if (relative_offset + sub_size > size)
            return {};

        return {offset + relative_offset, size};
    }

    /// Joins two overlapping ranges; returns an empty range if @p l and @p r do not overlap.
    [[nodiscard]]
    static constexpr TypedRangeBase join(TypedRangeBase l, TypedRangeBase r) noexcept
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
template<typename T>
constexpr TypedRangeBase<T> operator+(TypedRangeBase<T> l, TypedRangeBase<T> r) noexcept
{
    return TypedRangeBase<T>::join(l, r);
}

template<typename T> class TypedRange;

using ByteRange = TypedRange<std::byte>;

template<>
class TypedRange<std::byte> final : public TypedRangeBase<std::byte>
{
    using TypedRangeBase<std::byte>::TypedRangeBase;
};

template<typename T>
class TypedRange final : public TypedRangeBase<T>
{
    using TypedRangeBase<T>::TypedRangeBase;
    using Base = TypedRangeBase<T>;

public:
    /// Construct from a raw byte range. The range's offset alignment must match that of @p T, and the size must be a
    /// multiple of sizeof(T)
    constexpr explicit TypedRange(ByteRange byte_range) noexcept
        : TypedRangeBase<T>(static_cast<TypedOffset<T>>(byte_range.offset), byte_range.size / Base::stride) {}

    /// convert to a byte range
    constexpr explicit operator ByteRange () const noexcept
    {
        return ByteRange(static_cast<ByteOffset>(Base::offset), Base::size * Base::stride);
    }
};

/// A reference to a memory range within a buffer.
template<typename T>
class BufferRange final
{
public:
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

    /// Construct from a native buffer handle and a memory range.
    constexpr BufferRange(GL::BufferHandle handle, MemoryRange range) noexcept : m_range(range), m_buffer(handle) {}

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
        const TypedRangeBase result = m_range.subRange(relative_offset, new_size);

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

        const TypedRangeBase mem_range = l.getMemoryRange() + r.getMemoryRange();

        if (!mem_range)
            return {};

        return {l.m_buffer, mem_range};
    }

private:
    MemoryRange m_range;
    GL::BufferHandle m_buffer;
};

/// Buffer range with read and write access.
using RWBufferRange = BufferRange<true, true>;

/// Buffer range with read-only access.
using RBufferRange = BufferRange<true, false>;

/// Buffer range with write-only access.
using WBufferRange = BufferRange<false, true>;

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
    constexpr size_t getSize() const noexcept { return m_size; }

    /// Returns true if *this has an associated GPU buffer.
    [[nodiscard]] constexpr bool isValid() const { return !m_buffer.isZero(); }
    [[nodiscard]] constexpr explicit operator bool() const { return isValid(); }

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

    /// Returns the OpenGL handle for the GPU buffer.
    [[nodiscard]] GL::BufferHandle getGLHandle() const { return m_buffer; }

private:
    GL::Buffer m_buffer {};
    size_t m_size {0};
};

} // Simple::Renderer

#endif //SIMPLERENDERER_BUFFER_HPP
