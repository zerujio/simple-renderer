#ifndef SIMPLERENDERER_OSTREAM_OPERATORS_HPP
#define SIMPLERENDERER_OSTREAM_OPERATORS_HPP

#include "glm/fwd.hpp"

#include <ostream>

template<auto L, typename T, auto Q>
std::ostream& operator<< (std::ostream& out, const glm::vec<L, T, Q>& vec)
{
    out << "(" << vec[0];
    for (int i = 1; i < L; i++)
    {
        out << ", " << vec[i];
    }
    return out << ")";
}

#endif //SIMPLERENDERER_OSTREAM_OPERATORS_HPP
