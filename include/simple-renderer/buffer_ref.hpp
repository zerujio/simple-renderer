#ifndef PROCEDURALPLACEMENTLIB_BUFFER_REF_HPP
#define PROCEDURALPLACEMENTLIB_BUFFER_REF_HPP

#include "glutils/buffer.hpp"

#include <type_traits>
#include <stdexcept>

namespace simple {

template<bool Read, bool Write>
class BufferRef final
{
public:
    BufferRef() = default;

    BufferRef(GL::BufferHandle buffer, std::uintptr_t offset, std::size_t size)
            : m_buffer(buffer), m_offset(offset), m_size(offset)
    {}

    template<bool NewRead, bool NewWrite>
    constexpr operator const BufferRef<NewRead, NewWrite> &() const
    {
        static_assert((!NewRead || Read) && (!NewWrite || Write),
                      "can't convert to buffer reference type with greater access permissions");

        return reinterpret_cast<const BufferRef<NewRead, NewWrite> &>(*this);
    }

    template<bool NewRead, bool NewWrite>
    constexpr operator BufferRef<NewRead, NewWrite> &()
    { return const_cast<BufferRef<NewRead, NewWrite> &>(this->operator const BufferRef<NewWrite, NewRead> &()); }

    [[nodiscard]] std::uintptr_t getOffset() const
    { return m_offset; }

    [[nodiscard]] std::size_t getSize() const
    { return m_size; }

    std::enable_if_t<Write, void> write(const void *data) const
    { m_buffer.write(m_offset, m_size, data); }

    std::enable_if_t<Read, void> read(void *data) const
    { m_buffer.read(m_offset, m_size, data); }

    template<bool FromWrite, bool ToRead>
    friend void copyBufferData(const BufferRef<true, FromWrite> &from, const BufferRef<ToRead, true> &to);

    template<bool OtherRead>
    std::enable_if_t<Write, void> copyTo(const BufferRef<OtherRead, true> &other) const
    { copyBufferData(*this, other); }

    template<bool OtherWrite>
    std::enable_if_t<Read, void> copyFrom(const BufferRef<true, OtherWrite> &other) const
    { copyBufferData(other, *this); }

private:
    GL::BufferHandle m_buffer{};
    GLintptr m_offset{0};
    GLsizeiptr m_size{0};
};

template<bool FromWrite, bool ToRead>
void copyBufferData(const BufferRef<true, FromWrite> &from, const BufferRef<ToRead, true> &to)
{
    if (from.m_size != to.m_size)
        throw std::logic_error("copy between buffer sections of different size");

    GL::BufferHandle::copy(from.m_buffer, to.m_buffer, from.m_offset, to.m_offset, from.m_size);
}

using RBufferRef = BufferRef<true, false>;
using WBufferRef = BufferRef<false, true>;
using RWBufferRef = BufferRef<true, true>;

/// Polymorphic wrapper for buffer write operations.
struct BufferWriteOperation
{
    virtual void operator()(const RBufferRef &readable_buffer) = 0;
};

/// Polymorphic functor wrapper for BufferRef::read()
class WriteToBuffer : public BufferWriteOperation
{
public:
    WriteToBuffer(size_t size, const void *data) : m_size(size), m_data(data)
    {}

    void operator()(const RBufferRef &readable_buffer) override;

private:
    std::size_t m_size;
    const void *m_data;
};

/// Polymorphic functor wrapper for BufferRef::copyTo()
class CopyToBuffer : public BufferWriteOperation
{
public:

private:
    GL::Buf
};

/// Polymorphic wrapper for buffer read operations.

} // simple

#endif //PROCEDURALPLACEMENTLIB_BUFFER_REF_HPP
