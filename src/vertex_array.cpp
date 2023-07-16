#include "simple-renderer/vertex_array.hpp"

#include "glm/vec2.hpp"

namespace Simple::Renderer {

void VertexArray::bindVertexBuffer(BufferIndex buffer_index, ConstBufferRange<std::byte> buffer_range, uint stride) const
{
    m_gl_object.bindVertexBuffer(buffer_index.value(), buffer_range.m_buffer,
                                 static_cast<GLintptr>(buffer_range.getOffset()),
                                 static_cast<GLint>(stride));
}

void VertexArray::bindAttribute(AttribIndex attrib, BufferIndex buffer) const
{
    m_gl_object.bindAttribute(attrib.value(), buffer.value());
}

void VertexArray::enableAttribute(AttribIndex index) const
{
    m_gl_object.enableAttribute(index.value());
}

void VertexArray::disableAttribute(AttribIndex index) const
{
    m_gl_object.disableAttribute(index.value());
}

void VertexArray::setVertexBufferInstanceDivisor(BufferIndex index, uint divisor) const
{
    m_gl_object.setBindingDivisor(index.value(), divisor);
}

} // Simple::Renderer
