#ifndef PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP
#define PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP

#include "simple-renderer/buffer.hpp"
#include "typed_range.hpp"

#include <vector>
#include <array>
#include <utility>
#include <tuple>

namespace Simple::Renderer {

template<typename...> class VertexBuffer;

/// Used to initialize the contents of a vertex buffer.
template<typename T>
struct VertexDataInitializer final
{
public:
    static_assert(is_gpu_compatible<T>);
    using VertexType = T;
    using size_t = std::size_t;

    /// an empty initializer, does nothing.
    constexpr VertexDataInitializer() noexcept = default;

    /// Initialize from the contents of contiguous (i.e. array-like) container.
    template<typename ContiguousIterator>
    VertexDataInitializer(ContiguousIterator begin, ContiguousIterator end): m_data(&(*begin)), m_size(end - begin) {}

    template<typename Container>
    VertexDataInitializer(const Container& container): m_data(std::data(container)), m_size(std::size(container)) {}

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

    constexpr explicit operator bool() const noexcept { return size() > 0; }

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

    const VertexType* m_data {nullptr};
    size_t m_size {0};
    bool m_value_initialize {false};
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
    using BufferRangeByIndex = BufferRange<VertexTypeByIndex<I>>;

    template<size_t I>
    using TypedRangeByIndex = TypedRange<VertexTypeByIndex<I>>;

    /// Stride of the vertex type with index I
    template<size_t I>
    static constexpr size_t stride_by_index = sizeof(VertexTypeByIndex<I>);

    /// Number of distinct sections in the buffer.
    static constexpr size_t section_count = sizeof...(Ts);

    /// An empty vertex buffer, with no GPU buffer assigned.
    constexpr VertexBuffer() noexcept = default;

    /// Create a buffer with @p vertex_count value-initialized elements.
    explicit VertexBuffer(size_t vertex_count) : VertexBuffer({Ts(), vertex_count}...) {}

    /// Construct from a list of vertex data initializers, one for each section.
    explicit VertexBuffer(VertexDataInitializer<Ts>... initializers)
        : m_ranges{TypedRange<Ts>(TypedOffset<Ts>(), initializers.size())...}
    {
        initializeRangeOffsets();

        const auto last_range = getTypedRange<sizeof...(Ts) - 1>();
        const size_t buffer_size {last_range.offset + last_range.size};

        // create temporary byte array
        std::vector<std::byte> byte_array;
        byte_array.reserve(buffer_size);

        initializeByteArray(std::tie(initializers...), byte_array.data());

        // create buffer
        m_buffer = Buffer(buffer_size, byte_array.data());
    }

    /// Construct a BufferRange object for the section with the specified index.
    template<size_t I>
    [[nodiscard]]
    constexpr BufferRangeByIndex<I> getBufferRange() const noexcept
    {
        return m_buffer.makeRange(getTypedRange<I>());
    }

    /// Get a TypedRange representing the memory range occupied by the section with index @p I
    template<size_t I>
    [[nodiscard]]
    constexpr TypedRangeByIndex<I> getTypedRange() const noexcept
    {
        return std::get<I>(m_ranges);
    }

    /// Get the total allocated size of the buffer, in bytes.
    [[nodiscard]]
    constexpr size_t getSize() const noexcept { return m_buffer.getSize(); }

    /// Direct access to the underlying buffer object; use at your own risk.
    [[nodiscard]] const Buffer& getBuffer() const { return m_buffer; }

private:
    template<size_t I = 0>
    constexpr void initializeRangeOffsets(ByteOffset base_offset = ByteOffset(0))
    {
        if constexpr (I < sizeof...(Ts))
        {
            TypedRangeByIndex<I>& range = std::get<I>(m_ranges);
            using TypedOffset = decltype(range.offset);
            range.offset = static_cast<TypedOffset>(base_offset);

            initializeRangeOffsets<I + 1>(static_cast<ByteOffset>(range.offset + range.size));
        }
    }

    template<size_t I = 0, typename Tuple>
    constexpr void initializeByteArray(const Tuple& initializers, std::byte* array)
    {
        if constexpr (I < sizeof...(Ts))
        {
            initializeByteArraySection(std::get<I>(initializers), getTypedRange<I>().offset, array);

            initializeByteArray<I + 1>(initializers, array);
        }
    }

    template<typename T>
    void initializeByteArraySection(const VertexDataInitializer<T>& initializer, TypedOffset<T> offset, std::byte* array)
    {
        initializer(reinterpret_cast<T*>(array + offset.get()));
    }

    std::tuple<TypedRange<Ts>...> m_ranges;
    Buffer m_buffer;
};

/// Single type vertex buffer.
template<typename T>
class VertexBuffer<T> final
{
public:
    static_assert(is_gpu_compatible<T>);
    static_assert(!std::is_const_v<T>, "unexpected const qualifier");

    using size_t = Buffer::size_t;

    template<size_t I> using VertexTypeByIndex = std::enable_if_t<I == 0, T>;
    template<size_t I> using TypedRangeByIndex = TypedRange<std::remove_const_t<VertexTypeByIndex<I>>>;
    template<size_t I> using BufferRangeByIndex = BufferRange<VertexTypeByIndex<I>>;
    template<size_t I> static constexpr size_t stride_by_index = sizeof(VertexTypeByIndex<I>);

    using VertexType = T;
    using RangeType = TypedRange<std::remove_const_t<T>>;
    using BufferRangeType = BufferRange<T>;
    static constexpr size_t stride = stride_by_index<0>;
    static constexpr size_t section_count = 1;

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

    template<size_t I = 0>
    [[nodiscard]] constexpr RangeType getTypedRange() const noexcept
    {
        static_assert(I == 0, "section index out of range");
        return m_vertex_count;
    }

    template<size_t I = 0>
    [[nodiscard]] constexpr BufferRangeType getBufferRange() const noexcept
    {
        static_assert(I == 0, "section index out of range");
        return {m_buffer, getTypedRange()};
    }

    [[nodiscard]] constexpr size_t getSize() const noexcept { return m_buffer.getSize(); }

private:
    size_t m_vertex_count {0};
    Buffer m_buffer {};
};

} // Simple::Renderer

#endif //PROCEDURALPLACEMENTLIB_VERTEX_BUFFER_HPP
