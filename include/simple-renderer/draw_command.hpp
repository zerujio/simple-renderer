#ifndef PROCEDURALPLACEMENTLIB_DRAW_COMMAND_HPP
#define PROCEDURALPLACEMENTLIB_DRAW_COMMAND_HPP

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
    explicit DrawCommand(DrawMode draw_mode) : mode(draw_mode) {}

    /// invoke the corresponding OpenGL command
    virtual void operator() (const GLContext* context) const = 0;

    DrawMode mode;
};

/// glDrawArrays
struct DrawArraysCommand : DrawCommand
{
    explicit DrawArraysCommand(DrawMode draw_mode = DrawMode::points,
                      std::uint32_t first_index = 0,
                      std::uint32_t index_count = 0) :
            DrawCommand(draw_mode), first(first_index), count(index_count) {}

    void operator()(const GLContext *context) const override;

    std::uint32_t first;
    std::uint32_t count;
};

/// glDrawElements
struct DrawElementsCommand : DrawCommand
{
    explicit DrawElementsCommand(DrawMode draw_mode = DrawMode::points,
                        std::uint32_t index_count = 0,
                        IndexType index_type = IndexType::unsigned_int,
                        std::uintptr_t index_buffer_offset = 0) :
            DrawCommand(draw_mode), count(index_count), type(index_type), offset(index_buffer_offset) {}

    void operator()(const GLContext *context) const override;

    std::uint32_t count;
    IndexType type;
    std::uintptr_t offset;
};

/// Base class for instanced drawing commands. Exists mostly as a "tag" class.
struct InstancedDrawCommand
{
    explicit InstancedDrawCommand(std::uint32_t instance_count = 0) : instance_count(instance_count) {}
    std::uint32_t instance_count;
};

/// glDrawArraysInstanced
struct DrawArraysInstancedCommand : DrawArraysCommand, InstancedDrawCommand
{
    template<typename ...Args>
    explicit DrawArraysInstancedCommand(Args&&... args, std::uint32_t instance_count = 0) :
            DrawArraysCommand(std::forward<Args>(args)...), InstancedDrawCommand(instance_count) {}

    void operator()(const GLContext *context) const override;
};

struct DrawElementsInstancedCommand : DrawElementsCommand, InstancedDrawCommand
{
    template<typename ...Args>
    explicit DrawElementsInstancedCommand(Args&&... args, std::uint32_t instance_count = 0) :
            DrawElementsCommand(std::forward<Args>(args)...), InstancedDrawCommand(instance_count) {}

    void operator()(const GLContext *context) const override;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_DRAW_COMMAND_HPP
