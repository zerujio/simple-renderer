#ifndef SIMPLERENDERER_MESH_HPP
#define SIMPLERENDERER_MESH_HPP

#include "simple-renderer/drawable.hpp"
#include "simple-renderer/vertex_buffer.hpp"
#include "simple-renderer/vertex_array.hpp"
#include "simple-renderer/vertex_attribute_specification.hpp"
#include "simple-renderer/vertex_data_loader.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace Simple {

namespace Renderer {

/// Base class for renderer-able meshes.
class Mesh : public Drawable
{
public:
    Mesh(VertexDataInitializer <glm::vec3> positions, VertexDataInitializer <glm::vec3> normals,
         VertexDataInitializer <glm::vec2> uvs, VertexDataInitializer<unsigned int> indices = {});

    [[nodiscard]] DrawMode getDrawMode() const
    { return m_draw_mode; }

    void setDrawMode(DrawMode mode)
    { m_draw_mode = mode; }

    void collectDrawCommands(const CommandCollector &collector) const override;

    /// Does this mesh use indexed drawing?
    [[nodiscard]] bool isIndexed() const
    { return m_use_index_buffer; }

private:
    template<typename AttributeType, std::size_t SectionIndex>
    void bindAttributes(uint attrib_index);

    [[nodiscard]] DrawElementsCommand m_createDrawElementsCommand() const;

    [[nodiscard]] DrawArraysCommand m_createDrawArraysCommand() const;

    DrawMode m_draw_mode{DrawMode::triangles};

    VertexBuffer<glm::vec3, glm::vec3, glm::vec2, unsigned int> m_vertex_buffer;
    VertexArray m_vertex_array;

    bool m_use_index_buffer;
    std::uint32_t m_index_count;
    std::uint32_t m_first_index;
};

} // namespace Renderer

// old implementation

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

protected:
    template<typename CommandType>
    void m_emplaceDrawCommand(const CommandCollector& collector, CommandType&& command) const
    { m_vertex_specification.emplaceDrawCommand(collector, std::forward<CommandType>(command)); }

    [[nodiscard]] DrawElementsCommand m_createDrawElementsCommand() const;
    [[nodiscard]] DrawArraysCommand m_createDrawArraysCommand() const;

    [[nodiscard]] VertexAttributeSpecification& m_getVertexAttributes() { return m_vertex_specification; }
    [[nodiscard]] const VertexAttributeSpecification& m_getVertexAttributes() const { return m_vertex_specification; }

private:
    DrawMode m_draw_mode{DrawMode::triangles};
    VertexAttributeSpecification m_vertex_specification;
    VertexBuffer m_vertex_buffer;
    std::uint32_t m_index_count;
    union
    {
        std::uint32_t m_first_index;
        std::uintptr_t m_index_buffer_offset;
    };
    bool m_use_index_buffer;
};

} // Simple

#endif //SIMPLERENDERER_MESH_HPP
