#ifndef PROCEDURALPLACEMENTLIB_DRAW_COMMAND_HPP
#define PROCEDURALPLACEMENTLIB_DRAW_COMMAND_HPP

#include "type_set.hpp"

#include <cstdint>
#include <utility>

struct GladGLContext;
using GLContext = GladGLContext;

namespace simple {

enum class DrawMode
{
    points                      = 0x0000,
    lines                       = 0x0001,
    line_loop                   = 0x0002,
    line_strip                  = 0x0003,
    triangles                   = 0x0004,
    triangle_strip              = 0x0005,
    triangle_fan                = 0x0006,
    lines_adjacency             = 0x000A,
    line_strip_adjacency        = 0x000B,
    triangles_adjacency         = 0x000C,
    triangle_strip_adjacency    = 0x000D,
    patches                     = 0x000E
};

/// Valid index types for DrawElements* commands.
enum class IndexType
{
    unsigned_byte   = 0x1401,
    unsigned_short  = 0x1403,
    unsigned_int    = 0x1405
};

/// base class for drawing commands
struct DrawCommand
{
    DrawCommand() = default;
    explicit DrawCommand(DrawMode draw_mode) : mode(draw_mode) {}

    /// invoke the corresponding OpenGL command
    virtual void operator() (const GLContext& context) const = 0;

    DrawMode mode {DrawMode::points};
};

/// glDrawArrays
struct DrawArraysCommand : DrawCommand
{
    DrawArraysCommand() = default;
    DrawArraysCommand(DrawMode draw_mode, std::uint32_t first_index, std::uint32_t index_count) :
            DrawCommand(draw_mode), first(first_index), count(index_count) {}

    void operator()(const GLContext &context) const override;

    std::uint32_t first {0};
    std::uint32_t count {0};
};

/// glDrawElements
struct DrawElementsCommand : DrawCommand
{
    DrawElementsCommand() = default;
    DrawElementsCommand(DrawMode draw_mode, std::uint32_t index_count, IndexType index_type,
                        std::uintptr_t index_buffer_offset) :
            DrawCommand(draw_mode), count(index_count), type(index_type), offset(index_buffer_offset) {}

    void operator()(const GLContext &context) const override;

    std::uint32_t count {0};
    IndexType type {IndexType::unsigned_int};
    std::uintptr_t offset {0};
};

/// Base class for instanced drawing commands. Exists mostly as a "tag" class.
struct InstancedDrawCommand
{
    InstancedDrawCommand() = default;
    explicit InstancedDrawCommand(std::uint32_t instance_count) : instance_count(instance_count) {}
    std::uint32_t instance_count {0};
};

/// glDrawArraysInstanced
struct DrawArraysInstancedCommand : DrawArraysCommand, InstancedDrawCommand
{
    DrawArraysInstancedCommand() = default;
    DrawArraysInstancedCommand(DrawMode draw_mode, std::uint32_t first_index, std::uint32_t index_count,
                               std::uint32_t instance_count) :
            DrawArraysCommand(draw_mode, first_index, index_count), InstancedDrawCommand(instance_count) {}

    void operator()(const GLContext &context) const override;
};

struct DrawElementsInstancedCommand : DrawElementsCommand, InstancedDrawCommand
{
    DrawElementsInstancedCommand() = default;
    DrawElementsInstancedCommand(DrawMode draw_mode, std::uint32_t index_count, IndexType index_type,
                                 std::uintptr_t index_buffer_offset, std::uint32_t instance_count) :
            DrawElementsCommand(draw_mode, index_count, index_type, index_buffer_offset),
            InstancedDrawCommand(instance_count) {}

    void operator()(const GLContext &context) const override;
};

using RendererCommandSet = TypeSet<DrawArraysCommand, DrawElementsCommand, DrawArraysInstancedCommand, DrawElementsInstancedCommand>;

} // simple

#endif //PROCEDURALPLACEMENTLIB_DRAW_COMMAND_HPP
