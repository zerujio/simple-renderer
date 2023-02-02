#ifndef PROCEDURALPLACEMENTLIB_ALLOCATION_REGISTRY_HPP
#define PROCEDURALPLACEMENTLIB_ALLOCATION_REGISTRY_HPP

#include <vector>

namespace simple {

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

    /// Register as used a block of memory of at least @p size bytes and returns its offset.
    [[nodiscard]] uintptr allocate(uintptr size);

    /// Register as unused a previously allocated block of memory that starts at @p offset.
    void deallocate(uintptr offset);

private:
    struct Block;

    [[nodiscard]] auto m_findFreeBlock(uintptr size) noexcept;

    std::vector<Block> m_blocks;
    uintptr m_size;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_ALLOCATION_REGISTRY_HPP
