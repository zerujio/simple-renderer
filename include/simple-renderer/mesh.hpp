#ifndef SIMPLERENDERER_MESH_HPP
#define SIMPLERENDERER_MESH_HPP

#include "simple-renderer/drawable.hpp"
#include "simple-renderer/vertex_buffer.hpp"
#include "simple-renderer/vertex_array.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace Simple::Renderer {

/// Base class for renderer-able meshes.
class Mesh : public Drawable
{
public:
    Mesh(VertexDataInitializer<glm::vec3> positions, VertexDataInitializer<glm::vec3> normals,
         VertexDataInitializer<glm::vec2> uvs, VertexDataInitializer<unsigned int> indices = {});

    [[nodiscard]] DrawMode getDrawMode() const
    { return m_draw_mode; }

    void setDrawMode(DrawMode mode)
    { m_draw_mode = mode; }

    void collectDrawCommands(const CommandCollector &collector) const override;

    /// Does this mesh use indexed drawing?
    [[nodiscard]] bool isIndexed() const { return m_use_index_buffer; }

private:
    [[nodiscard]] DrawElementsCommand m_createDrawElementsCommand() const;
    [[nodiscard]] DrawArraysCommand m_createDrawArraysCommand() const;

    DrawMode m_draw_mode{DrawMode::triangles};

    VertexBuffer<glm::vec3, glm::vec3, glm::vec2, unsigned int> m_vertex_buffer;
    VertexArray m_vertex_array;

    bool m_use_index_buffer;
    std::uint32_t m_index_count;
    std::uint32_t m_first_index;
};

} // Simple::Renderer

#endif //SIMPLERENDERER_MESH_HPP
