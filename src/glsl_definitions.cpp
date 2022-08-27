#include "glsl_definitions.hpp"

namespace simple {

    // Vertex attributes

    const glsl::Definition vertex_position_def
    {
            .layout     {.location = 0},
            .storage    = glsl::Storage::in,
            .type       = glsl::Type::vec3_,
            .name       = "vertex_position"
    };

    const glsl::Definition vertex_normal_def
    {
            .layout     {.location = 1},
            .storage    = glsl::Storage::in,
            .type       = glsl::Type::vec3_,
            .name       = "vertex_normal"
    };

    const glsl::Definition vertex_uv_def
    {
            .layout     {.location = 2},
            .storage    = glsl::Storage::in,
            .type       = glsl::Type::vec2_,
            .name       = "vertex_uv"
    };

    // Uniforms

    const glsl::Definition model_matrix_def
    {
            .layout     {.location = 0},
            .storage    = glsl::Storage::uniform,
            .type       = glsl::Type::mat4_,
            .name       = "model_matrix",
            .init       {"mat4(1.0f)"}
    };

    const glsl::BlockDefinition camera_uniform_block_def
    {
            .layout     {.memory = glsl::Layout::Memory::std140, .binding = 0},
            .storage    = glsl::Storage::uniform,
            .block_name = "Camera",
            .defs {
               {.type = glsl::Type::mat4_, .name = "view_matrix"},
               {.type = glsl::Type::mat4_, .name = "proj_matrix"}
            }
    };
    const std::size_t view_matrix_block_index = 0;
    const std::size_t proj_matrix_block_index = 1;

    // Fragment outputs
    const glsl::Definition frag_color_def
    {
            .storage    = glsl::Storage::out,
            .type       = glsl::Type::vec4_,
            .name       = "frag_color"
    };
} // simple