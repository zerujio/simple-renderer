#ifndef SIMPLERENDERER_GLM_TYPE_TRAITS_HPP
#define SIMPLERENDERER_GLM_TYPE_TRAITS_HPP

#include "glm/fwd.hpp"

#include <type_traits>

namespace Simple {

template<typename T>
struct IsVector
{
    static constexpr bool value = false;
};

template<auto L, typename T, auto Q>
struct IsVector<glm::vec<L, T, Q>>
{
    static constexpr bool value = true;
};

/// True if T is a glm vector type
template<typename T>
constexpr bool IsVectorV = IsVector<T>::value;


template<typename T>
struct IsMatrix
{
    static constexpr bool value = false;
};

template<auto C, auto R, typename T, auto Q>
struct IsMatrix<glm::mat<C, R, T, Q>>
{
    static constexpr bool value = true;
};

// true if T is a glm matrix type
template<typename T>
constexpr bool IsMatrixV = IsMatrix<T>::value;

} // namespace Simple

#endif //SIMPLERENDERER_GLM_TYPE_TRAITS_HPP
