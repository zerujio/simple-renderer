#ifndef SIMPLERENDERER_SHADER_PROGRAM_HPP
#define SIMPLERENDERER_SHADER_PROGRAM_HPP

#include "glutils/program.hpp"
#include "glutils/guard.hpp"

namespace simple {

    /// Holds the data for a shader program.
    class ShaderProgram
    {
        friend class Renderer;
    public:
        /// Compile and link a new GLSL shader program.
        /**
         * Vertex shaders have access to the following vertex attributes:
         *      vec3 vertex_position: the position of the vertex in model space.
         *      vec3 vertex_normal  : the mesh normal at this vertex, in model space.
         *      vec4 vertex_color   : an RGBA color for this vertex. May be unused, in which case it defaults to
         *                          black (i.e. vec4(0, 0, 0, 0)).
         *
         * The only predefined output for the vertex stage is the built-in gl_Position variable.
         *
         * Both vertex and fragment shaders have access to the following uniforms:
         *      mat4 model_matrix   : model space to world space transform matrix.
         *      mat4 view_matrix    : world space to camera space transform matrix.
         *      mat4 proj_matrix    : camera space to clip space transform matrix.
         *
         * Fragment shaders have a single pre-defined output:
         *      vec4 frag_color : final color for this fragment.
         *
         * @param vert_src GLSL code for the vertex shader stage.
         * @param frag_src GLSL code for the fragment shader stage.
         */
        ShaderProgram(const char *vert_src, const char *frag_src);

    private:
        glutils::Guard<glutils::Program> m_program;
    };

} // simple

#endif //SIMPLERENDERER_SHADER_PROGRAM_HPP
