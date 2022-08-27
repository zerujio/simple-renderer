#ifndef SIMPLERENDERER_MESH_HPP
#define SIMPLERENDERER_MESH_HPP

#include <glutils/guard.hpp>
#include <glutils/buffer.hpp>
#include <glutils/vertex_array.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

namespace simple {

    /// A geometric mesh.
    class Mesh
    {
        friend class Renderer;
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
        Mesh(const std::vector<glm::vec3>& positions,
             const std::vector<glm::vec3>& normals = {},
             const std::vector<glm::vec2>& uvs = {},
             const std::vector<GLuint>& indices = {});

        enum class DrawMode
        {
            points  = GL_POINTS,
            lines   = GL_LINES,
            triangles = GL_TRIANGLES
        };

        void setDrawMode(DrawMode draw_mode);
        auto getDrawMode() const -> DrawMode;

    private:
        glutils::Guard<glutils::Buffer> m_vertex_buffer;
        glutils::Guard<glutils::Buffer> m_element_buffer;
        glutils::Guard<glutils::VertexArray> m_vertex_array;
        GLenum m_draw_mode {GL_TRIANGLES};
        GLenum m_index_type;
        GLsizei m_index_count;
        union {
            GLint m_array_index_offset;
            GLsizeiptr m_element_index_offset;
        };
    };

} // simple

#endif //SIMPLERENDERER_MESH_HPP
