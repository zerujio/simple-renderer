#include "simple_renderer/draw_command.hpp"

#include "glutils/gl.hpp"

namespace Simple {

void DrawArraysCommand::operator()() const
{
    glDrawArrays(static_cast<GLenum>(mode), static_cast<GLint>(first), static_cast<GLint>(count));
}

void DrawElementsCommand::operator()() const
{
    glDrawElements(static_cast<GLenum>(mode), static_cast<GLint>(count), static_cast<GLenum>(type), reinterpret_cast<void *>(offset));
}

void DrawArraysInstancedCommand::operator()() const
{
    glDrawArraysInstanced(static_cast<GLenum>(mode), static_cast<GLint>(first), static_cast<GLsizei>(count), static_cast<GLsizei>(instance_count));
}

void DrawElementsInstancedCommand::operator()() const
{
    glDrawElementsInstanced(static_cast<GLenum>(mode), static_cast<GLsizei>(count), static_cast<GLenum>(type),
                            reinterpret_cast<void *>(offset), static_cast<GLsizei>(instance_count));
}

} // simple