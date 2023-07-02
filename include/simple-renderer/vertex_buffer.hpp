#ifndef PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP
#define PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP

#include "simple-renderer/buffer.hpp"

#include <vector>
#include <array>
#include <utility>
#include <tuple>

namespace Simple::Renderer {

/// True if @p T is a data type which may be stored in a GPU buffer.
template<typename T>
constexpr bool is_vertex_data_type =
        std::is_standard_layout_v<T> && !std::is_pointer_v<T> && !std::is_member_pointer_v<T>;

template<typename T>
constexpr std::size_t vertex_stride = std::max(sizeof(T), alignof(T));

template<typename...> class VertexBuffer;

/// A reference to a buffer range which contains a contiguous array of elements of type @p T
template<typename T>
class VertexDataRange final
{
    static_assert(is_vertex_data_type<T>);

    template<typename ...> friend class VertexBuffer;

public:
    using size_t = Buffer::size_t;
    using BufferRange = BufferRange<true, !std::is_const_v<T>>;

    /// Distance (in bytes) between the start of two consecutive elements in the array.
    static constexpr size_t stride = std::max(sizeof(T), alignof(T));

    /// Creates an invalid range.
    constexpr VertexDataRange() noexcept = default;

    /// Construct from non-const variant.
    constexpr explicit VertexDataRange(const VertexDataRange<std::remove_const<T>>& other) noexcept : m_range(other.m_range) {}

    [[nodiscard]]
    constexpr const BufferRange& getBufferRange() const noexcept { return m_range; }

private:
    constexpr explicit VertexDataRange(const BufferRange& range) noexcept : m_range(range) {}

    BufferRange m_range;
};


template<typename T>
struct VertexDataInitializer final
{
public:
    static_assert(is_vertex_data_type<T>);
    using VertexType = T;
    using size_t = std::size_t;

    static constexpr size_t stride = vertex_stride<T>;

    /// Initialize from the contents of contiguous (i.e. array-like) container.
    template<typename ContiguousIterator>
    VertexDataInitializer(ContiguousIterator begin, ContiguousIterator end)
        : m_data(&(*begin)), m_size(end - begin), m_value_initialize(false) {}

    /// Initialize with @p count copies of @p value
    VertexDataInitializer(const T& value, size_t count)
        : m_data(&value), m_size(count), m_value_initialize(true) {}

    /// Number of elements in the array.
    [[nodiscard]] size_t size() const { return m_size; }

    /// Initialize contents of @p data
    void operator() (VertexType* data) const
    {
        if (m_value_initialize)
            valueInitialize(data);
        else
            copyInitialize(data);
    }

private:
    void valueInitialize(VertexType* data) const
    {
        for (VertexType* p = data; p != data + m_size; p++)
            *p = *m_data;
    }

    void copyInitialize(VertexType* data) const
    {
        for (const VertexType* p = m_data; p != m_data + m_size; p++)
            *data++ = *p;
    }

    const VertexType* m_data;
    size_t m_size;
    bool m_value_initialize;
};

/// A GPU buffer which contains one or more arrays of elements of types @Ts. Each array may have a different number of
/// elements.
template<typename ... Ts>
class VertexBuffer final
{
public:
    using size_t = Buffer::size_t;

    template<size_t N>
    using VertexType = std::tuple_element_t<N, std::tuple<Ts...>>;

    template<size_t N>
    using RangeType = VertexDataRange<VertexType<N>>;

    template<size_t N>
    static constexpr size_t stride_by_index = RangeType<N>::stride;

    /// Number of distinct sections in the buffer.
    static constexpr size_t section_count = sizeof...(Ts);

    /// Create a buffer with @p vertex_count value-initialized elements.
    explicit VertexBuffer(size_t vertex_count) : VertexBuffer({Ts(), vertex_count}...) {}

    /// Construct from a list of vertex data initializers, one for each section.
    explicit VertexBuffer(VertexDataInitializer<Ts>... initializers)
        : m_offsets{initializers.size() * decltype(initializers)::stride...},
          m_sizes{initializers.size()...}
    {
        // determine byte offsets
        size_t buffer_size = 0;
        for (size_t& offset : m_offsets)
        {
            const auto section_size = offset;
            offset = buffer_size;
            buffer_size += section_size;
        }

        // create temporary byte array
        std::vector<std::byte> initializer_values;
        initializer_values.reserve(buffer_size);

        // use initializers on byte array
        {
            const auto initialize = [](const auto& initializer, std::byte* byte_data)
            {
                using VertexType = typename std::remove_reference_t<decltype(initializer)>::VertexType;
                auto vertex_data = reinterpret_cast<VertexType*>(byte_data);
                initializer(vertex_data);
            };

            size_t array_index = 0;
            (initialize(initializers, initializer_values.data() + m_offsets[array_index++]), ...);
        }

        // create buffer
        m_buffer = Buffer(buffer_size, initializer_values.data());
    }

    /// Get a VertexDataRange object for the section with the specified index.
    template<size_t I>
    [[nodiscard]]
    RangeType<I> getSection() const
    {
        return RangeType<I>{m_buffer.makeRWRange(std::get<I>(m_offsets), std::get<I>(m_sizes) * stride_by_index<I>)};
    }

private:
    using SectionArray = std::array<size_t, section_count>;

    SectionArray m_offsets;    ///< byte offset for each section
    SectionArray m_sizes;    ///< element count for each section
    Buffer m_buffer;
};

} // Simple::Renderer

#endif //PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP
