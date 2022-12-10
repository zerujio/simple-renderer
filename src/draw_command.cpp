#include "simple-renderer/draw_command.hpp"

#include "glutils/gl.hpp"

namespace simple {

void DrawArraysCommand::operator()(const GLContext *context) const
{
    context->DrawArrays(static_cast<GLenum>(mode), first, count);
}

void DrawElementsCommand::operator()(const GLContext *context) const
{
    context->DrawElements(static_cast<GLenum>(mode), count, static_cast<GLenum>(type), reinterpret_cast<void*>(offset));
}

void DrawArraysInstancedCommand::operator()(const GLContext *context) const
{
    context->DrawArraysInstanced(static_cast<GLenum>(mode), first, count, instance_count);
}

void DrawElementsInstancedCommand::operator()(const GLContext *context) const
{
    context->DrawElementsInstanced(static_cast<GLenum>(mode), count, static_cast<GLenum>(type),
                                   reinterpret_cast<void*>(offset), instance_count);
}

} // simple