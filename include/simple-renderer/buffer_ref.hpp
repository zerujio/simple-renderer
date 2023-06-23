#ifndef PROCEDURALPLACEMENTLIB_BUFFER_REF_HPP
#define PROCEDURALPLACEMENTLIB_BUFFER_REF_HPP

#include "glutils/buffer.hpp"

#include <stdexcept>

namespace Simple {

class BufferRef
{
public:
    BufferRef(GL::BufferHandle buffer, std::uintptr_t offset, std::size_t size)
            : m_buffer(buffer), m_offset(static_cast<GLintptr>(offset)), m_size(static_cast<GLsizeiptr>(size))
    {}

    [[nodiscard]] constexpr std::uintptr_t getOffset() const
    { return m_offset; }

    [[nodiscard]] constexpr std::size_t getSize() const
    { return m_size; }

protected:
    GL::BufferHandle m_buffer{};
    GLintptr m_offset{0};
    GLsizeiptr m_size{0};

    void m_write(const void *data) const
    { m_buffer.write(m_offset, m_size, data); }

    void m_read(void *data) const
    { m_buffer.read(m_offset, m_size, data); }

    static void s_copy(const BufferRef &from, const BufferRef &to)
    {
        if (from.m_size != to.m_size)
            throw std::logic_error("attempt to copy data between buffer ranges of different size");

        GL::BufferHandle::copy(from.m_buffer, to.m_buffer, from.m_offset, to.m_offset, from.m_size);
    }

    void m_copyTo(const BufferRef &other) const
    { s_copy(*this, other); }

    void m_copyFrom(const BufferRef &other) const
    { s_copy(other, *this); }
};

class RBufferRef;

class WBufferRef;

struct RBufferRef final : BufferRef
{
    using BufferRef::BufferRef;

    void read(void *data) const
    { m_read(data); }

    inline void copyTo(const WBufferRef &other);
};

struct WBufferRef final : BufferRef
{
public:
    using BufferRef::BufferRef;

    void write(const void *data) const
    { m_write(data); }

    inline void copyFrom(const RBufferRef &other);
};

void RBufferRef::copyTo(const WBufferRef &other)
{ m_copyTo(other); }

void WBufferRef::copyFrom(const RBufferRef &other)
{ m_copyFrom(other); }

struct RWBufferRef final : BufferRef
{
    using BufferRef::BufferRef;

    void write(const void *data) const
    { m_write(data); }

    void read(void *data) const
    { m_read(data); }

    inline void copyTo(const WBufferRef &other)
    { m_copyTo(other); }

    inline void copyFrom(const RBufferRef &other)
    { m_copyFrom(other); }

    [[nodiscard]] WBufferRef toWriteOnlyRef() const
    { return {m_buffer, getOffset(), getSize()}; }

    [[nodiscard]] RBufferRef toReadOnlyRef() const
    { return {m_buffer, getOffset(), getSize()}; }

    [[nodiscard]] operator WBufferRef() const
    { return toWriteOnlyRef(); }

    [[nodiscard]] operator RBufferRef() const
    { return toReadOnlyRef(); }
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_BUFFER_REF_HPP
