#ifndef PROCEDURALPLACEMENTLIB_DRAWABLE_HPP
#define PROCEDURALPLACEMENTLIB_DRAWABLE_HPP

#include "glutils/program.hpp"
#include "glutils/vertex_array.hpp"

namespace simple {

/// Base class for all objects which may be drawn by the renderer.
class Drawable
{
    friend class Renderer;
private:
    /// Issue the OpenGL commands required to draw the primitives of this object.
    virtual void issueDrawCommands() const;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_DRAWABLE_HPP
