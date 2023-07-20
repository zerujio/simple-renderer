#ifndef SIMPLERENDERER_TYPED_RANGE_HPP
#define SIMPLERENDERER_TYPED_RANGE_HPP

#include "typed_offset.hpp"

namespace Simple::Renderer {

/// Represents a memory range, usually within a GPU buffer.
template<typename T>
struct TypedRange;

using ByteRange = TypedRange<std::byte>;

template<typename T>
struct TypedRange
{
    using Offset = TypedOffset<T>;
    using size_t = typename Offset::size_t;
    using ssize_t = typename Offset::ssize_t;

    /// Constructs an empty range (zero size and offset).
    constexpr TypedRange() noexcept = default;

    /// default copy constructor
    constexpr TypedRange(const TypedRange&) noexcept = default;

    /// Construct a range with specified offset and size.
    constexpr TypedRange(Offset offset, size_t size) noexcept: offset(offset), size(size)
    {}

    constexpr TypedRange(size_t offset, size_t size) noexcept: offset(offset), size(size)
    {}

    /// Explicit conversion to byte range.
    template<typename Y,
            std::enable_if_t<std::is_same_v<T, std::byte> && !std::is_same_v<Y, std::byte>, int> = 0>
    constexpr explicit TypedRange(const TypedRange<Y>& other) noexcept:
            offset(static_cast<TypedOffset<std::byte>>(other.offset)), size(other.size * sizeof(Y))
    {}

    /// Explicit conversion from byte range to other type.
    template<typename Y,
            std::enable_if_t<std::is_same_v<T, std::byte> && !std::is_same_v<Y, std::byte>, int> = 0>
    constexpr explicit operator TypedRange<Y>() const
    {
        return {static_cast<TypedOffset<Y>>(offset), size / sizeof(T)};
    }

    /// Checks if the size is zero.
    [[nodiscard]] constexpr bool isEmpty() const noexcept
    { return size == 0; }

    /// true if the range is not empty, false otherwise.
    [[nodiscard]] constexpr explicit operator bool() const noexcept
    { return !isEmpty(); }

    /**
     * @brief Creates a new object representing a subrange within @p range.
     * @param range the outer range.
     * @param sub_offset byte offset relative to the start of the current range.
     * @param sub_size size of the new range.
     * @return A new range with the specified offset and size, or an empty range if the specified @p sub_offset and
     * @p sub_size would not result in a sub-range of *this.
     */
    constexpr TypedRange sub(Offset sub_offset, size_t sub_size) const noexcept
    {
        if (sub_offset > (size - sub_size) * sizeof(T))
            return {};

        return {offset + sub_offset, sub_size};
    }

    /// Joins two overlapping ranges; returns an empty range if @p l and @p r do not overlap.
    static constexpr TypedRange join(const TypedRange& l, const TypedRange& r) noexcept
    {
        const size_t start = std::min(l.offset, r.offset);
        const size_t end = std::max(l.offset + l.size, r.offset + r.size);
        const size_t size = end - start;

        if (l.size + r.size < size)
            return {};
        else
            return {start, size};
    }

    Offset offset{0};  ///< byte offset of the memory range, usually relative to the start of a buffer.
    size_t size{0};    ///< number of elements of type @p T contained within the range.
};

template<typename T>
constexpr TypedRange<T> operator+(const TypedRange<T>& l, const TypedRange<T>& r) noexcept
{
    return TypedRange<T>::join(l, r);
}

template<typename T>
constexpr bool operator==(const TypedRange<T>& l, const TypedRange<T>& r) noexcept
{
    return l.offset == r.offset && l.size == r.size;
}

template<typename T>
constexpr bool operator!=(const TypedRange<T>& l, const TypedRange<T>& r) noexcept
{
    return !(l == r);
}

} // Simple::Renderer

#endif //SIMPLERENDERER_TYPED_RANGE_HPP
