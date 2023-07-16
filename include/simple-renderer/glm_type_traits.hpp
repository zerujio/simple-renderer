#ifndef SIMPLERENDERER_GLM_TYPE_TRAITS_HPP
#define SIMPLERENDERER_GLM_TYPE_TRAITS_HPP

#include "glm/fwd.hpp"

#include <type_traits>

namespace Simple {

template<typename T>
constexpr bool is_vector = false;

template<auto L, typename T, auto Q>
constexpr bool is_vector<glm::vec<L, T, Q>> = true;

template<typename T>
constexpr bool is_matrix = false;

template<auto C, auto R, typename T, auto Q>
constexpr bool is_matrix<glm::mat<C, R, T, Q>> = true;

} // namespace Simple

#endif //SIMPLERENDERER_GLM_TYPE_TRAITS_HPP
