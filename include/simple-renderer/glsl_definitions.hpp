#ifndef SIMPLERENDERER_GLSL_DEFINITIONS_HPP
#define SIMPLERENDERER_GLSL_DEFINITIONS_HPP

#include "glutils/glsl_syntax.hpp"

namespace simple {


    constexpr auto glsl_version_c_str = "#version 430 core\n";

    // Vertex attribute declarations
    extern const glutils::Definition vertex_position_def;
    extern const glutils::Definition vertex_normal_def;
    extern const glutils::Definition vertex_uv_def;

    // Uniform declarations

    extern const glutils::Definition model_matrix_def;
    extern const glutils::BlockDefinition camera_uniform_block_def;
    extern const std::size_t view_matrix_block_index;
    extern const std::size_t proj_matrix_block_index;

    // Fragment output definitions
    extern const glutils::Definition frag_color_def;
} // simple

#endif //SIMPLERENDERER_GLSL_DEFINITIONS_HPP
