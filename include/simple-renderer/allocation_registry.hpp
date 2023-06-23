#ifndef PROCEDURALPLACEMENTLIB_ALLOCATION_REGISTRY_HPP
#define PROCEDURALPLACEMENTLIB_ALLOCATION_REGISTRY_HPP

#include <vector>
#include <optional>

namespace Simple {

/// Keeps track of allocated memory, usually used for GPU buffers.
class AllocationRegistry final
{
public:
    using uintptr = std::size_t;

    /// Base-2 exponent of the alignment value.
    static constexpr uintptr alignment_exp = 2;

    /// Byte alignment of allocated memory blocks, must be base 2.
    static constexpr uintptr alignment = 1 << alignment_exp;

    /// Create a registry that will keep track of @p size bytes of contiguous memory.
    explicit AllocationRegistry(uintptr size);

    /// Register as used a block of memory of exactly @p size bytes and return its offset.
    [[nodiscard]] uintptr allocate(uintptr size);

    /// Same as allocate(), but returns an empty optional on failure instead of throwing an exception.
    [[nodiscard]] std::optional<uintptr> tryAllocate(uintptr size);

    /// Register as unused a previously allocated block of memory that starts at @p offset.
    void deallocate(uintptr offset);

    /// Same as deallocate(), but returns false on failure instead of throwing an axception.
    bool tryDeallocate(uintptr offset) noexcept;

    /// Return the biggest block that could be allocated.
    [[nodiscard]] uintptr getMaxAllocation() const noexcept;

private:

    class Block
    {
    public:
        // here an alignment of at least 2 is assumed
        constexpr Block(uintptr offset, bool is_used) noexcept : m_tagged_size((offset & s_size_mask) | is_used) {}

        [[nodiscard]] constexpr bool isFree() const noexcept { return m_tagged_size & s_tag_mask; }
        [[nodiscard]] constexpr uintptr getSize() const noexcept { return m_tagged_size & s_size_mask; }

        constexpr void setIsFree(bool tag) noexcept { m_tagged_size = getSize() | tag; }
        constexpr void setSize(uintptr offset) noexcept { m_tagged_size = (offset & s_size_mask) | isFree(); }

    private:
        static constexpr uintptr s_tag_mask = 0x1;
        static constexpr uintptr s_size_mask = ~s_tag_mask;

        uintptr m_tagged_size;
    };

    [[nodiscard]] auto m_findFreeBlock(uintptr size) noexcept;

    std::vector<Block> m_blocks;
    uintptr m_size;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_ALLOCATION_REGISTRY_HPP
