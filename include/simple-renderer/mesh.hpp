#ifndef SIMPLERENDERER_MESH_HPP
#define SIMPLERENDERER_MESH_HPP

#include "drawable.hpp"

#include "glutils/guard.hpp"
#include "glutils/buffer.hpp"
#include "glutils/vertex_array.hpp"
#include "glutils/gl_types.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include <vector>

namespace simple {

    /// A geometric mesh.
    class Mesh : public Drawable
    {
    public:
        
        /// Create a mesh from vertex data.
        /**
         *
         * @param vertex_position Array of vertex positions. Must not be empty.
         * @param normals Array of vertex normals. If not empty, must match the size of @p vertex_position.
         * @param uvs Array of vertex colors. If not empty, must match the size of @p vertex_position.
         * @param indices Indices for indexed drawing; may be empty, in which case the vertices are drawn in the order
         *      they appear in @p indices.
         */
        explicit Mesh(const std::vector<glm::vec3>& positions,
             const std::vector<glm::vec3>& normals = {},
             const std::vector<glm::vec2>& uvs = {},
             const std::vector<glutils::GLuint>& indices = {});

        /// create an array mesh
        explicit Mesh(glutils::Guard<glutils::Buffer> buffer, glutils::Guard<glutils::VertexArray> vertex_array,
                      DrawMode draw_mode, std::uint32_t index_count, std::uint32_t first_index,
                      std::uint32_t instance_count = 0) :
              m_buffer(std::move(buffer)),
              m_vertex_array(std::move(vertex_array)),
              m_draw_mode(draw_mode),
              m_index_count(index_count),
              m_draw_indexed(false),
              m_first_index(first_index),
              m_instance_count(instance_count)
        {}

        /// create an indexed mesh
        explicit Mesh(glutils::Guard<glutils::Buffer> buffer, glutils::Guard<glutils::VertexArray> vertex_array,
                      DrawMode draw_mode, IndexType index_type, std::uint32_t index_count, std::uintptr_t index_offset,
                      std::uint32_t instance_count = 0) :
                m_buffer(std::move(buffer)),
                m_vertex_array(std::move(vertex_array)),
                m_draw_mode(draw_mode),
                m_index_count(index_count),
                m_draw_indexed(true),
                m_index_buffer{index_type, index_offset},
                m_instance_count(instance_count)
        {}

        [[nodiscard]] DrawMode getDrawMode() const {return m_draw_mode;}
        void setDrawMode(DrawMode mode) {m_draw_mode = mode;}

        [[nodiscard]] auto getElementCount() const {return m_index_count;}

        [[nodiscard]] bool getIndexedDrawing() const {return m_draw_indexed;}
        [[nodiscard]] auto getIndexType() const {return m_index_buffer.type;}

        /// used for indexed drawing (glDrawElements)
        [[nodiscard]] auto getElementBufferOffset() const {return m_index_buffer.offset;}

        /// used for array drawing (glDrawArrays)
        [[nodiscard]] auto getArrayIndexOffset() const {return m_first_index;}

        [[nodiscard]] auto getInstanceCount() const {return m_instance_count;}
        [[nodiscard]] bool usesInstancedDrawing() const {return getInstanceCount();}

        void collectDrawCommands(const CommandCollector &collector) const override;

    private:
        glutils::Guard<glutils::Buffer> m_buffer;
        glutils::Guard<glutils::VertexArray> m_vertex_array;

        DrawMode m_draw_mode {DrawMode::points};
        std::uint32_t m_index_count;
        bool m_draw_indexed;
        union
        {
            struct
            {
                IndexType type = IndexType::unsigned_int;
                std::uintptr_t offset;
            } m_index_buffer;

            std::uint32_t m_first_index;
        };
        std::uint32_t m_instance_count {0};   // used for instanced rendering
    };

} // simple

#endif //SIMPLERENDERER_MESH_HPP
