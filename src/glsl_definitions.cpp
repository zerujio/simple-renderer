#include "simple-renderer/glsl_definitions.hpp"

namespace simple {

    using namespace GL;

    // Vertex attributes

    const Definition vertex_position_def
    {
            .layout     {.location = 0},
            .storage    = StorageQualifier::in,
            .type       = Type::vec3_,
            .name       = "vertex_position"
    };

    const Definition vertex_normal_def
    {
            .layout     {.location = 1},
            .storage    = StorageQualifier::in,
            .type       = Type::vec3_,
            .name       = "vertex_normal"
    };

    const Definition vertex_uv_def
    {
            .layout     {.location = 2},
            .storage    = StorageQualifier::in,
            .type       = Type::vec2_,
            .name       = "vertex_uv"
    };

    // Uniforms

    const Definition model_matrix_def
    {
            .layout     {.location = 0},
            .storage    = StorageQualifier::uniform,
            .type       = Type::mat4_,
            .name       = "model_matrix",
            .init       {"mat4(1.0f)"}
    };

    const BlockDefinition camera_uniform_block_def
    {
            .layout     {.memory = LayoutQualifiers::Memory::std140, .binding = 0},
            .storage    = StorageQualifier::uniform,
            .block_name = "Camera",
            .defs {
               {.type = Type::mat4_, .name = "view_matrix"},
               {.type = Type::mat4_, .name = "proj_matrix"}
            }
    };
    const std::size_t view_matrix_block_index = 0;
    const std::size_t proj_matrix_block_index = 1;

    // Fragment outputs
    const Definition frag_color_def
    {
            .storage    = StorageQualifier::out,
            .type       = Type::vec4_,
            .name       = "frag_color"
    };
} // simple