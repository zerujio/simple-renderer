#include "simple-renderer/allocation_registry.hpp"

#include <stdexcept>

namespace simple {

class AllocationRegistry::Block
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

AllocationRegistry::AllocationRegistry(uintptr size) : m_size(size) {}

auto AllocationRegistry::m_findFreeBlock(AllocationRegistry::uintptr size) noexcept
{
    auto it = m_blocks.begin();
    uintptr offset = 0;

    while (it != m_blocks.end() && !(it->isFree() && it->getSize() >= size))
        offset += it++->getSize();

    return std::make_pair(it, offset);
}

auto AllocationRegistry::tryAllocate(AllocationRegistry::uintptr size) -> std::optional<uintptr>
{
    const auto [iter, offset] = m_findFreeBlock(size);

    if (iter != m_blocks.cend())
    {
        if (iter->getSize() == size)
        {
            iter->setIsFree(false);
            return offset;
        }

        *iter = {iter->getSize() - size, true};
    }
    else if (m_size - offset < size)
        return {};

    m_blocks.insert(iter, {size, false});

    return offset;
}

auto AllocationRegistry::allocate(uintptr size) -> uintptr
{
    const auto opt = tryAllocate(size);
    if (!opt.has_value())
        throw std::logic_error("allocation registry: out of space");
    return *opt;
}

bool AllocationRegistry::tryDeallocate(AllocationRegistry::uintptr offset) noexcept
{
    uintptr acc_offset = 0;
    auto iter = m_blocks.begin();

    while (iter != m_blocks.end())
    {
        if (acc_offset == offset)
        {
            iter->setIsFree(true);
            return true;
        }

        acc_offset += iter++->getSize();
    }

    return false;
}

void AllocationRegistry::deallocate(uintptr offset)
{
    const bool succeeded = tryDeallocate(offset);
    if (!succeeded)
        throw std::logic_error("allocation registry: invalid deallocation offset");
}

} // simple