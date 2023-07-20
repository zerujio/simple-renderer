#include "simple_renderer/allocation_registry.hpp"

#include <stdexcept>
#include <algorithm>

namespace Simple {

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

AllocationRegistry::uintptr AllocationRegistry::getMaxAllocation() const noexcept
{
    uintptr max_free_block_size = 0;
    uintptr block_size_sum = 0;
    for (const auto& block : m_blocks)
    {
        const auto size = block.getSize();
        block_size_sum += size;
        if (size > max_free_block_size)
            max_free_block_size = size;
    }
    return std::max(max_free_block_size, m_size - block_size_sum);
}

} // simple