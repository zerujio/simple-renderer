#ifndef SIMPLERENDERER_MESH_HPP
#define SIMPLERENDERER_MESH_HPP

#include <glutils/guard.hpp>
#include <glutils/buffer.hpp>
#include <glutils/vertex_array.hpp>
#include <glutils/gl.hpp>

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

        /// create an array mesh
        explicit Mesh(glutils::Guard<glutils::Buffer> buffer, glutils::Guard<glutils::VertexArray> vertex_array,
                      GLenum draw_mode, GLsizei index_count, GLint first_index) :
              m_buffer(std::move(buffer)),
              m_vertex_array(std::move(vertex_array)),
              m_draw_mode(draw_mode),
              m_index_count(index_count),
              m_array_index_offset(first_index)
        {}

        /// create an indexed mesh
        explicit Mesh(glutils::Guard<glutils::Buffer> buffer, glutils::Guard<glutils::VertexArray> vertex_array,
                      GLenum draw_mode, GLenum index_type, GLsizei index_count, GLsizeiptr index_offset) :
                m_buffer(std::move(buffer)),
                m_vertex_array(std::move(vertex_array)),
                m_draw_mode(draw_mode),
                m_index_type(index_type),
                m_index_count(index_count),
                m_element_index_offset(index_offset)
        {}

        enum class DrawMode
        {
            points  = GL_POINTS,
            lines   = GL_LINES,
            triangles = GL_TRIANGLES
        };

        void setDrawMode(DrawMode draw_mode);
        [[nodiscard]] auto getDrawMode() const -> DrawMode;

        void setElementCount(GLsizei count) {m_index_count = count;}
        [[nodiscard]] auto getElementCount() const {return m_index_count;}

        void setIndexedDrawing(GLenum index_type) {m_index_type = index_type;}
        [[nodiscard]] bool getIndexedDrawing() const {return m_index_type;}
        [[nodiscard]] auto getIndexType() const {return m_index_type;}

        void setArrayDrawing() {m_index_type = 0;}
        [[nodiscard]] bool getArrayDrawing() const {return !m_index_type;}

        /// used for indexed drawing (glDrawElements)
        void setElementBufferOffset(GLsizeiptr offset) {m_element_index_offset = offset;}
        [[nodiscard]] auto getElementBufferOffset() const {return m_element_index_offset;}

        /// used for array drawing (glDrawArrays)
        void setArrayIndexOffset(GLint offset) {m_array_index_offset = offset;}
        [[nodiscard]] auto getArrayIndexOffset() const {return m_array_index_offset;}

    private:
        glutils::Guard<glutils::Buffer> m_buffer;
        glutils::Guard<glutils::VertexArray> m_vertex_array;
        GLenum m_draw_mode {GL_TRIANGLES};
        GLenum m_index_type {0};
        GLsizei m_index_count;
        union {
            GLint m_array_index_offset;
            GLsizeiptr m_element_index_offset;
        };
    };

} // simple

#endif //SIMPLERENDERER_MESH_HPP
