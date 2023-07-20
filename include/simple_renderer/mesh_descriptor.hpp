#ifndef SIMPLERENDERER_MESH_DESCRIPTOR_HPP
#define SIMPLERENDERER_MESH_DESCRIPTOR_HPP

#include "simple_renderer/vertex_buffer.hpp"

#include "glm/fwd.hpp"

namespace Simple::Renderer {

enum class VertexDataType
{
    int_8, int_16, int_32,
    uint_8, uint_16, uint_32,

};

/// Describes how a vertex attribute of type @p T is stored within a vertex buffer.
template<typename>
class VertexAttributeFormat
{
public:

private:

};

/// Describes how vertex data from one or more vertex buffers is to be interpreted to create a mesh.
struct MeshDescriptor
{
public:

private:
};

} // Simple::Renderer

#endif //SIMPLERENDERER_MESH_DESCRIPTOR_HPP
