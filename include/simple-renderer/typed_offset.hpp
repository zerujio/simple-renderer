#ifndef SIMPLERENDERER_TYPED_OFFSET_HPP
#define SIMPLERENDERER_TYPED_OFFSET_HPP

#include <cstdint>
#include <type_traits>
#include <stdexcept>

namespace Simple::Renderer {

template<typename T>
constexpr bool is_gpu_compatible_integer = std::is_integral_v<T>
                                           && sizeof(GLbyte) <= sizeof(T)
                                           && sizeof(T) <= sizeof(GLint);

/// True if @p T is a type that can be read by the GPU when stored in a buffer, false otherwise.
template<typename T>
constexpr bool is_gpu_compatible = std::is_standard_layout_v<T> && !std::is_pointer_v<T> && !std::is_member_pointer_v<T>;

static constexpr std::size_t log2u(std::size_t value)
{
    if (value == 0)
        throw std::domain_error("logarithm is undefined at x = 0");

    std::size_t result = 0;
    while (value != 1)
    {
        value >>= 1;
        result++;
    }
    return result;
}

static_assert(log2u(1) == 0);
static_assert(log2u(2) == 1);
static_assert(log2u(4) == 2);
static_assert(log2u(256) == 8);

/**
 * @brief An unsigned integer representing an offset into a GPU buffer.
 * @tparam T The data type of the element found at the offset.
 * Meant to be analogous to a pointer to @p T. All operators behave as if this were the case.
 */
template<typename T>
class TypedOffset;

using ByteOffset = TypedOffset<std::byte>;

template<typename T>
class TypedOffset
{
public:
    static_assert(!std::is_const_v<T>, "const qualifier is meaningless on an offset");
    static_assert(is_gpu_compatible<T> || std::is_same_v<T, std::byte>);

    using size_t = std::uintptr_t;
    using ssize_t = std::intptr_t;
    using offset_t = size_t;
    using difference_t = ssize_t;

    /// Type of the value found at the offset.
    using ValueType = T;

    /// aligns @p offset by rounding up to the nearest multiple of alignof(T)
    static constexpr offset_t align_offset(const offset_t offset)
    {
        constexpr size_t alignment = alignof(T);

        if constexpr (alignment == 1)
            return offset;

        constexpr size_t alignment_log2 = log2u(alignment);
        offset_t aligned = (offset >> alignment_log2) << alignment_log2;
        return aligned + (aligned < offset) * alignment;
    }

    /// Creates a zero offset.
    constexpr TypedOffset() noexcept = default;

    /**
     * @brief Initialize from an unsigned integer.
     * @param value The byte offset. If not a multiple of alignof(T), it will be rounded away from zero to ensure
     * proper alignment.
     */
    constexpr explicit TypedOffset(offset_t value) noexcept: m_value(align_offset(value)) {}

    /// defaulted copy constructor
    constexpr TypedOffset(const TypedOffset&) noexcept = default;

    /// Byte to @p Y conversion and vice-versa.
    template<typename Y,
            std::enable_if_t<std::is_same_v<T, std::byte> != std::is_same_v<Y, std::byte>, int> = 0>
    constexpr explicit TypedOffset(const TypedOffset<Y>& other) noexcept:
            TypedOffset(other.get())
    {}

    /// Retrieve the byte offset as an unsigned integer.
    [[nodiscard]] constexpr offset_t get() const noexcept
    { return m_value; }

    [[nodiscard]] constexpr explicit operator offset_t() const noexcept
    { return get(); }

    constexpr TypedOffset operator+(ssize_t n) const noexcept
    {
        return TypedOffset(m_value + sizeof(T) * n);
    }

    constexpr TypedOffset operator-(ssize_t n) noexcept
    {
        return operator+(-n);
    }

    constexpr TypedOffset &operator+=(ssize_t n) noexcept
    {
        m_value += sizeof(T) * n;
        return *this;
    }

    constexpr TypedOffset operator-=(ssize_t n) noexcept
    {
        return operator+=(-n);
    }

    constexpr TypedOffset &operator++() noexcept
    {
        m_value += sizeof(T);
        return *this;
    }

    constexpr TypedOffset operator++(int) noexcept
    {
        auto pre = *this;
        operator++();
        return pre;
    }

    constexpr TypedOffset operator--() noexcept
    {
        m_value -= sizeof(T);
        return *this;
    }

    constexpr TypedOffset operator--(int) noexcept
    {
        auto pre = *this;
        operator--();
        return pre;
    }

    constexpr difference_t operator-(const TypedOffset other) noexcept
    {
        constexpr size_t bitshift = log2u(alignof(T));
        return (static_cast<difference_t>(m_value) - static_cast<difference_t >(other.m_value)) >> bitshift;
    }

    constexpr bool operator==(const TypedOffset &rhs) const noexcept
    {
        return m_value == rhs.m_value;
    }

    constexpr bool operator!=(const TypedOffset &rhs) const noexcept
    {
        return m_value != rhs.m_value;
    }

    constexpr bool operator<(const TypedOffset &rhs) const noexcept
    {
        return m_value < rhs.m_value;
    }

    constexpr bool operator>(const TypedOffset &rhs) const noexcept
    {
        return m_value > rhs.m_value;
    }

    constexpr bool operator<=(const TypedOffset &rhs) const noexcept
    {
        return m_value <= rhs.m_value;
    }

    constexpr bool operator>=(const TypedOffset &rhs) const noexcept
    {
        return m_value >= rhs.m_value;
    }

private:
    offset_t m_value{0};
};

static_assert(TypedOffset<int>(8) - TypedOffset<int>(4) == 1);
static_assert(TypedOffset<std::intptr_t>(16) - TypedOffset<std::intptr_t>(8) == 1);

} // Simple::Renderer

#endif //SIMPLERENDERER_TYPED_OFFSET_HPP
