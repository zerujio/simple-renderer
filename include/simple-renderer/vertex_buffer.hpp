#ifndef PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP
#define PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP

#include "simple-renderer/buffer.hpp"

#include <vector>
#include <array>
#include <utility>
#include <tuple>

namespace Simple::Renderer {

template<typename...> class VertexBuffer;

/**
 * @brief A reference to a section within a vertex buffer.
 * @tparam T The data type of the vertices in the section.
 *
 * @warning This is a non-owning reference. Accessing the vertex data after the VertexBuffer it belongs to has been
 * deleted is undefined behavior.
 */
template<typename T>
class VertexDataRange final
{
    static_assert(is_vertex_data_type<T>);
    static_assert(!std::is_const_v<T>, "invalid const qualifier");

public:
    using size_t = Buffer::size_t;

    /// Distance (in bytes) between the start of two consecutive elements in the vertex array.
    static constexpr size_t stride = vertex_stride<T>;

    /// Creates a null handle.
    constexpr VertexDataRange() noexcept = default;

    /// Create a new handle; will be invalid if @p buffer is a null pointer or the pointed-to buffer is invalid.
    constexpr VertexDataRange(GL::BufferHandle buffer, size_t byte_offset, size_t vertex_count) noexcept
            : m_buffer(buffer), m_byte_offset(buffer ? byte_offset : 0), m_vertex_count(buffer ? vertex_count : 0)
    {}

    /// Number of vertices in the referenced buffer section; returns 0 if the handle is null.
    [[nodiscard]]
    constexpr size_t getVertexCount() const noexcept { return m_vertex_count; }

    /// Construct a read-only buffer range for the referenced vertex data; the range will be invalid if the handle is invalid.
    [[nodiscard]]
    constexpr RBufferRange getBufferRange() const noexcept
    {
        return {m_buffer, {m_byte_offset, m_vertex_count * stride}};
    }

    [[nodiscard]]
    constexpr explicit operator RBufferRange () const { return getBufferRange(); }

    /// True if the handle references a valid Buffer object.
    [[nodiscard]] constexpr bool isValid() const { return !m_buffer.isZero(); }
    [[nodiscard]] constexpr explicit operator bool() const { return isValid(); }

private:
    GL::BufferHandle m_buffer {};
    size_t m_byte_offset {0};
    size_t m_vertex_count {0};
};

/// Used to initialize the contents of a vertex buffer.
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

    /// Vertex type with index I
    template<size_t I>
    using VertexTypeByIndex = std::tuple_element_t<I, std::tuple<Ts...>>;

    /// VertexDataRange type of the vertex type with index I.
    template<size_t I>
    using RangeTypeByIndex = VertexDataRange<VertexTypeByIndex<I>>;

    /// Stride of the vertex type with index I
    template<size_t I>
    static constexpr size_t stride_by_index = vertex_stride<VertexTypeByIndex<I>>;

    /// Number of distinct sections in the buffer.
    static constexpr size_t section_count = sizeof...(Ts);

    /// An empty vertex buffer, with no GPU buffer assigned.
    constexpr VertexBuffer() noexcept = default;

    /// Create a buffer with @p vertex_count value-initialized elements.
    explicit VertexBuffer(size_t vertex_count) : VertexBuffer({Ts(), vertex_count}...) {}

    /// Construct from a list of vertex data initializers, one for each section.
    explicit VertexBuffer(VertexDataInitializer<Ts>... initializers)
        : m_offsets{initializers.size() * decltype(initializers)::stride_by_index...},
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
    RangeTypeByIndex<I> getSectionRange() const noexcept
    {
        return {m_buffer, std::get<I>(m_offsets), std::get<I>(m_sizes)};
    }

    /// Get the number of vertices in the i-th section.
    template<size_t I>
    [[nodiscard]]
    constexpr size_t getSectionSize() const noexcept { return std::get<I>(m_sizes); }

private:
    using SectionArray = std::array<size_t, section_count>;

    SectionArray m_offsets;    ///< byte offset for each section
    SectionArray m_sizes;    ///< element count for each section
    Buffer m_buffer;
};

/// Single type vertex buffer.
template<typename T>
class VertexBuffer<T> final
{
public:
    static_assert(is_vertex_data_type<T>);
    static_assert(!std::is_const_v<T>, "unexpected const qualifier");

    using size_t = Buffer::size_t;

    using VertexType = T;
    using RangeType = VertexDataRange<T>;
    static constexpr size_t stride = vertex_stride<T>;
    static constexpr size_t section_count = 1;

    template<size_t I> using VertexTypeByIndex = std::enable_if_t<I == 0, VertexType>;
    template<size_t I> using RangeTypeByIndex = VertexDataRange<VertexTypeByIndex<I>>;
    template<size_t I> static size_t stride_by_index;
    template<> static constexpr size_t stride_by_index<0> = stride;

    constexpr VertexBuffer() noexcept = default;

    VertexBuffer(size_t vertex_count, const T& value) : VertexBuffer({vertex_count, value}) {}

    template<typename ContiguousIterator>
    VertexBuffer(ContiguousIterator begin, ContiguousIterator end) : VertexBuffer({begin, end}) {}

    explicit VertexBuffer(VertexDataInitializer<T> initializer)
    {
        std::vector<T> vector;
        vector.resize(initializer.size());
        initializer(vector.data());
        m_buffer = Buffer(initializer.size() * stride, vector.data());
    }

    [[nodiscard]] constexpr size_t getSize() const noexcept { return m_vertex_count; }
    [[nodiscard]] constexpr RangeType getRange() const noexcept { return {m_buffer, 0, m_vertex_count}; }

    template<size_t I = 0>
    [[nodiscard]]
    RangeTypeByIndex<I> getSectionRange() const noexcept { static_assert(I == 0); return getRange(); }

    template<size_t I = 0>
    [[nodiscard]]
    constexpr size_t getSectionSize() const noexcept { static_assert(I == 0); return getSize(); }

private:
    size_t m_vertex_count {0};
    Buffer m_buffer {};
};

} // Simple::Renderer

#endif //PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP
