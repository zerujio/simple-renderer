#ifndef PROCEDURALPLACEMENTLIB_BUFFER_RANGE_HPP
#define PROCEDURALPLACEMENTLIB_BUFFER_RANGE_HPP

#include "glutils/buffer.hpp"

#include <cstdint>
#include <utility>
#include <algorithm>
#include <type_traits>
#include <memory>

namespace simple {

class RBufferRange;
class WBufferRange;

enum class BufferAccess{ Read, Write, ReadWrite };

/// Specifies a memory range within a buffer.

class BufferRange
{
public:
    using offset_t = std::uintptr_t;
    using size_t = std::uintptr_t;

    constexpr BufferRange(GL::BufferHandle buffer, offset_t offset, size_t size) noexcept
            : m_buffer(buffer), m_offset(offset), m_size(size)
    {}

    constexpr BufferRange(GL::BufferHandle buffer, GL::Buffer::Range range) noexcept
            : m_buffer(buffer), m_offset(range.offset), m_size(range.size)
    {}

    /// Get the byte offset into the buffer's data store.
    [[nodiscard]]
    constexpr offset_t getOffset() const noexcept
    { return m_offset; }

    /// Get the size of this memory range, in bytes.
    [[nodiscard]]
    constexpr size_t getSize() const noexcept
    { return m_size; }

    /// Checks if size is greater than zero.
    [[nodiscard]]
    constexpr bool isEmpty() const noexcept
    { return m_size == 0; }

    /// Checks if the range references a buffer (an empty range is still considered valid).
    [[nodiscard]]
    constexpr bool isValid() const noexcept
    { return m_buffer.getName(); }

    /// Construct a smaller range.
    /**
     *
     * @param relative_offset a byte relative_offset relative to the beginning of this range. If this value is greater than the size of
     * *this, the result will be a zero-sized range with that starts and ends at this->getSize() + this->getSize().
     * @param size the size for the new range. If greater than this->getSize() - @p relative_offset, the result will be have
     * the expected relative_offset and a size of max(0, this-getSize() - @p relative_offset).
     * @return A new BufferRange object that equivalent to the intersection of *this and a range with relative_offset equal to
     * this->getSize() + @p relative_offset and size equal to @p size.
     */
    [[nodiscard]]
    constexpr BufferRange subRange(offset_t relative_offset, size_t size) const noexcept
    {
        const auto clamped_offset = std::min(relative_offset, m_size);
        const auto clamped_size = std::min(size, m_size - clamped_offset);

        return {m_buffer, m_offset + clamped_offset, clamped_size};
    }

    /// Equivalent to subRange(0, @p size ) .
    [[nodiscard]]
    constexpr BufferRange lowerSubRange(std::size_t size) const noexcept
    { return {m_buffer, m_offset, std::min(size, m_size)}; }

    /// Equivalent to subRange( @p offset , max(0, getSize() - @p offset )).
    [[nodiscard]]
    constexpr BufferRange upperSubRange(std::uintptr_t relative_offset) const noexcept
    {
        const auto clamped_offset = std::min(relative_offset, m_size);
        return {m_buffer, m_offset + clamped_offset, m_size - relative_offset};
    }

    /// Divide this range into two smaller, non overlapping ranges.
    /**
     *
     * @param split_offset A byte offset relative to the start of this range. If greater than the size of this range,
     * the result will be computed as if that were its value.
     * @return A pair of ranges, the first one extending from the start of the current range to the to just before @p
     * split_offset and the second one from that point to the end of the current range.
     */
    [[nodiscard]]
    constexpr std::pair<BufferRange, BufferRange> splitAt(offset_t split_offset) const noexcept
    {
        const auto clamped_offset = std::min(split_offset, m_size);
        return {{m_buffer, m_offset, clamped_offset}, {m_buffer, m_offset + clamped_offset, m_size - clamped_offset}};
    }

    /// Constructs a range that joins together two adjacent or overlapping ranges.
    /**
     *
     * @tparam RangeT0 Any BufferRange type.
     * @tparam RangeT1 Any BufferRange type.
     * @param l_range A range.
     * @param r_range Another range on the same buffer.
     * @return A new range that unifies the contents of @p l_range and @p r_range. If the union of the ranges is not
     * a contiguous memory block, then the result will be a zero-size range. If the ranges reference different buffers,
     * then the result will be an invalid range.
     */
    template<typename RangeT0, typename RangeT1>
    [[nodiscard]]
    static constexpr std::common_type_t<RangeT0, RangeT1> join(RangeT0 l_range, RangeT1 r_range) noexcept
    {
        const offset_t begin = std::min(l_range.m_offset, r_range.m_offset);
        const offset_t end = std::max(l_range.offset + l_range.size, r_range.offset + r_range.size);
        const size_t size = end - begin;

        return {l_range.m_buffer == r_range.m_buffer ? l_range.m_buffer : GL::BufferHandle(),
                begin, size > l_range.m_size + r_range.m_size ? 0 : size};
    }

    /// Same as join(*this, other).
    template<typename RangeT>
    [[nodiscard]]
    constexpr auto joinWith(RangeT other) const noexcept
    { return join(*this, other); }

    static void copy(const RBufferRange& read_range, const WBufferRange& write_range);
    static void copy(const RBufferRange& read_range, GL::BufferHandle write_buffer, offset_t write_offset);
    static void copy(GL::BufferHandle read_buffer, offset_t read_offset, const WBufferRange& write_range);

    class MappedPtrDeleter final
    {
    public:
        explicit MappedPtrDeleter(GL::BufferHandle source_buffer) : m_buffer(source_buffer) {}
        void operator() (const std::byte*) const;
    private:
        GL::BufferHandle m_buffer;
    };

protected:
    [[nodiscard]] std::unique_ptr<std::byte[], MappedPtrDeleter> m_map(GL::Buffer::AccessFlags access_flags) const;

    [[nodiscard]] GL::BufferHandle m_getBuffer() const { return m_buffer; }

private:
    offset_t m_offset;
    size_t m_size;
    GL::BufferHandle m_buffer;
};

/// Specifies a read only memory range within a buffer.
class RBufferRange : public virtual BufferRange
{
public:
    using BufferRange::BufferRange;

    /// copy contents of this range to another range of the same size.
    inline void copyTo(const WBufferRange& destination_range) const;

    /// Copy the contents of this range into another buffer.
    inline void copyTo(GL::BufferHandle destination_buffer, std::uintptr_t write_offset) const;

    /// Directly access the contents of the range through a read-only pointer.
    [[nodiscard]] std::unique_ptr<const std::byte[], MappedPtrDeleter> mapForRead() const;

    /// Read the contents of the range.
    void read(void *data);
};

/// Specifies a write only memory range within a buffer.
class WBufferRange : public virtual BufferRange
{
public:
    using BufferRange::BufferRange;

    /// copy contents of another range into this range.
    inline void copyFrom(const RBufferRange& source_range) const;

    /// copy contents of a buffer into this range.
    inline void copyFrom(GL::BufferHandle source_buffer, std::uintptr_t read_offset) const;

    /// Directly access the contents of the range through a write-only pointer.
    [[nodiscard]] std::unique_ptr<std::byte[], MappedPtrDeleter> mapForWrite() const;

    /// Write data to the range.
    void write(const void *data);
};

/// Specifies a read-write memory range within a buffer.
class RWBufferRange final : public RBufferRange, public WBufferRange
{
public:
    using RBufferRange::RBufferRange;
    using WBufferRange::WBufferRange;

    /// Directly access the contents of the range through a read-write pointer.
    [[nodiscard]] std::unique_ptr<std::byte[], MappedPtrDeleter> map() const;
};

void RBufferRange::copyTo(const WBufferRange& destination_range) const
{ copy(*this, destination_range); }

void RBufferRange::copyTo(GL::BufferHandle destination_buffer, std::uintptr_t write_offset) const
{ copy(*this, destination_buffer, write_offset); }

void WBufferRange::copyFrom(const RBufferRange& source_range) const
{ copy(source_range, *this); }

void WBufferRange::copyFrom(GL::BufferHandle source_buffer, std::uintptr_t read_offset) const
{ copy(source_buffer, read_offset, *this); }

} // simple

#endif //PROCEDURALPLACEMENTLIB_BUFFER_RANGE_HPP
