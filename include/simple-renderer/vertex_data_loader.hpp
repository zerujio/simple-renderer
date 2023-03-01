#ifndef SIMPLERENDERER_VERTEX_DATA_INITIALIZER_HPP
#define SIMPLERENDERER_VERTEX_DATA_INITIALIZER_HPP

#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include "drawable.hpp"
#include "buffer_ref.hpp"

namespace simple {

/// Lightweight object to reference containers used for mesh construction.
template<typename T>
struct VertexDataInitializer
{
    /// Create an empty initializer.
    VertexDataInitializer() = default;

    /// Initialize from any container type for which implementations of std::size and std::data exist.
    template<typename Container>
    constexpr VertexDataInitializer(const Container &container) // NOLINT(google-explicit-constructor)
            : data(std::data(container)), size(std::size(container))
    {}

    /// Return the size in bytes of the referenced memory block.
    [[nodiscard]] constexpr std::size_t sizeBytes() const
    { return size * sizeof(T); }

    /// Does the initializer point to anything?
    [[nodiscard]] explicit operator bool() const
    { return data && size; }

    const T *data = nullptr;
    std::size_t size = 0;
};

} // simple

#endif //SIMPLERENDERER_VERTEX_DATA_INITIALIZER_HPP
