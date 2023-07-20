#ifndef PROCEDURALPLACEMENTLIB_RENDERER_EXAMPLE_COMMON_HPP
#define PROCEDURALPLACEMENTLIB_RENDERER_EXAMPLE_COMMON_HPP

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <vector>

struct Cube {
    static const std::vector<glm::vec3> vertex_positions;
    static const std::vector<glm::vec3> vertex_normals;
    static const std::vector<glm::vec2> vertex_uvs;
    static const std::vector<unsigned int> indices;
};

#endif //PROCEDURALPLACEMENTLIB_RENDERER_EXAMPLE_COMMON_HPP
