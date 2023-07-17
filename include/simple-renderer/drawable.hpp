#ifndef PROCEDURALPLACEMENTLIB_DRAWABLE_HPP
#define PROCEDURALPLACEMENTLIB_DRAWABLE_HPP

#include "draw_command.hpp"
#include "command_collector.hpp"

#include "glutils/program.hpp"
#include "glutils/vertex_array.hpp"

namespace Simple::Renderer {

/// Base class for all objects which may be drawn by the renderer.
class Drawable
{
    friend class Context;
public:
    using CommandCollector = RendererCommandSet::Instantiate<CommandCollector>;
protected:
    /// Issue the OpenGL commands required to draw the primitives for this object.
    virtual void collectDrawCommands(const CommandCollector& collector) const = 0;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_DRAWABLE_HPP
